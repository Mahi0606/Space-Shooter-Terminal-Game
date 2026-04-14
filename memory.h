/* ============================================================
 *  memory.h — Custom Memory Allocator (free-list on a big block)
 * ============================================================ */
#ifndef MEMORY_H
#define MEMORY_H

/* ---- configuration ---- */
#define VIRTUAL_RAM_SIZE  (1024 * 1024)   /* 1 MiB virtual RAM */

/* ---- public API ---- */
void   mem_init(void);
void   mem_shutdown(void);

void  *mem_alloc(int size);
void   mem_free(void *ptr);

int    mem_used(void);
int    mem_free_bytes(void);
int    mem_total(void);
void   mem_print_diagnostics(void);

#endif /* MEMORY_H */
