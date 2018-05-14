/*
 * File:        alloc.c
 * Purpose:     generic malloc() and free() engine
 *
 * Notes:       99% of this code stolen/borrowed from the K&R C
 *              examples.
 *
 */
#include <stdarg.h>
#include <stdio.h>
#include "stdlib.h"

#include "stm32f4xx.h"
#include "wujique_log.h"
#include "alloc.h"

/*
使用编译器定义的堆作为内存池，注意，直接使用malloc，会调用C库的函数，使用的就是堆，
要防止冲突
*/
//#define ALLOC_USE_HEAP	//用堆做内存池
#define ALLOC_USE_ARRAY		//用定义的数组做内存池

#ifdef ALLOC_USE_HEAP
#pragma section = "HEAP"
#endif

#ifdef ALLOC_USE_ARRAY
#define AllocArraySize (70*1024)

__align(4)//保证内存池四字节对齐
char AllocArray[AllocArraySize];
#endif

/********************************************************************/

/*
 * This struct forms the minimum block size which is allocated, and
 * also forms the linked list for the memory space used with alloc()
 * and free().  It is padded so that on a 32-bit machine, all malloc'ed
 * pointers are 16-byte aligned.
 */
 /*
	本内存分配方法最基本的结构就是下面这个结构体。
	在每一块空闲内存的头部都有一个，
	这个结构体size记录了本块内存的大小,
	ptr则连接到下一块空闲内存。
	分配内存时，从一块空闲内存切割需要的内存出去。

*/
typedef struct ALLOC_HDR
{
  struct
  {
    struct ALLOC_HDR *ptr;
    unsigned int size;/*本快内存容量*/
  } s;
  unsigned int align;
  unsigned int pad;
} ALLOC_HDR;

static ALLOC_HDR base;/*空闲内存链表头结点*/
static ALLOC_HDR *freep = NULL;

/********************************************************************/
void wjq_free_t( void *ap )
{
  ALLOC_HDR *bp, *p;

	/* 最好的判断是不是应该判断在堆范围内？*/
	if(ap == NULL)
		return;

	/* 函数传入的ap是可使用内存的指针，往前退一个结构体位置，
		也就是下面的bp，才是记录内存信息的位置*/
  bp = (ALLOC_HDR *) ap - 1; /* point to block header */

  /*
  	找到需要释放的内存的前后空闲块
  	其实就是比较内存块位置的大小
  */
  for ( p = freep; !( ( bp > p ) && ( bp < p->s.ptr ) ); p = p->s.ptr )
  {
    if ( ( p >= p->s.ptr ) && ( ( bp > p ) || ( bp < p->s.ptr ) ) )
    {
      break; /* freed block at start or end of arena */
    }
  }

	/*判断是否能跟一个块合并，能合并就合并，不能合并就用链表连起来*/
  if ( ( bp + bp->s.size ) == p->s.ptr )
  {
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  }
  else
  {
    bp->s.ptr = p->s.ptr;
  }
	/*同样处理跟后一块的关系*/
  if ( ( p + p->s.size ) == bp )
  {
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  }
  else
  {
    p->s.ptr = bp;
  }

  freep = p;

}

/********************************************************************/
void* wjq_malloc_t( unsigned nbytes )
{
  /* Get addresses for the HEAP start and end */
#ifdef ALLOC_USE_HEAP
  char* __HEAP_START = __section_begin("HEAP");
  char* __HEAP_END = __section_end("HEAP");
#endif

#ifdef ALLOC_USE_ARRAY
  char* __HEAP_START = AllocArray;
  char* __HEAP_END = __HEAP_START+AllocArraySize;
#endif

  ALLOC_HDR *p, *prevp;
  unsigned nunits;

	/*计算要申请的内存块数*/
  nunits = ( ( nbytes + sizeof(ALLOC_HDR) - 1 ) / sizeof(ALLOC_HDR) ) + 1;

/*第一次使用malloc，内存链表没有建立
  初始化链表*/
  if ( ( prevp = freep ) == NULL )
  {
    p = (ALLOC_HDR *) __HEAP_START;
    p->s.size = ( ( (uint32_t) __HEAP_END - (uint32_t) __HEAP_START )
      / sizeof(ALLOC_HDR) );
    p->s.ptr = &base;
    base.s.ptr = p;
    base.s.size = 0;
    prevp = freep = &base;
	/*经过初始化后，只有一块空闲块*/
  }

	/*查询链表，查找合适块*/
  for ( p = prevp->s.ptr;; prevp = p, p = p->s.ptr )
  {
    if ( p->s.size >= nunits )
    {
      if ( p->s.size == nunits )
      {
        prevp->s.ptr = p->s.ptr;
      }
      else
      {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
	  /*返回可用内存指针给用户，
	  可用内存要出去内存块管理结构体*/
      return (void *) ( p + 1 );
    }

	/*分配失败*/
    if ( p == freep )
    {
			#if 1
    	while(1)
    	{
    		/*对于嵌入式来说，没有机制整理内存，因此，不允许内存分配失败*/
    		wjq_log(LOG_ERR, "wujique malloc err!!\r\n");
    	}
			#else
      return NULL;
			#endif
    }
	
  }
}


void *wjq_malloc_m(unsigned nbytes)
{   
    void * p;
    
    p = wjq_malloc_t(nbytes);

    return p;
}

void wjq_free_m (void *ap)
{

    if(ap == NULL)
        return;
    wjq_free_t(ap);
}

/*
	本函数未测试
*/
void *wjq_calloc(size_t n, size_t size)
{
	void *p;
	p = wjq_malloc_m(n*size);
	if(p != NULL)
	{
		memset((char*)p, 0, n*size);
	}

	return p;
}


void wjq_malloc_test(void)
{
	char* p;
	
	p = (char *)wjq_malloc(1024);
	/*打印指针，看看是不是4字节对齐*/
	wjq_log(LOG_FUN, "pointer :%08x\r\n", p);
	
	memset(p, 0xf0, 1024);
	wjq_log(LOG_FUN, "data:%02x\r\n", *(p+1023));
	wjq_log(LOG_FUN, "data:%02x\r\n", *(p+1024));
	
	wjq_free(p);
	wjq_log(LOG_FUN, "alloc free ok\r\n");
	
	while(1);
}


/***************************** end ***************************************/

