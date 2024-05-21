#include <stdio.h>
#include <stdbool.h>

#include "mem_alloc.h"

/*
 * Template for 11malloc. If you want to implement it in C++, rename
 * this file to mem_alloc.cc.
 */

static inline void *nbrk(void *address);

#ifdef NOVA

#define NULL 0

/**********************************/
/* nbrk() implementation for NOVA */
/**********************************/

static inline unsigned syscall2(unsigned w0, unsigned w1)
{
    asm volatile("   mov %%esp, %%ecx    ;"
                 "   mov $1f, %%edx      ;"
                 "   sysenter            ;"
                 "1:                     ;"
                 : "+a"(w0)
                 : "S"(w1)
                 : "ecx", "edx", "memory");
    return w0;
}

static void *nbrk(void *address)
{
    return (void *)syscall2(3, (unsigned)address);
}
#else

#include <unistd.h>

/***********************************/
/* nbrk() implementation for Linux */
/***********************************/

static void *nbrk(void *address)
{
    void *current_brk = sbrk(0);
    if (address != NULL) {
        int ret = brk(address);
        if (ret == -1)
            return NULL;
    }
    return current_brk;
}

#endif

#define MEM_SUCCESS 0
#define MEM_FAILURE -1
#define BLOCK_MIN_SIZE 1 // in bytes

typedef unsigned long lsize_t;

typedef struct header {
    lsize_t size;
    bool is_free;
    struct header *next;
} header_t;

header_t *tail_hd = NULL;

void *get_block(header_t *block_hd) {
    return block_hd + 1;
}

header_t *get_header(void *addr) {
    return addr - sizeof(header_t);
}

lsize_t get_full_size(header_t *hd) {
    return hd->size + sizeof(header_t);
}

void *my_malloc(lsize_t size) {
    header_t *hd = NULL;
    header_t *prev_hd = NULL;
    bool block_found = false;

    for (hd = tail_hd; hd; prev_hd = hd, hd = hd->next) {
        if (hd->is_free && hd->size >= size) {
            block_found = true;
            hd->is_free = false;

            // try to split block
            if (hd->size >= size + sizeof(header_t) + BLOCK_MIN_SIZE) {
                header_t *mid = get_block(hd) + size;

                mid->size = hd->size - (size + sizeof(header_t));
                mid->next = hd->next;
                mid->is_free = true;

                hd->size = size;
                hd->next = mid;
            }

            break;
        }
    }

    if (!block_found) {
        hd = nbrk(NULL);
        if (!hd || !nbrk(get_block(hd) + size)) return NULL;

        hd->size = size;
        hd->next = NULL;
        hd->is_free = false;

        // create linked list, if it doesn't exist, otherwise connect 2 blocks
        if (!tail_hd) tail_hd = hd; else if (prev_hd) prev_hd->next = hd;
    }

    return get_block(hd);
}

int my_free(void *address) {
    int ret = MEM_FAILURE;
    header_t *hd_to_free = get_header(address);
    void *cur_brk = nbrk(NULL);

    if (cur_brk && address < cur_brk && tail_hd && !hd_to_free->is_free) {
        for (header_t *hd = tail_hd; hd != NULL; hd = hd->next) {
            if (hd == hd_to_free) {
                ret = MEM_SUCCESS;
                hd_to_free->is_free = true;
            }

            if (hd->is_free && hd->next && hd->next->is_free) {
                hd->size += get_full_size(hd->next);
                hd->next = hd->next->next;
            }
        }
    }

    return ret;
}
