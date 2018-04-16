/*
 *  tslib/src/ts_read_raw.c
 *
 *  Copyright (C) 2003 Chris Larson.
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 * Read raw pressure, x, y, and timestamp from a touchscreen device.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tslib-private.h"

int ts_read_raw(struct tsdev *ts, struct ts_sample *samp, int nr)
{
	int result = ts->list_raw->ops->read(ts->list_raw, samp, nr);

	return result;
}

