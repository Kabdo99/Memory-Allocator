#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUMS_64   1572
#define NUMS_1024 132

#define TO_ALLOC_SMALL 224
#define TO_ALLOC_LARGE  18

/**
 * Using 7 threads, allocate most of the small & large chunks.
*/
void * test(void * args) {
  long fail = 0;
  void * small_chunks[TO_ALLOC_SMALL];
  void * large_chunks[TO_ALLOC_LARGE];
  char buf[1024];

  for (int i = 0; i < TO_ALLOC_SMALL; ++i) {
    small_chunks[i] = myMalloc(64);
    if (!small_chunks[i]) {
      perror("ERROR: myMalloc returned NULL\n");
      fail = 1;
    }
    memset(small_chunks[i], i, 64);
  }

  for (int i = 0; i < TO_ALLOC_LARGE; ++i) {
    large_chunks[i] = myMalloc(1024);
    if (!large_chunks[i]) {
      perror("ERROR: myMalloc returned NULL\n");
      fail = 1;
    }
    memset(large_chunks[i], i, 1024);
  }

  for (int i = 0; i < TO_ALLOC_SMALL; ++i) {
    memset(buf, i, 64);
    if (memcmp(small_chunks[i], buf, 64) != 0) {
      fprintf(stderr, "ERROR: Small chunk %d was overwritten!\n", i);
      fail = 1;
    }
    myFree(small_chunks[i]);
  }

  for (int i = 0; i < TO_ALLOC_LARGE; ++i) {
    memset(buf, i, 1024);
    if (memcmp(large_chunks[i], buf, 1024) != 0) {
      fprintf(stderr, "ERROR: Small chunk %d was overwritten!\n", i);
      fail = 1;
    }
    myFree(large_chunks[i]);
  }

  return (void *) fail;
}

int main(int argc, char *argv[]) {
  if (myInit(7, 1) == -1) {
    perror("*** myInit() failed. ***\n");
    return 1;
  }

  pthread_t threads[7];
  long rc;

  for (int i = 0; i < 7; ++i) {
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

  printf("*** test-09-7t-coarse-mixed PASSED ***\n");
  return 0;
}
