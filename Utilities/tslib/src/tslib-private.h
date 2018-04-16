#ifndef _TSLIB_PRIVATE_H_
#define _TSLIB_PRIVATE_H_
/*
 *  tslib/src/tslib-private.h
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.
 *
 * $Id: tslib-private.h,v 1.3 2004/07/21 19:12:59 dlowder Exp $
 *
 * Internal touch screen library definitions.
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "tslib.h"
#include "tslib-filter.h"

/*
	ts设备，list是tslib模块链表，这个链表决定了read的执行顺序
	在打开设备后，会初始化这个链表，
	通常，非RAW状态下，这个列表会是
	->linear->degitter->variance->pthres->read_raw
	通过分析各个模块的read函数就可以知道，
	调用ts_read的时候，会调用链表下一个module的read，也就是linera_read，
	同理，一直往下，直到rad_raw，最后是__ts_read_raw，
	最后的__ts_read_raw函数，就是从input设备获取触摸屏原始样点的地方。

	因此，在移植的时候，因为没有LINUX系统，所以要人工处理好这个链表，
	同时将从ADC获取的样点，传递给__ts_read_raw.
*/
struct tsdev 
{
	int fd;
	struct tslib_module_info *list;
	struct tslib_module_info *list_raw; /* points to position in 'list' where raw reads
					       come from.  default is the position of the
					       ts_read_raw module. */
};

int __ts_attach(struct tsdev *ts, struct tslib_module_info *info);
int __ts_attach_raw(struct tsdev *ts, struct tslib_module_info *info);
int ts_load_module(struct tsdev *dev, const char *module, const char *params);
int ts_load_module_raw(struct tsdev *dev, const char *module, const char *params);
//int ts_error(const char *fmt, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _TSLIB_PRIVATE_H_ */

