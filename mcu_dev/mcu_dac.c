/**
 * @file            mcu_dac.c
 * @brief           STM32片上DAC驱动
 * @author          test
 * @date            2017年11月1日 星期三
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年11月1日 星期三
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

//#define MCU_DAC_DEBUG

#ifdef MCU_DAC_DEBUG
#define MCU_DAC_DEBUG	wjq_log 
#else
#define MCU_DAC_DEBUG(a, ...)
#endif



s32 mcu_dac_init(void)
{
	return 0;
}
/**
 *@brief:      mcu_dac_open
 *@details:    打开DAC控制器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_dac_open(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    DAC_InitTypeDef DAC_InitType;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//----使能 PA 时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);//----使能 DAC 时钟
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//---模拟模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//---下拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);//---初始化 GPIO
    
    DAC_InitType.DAC_Trigger=DAC_Trigger_None;  //---不使用触发功能 TEN1=0
    DAC_InitType.DAC_WaveGeneration=DAC_WaveGeneration_None;   //---不使用波形发生
    DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude=DAC_LFSRUnmask_Bit0;
    DAC_InitType.DAC_OutputBuffer=DAC_OutputBuffer_Enable ;        //---输出缓存关闭
    //DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_4095; //噪声生成器
	DAC_Init(DAC_Channel_2,&DAC_InitType); //---初始化 DAC 通道 2    

	DAC_Cmd(DAC_Channel_2, ENABLE); //---使能 DAC 通道 2
    DAC_SetChannel2Data(DAC_Align_12b_R, 0); //---12 位右对齐数据格式   输出0 
		
		return 0;
}
/**
 *@brief:      mcu_dac_output
 *@details:    设置DAC输出值
 *@param[in]   u16 vol， 电压，单位MV，0-Vref  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_dac_output_vol(u16 vol)
{

    u32 temp;
    
    temp = (0xfff*vol)/3300;

    MCU_DAC_DEBUG(LOG_DEBUG, "\r\n---test dac data:%d-----\r\n", temp);
    
    DAC_SetChannel2Data(DAC_Align_12b_R, temp);//12 位右对齐数据格式
		return 0;
}
/**
 *@brief:      mcu_dac_output
 *@details:    将一个数值作为DAC值输出
 *@param[in]   u16 data  
 *@param[out]  无
 *@retval:     
 */
void mcu_dac_output(u16 data)
{
    DAC_SetChannel2Data(DAC_Align_12b_R, data);//12 位右对齐数据格式
}


/**
 *@brief:      mcu_dac_test
 *@details:    DAC测试程序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_dac_test(void)
{
    wjq_log(LOG_INFO, "\r\n---test dac!-----\r\n");
    
    mcu_dac_open();
    mcu_dac_output_vol(1500);//1.5v
    while(1);
}

