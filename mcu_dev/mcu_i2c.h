#ifndef __MCU_I2C_H__
#define __MCU_I2C_H__

#include "list.h"
#include "mcu_bsp.h"

/*
	i2c设备定义
*/
#define MCU_DEV_I2C_NAME_SIZE	16//名字长度

typedef struct
{
	/*设备名称*/
	char name[MCU_DEV_I2C_NAME_SIZE];

	/*设备需要的资源，模拟I2C只需要两根IO口*/
	MCU_PORT sclport;
	u16 sclpin;

	MCU_PORT sdaport;
	u16 sdapin;
}DevI2c;

/*
	

*/
typedef struct
{

	s32 gd;
	DevI2c dev;	

	struct list_head list;
}DevI2cNode;


#define MCU_I2C_MODE_W 0
#define MCU_I2C_MODE_R 1


extern s32 mcu_i2c_init(void);
extern s32 mcu_i2c_transfer(DevI2cNode * node, u8 addr, u8 rw, u8* data, s32 datalen);
extern DevI2cNode *mcu_i2c_open(char *name);
extern s32 mcu_i2c_close(DevI2cNode * node);
extern void SCCB_GPIO_Config(void);
extern uint8_t bus_sccb_writereg(uint8_t DeviceAddr, uint16_t Addr, uint8_t Data);
extern uint8_t bus_sccb_readreg(uint8_t DeviceAddr, uint16_t Addr);

#endif
