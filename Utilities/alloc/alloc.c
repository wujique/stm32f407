

/*
 * File:		alloc.c
 * Purpose: 	generic malloc() and free() engine
 *
 * Notes:		99% of this code stolen/borrowed from the K&R C
 *				examples.
 *
 */
#include "stdlib.h"

#include "stm32f4xx.h"
#include "wujique_log.h"
#include "alloc.h"

/*
使用编译器定义的堆作为内存池，注意，直接使用malloc，会调用C库的函数，使用的就是堆
	，
要防止冲突
*/
//#define ALLOC_USE_HEAP	//用堆做内存池
#define ALLOC_USE_ARRAY //用定义的数组做内存池

/* 
内存分配方法选择，
_ALLOC_BEST_FIT_ 最适应法
_ALLOC_FIRST_FIT_ 首次适应法
*/
#define _ALLOC_BEST_FIT_	
//#define _ALLOC_FIRST_FIT_

#ifdef ALLOC_USE_HEAP
#pragma section = "HEAP"
#endif

#ifdef ALLOC_USE_ARRAY
#define AllocArraySize (76*1024)

__align(4) //保证内存池四字节对齐

char AllocArray[AllocArraySize];

#endif

/********************************************************************/

/*
 * This struct forms the minimum block size which is allocated, and
 * also forms the linked list for the memory space used with alloc()
 * and free().	It is padded so that on a 32-bit machine, all malloc'ed
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
		struct ALLOC_HDR*ptr;
		unsigned int size;									/*本快内存容量*/
	} s;


unsigned int align;
unsigned int pad;
} ALLOC_HDR;


static ALLOC_HDR base; /*空闲内存链表头结点*/
static ALLOC_HDR*freep = NULL;


u32 AllocCnt = 0;

/*-------------------------------------------------------------------*/
void wjq_free_t(void*ap)
{
	ALLOC_HDR*bp, *p;

	/* 最好的判断是不是应该判断在堆范围内？*/
	if(ap==NULL)
		return;

	/* 函数传入的ap是可使用内存的指针，往前退一个结构体位置，
		也就是下面的bp，才是记录内存信息的位置*/
	bp = (ALLOC_HDR*)ap-1;											/* point to block header */

	AllocCnt -= bp->s.size;

	/*
	  找到需要释放的内存的前后空闲块
	  其实就是比较内存块位置的地址大小
	*/
	for(p = freep; ! ((bp>p)&&(bp<p->s.ptr)); p = p->s.ptr)
	{
		if((p>=p->s.ptr)&&((bp>p)||(bp<p->s.ptr)))
		{
			/*
				当一个块，
				p>=p->s.ptr 本身起始地址指针大于下一块地址指针
				bp>p 要释放的块，地址大于P
				bp<p->s.ptr 要释放的块，地址小于下一块
			*/
			break;		/* freed block at start or end of arena */
		}
	}

	/*判断是否能跟一个块合并，能合并就合并，不能合并就用链表连起来*/
	if((bp+bp->s.size)==p->s.ptr)
	{
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	}
	else
	{
		bp->s.ptr = p->s.ptr;
	}

	/*同样处理跟后一块的关系*/
	if((p+p->s.size)==bp)
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


/*---------------------------------------------------------*/
void*wjq_malloc_t(unsigned nbytes)
{
	/* Get addresses for the HEAP start and end */
#ifdef ALLOC_USE_HEAP
	char*__HEAP_START = __section_begin("HEAP");
	char*__HEAP_END = __section_end("HEAP");
#endif

#ifdef ALLOC_USE_ARRAY
	char*__HEAP_START = AllocArray;
	char*__HEAP_END = __HEAP_START+AllocArraySize;
#endif

	ALLOC_HDR*p, *prevp;
	unsigned nunits;

#ifdef _ALLOC_BEST_FIT_
	ALLOC_HDR *bp = NULL;
    ALLOC_HDR *bprevp;
#endif

	/*计算要申请的内存块数*/
	nunits = ((nbytes+sizeof(ALLOC_HDR)-1) / sizeof(ALLOC_HDR))+1;

	AllocCnt += nunits;
	//wjq_log(LOG_DEBUG, "AllocCnt:%d\r\n", AllocCnt*sizeof(ALLOC_HDR));

	/*第一次使用malloc，内存链表没有建立
	  初始化链表*/
	if((prevp = freep)==NULL)
	{
		p = (ALLOC_HDR*)
		__HEAP_START;
		p->s.size = (((uint32_t) __HEAP_END- (uint32_t) __HEAP_START) / sizeof(ALLOC_HDR));
		p->s.ptr =&base;
		base.s.ptr = p;
		base.s.size = 0;
		prevp = freep =&base;

		/*经过初始化后，只有一块空闲块*/
	}

	/*查询链表，查找合适块*/
	for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr)
	{

		if(p->s.size==nunits)
		{
			prevp->s.ptr = p->s.ptr;
			freep = prevp;

			/*返回可用内存指针给用户，
			可用内存要出去内存块管理结构体*/
			return (void*) (p+1);
		}
		else if(p->s.size > nunits)
		{
			#ifdef _ALLOC_BEST_FIT_/*最适合法*/
			if(bp == NULL)
            {
                bp = p;
                bprevp = prevp;
            }
			
            if(bp->s.size > p->s.size)
            {
                bprevp = prevp;
                bp = p;                
            }
			#else/*首次适应法*/
			p->s.size -= nunits;
			p += p->s.size;
			p->s.size = nunits;

			freep = prevp;
			/*返回可用内存指针给用户，
			可用内存要出去内存块管理结构体*/
			return (void*) (p+1);
			#endif
		}

		/*分配失败*/
		if(p==freep)
		{
			#ifdef _ALLOC_BEST_FIT_
			if(bp != NULL)
			{
                freep = bprevp;
                p = bp;
                
                p->s.size -= nunits;
                p += p->s.size;     //P 指向将分配出去的空间
                p->s.size = nunits; //记录分配的大小，这里不用设置ptr了，因为被分配出去了

                return (void *)(p + 1); //减去头结构体才是真正分配的内存    
            }
			#endif
			
			while(1)
			{
				/*对于嵌入式来说，没有机制整理内存，因此，不允许内存分配失败*/
				wjq_log(LOG_ERR, "wujique malloc err!!\r\n");
			}
			return NULL;
		}

	}
}

/*
	二次封装，如果需要做互斥，在_m后缀的函数内实现。
*/
void*wjq_malloc_m(unsigned nbytes)
{
	void*p;
	//wjq_log(LOG_DEBUG, "malloc:%d\r\n", nbytes);
	
	p = wjq_malloc_t(nbytes);

	return p;
}


void wjq_free_m(void*ap)
{
	if(ap==NULL)
		return;
	
	wjq_free_t(ap);
}


void*wjq_calloc(size_t n, size_t size)
{
	void *p;

	//wjq_log(LOG_DEBUG, "wjq_calloc\r\n");

	p = wjq_malloc_t(n*size);

	if(p!=NULL)
	{
		memset((char*) p, 0, n*size);
	}
	return p;
}

void *wjq_realloc(void *ap, unsigned int newsize)
{
	ALLOC_HDR*bp, *p, *np;
	
	unsigned nunits;
	unsigned aunits;

	
	//wjq_log(LOG_DEBUG, "wjq_realloc: %d\r\n", newsize);

	if(ap == NULL)
	{
		bp = wjq_malloc_t(newsize);
		return bp;	
	}

	if(newsize == 0)
	{
		wjq_free(ap);
		return NULL;
	}
	/*计算要申请的内存块数*/
	nunits = ((newsize + sizeof(ALLOC_HDR)-1) / sizeof(ALLOC_HDR))+1;

	/* 函数传入的ap是可使用内存的指针，往前退一个结构体位置，
		也就是下面的bp，才是记录内存信息的位置*/
	bp = (ALLOC_HDR*)ap-1;											/* point to block header */
	if(nunits <= bp->s.size)
	{
		/*
		新的申请数不比原来的大，暂时不处理
		浪费点内存。
		*/
		return ap;
	}
	
	#if 1
	/*无论如何都直接申请内存然后拷贝数据*/
	bp = wjq_malloc_t(newsize);
	memcpy(bp, ap, newsize);
	wjq_free(ap);
	
	return bp;
	#else
	/*
	  找到需要释放的内存的前后空闲块
	  其实就是比较内存块位置的地址大小
	*/
	for(p = freep; ! ((bp>p)&&(bp<p->s.ptr)); p = p->s.ptr)
	{
		if((p>=p->s.ptr)&&((bp>p)||(bp<p->s.ptr)))
		{
			/*
				当一个块，
				p>=p->s.ptr 本身起始地址指针大于下一块地址指针
				bp>p 要释放的块，地址大于P
				bp<p->s.ptr 要释放的块，地址小于下一块
			*/
			break;		/* freed block at start or end of arena */
		}
	}

	/**/
	if((bp + bp->s.size) == p->s.ptr)
	{
		/*增加的内存块*/
		aunits = (nunits - bp->s.size);
		if( aunits == p->s.ptr->s.size)
		{	
			/*刚刚好相等*/
			p->s.ptr = p->s.ptr->s.ptr;
			bp->s.size = nunits;
			return ap;
		}
		else if(aunits < p->s.ptr->s.size)
		{
			np = p->s.ptr + aunits;//切割aunits分出去，np就是剩下块的地址
			np->s.ptr = p->s.ptr->s.ptr;
			np->s.size = p->s.ptr->s.size - aunits;
				
			p->s.ptr = np;

			bp->s.size = nunits;
			return ap;
		}
		
	}
	
	/*需要重新申请内存*/
	bp = wjq_malloc_t(newsize);
	memcpy(bp, ap, newsize);
	wjq_free(ap);
	
	return bp;
	#endif
	
}

void wjq_malloc_test(void)
{
	char*p;

	p = (char*)
	wjq_malloc(1024);

	/*打印指针，看看是不是4字节对齐*/
	wjq_log(LOG_FUN, "pointer :%08x\r\n", p);

	memset(p, 0xf0, 1024);
	wjq_log(LOG_FUN, "data:%02x\r\n", * (p+1023));
	wjq_log(LOG_FUN, "data:%02x\r\n", * (p+1024));

	wjq_free(p);
	wjq_log(LOG_FUN, "alloc free ok\r\n");

	while(1)
		;
}


/***************************** end ***************************************/

