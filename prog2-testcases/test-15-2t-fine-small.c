#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUM_THREADS 2

#define NUMS_64   1572

/**
 * Forked from testcase 6, but uses fine-grained allocation instead
*/
void * test(void * args) {
  char buf[64];
  void ** small_chunks = malloc(sizeof(void *) * NUMS_64);
  long fail = 0;

  if (!small_chunks) {
    perror("System malloc returned NULL!\n");
    return (void *) -1;
  }

  for (int i = 0; i < NUMS_64; ++i) {
    small_chunks[i] = myMalloc(64);
    memset(small_chunks[i], i, 64);
  }

  for (int i = 0; i < NUMS_64; ++i) {
    memset(buf, i, 64);
    if (memcmp(small_chunks[i], buf, 64) != 0) {
      fail = 1;
    }
    myFree(small_chunks[i]);
  }

  return (void *) fail;
}

int main(int argc, char *argv[]) {
  if (myInit(NUM_THREADS, 2) == -1) {
    perror("*** myInit() failed. ***\n");
    return 1;
  }

  pthread_t t0, t1;
  if (pthread_create(&t0, NULL, test, (void *) 0)) {
    perror("*** CRITICAL: Could not create thread 0 ***\n");
    return -1;
  }
  if (pthread_create(&t1, NULL, test, (void *) 1)) {
    perror("*** CRITICAL: Could not create thread 1 ***\n");
    return -1;
  }

  long verdict = 0;
  pthread_join(t0, (void *) &verdict);
  if (verdict) {
    fprintf(stderr, "*** test-15-2t-fine-small failed: thread 0 reported chunk %ld was overwritten ***\n", verdict-1);
    return 1;
  }
  pthread_join(t1, (void *) &verdict);
  if (verdict) {
    fprintf(stderr, "*** test-15-2t-fine-small failed: thread 1 reported chunk %ld was overwritten ***\n", verdict-1);
    return 1;
  }

  char * base = "ls Id-%d";
  char cmd[64];
  for (int i = 1; i <= NUM_THREADS; ++i) {
    snprintf(cmd, 64, base, i);
    if (system(cmd)) {
      fprintf(stderr, "*** test-15-2t-fine-small failed: file Id-%d not found ***\n", i);
      return 1;
    }
  }

  base = "ls Id-%d >/dev/null 2>&1";
  for (int i = NUM_THREADS+1; i <= 9; ++i) {
    snprintf(cmd, 64, base, i);
    if (!system(cmd)) { // These should fail!
      fprintf(stderr, "*** test-15-2t-fine-small failed: file Id-%d FOUND when it should NOT be! ***\n", i);
      return 1;
    }
  }

  puts("*** test-15-2t-fine-small PASSED ***");
  return 0;
}
