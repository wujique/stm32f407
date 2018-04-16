/*
 *  tslib/plugins/variance.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: variance.c,v 1.6 2005/02/26 01:47:23 kergoth Exp $
 *
 * Variance filter for touchscreen values.
 *
 * Problem: some touchscreens are sampled very roughly, thus even if
 * you hold the pen still, the samples can differ, sometimes substantially.
 * The worst happens when electric noise during sampling causes the result
 * to be substantially different from the real pen position; this causes
 * the mouse cursor to suddenly "jump" and then return back.
 *
 * Solution: delay sampled data by one timeslot. If we see that the last
 * sample read differs too much, we mark it as "suspicious". If next sample
 * read is close to the sample before the "suspicious", the suspicious sample
 * is dropped, otherwise we consider that a quick pen movement is in progress
 * and pass through both the "suspicious" sample and the sample after it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tslib.h"
#include "tslib-filter.h"

#define VAR_PENDOWN 		0x00000001
#define VAR_LASTVALID		0x00000002
#define VAR_NOISEVALID		0x00000004
#define VAR_SUBMITNOISE 	0x00000008

struct tslib_variance 
{
	struct tslib_module_info module;
	int delta;
    struct ts_sample last;	//上一个样点
    struct ts_sample noise;//噪声（可疑）
	unsigned int flags;
};

static int sqr (int x)
{
	return x * x;
}
/**
 *@brief:      variance_read
 *@details:    滤波算法插件
 				处理飞点
 *@param[in]   struct tslib_module_info *info  
               struct ts_sample *samp          
               int nr                          
 *@param[out]  无
 *@retval:     static
 */
static int variance_read(struct tslib_module_info *info, struct ts_sample *samp, int nr)
{
	struct tslib_variance *var = (struct tslib_variance *)info;
	struct ts_sample cur;
	int count = 0, dist;

	while (count < nr) 
	{
		/*
			如果N+1、N+2两个点跟N点的距离超出阀值，则可能是快速移动，
			这时会将VAR_SUBMITNOISE标识置起，
			置这个标识是为了下一个循环判断N+1跟N+2之间是否也超出阀值，
		*/
		if (var->flags & VAR_SUBMITNOISE) 
		{
			cur = var->noise;////不重新获取新样点，而是将noise中的样点作为当前样点
			var->flags &= ~VAR_SUBMITNOISE;
		} 
		else 
		{
			/*  
			读取一个新样点
			*/
			if (info->next->ops->read(info->next, &cur, 1) < 1)
				return count;
		}

		if (cur.pressure == 0)//样点压力值为0, 
		{
			/* Flush the queue immediately when the pen is just
			 * released, otherwise the previous layer will
			 * get the pen up notification too late. This 
			 * will happen if info->next->ops->read() blocks.
			 */
			if (var->flags & VAR_PENDOWN) 
			{
				/*上一点pressure不为0，会将VAR_PENDOWN 标识置起，
				这次压力为0，说明可能是提笔，可能而已，也可能是个噪声样点
				先保存到noise*/
				var->flags |= VAR_SUBMITNOISE;
				var->noise = cur;//先将样点保存到noise
			}
			/* Reset the state machine on pen up events. */
			var->flags &= ~(VAR_PENDOWN | VAR_NOISEVALID | VAR_LASTVALID);
			goto acceptsample;
		} else
			var->flags |= VAR_PENDOWN;

		if (!(var->flags & VAR_LASTVALID)) 
		{
			var->last = cur;
			var->flags |= VAR_LASTVALID;
			continue;
		}

		if (var->flags & VAR_PENDOWN) {
			/* Compute the distance between last sample and current */
			dist = sqr (cur.x - var->last.x) +
			       sqr (cur.y - var->last.y);

			if (dist > var->delta) {
				//uart_printf("%d-",dist);
				/* 
				Do we suspect the previous sample was a noise? 
				
				上一个样点如果也超出阈值，就会将VAR_NOISEVALID置位，
				这次的样点又超出阈值，那么就可以认为是快速移动
				*/
				if (var->flags & VAR_NOISEVALID) {
					//uart_printf("q-");	
					/* Two "noises": it's just a quick pen movement */
					samp [count++] = var->last = var->noise;
					
					var->flags = (var->flags & ~VAR_NOISEVALID) |
									VAR_SUBMITNOISE;
				} else{
					/*第一次超出阈值，置位标志*/
					var->flags |= VAR_NOISEVALID;
				}
				/* The pen jumped too far, maybe it's a noise ... */
				var->noise = cur;
				continue;
			} else{
				//uart_printf("g ");
				var->flags &= ~VAR_NOISEVALID;
			}
		}

acceptsample:
		samp [count++] = var->last;
		var->last = cur;
	}
	
	return count;
}

static int variance_fini(struct tslib_module_info *info)
{
    return 0;
}

static const struct tslib_ops variance_ops =
{
	.read	= variance_read,
	.fini	= variance_fini,
};

static int variance_limit(struct tslib_module_info *inf, char *str, void *data)
{
	struct tslib_variance *var = (struct tslib_variance *)inf;
	unsigned long v;

	v = strtoul(str, NULL, 0);

	if (v == 0xffffffff)
		return -1;

	
	switch ((int)data) 
	{
	case 1:
		var->delta = v;
		break;

	default:
		return -1;
	}
	return 0;
}

static const struct tslib_vars variance_vars[] =
{
	{ "delta",	(void *)1, variance_limit },
};

#define NR_VARS (sizeof(variance_vars) / sizeof(variance_vars[0]))

struct tslib_variance Stm32TsVariance;

struct tslib_module_info *mod_variance_init(struct tsdev *dev, const char *params)
{
	struct tslib_variance *var;

	wjq_log(LOG_DEBUG, "mod_variance_init\r\n");

	var = &Stm32TsVariance;

	if (var == NULL)
		return NULL;

	var->module.ops = &variance_ops;

	var->delta = 10;
	var->flags = 0;

	if (tslib_parse_vars(&var->module, variance_vars, NR_VARS, params)) 
	{
		return NULL;
	}

    var->delta = sqr (var->delta);

	return &var->module;
}

