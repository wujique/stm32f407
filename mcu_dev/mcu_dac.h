#ifndef _MCU_DAC_H_
#define _MCU_DAC_H_

extern s32 mcu_dac_init(void);
/**
 *@brief:      mcu_dac_open
 *@details:    打开DAC控制器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
extern s32 mcu_dac_open(void);
/**
 *@brief:      mcu_dac_output
 *@details:    设置DAC输出值
 *@param[in]   u16 vol， 电压，单位MV，0-Vref  
 *@param[out]  无
 *@retval:     
 */
extern s32 mcu_dac_output_vol(u16 vol);
/**
 *@brief:      mcu_dac_output
 *@details:    将一个数值作为DAC值输出
 *@param[in]   u16 data  
 *@param[out]  无
 *@retval:     
 */
extern s32 mcu_dac_output(u16 data);
/**
 *@brief:      mcu_dac_test
 *@details:    DAC测试程序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
extern s32 mcu_dac_test(void);

#endif

