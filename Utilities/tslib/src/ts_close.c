/*
 *  tslib/src/ts_close.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: ts_close.c,v 1.1.1.1 2001/12/22 21:12:06 rmk Exp $
 *
 * Close a touchscreen device.
 */

#include <stdlib.h>
#include "tslib-private.h"
/**
 *@brief:      ts_close
 *@details:    关闭一个ts设备
 *@param[in]   struct tsdev *ts  
 *@param[out]  无
 *@retval:     
 在linux中其实就是释放打开的时候申请的变量
 */
int ts_close(struct tsdev *ts)
{
	return 0;
}


