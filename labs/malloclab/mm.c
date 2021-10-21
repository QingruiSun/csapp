/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};


#define SIZE1 (1 << 4)
#define SIZE2 (1 << 5)
#define SIZE3 (1 << 6)
#define SIZE4 (1 << 7)
#define SIZE5 (1 << 8)
#define SIZE6 (1 << 9)
#define SIZE7 (1 << 10)
#define SIZE8 (1 << 11)
#define SIZE9 (1 << 12)
#define SIZE10 (1 << 13)
#define SIZE11 (1 << 14)
#define SIZE12 (1 << 15)

#define LIST_NUM 13

#define CHUNKSIZE (1 << 12)


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8

#define PACK(size, alloc) ((size) | (alloc))

// read or write a word at address p
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define PACK(size, alloc) ((size) | (alloc))

#define MAX(x, y) ((x) > (y)? (x) : (y))
#define GET_SIZE(p) (GET(p) & (~0x7))
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(p) ((char *)(p) - WSIZE)
#define FTRP(p) ((char *)(p) + GET_SIZE(HDRP(p)) - DSIZE)


#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define GET_PTR(bp) ((char *)(long)(GET(bp)))
#define PUT_PTR(bp, val) (*(unsigned int *)(bp) = ((long)(val)))

static char *heap_listp = NULL;

void *coalesce(char *bp);
char *find_fit(size_t size);
static void *extend_heap(size_t words);
void place(char *bp, size_t asize);
void delete_list(char *p);
void insert_list(char *bp);
int get_list_index(size_t size);


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  int i = 0;
  void * bp = NULL;
  if ((heap_listp = mem_sbrk(16 * WSIZE)) == (void *)-1) {
    return -1;
  }
  PUT(heap_listp + LIST_NUM * WSIZE, PACK(DSIZE, 1)); //prologue header
  PUT(heap_listp + (LIST_NUM + 1) * WSIZE, PACK(DSIZE, 1)); //prologue footer
  PUT(heap_listp + (LIST_NUM + 2) * WSIZE, PACK(0, 1)); // epilogue
  for (i = 0; i < LIST_NUM; ++i) {
    PUT_PTR(heap_listp + i * WSIZE, NULL);
  }
  if ((bp = extend_heap(CHUNKSIZE / WSIZE)) == NULL) {
    return -1;
  }
  return 0; 
}


static void *extend_heap(size_t words)
{
  char *bp;
  size_t size;
  size = (((words % 2) == 0) ? (words * WSIZE) : ((words + 1)) * WSIZE);
  if ((long)(bp = mem_sbrk(size)) == -1) {
    return NULL;
  }
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));
  return coalesce(bp);
}


void insert_list(char * bp)
{
  int index;
  size_t size = GET_SIZE(HDRP(bp));
  index = get_list_index(size);
  char *start = heap_listp + WSIZE * index;
  if (GET_PTR(start) == NULL) {
    PUT_PTR(start, bp);
    PUT_PTR(bp + WSIZE, NULL); //set next block.
    PUT_PTR(bp, start); // set prev block.
  } else {
    char *next_bp = GET_PTR(start);
    PUT_PTR(next_bp, bp);
    PUT_PTR(bp + WSIZE, next_bp);
    PUT_PTR(start, bp);
    PUT_PTR(bp, start);
  } 
}


int get_list_index(size_t size)
{
  if (size <= SIZE1) {
    return 0;
  } else if (size <= SIZE2) {
    return 1;
  } else if (size <= SIZE3) {
    return 2;
  } else if (size <= SIZE4) {
    return 3;
  } else if (size <= SIZE5) {
    return 4;
  } else if (size <= SIZE6) {
    return 5;
  } else if (size <= SIZE7) {
    return 6;
  } else if (size <= SIZE8) {
    return 7;
  } else if (size <= SIZE9) {
    return 8;
  } else if (size <= SIZE10) {
    return 9;
  } else if (size <= SIZE11) {
    return 10;
  } else if (size <= SIZE12) {
    return 11;
  } else {
    return 12;
  }
}

void *coalesce(char *bp)
{
  int prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  int next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

  if (prev_alloc && next_alloc) {
    bp = bp;
  } else if (next_alloc != 0) {
    char *prev_block = PREV_BLKP(bp);
    delete_list(prev_block);
    size_t new_size = GET_SIZE(HDRP(prev_block)) + GET_SIZE(HDRP(bp));
    PUT(HDRP(prev_block), PACK(new_size, 0));
    PUT(FTRP(bp), PACK(new_size, 0));
    bp = prev_block;
  } else if (prev_alloc != 0) {
    char *next_block = NEXT_BLKP(bp);
    delete_list(next_block);
    size_t new_size = GET_SIZE(HDRP(next_block)) + GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(new_size, 0));
    PUT(FTRP(bp), PACK(new_size, 0));
    bp = bp;
  } else {
    char *prev_block = PREV_BLKP(bp);
    char *next_block = NEXT_BLKP(bp);
    delete_list(prev_block);
    delete_list(next_block);
    size_t new_size = GET_SIZE(HDRP(prev_block)) + GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(next_block));
    PUT(HDRP(prev_block), PACK(new_size, 0));
    PUT(FTRP(next_block), PACK(new_size, 0));
    bp = prev_block;
  }
  insert_list(bp);
  return bp;
}



/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  size_t asize;
  size_t extendsize;
  char *bp;
    
  if (size <= 0) {
    return NULL;
  }

  if (size <= DSIZE) {
    asize = 2 * DSIZE;
  } else {
    asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
  }

  if ((bp = find_fit(asize)) != NULL) {
    place(bp, asize);
    return bp;
  }
  
  extendsize = MAX(asize, CHUNKSIZE);
  if ((bp = extend_heap(extendsize / WSIZE)) == NULL) {
    return NULL;
  }
  place(bp, asize);
  return bp;

}


char *find_fit(size_t size)
{
  int index = get_list_index(size);
  char *p = NULL;

  while (index < LIST_NUM) {
    p = GET_PTR(heap_listp + index * WSIZE);
    if (p == NULL) {
      index++;
      continue;
    }
    while (p != NULL) {
      if (GET_SIZE(HDRP(p)) >= size) {
	return (char *)p;
      } else {
	p = GET_PTR(p + WSIZE);
      }
    }
    index++;
  }
  return NULL;
}

void place(char *bp, size_t asize)
{
  size_t csize = GET_SIZE(HDRP(bp));
  delete_list(bp);
  if ((csize - asize) >= 2 * DSIZE) {
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    char *next_block = NEXT_BLKP(bp);
    PUT(HDRP(next_block), PACK(csize - asize, 0));
    PUT(FTRP(next_block), PACK(csize - asize, 0));
    insert_list(next_block);
  } else {
    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
  }
}

void delete_list(char *p)
{
  size_t size = GET_SIZE(HDRP(p));
  int index = get_list_index(size);
  char *start = heap_listp + index * WSIZE;
  if (((char *)GET_PTR(p) == start) && (GET_PTR(p + WSIZE) == NULL)) {
    PUT_PTR(start, NULL);
  } else if (GET_PTR(p + WSIZE) == NULL) {
    char *prev_block = (char *)GET_PTR(p);
    PUT_PTR(prev_block + WSIZE, NULL);
  } else if ((char *)GET_PTR(p) == start) {
    char *next_block = GET_PTR(p + WSIZE);
    PUT_PTR(next_block, start);
    PUT_PTR(start, next_block);
  } else {
    char *prev_block = GET_PTR(p);
    char *next_block = GET_PTR(p + WSIZE);
    PUT_PTR(next_block, prev_block);
    PUT_PTR(prev_block + 4, next_block);
  }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  size_t size = GET_SIZE(HDRP(ptr));
  PUT(HDRP(ptr), PACK(size, 0));
  PUT(FTRP(ptr), PACK(size, 0));
  coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  char *new_ptr;
  int asize;
  if (ptr == NULL) {
    return mm_malloc(size);    
  }
  if (size == 0) {
    mm_free(ptr);
    return NULL;
  }
  if (size <= DSIZE) {
    asize = 2 * DSIZE;
  } else {
    asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
  }
  if (asize == GET_SIZE(HDRP(ptr))) {
    return ptr;
  }
  if (asize < GET_SIZE(HDRP(ptr))) {
    new_ptr = mm_malloc(size);
    if (new_ptr == NULL) {
      return NULL;
    }
    memmove(new_ptr, ptr, size);
    mm_free(ptr);
    return new_ptr;   
  }
  new_ptr = mm_malloc(size);
  if (new_ptr == NULL) {
    return NULL;
  }
  memmove(new_ptr, ptr, size);
  mm_free(ptr);
  return new_ptr;
}














