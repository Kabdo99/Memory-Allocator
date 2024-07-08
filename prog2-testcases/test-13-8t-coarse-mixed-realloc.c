#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUM_THREADS 8

pthread_barrier_t barrier;

/**
 * 8 threads that, together, exhaustively allocate all small & large chunks,
 * then release them, and re-run another 3 times to make sure nodes are properly restored.
*/
void * test(void * args) {
  long fail = 0;
  char buf[1024];


  // number of small and large chunks allocated by each thread
  int NUMS_64 = 196, NUMS_1024 = 16;

  // thread 0 allocates 4 extra of each type to truly exhaust all available memory
  long id = (long) args;
  if (id == 0) {
    NUMS_64 += 4;
    NUMS_1024 += 4;
  }

  void ** small_chunks = malloc(NUMS_64 * sizeof(void *));
  void ** large_chunks = malloc(NUMS_1024 * sizeof(void *));
  
  if (!small_chunks || ! large_chunks) {
    perror("System malloc returned NULL!\n");
    return (void *) -1;
  }

  for (int iter = 0; iter < 4; ++iter) {
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
    
    pthread_barrier_wait(&barrier);

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

    pthread_barrier_wait(&barrier);
  }
  

  free(small_chunks);
  free(large_chunks);

  return (void *) fail;
}

int main(int argc, char *argv[]) {
  if (myInit(NUM_THREADS, 1) == -1) {
    perror("*** myInit() failed. ***\n");
    return 1;
  }

  pthread_barrier_init(&barrier, NULL, NUM_THREADS);

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

  pthread_barrier_destroy(&barrier);

  printf("*** test-13-8t-coarse-mixed PASSED ***\n");
  return 0;
}
