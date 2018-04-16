/*
 *  tslib/src/ts_open.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: ts_open.c,v 1.4 2004/07/21 19:12:59 dlowder Exp $
 *
 * Open a touchscreen device.
 */
#include <stdlib.h>
#include <string.h>
#include "tslib-private.h"

struct tsdev Stm32TslibDev;//原来是通过malloc申请，移植的时候修改为直接定义一个设备。
/**
 *@brief:      ts_open
 *@details:    打开一个TS设备
 *@param[in]   const char *name  
               int nonblock      
 *@param[out]  无
 *@retval:     struct

 在linux环境下，其实就是申请一个tsdev变量
 */
struct tsdev *ts_open(const char *name, int nonblock)
{
	struct tsdev *ts;

	ts = &Stm32TslibDev;

	memset(ts, 0, sizeof(struct tsdev));
	
	ts->fd = 0;//打开设备返回的句柄
	
	return ts;

}


