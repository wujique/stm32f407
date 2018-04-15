/**
 * @file            dev_ILI9341.c
 * @brief           TFT LCD 驱动芯片ILI6341驱动程序
 * @author          wujique
 * @date            2017年11月8日 星期三
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年11月8日 星期三
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
#include "stdlib.h"
#include "string.h"

#include "stm32f4xx.h"
#include "stm324xg_eval_fsmc_sram.h"
#include "wujique_log.h"
#include "dev_ILI9341.h"

#define DEV_ILI9341_DEBUG

#ifdef DEV_ILI9341_DEBUG
#define ILI9341_DEBUG	wjq_log 
#else
#define ILI9341_DEBUG(a, ...)
#endif

extern void Delay(__IO uint32_t nTime);

struct _strlcd_obj LCD;

/*

	本文件名字虽然叫9341，但是内容暂时存放所有并口总线LCD驱动

*/
#define TFT_LCD_DRIVER_9341
#define TFT_LCD_DRIVER_9325

/*

	8080总线接口初始化，包括复位背光


*/
	/*	 背光和复位脚定义 */
#define DEV_TFTLCD_BL_CTL_PORT GPIOB
#define DEV_TFTLCD_BL_CTL 	GPIO_Pin_15
	
#define DEV_TFTLCD_RESET_PORT GPIOA
#define DEV_TFTLCD_RESET  	GPIO_Pin_15

/*

	初始化LCD 8080接口

*/
s32 Bus8080Gd = -1;

s32 bus_8080interface_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//初始化背光控制和硬复位管脚
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = DEV_TFTLCD_BL_CTL; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DEV_TFTLCD_BL_CTL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = DEV_TFTLCD_RESET; 
    GPIO_Init(DEV_TFTLCD_RESET_PORT, &GPIO_InitStructure);

	GPIO_ResetBits(DEV_TFTLCD_BL_CTL_PORT, DEV_TFTLCD_BL_CTL);
	
	Delay(5);
	ILI9341_DEBUG(LOG_DEBUG, "LCD RESET LOW!\r\n");
	GPIO_ResetBits(DEV_TFTLCD_RESET_PORT, DEV_TFTLCD_RESET);
	Delay(5);
	ILI9341_DEBUG(LOG_DEBUG, "LCD RESET HIGH!\r\n");
	GPIO_SetBits(DEV_TFTLCD_RESET_PORT, DEV_TFTLCD_RESET);
	Delay(5);
	
	//初始FSMC
	mcu_fsmc_lcd_Init();	
	
	ILI9341_DEBUG(LOG_DEBUG, "lcd 8080 init finish!\r\n");
	return 0;
}

s32 bus_8080interface_open(void)
{
	if(Bus8080Gd == -1)
	{
		Bus8080Gd = 0;	
		return 0;
	}
	else
	{
		return -1;
	}
}

s32 bus_8080interface_close(void)
{
	Bus8080Gd = -1;
	return 0;
}


/*
	写寄存器要两步
	*LcdReg = LCD_Reg; //写入要写的寄存器序号
	*LcdData = LCD_RegValue; //写入数据 
*/

volatile u16 *LcdReg = (u16*)0x6C000000;
volatile u16 *LcdData = (u16*)0x6C010000;
/**
 *@brief:      dev_lcd_write_cmd
 *@details:       写命令
 *@param[in]  u16 CMD  
 *@param[out]  无
 *@retval:     
 */
void bus_8080_write_cmd(u16 CMD)
{			
	*LcdReg = CMD;
}

void bus_8080_write_data(u16 data)
{			
	*LcdData = data;
}
/**
 *@brief:      dev_8080_bl
 *@details:    背光控制
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     
 */
void bus_8080_lcd_bl(u8 sta)
{
	if(sta == 1)
	{
		GPIO_SetBits(DEV_TFTLCD_BL_CTL_PORT, DEV_TFTLCD_BL_CTL);
	}
	else
	{
		GPIO_ResetBits(DEV_TFTLCD_BL_CTL_PORT, DEV_TFTLCD_BL_CTL);	
	}	

	return;
}

/* ----------下面为不同LCD 驱动--------*/

/*

	9341驱动

*/
#ifdef TFT_LCD_DRIVER_9341
/*9341命令定义*/
#define ILI9341_CMD_WRAM 0x2c
#define ILI9341_CMD_SETX 0x2a
#define ILI9341_CMD_SETY 0x2b

s32 drv_ILI9341_init(void);
static s32 drv_ILI9341_drawpoint(u16 x, u16 y, u16 color);
s32 drv_ILI9341_color_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 color);
s32 drv_ILI9341_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 *color);
static s32 drv_ILI9341_display_onoff(u8 sta);
s32 drv_ILI9341_prepare_display(u16 sx, u16 ex, u16 sy, u16 ey);
static void drv_ILI9341_scan_dir(u8 dir);
void drv_ILI9341_lcd_bl(u8 sta);

/*

	定义一个TFT LCD，使用ILI9341驱动IC的设备

*/
_lcd_drv TftLcdILI9341Drv = {
							.id = 0X9341,

							.init = drv_ILI9341_init,
							.draw_point = drv_ILI9341_drawpoint,
							.color_fill = drv_ILI9341_color_fill,
							.fill = drv_ILI9341_fill,
							.onoff = drv_ILI9341_display_onoff,
							.prepare_display = drv_ILI9341_prepare_display,
							.set_dir = drv_ILI9341_scan_dir,
							.backlight = drv_ILI9341_lcd_bl
							};

void drv_ILI9341_lcd_bl( u8 sta)
{
	bus_8080_lcd_bl(sta);
}
	
/**
 *@brief:      drv_ILI9341_scan_dir
 *@details:    设置显存扫描方向， 本函数为竖屏角度
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:     static
 */
static void drv_ILI9341_scan_dir(u8 dir)
{
	u16 regval=0;

	/*设置从左边到右边还是右边到左边*/
	switch(dir)
	{
		case R2L_U2D:
		case R2L_D2U:
		case U2D_R2L:
		case D2U_R2L:
			regval|=(1<<6); 
			break;	 
	}

	/*设置从上到下还是从下到上*/
	switch(dir)
	{
		case L2R_D2U:
		case R2L_D2U:
		case D2U_L2R:
		case D2U_R2L:
			regval|=(1<<7); 
			break;	 
	}

	/*
		设置先左右还是先上下 Reverse Mode
		如果设置为1，LCD控制器已经将行跟列对调了，
		因此需要在显示中进行调整
	*/
	switch(dir)
	{
		case U2D_L2R:
		case D2U_L2R:
		case U2D_R2L:
		case D2U_R2L:
			regval|=(1<<5);
			break;	 
	}
	/*
		还可以设置RGB还是GBR
		还可以设置调转上下
	*/	
	regval|=(1<<3);//0:GBR,1:RGB  跟R61408相反

	*LcdReg = (0x36); 
	*LcdData = (regval); 
}

/**
 *@brief:      drv_ILI9341_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_set_cp_addr(u16 sc, u16 ec, u16 sp, u16 ep)
{

	*LcdReg  = ILI9341_CMD_SETX; 
	*LcdData = sc>>8; 
	*LcdData = sc&0XFF;	 
	*LcdData = ec>>8; 
	*LcdData = ec&0XFF;	 

	*LcdReg  = ILI9341_CMD_SETY; 
	*LcdData = sp>>8; 
	*LcdData = sp&0XFF;
	*LcdData = ep>>8; 
	*LcdData = ep&0XFF;

	*LcdReg  = ILI9341_CMD_WRAM; 
	return 0;
}

/**
 *@brief:      drv_ILI9341_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ILI9341_display_onoff(u8 sta)
{
	if(sta == 1)
		*LcdReg	= 0x29;
	else
		*LcdReg	= 0x28;

	return 0;
}

/**
 *@brief:      drv_ILI9341_init
 *@details:    初始化FSMC，并且读取ILI9341的设备ID
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_init(void)
{
	u16 data;

	*LcdReg = 0x00d3;
	data = *LcdData; //dummy read
	data = *LcdData; //读到 0X00
	data = *LcdData; //读取 93 
	data<<=8;
	data |= *LcdData; //读取 41

	ILI9341_DEBUG(LOG_DEBUG, "read reg:%04x\r\n", data);

	
	if(data != TftLcdILI9341Drv.id)
	{
		ILI9341_DEBUG(LOG_DEBUG, "lcd drive no 9341\r\n");	
		return -1;
	}

	*LcdReg=0xCF ;//Power control B
	*LcdData = 0x00;*LcdData = 0xC1;*LcdData = 0x30;

	*LcdReg = 0xED;//Power on sequence control 
	*LcdData = 0x64;*LcdData = 0x03;*LcdData = 0x12;*LcdData = 0x81;

	*LcdReg = 0xE8;	//Driver timing control A
	*LcdData = 0x85;*LcdData = 0x01;*LcdData = 0x7A;

	*LcdReg = 0xCB;//Power control 
	*LcdData = 0x39;*LcdData = 0x2C;*LcdData = 0x00;
	*LcdData = 0x34;*LcdData = 0x02;

	*LcdReg = 0xF7;//Pump ratio control
	*LcdData = 0x20;

	*LcdReg = 0xEA;//Driver timing control
	*LcdData = 0x00;*LcdData = 0x00;

	*LcdReg = 0xC0; //Power control
	*LcdData = 0x21; //VRH[5:0]

	*LcdReg = 0xC1; //Power control
	*LcdData = 0x11; //SAP[2:0];BT[3:0]

	*LcdReg = 0xC5; //VCM control
	*LcdData = 0x31;*LcdData = 0x3C;

	*LcdReg = 0xC7; //VCM control2
	*LcdData = 0x9f;

	*LcdReg = 0x36; // Memory Access Control
	*LcdData = 0x08;

	*LcdReg = 0x3A; // Memory Access Control
	*LcdData = 0x55;

	*LcdReg = 0xB1;
	*LcdData = 0x00;*LcdData = 0x1B;

	*LcdReg = 0xB6; // Display Function Control
	*LcdData = 0x0A;*LcdData = 0xA2;

	*LcdReg = 0xF2; // 3Gamma Function Disable
	*LcdData = 0x00;

	*LcdReg = 0x26; //Gamma curve selected
	*LcdData = 0x01;

	*LcdReg = 0xE0; //Set Gamma
	*LcdData = 0x0F;*LcdData = 0x20;*LcdData = 0x1d;*LcdData = 0x0b;
	*LcdData = 0x10;*LcdData = 0x0a;*LcdData = 0x49;*LcdData = 0xa9;
	*LcdData = 0x3b;*LcdData = 0x0a;*LcdData = 0x15;*LcdData = 0x06;
	*LcdData = 0x0c;*LcdData = 0x06;*LcdData = 0x00;
	
	*LcdReg = 0XE1; //Set Gamma
	*LcdData = 0x00;*LcdData = 0x1f;*LcdData = 0x22;*LcdData = 0x04;
	*LcdData = 0x0f;*LcdData = 0x05;*LcdData = 0x36;*LcdData = 0x46;
	*LcdData = 0x46;*LcdData = 0x05;*LcdData = 0x0b;*LcdData = 0x09;
	*LcdData = 0x33;*LcdData = 0x39;*LcdData = 0x0F;

	*LcdReg = 0x11; // Sleep out
	Delay(12);

	return 0;
}
/**
 *@brief:      drv_ILI9341_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_xy2cp(u16 sx, u16 ex, u16 sy, u16 ey, u16 *sc, u16 *ec, u16 *sp, u16 *ep)
{
	struct _strlcd_obj *obj;
	obj = &LCD;

	/*
		显示XY轴范围
	*/
	if(sx > obj->width)
		sx = obj->width;
	
	if(ex > obj->width)
		ex = obj->width;
	
	if(sy > obj->height)
		sy = obj->height;
	
	if(ey > obj->height)
		ey = obj->height;
	/*
		XY轴，实物角度来看，方向取决于横屏还是竖屏
		CP轴，是控制器显存角度，
		XY轴的映射关系取决于扫描方向
	*/
	if(
		(((obj->scandir&LRUD_BIT_MASK) == LRUD_BIT_MASK)
		&&(obj->dir == H_LCD))
		||
		(((obj->scandir&LRUD_BIT_MASK) == 0)
		&&(obj->dir == W_LCD))
		)
		{
			*sc = sy;
			*ec = ey;
			*sp = sx;
			*ep = ex;
		}
	else
	{
		*sc = sx;
		*ec = ex;
		*sp = sy;
		*ep = ey;
	}
	
	return 0;
}
/**
 *@brief:      drv_ILI9341_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ILI9341_drawpoint(u16 x, u16 y, u16 color)
{
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(x, x, y, y, &sc,&ec,&sp,&ep);
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);
	*LcdData = color; 
	return 0;
}
/**
 *@brief:      drv_ILI9341_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_color_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i,j;
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);

	width=(ec+1)-sc;//得到填充的宽度 +1是因为坐标从0开始
	height=(ep+1)-sp;//高度
	
	//uart_printf("ili9341 width:%d, height:%d\r\n", width, height);
	
	for(i=0; i<height; i++)
	{
		//uart_printf("x:%d, y:%d\r\n", sx, sy+i);
		for(j=0; j<width; j++)
		{
			//Delay(1);
			*LcdData = color;//写入数据 
		}
		//uart_printf("\r\n");
	}	 

	return 0;

}

/**
 *@brief:      drv_ILI9341_color_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9341_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u32 i,j;
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);

	width=(ec+1)-sc;//得到填充的宽度 +1是因为坐标从0开始
	height=(ep+1)-sp;//高度
	
	//uart_printf("ili9341 width:%d, height:%d\r\n", width, height);
	j = width*height;
	
	for(i=0; i<j; i++)
	{
		*LcdData = *(color+i);//写入数据 
	}	 

	return 0;

} 

s32 drv_ILI9341_prepare_display(u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 sc,ec,sp,ep;

	drv_ILI9341_xy2cp(sx, ex, sy, ey, &sc,&ec,&sp,&ep);
	drv_ILI9341_set_cp_addr(sc, ec, sp, ep);	
	return 0;
}
#endif

//----------------------------------------------------------------------
/*

	9325

*/
#ifdef TFT_LCD_DRIVER_9325

/*
	9325特点：
	9325不使用page跟colum，使用Horizontal和Vertical，对于我们的模组，H是240短边，V是320长边
	1 设置扫描起始地址,  也就是 H, V地址， HV地址组成参数AD
	  AD低八位对应H，高八位对应V
	  
	2 扫描窗口地址，
	  9325叫做HSA/HEA/VSA/VEA
	  窗口H要大于等于4
	  “00”h ≤ HSA[7:0]< HEA[7:0] ≤ “EF”h. and “04”h≦HEA-HAS
	  “000”h ≤ VSA[8:0]< VEA[8:0] ≤ “13F”h.

	3 在设置扫描方向，先上下还是先左右时，H跟V不变。

	4 无论扫描方向怎么变，原点都是竖屏的左上角。其他屏幕是扫描方向的起点。
	例如，设置为横屏，左到右，上到下
	dev_tftlcd_setdir( W_LCD, L2R_U2D);
	然后刷新一个数据块
	TftLcd->color_fill(0, TftLcd->width/4, 0 , TftLcd->height/4, RED);
	其他屏幕刷新的是横屏的左上角，
	9325刷的是左下下角，
	而且是先刷最底下一行，也就是第0行，方向是从左到右。
	然后再刷最上一行，也是从左到右。
	因此需要将扫描起点设为XY轴的起点
	
	
*/
#define ILI9325_CMD_WRAM 0x22
#define ILI9325_CMD_SETV 0x21
#define ILI9325_CMD_SETH 0x20


s32 drv_ILI9325_init(void);
static s32 drv_ILI9325_drawpoint(u16 x, u16 y, u16 color);
s32 drv_ILI9325_color_fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);
s32 drv_ILI9325_fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color);
static s32 drv_ILI9325_display_onoff(u8 sta);
s32 drv_ILI9325_prepare_display(u16 sc, u16 ec, u16 sp, u16 ep);
static void drv_ILI9325_scan_dir(u8 dir);
void drv_ILI9325_lcd_bl(u8 sta);
/*

	9325驱动

*/
_lcd_drv TftLcdILI9325Drv = {
							.id = 0X9325,

							.init = drv_ILI9325_init,
							.draw_point = drv_ILI9325_drawpoint,
							.color_fill = drv_ILI9325_color_fill,
							.fill = drv_ILI9325_fill,
							.onoff = drv_ILI9325_display_onoff,
							.prepare_display = drv_ILI9325_prepare_display,
							.set_dir = drv_ILI9325_scan_dir,
							.backlight = drv_ILI9325_lcd_bl
							};
void drv_ILI9325_lcd_bl(u8 sta)
{
	bus_8080_lcd_bl(sta);
}	
/**
 *@brief:	   drv_ILI9325_scan_dir
 *@details:    设置显存扫描方向
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:	   static OK
 */
static void drv_ILI9325_scan_dir(u8 dir)
{
	u16 regval=0;
	u8 dirreg=0;
	
	switch(dir)
    {
        case L2R_U2D://从左到右,从上到下
            regval|=(1<<5)|(1<<4)|(0<<3); 
            break;
        case L2R_D2U://从左到右,从下到上
            regval|=(0<<5)|(1<<4)|(0<<3); 
            break;
        case R2L_U2D://从右到左,从上到下
            regval|=(1<<5)|(0<<4)|(0<<3);
            break;
        case R2L_D2U://从右到左,从下到上
            regval|=(0<<5)|(0<<4)|(0<<3); 
            break;   
        case U2D_L2R://从上到下,从左到右
            regval|=(1<<5)|(1<<4)|(1<<3); 
            break;
        case U2D_R2L://从上到下,从右到左
            regval|=(1<<5)|(0<<4)|(1<<3); 
            break;
        case D2U_L2R://从下到上,从左到右
            regval|=(0<<5)|(1<<4)|(1<<3); 
            break;
        case D2U_R2L://从下到上,从右到左
            regval|=(0<<5)|(0<<4)|(1<<3); 
            break;   
    }
	
    dirreg=0X03;
    regval|=1<<12;  
    
	*LcdReg  = dirreg; 
	*LcdData = regval; 
}

/**
 *@brief:      drv_ILI9325_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     9325设置扫描区域有一个限制，那就是窗口宽度不能小于四
 */
static s32 drv_ILI9325_set_cp_addr(u16 hsa, u16 hea, u16 vsa, u16 vea)
{
	struct _strlcd_obj *obj;
	obj = &LCD;
	u16 heatmp;

	/* 设置扫描窗口 */
	if((hsa+4) > hea)
		heatmp = hsa+4;
	else
		heatmp = hea;
	
	*LcdReg = 0x0050;//HSA
	*LcdData = hsa;
	
	*LcdReg = 0x0051;//HEA
	*LcdData = heatmp;

	*LcdReg = 0x0052;//VSA
	*LcdData = vsa;
	
	*LcdReg = 0x0053;//VEA
	*LcdData = vea;

	/*
		设置扫描起始地址。
	*/
	if((obj->scandir&LR_BIT_MASK) == LR_BIT_MASK)
	{
		*LcdReg  = ILI9325_CMD_SETH; 
		*LcdData = hea&0XFF; 
	}
	else
	{
		*LcdReg  = ILI9325_CMD_SETH; 
		*LcdData = hsa&0XFF; 	  
	}

	if((obj->scandir&UD_BIT_MASK) == UD_BIT_MASK)
	{
		*LcdReg  = ILI9325_CMD_SETV;  
		*LcdData = vea&0X1FF;
	}
	else
	{
		*LcdReg  = ILI9325_CMD_SETV;  
		*LcdData = vsa&0X1FF;
	}
	
	*LcdReg  = ILI9325_CMD_WRAM; 
	
	return 0;
}

/**
 *@brief:	   drv_ILI9325_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:	   static OK
 */
static s32 drv_ILI9325_display_onoff(u8 sta)
{
	if(sta == 1)
	{

		*LcdReg = 0X07;
		*LcdData = 0x0173;
	}
	else
	{
		*LcdReg = 0X07;
		*LcdData = 0x00;
	}
	return 0;
}

/**
 *@brief:	   drv_ILI9325_init
 *@details:    初始化FSMC，并且读取ILI9325的设备ID
 *@param[in]   void  
 *@param[out]  无
 *@retval:	   
 */
s32 drv_ILI9325_init(void)
{
	u16 data;

	/*
		读9325的ID
		
	*/
	*LcdReg = 0x0000;
	*LcdData = 0x0001;
	
	*LcdReg = 0x0000;
    data = *LcdData; 

	ILI9341_DEBUG(LOG_DEBUG, "read reg:%04x\r\n", data);
	if(data != TftLcdILI9325Drv.id)
	{
		ILI9341_DEBUG(LOG_DEBUG, "lcd drive no 9325\r\n");	
		return -1;
	}

	*LcdReg = 0x00E5;
	*LcdData = 0x78F0;

	*LcdReg = 0x0001;
	*LcdData = 0x0100;

	*LcdReg = 0x0002;
	*LcdData = 0x0700;
	
	*LcdReg = 0x0003;
	*LcdData = 0x1030;
	
	*LcdReg = 0x0004;
	*LcdData = 0x0000;
	
	*LcdReg = 0x0008;
	*LcdData = 0x0202;
	
 	*LcdReg = 0x0009;
	*LcdData = 0x0000;
	
	*LcdReg = 0x000A;
	*LcdData = 0x0000;
	
	*LcdReg = 0x000C;
	*LcdData = 0x0000;

	*LcdReg = 0x000D;
	*LcdData = 0x0000;

	*LcdReg = 0x000F;
	*LcdData = 0x0000;

	//power on sequence VGHVGL
	*LcdReg = 0x0010;
	*LcdData = 0x0000;
	
  	*LcdReg = 0x0011;
	*LcdData = 0x0007;
	
 	*LcdReg = 0x0012;
	*LcdData = 0x0000;
	
 	*LcdReg = 0x0013;
	*LcdData = 0x0000;
	
 	*LcdReg = 0x0007;
	*LcdData = 0x0000;
	
	//vgh 
	*LcdReg = 0x0010;
	*LcdData = 0x1690;

	*LcdReg = 0x0011;
	*LcdData = 0x0227;
	
	//vregiout 
	*LcdReg = 0x0012;
	*LcdData = 0x009D;

	//vom amplitude
	*LcdReg = 0x0013;
	*LcdData = 0x1900;

	//vom H
	*LcdReg = 0x0029;
	*LcdData = 0x0025;

	*LcdReg = 0x002B;
	*LcdData = 0x000D;

	//gamma
	*LcdReg = 0x0030;
	*LcdData = 0x0007;

	*LcdReg = 0x0031;
	*LcdData = 0x0303;
	
	*LcdReg = 0x0032;
	*LcdData = 0x0003;

	*LcdReg = 0x0035;
	*LcdData = 0x0206;
	
	*LcdReg = 0x0036;
	*LcdData = 0x0008;
	
	*LcdReg = 0x0037;
	*LcdData = 0x0406;

	*LcdReg = 0x0038;
	*LcdData = 0x0304;

	*LcdReg = 0x0039;
	*LcdData = 0x0007;
	
	*LcdReg = 0x003C;
	*LcdData = 0x0602;
	
	*LcdReg = 0x003D;
	*LcdData = 0x0008;
	
	/*
	Horizontal and Vertical RAM Address Position 219*319
	设置扫描窗口

	*/
	*LcdReg = 0x0050;
	*LcdData = 0x0000;
	
	*LcdReg = 0x0051;
	*LcdData = 0x00EF;

	*LcdReg = 0x0052;
	*LcdData = 0x0000;
	
	*LcdReg = 0x0053;
	*LcdData = 0x013F;
	//-------------------------------------
 	*LcdReg = 0x0060;
	*LcdData = 0xA700;
	
	*LcdReg = 0x0061;
	*LcdData = 0x0001;

	*LcdReg = 0x006A;
	*LcdData = 0x0000;
	
	*LcdReg = 0x0080;
	*LcdData = 0x0000;

	*LcdReg = 0x0081;
	*LcdData = 0x0000;

	*LcdReg = 0x0082;
	*LcdData = 0x0000;
	
	*LcdReg = 0x0083;
	*LcdData = 0x0000;

	*LcdReg = 0x0084;
	*LcdData = 0x0000;

	*LcdReg = 0x0085;
	*LcdData = 0x0000;

	*LcdReg = 0x0090;
	*LcdData = 0x0010;

	*LcdReg = 0x0092;
	*LcdData = 0x0600;

	*LcdReg = 0x0007;
	*LcdData = 0x0133;	

	*LcdReg = 0x00;
	*LcdData = 0x0022;

	return 0;
}
/**
 *@brief:      drv_ILI9325_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9325_xy2cp(u16 sx, u16 ex, u16 sy, u16 ey, u16 *hsa, u16 *hea, u16 *vsa, u16 *vea)
{
	struct _strlcd_obj *obj;
	obj = &LCD;

	/*
		显示XY轴范围
	*/
	if(sx >= obj->width)
		sx = obj->width-1;
	
	if(ex >= obj->width)
		ex = obj->width-1;
	
	if(sy >= obj->height)
		sy = obj->height-1;
	
	if(ey >= obj->height)
		ey = obj->height-1;
	/*
		XY轴，实物来看，方向取决于横屏还是竖屏
		CP轴，是控制器显存，
		映射关系取决于扫描方向
	*/
	/* 
		横屏，用户视角的XY坐标，跟LCD扫描的CP坐标要进行一个对调
		而且，9325在横竖屏也要进行映射
		
	*/
	if(obj->dir == W_LCD)
	{
		*hsa = (obj->height - ey) - 1;
		*hea = (obj->height - sy) - 1;
		
		*vsa = sx;
		*vea = ex;
	}
	else
	{
		*hsa = sx;
		*hea = ex;
		*vsa = sy;
		*vea = ey;
	}
	
	return 0;
}
/**
 *@brief:      drv_ILI9325_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_ILI9325_drawpoint(u16 x, u16 y, u16 color)
{
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(x, x, y, y, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(hsa, hea, vsa, vea);
	*LcdData = color; 
	return 0;
}
/**
 *@brief:      dev_ILI9325_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9325_color_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i,j;
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(hsa, hea, vsa, vea);

	width = hea - hsa + 1;//得到填充的宽度
	height = vea - vsa + 1;//高度
	
	//uart_printf("ili9325 width:%d, height:%d\r\n", width, height);

	for(i=0; i<height; i++)
	{
		//uart_printf("x:%d, y:%d\r\n", sx, sy+i);
		for(j=0; j<width; j++)
		{
			//Delay(1);
			*LcdData = color;//写入数据 
		}
		//uart_printf("\r\n");
	}	 

	return 0;

}

/**
 *@brief:      dev_ILI9325_color_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9325_fill(u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u16 i,j;
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(hsa, hea, vsa, vea);

	width=(hea +1) - hsa ;//得到填充的宽度 +1是因为坐标从0开始
	height=(vea +1) - vsa;//高度
	
	//uart_printf("ili9325 width:%d, height:%d\r\n", width, height);
	
	for(i=0; i<height; i++)
	{
		//uart_printf("x:%d, y:%d\r\n", sx, sy+i);
		for(j=0; j<width; j++)
		{
			//Delay(10);
			*LcdData = *(color+i*height+j);//写入数据 
		}
		//uart_printf("\r\n");
	}	 

	return 0;

} 

s32 drv_ILI9325_prepare_display(u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(hsa, hea, vsa, vea);	
	return 0;
}
#endif

/*
	以下函数是属于LCD层，统管所有LCD，
	目前只是为了测试彩屏而已，不完善，不建议使用
	请下载最新代码使用。
*/

/**
 *@brief:      dev_lcd_setdir
 *@details:    设置横屏或竖屏，扫描方向
 *@param[in]   u8 dir       0 竖屏1横屏
               u8 scan_dir  参考宏定义L2R_U2D       
 *@param[out]  无
 *@retval:     
 */
void dev_lcd_setdir(u8 dir, u8 scan_dir)
{
	struct _strlcd_obj *obj;
	obj = &LCD;

	u16 temp;
	u8 scan_dir_tmp;
	
	if(dir != obj->dir)//切换屏幕方向	
	{
		
		obj->dir = obj->dir^0x01;
		temp = obj->width;
		obj->width = obj->height;
		obj->height = temp;
		ILI9341_DEBUG(LOG_DEBUG, "set dir w:%d, h:%d\r\n", obj->width, obj->height);
	}
	
	
	if(obj->dir == W_LCD)//横屏，扫描方向映射转换
	{
		/*
			横屏	 竖屏
			LR----UD
			RL----DU
			UD----RL
			DU----LR
			UDLR----LRUD
		*/
		scan_dir_tmp = 0;
		if((scan_dir&LRUD_BIT_MASK) == 0)
		{
			scan_dir_tmp += LRUD_BIT_MASK;
		}

		if((scan_dir&LR_BIT_MASK) == LR_BIT_MASK)
		{
			scan_dir_tmp += UD_BIT_MASK;	
		}

		if((scan_dir&UD_BIT_MASK) == 0)
		{
			scan_dir_tmp += LR_BIT_MASK;
		}
	}
	else
	{
		scan_dir_tmp = scan_dir;
	}
	
	obj->scandir = scan_dir_tmp;
	
	obj->drv->set_dir(obj->scandir);
}

s32 dev_lcd_init(void)
{
	s32 ret = -1;
	
	/*初始化8080接口，包括背光信号*/
	bus_8080interface_init();

	if(ret != 0)
	{
		/*尝试初始化9341*/
		ret = drv_ILI9341_init();
		if(ret == 0)
		{
			LCD.drv = &TftLcdILI9341Drv;//将9341驱动赋值到LCD
			LCD.dir = H_LCD;//默认竖屏
			LCD.height = 320;
			LCD.width = 240;
		}
	}
	
	#ifdef TFT_LCD_DRIVER_9325
	if(ret != 0)
	{
		/* 尝试初始化9325 */
		ret = drv_ILI9325_init();
		if(ret == 0)
		{
			LCD.drv = &TftLcdILI9325Drv;
			LCD.dir = H_LCD;
			LCD.height = 320;
			LCD.width = 240;
		}
	}
	#endif
	/*设置屏幕方向，扫描方向*/
	dev_lcd_setdir(H_LCD, L2R_U2D);
	LCD.drv->onoff(1);//打开显示
	bus_8080_lcd_bl(1);//打开背光	
	LCD.drv->color_fill(0, LCD.width, 0, LCD.height, YELLOW);
	
	return 0;
}


s32 dev_lcd_drawpoint(u16 x, u16 y, u16 color)
{
	return LCD.drv->draw_point(x, y, color);
}

/* 

从tslib拷贝一些显示函数到这里
这些函数可以归为GUI


*/
#include "font.h"

/**
 *@brief:      line
 *@details:    画一条线
 *@param[in]   int x1           
               int y1           
               int x2           
               int y2           
               unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void line (int x1, int y1, int x2, int y2, unsigned colidx)
{
	int tmp;
	int dx = x2 - x1;
	int dy = y2 - y1;

	if (abs (dx) < abs (dy)) 
	{
		if (y1 > y2) 
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		x1 <<= 16;
		/* dy is apriori >0 */
		dx = (dx << 16) / dy;
		while (y1 <= y2)
		{
			LCD.drv->draw_point (x1 >> 16, y1, colidx);
			x1 += dx;
			y1++;
		}
	} 
	else 
	{
		if (x1 > x2) 
		{
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		
		y1 <<= 16;
		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2) 
		{
			LCD.drv->draw_point (x1, y1 >> 16, colidx);
			y1 += dy;
			x1++;
		}
	}
}

/**
 *@brief:     put_cross
 *@details:   画十字
 *@param[in]  int x            
              int y            
              unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void put_cross(int x, int y, unsigned colidx)
{
	line (x - 10, y, x - 2, y, colidx);
	line (x + 2, y, x + 10, y, colidx);
	line (x, y - 10, x, y - 2, colidx);
	line (x, y + 2, x, y + 10, colidx);

	line (x - 6, y - 9, x - 9, y - 9, colidx + 1);
	line (x - 9, y - 8, x - 9, y - 6, colidx + 1);
	line (x - 9, y + 6, x - 9, y + 9, colidx + 1);
	line (x - 8, y + 9, x - 6, y + 9, colidx + 1);
	line (x + 6, y + 9, x + 9, y + 9, colidx + 1);
	line (x + 9, y + 8, x + 9, y + 6, colidx + 1);
	line (x + 9, y - 6, x + 9, y - 9, colidx + 1);
	line (x + 8, y - 9, x + 6, y - 9, colidx + 1);

}
/**
 *@brief:      put_char
 *@details:    显示一个英文
 *@param[in]   int x       
               int y       
               int c       
               int colidx  
 *@param[out]  无
 *@retval:     
 */
void put_char(int x, int y, int c, int colidx)
{
	int i,j,bits;

	for (i = 0; i < font_vga_8x8.height; i++) 
	{
		bits = font_vga_8x8.data [font_vga_8x8.height * c + i];
		for (j = 0; j < font_vga_8x8.width; j++, bits <<= 1)
		{
			if (bits & 0x80)
			{
				LCD.drv->draw_point(x + j, y + i, colidx);
			}
		}
	}
}
/**
 *@brief:      put_string
 *@details:    显示一个字符串
 *@param[in]   int x            
               int y            
               char *s          
               unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void put_string(int x, int y, char *s, unsigned colidx)
{
	int i;
	
	for (i = 0; *s; i++, x += font_vga_8x8.width, s++)
		put_char(x, y, *s, colidx);
}
/**
 *@brief:      put_string_center
 *@details:    居中显示一个字符串
 *@param[in]   int x            
               int y            
               char *s          
               unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void put_string_center(int x, int y, char *s, unsigned colidx)
{
	int sl = strlen (s);
	
    put_string (x - (sl / 2) * font_vga_8x8.width,
                y - font_vga_8x8.height / 2, s, colidx);
}

/**
 *@brief:      rect
 *@details:    画一个矩形框
 *@param[in]   int x1           
               int y1           
               int x2           
               int y2           
               unsigned colidx  
 *@param[out]  无
 *@retval:     
 */
void rect (int x1, int y1, int x2, int y2, unsigned colidx)
{
	line (x1, y1, x2, y1, colidx);
	line (x2, y1, x2, y2, colidx);
	line (x2, y2, x1, y2, colidx);
	line (x1, y2, x1, y1, colidx);
}


/**
 *@brief:      dev_lcd_test
 *@details:    LCD测试函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void dev_lcd_test(void)
{

	while(1)
	{		
		LCD.drv->color_fill(0,LCD.width,0,LCD.height,BLUE);
		Delay(100);
		LCD.drv->color_fill(0,LCD.width/2,0,LCD.height/2,RED);
		Delay(100);
		LCD.drv->color_fill(0,LCD.width/4,0,LCD.height/4,GREEN);
		Delay(100);
		
		put_string_center (LCD.width/2+50, LCD.height/2+50,
			   "ADCD WUJIQUE !", 0xF800);
		Delay(100);
	}

}


