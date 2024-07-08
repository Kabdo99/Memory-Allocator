#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUMS_64   10

/**
 * Both threads will (together) exhaustively request all small chunks.
*/
void * test(void * args) {
  char buf[64];
  void ** small_chunks = malloc(sizeof(void *) * NUMS_64/2);
  long fail = 0;

  if (!small_chunks) {
    perror("System malloc returned NULL!\n");
    return (void *) -1;
  }

  for (int i = 0; i < NUMS_64/2; ++i) {
    small_chunks[i] = myMalloc(64);
    memset(small_chunks[i], i, 64);
  }

  for (int i = 0; i < NUMS_64/2; ++i) {
    memset(buf, i, 64);
    if (memcmp(small_chunks[i], buf, 64) != 0) {
      fail = 1;
    }
    myFree(small_chunks[i]);
  }

  return (void *) fail;
}

int main(int argc, char *argv[]) {
  if (myInit(2, 1) == -1) {
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
    perror("*** test-06-2t-coarse-small failed: thread 0 exited with a non-zero status ***\n");
    return 1;
  }
  pthread_join(t1, (void *) &verdict);
  if (verdict) {
    perror("*** test-06-2t-coarse-small failed: thread 1 exited with a non-zero status ***\n");
    return 1;
  }

  puts("*** test-06-2t-coarse-small PASSED ***");
  return 0;
}
