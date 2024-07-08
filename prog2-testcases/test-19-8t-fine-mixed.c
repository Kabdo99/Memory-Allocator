#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUM_THREADS 8

#define NUMS_64   1572
#define NUMS_1024 132

/**
 * Forked from testcase 10, but uses fine-grained allocation.
*/
void * test(void * args) {
  long fail = 0;
  char buf[1024];

  void ** small_chunks = malloc(NUMS_64 * sizeof(void *));
  void ** large_chunks = malloc(NUMS_1024 * sizeof(void *));
  
  if (!small_chunks || !large_chunks) {
    perror("System malloc returned NULL!\n");
    return (void *) -1;
  }

  // Grab most (not quite all) chunks
  for (int i = 0; i < NUMS_64; ++i) {
    small_chunks[i] = myMalloc(64);
    if (!small_chunks[i]) {
      perror("ERROR: myMalloc returned NULL\n");
      fail = 1;
    }
    memset(small_chunks[i], i, 64);
  }

  for (int i = 0; i < NUMS_1024; ++i) {
    large_chunks[i] = myMalloc(1024);
    if (!large_chunks[i]) {
      perror("ERROR: myMalloc returned NULL\n");
      fail = 1;
    }
    memset(large_chunks[i], i, 1024);
  }

  // Check
  for (int i = 0; i < NUMS_64; ++i) {
    memset(buf, i, 64);
    if (memcmp(small_chunks[i], buf, 64) != 0) {
      fprintf(stderr, "ERROR: Small chunk %d was overwritten!\n", i);
      fail = 1;
    }
  }
  for (int i = 0; i < NUMS_1024; ++i) {
    memset(buf, i, 1024);
    if (memcmp(large_chunks[i], buf, 1024) != 0) {
      fprintf(stderr, "ERROR: Large chunk %d was overwritten!\n", i);
      fail = 1;
    }
  }

  // Release all memory
  for (int i = 0; i < NUMS_64; ++i) {
    myFree(small_chunks[i]);
  }
  for (int i = 0; i < NUMS_1024; ++i) {
    myFree(large_chunks[i]);
  }

  free(small_chunks);
  free(large_chunks);

  return (void *) fail;
}

int main(int argc, char *argv[]) {
  if (myInit(NUM_THREADS, 2) == -1) {
    perror("*** myInit() failed. ***\n");
    return 1;
  }

  pthread_t threads[NUM_THREADS];
  long rc;

  for (int i = 0; i < NUM_THREADS; ++i) {
    rc = (long) pthread_create(&threads[i], NULL, test, NULL);

    if (rc) {
      fprintf(stderr, "*** CRITICAL: Cannot create thread %d ***\n", i);
      return -1;
    }
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], (void *) &rc);
    if (rc) {
      fprintf(stderr, "ERROR: Thread %d exited with a non-zero status\n", i);
    }
  }

  char * base = "ls Id-%d";
  char cmd[64];
  for (int i = 1; i <= NUM_THREADS; ++i) {
    snprintf(cmd, 64, base, i);
    if (system(cmd)) {
      fprintf(stderr, "*** test-19-8t-fine-mixed failed: file Id-%d not found ***\n", i);
      return 1;
    }
  }

base = "ls Id-%d >/dev/null 2>&1";
  for (int i = NUM_THREADS+1; i <= 9; ++i) {
    snprintf(cmd, 64, base, i);
    if (!system(cmd)) { // These should fail!
      fprintf(stderr, "*** test-19-8t-fine-mixed failed: file Id-%d FOUND when it should NOT be! ***\n", i);
      return 1;
    }
  }

  printf("*** test-19-8t-fine-mixed PASSED ***\n");
  return 0;
}
