// sample driver for CSc 422, Program 2
// uses only one thread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

#include "myMalloc.h"

#define NUMS_1024 132

int pass = 0;

/**
 * Use one thread to allocate all large chunks, then release, in 8 iterations
 * in total.
*/
void test() {
  int i, j, k;
  int *array[NUMS_1024];

  for (j = 0; j < 8; j++) {
    for(i = 0; i < NUMS_1024; i++) {
      array[i] = (int *) myMalloc (sizeof(int) * j);
      if(array[i]==NULL){
        printf("myMalloc() failed\n");
        return;
      }
      for (k = 0; k < j; k++) {
        array[i][k] = i + k;
      }
    }

    for (i = 0; i < NUMS_1024;i++) {
      for (k = 0; k < j; k++) {
        if (array[i][k] != i + k) {
          printf("Invalid value (expected %d but got %d at chunk [%d][%d] in thread %ld run %d)\n", i+k, array[i][k], i, k, syscall(SYS_gettid), j);
          return;
        }
      }
      myFree(array[i]);
    }
  }

  pass = 1;
  return;
}

int main(int argc, char *argv[]){

  if (myInit(1, 1) == -1){
    printf("myInit() failed.\n");
    return 1;
  }

  test();
  
  if (pass != 1) {
    printf("test failed\n");
    return 1;
  }

  char * base = "ls Id-%d";
  char cmd[64] = "ls Overflow";
  if (system(cmd)) {
    printf("*** test-25-1t-coarse-large: Overflow file not found ***\n");
    return 1;
  }

  base = "ls Id-%d >/dev/null 2>&1";
  for (int i = 0; i <= 9; ++i) {
    snprintf(cmd, 64, base, i);
    if (!system(cmd)) { // These should fail!
      fprintf(stderr, "*** test-25-1t-coarse-large failed: file Id-%d FOUND when it should NOT be! ***\n", i);
      return 1;
    }
  }

  printf("*** test-25-1t-coarse-large PASSED ***\n");
  return 0;
}
