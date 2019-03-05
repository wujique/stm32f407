/**
 * @file            mcu_bsp_stm32.c
 * @brief           adc驱动
 * @author          wujique
 * @date            2017年12月8日 星期五
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年12月8日 星期五
 *   作    者:         wujique
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
	封装IO操作，以便移植到其他芯片
*/
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "mcu_bsp.h"

const GPIO_TypeDef *Stm32PortList[16] = {NULL, GPIOA,  GPIOB, GPIOC, GPIOD,
									GPIOE, GPIOF, GPIOG, GPIOH,
									GPIOI, GPIOJ, GPIOK, NULL};

void mcu_io_config_in(MCU_PORT port, u16 pin)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	
	if(port == NULL)
		return;
	
    GPIO_InitStructure.GPIO_Pin = pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init((GPIO_TypeDef *)Stm32PortList[port], &GPIO_InitStructure);//初始化  	
}


void mcu_io_config_out(MCU_PORT port, u16 pin)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	if(port == NULL)
		return;
	
	GPIO_InitStructure.GPIO_Pin = pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//上拉
	GPIO_Init((GPIO_TypeDef *)Stm32PortList[port], &GPIO_InitStructure);//初始化
}

void mcu_io_output_setbit(MCU_PORT port, u16 pin)
{
	if(port == NULL)
		return;
	
	GPIO_SetBits((GPIO_TypeDef *)Stm32PortList[port], pin);
}

void mcu_io_output_resetbit(MCU_PORT port, u16 pin)
{
	if(port == NULL)
		return;
	
	GPIO_ResetBits((GPIO_TypeDef *)Stm32PortList[port], pin);
}		

u8 mcu_io_input_readbit(MCU_PORT port, u16 pin)
{
	if(port == NULL)
		return Bit_SET;
	
	return GPIO_ReadInputDataBit((GPIO_TypeDef *)Stm32PortList[port], pin);
}




