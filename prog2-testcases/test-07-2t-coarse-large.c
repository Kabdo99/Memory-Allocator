#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUMS_1024 132

/**
 * Both threads will (exhaustively) request large chunks
*/
void * test(void * args) {
  char buf[1024];
  void ** large_chunks = malloc(sizeof(void *) * NUMS_1024/2);
  long fail = 0;

  if (!large_chunks) {
    perror("System malloc returned NULL!\n");
    return (void *) -1;
  }

  for (int i = 0; i < NUMS_1024/2; ++i) {
    large_chunks[i] = myMalloc(1024);
    memset(large_chunks[i], i, 1024);
  }

  for (int i = 0; i < NUMS_1024/2; ++i) {
    memset(buf, i, 1024);
    if (memcmp(large_chunks[i], buf, 1024) != 0) {
      fail = 1;
    }
    myFree(large_chunks[i]);
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
    perror("*** CRITICAL: Could not create thread ***\n");
    return -1;
  }
  if (pthread_create(&t1, NULL, test, (void *) 1)) {
    perror("*** CRITICAL: Could not create thread ***\n");
    return -1;
  }

  long verdict = 0;
  pthread_join(t0, (void *) &verdict);
  if (verdict) {
    perror("*** test-07-2t-coarse-large failed: thread 0 exited with a non-zero status ***\n");
    return 1;
  }
  pthread_join(t1, (void *) &verdict);
  if (verdict) {
    perror("*** test-07-2t-coarse-large failed: thread 1 exited with a non-zero status ***\n");
    return 1;
  }

  puts("*** test-07-2t-coarse-large PASSED ***");
  return 0;
}
