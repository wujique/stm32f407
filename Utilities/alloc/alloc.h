#ifndef __ALLOC_H_
#define __ALLOC_H_


extern void wjq_free_m( void *ap );
extern void* wjq_malloc_m( unsigned nbytes );
extern void *wjq_realloc(void *mem_address, unsigned int newsize);

#define wjq_free(p) {wjq_free_m(p); p = 0;}
//#define wjq_malloc(n) wjq_malloc_m(n, __FUNCTION__, __LINE__);
#define wjq_malloc(n) wjq_malloc_m(n);

#endif

