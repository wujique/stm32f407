/*
 *  tslib/src/ts_read.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: ts_parse_vars.c,v 1.3 2003/03/04 17:09:47 dlowder Exp $
 *
 * Read raw pressure, x, y, and timestamp from a touchscreen device.
 */

#include <string.h>
#include <stdlib.h>
#include "tslib-private.h"
#include "alloc.h"


/*
	(&var->module, variance_vars, NR_VARS, params)
	第一个参数是模块指针，指向每个模块的实体结构体内的ts module
	第二个参数是模块的参数数组，
	第三个是模块的参数个数
	第四个是conf文件配置的参数字符串
*/
int tslib_parse_vars(struct tslib_module_info *mod,
		     const struct tslib_vars *vars, int nr,
		     const char *str)
{
	char *s, *p;
	int ret = 0;

	
	if (!str)
		return 0;

	s = (char *)wjq_malloc(1024);
	memset(s, 0, 1024);
	strncpy(s, str, strlen(str));

	while ((p = (char *)strsep(&s, " \t")) != NULL && ret == 0) 
	{
		const struct tslib_vars *v;
		char *eq;

		eq = strchr(p, '=');
		if (eq)
			*eq++ = '\0';

		for (v = vars; v < vars + nr; v++)
		{
			if (strcmp(v->name, p) == 0) 
			{
				ret = v->fn(mod, eq, v->data);
				break;
			}
		}
	}
	
	wjq_free(s);
	return ret;
}
			 
