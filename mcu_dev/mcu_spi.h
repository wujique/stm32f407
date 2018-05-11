#ifndef __MCU_SPI_H_
#define __MCU_SPI_H_

#include "list.h"
#include "mcu_bsp.h"

typedef enum{
	DEV_SPI_H = 1,//硬件SPI控制器
	DEV_SPI_V = 2,//IO模拟SPI
}DEV_SPI_TYPE;

/*
	SPI 分两层，
	1层是SPI控制器，不包含CS
	2层是SPI通道，由控制器+CS组成
	
*/
/*

	SPI 设备定义

*/
typedef struct
{
	/*设备名称*/
	char name[16];
	/*设备类型，IO模拟 or 硬件控制器*/
	DEV_SPI_TYPE type;
	
	MCU_PORT clkport;
	u16 clkpin;

	MCU_PORT mosiport;
	u16 mosipin;

	MCU_PORT misoport;
	u16 misopin;

}DevSpi;

/*

	SPI控制器设备节点
	
*/
typedef struct
{
	/*句柄，空闲为-1，打开为0，spi控制器不能重复打开*/
	s32 gd;
	/*控制器硬件信息，初始化控制器时拷贝设备树的信息到此*/
	DevSpi dev;	
	/*链表*/
	struct list_head list;
}DevSpiNode;

/*
	SPI 通道定义
	一个SPI通道，有一个SPI控制器+一根CS引脚组成

*/
typedef struct
{
	/*通道名称，相当于设备名称*/
	char name[16];
	/*SPI控制器名称*/
	char spi[16];

	/*cs脚*/
	MCU_PORT csport;
	u16 cspin;
}DevSpiCh;

/*SPI通道节点*/
typedef struct
{
	/**/
	s32 gd;
	DevSpiCh dev;	
	DevSpiNode *spi;//控制器节点指针
	struct list_head list;
}DevSpiChNode;

/*

SPI模式

*/
typedef enum{
	SPI_MODE_0 =0,
	SPI_MODE_1,
	SPI_MODE_2,
	SPI_MODE_3,
	SPI_MODE_MAX
}SPI_MODE;

extern DevSpiChNode *mcu_spi_open(char *name, SPI_MODE mode, u16 pre);
extern s32 mcu_spi_close(DevSpiChNode * node);
extern s32 mcu_spi_transfer(DevSpiChNode * node, u8 *snd, u8 *rsv, s32 len);
extern s32 mcu_spi_cs(DevSpiChNode * node, u8 sta);


#endif

