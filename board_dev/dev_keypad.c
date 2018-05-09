/**
 * @file            dev_keypad.c
 * @brief           矩阵按键扫描
 * @author          wujique
 * @date            2018年3月10日 星期六
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年3月10日 星期六
 *   作    者:         屋脊雀工作室
 *   修改内容:   创建文件

		版权说明：
		1 源码归屋脊雀工作室所有。
		2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
		3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
		4 可随意修改源码并分发，但不可直接销售本代码获利，并且保留版权说明。
		5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
		6 使用本源码则相当于认同本版权说明。
		7 如侵犯你的权利，请联系：code@wujique.com
*/
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "wujique_sysconf.h"
#include "dev_keypad.h"

//#define DEV_KEYPAD_DEBUG

#ifdef DEV_KEYPAD_DEBUG
#define KEYPAD_DEBUG	wjq_log 
#else
#define KEYPAD_DEBUG(a, ...)
#endif

/*

	原则：
	扫描程序只提供键位，不提供功能
	按键定义是APP的事情，长按短按连续按，也是APP的事情。
	4*4矩阵按键，键位定义，左上角为1，从左到右，从上到下。

	扫描方法：
	行轮流输出0，读列IO的状态。

	键值参考键盘，扫描后填充到缓冲的数据是按键位置通断信息。

*/
#define KEY_PAD_COL_NUM (4)//4列
#define KEY_PAD_ROW_NUM (4)//4行
/*无按键按下时，列IO状态，同时也标识列状态哪些位有效*/
#define KEYPAD_INIT_STA_MASK (0X0F)
/*
防抖时间=防抖次数*scan函数执行间隔*KEY_PAD_ROW_NUM
不同按键建议防抖时间：
震动按键：10ms
锅仔：40ms-50ms
机械锁定按键：100ms
*/
#define KEY_PAD_DEC_TIME (3)//防抖扫描次数
/*
	扫描过程使用的变量
*/
struct _strKeyPadCtrl
{
	u8 dec;//防抖计数
	u8 oldsta;//上一次的稳定状态
	u8 newsta;
};
struct _strKeyPadCtrl KeyPadCtrl[KEY_PAD_ROW_NUM];


/*
	按键缓冲，扫描到的按键位置键值，只说明位置的通断，
	最高位为0标识通（按下），为1，则标识断开。
*/
#define KEYPAD_BUFF_SIZE (32)
static u8 KeyPadBuffW = 0;//写索引
static u8 KeyPadBuffR = 0;//读索引
static u8 KeyPadBuff[KEYPAD_BUFF_SIZE];

static s32 DevKeypadGd = -2;//设备控制

/**
 *@brief:      dev_keypad_init
 *@details:    初始化矩阵按键IO口
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_keypad_init(void)
{
	/*
	c:PF8-PF11   当做输输入
	r:PF12-PF15  当做输出
	*/
	#ifdef SYS_USE_KEYPAD
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
 
	/* r */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* c */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_SetBits(GPIOF, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

	u8 i;
	for(i = 0; i< KEY_PAD_ROW_NUM; i++)
	{
		KeyPadCtrl[i].dec = 0;
		KeyPadCtrl[i].oldsta = KEYPAD_INIT_STA_MASK;
		KeyPadCtrl[i].newsta = KEYPAD_INIT_STA_MASK;
	}
	
	DevKeypadGd = -1;
	#else
	wjq_log(LOG_INFO, ">>>>>>keypad not init!\r\n");
	#endif
	
	return 0;
}

/**
 *@brief:      dev_keypad_open
 *@details:    打开按键，只有打开按键，扫描才会进行
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_keypad_open(void)
{
	if(DevKeypadGd == -1)
	{
		DevKeypadGd	 = 0;
	}
	else
		return -1;
	
	return 0;
}
/**
 *@brief:      dev_keypad_read
 *@details:    读键值
 *@param[in]   u8 *key  
               u8 len   期望读键值数量   
 *@param[out]  无
 *@retval:     返回读到键值数
 */
s32 dev_keypad_read(u8 *key, u8 len)
{
	u8 i =0;
	
	if(DevKeypadGd != 0)
		return -1;

	while(1)
	{
		if(KeyPadBuffR != KeyPadBuffW)
		{
			*(key+i) = KeyPadBuff[KeyPadBuffR];
			KeyPadBuffR++;
			if(KeyPadBuffR >= KEYPAD_BUFF_SIZE)
				KeyPadBuffR = 0;
		}
		else
			break;

		i++;
		if(i>= len)
			break;
	}

	return i;
}

/**
 *@brief:      dev_keypad_scan
 *@details:    按键扫描，在定时器或者任务中定时执行
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_keypad_scan(void)
{
	u16 ColSta;
	u8 chgbit;
	static u8 scanrow = 0;
	u8 keyvalue;
	
	if(DevKeypadGd != 0)
		return -1;
	
	/*读输入的状态，如果不是连续IO，先拼成连续IO*/
	ColSta = GPIO_ReadInputData(GPIOF);
	ColSta = (ColSta>>8)&KEYPAD_INIT_STA_MASK;

	/*记录新状态，新状态必须是连续稳定，否则重新计数*/
	if(ColSta != KeyPadCtrl[scanrow].newsta)
	{
		KeyPadCtrl[scanrow].newsta = ColSta;
		KeyPadCtrl[scanrow].dec = 0;
	}

	/*如新状态与旧状态有变化，进行扫描判断*/
	if(ColSta != KeyPadCtrl[scanrow].oldsta)
	{
		KEYPAD_DEBUG(LOG_DEBUG, " chg--");
		KeyPadCtrl[scanrow].dec++;
		if(KeyPadCtrl[scanrow].dec >= KEY_PAD_DEC_TIME)//大于防抖次数
		{
			/*确定有变化*/
			KeyPadCtrl[scanrow].dec = 0;
			/*新旧对比，找出变化位*/
			chgbit = KeyPadCtrl[scanrow].oldsta^KeyPadCtrl[scanrow].newsta;
			KEYPAD_DEBUG(LOG_DEBUG, "row:%d, chage bit:%02x\r\n",scanrow,chgbit);

			/*根据变化的位，求出变化的按键位置*/
			u8 i;
			for(i=0;i<KEY_PAD_COL_NUM;i++)
			{
				if((chgbit & (0x01<<i))!=0)
				{
					keyvalue = 	scanrow*KEY_PAD_COL_NUM+i;
					/*添加通断（按下松开）标志*/
					if((KeyPadCtrl[scanrow].newsta&(0x01<<i)) == 0)
					{
						KEYPAD_DEBUG(LOG_DEBUG, "press\r\n");
					}
					else
					{
						KEYPAD_DEBUG(LOG_DEBUG, "rel\r\n");
						keyvalue += KEYPAD_PR_MASK;
					}
					/**/
					KeyPadBuff[KeyPadBuffW] =keyvalue+1;//+1，调整到1开始，不从0开始
					KeyPadBuffW++;
					if(KeyPadBuffW>=KEYPAD_BUFF_SIZE)
						KeyPadBuffW = 0;
				}
			}
			
			KeyPadCtrl[scanrow].oldsta = KeyPadCtrl[scanrow].newsta;
			
		}
	}

	/*将下一行的IO输出0*/
	scanrow++;
	if(scanrow >= KEY_PAD_ROW_NUM)
		scanrow = 0;
	
	GPIO_SetBits(GPIOF, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	
	switch(scanrow)
	{
		case 0:
			GPIO_ResetBits(GPIOF, GPIO_Pin_12);
			break;
		case 1:
			GPIO_ResetBits(GPIOF, GPIO_Pin_13);
			break;
		case 2:
			GPIO_ResetBits(GPIOF, GPIO_Pin_14);
			break;
		case 3:
			GPIO_ResetBits(GPIOF, GPIO_Pin_15);
			break;
	}
	return 0;
}
/**
 *@brief:      dev_keypad_test
 *@details:    测试
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_keypad_test(void)
{
	u8 key;
	s32 res;
	res = dev_keypad_read(&key, 1);
	if(res == 1)
	{
		wjq_log(LOG_FUN, "get a key:%02x\r\n", key);
	}
	return 0;
}



