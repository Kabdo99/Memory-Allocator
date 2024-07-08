#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUM_THREADS 2

#define NUMS_64   1572
#define NUMS_1024 (132 + 132 / NUM_THREADS)

/**
 * Forked from testcase 15; threads will need to reach the global overflow list
*/
void * test(void * args) {
  char buf[1024];
  void ** large_chunks = malloc(sizeof(void *) * NUMS_1024);
  long fail = 0;

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
      fail = 1;
    }
    myFree(large_chunks[i]);
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
    perror("*** test-21-2t-fine-large-overflow failed: thread 0 exited with a non-zero status ***\n");
    return 1;
  }
  pthread_join(t1, (void *) &verdict);
  if (verdict) {
    perror("*** test-21-2t-fine-large-overflow failed: thread 1 exited with a non-zero status ***\n");
    return 1;
  }

  if (system("ls Overflow")) {
    perror("*** test-21-2t-fine-large-overflow failed: file Overflow not found ***");
    return 1;
  }

  char * base = "ls Id-%d";
  char cmd[64];
  for (int i = 1; i <= NUM_THREADS; ++i) {
    snprintf(cmd, 64, base, i);
    if (system(cmd)) {
      fprintf(stderr, "*** test-21-2t-fine-large-overflow  failed: file Id-%d not found ***\n", i);
      return 1;
    }
  }

base = "ls Id-%d >/dev/null 2>&1";
  for (int i = NUM_THREADS+1; i <= 9; ++i) {
    snprintf(cmd, 64, base, i);
    if (!system(cmd)) { // These should fail!
      fprintf(stderr, "*** test-21-2t-fine-large-overflow failed: file Id-%d FOUND when it should NOT be! ***\n", i);
      return 1;
    }
  }

  puts("*** test-21-2t-fine-large-overflow PASSED ***");
  return 0;
}
