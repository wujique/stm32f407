/**
 * @file            dev_IL91874.c
 * @brief           TFT LCD 驱动芯片IL91874驱动程序
 * @author          wujique
 * @date            2017年11月8日 星期五
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年11月8日 星期五
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

/*
	91874 大连佳显 2.7寸电子纸 三色屏
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
#include "dev_IL91874.h"

#define DEV_IL91874_DEBUG

#ifdef DEV_IL91874_DEBUG
#define IL91874_DEBUG	wjq_log 
#else
#define IL91874_DEBUG(a, ...)
#endif

extern void Delay(__IO uint32_t nTime);


/*
	备忘：
	1 91874的通信，每个字节都需要操作CS，很特殊，暂时放在本驱动，先不修改LCD BUS。
	2 91874有一根BUSY管脚，这个管脚，应该是属于LCD 硬件接口层的。放在本驱动不是很合适。

	3 电子纸刷新很慢，最好能改为：指定显示区域，填充显示缓冲，refresh。三步式。
	  等评估后，将所有LCD的驱动接口改为支持三步式。
*/

/*

	电子纸的显存扫描方向：
	gram[0], 是竖屏时左上角横向8个点。
	这是做些color fill和fill函数要考虑的。

	例如大连佳显的2.74寸 176*264像素的
	一个page就是竖屏的一行X轴方向的点。
	也就是说，一个page有 176/8 个字节，
	LCD总共有264page，竖屏(0,0)是page0第1个字节的bit7。

*/
#define IL91874_PAGE_SIZE ((lcd->dev.width+7)/8)

struct _epaper_drv_data
{
	/*显存，通过动态申请，根据屏幕大小申请*/
	u8 *bgram;
	u8 *rgram;
	
	/*刷新区域*/
	u16 sx;
	u16 ex;
	u16 sy;
	u16 ey;
	u16 disx;
	u16 disy;

};



#ifdef TFT_LCD_DRIVER_91874

s32 drv_IL91874_init(DevLcdNode *lcd);
static s32 drv_IL91874_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color);
s32 drv_IL91874_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color);
s32 drv_IL91874_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color);
static s32 drv_IL91874_display_onoff(DevLcdNode *lcd, u8 sta);
s32 drv_IL91874_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey);
static void drv_IL91874_scan_dir(DevLcdNode *lcd, u8 dir);
void drv_IL91874_lcd_bl(DevLcdNode *lcd, u8 sta);
s32 drv_IL91874_flush(DevLcdNode *lcd, u16 *color, u32 len);
s32 drv_IL91874_update(DevLcdNode *lcd);


/*

	定义一个TFT LCD，使用IL91874驱动IC的设备

*/
_lcd_drv TftLcdIL91874Drv = {
							.id = 0X9187,

							.init = drv_IL91874_init,
							.draw_point = drv_IL91874_drawpoint,
							.color_fill = drv_IL91874_color_fill,
							.fill = drv_IL91874_fill,
							.onoff = drv_IL91874_display_onoff,
							.prepare_display = drv_IL91874_prepare_display,
							.flush = drv_IL91874_flush,
							.set_dir = drv_IL91874_scan_dir,
							.backlight = drv_IL91874_lcd_bl,
							.update = drv_IL91874_update,
							};

void SPI_Delay(unsigned char xrate)
{
	unsigned char i;
	while(xrate)
	{
		for(i=0;i<2;i++);
		xrate--;
	}
}


/*
	IL91874, 每个字节的SPI通信都需要CS下降沿
	为了不影响LCD BUS，对CS的操作放在驱动中，
	属于一种特殊情况

	mcu_spi_cs
*/
s32 drv_il91874_write_cmd(DevLcdBusNode *node, u8 cmd)
{
	mcu_spi_cs((DevSpiChNode *)node->basenode, 0);
	bus_lcd_write_cmd(node, cmd);
	mcu_spi_cs((DevSpiChNode *)node->basenode, 1);
}

s32 drv_il91874_write_data(DevLcdBusNode *node, u8 *data, u32 len)
{
	mcu_spi_cs((DevSpiChNode *)node->basenode, 0);
	bus_lcd_write_data(node, data, len);
	mcu_spi_cs((DevSpiChNode *)node->basenode, 1);
}


/*
	墨水屏没有背光，有前置光源
*/
void drv_IL91874_lcd_bl(DevLcdNode *lcd, u8 sta)
{
	DevLcdBusNode * node;
	
	node = bus_lcd_open(lcd->dev.buslcd);
	bus_lcd_bl(node, sta);
	bus_lcd_close(node);

}
	
/**
 *@brief:      drv_IL91874_scan_dir
 *@details:    设置显存扫描方向， 本函数为竖屏角度
 *@param[in]   u8 dir  
 *@param[out]  无
 *@retval:     static
 */
static void drv_IL91874_scan_dir(DevLcdNode *lcd, u8 dir)
{

}

/**
 *@brief:      drv_IL91874_set_cp_addr
 *@details:    设置控制器的行列地址范围
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL91874_set_cp_addr(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{

}
/**
 *@brief:      drv_IL91874_refresh_gram
 *@details:       刷新指定区域到屏幕上
                  坐标是横屏模式坐标
 *@param[in]   u16 sc  
               u16 ec  
               u16 sp  
               u16 ep  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_IL91874_refresh_gram(DevLcdNode *lcd, u16 sc, u16 ec, u16 sp, u16 ep)
{	
	struct _epaper_drv_data *drvdata; 

	DevLcdBusNode *node;
	u32 cnt;
	u32 gramsize;
	u16 i,j;
	u8 data;
	u16 w,l;
	
	drvdata = (struct _epaper_drv_data *)lcd->pri;
	node = bus_lcd_open(lcd->dev.buslcd);

	wjq_log(LOG_DEBUG, "drv_IL91874_refresh_gram: %d, %d, %d, %d\r\n ", sc, ec, sp, ep);
	
	#if 0 /*局部刷*/
	/* 对SC和w进行校正，必须是8的整数倍 */
	sc = sc&0xfff8;
	w = ec-sc+1;
	l = ep-sp+1;
	w = w&0xfff8;
	
	wjq_log(LOG_DEBUG, "drv_IL91874_refresh_gram: %d, %d, %d, %d\r\n ", sc, sp, w, l);
	
	
	bus_lcd_write_cmd(node, (0x14));
	data = sc>>8;
	bus_lcd_write_data(node, &data, 1);
	data = sc&0xff;
	bus_lcd_write_data(node, &data, 1);

	data = sp>>8;
	bus_lcd_write_data(node, &data, 1);
	data = sp&0xff;
	bus_lcd_write_data(node, &data, 1);

	data = w>>8;
	bus_lcd_write_data(node, &data, 1);
	data = w&0xff;
	bus_lcd_write_data(node, &data, 1);
	
	data = l>>8;
	bus_lcd_write_data(node, &data, 1);
	data = l&0xff;
	bus_lcd_write_data(node, &data, 1);


	for(j=0;j < l;j++)
		for(i=0;i < w/8;i++)
			bus_lcd_write_data(node, (u8 *)&drvdata->bgram[j*IL91874_PAGE_SIZE + i], 1);
		
	bus_lcd_write_cmd(node, (0x15));
	data = sc>>8;
	bus_lcd_write_data(node, &data, 1);
	data = sc&0xff;
	bus_lcd_write_data(node, &data, 1);

	data = sp>>8;
	bus_lcd_write_data(node, &data, 1);
	data = sp&0xff;
	bus_lcd_write_data(node, &data, 1);

	data = w>>8;
	bus_lcd_write_data(node, &data, 1);
	data = w&0xff;
	bus_lcd_write_data(node, &data, 1);
	
	data = l>>8;
	bus_lcd_write_data(node, &data, 1);
	data = l&0xff;
	bus_lcd_write_data(node, &data, 1);
	
	for(j=0;j < l;j++)
		for(i=0;i < w/8;i++)
			bus_lcd_write_data(node, (u8 *)&drvdata->rgram[j*IL91874_PAGE_SIZE + i], 1);

		
	wjq_log(LOG_DEBUG, " DISPLAY REFRESH\r\n ");

	bus_lcd_write_cmd(node, (0x16));//DISPLAY REFRESH 	
	data = sc>>8;
	bus_lcd_write_data(node, &data, 1);
	data = sc&0xff;
	bus_lcd_write_data(node, &data, 1);

	data = sp>>8;
	bus_lcd_write_data(node, &data, 1);
	data = sp&0xff;
	bus_lcd_write_data(node, &data, 1);

	data = w>>8;
	bus_lcd_write_data(node, &data, 1);
	data = w&0xff;
	bus_lcd_write_data(node, &data, 1);
	
	data = l>>8;
	bus_lcd_write_data(node, &data, 1);
	data = l&0xff;
	bus_lcd_write_data(node, &data, 1);
	#endif

	#if 1 /*全刷*/
	/*注意，要用dev中的w和h，因为gram跟横屏竖屏调换没关系，
	只和定义一致*/
	gramsize = lcd->dev.height * IL91874_PAGE_SIZE;
	//wjq_log(LOG_DEBUG, "gram size: %d\r\n ", gramsize);


	drv_il91874_write_cmd(node, (0x10));
	for(cnt = 0; cnt <gramsize; cnt++)
		drv_il91874_write_data(node, (u8 *)&drvdata->bgram[cnt], 1);

	drv_il91874_write_cmd(node, (0x13));
	for(cnt = 0; cnt <gramsize; cnt++)
		drv_il91874_write_data(node, (u8 *)&drvdata->rgram[cnt], 1);
	
	wjq_log(LOG_DEBUG, " DISPLAY REFRESH\r\n ");

	drv_il91874_write_cmd(node, (0x12));//DISPLAY REFRESH 	
	#endif

	Delay(1);//!!!The delay here is necessary, 200uS at least!!!  
	
	unsigned char busy;
	do
	{
		drv_il91874_write_cmd(node, (0x71));
		busy = mcu_io_input_readbit(node->dev.staport, node->dev.stapin);
		busy =!(busy & 0x01);        
	}
	while(busy);  
	wjq_log(LOG_DEBUG, "IL91874 refresh finish\r\n ");
	bus_lcd_close(node);
	
	return 0;
}

/**
 *@brief:      drv_IL91874_display_onoff
 *@details:    显示或关闭
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_IL91874_display_onoff(DevLcdNode *lcd, u8 sta)
{
	wjq_log(LOG_DEBUG, " drv_IL91874_display_onoff\r\n ");	
	return 0;
}

/**
 *@brief:      drv_IL91874_init
 *@details:    
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL91874_init(DevLcdNode *lcd)
{
	u16 data;
	DevLcdBusNode * node;
	u8 tmp[16];
	u8 testbuf[2];
	
	node = bus_lcd_open(lcd->dev.buslcd);

	mcu_io_config_in(node->dev.staport, node->dev.stapin);

	bus_lcd_rst(node, 1);
	Delay(50);
	bus_lcd_rst(node, 0);
	Delay(1000);
	bus_lcd_rst(node, 1);
	Delay(1000);

	
	drv_il91874_write_cmd(node, (0x06));//boost soft start
	tmp[0] = 0x07;
	drv_il91874_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0x07;
	drv_il91874_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0x17;
	drv_il91874_write_data(node, (u8*)tmp, 1);
     
	drv_il91874_write_cmd(node, (0x04)); 
	
	unsigned char busy;
	do
	{
		drv_il91874_write_cmd(node, (0x71));
		busy = mcu_io_input_readbit(node->dev.staport, node->dev.stapin);
		busy =!(busy & 0x01);        
	}
	while(busy);   
	Delay(200); 

	drv_il91874_write_cmd(node, (0x00));//panel setting	
	tmp[0] = 0x0f;
	drv_il91874_write_data(node, (u8*)tmp, 1);//LUT from OTP，128x296
	drv_il91874_write_cmd(node, (0x0d));//VCOM to 0V fast

	drv_il91874_write_cmd(node, (0x16));
	tmp[0] = 0x00;
	drv_il91874_write_data(node, (u8*)tmp, 1);//KW-BF   KWR-AF	BWROTP 0f	

	drv_il91874_write_cmd(node, (0xF8));         //boost设定
	tmp[0] = 0x60;
	drv_il91874_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0xa5;
	drv_il91874_write_data(node, (u8*)tmp, 1);
	
	drv_il91874_write_cmd(node, (0xF8));         //boost设定
	tmp[0] = 0x73;
	drv_il91874_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0x23;
	drv_il91874_write_data(node, (u8*)tmp, 1);
	
	drv_il91874_write_cmd(node, (0xF8));        //boost设定
	tmp[0] = 0x7C;
	drv_il91874_write_data(node, (u8*)tmp, 1);
	tmp[0] = 0x00;
	drv_il91874_write_data(node, (u8*)tmp, 1);
	
	bus_lcd_close(node);
	
	Delay(50);

	
	/*申请显存，永不释放*/
	lcd->pri = (void *)wjq_malloc(sizeof(struct _epaper_drv_data));
	memset((char*)lcd->pri, 0x00, sizeof(struct _epaper_drv_data));

	struct _epaper_drv_data *p;
	u16 gramsize;

	/*三色电子纸，要两个缓冲*/
	p = (struct _epaper_drv_data *)lcd->pri;

	gramsize = lcd->dev.height * IL91874_PAGE_SIZE;
	wjq_log(LOG_DEBUG, "gram size: %d\r\n ", gramsize);

	
	p->bgram = (u8 *)wjq_malloc(gramsize);
	p->rgram = (u8 *)wjq_malloc(gramsize);
	
	wjq_log(LOG_DEBUG, "drv_IL91874_init finish\r\n ");

	return 0;
}
/**
 *@brief:      drv_IL91874_xy2cp
 *@details:    将xy坐标转换为CP坐标
 *@param[in]   无
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL91874_xy2cp(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 *sc, u16 *ec, u16 *sp, u16 *ep)
{

	return 0;
}
/**
 *@brief:      drv_IL91874_drawpoint
 *@details:    画点
 *@param[in]   u16 x      
               u16 y      
               u16 color  
 *@param[out]  无
 *@retval:     static
 */
static s32 drv_IL91874_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	wjq_log(LOG_DEBUG, " drv_IL91874_drawpoint------\r\n ");
	return 0;
}
/**
 *@brief:      drv_IL91874_color_fill
 *@details:    将一块区域设定为某种颜色
 *@param[in]   u16 sx     
               u16 sy     
               u16 ex     
               u16 ey     
               u16 color  
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL91874_color_fill(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey, u16 color)
{
	u16 i,j;
	u16 xtmp,ytmp;
	u16 page, colum;
	u16 sp, ep, sc, ec;
	
	struct _epaper_drv_data *drvdata;

	wjq_log(LOG_DEBUG, " drv_IL91874_color_fill\r\n ");

	drvdata = (struct _epaper_drv_data *)lcd->pri;

	/*防止坐标溢出*/
	if(sy >= lcd->height)
	{
		sy = lcd->height-1;
	}
	if(sx >= lcd->width)
	{
		sx = lcd->width-1;
	}
	
	if(ey >= lcd->height)
	{
		ey = lcd->height-1;
	}
	if(ex >= lcd->width)
	{
		ex = lcd->width-1;
	}

	/*求出被改动的page和cloum*/
	if(lcd->dir == H_LCD)
	{
		sp = sy;
		ep = ey;

		sc = sx;
		ec = ex;
			
	}
	else//如果是竖屏，XY轴跟显存的映射要对调
	{
		sp = sx;
		ep = ex;

		ec = (lcd->height - sy);
		sc = (lcd->height - ey);
	}

	for(j=sy;j<=ey;j++)
	{
		//wjq_log(LOG_DEBUG, "\r\n");
		
		for(i=sx;i<=ex;i++)
		{

			if(lcd->dir == H_LCD)
			{
				xtmp = i;
				ytmp = j;
			}
			else//如果是竖屏，XY轴跟显存的映射要对调
			{
				/* 不同的算法，相当于不同的扫描方式*/
			
				//xtmp = j;
				//ytmp = lcd->width-i;

				xtmp = lcd->height - 1 - j;
				ytmp = i;
			}

			page = ytmp; //页地址
			colum = xtmp/8;//列地址
		

			if(color == RED)
			{
				drvdata->rgram[page*IL91874_PAGE_SIZE + colum] |= (0x80>>(xtmp%8));
				drvdata->bgram[page*IL91874_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				//uart_printf("*");
			}
			else if(color == BLACK)
			{
				drvdata->bgram[page*IL91874_PAGE_SIZE + colum] |= (0x80>>(xtmp%8));
				drvdata->rgram[page*IL91874_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				//uart_printf("*");
			}	
			else
			{
				drvdata->bgram[page*IL91874_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				drvdata->rgram[page*IL91874_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				//uart_printf("-");
			}
		}
	}

	/*
		只刷新需要刷新的区域
		坐标范围是横屏模式
	*/
	//drv_IL91874_refresh_gram(lcd, sc, ec, sp, ep);
	wjq_log(LOG_DEBUG, " drv_IL91874_color_fill finish\r\n ");

	return 0;
}

/**
 *@brief:      drv_IL91874_fill
 *@details:    填充矩形区域
 *@param[in]   u16 sx      
               u16 sy      
               u16 ex      
               u16 ey      
               u16 *color  每一个点的颜色数据
 *@param[out]  无
 *@retval:     
 */
s32 drv_IL91874_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{
	u16 i,j;
	u16 xtmp,ytmp;
	u16 xlen,ylen;
	u16 page, colum;
	u32 index;
	u16 cdata;
	u16 sp, ep, sc, ec;
	
	struct _epaper_drv_data *drvdata;

	wjq_log(LOG_DEBUG, " drv_IL91874_fill:%d,%d,%d,%d\r\n", sx,ex,sy,ey);
	
	drvdata = (struct _epaper_drv_data *)lcd->pri;

	/*xlen跟ylen是用来取数据的，不是填LCD*/
	xlen = ex-sx+1;//全包含
	ylen = ey-sy+1;

	/*防止坐标溢出*/
	if(sy >= lcd->height)
	{
		sy = lcd->height-1;
	}
	
	if(sx >= lcd->width)
	{
		sx = lcd->width-1;
	}
	
	if(ey >= lcd->height)
	{
		ey = lcd->height-1;
	}
	if(ex >= lcd->width)
	{
		ex = lcd->width-1;
	}

	/*求出被改动的page和cloum*/
	if(lcd->dir == H_LCD)
	{
		sp = sy;
		ep = ey;

		sc = sx;
		ec = ex;
			
	}
	else//如果是竖屏，XY轴跟显存的映射要对调
	{
		sp = sx;
		ep = ex;

		ec = (lcd->height - sy);
		sc = (lcd->height - ey);
	}
	
	for(j=sy;j<=ey;j++)
	{
		//uart_printf("\r\n");

		index = (j-sy)*xlen;
		
		for(i=sx;i<=ex;i++)
		{

			if(lcd->dir == H_LCD)
			{
				xtmp = i;
				ytmp = j;
			}
			else
			{
				xtmp = lcd->height - 1 - j;
				ytmp = i;
			}
			
			page = ytmp; //页地址
			colum = xtmp/8;//列地址
			
			cdata = *(color+index+i-sx);

			if(cdata == RED)
			{
				drvdata->rgram[page*IL91874_PAGE_SIZE + colum] |= (0x80>>(xtmp%8));
				drvdata->bgram[page*IL91874_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				//uart_printf("*");
			}
			else if(cdata == BLACK)
			{
				drvdata->bgram[page*IL91874_PAGE_SIZE + colum] |= (0x80>>(xtmp%8));
				drvdata->rgram[page*IL91874_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				//uart_printf("*");
			}	
			else
			{
				drvdata->bgram[page*IL91874_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				drvdata->rgram[page*IL91874_PAGE_SIZE + colum] &= ~(0x80>>(xtmp%8));
				//uart_printf("-");
			}
		}
	}

	/*
		只刷新需要刷新的区域
		坐标范围是横屏模式
	*/
	//drv_IL91874_refresh_gram(lcd, sc,ec,sp,ep);
	//uart_printf("refresh ok\r\n");		
	return 0;
}


s32 drv_IL91874_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	wjq_log(LOG_DEBUG, " drv_IL91874_prepare_display\r\n ");
	return 0;
}

s32 drv_IL91874_flush(DevLcdNode *lcd, u16 *color, u32 len)
{
	wjq_log(LOG_DEBUG, " drv_IL91874_flush\r\n ");
	return 0;
} 

s32 drv_IL91874_update(DevLcdNode *lcd)
{
	drv_IL91874_refresh_gram(lcd, 0, 0, 0, 0);	
	return 0;
}

#endif


