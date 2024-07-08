#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUMS_64   1572
#define NUMS_1024 132

/**
 * Create 2 threads. Thread 0 only ever requests small chunks,
 * and thread 1 only ever requests large chunks. Both threads will write to the
 * entire allocated chunk, then look for signs of memory being overwritten by
 * the other thread.
*/
void * testSmall(void * args) {
  char buf[64];
  void ** small_chunks = malloc(sizeof(void *) * NUMS_64);
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
      fprintf(stderr, "ERROR: Small chunk at index %d was overwritten!\n", i);
    }

    myFree(small_chunks[i]);
  }

  free(small_chunks);
  return (void *) 0;
}

void * testLarge(void * args) {
  char buf[1024];
  void ** large_chunks = malloc(sizeof(void *) * NUMS_1024);
  if (!large_chunks) {
    perror("System malloc returned NULL!\n");
    return (void *) -1;
  }

  for (int i = 0; i < NUMS_1024; ++i) {
    large_chunks[i] = myMalloc(1024);
    memset(large_chunks[i], i, 1024);
  }

  for (int i = 0; i < NUMS_1024; ++i) {
    memset(buf, i, 1024);

    if (memcmp(large_chunks[i], buf, 1024) != 0) {
      fprintf(stderr, "ERROR: Large chunk at index %d was overwritten!\n", i);
    }
    myFree(large_chunks[i]);
  }

  free(large_chunks);
  return (void *) 0;
}

int main(int argc, char *argv[]) {
  if (myInit(2, 1) == -1) {
    perror("*** test-05-2t-coarse-no-overlap: myInit() failed. ***\n");
    return 1;
  }

  pthread_t t0, t1;
  if (pthread_create(&t0, NULL, testSmall, (void *) 0)) {
    perror("*** CRITICAL: Could not create thread ***\n");
    return -1;
  }
  if (pthread_create(&t1, NULL, testLarge, (void *) 1)) {
    perror("*** CRITICAL: Could not create thread ***\n");
    return -1;
  }

  long verdict = 0;
  pthread_join(t0, (void *) &verdict);
  if (verdict) {
    perror("*** test-05-2t-coarse-no-overlap failed: thread 0 exited with a non-zero status ***\n");
    return 1;
  }
  pthread_join(t1, (void *) verdict);
  if (verdict) {
    perror("*** test-05-2t-coarse-no-overlap failed: thread 1 exited with a non-zero status***\n");
    return 1;
  }

  puts("*** test-05-2t-coarse-no-overlap PASSED ***");
  return 0;
}
