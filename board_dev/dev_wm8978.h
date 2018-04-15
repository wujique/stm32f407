/**
 * @file            dev_wm8978.h
 * @brief           wm8978设备对外定义
 * @author          wujique
 * @date            2018年1月7日 星期日
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年1月7日 星期日
 *   作    者:         wujique
 *   修改内容:   创建文件
*/
#ifndef _DEV_WM8978_H_
#define _DEV_WM8978_H_

#define 	WM8978_I2S_LSB         0x00	
#define 	WM8978_I2S_MSB         0x01
#define 	WM8978_I2S_Phillips    0x02
#define 	WM8978_I2S_PCM         0x03
	
#define 	WM8978_I2S_Data_16b    0x00
#define 	WM8978_I2S_Data_20b    0x01
#define 	WM8978_I2S_Data_24b    0x02
#define 	WM8978_I2S_Data_32b    0x03

typedef enum{
	WM8978_INPUT_NULL=0x00,
	WM8978_INPUT_LMIC = 0x01,
	WM8978_INPUT_RMIC = 0x02,
	WM8978_INPUT_LINE = 0x04,
	WM8978_INPUT_AUX = 0x08,
	WM8978_INPUT_DAC = 0x10,
	WM8978_INPUT_ADC = 0x20,
}WM8978_INIPUT;

typedef enum{
	WM8978_OUTPUT_NULL = 0x00,
	WM8978_OUTPUT_PHONE = 0x01,
	WM8978_OUTPUT_SPK	= 0x02,
	WM8978_OUTPUT_LINE  = 0X04,
}WM8978_OUTPUT;


extern s32 dev_wm8978_init(void);
extern s32 dev_wm8978_open(void);
extern s32 dev_wm8978_dataformat(u32 Freq, u8 Standard, u8 Format);
extern s32 dev_wm8978_transfer(u8 onoff);
extern s32 dev_wm8978_test(void);

#endif

