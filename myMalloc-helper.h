// CSc 422, Spring 2024
// Program 2 header file for sequential myMalloc-helper

// a chunk is a integer (allocSize) along with prev and next pointers
// implicitly, after each of these, there is data of allocSize
typedef struct chunk
{
  int allocSize;
  struct chunk *prev;
  struct chunk *next;
} chunk;

chunk *createList();
chunk *listFront(chunk *list);
void insertAfter(chunk *item, chunk *curItem);
void insertBefore(chunk *item, chunk *curItem);
void listAppend(chunk *list, chunk *item);
void listPush(chunk *list, chunk *item);
void unlinkItem(chunk *item);

void setUpChunks(chunk *list, void *mem, int num, int blockSize);
void moveBetweenLists(chunk *toMove, chunk *fromList, chunk *toList);
chunk *getChunk(chunk *freeList, chunk *allocList);
void returnChunk(chunk *freeList, chunk *allocList, chunk *toFree);