#ifndef __MCU_SPI_H_
#define __MCU_SPI_H_

#define SYS_USE_VSPI1

typedef enum{
	DEV_SPI_NULL = 0,
		
	DEV_SPI_3_1 = 0X31,//核心板上的SPI使用SPI3，定义为SPI_3_1
	DEV_SPI_3_2,	//底板板上的SPI使用SPI3，定义为SPI_3_2
	DEV_SPI_3_3,		//外扩的SPI定义为SPI_3_3

	DEV_VSPI_0 = 0XA0,	//空的，不用
	DEV_VSPI_1 = 0XA1, //触摸屏XPT2046方案，用原来的ADC方案管脚模拟SPI

}SPI_DEV;

typedef enum{
	SPI_MODE_0 =0,
	SPI_MODE_1,
	SPI_MODE_2,
	SPI_MODE_3,
	SPI_MODE_MAX
}SPI_MODE;

extern s32 mcu_spi_init(void);
extern s32 mcu_spi_open(SPI_DEV dev, SPI_MODE mode, u16 pre);
extern s32 mcu_spi_close(SPI_DEV dev);
extern s32 mcu_spi_transfer(SPI_DEV dev, u8 *snd, u8 *rsv, s32 len);
extern s32 mcu_spi_cs(SPI_DEV dev, u8 sta);


#endif

