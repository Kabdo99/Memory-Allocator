#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUMS_1024 132

/**
 * Exhaustively allocate all large chunks and release them, to make sure that
 * the memory is both correctly allocated and freed
*/
int test() {
  int fail = 0;
  int * alloc_arrays[NUMS_1024];

  for (int i = 0; i < NUMS_1024; ++i) {
    // Allocate a large chunk
    alloc_arrays[i] = (int *) myMalloc(1024);
    if (!alloc_arrays[i]) {
      perror("*** myMalloc returned NULL ***\n");
      return 1;
    }
    
    // Write to the entire page 
    memset(alloc_arrays[i], i & 0xff, 1024);
  }

  char ref[1024];

  for (int i = 0; i < NUMS_1024; ++i) {
    memset(ref, i & 0xff, 1024);
    if (memcmp(alloc_arrays[i], ref, 1024)) {
        fprintf(stderr, "Data at chunk %d was overwritten!\n", i);
    }
  }

  for (int i = 0; i < NUMS_1024; ++i) {
    myFree(alloc_arrays[i]);
  }

  return fail;
}

int main(int argc, char *argv[]) {
  if (myInit(1, 1) == -1) {
    perror("*** test-03-1t-large-realloc: myInit() failed. ***\n");
    return 1;
  }

  for (int i = 0; i < 4; ++i) {
    if (test()) {
      perror("*** test-03-1t-large-realloc failed ***\n");
      return 1;
    }
  }

  printf("*** test-03-1t-large-realloc PASSED ***\n");
  return 0;
}
