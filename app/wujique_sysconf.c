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
/*-------------------------------
	I2C控制器
-------------------------------*/
/*
	IO口模拟的I2C1
	WM8978、TEA5767、外扩接口的I2C使用
*/
const DevI2c DevVi2c1={
		.name = "VI2C1",
		
		.sclport = MCU_PORT_D,
		.sclpin = GPIO_Pin_6,

		.sdaport = MCU_PORT_D,
		.sdapin = GPIO_Pin_7,
		};
	
		
#if 0
/*
	外扩IO口模拟I2C，和矩阵按键，模拟SPI冲突
*/
const DevI2c DevVi2c2={
		.name = "VI2C2",
		
		.sclport = MCU_PORT_F,
		.sclpin = GPIO_Pin_11,

		.sdaport = MCU_PORT_F,
		.sdapin = GPIO_Pin_10,
		};	
#endif
/*----------------------
	IO口模拟SPI控制器
------------------------*/
/*
	VSPI1，核心板上的LCD接口中的4根IO模拟SPI，
	用于XPT2046方案触摸处理，可读可写。
*/					
const DevSpi DevVSpi1IO={
		.name = "VSPI1",
		.type = DEV_SPI_V,
		
		/*clk*/
		.clkport = MCU_PORT_B,
		.clkpin = GPIO_Pin_0,
		/*mosi*/
		.mosiport = MCU_PORT_D,
		.mosipin = GPIO_Pin_11,
		/*miso*/
		.misoport = MCU_PORT_D,
		.misopin = GPIO_Pin_12,
	};

#if 1
/*
	模拟SPI，无miso
	用于测试SPI程序框架
*/
const DevSpi DevVspi3IO={
		.name = "VSPI3",
		.type = DEV_SPI_V,
		
		/*clk*/
		.clkport = MCU_PORT_G,
		.clkpin = GPIO_Pin_6,
		
		/*mosi*/
		.mosiport = MCU_PORT_F,
		.mosipin = GPIO_Pin_2,

		/*miso*/
		.misoport = NULL,
		.misopin = NULL,

	};
#endif

#if 1
/*  
	外扩接口模拟VSPI2， 与矩阵键盘，模拟I2C2冲突  
*/			
const DevSpi DevVspi2IO={
		.name = "VSPI2",
		.type = DEV_SPI_V,
		
		/*clk*/
		.clkport = MCU_PORT_F,
		.clkpin = GPIO_Pin_11,
		
		/*mosi*/
		.mosiport = MCU_PORT_F,
		.mosipin = GPIO_Pin_10,

		/*miso*/
		.misoport = MCU_PORT_F,
		.misopin = GPIO_Pin_9,

	};
#endif
/*
	硬件SPI控制器：SPI3
	SPI驱动暂时支持SPI3，
	如果添加其他控制器，请修改mcu_spi.c中的硬件SPI控制器初始化代码
*/
const DevSpi DevSpi3IO={
		.name = "SPI3",
		.type = DEV_SPI_H,
		
		/*clk*/
		.clkport = MCU_PORT_B,
		.clkpin = GPIO_Pin_3,
		
		/*mosi*/
		.mosiport = MCU_PORT_B,
		.mosipin = GPIO_Pin_5,

		/*miso*/
		.misoport = MCU_PORT_B,
		.misopin = GPIO_Pin_4,
	};

/*------------------------ 
	SPI通道
-------------------------*/
/*
	FLASH用
*/
const DevSpiCh DevSpi3CH1={
		.name = "SPI3_CH1",
		.spi = "SPI3",
		
		.csport = MCU_PORT_B,
		.cspin = GPIO_Pin_14,
		
	};		
/*
	FLASH用
*/
const DevSpiCh DevSpi3CH2={
		.name = "SPI3_CH2",
		.spi = "SPI3",
		
		.csport = MCU_PORT_G,
		.cspin = GPIO_Pin_15,
		
	};
/*
	外扩接口的SPI，可接COG、OLED、SPI TFT、RF24L01
*/			
const DevSpiCh DevSpi3CH3={
		.name = "SPI3_CH3",
		.spi = "SPI3",
		
		.csport = MCU_PORT_G,
		.cspin = GPIO_Pin_6,
		
	};
		
#if 0		
/*
	外扩接口的SPI, 
	跟DevSpi3CH3用相同的IO，
	主要是接SPI接口的LCD,
	同时带触摸屏XPT2046，
	本通道就是控制XPT2046的。

	但是实际上，SPI接口的TFT 2.7寸LCD太慢了，
	只能用来测试程序框架。
*/
const DevSpiCh DevSpi3CH4={
		.name = "SPI3_CH4",
		.spi = "SPI3",
		
		.csport = MCU_PORT_F,
		.cspin = GPIO_Pin_2,
		
	};
#endif

#if 1
/*
	模拟SPI通道，无CS
	用来调试没有CS的LCD屏幕，
	VSPI3，其实是外扩接口SPI附近的两个IO模拟的。
*/	
const DevSpiCh DevVSpi3CH1={
		.name = "VSPI3_CH1",
		.spi = "VSPI3",
		
		.csport = NULL,
		.cspin = NULL,
		
	};
#endif

/* 
	LCD座子中的触摸屏接口, IO模拟SPI
*/
const DevSpiCh DevVSpi1CH1={
		.name = "VSPI1_CH1",
		.spi = "VSPI1",
		
		.csport = MCU_PORT_B,
		.cspin = GPIO_Pin_1,
		
	};
#if 0		
/* 
	SPI彩屏，跟触摸屏用相同的控制器
*/		
const DevSpiCh DevVSpi1CH2={
		.name = "VSPI1_CH2",
		.spi = "VSPI1",
		
		.csport = MCU_PORT_D,
		.cspin = GPIO_Pin_14,
	};
#endif

#if 0
/*
	外扩IO VSPI通道
*/
const DevSpiCh DevVSpi2CH1={
		.name = "VSPI2_CH1",
		.spi = "VSPI2",
		
		.csport = MCU_PORT_F,
		.cspin = GPIO_Pin_12,
		
	};
#endif	

/*-------------------------------------
	LCD硬件接口总线定义
	LCD硬件接口包含一个基本通信接口basebus、A0管脚，复位管脚，背光管脚。
---------------------------------------*/
#if 1
/*
	串行LCD接口，使用真正的SPI控制
	外扩接口中的SPI接口
*/
const DevLcdBus BusLcdSpi3={
	.name = "BusLcdSpi3",
	.type = LCD_BUS_SPI,
	.basebus = "SPI3_CH3",

	.A0port = MCU_PORT_G,
	.A0pin = GPIO_Pin_4,

	.rstport = MCU_PORT_G,
	.rstpin = GPIO_Pin_7,

	.blport = MCU_PORT_G,
	.blpin = GPIO_Pin_9,

	.staport = MCU_PORT_F, 
	.stapin = GPIO_Pin_2,
};
#else
/*
	用来接没有CS和MISO的1.33寸LCD屏
*/
const DevLcdBus BusLcdVSpi3={
	.name = "BusLcdVSpi3",
	.type = LCD_BUS_SPI,
	.basebus = "VSPI3_CH1",

	.A0port = MCU_PORT_G,
	.A0pin = GPIO_Pin_4,

	.rstport = MCU_PORT_G,
	.rstpin = GPIO_Pin_7,

	.blport = MCU_PORT_G,
	.blpin = GPIO_Pin_9,
};

#endif 
/*
	I2C接口的LCD总线
*/
const DevLcdBus BusLcdI2C1={
	.name = "BusLcdI2C1",
	.type = LCD_BUS_I2C,
	.basebus = "VI2C1",

	/*I2C接口的LCD总线，不需要其他IO*/

};
/*
	8080接口的LCD总线
*/	
const DevLcdBus BusLcd8080={
	.name = "BusLcd8080",
	.type = LCD_BUS_8080,
	.basebus = "8080",//不使用用，8080操作直接嵌入在LCD BUS代码内

	/* 8080 不用A0脚，填背光进去 */
	.A0port = MCU_PORT_B,
	.A0pin = GPIO_Pin_15,

	.rstport = MCU_PORT_A,
	.rstpin = GPIO_Pin_15,
	
	.blport = MCU_PORT_B,
	.blpin = GPIO_Pin_15,

};


#if 0
/* 模拟SPI2（外扩IO）可接SPI接口的屏 */
const DevLcdBus BusLcdVSpi2CH1={
	.name = "BusLcdVSpi2CH1",
	.type = LCD_BUS_SPI,
	.basebus = "VSPI2_CH1",

	.A0port = MCU_PORT_F,
	.A0pin = GPIO_Pin_8,

	.rstport = MCU_PORT_F,
	.rstpin = GPIO_Pin_13,

	.blport = MCU_PORT_F,
	.blpin = GPIO_Pin_14,

	.staport = MCU_PORT_F, 
	.stapin = GPIO_Pin_15,
};
#endif

#if 0
/*
	
*/
const DevLcdBus BusLcdVSpi1CH2={
	.name = "BusLcdVSpi1CH2",
	.type = LCD_BUS_SPI,
	.basebus = "VSPI1_CH2",

	.A0port = MCU_PORT_D,
	.A0pin = GPIO_Pin_15,

	.rstport = MCU_PORT_A,
	.rstpin = GPIO_Pin_15,

	.blport = MCU_PORT_B,
	.blpin = GPIO_Pin_15,
};
#endif
/*----------------------------
	FLASH设备定义
-----------------------------*/
const DevSpiFlash DevSpiFlashCore={
	/*有一个叫做board_spiflash的SPI FLASH挂在DEV_SPI_3_2上，型号未知*/
	"board_spiflash", 
	"SPI3_CH2", 
	NULL
};

const DevSpiFlash DevSpiFlashBoard={
	"core_spiflash",  
	"SPI3_CH1", 
	NULL
};

/*--------------------------------
	lcd设备树定义
	指明系统有多少个LCD设备，挂在哪个LCD总线上。
---------------------------------*/

/*I2C接口的 OLED*/

const DevLcd DevLcdOled1={
	.name = "i2coledlcd",  
	.buslcd = "BusLcdI2C1",  
	.id = 0X1315, 
	.width = 64, 
	.height = 128
	};

/*SPI接口的 OLED*/
/*
DevLcd DevLcdSpiOled	=	{
	.name = "spioledlcd", 	
	.buslcd = "BusLcdSpi3", 	
	.id = 0X1315, 
	.width = 64, 
	.height = 128};
*/
/*SPI接口的 COG LCD*/
	/*
const DevLcd DevLcdCOG1	=	{
	.name = "spicoglcd", 
	//.buslcd = "BusLcdVSpi2CH1", 
	.buslcd = "BusLcdSpi3",
	.id = 0X7565, 
	.width = 64, 
	.height = 128};
*/	
/*fsmc接口的 tft lcd*/
const DevLcd DevLcdtTFT	=	{"tftlcd", 		"BusLcd8080", 	NULL, 240, 320};
//const DevLcd DevLcdtTFT	=	{"tftlcd", 		"BusLcd8080", 	0x9325, 240, 320};
//const DevLcd DevLcdtTFT	=	{"tftlcd", 		"BusLcd8080", 	0x9341, 240, 320};
/*1408, 4.0寸的IPS屏幕*/
//const DevLcd DevLcdtTFT	=	{"tftlcd", 		"BusLcd8080", 	0x1408, 480, 800};
/*SPI接口的 tft lcd*/
//const DevLcd DevLcdtTFT	=	{"spitftlcd", 		"BusLcdSpi3", 	0x9342, 240, 320};
//const DevLcd DevLcdtTFT	=	{"spitftlcd", 		"BusLcdVSpi1CH2", 	0x9342, 240, 320};
/*1.44寸 中景园*/
//const DevLcd DevSpiLcdtTFT	=	{"spitftlcd",   "BusLcdSpi3", 	0x7735, 128, 128};

/* 1.3寸，IPS，中景园，只有SCL&SDA的SPI接口LCD*/
//const DevLcd DevLcdVSPITFT =	{"vspitftlcd",		"BusLcdVSpi3",	0x7789, 240, 240};

/* spi 接口的三色墨水屏 */
//const DevLcd DevLcdSPIEPaper =	{"spiE-Paper",		"BusLcdSpi3",	0x9187, 176, 264};
/* spi 接口 黑白墨水屏 1.54寸 GDEH154D27*/
//const DevLcd DevLcdSPIEPaper =	{"spiE-Paper",		"BusLcdSpi3",	0x3820, 200, 200};

/*
	系统设备注册
	通过缩进区分层级和依赖关系。
	后续可以考虑实现可见字符表示的设备树
*/
s32 sys_dev_register(void)
{
	/*注册I2C总线*/
	mcu_i2c_register(&DevVi2c1);
			dev_lcdbus_register(&BusLcdI2C1);
					dev_lcd_register(&DevLcdOled1);
					
	#ifdef SYS_USE_VI2C2
	//mcu_i2c_register(&DevVi2c2);
	#endif
	
	/*硬SPI3控制器，核心板和底板的FLASH、外扩接口的SPI口*/
	mcu_spi_register(&DevSpi3IO);
			mcu_spich_register(&DevSpi3CH1);
					dev_spiflash_register(&DevSpiFlashBoard);
			mcu_spich_register(&DevSpi3CH2);
					dev_spiflash_register(&DevSpiFlashCore);
			mcu_spich_register(&DevSpi3CH3);
					//dev_lcdbus_register(&BusLcdSpi3);
						//	dev_lcd_register(&DevLcdSPIEPaper);
							//dev_lcd_register(&DevLcdCOG1);
			//mcu_spich_register(&DevSpi3CH4);
	
	#ifdef SYS_USE_VSPI1
	mcu_spi_register(&DevVSpi1IO);
			mcu_spich_register(&DevVSpi1CH1);//8080接口的触摸屏
			//mcu_spich_register(&DevVSpi1CH2);
					//dev_lcdbus_register(&BusLcdVSpi1CH2);
	#endif
	
	#ifdef SYS_USE_VSPI2
	mcu_spi_register(&DevVspi2IO);
			mcu_spich_register(&DevVSpi2CH1);
					dev_lcdbus_register(&BusLcdVSpi2CH1);
							dev_lcd_register(&DevLcdSpiOled);
	#endif
	
	/*测试1.33寸IPS屏幕采用*/
	#if 0
	mcu_spi_register(&DevVspi3IO);
			mcu_spich_register(&DevVSpi3CH1);
					dev_lcdbus_register(&BusLcdVSpi3);
							dev_lcd_register(&DevLcdVSPITFT);
	#endif
	dev_lcdbus_register(&BusLcd8080);
			dev_lcd_register(&DevLcdtTFT);
	
	return 0;
}

/*

硬件参数配置表想法，类似LINUX的设备树
实际应用中，修改接口IO的可能性应该不大，
修改外设的可能性较大。
例如兼容多个LCD，降本的时候，会替代LCD，
很多LCD无法自动识别，所以，能用硬件配置文件最好。

格式：
[层级]类型:名称
	{
		参数
	}

例如：	
{
	[0]cpu:stm32f407
		{}
		[1]VI2C:VI2C1
			{	
				
			}
			[2]LCDBUS:BusLcdI2C1
				{
					
				}
				[3]LCD:i2coledlcd
					{
						
					}

}


*/

