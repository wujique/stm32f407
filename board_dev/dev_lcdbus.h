#ifndef _DEV_LCDBUS_H_
#define _DEV_LCDBUS_H_

/*
系统总共有三种LCD总线
*/
typedef enum{
	LCD_BUS_NULL = 0,
	LCD_BUS_SPI,

	#ifdef SYS_USE_LCDBUS_VSPI
	LCD_BUS_VSPI,
	#endif

	LCD_BUS_VI2C1,//OLED使用，只要两根线，背光也不需要控制，复位也不需要

	#ifdef SYS_USE_LCDBUS_VI2C2
	LCD_BUS_VI2C2,
	#endif
	
	LCD_BUS_8080,
	LCD_BUS_MAX
}LcdBusType;


/*
	LCD接口定义
*/
typedef struct  
{	
	char * name;
	
	s32 (*init)(void);
	s32 (*open)(void);
	s32 (*close)(void);
	s32 (*writedata)(u8 *data, u16 len);
	s32 (*writecmd)(u8 cmd);
	s32 (*bl)(u8 sta);
}_lcd_bus; 

extern _lcd_bus *dev_lcdbus_find(LcdBusType bus);
extern s32 dev_lcdbus_init(void);
#endif

