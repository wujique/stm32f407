/*
 *  tslib/src/ts_read_raw_module.c
 *
 *  Original version:
 *  Copyright (C) 2001 Russell King.
 *
 *  Rewritten for the Linux input device API:
 *  Copyright (C) 2002 Nicolas Pitre
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: input-raw.c,v 1.5 2005/02/26 01:47:23 kergoth Exp $
 *
 * Read raw pressure, x, y, and timestamp from a touchscreen device.

参考input_raw.c，编写一个屋脊雀stm32_raw.c
 
 */
#include <stdio.h>
#include <stdlib.h>
#include "tslib-private.h"

extern s32 dev_touchscreen_read(struct ts_sample *samp, int nr);

struct tslib_input {
	struct tslib_module_info module;

	int	current_x;
	int	current_y;
	
	int	current_p;
};

/*

	这个即是最低层的读接口，直接读STM32采样的原始数据。

*/
static int ts_input_read(struct tslib_module_info *inf,
			 struct ts_sample *samp, int nr)
{
	int ret = nr;
	
	ret = dev_touchscreen_read(samp, nr);

	return ret;
}

static int ts_input_fini(struct tslib_module_info *inf)
{
	return 0;
}

static const struct tslib_ops __ts_input_ops = {
	.read	= ts_input_read,
	.fini	= ts_input_fini,
};

struct tslib_input Stm32TsInput;

struct tslib_module_info *mod_stm32_init(struct tsdev *dev, const char *params)
{
	struct tslib_input *i;

	wjq_log(LOG_DEBUG, "mod_stm32_init\r\n");

	i = &Stm32TsInput;
	if (i == NULL)
		return NULL;

	i->module.ops = &__ts_input_ops;
	i->current_x = 0;
	i->current_y = 0;
	i->current_p = 0;

	return &(i->module);
}
