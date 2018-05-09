/**
 * @file            dev_touchkey.c
 * @brief           触摸按键驱动程序
 * @author          wujique
 * @date            2017年11月7日 星期二
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年11月7日 星期二
 *   作    者:      屋脊雀工作室
 *   修改内容:   创建文件
   		1 源码归屋脊雀工作室所有。
		2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
		3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
		4 可随意修改源码并分发，但不可直接销售本代码获利，并且请保留WUJIQUE版权说明。
		5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
		6 使用本源码则相当于认同本版权说明。
		7 如侵犯你的权利，请联系：code@wujique.com
		8 一切解释权归屋脊雀工作室所有。
*/
/*
	电容触摸按键流程
	1 IO口输出高电平，对触摸铜箔充电。
	2 设置定时器输入捕获。
	3 查询，得到捕获值。
	4 处理捕获值（与历史值一起做分析），得到触摸状态，如有触摸则填入触摸按键缓冲区。
		
*/

#include "stm32f4xx.h"
#include "wujique_log.h"
#include "mcu_timer.h"
#include "dev_touchkey.h"

//#define DEV_TOUCHKEY_DEBUG

#ifdef DEV_TOUCHKEY_DEBUG
#define TOUCHKEY_DEBUG	wjq_log 
#else
#define TOUCHKEY_DEBUG(a, ...)
#endif


#define DEV_TOUCHKEY_GATE (50)//确认状态变化的门限值，根据硬件性能调节本参数到合适灵敏度即可。
#define DEV_TOUCHKEY_DATA_NUM (50)//一轮稳定状态需要的时间流个数，可以通过修改这个调节触摸扫描时间
static u16 TouchKeyLastCap = 0;//最后一次稳定的CAP平均值

#define DEV_TOUCHKEY_BUF_SIZE (16)
static u8 TouchKeyBuf[DEV_TOUCHKEY_BUF_SIZE];//触摸按键缓冲区，处理得到时间后就将时间放到这个缓冲区
static u8 TouchKeyWrite = 0;
static u8 TouchKeyRead = 0;

s32 TouchKeyGd = -2;

/**
 *@brief:      dev_touchkey_resetpad
 *@details:    复位触摸按键（输出高电平充电）
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static s32 dev_touchkey_resetpad(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA,GPIO_Pin_3);	

	return 0;
}
/**
 *@brief:      dev_touchkey_iocap
 *@details:       IO设置为定时器捕获功能
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static s32 dev_touchkey_iocap(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能 PORTA时钟
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM2); //PA3 复用位定时器 2
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //GPIOA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//速度 100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//不带上下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化 PA3

	return 0;

}

s32 dev_touchkey_init(void)
{
	TouchKeyGd = -1;
	return 0;
}

s32 dev_touchkey_open(void)
{
	if(TouchKeyGd != -1)
		return -1;
	TouchKeyGd = 0;
	return 0;
}

s32 dev_touchkey_close(void)
{
	TouchKeyGd = -1;
	return 0;
}
/**
 *@brief:      dev_touchkey_read
 *@details:    读设备，获取触摸事件
 *@param[in]   u8 *buf    
               u32 count  
 *@param[out]  无
 *@retval:     
 */
s32 dev_touchkey_read(u8 *buf, u32 count)
{
	u32 cnt = 0;

	if(TouchKeyGd != 0)
		return -1;
	
	while(1)
	{
		if(TouchKeyWrite ==  TouchKeyRead)
			break;

		if(cnt >= count)
			break;
		
		*(buf+cnt) = TouchKeyBuf[TouchKeyRead++];
		if(TouchKeyRead >= DEV_TOUCHKEY_BUF_SIZE)
			TouchKeyRead = 0;

		cnt++;
	}
	
	return cnt;

}

s32 dev_touchkey_write(void)
{
	return 0;
}

/**
 *@brief:      dev_touchkey_scan
 *@details:    扫描触摸捕获的数据流
 *@param[in]   u32  
 *@param[out]  无
 *@retval:                  	
 */
static s32 dev_touchkey_scan(u32 cap)
{
	static u16 average = 0;//平均值
	static u8 cap_cnt = 0;//有效捕获计数
	static u8 last_dire = DEV_TOUCHKEY_IDLE;//上一个值的方向，1为变大,触摸；2为变小，松开
	static u8 last_chg = NULL;
	
	TOUCHKEY_DEBUG(LOG_DEBUG, "--%08x-%04x-", cap, TouchKeyLastCap);
	if(cap > TouchKeyLastCap + DEV_TOUCHKEY_GATE)
	{
		/*与上一次变化的平均值比较， 大，进入*/
		if(last_dire != DEV_TOUCHKEY_TOUCH)
		{
			cap_cnt = 0;
			average = 0;
			last_dire = DEV_TOUCHKEY_TOUCH;
		}

		cap_cnt++;
		average = average + cap;
	}
	else if(cap < TouchKeyLastCap - DEV_TOUCHKEY_GATE)
	{
		if(last_dire != DEV_TOUCHKEY_RELEASE)
		{
			cap_cnt = 0;
			average = 0;
			last_dire = DEV_TOUCHKEY_RELEASE;
		}	
		cap_cnt++;
		average = average + cap;

	}
	else
	{

		cap_cnt = 0;
		average = 0;
		last_dire = DEV_TOUCHKEY_IDLE;
	}

	if(cap_cnt >= DEV_TOUCHKEY_DATA_NUM)
	{

		if(TouchKeyLastCap == 0)
		{
			TOUCHKEY_DEBUG(LOG_DEBUG, "\r\n-------------------init\r\n");	
		}
		else
		{
			TOUCHKEY_DEBUG(LOG_DEBUG, "\r\n-------------------chg\r\n");
			if(last_chg != last_dire)//防止重复上报
			{
				TOUCHKEY_DEBUG(LOG_DEBUG, "\r\n--------report\r\n");
				TouchKeyBuf[TouchKeyWrite++] = last_dire;
				if(TouchKeyWrite >= DEV_TOUCHKEY_BUF_SIZE)
				TouchKeyWrite = 0;
			}
			
			last_chg = last_dire;
			
		}
		/*保存新的平均值*/
		TouchKeyLastCap = average/DEV_TOUCHKEY_DATA_NUM;
		cap_cnt = 0;
		average = 0;
	}
	return 0;
}
/**
 *@brief:      dev_touchkey_task
 *@details:    触摸按键线程，常驻任务
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_touchkey_task(void)
{
	volatile u32 i = 0;
	u32 cap;

	if(TouchKeyGd != 0)
		return -1;
	//IO输出1，对电容充电
	dev_touchkey_resetpad();
	//延时一点，充电
	for(i=0;i++;i<0x12345);
	//将IO口设置为定时去输入捕获通道
	dev_touchkey_iocap();
	//开定时器捕获，如果预分频8，一个定时器计数是100ns左右 ，这个值要通过调试，
	mcu_timer_cap_init(0xffffffff, 8);
	cap = mcu_timer_get_cap();
	TOUCHKEY_DEBUG(LOG_DEBUG, "\r\n%08x---", cap);

	dev_touchkey_scan(cap);
	
	return 0;
}

/**
 *@brief:      dev_touchkey_test
 *@details:    触摸按键测试程序
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 dev_touchkey_test(void)
{
	u8 tmp;
	s32 res;

	//dev_touchkey_open();
	
	res = dev_touchkey_read(&tmp, 1);
	if(1 == res)
	{
		if(tmp == DEV_TOUCHKEY_TOUCH)
		{
			wjq_log(LOG_FUN, "touch key test get a touch event!\r\n");
		}
		else if(tmp == DEV_TOUCHKEY_RELEASE)
		{
			wjq_log(LOG_FUN, "touch key test get a release event!\r\n");
		}
	}
	return 0;
	
}


