#ifndef __MCU_I2C_H__
#define __MCU_I2C_H__


#define MCU_I2C_MODE_W 0
#define MCU_I2C_MODE_R 1


extern s32 mcu_i2c_init(void);
extern s32 mcu_i2c_transfer(u8 addr, u8 rw, u8* data, s32 datalen);

extern void SCCB_GPIO_Config(void);
extern uint8_t bus_sccb_writereg(uint8_t DeviceAddr, uint16_t Addr, uint8_t Data);
extern uint8_t bus_sccb_readreg(uint8_t DeviceAddr, uint16_t Addr);

#endif
