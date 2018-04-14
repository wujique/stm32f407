/**
 * @file            dev_buzzer.c
 * @brief           PWM蜂鸣器驱动
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

/*
    定时器时钟为84M,
    Tout=((SYSTEM_CLK_PERIOD)*(SYSTEM_CLK_PRESCALER))/Ft us.
*/
#define BUZZER_CLK_PRESCALER    84 //预分频,84个时钟触发一次定时器计数 
                                    //一个定时器计数的时间就是(1/84M)*84 = 1us                       
#define BUZZER_CLK_PERIOD       250//定时周期，250*1=250us, 频率正好是4K，蜂鸣器的响应频率。

/**
 *@brief:      dev_buzzer_init
 *@details:    初始化蜂鸣器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_buzzer_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); //---使能 GPIOD 时钟
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_TIM4); //---管脚复用为 TIM4功能
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //---复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //---速度 50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //---推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //---上拉
    GPIO_Init(GPIOD,&GPIO_InitStructure);
	
    mcu_tim4_pwm_init(BUZZER_CLK_PERIOD,BUZZER_CLK_PRESCALER);
	
	return 0;
}
/**
 *@brief:      dev_buzzer_open
 *@details:    打开蜂鸣器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_buzzer_open(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); //---使能 GPIOD 时钟
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_TIM4); //---管脚复用为 TIM4功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //---复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //---速度 50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //---推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //---上拉
    GPIO_Init(GPIOD,&GPIO_InitStructure);
	
    TIM_Cmd(TIM4, ENABLE); //---使能 TIM4 
	
	return 0;
}
/**
 *@brief:      dev_buzzer_close
 *@details:    关闭蜂鸣器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_buzzer_close(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
    TIM_Cmd(TIM4, DISABLE); //---关闭定时器 TIM4 

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); //---使能 GPIOD 时钟

	/*关闭蜂鸣器时，要将IO改为普通IO，并且输出低电平，否则蜂鸣器会造成大电流*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //---复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //---速度 50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //---推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //---上拉
    GPIO_Init(GPIOD,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOD, GPIO_Pin_13);
	
	return 0;
}
/**
 *@brief:      dev_buzzer_test
 *@details:    测试蜂鸣器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void dev_buzzer_test(void)
{
    
}

