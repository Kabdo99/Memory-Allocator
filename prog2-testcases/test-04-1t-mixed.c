#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.h"
#include "myMalloc-helper.h"

#define NUMS_64   1572
#define NUMS_1024 132

int ** small_chunks;
void ** large_chunks;

/**
 * Exhaustively allocate both small and large chunks. Some calls to myMalloc
 * request more memory than available in small chunks, but less than what large
 * chunks can provide.
*/
int test() {
  int fail = 0;
  char buf[1024];

  // Allocate
  small_chunks = malloc(sizeof(int *)  * NUMS_64);
  large_chunks = malloc(sizeof(void *) * NUMS_1024);

  if (small_chunks == NULL || large_chunks == NULL) {
    perror("Could not initialize test-04-1t-mixed\n");
    return -1;
  }

  for (int i = 0; i < NUMS_64; ++i) {
    small_chunks[i] = myMalloc(sizeof(int));
    if (!small_chunks[i]) {
      perror("*** myMalloc returned NULL ***\n");
      return 1;
    }
    *small_chunks[i] = i;
  }

  for (int i = 0; i < NUMS_1024/2; ++i) {
    large_chunks[i] = myMalloc(128);
    if (!large_chunks[i]) {
      perror("*** myMalloc returned NULL ***\n");
      return 1;
    }
    memset(large_chunks[i], i, 128);
  }

  for (int i = NUMS_1024/2; i < NUMS_1024; ++i) {
    large_chunks[i] = myMalloc(1024);
    if (!large_chunks[i]) {
      perror("*** myMalloc returned NULL ***\n");
      return 1;
    }
    memset(large_chunks[i], i, 1024);
  }

  // Check
  for (int i = 0; i < NUMS_64; ++i) {
    if (*(small_chunks[i]) != i) {
      fprintf(stderr, "ERROR: Small chunk at index %d was overwritten!\n", i);
    }
    myFree(small_chunks[i]);
  }
  
  for (int i = 0; i < NUMS_1024/2; ++i) {
    memset(buf, i, 128);
    if (memcmp(large_chunks[i], buf, 128) != 0) {
      fprintf(stderr, "ERROR: Large chunk at index %d was overwritten!\n", i);
    }
    myFree(large_chunks[i]);
  }

  for (int i = NUMS_1024/2; i < NUMS_1024; ++i) {
    memset(buf, i, 1024);
    if (memcmp(large_chunks[i], buf, 1024) != 0) {
      fprintf(stderr, "ERROR: Large chunk at index %d was overwritten!\n", i);
    }
    myFree(large_chunks[i]);
  }

  free(small_chunks);
  free(large_chunks);

  return fail;
}

int main(int argc, char *argv[]) {
  if (myInit(1, 1) == -1) {
    perror("*** test-04-1t-mixed: myInit() failed. ***\n");
    return 1;
  }

  if (test()) {
    perror("*** test-04-1t-mixed failed ***\n");
    return 1;
  }
  
  printf("*** test-04-1t-mixed PASSED ***\n");
  return 0;
}
