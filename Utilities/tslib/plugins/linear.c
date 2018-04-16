/*
 *  tslib/plugins/linear.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: linear.c,v 1.10 2005/02/26 01:47:23 kergoth Exp $
 *
 * Linearly scale touchscreen values
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tslib.h"
#include "tslib-filter.h"


extern calibration cal;
/*
	通过分析你会发现，每一个module的结构体，
	首先会包含一个ts标准module，
	然后是本模块的参数
*/
struct tslib_linear 
{
	struct tslib_module_info module;

	int	swap_xy;

// Linear scaling and offset parameters for pressure
	int	p_offset;
	int	p_mult;
	int	p_div;

// Linear scaling and offset parameters for x,y (can include rotation)
	int	a[7];
};
/*

	计算参数个数，定义数组时有要求
	
*/
#define NR_VARS (sizeof(linear_vars) / sizeof(linear_vars[0]))

struct tslib_linear Stm32TsLinear;

/*

	传入的samp只是个样点缓冲，里面不需要数据，
	数据通过info里面的read函数从下一个模块获取

	nr只是希望处理的数据，实际处理多少，还有看能读到多少
*/
/**
 *@brief:      linear_read
 *@details:    校准
 *@param[in]   struct tslib_module_info *info  
               struct ts_sample *samp          
               int nr                          
 *@param[out]  无
 *@retval:     static
 */
static int linear_read(struct tslib_module_info *info, struct ts_sample *samp, int nr)
{
	struct tslib_linear *lin = (struct tslib_linear *)info;
	int ret;
	int xtemp,ytemp;

	// 从下一层读取样点
	ret = info->next->ops->read(info->next, samp, nr);
	//读到ret个样点
	if (ret >= 0) 
	{
		int nr;//重新申请一个nr变量?有必要也叫nr吗？让人误解

		for (nr = 0; nr < ret; nr++, samp++) 
		{
			xtemp = samp->x; ytemp = samp->y;

			samp->x = 	( lin->a[2] +
					lin->a[0]*xtemp + 
					lin->a[1]*ytemp ) / lin->a[6];
			
			samp->y =	( lin->a[5] +
					lin->a[3]*xtemp +
					lin->a[4]*ytemp ) / lin->a[6];
			
			samp->pressure = ((samp->pressure + lin->p_offset)
						 * lin->p_mult) / lin->p_div;

			/*XY轴对调*/
			if (lin->swap_xy) 
			{
				int tmp = samp->x;
				samp->x = samp->y;
				samp->y = tmp;
			}
		}
	}

	return ret;
}

static int linear_fini(struct tslib_module_info *info)
{
	return 0;
}

static const struct tslib_ops linear_ops =
{
	.read	= linear_read,
	.fini	= linear_fini,
};

/*

	linear没有参数可以设置，就是用1

*/
static int linear_xyswap(struct tslib_module_info *inf, char *str, void *data)
{
	struct tslib_linear *lin = (struct tslib_linear *)inf;

	lin->swap_xy = 1;
	return 0;
}

/*

	linear 模块的参数

*/
static const struct tslib_vars linear_vars[] =
{
	{ "xyswap",	(void *)1, linear_xyswap }
};

/*
	移植到stm32平台时，将mod_init函数改为mod_linear_init，
	单片机嵌入式平台，函数同名，暂时没有机制处理。

*/
struct tslib_module_info *mod_linear_init(struct tsdev *dev, const char *params)
{

	struct tslib_linear *lin;
	
	wjq_log(LOG_DEBUG, "mod_linear_init\r\n");

	lin = &Stm32TsLinear;//stm32直接定义一个结构体，赋值指针给lin

	lin->module.ops = &linear_ops;//这里其实就是把本模块的两个相关函数赋值到lin结构体

	/*  下面四个数据不会修改？  */
	lin->p_offset = 0;
	lin->p_mult   = 1;
	lin->p_div    = 1;
	lin->swap_xy  = 0;

	/*
	 以下内容就是获取系统的校准数据
	 */
	lin->a[0] = cal.a[0];
	lin->a[1] = cal.a[1];
	lin->a[2] = cal.a[2];
	lin->a[3] = cal.a[3];
	lin->a[4] = cal.a[4];
	lin->a[5] = cal.a[5];
	lin->a[6] = cal.a[6];
		
	/*
	 * Parse the parameters. 解析传入的参数
	 */
	if (tslib_parse_vars(&lin->module, linear_vars, NR_VARS, params)) 
	{	
		return NULL;
	}
	
	wjq_log(LOG_DEBUG, "mod linear init ok\r\n");
	
	return &lin->module;
}

