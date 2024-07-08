/**
 * Starter code author: Dr. David Lowenthal
 * Additinal code author: Abdullah Alkhamis
 * Class: CSc 422 - Parallel & Distributed Programming
 * Description: A thread-safe memory allocator with sequential, coarse-grained,
 * and fine-grained concurrent versions. The allocator manages a pool of memory
 * divided into small and large blocks, with support for per-core lists and an
 * overflow list for the fine-grained version.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "myMalloc-helper.h"

// total amount of memory to allocate, and size of each small chunk
#define SIZE_TOTAL 276672
#define SIZE_SMALL 64
// size of each large chunk
#define SIZE_LARGE 1024
// maximum no. of threads
#define MAX_THREADS 8

// maintain lists of free blocks and allocated blocks
typedef struct memoryManager
{
  chunk *freeSmall[MAX_THREADS];
  chunk *allocSmall[MAX_THREADS];
  // fields for large blocks
  chunk *freeLarge[MAX_THREADS];
  chunk *allocLarge[MAX_THREADS];
  // overflow chunks
  chunk *overflowSmall;
  chunk *overflowLarge;
  chunk *allocOverflowSmall;
  chunk *allocOverflowLarge;
  // locks
  pthread_mutex_t idLock;
  pthread_mutex_t overflowLock;

  char *overflowStart;
  char *overflowEnd;
  // run the correct version
  int flag;
} memManager;

// global variables
memManager *mMan;
pthread_key_t key;
pthread_mutex_t idLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadIdLock = PTHREAD_MUTEX_INITIALIZER;
int seenId[MAX_THREADS] = {0};
bool overflowFlag = false;
int numId = 0;

/**
 * this method initializes the memory manager and sets up the required
 * data structures
 */
int myInit(int numCores, int flag) 
{
  // number of small and large blocks
  int numSmall, numLarge;

  // set up memory manager
  mMan = (memManager *)malloc(sizeof(memManager));
  if (mMan == NULL) {
      return -1;
  }

  // set the flag variable
  mMan->flag = flag;

  // initialize locks
  pthread_mutex_init(&mMan->idLock, NULL);
  pthread_mutex_init(&mMan->overflowLock, NULL);

  // initialize thread specific data key
  pthread_key_create(&key, NULL);

  // intitialize thread id lock to retrieve threads with no race condition
  pthread_mutex_init(&threadIdLock, NULL);

  // calculate number of small and large chunks
  numSmall = (SIZE_TOTAL / 2) / (SIZE_SMALL + sizeof(chunk));
  numLarge = (SIZE_TOTAL / 2) / (SIZE_LARGE + sizeof(chunk));

  // initialize the lists based on the flag
  if (flag == 1 || flag == 0) {
      // sequential and coarse-grain versions
      void *mem = malloc(SIZE_TOTAL);
      if (mem == NULL) {
          free(mMan);
          return -1;
      }

      mMan->freeSmall[0] = createList();
      mMan->allocSmall[0] = createList();
      mMan->freeLarge[0] = createList();
      mMan->allocLarge[0] = createList();

      setUpChunks(mMan->freeSmall[0], mem, numSmall, SIZE_SMALL);
      setUpChunks(mMan->freeLarge[0], (char *)mem + (SIZE_TOTAL / 2), numLarge, SIZE_LARGE);
  } else if (flag == 2) {
      // fine-grain version
      for (int i = 0; i < numCores; i++) {
          // allocate SIZE_TOTAL bytes for each core
          void *coreMem = malloc(SIZE_TOTAL);
          if (coreMem == NULL) {
              // handle memory allocation failure
              for (int j = 0; j < i; j++) {
                  free((char *)mMan->freeSmall[j] - sizeof(chunk));
                  free((char *)mMan->freeLarge[j] - sizeof(chunk));
              }
              free(mMan);
              return -1;
          }

          mMan->freeSmall[i] = createList();
          mMan->allocSmall[i] = createList();
          mMan->freeLarge[i] = createList();
          mMan->allocLarge[i] = createList();

          // set up memory chunks for this core
          setUpChunks(mMan->freeSmall[i], coreMem, numSmall, SIZE_SMALL);
          setUpChunks(mMan->freeLarge[i], (char *)coreMem + (SIZE_TOTAL / 2), numLarge, SIZE_LARGE);
      }

      mMan->overflowSmall = createList();
      mMan->overflowLarge = createList();
      mMan->allocOverflowSmall = createList();
      mMan->allocOverflowLarge = createList();

      // calculate overflow memory ranges
      mMan->overflowStart = (char *)malloc(2 * SIZE_TOTAL / (numCores + 1));
      if (mMan->overflowStart == NULL) {
          // handle memory allocation failure for overflow
          for (int i = 0; i < numCores; i++) {
              free((char *)mMan->freeSmall[i] - sizeof(chunk));
              free((char *)mMan->freeLarge[i] - sizeof(chunk));
          }
          free(mMan);
          return -1;
      }
      mMan->overflowEnd = mMan->overflowStart + 2 * SIZE_TOTAL / (numCores + 1);

      // calculate number of small and large chunks for the overflow lists
      numSmall = (2 * SIZE_TOTAL / (numCores + 1)) / (SIZE_SMALL + sizeof(chunk));
      numLarge = (2 * SIZE_TOTAL / (numCores + 1)) / (SIZE_LARGE + sizeof(chunk));

      // set up chunks for overflow lists
      setUpChunks(mMan->overflowSmall, mMan->overflowStart, numSmall, SIZE_SMALL);
      setUpChunks(mMan->overflowLarge, mMan->overflowStart, numLarge, SIZE_LARGE);
  }

  return 0;
}

/**
 * this method assigns a unique id to each thread
 */
int getThreadId()
{
  // get thread specific data
  int *id = pthread_getspecific(key);
  if (id == NULL)
  {
    // aquire thread lock
    pthread_mutex_lock(&threadIdLock);
    // allocate memore for the thread id
    id = malloc(sizeof(int));
    // assign next thread id
    *id = numId++;
    // store thread data
    pthread_setspecific(key, id);
    // release thread lock
    pthread_mutex_unlock(&threadIdLock);
  }
  return *id;
}

/**
 * this methods creates files when allocating memory from a new list
 */
void createFileForNewList(int id)
{
  // track the seen ids to avoid duplicates
  if (!seenId[id])
  {
    char str[16];
    sprintf(str, "touch Id-%d\n", id + 1); // construct the filename
    system(str);
    seenId[id] = 1; // mark thread as seen
  }
}

/**
 * this method determines if the given chunk belongs to the overflow list
 */
int isFromOverflowList(chunk *chunk)
{
  // check if the chunk pointer is within the overflow list range
  return ((char *)chunk >= (char *)mMan->overflowStart && (char *)chunk < (char *)mMan->overflowEnd);
}

/**
* this method allocates blocks of memory with the given size
*/

void *myMalloc(int size)
{
  // create chunk pointer
  chunk *toAlloc;
  int id = getThreadId();
 
  createFileForNewList(id);
 
  if (size <= SIZE_SMALL)
  {
    if (mMan->flag == 1)
    {
      // coarse-grain version
      pthread_mutex_lock(&mMan->idLock);
      toAlloc = getChunk(mMan->freeSmall[0], mMan->allocSmall[0]);
      pthread_mutex_unlock(&mMan->idLock);
    }
    else if (mMan->flag == 2)
    {
      // fine-grain version
      if (mMan->freeSmall[id]->next == mMan->freeSmall[id])
      {
        pthread_mutex_lock(&mMan->overflowLock);
        toAlloc = getChunk(mMan->overflowSmall, mMan->allocOverflowSmall);
        pthread_mutex_unlock(&mMan->overflowLock);
      }
      else
      {
        // Thread-specific list is not empty, allocate from thread-specific list
        toAlloc = getChunk(mMan->freeSmall[id], mMan->allocSmall[id]);
      }
    }
    else
    {
      // sequential version
      toAlloc = getChunk(mMan->freeSmall[0], mMan->allocSmall[0]);
    }
  }
  else if (size <= SIZE_LARGE)
  {
    if (mMan->flag == 1)
    {
      // coarse-grain version
      pthread_mutex_lock(&mMan->idLock);
      toAlloc = getChunk(mMan->freeLarge[0], mMan->allocLarge[0]);
      pthread_mutex_unlock(&mMan->idLock);
    }
    else if (mMan->flag == 2)
    {
      // fine-grain version
      if (mMan->freeSmall[id]->next == mMan->freeSmall[id])
      {
        pthread_mutex_lock(&mMan->overflowLock);
        toAlloc = getChunk(mMan->overflowLarge, mMan->allocOverflowLarge);
        pthread_mutex_unlock(&mMan->overflowLock);
      }
      else
      {
          // Thread-specific list is not empty, allocate from thread-specific list
          toAlloc = getChunk(mMan->freeSmall[id], mMan->allocSmall[id]);
      }
    }
    else
    {
      // sequential version
      toAlloc = getChunk(mMan->freeSmall[0], mMan->allocSmall[0]);
    }
  }
  return ((void *)((char *)toAlloc) + sizeof(chunk));
}

/**
 * this method frees allocated memory blocks
 */
void myFree(void *ptr)
{
  int id;
  // find the front of the chunk
  chunk *toFree = (chunk *)ptr - 1;
  if (toFree->allocSize == SIZE_SMALL)
  {
    if (mMan->flag == 1)
    {
      // coarse-grain version
      pthread_mutex_lock(&mMan->idLock);
      returnChunk(mMan->freeSmall[0], mMan->allocSmall[0], toFree);
      pthread_mutex_unlock(&mMan->idLock);
    }
    else if (mMan->flag == 2)
    {
      // fine-grain version
      id = getThreadId();
      // check if it belongs to the over flow list and free it
      if (isFromOverflowList(toFree))
      {
        pthread_mutex_lock(&mMan->overflowLock);
        returnChunk(mMan->overflowSmall, mMan->allocOverflowSmall, toFree);
        pthread_mutex_unlock(&mMan->overflowLock);
      }
      else
      {
        returnChunk(mMan->freeSmall[id], mMan->allocSmall[id], toFree);
      }
    }
    else
    {
      // sequential version
      returnChunk(mMan->freeSmall[0], mMan->allocSmall[0], toFree);
    }
  }
  else if (toFree->allocSize == SIZE_LARGE)
  {
    if (mMan->flag == 1)
    {
      // coarse-grain version
      pthread_mutex_lock(&mMan->idLock);
      returnChunk(mMan->freeLarge[0], mMan->allocLarge[0], toFree);
      pthread_mutex_unlock(&mMan->idLock);
    }
    else if (mMan->flag == 2)
    {
      // fine-grain version
      id = getThreadId();
      // check if it belongs to the over flow list and free it
      if (isFromOverflowList(toFree))
      {
        pthread_mutex_lock(&mMan->overflowLock);
        returnChunk(mMan->overflowLarge, mMan->allocOverflowLarge, toFree);
        pthread_mutex_unlock(&mMan->overflowLock);
      }
      else
      {
        returnChunk(mMan->freeLarge[id], mMan->allocLarge[id], toFree);
      }
    }
    else
    {
      // sequential version
      returnChunk(mMan->freeLarge[0], mMan->allocLarge[0], toFree);
    }
  }
}
