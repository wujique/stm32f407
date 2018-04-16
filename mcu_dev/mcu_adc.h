#ifndef __MCU_ADC_H__
#define __MCU_ADC_H__


/*定义则使用中断方式*/
#define MCU_ADC_IRQ 1


extern void mcu_adc_init(void);
extern u16 mcu_adc_get_conv(u8 ch);
extern s32 mcu_adc_start_conv(u8 ch);


#endif
