/*
 *  tslib/src/ts_load_module.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: ts_load_module.c,v 1.4 2004/10/19 22:01:27 dlowder Exp $
 *
 * Close a touchscreen device.
 */
#include <stdlib.h>
#include <string.h>

#include "tslib-private.h"

extern int ts_stm32_gei_init(char *name);

/**
 *@brief:      __ts_load_module
 *@details:    根据名字和参数，将一个模块添加到ts设备
 *@param[in]   struct tsdev *ts    
               const char *module  
               const char *params  
               int raw             
 *@param[out]  无
 *@retval:     
 */
int __ts_load_module(struct tsdev *ts, const char *module, const char *params, int raw)
{
	struct tslib_module_info * (*init)(struct tsdev *, const char *);
	struct tslib_module_info *info;

	void *handle;
	int ret;
	/*
		打开module
		查找模块的初始化函数指针
	*/
	init = (struct tslib_module_info * (*)(struct tsdev *, const char *))ts_stm32_gei_init((char *)module);
	if (!init) 
	{
		return -1;
	}
	/*
		执行mod_init函数，返回值给info
		返回的其实是对应的module结构体第一个成员的指针
		例如，dejitter模块 struct tslib_dejitter 第一个 module
	*/
	info = init(ts, params);
	if (!info) 
	{
		wjq_log(LOG_DEBUG, "__ts_load_module init params err\r\n");
		return -1;
	}
	/* 将模块的句柄赋值给模块里面定义的info结构体handle句柄*/
	info->handle = handle;

	/*attach，其实才是将module添加到tsdev链表里面*/
	if (raw) 
	{
		ret = __ts_attach_raw(ts, info);
	} 
	else 
	{
		ret = __ts_attach(ts, info);
	}

	/*添加失败，释放module*/
	if (ret) 
	{
		info->ops->fini(info);

	}

	return ret;
}

int ts_load_module(struct tsdev *ts, const char *module, const char *params)
{
	return __ts_load_module(ts, module, params, 0);
}

int ts_load_module_raw(struct tsdev *ts, const char *module, const char *params)
{
	return __ts_load_module(ts, module, params, 1);
}
