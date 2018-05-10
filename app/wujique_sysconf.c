/**
 * @file            wujique_sysconf.c
 * @brief           系统管理
 * @author          wujique
 * @date            2018年4月9日 星期一
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年4月9日 星期一
 *   作    者:         wujique
 *   修改内容:   创建文件
*/
#include "stm32f4xx.h"
#include "wujique_sysconf.h"
#include "mcu_i2c.h"
#include "mcu_spi.h"

#include "dev_spiflash.h"
#include "dev_lcdbus.h"

#include "dev_lcd.h"

/*

	本文件用于配置系统有哪些设备
	1.最底层，按照STM32设计，用在其他CPU需要移植。

*/

/*

	IO口模拟的I2C1
	WM8978、TEA5767、外扩I2C使用

*/
#define MCU_I2C1_PORT GPIOD
#define MCU_I2C1_SCL GPIO_Pin_6
#define MCU_I2C1_SDA GPIO_Pin_7
#define MCU_I2C1_RCC RCC_AHB1Periph_GPIOD

DevI2c DevVi2c1={
		.name = "VI2C1",
		
		.sclrcc = MCU_I2C1_RCC,
		.sclport = MCU_I2C1_PORT,
		.sclpin = MCU_I2C1_SCL,

		.sdarcc = MCU_I2C1_RCC,
		.sdaport = MCU_I2C1_PORT,
		.sdapin = MCU_I2C1_SDA,
		};
	
/*

	外扩IO口模拟I2C，和矩阵按键，模拟SPI冲突

*/		
#define MCU_I2C2_PORT GPIOF
#define MCU_I2C2_SCL GPIO_Pin_11
#define MCU_I2C2_SDA GPIO_Pin_10
#define MCU_I2C2_RCC RCC_AHB1Periph_GPIOF

DevI2c DevVi2c2={
		.name = "VI2C2",
		
		.sclrcc = MCU_I2C2_RCC,
		.sclport = MCU_I2C2_PORT,
		.sclpin = MCU_I2C2_SCL,

		.sdarcc = MCU_I2C2_RCC,
		.sdaport = MCU_I2C2_PORT,
		.sdapin = MCU_I2C2_SDA,
		};	
/*

	IO口模拟SPI控制器

*/
/*
	VSPI1，使用触摸屏四线接口模拟SPI，用于XPT2046方案触摸处理，可读可写。
*/					
DevSpi DevVSpi1IO={
		"VSPI1",
		DEV_SPI_V,
		
		/*clk*/
		RCC_AHB1Periph_GPIOB,
		GPIOB,
		GPIO_Pin_0,
		/*mosi*/
		RCC_AHB1Periph_GPIOD,
		GPIOD,
		GPIO_Pin_11,
		/*miso*/
		RCC_AHB1Periph_GPIOD,
		GPIOD,
		GPIO_Pin_12,
	};
		
/*  外扩接口模拟VSPI2， 与矩阵键盘，模拟I2C2冲突    */						
DevSpi DevVspi2IO={
		"VSPI2",
		DEV_SPI_V,
		
		RCC_AHB1Periph_GPIOF,
		GPIOF,
		GPIO_Pin_11,
		
		RCC_AHB1Periph_GPIOF,
		GPIOF,
		GPIO_Pin_10,

		RCC_AHB1Periph_GPIOF,
		GPIOF,
		GPIO_Pin_9,

	};

/*

	硬件SPI控制器：SPI3
	SPI驱动暂时支持SPI3，
	如果添加其他控制器，请修改mcu_spi.c中的硬件SPI控制器初始化代码
	*/
DevSpi DevSpi3IO={
		"SPI3",
		DEV_SPI_H,
		
		RCC_AHB1Periph_GPIOB,
		GPIOB,
		GPIO_Pin_11,
		
		RCC_AHB1Periph_GPIOB,
		GPIOB,
		GPIO_Pin_10,

		RCC_AHB1Periph_GPIOB,
		GPIOB,
		GPIO_Pin_9,
	};

/* SPI通道*/
/* FLASH*/
DevSpiCh DevSpi3CH1={
		"SPI3_CH1",
		"SPI3",
		
		RCC_AHB1Periph_GPIOB,
		GPIOB,
		GPIO_Pin_14,
		
	};
		
DevSpiCh DevSpi3CH2={
		"SPI3_CH2",
		"SPI3",
		
		RCC_AHB1Periph_GPIOG,
		GPIOG,
		GPIO_Pin_15,
		
	};
/*外扩SPI*/		
DevSpiCh DevSpi3CH3={
		"SPI3_CH3",
		"SPI3",
		
		RCC_AHB1Periph_GPIOG,
		GPIOG,
		GPIO_Pin_6,
		
	};
/* 触摸屏IO模拟SPI*/
DevSpiCh DevVSpi1CH1={
		"VSPI1_CH1",
		"VSPI1",
		
		RCC_AHB1Periph_GPIOB,
		GPIOB,
		GPIO_Pin_1,
		
	};
/*外扩IO*/
DevSpiCh DevVSpi2CH1={
		"VSPI2_CH1",
		"VSPI2",
		
		RCC_AHB1Periph_GPIOF,
		GPIOF,
		GPIO_Pin_12,
		
	};
	
/*
	串行LCD接口，使用真正的SPI控制

*/
DevLcdBus BusLcdSpi3={
	.name = "BusLcdSpi3",
	.type = LCD_BUS_SPI,
	.basebus = "SPI3_CH3",

	.A0rcc = RCC_AHB1Periph_GPIOG,
	.A0port = GPIOG,
	.A0pin = GPIO_Pin_4,

	.rstrcc = RCC_AHB1Periph_GPIOG,
	.rstport = GPIOG,
	.rstpin = GPIO_Pin_7,

	.blrcc = RCC_AHB1Periph_GPIOG,
	.blport = GPIOG,
	.blpin = GPIO_Pin_9,
};


DevLcdBus BusLcdI2C1={
	.name = "BusLcdI2C1",
	.type = LCD_BUS_I2C,
	.basebus = "VI2C1",

	/*I2C接口的LCD总线，不需要其他IO*/

};
DevLcdBus BusLcd8080={
	.name = "BusLcd8080",
	.type = LCD_BUS_8080,
	.basebus = "8080",//不适用，8080操作直接嵌入在LCD BUS代码内

	/*8080 不用A0脚，填背光进去*/
	.A0rcc = RCC_AHB1Periph_GPIOB,
	.A0port = GPIOB,
	.A0pin = GPIO_Pin_15,

	.rstrcc = RCC_AHB1Periph_GPIOA,
	.rstport = GPIOA,
	.rstpin = GPIO_Pin_15,

	.blrcc = RCC_AHB1Periph_GPIOB,
	.blport = GPIOB,
	.blpin = GPIO_Pin_15,

};

DevLcdBus BusLcdVSpi2CH1={
	.name = "BusLcdVSpi2CH1",
	.type = LCD_BUS_SPI,
	.basebus = "VSPI2_CH1",

	.A0rcc = RCC_AHB1Periph_GPIOF,
	.A0port = GPIOF,
	.A0pin = GPIO_Pin_8,

	.rstrcc = RCC_AHB1Periph_GPIOF,
	.rstport = GPIOF,
	.rstpin = GPIO_Pin_13,

	.blrcc = RCC_AHB1Periph_GPIOF,
	.blport = GPIOF,
	.blpin = GPIO_Pin_14,
};


DevSpiFlash DevSpiFlashCore={
	/*有一个叫做board_spiflash的SPI FLASH挂在DEV_SPI_3_2上，型号未知*/
	"board_spiflash", 
	"SPI3_CH2", 
	NULL
};

DevSpiFlash DevSpiFlashBoard={
	"core_spiflash",  
	"SPI3_CH1", 
	NULL
};

/*
	设备树定义
	指明系统有多少个LCD设备，挂在哪个LCD总线上。
*/
DevLcd DevLcdOled1	=	{"i2coledlcd",  "BusLcdI2C1",  0X1315};
//LcdObj DevLcdOled2	=	{"i2coledlcd2", LCD_BUS_VI2C2,  0X1315};
//LcdObj DevLcdOled3	=	{"vspioledlcd", LCD_BUS_VSPI, 	0X1315};
DevLcd DevLcdOled4	=	{"spioledlcd", 	"BusLcdSpi3", 	0X1315};
DevLcd DevLcdCOG1	=	{"spicoglcd", 	"BusLcdSpi3", 	0X7565};
//LcdObj DevLcdCOG2	=	{"vspicoglcd", 	LCD_BUS_VSPI, 	0X7565};
DevLcd DevLcdtTFT	=	{"tftlcd", 		"BusLcd8080", 	NULL};


s32 sys_dev_init(void)
{
	/*注册I2C总线*/
	mcu_i2c_register(&DevVi2c1);

	#ifdef SYS_USE_VI2C2
	//mcu_i2c_register(&DevVi2c2);
	#endif
	
	/*注册SPI控制器*/
	mcu_spi_register(&DevSpi3IO);
	
	#ifdef SYS_USE_VSPI1
	mcu_spi_register(&DevVSpi1IO);
	#endif

	#ifdef SYS_USE_VSPI2
	mcu_spi_register(&DevVspi2IO);
	#endif
	
	/*注册SPI 通道*/
	mcu_spich_register(&DevSpi3CH1);
	mcu_spich_register(&DevSpi3CH2);
	mcu_spich_register(&DevSpi3CH3);
	
	mcu_spich_register(&DevVSpi1CH1);

	mcu_spich_register(&DevVSpi2CH1);
	
	/*注册LCD总线*/
	dev_lcdbus_register(&BusLcdSpi3);
	dev_lcdbus_register(&BusLcdI2C1);
	dev_lcdbus_register(&BusLcd8080);
	dev_lcdbus_register(&BusLcdVSpi2CH1);
	/*
		注册设备

	*/
	/*注册FLASH设备*/
	dev_spiflash_register(&DevSpiFlashCore);
	dev_spiflash_register(&DevSpiFlashBoard);
	/*注册LCD设备*/
	dev_lcd_register(&DevLcdOled1);
	dev_lcd_register(&DevLcdtTFT);
	dev_lcd_register(&DevLcdCOG1);
}



