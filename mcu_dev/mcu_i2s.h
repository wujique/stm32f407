#ifndef _MCU_I2S_H_
#define _MCU_I2S_H_


extern void mcu_i2s_init (void);
extern void mcu_i2s_config(u32 AudioFreq, u16 Standard,u16 DataFormat);
extern void mcu_i2s_dma_init(u16 *buffer0,u16 *buffer1,u32 len);
extern void mcu_i2s_dma_start(void);
extern void mcu_i2s_dma_stop(void);
extern void mcu_i2s_dma_process(void);

#endif

