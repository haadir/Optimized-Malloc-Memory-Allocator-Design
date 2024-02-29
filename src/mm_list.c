#include <mm_list.h>  // prototypes of functions implemented in this file
#include <unistd.h>   // NULL

// TEMP
#include <stdio.h>

BlockHeader *mm_list_headp;
BlockHeader *mm_list_tailp;

/**
 * Initializes to an empty list.
 */
void mm_list_init() {
    mm_list_headp = NULL;
    mm_list_tailp = NULL;
}

/**
 * In addition to the block header with size/allocated bit, a free block has
 * pointers to the headers of the previous and next blocks on the free list.
 *
 * Pointers use 4 bytes because this project is compiled with -m32.
 * Check Figure 9.48(b) in the textbook.
 */
typedef struct {
    BlockHeader header;
    BlockHeader *prev_free;
    BlockHeader *next_free;
} FreeBlockHeader;

/**
 * Find the header address of the previous **free** block on the **free list**.
 *
 * @param bp address of a block header (it must be a free block)
 * @return address of the header of the previous free block on the list
 */
BlockHeader *mm_list_prev(BlockHeader *bp) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    return fp->prev_free;
}

/**
 * Find the header address of the next **free** block on the **free list**.
 *
 * @param bp address of a block header (it must be a free block)
 * @return address of the header of the next free block on the list
 */
BlockHeader *mm_list_next(BlockHeader *bp) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    return fp->next_free;
}

/**
 * Set the pointer to the previous **free** block.
 *
 * @param bp address of a free block header
 * @param prev address of the header of the previous free block (to be set)
 */
static void mm_list_prev_set(BlockHeader *bp, BlockHeader *prev) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    fp->prev_free = prev;
}


/**
 * Set the pointer to the next **free** block.
 *
 * @param bp address of a free block header
 * @param next address of the header of the next free block (to be set)
 */
static void mm_list_next_set(BlockHeader *bp, BlockHeader *next) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    fp->next_free = next;
}

/**
 * Add a block at the beginning of the free list.
 *
 * @param bp address of the header of the block to add
 */
void mm_list_prepend(BlockHeader *bp) {
    // TODO: implement
    FreeBlockHeader *free_bp = (FreeBlockHeader *)bp;

    // empty list
    if (mm_list_headp == NULL) {
        mm_list_headp = bp;
        mm_list_tailp = bp;
        free_bp->prev_free = NULL;
        free_bp->next_free = NULL;
    }
    else {
        FreeBlockHeader *head = (FreeBlockHeader *)mm_list_headp;
        head->prev_free = bp;
        free_bp->next_free = mm_list_headp;
        free_bp->prev_free = NULL;
        mm_list_headp = bp;
    }
}

/**
 * Add a block at the end of the free list.
 *
 * @param bp address of the header of the block to add
 */
void mm_list_append(BlockHeader *bp) {
    // TODO: implement

    // Cast it so we can access prev and next
    FreeBlockHeader *free_bp = (FreeBlockHeader *)bp;

    // for empty list case
    if (mm_list_headp == NULL) {
        mm_list_headp = bp;
        mm_list_tailp = bp;
        free_bp->prev_free = NULL;
        free_bp->next_free = NULL;
    } else {
        FreeBlockHeader *tail = (FreeBlockHeader *)mm_list_tailp;
        tail->next_free = bp;
        free_bp->prev_free = mm_list_tailp;
        free_bp->next_free = NULL;
        mm_list_tailp = bp;
    }

}

/**
 * Remove a block from the free list.
 *
 * @param bp address of the header of the block to remove
 */
void mm_list_remove(BlockHeader *bp) {
    // TODO: implement

    FreeBlockHeader *free_bp = (FreeBlockHeader *)bp;
    FreeBlockHeader *prev = (FreeBlockHeader *)free_bp->prev_free;
    FreeBlockHeader *next = (FreeBlockHeader *)free_bp->next_free;

    //printf("Removing block: %p, prev: %p, next: %p\n", bp, prev, next);

    if (bp == mm_list_headp) {
        //printf("HERE11\n");
        mm_list_headp = (BlockHeader *)next;
    }

    if (bp == mm_list_tailp) {
        //printf("HERE22\n");
        mm_list_tailp = (BlockHeader *)prev;
    }

    if (prev != NULL) {
        //printf("HERE33\n");
        prev->next_free = next;
    }

    if (next != NULL) {
        //printf("HERE44\n");
        next->prev_free = prev;
    }

}
