#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include "my_malloc.h"

#define METASIZE sizeof(mData)
#define ALIGN(size)  (((size - 1)>>2)<<2)+4
void * basePtr = NULL;

/** This function is to split the data size when the available data size is larger than required data size
 * @param chunk  the pointer point to the splitted data chunk
 * @param size the required malloc size
 */
void split(mData * chunk, size_t size) {
  mData * splited = (mData *)((unsigned long)chunk + METASIZE + size);
  splited->size = chunk->size - size - METASIZE;
  splited->next = chunk->next;
  if (splited->next != NULL) {
    splited->next->prev = splited;
  }
  splited->prev = chunk;
  chunk->next = splited;
  chunk->size = size;
  chunk->isFree = false;
  splited->isFree = true;
}

/** This function is to use sbrk() to generate a new space in heap if there is no available space
 * @param last  the pointer point to the last chunk and based on this chunk, we create a new data chunk
 * @param size the required malloc size
 */
mData * generate (mData * last, size_t size) {
    // printf("request a new space \n");
  mData * new = sbrk(0);
  if (sbrk(size + METASIZE) == (void *) -1) {
    return NULL;
  }
  if(last) {
    last->next = new;
    new->prev = last;
  }
  new->size = size;
  new->next = NULL;
  new->prev = NULL;
  new->isFree = false;
  return new;
}

/** This function is to find the avalible data chunk using the first-fit strategy
 * @param last  the pointer of pointer point to the last chunk we find
 * @param size the required malloc size
 */
mData * ffChunk(mData ** last, size_t size) {
  mData * find ;
  for (find = basePtr; find != NULL; *last = find, find = find -> next) {
        if (find-> isFree && find->size >= size) {
          return find;
        }   
    }
  return find;
}

/** This function is to find the avalible data chunk using the best-fit strategy
 * @param last  the pointer of pointer point to the last chunk we find
 * @param size the required malloc size
 */
mData * bfChunk(mData ** last, size_t size) {
  mData * find;
  mData * bestFit = basePtr;
  size_t difference = LONG_MAX;
  for (find = basePtr; find != NULL; *last = find, find = find -> next) {
        if (find->isFree && find->size >= size ) {
          if (find->size == size) return find;
          if (find->size - size < difference) {
            bestFit = find;
            difference = find->size - size;
          }           
        }
  }
  if (bestFit == basePtr) return NULL;
  return bestFit;
}

/** This function is to allocate the space on heap. 
 * When there is avalible space to hold the required data, we put the chunk in this space.
 * If the available space is larger than required space, we split it.
 * If there is no space to use, we generate a new one using sbrk().
 * @param size the required malloc size
 * @param type the bf strategy (1) or the ff stategy(0)
 */
void * myMalloc(size_t size, int type) {
    if (size <= 0) return NULL;
    //ask for dynamic allocation
    mData * meta;
    mData * last;
    size_t alignSize = ALIGN(size);
    if (basePtr == NULL) {
      meta = generate(NULL, alignSize);
      if (meta == NULL) {
        return NULL;
      }
      basePtr = meta;
    }
    else {
      last = basePtr;
      meta = type ? bfChunk(&last, alignSize) : ffChunk (&last, alignSize);
      if (meta == NULL) {
        meta = generate(last, alignSize);
        if (meta == NULL) {
          return NULL;
        }
      }
      else {
        if (meta->size - alignSize > METASIZE ) {
          split(meta, alignSize);
        }      
        meta->isFree = false; 
      }
    }
    return meta + 1; 
}

/** This function is to merge the free data with its next chunk if its next chunk is free as well. After finishing merging, we return the address of the first chunk
 * @param chunk the target chunk 
 */
mData * merge(mData * chunk) {
  if (chunk->next && chunk->next->isFree) {
    chunk->size = chunk->size + METASIZE + chunk->next->size;
    chunk->next = chunk->next->next;
    if (chunk->next) {
    chunk->next->prev = chunk;
    }
  }
  return chunk;
}

/** This function is to return to the address after freeing the target data
 * @param ptr the target pointer
 */
mData * getChunk(void * ptr) {
  return (mData *) ptr - 1;
}

/** This function is to allocate the space on heap using first-fit strategy
 * @param size the required malloc size
 */
void * ff_malloc(size_t size){
  myMalloc(size, 0);
}

/** This function is to allocate the space on heap using best-fit strategy
 * @param size the required malloc size
 */
void * bf_malloc(size_t size){
  myMalloc(size, 1);
}

/** This function is to free the data on heap using first-fit strategy
 * @param ptr the required address to free
 */
void ff_free(void * ptr){
  if (ptr == NULL) return ;
  mData * freeMeta; 
  freeMeta =  getChunk(ptr);
  freeMeta->isFree = true;
  if(freeMeta->next) {
    freeMeta = merge(freeMeta);
  }
  if (freeMeta->prev && freeMeta->prev->isFree) {
    freeMeta = merge(freeMeta->prev);
  }
}

/** This function is to free the data on heap using best-fit strategy, which is as the same as ff_free function
 * @param ptr the required address to free
 */
void bf_free(void * ptr){
  ff_free(ptr);
}

/** This function is to get the largest size of free data segment*/
unsigned long get_largest_free_data_segment_size(){
  mData * find;
  unsigned long largest = 0;
  for (find = basePtr; find != NULL; find = find -> next) {
        if (find->isFree && find->size > largest ) {
            largest = find->size;
        }
  }
  return largest;
}

/** This function is to get the total size of free data segment*/
unsigned long get_total_free_size(){
  mData * find;
  unsigned long total = 0;
  for (find = basePtr; find != NULL; find = find -> next) {
        if (find->isFree) {
            total += find->size;
        }
  }
  return total;
}
