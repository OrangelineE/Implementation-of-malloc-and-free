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

void * myMalloc(size_t size, int type);
void myFree(void * ptr);

//First Fit malloc
void * ff_malloc(size_t size);
//Best Fit malloc
void * bf_malloc(size_t size);

//First Fit free
void ff_free(void * ptr);
//Best Fit free
void bf_free(void * ptr);

unsigned long get_largest_free_data_segment_size();//in bytes
unsigned long get_total_free_size();//in bytes