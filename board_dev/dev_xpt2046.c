/**
 * @file            dev_xpt2046.c
 * @brief           电阻触摸屏控制芯片xpt2046
 * @author          wujique
 * @date            2018年3月21日 星期三
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年3月21日 星期三
 *   作    者:         wujique
 *   修改内容:   创建文件
		版权说明：
		1 源码归屋脊雀工作室所有。
		2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
		3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
		4 可随意修改源码并分发，但不可直接销售本代码获利，并且保留版权说明。
		5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
		6 使用本源码则相当于认同本版权说明。
		7 如侵犯你的权利，请联系：code@wujique.com
		8 一切解释权归屋脊雀工作室所有。
*/

#include "stm32f4xx.h"
#include "wujique_log.h"
#include "tslib.h"

#include "mcu_spi.h"
#include "dev_touchscreen.h"
#include "mcu_timer.h"
#include "wujique_sysconf.h"

extern s32 dev_touchscreen_write(struct ts_sample *samp, int nr);

/*xpt 2046使用 模拟串口1*/
#define XPT2046_SPI DEV_VSPI_1


/*
	命令字意义：
	bit7：S，启动标志，固定为1
	bit6-bit4：A2/A1/A0,通道选择，差分模式跟单端不一样，下面只说差分
		001，测Y轴
		011，测Z1
		100，测Z2
		101，测X轴
	bit3 ：MODE,1 =8位ADC，0 =12位ADC
	bit2：SEF/DFR,单端模式还是差分模式，XY轴跟压力可以用差分，最好用差分；其他只能用单端
	bit1-bit0：PD1/PD0, 工作模式 11，总是处于供电状态，00，在变换之间进入低功耗

	测量压力时，Z2电压值大于Z1，当两者的差，小于阈值，则认为笔按下
*/
#define XPT2046_CMD_X 0xd0
#define XPT2046_CMD_Y 0X90
#define XPT2046_CMD_Z1 0xb0
#define XPT2046_CMD_Z2 0xc0

/*
测量到的压力电压阈值，小于PENdown，认为是下笔，
大于penup认为是起笔，两者之间不处理
这个值跟压力不一样，
上送的样点压力最小是0，也就是起笔后，
最大是电压阈值为0的时候（可能达不到），
通过计算转换，也就是0-400；0起笔，400，电压压力为0

*/
#define DEV_XPT2046_PENDOWN_GATE (3500)
#define DEV_XPT2046_PENUP_GATE	 (3700)


s32 DevXpt2046Gd = -2;

void dev_xpt2046_task(void);

/**
 *@brief:      dev_xpt2046_init
 *@details:    初始化XPT2046
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_xpt2046_init(void)
{
	#ifdef SYS_USE_TS_IC_CASE
	DevXpt2046Gd = -1;
	
	mcu_timer7_init();
	#else
	wjq_log(LOG_INFO, ">>>>>>>>>xpt2046 not init!\r\n");

	#endif

	return 0;

}
/**
 *@brief:      dev_xpt2046_open
 *@details:    打开xpt2046
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_xpt2046_open(void)
{

	if(DevXpt2046Gd != -1)
		return -1;
	mcu_spi_open(XPT2046_SPI, SPI_MODE_0, 2);
	mcu_tim7_start(100, dev_xpt2046_task, 0);
	DevXpt2046Gd = 0;
	return 0;
}
/**
 *@brief:      dev_xpt2046_close
 *@details:    关闭XPT2046
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_xpt2046_close(void)
{

	if(DevXpt2046Gd != 0)
		return -1;
	
	mcu_spi_close(XPT2046_SPI);
	DevXpt2046Gd = -1;
	return 0;
}
/**
 *@brief:      dev_xpt2046_task
 *@details:    xpt2046检测流程
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void dev_xpt2046_task(void)
{

	static u16 pre_y, pre_x;
	static u16 sample_y, sample_x;

	static u8 pendownup = 1;
	struct ts_sample tss;
	
	u8 stmp[4];
	u8 rtmp[4];
	
	if(DevXpt2046Gd != 0)
		return;
	/*

		整个流程分四步
		读Z1,Z2，用于计算压力
		读X,Y轴，用于计算坐标

		1 使用了快速16CLK操作法，过程100us左右。
		经测试，中间不需要延时。
		2 没有使用下笔中断，通过压力判断是否下笔。但是有点疑惑，理论上接触电阻应该很小的，
		 用ADC方案，正常，用XPT2046方案，感觉接触电阻比较大，不知道是哪里没有理解对。

		3 快速CLK操作，也就是在读最后一个字节的时候同时发送下一个转换命令。
		  一定要第一个字节发送0X00，第二个字节发送命令。如果第一个字节不是00，而且正好BIT7是1，
		  芯片会重新启动转换，读回来的电压值就都不对了。

		4 实测，不需要延时，如果你的SPI时钟较快，请注意延时等待转换结束。
		5 理论上还可以节省一个字节的发送时间，请自行优化。
	*/
	
	/*------------------------*/

	stmp[0] = XPT2046_CMD_Z2;
	mcu_spi_transfer(XPT2046_SPI, stmp, NULL, 1);
	//vspi_delay(100);
	stmp[0] = 0x00;
	stmp[1] = XPT2046_CMD_Z1;
	mcu_spi_transfer(XPT2046_SPI, stmp, rtmp, 2);
	pre_y = ((u16)(rtmp[0]&0x7f)<<5) + (rtmp[1]>>3);
	/*------------------------*/
	//vspi_delay(100);
	stmp[0] = 0x00;
	stmp[1] = XPT2046_CMD_X;
	mcu_spi_transfer(XPT2046_SPI, stmp, rtmp, 2);
	pre_x = ((u16)(rtmp[0]&0x7f)<<5) + (rtmp[1]>>3);
	/*------------------------*/
	//vspi_delay(100);
	stmp[0] = 0x00;
	stmp[1] = XPT2046_CMD_Y;
	mcu_spi_transfer(XPT2046_SPI, stmp, rtmp, 2);
	sample_x = ((u16)(rtmp[0]&0x7f)<<5) + (rtmp[1]>>3);
	/*------------------------*/
	//vspi_delay(100);
	stmp[0] = 0x00;
	stmp[1] = 0X00;
	mcu_spi_transfer(XPT2046_SPI, stmp, rtmp, 2);
	sample_y = ((u16)(rtmp[0]&0x7f)<<5) + (rtmp[1]>>3);

	/*
		算压力
		简化算法
		实际：
		R触摸电阻=Rx面板*（X位置/4096）*(Z2/Z1-1)
	*/
	if(pre_x +  DEV_XPT2046_PENDOWN_GATE > pre_y)
	{
		/*有压力*/
		tss.pressure = 200;//DEV_XPT2046_PENDOWN_GATE - rpress;
		tss.x = sample_x;
		tss.y = sample_y;
		dev_touchscreen_write(&tss,1);
		//uart_printf("%d,%d,%d\r\n", tss.pressure, tss.x, tss.y);
		pendownup = 0;
	}
	else if(pre_x + DEV_XPT2046_PENUP_GATE < pre_y)//没压力，不进行XY轴检测
	{
		/* 起笔只上送一点缓冲*/
		if(pendownup == 0)
		{
			pendownup = 1;
			tss.pressure = 0;//压力要搞清楚怎么计算
			tss.x = 0xffff;
			tss.y = 0xffff;
			dev_touchscreen_write(&tss,1);
		}

	}
	else
	{
		//uart_printf("--press :%d %d\r\n", pre_y, pre_x);
		/*上下笔的过渡，丢弃*/

	}

}


