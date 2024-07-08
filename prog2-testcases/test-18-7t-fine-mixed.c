#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUM_THREADS 7

#define NUMS_64   1572
#define NUMS_1024 132

/**
 * Forked from testcase 9, but uses fine-grained allocation instead
*/
void * test(void * args) {
  long fail = 0;
  void * chunks[128];
  char buf[64];

  for (int i = 0; i < 128; ++i) {
    chunks[i] = myMalloc(64);
    if (!chunks[i]) {
      perror("ERROR: myMalloc returned NULL\n");
      fail = 1;
    }
    memset(chunks[i], i, 64);
  }

  for (int i = 0; i < 128; ++i) {
    memset(buf, i, 64);
    if (memcmp(chunks[i], buf, 64) != 0) {
      fprintf(stderr, "ERROR: Chunk %d was overwritten!\n", i);
      fail = 1;
    }
    myFree(chunks[i]);
  }

  return (void *) fail;
}

int main(int argc, char *argv[]) {
  if (myInit(NUM_THREADS, 2) == -1) {
    perror("*** myInit() failed. ***\n");
    return 1;
  }

  pthread_t threads[7];
  long rc;

  for (int i = 0; i < NUM_THREADS; ++i) {
    rc = (long) pthread_create(&threads[i], NULL, test, NULL);

    if (rc) {
      fprintf(stderr, "*** CRITICAL: Cannot create thread %d ***\n", i);
      return -1;
    }
  }

  for (int i = 0; i < 7; i++) {
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
      fprintf(stderr, "*** test-18-7t-fine-mixed failed: file Id-%d not found ***\n", i);
      return 1;
    }
  }

base = "ls Id-%d >/dev/null 2>&1";
  for (int i = NUM_THREADS+1; i <= 9; ++i) {
    snprintf(cmd, 64, base, i);
    if (!system(cmd)) { // These should fail!
      fprintf(stderr, "*** test-18-7t-fine-mixed failed: file Id-%d FOUND when it should NOT be! ***\n", i);
      return 1;
    }
  }

  printf("*** test-18-7t-fine-mixed PASSED ***\n");
  return 0;
}
