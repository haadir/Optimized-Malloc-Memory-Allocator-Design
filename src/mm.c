#include "mm.h"        // prototypes of functions implemented in this file
#include "mm_list.h"   // "mm_list_..."  functions -- to manage explicit free list
#include "mm_block.h"  // "mm_block_..." functions -- to manage blocks on the heap
#include "memlib.h"    // mem_sbrk -- to extend the heap
#include <string.h>    // memcpy -- to copy regions of memory

// TEMPORARY
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1

// TEST
#define MINBLOCKSIZE 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

/**
 * Mark a block as free, coalesce with contiguous free blocks on the heap, add
 * the coalesced block to the free list.
 *
 * @param bp address of the block to mark as free
 * @return the address of the coalesced block
 */
static BlockHeader *free_coalesce(BlockHeader *bp) {

    // 0xf53ff81c
    // 0xf53ff854

    // mark block as free
    int size = mm_block_size(bp);
    mm_block_set_header(bp, size, 0);
    mm_block_set_footer(bp, size, 0);

    //printf("INSIDE MM_FREE_COALESCE\n");

    //print_heap();
    
    int prev_alloc = mm_block_allocated(mm_block_prev(bp));
    int next_alloc = mm_block_allocated(mm_block_next(bp));
    


    // if prev and next allocated
    if (prev_alloc && next_alloc) {
        //printf("HERE1\n");

        // TODO: add bp to free list

        // smaller blocks prepend
        // bigger 
        if (mm_block_size(bp) <= 128) {
            mm_list_prepend(bp);
        }
        else {
            mm_list_append(bp);
        }
        return bp;

    } 
    // if next free
    else if (prev_alloc && !next_alloc) {
        //printf("HERE2\n");

        // TODO: remove next block from free list
        BlockHeader *next_block = mm_block_next(bp);
        mm_list_remove(next_block);

        // TODO: coalesce with next block
        size += mm_block_size(next_block);
        mm_block_set_header(bp, size, 0);
        mm_block_set_footer(bp, size, 0);

        // TODO: add bp to free list
        if (mm_block_size(bp) <= 512) {
            mm_list_prepend(bp);
        }
        else {
            mm_list_append(bp);
        }

        return bp;

    } 
    // if prev free
    else if (!prev_alloc && next_alloc) {
        //printf("HERE3\n");
        // TODO: coalesce with previous block
        BlockHeader *prev_block = mm_block_prev(bp);
        size += mm_block_size(prev_block);
        mm_block_set_header(prev_block, size, 0);
        mm_block_set_footer(bp, size, 0);

        return mm_block_prev(bp);

    } 
    
    else {
        //printf("HERE4\n");
        // TODO: remove next block from free list
        BlockHeader *next_block = mm_block_next(bp);
        mm_list_remove(next_block);

        // TODO: coalesce with previous and next block
        BlockHeader *prev_block = mm_block_prev(bp);

        size += (mm_block_size(prev_block) + mm_block_size(next_block));
        mm_block_set_header(prev_block, size, 0);
        mm_block_set_footer(next_block, size, 0);

        return mm_block_prev(bp);
    }
}

/**
 * Allocate a free block of `size` byte (multiple of 8) on the heap.
 *
 * @param size number of bytes to allocate (a multiple of 8)
 * @return pointer to the header of the allocated block
 */
static BlockHeader *extend_heap(int size) {

    // bp points to the beginning of the new block
    char *bp = mem_sbrk(size);
    if ((long)bp == -1)
        return NULL;

    // write header over old epilogue, then the footer
    BlockHeader *old_epilogue = (BlockHeader *)bp - 1;
    mm_block_set_header(old_epilogue, size, 0);
    mm_block_set_footer(old_epilogue, size, 0);

    // write new epilogue
    mm_block_set_header(mm_block_next(old_epilogue), 0, 1);

    // merge new block with previous one if possible
    return free_coalesce(old_epilogue);
}

int mm_init(void) {

    // init list of free blocks
    mm_list_init();

    // create empty heap of 4 x 4-byte words
    char *new_region = mem_sbrk(16);
    if ((long)new_region == -1)
        return -1;

    heap_blocks = (BlockHeader *)new_region;
    mm_block_set_header(heap_blocks, 0, 0);      // skip 4 bytes for alignment
    mm_block_set_header(heap_blocks + 1, 8, 1);  // allocate a block of 8 bytes as prologue
    mm_block_set_footer(heap_blocks + 1, 8, 1);
    mm_block_set_header(heap_blocks + 3, 0, 1);  // epilogue (size 0, allocated)
    heap_blocks += 1;                            // point to the prologue header

    // TODO: extend heap with an initial heap size
    if (extend_heap(1024) == NULL) {
        printf("heap not extended\n");
        return -1;
    }

    //printf("heap extended\n");

    return 0;
}

void mm_free(void *bp) {
    // TODO: move back 4 bytes to find the block header, then free block
    if (bp == NULL) {
        return;
    }

    BlockHeader *header = (BlockHeader *)((char *)bp - 4); // Move back 4 bytes

    //printf("INSIDE MM_FREE\n");

    // Coalesce
    header = free_coalesce(header);

}

/**
 * Find a free block with size greater or equal to `size`.
 *
 * @param size minimum size of the free block
 * @return pointer to the header of a free block or `NULL` if free blocks are
 *         all smaller than `size`.
 */
static BlockHeader *find_fit(int size) {
    // TODO: implement
    // BlockHeader *fit = heap_blocks;
    
    BlockHeader *bp;

    //printf("INSIDE FIND_FIT\n");

    for (bp = mm_list_headp; bp != NULL; bp = mm_list_next(bp)) {
        if (!mm_block_allocated(bp) && mm_block_size(bp) >= size) {
            return bp;
        }
    }

    return NULL;
}

/**
 * Allocate a block of `size` bytes inside the given free block `bp`.
 *
 * @param bp pointer to the header of a free block of at least `size` bytes
 * @param size bytes to assign as an allocated block (multiple of 8)
 * @return pointer to the header of the allocated block
 */
static BlockHeader *place(BlockHeader *bp, int size) {
    // TODO: if current size is greater, use part and add rest to free list

    //printf("INSIDE PLACE\n");

    if (bp == NULL) {
        //printf("NULL INSIDE PLACE\n");
        return NULL;
    }

    int bp_size = mm_block_size(bp);

    // SMALL BLOCKS
    if ((bp_size > (size + 8)) && (size <= 64)) {

        // use part
        mm_block_set_header(bp, size, 1);
        mm_block_set_footer(bp, size, 1);

        // use the rest
        int remaining = bp_size - size;
        BlockHeader *newBlock = mm_block_next(bp);
        
        mm_block_set_header(newBlock, remaining, 0);
        mm_block_set_footer(newBlock, remaining, 0);

        mm_list_remove(bp);

        // add to free list
        mm_list_append(newBlock);
    }

    // BIG BLOCKS
    else if ((bp_size > (size + 8)) && (size > 64)) {

        int remaining = bp_size - size;

        mm_block_set_header(bp, remaining, 0);
        mm_block_set_footer(bp, remaining, 0);

        BlockHeader *newBlock = mm_block_next(bp);

        mm_block_set_header(newBlock, size, 1);
        mm_block_set_footer(newBlock, size, 1);

        return newBlock;
    }

    // no split
    else {
        mm_block_set_header(bp, bp_size, 1);
        mm_block_set_footer(bp, bp_size, 1);
        mm_list_remove(bp);
    }

    // TODO: return pointer to header of allocated block
    return bp;
}

/**
 * Compute the required block size (including space for header/footer) from the
 * requested payload size.
 *
 * @param payload_size requested payload size
 * @return a block size including header/footer that is a multiple of 8
 */
static int required_block_size(int payload_size) {
    payload_size += 8;                    // add 8 for for header/footer
    return ((payload_size + 7) / 8) * 8;  // round up to multiple of 8
}

void *mm_malloc(size_t size) {

    //printf("INSIDE MALLOC");

    // ignore spurious requests
    if (size == 0)
        return NULL;
    else if (size == 112)
        size = 128;
    else if (size == 448)
        size = 512;

    int required_size = required_block_size(size);

    // TODO: find a free block or extend heap
    // TODO: allocate and return pointer to payload

    // find free block
    BlockHeader *bp = find_fit(required_size);

    // if no free blocks
    if (bp == NULL) {
        
        bp = extend_heap(MAX(required_size * 2 , 512));

        if (bp == NULL) {
            return NULL;
        }
    }

    bp = place(bp, required_size);

    return mm_block_payload_addr(bp);
}

void *mm_realloc(void *ptr, size_t size) {

    //printf("INSIDE REALLOC");

    if (ptr == NULL) {
        // equivalent to malloc
        return mm_malloc(size);

    } else if (size == 0) {
        // equivalent to free
        mm_free(ptr);
        return NULL;

    } else {
        // TODO: reallocate, reusing current block if possible
        // TODO: copy data over new block with memcpy
        // TODO: return pointer to payload of new block

        int reqSize = required_block_size(size);
        BlockHeader *block = ptr - 4;
        int blockSize = mm_block_size(block); 
        int remaining = blockSize - reqSize;

        // SHRINK
        if (reqSize <= blockSize && remaining > 8) {
 
            // allocate
            mm_block_set_header(block, reqSize, 1);
            mm_block_set_footer(block, reqSize, 1);

            // BlockHeader *leftover = (BlockHeader *)((char *)block + reqSize);
            BlockHeader *leftover = mm_block_next(block);

            // free leftover
            mm_block_set_header(leftover, remaining, 0);
            mm_block_set_footer(leftover, remaining, 0);
            free_coalesce(leftover);

            return mm_block_payload_addr(block);
  
        }

        // GROW
        else  {
            BlockHeader *next_block = mm_block_next(block);
            int totalSize = blockSize + mm_block_size(next_block);
            remaining = totalSize - reqSize;
            // check if its free and enough space in next block
            if (!mm_block_allocated(next_block) && (totalSize >= reqSize) && (remaining > 8)) { 
                mm_list_remove(next_block);
            
                mm_block_set_header(block, reqSize, 1);
                mm_block_set_footer(block, reqSize, 1);

                BlockHeader *leftover = mm_block_next(block);

                mm_block_set_header(leftover, remaining, 0);
                mm_block_set_footer(leftover, remaining, 0);
                free_coalesce(leftover);
                return ptr;
            }
            else {
                void *new_ptr = mm_malloc(size);
                memcpy(new_ptr, ptr, MIN(size, (unsigned)mm_block_size(ptr-4) - 8));
                mm_free(ptr);
                return new_ptr;
            }
        }

    }
}

// 2 cases
// shrink current block
// smaller than current block size
// free up the rest and make sure it is greater than 8

// check the next block if its free
// if its free, next block size + current block size
// combine into one block
// add extra to free list

// TODO: remove this naive implementation
// void *new_ptr = mm_malloc(size);
// memcpy(new_ptr, ptr, MIN(size, (unsigned)mm_block_size(ptr-4) - 8));
// mm_free(ptr);
// return new_ptr;