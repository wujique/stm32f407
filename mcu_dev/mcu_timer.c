/**
 * @file            mcu_timer.c
 * @brief           CPU片上定时器驱动
 * @author          test
 * @date            2017年10月25日 星期三
 * @version         初稿
 * @par             
 * @par History:
 * 1.日    期:      2017年10月25日 星期三
 *   作    者:      屋脊雀工作室
 *   修改内容:      创建文件
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
#include "mcu_timer.h"

#define MCU_TIME_DEBUG

#ifdef MCU_TIME_DEBUG
#define TIME_DEBUG	wjq_log 
#else
#define TIME_DEBUG(a, ...)
#endif


void mcu_tim5_test(void);


#define TestTim TIM5
/*
    定时器时钟为84M,
    Tout=((SYSTEM_CLK_PERIOD)*(SYSTEM_CLK_PRESCALER))/Ft us.

	预分频,8400个时钟才触发一次定时器计数 
	那么一个定时器计数的时间就是(1/84M)*8400 = 100us	  
*/
#define SYSTEM_CLK_PRESCALER    8400                  
#define SYSTEM_CLK_PERIOD       10000//定时周期


void mcu_tim5_test(void);
/**
 *@brief:      mcu_timer_init
 *@details:    定时器初始化
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_timer_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    //打开定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    //复位定时器
    TIM_Cmd(TestTim, DISABLE);
    TIM_SetCounter(TestTim, 0);
    
    //设定TIM5中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      //响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInitStruct.TIM_Period = SYSTEM_CLK_PERIOD - 1;  //周期
    TIM_TimeBaseInitStruct.TIM_Prescaler = SYSTEM_CLK_PRESCALER-1;//分频
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 1;
    TIM_TimeBaseInit(TestTim, &TIM_TimeBaseInitStruct);
    
    TIM_ITConfig(TestTim, TIM_IT_Update, ENABLE);//打开定时器中断
    
    TIM_Cmd(TestTim, ENABLE);//使能定时器(启动)
		
		return 0;
}  
/**
 *@brief:      mcu_tim5_IRQhandler
 *@details:    定时器中断处理函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_tim5_IRQhandler(void)
{
    if(TIM_GetITStatus(TIM5, TIM_FLAG_Update) == SET)
    {                                       
        TIM_ClearFlag(TIM5, TIM_FLAG_Update);

        mcu_tim5_test();
 
    }
}
/**
 *@brief:      mcu_tim5_test
 *@details:    定时器测试
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_tim5_test(void)
{
    static u8 i = 0;

    i++;
    if(1 == i)
    {
        TIME_DEBUG(LOG_DEBUG, "tim int 1\r\n");    
    }
    else if(2 == i)
    {
        TIME_DEBUG(LOG_DEBUG, "tim int 2\r\n");
    }
    else if(3 == i)
    {
        TIME_DEBUG(LOG_DEBUG, "tim int 3\r\n");
    }
    else
    {
        TIME_DEBUG(LOG_DEBUG, "tim int 4\r\n");
        i = 0;   
    }
}

/*

		蜂鸣器用定时器4 PWM功能

*/


/**
 *@brief:      mcu_tim4_pwm_init
 *@details:    初始化定时器4PWM功能，默认配置的是CH2
 *@param[in]   u32 arr  定时周期  
               u32 psc  时钟预分频
 *@param[out]  无
 *@retval:     
 */
void mcu_tim4_pwm_init(u32 arr,u32 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);//---TIM4 时钟使能

    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_Prescaler = psc - 1; //---定时器分频
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //---向上计数模式
    TIM_TimeBaseStructure.TIM_Period= arr - 1; //---自动重装载值
    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);//---初始化定时器 4
    
    //----初始化 TIM4 PWM 模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //---PWM 调制模式 1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //---比较输出使能
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //---输出极性低

	/*默认配置的是通道2*/
    TIM_OC2Init(TIM4, &TIM_OCInitStructure); //---初始化外设 TIM4
    TIM_SetCompare2(TIM4, arr/2);//---占空比50%
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable); //---使能预装载寄存器
    TIM_ARRPreloadConfig(TIM4,ENABLE);

}

/*


	定时器捕获

*/
/**
 *@brief:      mcu_timer_cap_init
 *@details:    初始化定时器捕获，不使用中断
 *@param[in]   u32 arr  
               u16 psc  
 *@param[out]  无
 *@retval:     
 */
void mcu_timer_cap_init(u32 arr,u16 psc)
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM2_ICInitStructure;

	//初始化 TIM2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // 时钟使能
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //预分频器 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM 向上计数
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); // 初始化定时器 2

	//初始化通道 4
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_4; //选择输入端 IC4 映射到 TIM2
	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling; //下降沿捕获
	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1; //配置输入分频,不分频
	TIM2_ICInitStructure.TIM_ICFilter = 0x00;//配置输入滤波器 不滤波
	TIM_ICInit(TIM2, &TIM2_ICInitStructure);//初始化 TIM2 IC4

	TIM_ClearITPendingBit(TIM2, TIM_IT_CC4|TIM_IT_Update); //清除中断标志
	TIM_SetCounter(TIM2,0); 

	TIM_Cmd(TIM2,ENABLE); //使能定时器 2
}
/**
 *@brief:      mcu_timer_get_cap
 *@details:    查询获取定时去捕获值
 *@param[in]   void  
 *@param[out]  无
 *@retval:     捕获值，超时则返回最大值	
 */
u32 mcu_timer_get_cap(void)
{ 

	while(TIM_GetFlagStatus(TIM2, TIM_IT_CC4) == RESET)//等待捕获上升沿
	{
		if(TIM_GetCounter(TIM2) > 0xffffffff-1000)
			return TIM_GetCounter(TIM2);//超时了,直接返回 CNT 的值
	}
	return TIM_GetCapture4(TIM2);
}

/*

	定时器3，做DAC语音播放定时发送数据到DAC

*/

#define DacTim TIM3
#define TIM3_CLK_PRESCALER    84 //预分频,84个时钟才触发一次定时器计数 
                                    //一个定时器计数的时间就是(1/84M)*84 = 1us                       
#define TIM3_CLK_PERIOD       125//定时周期 125us

/**
 *@brief:      mcu_timer_init
 *@details:    定时器初始化
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_tim3_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    //打开定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    //复位定时器
    TIM_Cmd(DacTim, DISABLE);
    TIM_SetCounter(DacTim, 0);
    
    //设定TIM7中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      //响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInitStruct.TIM_Period = TIM3_CLK_PERIOD - 1;  //周期
    TIM_TimeBaseInitStruct.TIM_Prescaler = TIM3_CLK_PRESCALER-1;//分频
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 1;
    TIM_TimeBaseInit(DacTim, &TIM_TimeBaseInitStruct);
    
    TIM_ITConfig(DacTim, TIM_IT_Update, ENABLE);//打开定时器中断

}  

s32 mcu_tim3_start(void)
{
	 TIM_Cmd(DacTim, ENABLE);//使能定时器(启动)	
	return 0;
}

s32 mcu_tim3_stop(void)
{
	TIM_Cmd(DacTim, DISABLE);//停止定时器	
	return 0;
}

extern s32 dev_dacsound_timerinit(void);
/**
 *@brief:      mcu_tim6_IRQhandler
 *@details:    定时器中断处理函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_tim3_IRQhandler(void)
{
    if(TIM_GetITStatus(DacTim, TIM_FLAG_Update) == SET)
    {                                       
        TIM_ClearFlag(DacTim, TIM_FLAG_Update);

		dev_dacsound_timerinit();

    }
}

/*

	使用内部ADC转换检测触摸屏，
	用定时器做精确延时

*/
void (*Tim7Callback)(void);
u8 Tim7Type = 1;//1，执行一次，不重复，0重复

#define TpTim TIM7
#define TIM7_CLK_PRESCALER    840 //预分频,840个时钟才触发一次定时器计数 
                                    //一个定时器计数的时间就是(1/84M)*840 = 10us                       
#define TIM7_CLK_PERIOD       100//定时周期 1毫秒

/**
 *@brief:      mcu_timer_init
 *@details:    定时器初始化
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_timer7_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    //打开定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    //复位定时器
    TIM_Cmd(TpTim, DISABLE);
    TIM_SetCounter(TpTim, 0);
    
    //设定TIM7中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      //响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
    TIM_ITConfig(TpTim, TIM_IT_Update, ENABLE);//打开定时器中断
    
    //TIM_Cmd(TpTim, ENABLE);//使能定时器(启动)
	Tim7Callback = NULL;
}  

s32 mcu_tim7_start(u32 Delay_10us, void (*callback)(void), u8 type)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

	Tim7Callback = callback;
	Tim7Type = type;
	//复位定时器
    TIM_Cmd(TpTim, DISABLE);
    TIM_SetCounter(TpTim, 0);

	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInitStruct.TIM_Period = Delay_10us - 1;  //周期
    TIM_TimeBaseInitStruct.TIM_Prescaler = TIM7_CLK_PRESCALER-1;//分频
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 1;
    TIM_TimeBaseInit(TpTim, &TIM_TimeBaseInitStruct);

	TIM_ClearFlag(TpTim, TIM_FLAG_Update);
	
	TIM_ITConfig(TpTim, TIM_IT_Update, ENABLE);//打开定时器中断

	TIM_Cmd(TpTim, ENABLE);//使能定时器(启动)	
	
	return 0;
}
/**
 *@brief:      mcu_tim6_IRQhandler
 *@details:    定时器中断处理函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_tim7_IRQhandler(void)
{
    if(TIM_GetITStatus(TpTim, TIM_FLAG_Update) == SET)
    {                                       
        TIM_ClearFlag(TpTim, TIM_FLAG_Update);
		if(Tim7Type == 1)
			TIM_Cmd(TpTim, DISABLE);//停止定时器
			
		Tim7Callback();
		
    }
}

