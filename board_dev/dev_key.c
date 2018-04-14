/**
 * @file                dev_key.c
 * @brief           按键扫描驱动
 * @author          wujique
 * @date            2018年2月1日 星期四
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年2月1日 星期四
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
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "dev_key.h"

//#define DEV_KEY_DEBUG

#ifdef DEV_KEY_DEBUG
#define KEY_DEBUG	wjq_log 
#else
#define KEY_DEBUG(a, ...)
#endif

extern void Delay(__IO uint32_t nTime);

/*按键缓冲*/
#define KEY_BUF_SIZE (12)
u8 KeyBuf[KEY_BUF_SIZE];
u8 KeyR =0;
u8 KeyW = 0;
/*按键设备符*/
s32 KeyGd = -1;
/*  防抖动次数，时间等于次数乘上scan key执行间隔*/
#define DEV_KEY_DEBOUNCE	(10)
/**
 *@brief:      dev_key_init
 *@details:    初始化按键
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_key_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//使能 GPIOA 时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//对应引脚 PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化 GPIOA0	
	return 0;
}
/**
 *@brief:      dev_key_open
 *@details:    打开按键设备
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_key_open(void)
{
	KeyGd = 0;
	return 0;
}
/**
 *@brief:      dev_key_close
 *@details:    关闭按键
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_key_close(void)
{
	KeyGd = -1;
	return 0;
}

/**
 *@brief:      dev_key_scan
 *@details:    扫描按键
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_key_scan(void)
{
	volatile u8 sta;//局部变量，放在栈空间，进入函数时使用，退出后释放。
	static u8 new_sta = Bit_SET;
	static u8 old_sta = Bit_SET;
	u8 key_value;
	static u8 cnt = 0;

	if(KeyGd != 0)
		return -1;

	/*读按键状态*/
	sta = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
	/*
		判断跟上次读的状态是不是一样，
		原因是，保证防抖过程的状态是连续一样的。
		不明白可以想象按键状态快速变化。
		这种情况我们不要认为是有按键。
	*/
	if((sta != new_sta))
	{
		cnt = 0;
		new_sta = sta;
	}
	/*
		与上次得到键值的状态比较，
		如果不一样，说明按键有变化
	*/
	if(sta != old_sta)
	{
		cnt++;

		if(cnt >= DEV_KEY_DEBOUNCE)
		{
			/*防抖次数达到，扫描到一个按键变化*/
			cnt = 0;
			key_value = DEV_KEY_PRESS;

			/*判断是松开还是按下*/
			if(sta == Bit_RESET)
			{
				KEY_DEBUG(LOG_DEBUG, "key press!\r\n");
			}
			else
			{
				key_value += DEV_KEY_PR_MASK;
				KEY_DEBUG(LOG_DEBUG, "key rel!\r\n");
			}
			/*键值写入环形缓冲*/
			KeyBuf[KeyW] = key_value;
			KeyW++;
			if(KeyW>= KEY_BUF_SIZE)
			{
				KeyW = 0;
			}
			/*更新状态*/
			old_sta = new_sta;
		}
	}
	return 0;
}
/**
 *@brief:      dev_key_read
 *@details:    读按键
 *@param[in]   u8 *key  按键存放指针	
               u8 len   读个数   
 *@param[out]  无
 *@retval:     读到按键个数	
 */
s32 dev_key_read(u8 *key, u8 len)
{
	u8 i =0;
	
	if(KeyGd != 0)
		return -1;
	
	while(1)
	{
		if(KeyR != KeyW)
		{
			*(key+i) = KeyBuf[KeyR];
			KeyR++;
			if(KeyR >= KEY_BUF_SIZE)
				KeyR = 0;
		}
		else
		{
			break;
		}
		
		i++;
		if(i>= len)
			break;
	}

	return i;
}

/*


	测试用，以后要移动到测试程序SDK

*/
s32 dev_key_waitkey(void)
{
	s32 res;
	u8 key;

	while(1)
	{
		res = dev_key_read(&key, 1);

		if(res == 1)
		{
			if(key == DEV_KEY_PRESS)	
			{

				break;
			}
		}
	}

	return 0;
}



