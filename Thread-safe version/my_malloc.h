#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
typedef struct metaData{
    bool isFree;// whether the block is avaliable
    size_t size;//the size that a user asks
    struct metaData * prev;
    struct metaData * next;
} mData;
unsigned long get_largest_free_data_segment_size();//in bytes
unsigned long get_total_free_size();//in bytes
//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

