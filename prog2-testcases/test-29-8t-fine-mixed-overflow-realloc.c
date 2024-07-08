// sample driver for CSc 422, Program 2
// uses only one thread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

#include "myMalloc.h"

#define NUM_THREADS 8
#define SMALL_COUNT (1572 + 1572 / NUM_THREADS)
#define LARGE_COUNT (132 + 132 / NUM_THREADS)

#define SMALL_SIZE 64
#define LARGE_SIZE 1024

int pass = 0;

/**
 * Using 8 threads, allocate all local small and large blocks, then release,
 * then repeat (16 trials in total for small blocks; 256 for large blocks).
 * Threads will need to eventually reach the overflow list.
*/
void alloc_test(int blocksize, int blockcount) {
  int i, j, k;
  int *array[blockcount];

  for (j = 0; j < blocksize / sizeof(int); j++) {
    for(i = 0; i < blockcount; i++) {
      array[i] = (int *) myMalloc (sizeof(int) * j);
      if(array[i]==NULL){
        printf("myMalloc() failed\n");
        return;
      }
      for (k = 0; k < j; k++) {
        array[i][k] = i + k;
      }
    }

    for (i = 0; i < blockcount; i++) {
      for (k = 0; k < j; k++) {
        if (array[i][k] != i + k) {
          printf("Invalid value (expected %d but got %d at chunk [%d][%d] in thread %ld run %d)\n", i+k, array[i][k], i, k, syscall(SYS_gettid), j);
          return;
        }
      }
      myFree(array[i]);
    }
  }
  __sync_fetch_and_add(&pass, 1);
}

void* test_thread(void* _) {
  alloc_test(SMALL_SIZE, SMALL_COUNT);
  alloc_test(LARGE_SIZE, LARGE_COUNT);
}

int main(int argc, char *argv[]){
  
  if (myInit(NUM_THREADS, 2) == -1){
    printf("myInit() failed.\n");
    return 1;
  }

  pthread_t thread_ids[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&thread_ids[i], NULL, test_thread, NULL);
  } 
  
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(thread_ids[i], NULL);
  } 
 
  if (pass != NUM_THREADS*2) {
    printf("*** test-29-8t-fine-mixed failed: (only %d threads passed)\n", pass);
    return 1;
  }

    if (system("ls Overflow")) {
    perror("*** test-29-8t-fine-mixed failed: file Overflow not found ***");
    return 1;
  }

  char * base = "ls Id-%d";
  char cmd[64];
  for (int i = 1; i <= NUM_THREADS; ++i) {
    snprintf(cmd, 64, base, i);
    if (system(cmd)) {
      fprintf(stderr, "*** test-29-8t-fine-mixed failed: file Id-%d not found ***\n", i);
      return 1;
    }
  }

base = "ls Id-%d >/dev/null 2>&1";
  for (int i = NUM_THREADS+1; i <= 9; ++i) {
    snprintf(cmd, 64, base, i);
    if (!system(cmd)) { // These should fail!
      fprintf(stderr, "*** test-29-8t-fine-mixed failed: file Id-%d FOUND when it should NOT be! ***\n", i);
      return 1;
    }
  }

  printf("*** test-29-8t-fine-mixed PASSED ***\n");
  return 0;
}

