#ifndef _MCU_RTC_H_
#define _MCU_RTC_H_

typedef long time_t;

/*
	从1970年1月1日0:0:0开始过了多少秒
*/
struct timeval {
	time_t tv_sec; /*秒*/    
	time_t tv_usec; /*微秒*/ 
};

struct tm {
	int tm_sec; /*秒，正常范围0-59， 但允许至61*/    
	int tm_min; /*分钟，0-59*/    
	int tm_hour; /*小时， 0-23*/     
	int tm_mday; /*日，即一个月中的第几天，1-31*/    
	int tm_mon; /*月， 从一月算起，0-11*/  
	int tm_year; /*年， 从1900至今已经多少年*/ 
	int tm_wday; /*星期，一周中的第几天， 从星期日算起，0-6*/    
	int tm_yday; /*从今年1月1日到目前的天数，范围0-365*/    
	int tm_isdst; /*夏令时标识符 1: 是 DST，0: 不是 DST，负数：不了解*/ 
	long int tm_gmtoff;/*指定了日期变更线东面时区中UTC东部时区正秒数或UTC西部时区的负秒数*/
	const char *tm_zone;     /*当前时区的名字(与环境变量TZ有关)*/
};

extern s32 mcu_rtc_init(void);
extern s32 mcu_rtc_set_time(u8 hours, u8 minutes, u8 seconds);
extern s32 mcu_rtc_set_date(u8 year, u8 weekday, u8 month, u8 date);
#endif
