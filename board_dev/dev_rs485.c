/**
 * @file            dev_rs485.c
 * @brief           485通信驱动
 * @author          wujique
 * @date            2017年12月8日 星期五
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年12月8日 星期五
 *   作    者:         wujique
 *   修改内容:   		创建文件
 					485驱动基于串口驱动
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
#include "wujique_sysconf.h"


/*rs485使用串口1*/
#define DEV_RS485_UART MCU_UART_1
s32 RS485Gd = -2;
/**
 *@brief:      dev_rs485_init
 *@details:    初始化485设备
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_rs485_init(void)
{
	#ifdef SYS_USE_RS485
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE); //使能 PG时钟
	//PG8 推挽输出， 485 模式控制
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //GPIOG8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //速度 100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOG, &GPIO_InitStructure); //初始化 PG8	

	//初始化设置为接收模式
	GPIO_ResetBits(GPIOG, GPIO_Pin_8);

	RS485Gd = -1;
	#else
	wjq_log(LOG_INFO, ">---------------RS485 IS NO ININT!\r\n");
	#endif
	return 0;
}
/**
 *@brief:      dev_rs485_open
 *@details:    打开RS485设备
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_rs485_open(void)
{
	if(RS485Gd!= -1)
		return -1;
	
	mcu_uart_open(DEV_RS485_UART);
	mcu_uart_set_baud(DEV_RS485_UART, 9600);	
	RS485Gd = 0;
	return 0;
}
/**
 *@brief:      dev_rs485_close
 *@details:    关闭RS485设备
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_rs485_close(void)
{
	if(RS485Gd!= 0)
		return -1;
	
	mcu_uart_close(DEV_RS485_UART);
	return 0;
}

/**
 *@brief:      dev_rs485_read
 *@details:    RS485接收数据
 *@param[in]   u8 *buf  
               s32 len  
 *@param[out]  无
 *@retval:     
 */
s32 dev_rs485_read(u8 *buf, s32 len)
{
	s32 res;
	
	if(RS485Gd!= 0)
		return -1;
	res = mcu_uart_read(DEV_RS485_UART, buf, len);	
	
	return res;
}
/**
 *@brief:      dev_rs485_write
 *@details:    rs485发送数据
 *@param[in]   u8 *buf  
               s32 len  
 *@param[out]  无
 *@retval:     
 */
s32 dev_rs485_write(u8 *buf, s32 len)
{
	s32 res;
	
	if(RS485Gd!= 0)
		return -1;
	GPIO_SetBits(GPIOG, GPIO_Pin_8);//设置为发送模式
	res = mcu_uart_write(DEV_RS485_UART, buf, len);
	GPIO_ResetBits(GPIOG, GPIO_Pin_8);//发送结束后设置为接收模式

	return res;
}
/**
 *@brief:      dev_rs485_ioctl
 *@details:    IOCTL
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_rs485_ioctl(void)
{
	if(RS485Gd!= 0)
		return -1;

		return 0;
}

extern void Delay(__IO uint32_t nTime);

/**
 *@brief:      dev_rs485_test
 *@details:    RS485测试程序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_rs485_test(u8 mode)
{
	u8 buf[20];
	u8 len;
	s32 res;
	
	dev_rs485_open();

	if(mode == 1)// 发送端测试
	{
		while(1)
		{
			Delay(1000);
			res = dev_rs485_write("rs485 test\r\n", 13);
			wjq_log(LOG_FUN, "dev rs485 write:%d\r\n", res);
		}
	}
	else//接收端测试
	{
		while(1)
		{
			Delay(20);
			len = dev_rs485_read(buf, sizeof(buf));
			if(len > 0)
			{
				buf[len] = 0;
				wjq_log(LOG_FUN, "%s", buf);
				memset(buf, 0, sizeof(buf));
			}
		}
	}

}

/*------ end file --------*/


