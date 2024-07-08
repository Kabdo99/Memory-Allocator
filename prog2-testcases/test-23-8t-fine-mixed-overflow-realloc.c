#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUM_THREADS 8

#define DEBUG 1

pthread_barrier_t barrier;

/**
 * Forked from testcase 18; threads will need to reach the global overflow list,
*/
void * test(void * args) {
  long fail = 0;
  char buf[1024];

  // Each thread would allocate this many small and large chunks, respectively
  int NUMS_64 = 1768, NUMS_1024 = 148;

  // This will leave 4 chunks of each type unallocated - set thread 0 to
  // gobble up the rest of them
  long id = (long) args;
  if (id == 0) {
    NUMS_64 += 4;
    NUMS_1024 += 4;
  }

  void ** smallChunks = malloc(NUMS_64 * sizeof(void *));
  void ** largeChunks = malloc(NUMS_1024 * sizeof(void *));

  if (!smallChunks || !largeChunks) {
    perror("System malloc returned NULL!\n");
    return (void *) -1;
  }

  for (int iter = 0; iter < 4; ++iter) {
    // Allocate all the memory first
    for (int i = 0; i < NUMS_64; ++i) 
      smallChunks[i] = myMalloc(64);

    for (int i = 0; i < NUMS_1024; ++i) {
      largeChunks[i] = myMalloc(1024);
    }
      

    // Then try to write to the allocated chunks
    for (int i = 0; i < NUMS_64; ++i)
      memset(smallChunks[i], i, 64);

    for (int i = 0; i < NUMS_1024; ++i)
      memset(largeChunks[i], i, 1024);

    // test
    for (int i = 0; i < NUMS_64; ++i) {
      memset(buf, i, 64);
      if (memcmp(smallChunks[i], buf, 64) != 0)
        fail = 1;

      myFree(smallChunks[i]);
    }

    for (int i = 0; i < NUMS_1024; ++i) {
      memset(buf, i, 1024);
      if (memcmp(largeChunks[i], buf, 1024) != 0)
        fail = 1;
        
      myFree(largeChunks[i]);
    }
  }

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
    rc = (long) pthread_create(&threads[i], NULL, test, (void *)(long) i);

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

  if (system("ls Overflow")) {
    perror("*** test-23-8t-fine-mixed-overflow failed: file Overflow not found ***");
    return 1;
  }

  char * base = "ls Id-%d";
  char cmd[64];
  for (int i = 1; i <= NUM_THREADS; ++i) {
    snprintf(cmd, 64, base, i);
    if (system(cmd)) {
      fprintf(stderr, "*** test-23-8t-fine-mixed-overflow failed: file Id-%d not found ***\n", i);
      return 1;
    }
  }

  printf("*** test-23-8t-fine-mixed-overflow PASSED ***\n");
  return 0;
}
