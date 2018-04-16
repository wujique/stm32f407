#ifndef _TSLIB_H_
#define _TSLIB_H_
/*
 *  tslib/src/tslib.h
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.
 *
 * $Id: tslib.h,v 1.4 2005/02/26 01:47:23 kergoth Exp $
 *
 * Touch screen library interface definitions.
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <stdarg.h>
#include "wujique_log.h"


#ifndef ts_error
#define ts_error wjq_log
#endif
/*---------------*/
struct tsdev;

struct ts_sample //触摸屏一个样点
{
	int		x;
	int		y;
	unsigned int	pressure;
	//struct timeval	tv;//时间，移植到STM32平台，应该不需要
};

/*
	使用五点校准法，将5对LCD坐标跟触摸屏数据传入，计算出7个校准数据
	原来放在校准文件的，移植后全部归入tslib模块
*/
typedef struct {
	int x[5], xfb[5]; //x,y是触摸屏，xfb，yfb是对应的LCD坐标值
	int y[5], yfb[5];
	unsigned int a[7];	//校准得到的7个参数
} calibration;


/*
 * Close the touchscreen device, free all resources.
 */
extern int ts_close(struct tsdev *);

/*
 * Configure the touchscreen device.
 */
extern int ts_config(struct tsdev *);

/*
 * Change this hook to point to your custom error handling function.
 */
extern int (*ts_error_fn)(const char *fmt, va_list ap);

/*
 * Returns the file descriptor in use for the touchscreen device.
 */
extern int ts_fd(struct tsdev *);

/*
 * Load a filter/scaling module
 */
extern int ts_load_module(struct tsdev *, const char *mod, const char *params);

/*
 * Open the touchscreen device.
 */
extern struct tsdev *ts_open(const char *dev_name, int nonblock);

/*
 * Return a scaled touchscreen sample.
 */
extern int ts_read(struct tsdev *, struct ts_sample *, int);

/*
 * Returns a raw, unscaled sample from the touchscreen.
 */
extern int ts_read_raw(struct tsdev *, struct ts_sample *, int);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _TSLIB_H_ */
