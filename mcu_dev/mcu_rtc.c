/**
 * @file            mcu_rtc.c
 * @brief           stm32片上RTC驱动
 * @author          wujique
 * @date            2018年1月24日 星期三
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年1月24日 星期三
 *   作    者:         wujique
 *   修改内容:   创建文件
*/
#include <stdarg.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "mcu_rtc.h"

/**
 *@brief:      mcu_rtc_init
 *@details:    复位时初始化RTC
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_rtc_init(void)
{

	RTC_InitTypeDef RTC_InitStructure;
	volatile u32 cnt = 0; 
	
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_AHB1Periph_BKPSRAM, ENABLE);
	/*操作RTC寄存器，需要使能备份区*/
	PWR_BackupAccessCmd(ENABLE); 
	
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x55aa)	
	{
		wjq_log(LOG_DEBUG, " init rtc\r\n");
		/*开启LSE时钟*/
		RCC_LSEConfig(RCC_LSE_ON);
		/*等待RCC LSE时钟就绪*/
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	
		{
			cnt++;
			if(cnt>0x2000000)
			{
				wjq_log(LOG_ERR, "lse not rdy\r\n");
				return -1;	

			}
		}		
			
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);

    	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC异步分频系数(1~0X7F)
    	RTC_InitStructure.RTC_SynchPrediv  = 0xFF;//RTC同步分频系数(0~7FFF)
    	RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//RTC设置为,24小时格式
    	RTC_Init(&RTC_InitStructure);

		RTC_TimeTypeDef RTC_TimeStructure;

		RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
		RTC_TimeStructure.RTC_Hours = 0;
		RTC_TimeStructure.RTC_Minutes = 0;
		RTC_TimeStructure.RTC_Seconds = 0;
		RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

		RTC_DateTypeDef RTC_DateStructure;
		RTC_DateStructure.RTC_Date = 1;
		RTC_DateStructure.RTC_Month = RTC_Month_January;
		RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Thursday;
		RTC_DateStructure.RTC_Year = 0;//已1970年为起点，
		RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
	 
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x55aa);	//标记已经初始化过了
	} 

	wjq_log(LOG_INFO, " init rtc finish\r\n");	
	return 0;	
}

/**
 *@brief:      mcu_rtc_set_time
 *@details:    设置时间
 *@param[in]   u8 hours    
               u8 minutes  
               u8 seconds  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_rtc_set_time(u8 hours, u8 minutes, u8 seconds)
{
	RTC_TimeTypeDef RTC_TimeStructure;

	RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
	RTC_TimeStructure.RTC_Hours = hours;
	RTC_TimeStructure.RTC_Minutes = minutes;
	RTC_TimeStructure.RTC_Seconds = seconds;
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
	return 0;
}		
/**
 *@brief:      mcu_rtc_set_date
 *@details:    设置日期
 *@param[in]   u8 year     
               u8 weekday  
               u8 month    
               u8 date     
 *@param[out]  无
 *@retval:     
 */
s32 mcu_rtc_set_date(u8 year, u8 weekday, u8 month, u8 date)
{
	RTC_DateTypeDef RTC_DateStructure;
	RTC_DateStructure.RTC_Date = date;
	RTC_DateStructure.RTC_Month = month;
	RTC_DateStructure.RTC_WeekDay = weekday;
	RTC_DateStructure.RTC_Year = year - 1970;//已1970年为起点，
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
	return 0;
}

/**
 *@brief:      mcu_rtc_get_date
 *@details:    读取日期
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_rtc_get_date(void)
{
	RTC_DateTypeDef RTC_DateStructure;
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	wjq_log(LOG_DEBUG, "%d, %d, %d, %d\r\n", 
					RTC_DateStructure.RTC_Year+1970,
					RTC_DateStructure.RTC_Month,
					RTC_DateStructure.RTC_Date,
					RTC_DateStructure.RTC_WeekDay);
	
	return 0;
}
/**
 *@brief:      mcu_rtc_get_time
 *@details:    读取时间
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_rtc_get_time(void)
{
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	wjq_log(LOG_DEBUG, "%d, %d, %d\r\n",
					RTC_TimeStructure.RTC_Hours,
					RTC_TimeStructure.RTC_Minutes,
					RTC_TimeStructure.RTC_Seconds);
	return 0;
}
/*

	获取UTC时间

*/
time_t mcu_rtc_get_utc_time()
{
	time_t time;

	wjq_log(LOG_DEBUG, "%lu\n",time);

	return time;
}

struct tm* gmtime (const time_t *timep)
{
	return 0;
}
/*

	将UTC时间转化为当地时间

*/
struct tm* localtime (const time_t *timep)
{
	return 0;
}

s32 mcu_rtc_test(void)
{
		return 0;
}

