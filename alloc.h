#ifndef _CUSTOM_ALLOC_
#define _CUSTOM_ALLOC_
#include <sys/types.h>

extern void *t_malloc(size_t size);
extern void *t_realloc(void *p, size_t size);
extern void t_free(void *p);
extern void *calloc(size_t number, size_t size);

#endif