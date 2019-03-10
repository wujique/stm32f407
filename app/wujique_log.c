/**
 * @file                wujique_log.c
 * @brief           调试信息管理
 * @author          wujique
 * @date            2018年4月12日 星期四
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年4月12日 星期四
 *   作    者:         wujique
 *   修改内容:   创建文件
 版权说明：
 					1 源码归屋脊雀工作室所有。
 					2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
 					3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
 					4 可随意修改源码并分发，但不可直接销售本代码获利，并且请保留WUJIQUE版权说明。
 					5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
 					6 使用本源码则相当于认同本版权说明。
 					7 如侵犯你的权利，请联系：code@wujique.com
					8 一切解释权归屋脊雀工作室所有。
*/
#include <stdarg.h>
#include <stdio.h>

#include "stm32f4xx.h"
#include "mcu_uart.h"
#include "wujique_log.h"

LOG_L LogLevel = LOG_DEBUG;//系统调试信息等级
/*
使用串口输出调试信息
*/
s8 string[256];//调试信息缓冲，输出调试信息一次不可以大于256

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    USART_SendData(USART3, (uint8_t) ch);

    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return ch;
}

extern int vsprintf(char * s, const char * format, __va_list arg);
/**
 *@brief:      uart_printf
 *@details:    从串口格式化输出调试信息
 *@param[in]   s8 *fmt  
               ...      
 *@param[out]  无
 *@retval:     
 */
void uart_printf(s8 *fmt,...)
{
    s32 length = 0;
    va_list ap;

    s8 *pt;
    
    va_start(ap,fmt);
    vsprintf((char *)string,(const char *)fmt,ap);
    pt = &string[0];
    while(*pt!='\0')
    {
        length++;
        pt++;
    }
    
    mcu_uart_write(PC_PORT, (u8*)&string[0], length);  //写串口
    
    va_end(ap);
}

void wjq_log(LOG_L l, s8 *fmt,...)
{
	if(l > LogLevel)
		return;

	s32 length = 0;
    va_list ap;

    s8 *pt;
    
    va_start(ap,fmt);
    vsprintf((char *)string,(const char *)fmt,ap);
    pt = &string[0];
    while(*pt!='\0')
    {
        length++;
        pt++;
    }
    
    mcu_uart_write(PC_PORT, (u8*)&string[0], length);  //写串口
    
    va_end(ap);
}
/**
 *@brief:      PrintFormat
 *@details:    格式化输出BUF中的数据
 *@param[in]   u8 *wbuf  
               s32 wlen  
 *@param[out]  无
 *@retval:     
 */
void PrintFormat(u8 *wbuf, s32 wlen)
{   
    s32 i;
    for(i=0; i<wlen; i++)
    {
        if((0 == (i&0x0f)))//&&(0 != i))
        {
            uart_printf("\r\n");
        }
        uart_printf("%02x ", wbuf[i]);
    }
    uart_printf("\r\n");
}

void cmd_uart_printf(s8 *fmt,...)
{
    s32 length = 0;
    va_list ap;

    s8 *pt;
    
    va_start(ap,fmt);
    vsprintf((char *)string,(const char *)fmt,ap);
    pt = &string[0];
    while(*pt!='\0')
    {
        length++;
        pt++;
    }
    
    mcu_uart_write(PC_PORT, (u8*)&string[0], length);  //写串口
    
    va_end(ap);
}

