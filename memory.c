/* ============================================================
 *  memory.c — Custom Memory Allocator
 *  -----------------------------------------------------------
 *  Uses a single stdlib malloc() to grab a 1 MiB block at init.
 *  All game allocations go through mem_alloc / mem_free which
 *  manage that block with a first-fit free-list allocator.
 *  Adjacent free blocks are coalesced on every free().
 * ============================================================ */
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>  /* only for the one-time malloc / free of the block */

/* ---- block header ---- */
typedef struct BlockHeader {
    int                  size;    /* usable payload bytes (excl. header) */
    int                  used;    /* 1 = allocated, 0 = free             */
    struct BlockHeader  *next;    /* next block in the linear list       */
} BlockHeader;

#define HEADER_SIZE  ((int)sizeof(BlockHeader))

/* ---- module state ---- */
static unsigned char *g_pool      = NULL;   /* raw byte pool              */
static BlockHeader   *g_first     = NULL;   /* first block in the list    */
static int            g_allocated = 0;      /* total bytes handed out     */

/* ---- helpers ---- */
static unsigned char *block_payload(BlockHeader *b)
{
    return (unsigned char *)b + HEADER_SIZE;
}

static BlockHeader *payload_to_header(void *ptr)
{
    return (BlockHeader *)((unsigned char *)ptr - HEADER_SIZE);
}

/* Coalesce current block with its neighbour if also free */
static void coalesce(BlockHeader *b)
{
    while (b->next && !b->next->used) {
        b->size += HEADER_SIZE + b->next->size;
        b->next  = b->next->next;
    }
}

/* ---- public API ---- */

void mem_init(void)
{
    g_pool = (unsigned char *)malloc(VIRTUAL_RAM_SIZE);
    if (!g_pool) {
        fprintf(stderr, "[mem] FATAL: could not allocate virtual RAM\n");
        exit(1);
    }

    /* The whole pool is one big free block */
    g_first       = (BlockHeader *)g_pool;
    g_first->size = VIRTUAL_RAM_SIZE - HEADER_SIZE;
    g_first->used = 0;
    g_first->next = NULL;
    g_allocated   = 0;
}

void mem_shutdown(void)
{
    if (g_allocated > 0) {
        fprintf(stderr, "[mem] WARNING: %d bytes still allocated at shutdown\n",
                g_allocated);
    }
    free(g_pool);
    g_pool    = NULL;
    g_first   = NULL;
    g_allocated = 0;
}

void *mem_alloc(int size)
{
    if (size <= 0) return NULL;

    /* Align to 8 bytes */
    int aligned = (size + 7) & ~7;

    BlockHeader *b = g_first;
    while (b) {
        if (!b->used && b->size >= aligned) {
            /* Split if there is enough room for another block */
            int leftover = b->size - aligned - HEADER_SIZE;
            if (leftover >= 8) {
                BlockHeader *nb = (BlockHeader *)(block_payload(b) + aligned);
                nb->size = leftover;
                nb->used = 0;
                nb->next = b->next;
                b->next  = nb;
                b->size  = aligned;
            }
            b->used      = 1;
            g_allocated += b->size;

            /* Zero-fill payload */
            {
                unsigned char *p = block_payload(b);
                int i;
                for (i = 0; i < b->size; i++) p[i] = 0;
            }
            return block_payload(b);
        }
        b = b->next;
    }

    fprintf(stderr, "[mem] ERROR: out of memory (requested %d)\n", size);
    return NULL;
}

void mem_free(void *ptr)
{
    if (!ptr) return;                       /* null-safe                  */

    BlockHeader *b = payload_to_header(ptr);

    if (!b->used) {                         /* double-free guard         */
        fprintf(stderr, "[mem] WARNING: double free detected\n");
        return;
    }

    b->used      = 0;
    g_allocated -= b->size;

    /* Forward coalesce */
    coalesce(b);

    /* Backward coalesce: walk from start */
    BlockHeader *prev = g_first;
    while (prev && prev != b) {
        if (!prev->used) coalesce(prev);
        prev = prev->next;
    }
}

int mem_used(void)       { return g_allocated; }
int mem_free_bytes(void) { return VIRTUAL_RAM_SIZE - HEADER_SIZE - g_allocated; }
int mem_total(void)      { return VIRTUAL_RAM_SIZE; }

void mem_print_diagnostics(void)
{
    fprintf(stderr, "[mem] used=%d  free=%d  total=%d\n",
            g_allocated, mem_free_bytes(), VIRTUAL_RAM_SIZE);
}
