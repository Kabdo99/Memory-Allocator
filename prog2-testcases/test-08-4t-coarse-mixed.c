#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUMS_64   1572
#define NUMS_1024 132

/**
 * Two threads will allocate small chunks, while the other two will request
 * large chunks.
*/
void * testSmall(void * args) {
  long fail = 0;
  char buf[64];

  void ** chunks = malloc(sizeof(void *) * NUMS_64 / 2);
  if (!chunks) {
    perror("System malloc returned NULL!\n");
    return (void *) 1;
  }

  for (int i = 0; i < NUMS_64 / 2; ++i) {
    chunks[i] = myMalloc(64);
    memset(chunks[i], i, 64);
  }

  for (int i = 0; i < NUMS_64 / 2; ++i) {
    memset(buf, i, 64);
    if (memcmp(chunks[i], buf, 64) != 0) {
      fail = 1;
    }
    myFree(chunks[i]);
  }

  return (void *) fail;
}

void * testLarge(void * args) {
  long fail = 0;
  char buf[1024];

  void ** chunks = malloc(sizeof(void *) * NUMS_1024 / 2);
  if (!chunks) {
    perror("System malloc returned NULL!\n");
    return (void *) 1;
  }

  for (int i = 0; i < NUMS_1024 / 2; ++i) {
    chunks[i] = myMalloc(1024);
    memset(chunks[i], i, 1024);
  }

  for (int i = 0; i < NUMS_1024 / 2; ++i) {
    memset(buf, i, 1024);
    if (memcmp(chunks[i], buf, 1024) != 0) {
      fail = 1;
    }
    myFree(chunks[i]);
  }

  return (void *) fail;
}

int main(int argc, char *argv[]) {
  if (myInit(4, 1) == -1) {
    perror("*** myInit() failed. ***\n");
    return 1;
  }

  pthread_t threads[4];
  long rc;

  for (int i = 0; i < 4; ++i) {
    if (i % 2 == 0) {
      rc = (long) pthread_create(&threads[i], NULL, testSmall, NULL);
    }
    else {
      rc = (long) pthread_create(&threads[i], NULL, testLarge, NULL);
    }

    if (rc) {
      fprintf(stderr, "*** CRITICAL: Cannot create thread %d ***\n", i);
      return -1;
    }
  }

  for (int i = 0; i < 4; i++) {
    pthread_join(threads[i], (void *) &rc);
    if (rc) {
      fprintf(stderr, "ERROR: Thread %d exited with a non-zero status\n", i);
    }
  }

  printf("*** test-08-4t-coarse-mixed PASSED ***\n");
  return 0;
}
