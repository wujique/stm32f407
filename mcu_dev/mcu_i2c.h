#ifndef __MCU_I2C_H__
#define __MCU_I2C_H__

#include "wujique_sysconf.h"

typedef enum{
	DEV_VI2C_NULL = 0,
		
	DEV_VI2C_1,//WM8978、TEA5767、外扩I2C
	
	#ifdef SYS_USE_VI2C2
	DEV_VI2C_2,	//外扩IO模拟，跟VSPI2，矩阵按键共用
	#endif
	
}VI2C_DEV;


#define MCU_I2C_MODE_W 0
#define MCU_I2C_MODE_R 1


extern s32 mcu_i2c_init(void);
extern s32 mcu_i2c_transfer(VI2C_DEV dev, u8 addr, u8 rw, u8* data, s32 datalen);

extern void SCCB_GPIO_Config(void);
extern uint8_t bus_sccb_writereg(uint8_t DeviceAddr, uint16_t Addr, uint8_t Data);
extern uint8_t bus_sccb_readreg(uint8_t DeviceAddr, uint16_t Addr);

#endif
