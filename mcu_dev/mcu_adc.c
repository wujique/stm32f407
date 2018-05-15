/**
 * @file            mcu_adc.c
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
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "mcu_adc.h"

//#define MCU_ADC_DEBUG_G

#ifdef MCU_ADC_DEBUG_G
#define MCU_ADC_DEBUG	wjq_log 
#else
#define MCU_ADC_DEBUG(a, ...)
#endif

extern s32 dev_ts_adc_task(u16 dac_value);

/**
 *@brief:      mcu_adc_init
 *@details:    adc初始化，用于电阻触摸屏检测
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
void mcu_adc_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使用GPIOB时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//---模拟模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//---不上下拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//---初始化 GPIO

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);//使能ADC 时钟

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = 	ADC_TwoSamplingDelay_20Cycles;//两个采样阶段之间的延迟 5 个时钟
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA 失能
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;//预分频 6分频。
	ADC_CommonInit(&ADC_CommonInitStructure);//初始化

	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12 位模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//非连续转换
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;//本值是触发源，我们已经禁止触发，因此本值无意义
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
	ADC_InitStructure.ADC_NbrOfConversion = 1;//1 个转换在规则序列中, 也就是说一次转换一个通道
	ADC_Init(ADC2, &ADC_InitStructure);//ADC 初始化

	#ifdef MCU_ADC_IRQ
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      //响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	ADC_ITConfig(ADC2,  ADC_IT_EOC,  ENABLE);//打开ADC EOC中断
	ADC_ClearFlag(ADC2, ADC_FLAG_EOC);
	#endif
	
	ADC_Cmd(ADC2, ENABLE);	
}
/**
 *@brief:      mcu_adc_get_conv
 *@details:    查询模式进行ADC转换
 *@param[in]   u8 ch  
 *@param[out]  无
 *@retval:     
 */
u16 mcu_adc_get_conv(u8 ch)
{
	u16 adcvalue;
	FlagStatus ret;
	
	//设置指定 ADC 的规则组通道，一个序列，采样时间
	MCU_ADC_DEBUG(LOG_DEBUG, "str--");
	ADC_ClearFlag(ADC2, ADC_FLAG_OVR);

	ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_480Cycles ); 
	ADC_SoftwareStartConv(ADC2); 	//使能指定的 ADC 的软件转换启动功能
	
	while(1)//等待转换结束
	{
		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_EOC\r\n");
			break;
		}
		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_AWD);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_AWD\r\n");

		}
		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_JEOC);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_JEOC\r\n");

		}
		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_JSTRT);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_JSTRT\r\n");

		}

		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_STRT);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_STRT\r\n");

		}

		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_OVR);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_OVR\r\n");

		}
	}
	adcvalue = ADC_GetConversionValue(ADC2);
	return adcvalue;
}

/**
 *@brief:      mcu_adc_start_conv
 *@details:    启动ADC转换
 *@param[in]   u8 ch  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_adc_start_conv(u8 ch)
{	
	ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_480Cycles ); 
	ADC_SoftwareStartConv(ADC2); 	//使能指定的 ADC 的软件转换启动功能
	return 0;
}
/**
 *@brief:      mcu_adc_IRQhandler
 *@details:    ADC中断服务程序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_adc_IRQhandler(void)
{
	volatile u16 adc_value;
	FlagStatus ret;
	ITStatus itret;
	
	itret = ADC_GetITStatus(ADC2, ADC_IT_EOC);
	if( itret == SET)
    {    

		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC);
		if(ret == SET)
		{
			//uart_printf("ADC_FLAG_EOC t\r\n");
			adc_value = ADC_GetConversionValue(ADC2);
			//MCU_ADC_DEBUG(LOG_DEBUG, "%d ", adc_value);
			
			dev_ts_adc_task(adc_value);

		}
		
		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_AWD);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_AWD t\r\n");

		}
		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_JEOC);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_JEOC t\r\n");

		}
		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_JSTRT);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_JSTRT t\r\n");

		}

		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_STRT);
		if(ret == SET)
		{
			//uart_printf("ADC_FLAG_STRT t\r\n");

		}

		ret = ADC_GetFlagStatus(ADC2, ADC_FLAG_OVR);
		if(ret == SET)
		{
			MCU_ADC_DEBUG(LOG_DEBUG, "ADC_FLAG_OVR t\r\n");
			ADC_ClearFlag(ADC2, ADC_FLAG_OVR);
		}
		
		ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);
    }	
}

/*

	adc 测试，只要能跑通，不死机即可。
	值是否准确，在触摸屏里面测试

*/
extern void Delay(__IO uint32_t nTime);

s32 mcu_adc_test(void)
{
	mcu_adc_init();
	
#ifndef MCU_ADC_IRQ/*查询模式*/
	u16 adc_value;

	wjq_log(LOG_FUN, "mcu_adc_test check\r\n");

	while(1)
	{
		adc_value = mcu_adc_get_conv(ADC_Channel_8);
	
		wjq_log(LOG_FUN, "ADC_Channel_8:%d\r\n", adc_value);
		Delay(1000);
		
		adc_value = mcu_adc_get_conv(ADC_Channel_9);
		wjq_log(LOG_FUN, "ADC_Channel_9:%d\r\n", adc_value);
		Delay(1000);
	}
#else/*中断模式*/
	wjq_log(LOG_FUN, "mcu_adc_test int\r\n");

	while(1)
	{		
		wjq_log(LOG_FUN, "r ");
		mcu_adc_start_conv(ADC_Channel_8);
		Delay(1000);
		wjq_log(LOG_FUN, "d ");
		mcu_adc_start_conv(ADC_Channel_9);
		Delay(1000);
	}

#endif
}



/*

	测试CPU温度	

*/
void  mcu_adc_temprate_init(void)
{	 
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);//使能ADC1时钟

	//先初始化IO口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;// 上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化  

	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);	//ADC1复位
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE);	//复位结束	 

	ADC_TempSensorVrefintCmd(ENABLE);//使能内部温度传感器

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4; //ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐	
	ADC_InitStructure.ADC_NbrOfConversion = 1;//1个转换在规则序列中 也就是只转换规则序列1 
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_480Cycles );	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_480Cycles );		
	ADC_Cmd(ADC1, ENABLE);//开启AD转换器	 	

}				  

u16 mcu_tempreate_get_adc_value(void)	 
{
	ADC_ClearFlag(ADC2, ADC_FLAG_OVR);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_480Cycles );				
	ADC_SoftwareStartConv(ADC1);		//使能指定的ADC1的软件转换启动功能	
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u32 mcu_tempreate_get_tempreate(void)
{
	u32 adcx;

	adcx=mcu_tempreate_get_adc_value();	
	adcx = adcx*4125/128-27900;			
	//wjq_log(LOG_DEBUG, "%d-\r\n", adcx);
	
	return adcx;
}


