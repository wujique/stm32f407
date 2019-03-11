/**
 * @file            dev_ILI9325.c
 * @brief           TFT LCD 驱动芯片ILI9325驱动程序
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

#include "wujique_sysconf.h"

#include "stm324xg_eval_fsmc_sram.h"
#include "alloc.h"
#include "wujique_log.h"
#include "dev_lcdbus.h"
#include "dev_lcd.h"

extern void Delay(__IO uint32_t nTime);

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


s32 drv_ILI9325_init(DevLcdNode *lcd);
static s32 drv_ILI9325_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color);
s32 drv_ILI9325_color_fill(DevLcdNode *lcd, u16 sx,u16 sy,u16 ex,u16 ey,u16 color);
s32 drv_ILI9325_fill(DevLcdNode *lcd, u16 sx,u16 sy,u16 ex,u16 ey,u16 *color);
static s32 drv_ILI9325_display_onoff(DevLcdNode *lcd, u8 sta);
s32 drv_ILI9325_prepare_display(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep);
static void drv_ILI9325_scan_dir(DevLcdNode *lcd, u8 dir);
void drv_ILI9325_lcd_bl(DevLcdNode *lcd, u8 sta);
s32 drv_ILI9325_flush(DevLcdNode *lcd, u16 *color, u32 len);

/*

	9325驱动

*/
_lcd_drv TftLcdILI9325Drv = {
							.id = 0X9325,

							.init = drv_ILI9325_init,
							.draw_point = drv_ILI9325_drawpoint,
							.color_fill = drv_ILI9325_color_fill,
							.fill = drv_ILI9325_fill,
							.prepare_display = drv_ILI9325_prepare_display,
							.flush = drv_ILI9325_flush,
							
							.onoff = drv_ILI9325_display_onoff,
							.set_dir = drv_ILI9325_scan_dir,
							.backlight = drv_ILI9325_lcd_bl
							};
void drv_ILI9325_lcd_bl(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_bl(node, sta);
	bus_lcd_close(node);

}	
/**
 *@brief:	   drv_ILI9325_scan_dir
 *@details:    设置显存扫描方向
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:	   static OK
 */
static void drv_ILI9325_scan_dir(DevLcdNode *lcd, u8 dir)
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

	u16 tmp[16];
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	
	bus_lcd_write_cmd(node, (dirreg));
	tmp[0] = regval;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_close(node);

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
static s32 drv_ILI9325_set_cp_addr(DevLcdNode *lcd, u16 hsa, u16 hea, u16 vsa, u16 vea)
{
	u16 heatmp;
	u16 tmp[2];
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	/* 设置扫描窗口 */
	if((hsa+4) > hea)
		heatmp = hsa+4;
	else
		heatmp = hea;
	
	bus_lcd_write_cmd(node, (0x0050));//HSA
	tmp[0] = hsa;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0051));//HEA
	tmp[0] = heatmp;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0x0052));//VSA
	tmp[0] = vsa;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0053));//VEA
	tmp[0] = vea;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	/*
		设置扫描起始地址。
	*/
	if((lcd->scandir&LR_BIT_MASK) == LR_BIT_MASK)
	{
		bus_lcd_write_cmd(node, (ILI9325_CMD_SETH));
		tmp[0] = hea&0XFF;
		bus_lcd_write_data(node, (u8*)tmp, 1);
	
	}
	else
	{
		bus_lcd_write_cmd(node, (ILI9325_CMD_SETH));
		tmp[0] = hsa&0XFF;
		bus_lcd_write_data(node, (u8*)tmp, 1);
			  
	}

	if((lcd->scandir&UD_BIT_MASK) == UD_BIT_MASK)
	{
		bus_lcd_write_cmd(node, (ILI9325_CMD_SETV));
		tmp[0] = vea&0X1FF;
		bus_lcd_write_data(node, (u8*)tmp, 1);
		
	}
	else
	{
		bus_lcd_write_cmd(node, (ILI9325_CMD_SETV));
		tmp[0] = vsa&0X1FF;
		bus_lcd_write_data(node, (u8*)tmp, 1);
	}
	bus_lcd_write_cmd(node, (ILI9325_CMD_WRAM));
	bus_lcd_close(node);
	return 0;
}

/**
 *@brief:	   drv_ILI9325_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:	   static OK
 */
static s32 drv_ILI9325_display_onoff(DevLcdNode *lcd, u8 sta)
{
	u16 tmp[2];
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	if(sta == 1)
	{
		bus_lcd_write_cmd(node, (0X07));
		tmp[0] = 0x0173;
		bus_lcd_write_data(node, (u8*)tmp, 1);
	}
	else
	{
		bus_lcd_write_cmd(node, (0X07));
		tmp[0] = 0x00;
		bus_lcd_write_data(node, (u8*)tmp, 1);
	}
	bus_lcd_close(node);
	return 0;
}

/**
 *@brief:	   drv_ILI9325_init
 *@details:    初始化FSMC，并且读取ILI9325的设备ID
 *@param[in]   void  
 *@param[out]  无
 *@retval:	   
 */
s32 drv_ILI9325_init(DevLcdNode *lcd)
{
	u16 data;
	u16 tmp[16];
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	bus_lcd_rst(node, 1);
	Delay(50);
	bus_lcd_rst(node, 0);
	Delay(50);
	bus_lcd_rst(node, 1);
	Delay(50);
	/*
		读9325的ID
		
	*/
	bus_lcd_write_cmd(node, (0x0000));
	tmp[0] = 0x0001;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_write_cmd(node, (0x0000));
	bus_lcd_read_data(node, (u8*)tmp, 1);
    data = tmp[0]; 

	wjq_log(LOG_DEBUG, "read reg:%04x\r\n", data);
	if(data != TftLcdILI9325Drv.id)
	{
		wjq_log(LOG_DEBUG, "lcd drive no 9325\r\n");	
		bus_lcd_close(node);
		return -1;
	}
	
	bus_lcd_write_cmd(node, (0x00E5));
	tmp[0] = 0x78F0;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0001));
	tmp[0] = 0x0100;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0002));
	tmp[0] = 0x0700;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0003));
	tmp[0] = 0x1030;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0004));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0008));
	tmp[0] = 0x0202;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0009));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x000A));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x000C));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x000D));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x000F));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0010));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0011));
	tmp[0] = 0x0007;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0012));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0013));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0007));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0010));
	tmp[0] = 0x1690;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0011));
	tmp[0] = 0x0227;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0012));
	tmp[0] = 0x009D;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0013));
	tmp[0] = 0x1900;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0029));
	tmp[0] = 0x0025;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x002B));
	tmp[0] = 0x000D;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0030));
	tmp[0] = 0x0007;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0031));
	tmp[0] = 0x0303;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0032));
	tmp[0] = 0x0003;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0035));
	tmp[0] = 0x0206;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0036));
	tmp[0] = 0x0008;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0037));
	tmp[0] = 0x0406;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0038));
	tmp[0] = 0x0304;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0039));
	tmp[0] = 0x0007;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x003C));
	tmp[0] = 0x0602;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x003D));
	tmp[0] = 0x0008;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	/*
	Horizontal and Vertical RAM Address Position 219*319
	设置扫描窗口

	*/
	bus_lcd_write_cmd(node, (0x0050));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0051));
	tmp[0] = 0x00EF;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0052));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0053));
	tmp[0] = 0x013F;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	//-------------------------------------
	bus_lcd_write_cmd(node, (0x0060));
	tmp[0] = 0xA700;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0061));
	tmp[0] = 0x0001;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x006A));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0080));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0081));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0082));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0083));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);	

	bus_lcd_write_cmd(node, (0x0084));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0085));
	tmp[0] = 0x0000;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0090));
	tmp[0] = 0x0010;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0092));
	tmp[0] = 0x0600;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x0007));
	tmp[0] = 0x0133;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_write_cmd(node, (0x00));
	tmp[0] = 0x0022;
	bus_lcd_write_data(node, (u8*)tmp, 1);

	bus_lcd_close(node);
	return 0;
}
/**
 *@brief:      drv_ILI9325_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_ILI9325_xy2cp(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 *hsa, u16 *hea, u16 *vsa, u16 *vea)
{
	/*
		显示XY轴范围
	*/
	if(sx >= lcd->width)
		sx = lcd->width-1;
	
	if(ex >= lcd->width)
		ex = lcd->width-1;
	
	if(sy >= lcd->height)
		sy = lcd->height-1;
	
	if(ey >= lcd->height)
		ey = lcd->height-1;
	/*
		XY轴，实物来看，方向取决于横屏还是竖屏
		CP轴，是控制器显存，
		映射关系取决于扫描方向
	*/
	/* 
		横屏，用户视角的XY坐标，跟LCD扫描的CP坐标要进行一个对调
		而且，9325在横竖屏也要进行映射
		
	*/
	if(lcd->dir == W_LCD)
	{
		*hsa = (lcd->height - ey) - 1;
		*hea = (lcd->height - sy) - 1;
		
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
static s32 drv_ILI9325_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(lcd, x, x, y, y, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(lcd, hsa, hea, vsa, vea);
	
	DevLcdBusNode * node;
	node = bus_lcd_open(lcd->dev.buslcd);
	
	u16 tmp[2];
	tmp[0] = color;
	bus_lcd_write_data(node, (u8*)tmp, 1);
	bus_lcd_close(node);
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
s32 drv_ILI9325_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{

	u16 height,width;
	u16 i,j;
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(lcd, sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(lcd, hsa, hea, vsa, vea);

	width = hea - hsa + 1;//得到填充的宽度
	height = vea - vsa + 1;//高度
	
	//uart_printf("ili9325 width:%d, height:%d\r\n", width, height);
	//GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	
	DevLcdBusNode * node;
	#define TMP_BUF_SIZE 32
	u16 tmp[TMP_BUF_SIZE];
	u32 cnt;

	for(cnt = 0; cnt < TMP_BUF_SIZE; cnt ++)
	{
		tmp[cnt] = color;
	}
	
	cnt = height*width;
	
	node = bus_lcd_open(lcd->dev.buslcd);

	while(1)
	{
		if(cnt < TMP_BUF_SIZE)
		{
			bus_lcd_write_data(node, (u8 *)tmp, cnt);
			cnt -= cnt;
		}
		else
		{
			bus_lcd_write_data(node, (u8 *)tmp, TMP_BUF_SIZE);
			cnt -= TMP_BUF_SIZE;
		}

		if(cnt <= 0)
			break;
	}
	
	bus_lcd_close(node);

	//GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
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
s32 drv_ILI9325_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{

	u16 height,width;
	u16 i,j;
	u16 hsa,hea,vsa,vea;

	drv_ILI9325_xy2cp(lcd, sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);
	drv_ILI9325_set_cp_addr(lcd, hsa, hea, vsa, vea);

	width=(hea +1) - hsa ;//得到填充的宽度 +1是因为坐标从0开始
	height=(vea +1) - vsa;//高度
	
	//uart_printf("ili9325 width:%d, height:%d\r\n", width, height);
	//GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);

	DevLcdBusNode * node;

	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_write_data(node, (u8 *)color, height*width);	
	bus_lcd_close(node);

	//GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
	
	return 0;

} 

s32 drv_ILI9325_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	u16 hsa,hea,vsa,vea;

	wjq_log(LOG_DEBUG, "XY:-%d-%d-%d-%d-\r\n", sx, ex, sy, ey);

	drv_ILI9325_xy2cp(lcd, sx, ex, sy, ey, &hsa,&hea,&vsa,&vea);

	wjq_log(LOG_DEBUG, "HV:-%d-%d-%d-%d-\r\n", hsa, hea, vsa, vea);
	
	drv_ILI9325_set_cp_addr(lcd, hsa, hea, vsa, vea);	
	return 0;
}
s32 drv_ILI9325_flush(DevLcdNode *lcd, u16 *color, u32 len)
{
	lcd->busnode = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_write_data(lcd->busnode, (u8 *)color,  len);	
	bus_lcd_close(lcd->busnode);
	return 0;
} 

#endif


