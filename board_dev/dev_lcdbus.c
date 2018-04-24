/**
 * @file            dev_lcdbus.c
 * @brief           对各种LCD接口封装
 * @author          wujique
 * @date            2018年4月18日 星期三
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年4月18日 星期三
 *   作    者:         wujique
 *   修改内容:   创建文件
	1 源码归屋脊雀工作室所有。
	2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
	3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
	4 可随意修改源码并分发，但不可直接销售本代码获利，并且请保留WUJIQUE版权说明。
	5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
	6 使用本源码则相当于认同本版权说明。
	7 如侵犯你的权利，请联系：code@wujique.com
	8 一切解释权归屋脊雀工作室所有。
*/
#include <stdarg.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "main.h"
#include "wujique_log.h"
#include "mcu_spi.h"
#include "mcu_i2c.h"
#include "dev_lcdbus.h"

extern s32 bus_8080interface_init(void);

/*
	一个LCD接口
	除了通信的接口
	还有其他不属于通信接口的信号
*/

/*
	串行LCD接口，使用真正的SPI控制

*/
/* 命令数据控制*/
#define SERIALLCD_SPI_A0_PORT GPIOG
#define SERIALLCD_SPI_A0_PIN GPIO_Pin_4
/*复位*/	
#define SERIALLCD_SPI_RST_PORT GPIOG
#define SERIALLCD_SPI_RST_PIN GPIO_Pin_7
/* 背光*/	
#define SERIALLCD_SPI_BL_PORT GPIOG
#define SERIALLCD_SPI_BL_PIN GPIO_Pin_9

//复位
#define SERIALLCD_SPI_RST_Clr() GPIO_ResetBits(SERIALLCD_SPI_RST_PORT, SERIALLCD_SPI_RST_PIN)
#define SERIALLCD_SPI_RST_Set() GPIO_SetBits(SERIALLCD_SPI_RST_PORT, SERIALLCD_SPI_RST_PIN)
//命令
#define SERIALLCD_SPI_RS_Clr() GPIO_ResetBits(SERIALLCD_SPI_A0_PORT, SERIALLCD_SPI_A0_PIN)
#define SERIALLCD_SPI_RS_Set() GPIO_SetBits(SERIALLCD_SPI_A0_PORT, SERIALLCD_SPI_A0_PIN)

static void bus_seriallcd_spi_IO_init(void) 
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	//DC(A0)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
	GPIO_InitStructure.GPIO_Pin = SERIALLCD_SPI_A0_PIN;
	GPIO_Init(SERIALLCD_SPI_A0_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SERIALLCD_SPI_A0_PORT,SERIALLCD_SPI_A0_PIN);

	//RST
	GPIO_InitStructure.GPIO_Pin = SERIALLCD_SPI_RST_PIN; //OUT推挽输出   RST
	GPIO_Init(SERIALLCD_SPI_RST_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SERIALLCD_SPI_RST_PORT,SERIALLCD_SPI_RST_PIN);

	//bl
	GPIO_InitStructure.GPIO_Pin = SERIALLCD_SPI_BL_PIN; //OUT推挽输出 
	GPIO_Init(SERIALLCD_SPI_BL_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SERIALLCD_SPI_BL_PORT, SERIALLCD_SPI_BL_PIN);	

}


static s32 bus_seriallcd_spi_init()
{
	bus_seriallcd_spi_IO_init();
	Delay(100);
	SERIALLCD_SPI_RST_Clr();
	Delay(100);
	SERIALLCD_SPI_RST_Set();
	Delay(100);
	return 0;
}

static s32 bus_seriallcd_spi_open(void)
{
	s32 res;
	res = mcu_spi_open(DEV_SPI_3_3, SPI_MODE_3, SPI_BaudRatePrescaler_4);
	return res;
}

static s32 bus_seriallcd_spi_close(void)
{
	s32 res;
	res = mcu_spi_close(DEV_SPI_3_3);
	return res;
}

static s32 bus_seriallcd_spi_write_data(u8 *data, u16 len)
{
	SERIALLCD_SPI_RS_Set();	
	mcu_spi_cs(DEV_SPI_3_3,0);
	mcu_spi_transfer(DEV_SPI_3_3, data, NULL, len);
	mcu_spi_cs(DEV_SPI_3_3,1);
	return 0;
}

static s32 bus_seriallcd_spi_write_cmd(u8 cmd)
{
	u8 tmp[2];
	
	SERIALLCD_SPI_RS_Clr();
	tmp[0] = cmd;
	mcu_spi_cs(DEV_SPI_3_3,0);
	mcu_spi_transfer(DEV_SPI_3_3, &tmp[0], NULL, 1);
	mcu_spi_cs(DEV_SPI_3_3,1);
	return 0;
}

static s32 bus_seriallcd_spi_bl(u8 sta)
{
	if(sta ==1)
	{
		GPIO_SetBits(SERIALLCD_SPI_BL_PORT, SERIALLCD_SPI_BL_PIN);
	}
	else
	{
		GPIO_ResetBits(SERIALLCD_SPI_BL_PORT, SERIALLCD_SPI_BL_PIN);	
	}
	return 0;
}


_lcd_bus BusSerialLcdSpi={
		.name = "BusSerivaLcdSpi",
		.init =bus_seriallcd_spi_init,
		.open =bus_seriallcd_spi_open,
		.close =bus_seriallcd_spi_close,
		.writedata =bus_seriallcd_spi_write_data,
		.writecmd =bus_seriallcd_spi_write_cmd,
		.bl =bus_seriallcd_spi_bl,				
};
		
/*

	定义一个串行LCD接口2，使用模拟SPI。

*/
#define SERIALLCD_VSPI_A0_PORT GPIOF
#define SERIALLCD_VSPI_A0_PIN GPIO_Pin_8
	
#define SERIALLCD_VSPI_RST_PORT GPIOF
#define SERIALLCD_VSPI_RST_PIN GPIO_Pin_13
	
#define SERIALLCD_VSPI_BL_PORT GPIOF
#define SERIALLCD_VSPI_BL_PIN GPIO_Pin_14

//复位
#define SERIALLCD_VSPI_RST_Clr() GPIO_ResetBits(SERIALLCD_VSPI_RST_PORT, SERIALLCD_VSPI_RST_PIN)
#define SERIALLCD_VSPI_RST_Set() GPIO_SetBits(SERIALLCD_VSPI_RST_PORT, SERIALLCD_VSPI_RST_PIN)
//命令
#define SERIALLCD_VSPI_RS_Clr() GPIO_ResetBits(SERIALLCD_VSPI_A0_PORT, SERIALLCD_VSPI_A0_PIN)
#define SERIALLCD_VSPI_RS_Set() GPIO_SetBits(SERIALLCD_VSPI_A0_PORT, SERIALLCD_VSPI_A0_PIN)

static void bus_seriallcd_vspi_IO_init(void) 
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	//DC(A0)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
	GPIO_InitStructure.GPIO_Pin = SERIALLCD_VSPI_A0_PIN;
	GPIO_Init(SERIALLCD_VSPI_A0_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SERIALLCD_VSPI_A0_PORT,SERIALLCD_VSPI_A0_PIN);

	//RST
	GPIO_InitStructure.GPIO_Pin = SERIALLCD_VSPI_RST_PIN; //OUT推挽输出   RST
	GPIO_Init(SERIALLCD_VSPI_RST_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SERIALLCD_VSPI_RST_PORT,SERIALLCD_VSPI_RST_PIN);

	//bl
	GPIO_InitStructure.GPIO_Pin = SERIALLCD_VSPI_BL_PIN; //OUT推挽输出 
	GPIO_Init(SERIALLCD_VSPI_BL_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SERIALLCD_VSPI_BL_PORT, SERIALLCD_VSPI_BL_PIN);	

}


static s32 bus_seriallcd_vspi_init()
{
	bus_seriallcd_vspi_IO_init();
	Delay(100);
	SERIALLCD_VSPI_RST_Clr();
	Delay(100);
	SERIALLCD_VSPI_RST_Set();
	Delay(100);
	return 0;
}

static s32 bus_seriallcd_vspi_open(void)
{
	s32 res;
	res = mcu_spi_open(DEV_VSPI_2, SPI_MODE_3, SPI_BaudRatePrescaler_4);
	return res;
}

static s32 bus_seriallcd_vspi_close(void)
{
	s32 res;
	res = mcu_spi_close(DEV_VSPI_2);
	return res;
}

static s32 bus_seriallcd_vspi_write_data(u8 *data, u16 len)
{
	SERIALLCD_VSPI_RS_Set();	
	mcu_spi_cs(DEV_VSPI_2,0);
	mcu_spi_transfer(DEV_VSPI_2, data, NULL, len);
	mcu_spi_cs(DEV_VSPI_2,1);
	return 0;
}

static s32 bus_seriallcd_vspi_write_cmd(u8 cmd)
{
	u8 tmp[2];
	
	SERIALLCD_VSPI_RS_Clr();
	tmp[0] = cmd;
	mcu_spi_cs(DEV_VSPI_2,0);
	mcu_spi_transfer(DEV_VSPI_2, &tmp[0], NULL, 1);
	mcu_spi_cs(DEV_VSPI_2,1);
	return 0;
}

static s32 bus_seriallcd_vspi_bl(u8 sta)
{
	if(sta ==1)
	{
		GPIO_SetBits(SERIALLCD_VSPI_BL_PORT, SERIALLCD_VSPI_BL_PIN);
	}
	else
	{
		GPIO_ResetBits(SERIALLCD_VSPI_BL_PORT, SERIALLCD_VSPI_BL_PIN);	
	}
	return 0;
}


_lcd_bus BusSerialLcdVSpi={
		.name = "BusSerivaLcdVSpi",
		.init =bus_seriallcd_vspi_init,
		.open =bus_seriallcd_vspi_open,
		.close =bus_seriallcd_vspi_close,
		.writedata =bus_seriallcd_vspi_write_data,
		.writecmd =bus_seriallcd_vspi_write_cmd,
		.bl =bus_seriallcd_vspi_bl,				
};

/*
	定义一个LCD串行总线，用模拟 I2C

*/
static s32 bus_seriallcd_vi2c_init()
{
	return 0;
}

static s32 bus_seriallcd_vi2c_open(void)
{

	return 0;
}

static s32 bus_seriallcd_vi2c_close(void)
{
	return 0;
}

static s32 bus_seriallcd_vi2c_write_data(u8 *data, u16 len)
{
	u8 tmp[256];
	
	tmp[0] = 0x40;
	memcpy(&tmp[1], data, len);
	mcu_i2c_transfer(0x3C, MCU_I2C_MODE_W, tmp, len+1);	
	return 0;
}

static s32 bus_seriallcd_vi2c_write_cmd(u8 cmd)
{
	u8 tmp[2];
	
	tmp[0] = 0x00;
	tmp[1] = cmd;
	mcu_i2c_transfer(0x3C, MCU_I2C_MODE_W, tmp, 2);	
	return 0;
}

static s32 bus_seriallcd_vi2c_bl(u8 sta)
{

	return 0;
}


_lcd_bus BusSerialLcdVI2C={
		.name = "BusSerivaLcdVI2C",
		.init =bus_seriallcd_vi2c_init,
		.open =bus_seriallcd_vi2c_open,
		.close =bus_seriallcd_vi2c_close,
		.writedata =bus_seriallcd_vi2c_write_data,
		.writecmd =bus_seriallcd_vi2c_write_cmd,
		.bl =bus_seriallcd_vi2c_bl,				
};


/*
	定义一个LCD总线，空的，不存在
*/
static s32 bus_seriallcd_null_init()
{
	return 0;
}

static s32 bus_seriallcd_null_open(void)
{

	return 0;
}

static s32 bus_seriallcd_null_close(void)
{
	return 0;
}

static s32 bus_seriallcd_null_write_data(u8 *data, u16 len)
{
	return 0;
}

static s32 bus_seriallcd_null_write_cmd(u8 cmd)
{

	return 0;
}

static s32 bus_seriallcd_null_bl(u8 sta)
{

	return 0;
}


_lcd_bus BusSerialLcdNULL={
		.name = "NULL",
		.init =bus_seriallcd_null_init,
		.open =bus_seriallcd_null_open,
		.close =bus_seriallcd_null_close,
		.writedata =bus_seriallcd_null_write_data,
		.writecmd =bus_seriallcd_null_write_cmd,
		.bl =bus_seriallcd_null_bl,				
};

/*

	顺序要跟LcdBusType枚举一致

*/
_lcd_bus *LcdBusList[] = {
		&BusSerialLcdNULL,
		&BusSerialLcdSpi,

		/*vspi跟矩阵按键冲突*/
		#ifdef SYS_USE_VSPI2
		&BusSerialLcdVSpi,
		#else
		&BusSerialLcdNULL,
		#endif
		
		&BusSerialLcdVI2C,
	};

/**
 *@brief:      dev_lcdbus_init
 *@details:    初始化所有LCD总线
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_lcdbus_init(void)
{
	bus_8080interface_init();
	return 0;
}
/**
 *@brief:      dev_lcdbus_find
 *@details:    根据总线编号查找总线驱动
 *@param[in]   LcdBusType bus  
 *@param[out]  无
 *@retval:     _lcd_bus
 */
_lcd_bus *dev_lcdbus_find(LcdBusType bus)
{
	u8 num;

	num = (u8)LCD_BUS_MAX;

	if(num > (sizeof(LcdBusList)/sizeof(_lcd_bus *)))
		num = (sizeof(LcdBusList)/sizeof(_lcd_bus *));
	
	if(bus>= num)
		return NULL;
	else
		return LcdBusList[bus];
	
}



