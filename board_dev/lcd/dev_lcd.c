/**
 * @file            dev_lcd.c
 * @brief           LCD 中间层
 * @author          wujique
 * @date            2018年4月17日 星期二
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年4月17日 星期二
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
#include "string.h"
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "list.h"
#include "alloc.h"

#include "dev_lcdbus.h"
#include "dev_lcd.h"

#include "dev_ILI9341.h"
#include "dev_str7565.h"
#include "dev_IL91874.h"
#include "dev_IL3820.h"
#include "dev_ILI9325.h"
#include "dev_st7789.h"
#include "dev_st7735r.h"

#define DEV_LCD_DEBUG

#ifdef DEV_LCD_DEBUG
#define LCD_DEBUG	wjq_log 
#else
#define LCD_DEBUG(a, ...)
#endif

s32 LcdMagNum = 95862;

u16 PenColor = BLACK;
u16 BackColor = BLUE;


/*
	所有驱动列表
*/
_lcd_drv *LcdDrvList[] = {
					&TftLcdILI9341Drv,
					&TftLcdILI9325Drv,
					&CogLcdST7565Drv,
					&OledLcdSSD1615rv,
					&TftLcdILI9341_8_Drv,
					&TftLcdST7735R_Drv,
					&TftLcdST7789_Drv,
					&TftLcdIL91874Drv,
					&TftLcdIL3820Drv,
};
/*

	可自动识别ID的驱动

*/
_lcd_drv *LcdProbDrv8080List[] = {
					&TftLcdILI9341Drv,
					&TftLcdILI9325Drv,
};


/**
 *@brief:      dev_lcd_finddrv
 *@details:    根据ID查找设备驱动
 *@param[in]   u16 id  
 *@param[out]  无
 *@retval:     _lcd_drv
 */
static _lcd_drv *dev_lcd_finddrv(u16 id)
{
	u8 i =0;
	
	while(1)
	{
		if(LcdDrvList[i]->id == id)
		{
			return LcdDrvList[i];
		}
		i++;
		if(i>= sizeof(LcdDrvList)/sizeof(_lcd_drv *))
		{
			return NULL;
		}
	}
}

struct list_head DevLcdRoot = {&DevLcdRoot, &DevLcdRoot};	
/**
 *@brief:      dev_lcd_register
 *@details:    注册LCD设备
 *@param[in]   
 *@param[out]  
 *@retval:     
 */
s32 dev_lcd_register(const DevLcd *dev)
{
	struct list_head *listp;
	DevLcdNode *plcdnode;
	s32 ret = -1;
	
	wjq_log(LOG_INFO, "[register] lcd :%s, base on:%s!\r\n", dev->name, dev->buslcd);

	/*
		先要查询当前，防止重名
	*/
	listp = DevLcdRoot.next;
	while(1)
	{
		if(listp == &DevLcdRoot)
			break;

		plcdnode = list_entry(listp, DevLcdNode, list);

		if(strcmp(dev->name, plcdnode->dev.name) == 0)
		{
			wjq_log(LOG_INFO, "lcd dev name err!\r\n");
			return -1;
		}

		if(strcmp(dev->buslcd, plcdnode->dev.buslcd) == 0)
		{
			wjq_log(LOG_INFO, "one lcd bus just for one lcd!\r\n");
			return -1;
		}
		
		listp = listp->next;
	}

	/* 
		申请一个节点空间 
	*/
	plcdnode = (DevLcdNode *)wjq_malloc(sizeof(DevLcdNode));
	list_add(&(plcdnode->list), &DevLcdRoot);
	
	/*复制设备信息*/
	memcpy((u8 *)&plcdnode->dev, (u8 *)dev, sizeof(DevLcd));
	plcdnode->gd = -1;

	/*初始化*/
	if(dev->id == NULL)
	{
		LCD_DEBUG(LOG_DEBUG, "prob LCD id\r\n");

		/*找到驱动跟规格后，初始化*/
		u8 j = 0;

		while(1)
		{
			ret = LcdProbDrv8080List[j]->init(plcdnode);
			if(ret == 0)
			{
				LCD_DEBUG(LOG_DEBUG, "lcd drv prob ok!\r\n");	
				plcdnode->drv = LcdProbDrv8080List[j];
				break;
			}	
			else
			{
				j++;
				if(j >= sizeof(LcdProbDrv8080List)/sizeof(_lcd_drv *))
				{
					LCD_DEBUG(LOG_DEBUG, "lcd prob err\r\n");
					break;
				}
			}
		}

	}
	else
	{
		ret = -1;
		
		plcdnode->drv = dev_lcd_finddrv(dev->id);
		if(plcdnode->drv != NULL)
		{
			/*找到驱动跟规格后，初始化*/
			ret = plcdnode->drv->init(plcdnode);

		}
		else
		{
			
			LCD_DEBUG(LOG_DEBUG, "lcd find drv fail!\r\n");
		}
	}

	if(ret == 0)
	{
		plcdnode->gd = -1;
		
		plcdnode->dir = H_LCD;
		
		plcdnode->height = plcdnode->dev.height;
		plcdnode->width = plcdnode->dev.width;
		
		dev_lcd_setdir(plcdnode, W_LCD, L2R_U2D);
		
		plcdnode->drv->onoff((plcdnode),1);//打开显示
		
		plcdnode->drv->color_fill(plcdnode, 0, plcdnode->width, 0, plcdnode->height, BLUE);
		plcdnode->drv->update(plcdnode);
		plcdnode->drv->backlight(plcdnode, 1);

		wjq_log(LOG_INFO, "lcd init OK\r\n");
	}
	else
	{
		plcdnode->gd = -2;
		wjq_log(LOG_INFO, "lcd drv init err!\r\n");
	}
	
	return 0;
}


/**
 *@brief:      dev_lcd_open
 *@details:    打开LCD
 *@param[in]   char *name  
 *@param[out]  无
 *@retval:     DevLcd
 */
DevLcdNode *dev_lcd_open(char *name)
{

	DevLcdNode *node;
	struct list_head *listp;
	
	//LCD_DEBUG(LOG_INFO, "lcd open:%s!\r\n", name);

	listp = DevLcdRoot.next;
	node = NULL;
	
	while(1)
	{
		if(listp == &DevLcdRoot)
			break;

		node = list_entry(listp, DevLcdNode, list);
		//LCD_DEBUG(LOG_INFO, "lcd name:%s!\r\n", node->dev.name);
		
		if(strcmp(name, node->dev.name) == 0)
		{
			//LCD_DEBUG(LOG_INFO, "lcd dev get ok!\r\n");
			break;
		}
		else
		{
			node = NULL;
		}
		
		listp = listp->next;
	}

	if(node != NULL)
	{
		if(node->gd > (-2))
			node->gd++;
		else
			return NULL;
	}
	
	return node;
}

/**
 *@brief:      dev_lcd_close
 *@details:    关闭LCD
 *@param[in]   DevLcd *dev  
 *@param[out]  无
 *@retval:     
 */
s32 dev_lcd_close(DevLcdNode *node)
{
	if(node->gd <0)
		return -1;
	else
	{
		node->gd -= 1;
		return 0;
	}
}
/*
坐标-1 是坐标原点的变化，
在APP层，原点是（1，1），这样更符合平常人。

到驱动就换为(0,0)，无论程序还是控制器显存，都是从（0，0）开始

*/
s32 dev_lcd_drawpoint(DevLcdNode *lcd, u16 x, u16 y, u16 color)
{
	if(lcd == NULL)
		return -1;
	
	return lcd->drv->draw_point(lcd, x-1, y-1, color);
}

s32 dev_lcd_prepare_display(DevLcdNode *lcd, u16 sx, u16 ex, u16 sy, u16 ey)
{
	if(lcd == NULL)
		return -1;
	
	return lcd->drv->prepare_display(lcd, sx-1, ex-1, sy-1, ey-1);
}


s32 dev_lcd_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 *color)
{	
	if(lcd == NULL)
		return -1;
	
	return lcd->drv->fill(lcd, sx-1,ex-1,sy-1,ey-1,color);
}
s32 dev_lcd_color_fill(DevLcdNode *lcd, u16 sx,u16 ex,u16 sy,u16 ey,u16 color)
{
	if(lcd == NULL)
		return -1;
	
	return lcd->drv->color_fill(lcd, sx-1,ex-1,sy-1,ey-1,color);
}
s32 dev_lcd_backlight(DevLcdNode *lcd, u8 sta)
{
	if(lcd == NULL)
		return -1;
	
	lcd->drv->backlight(lcd, sta);
	return 0;
}
s32 dev_lcd_display_onoff(DevLcdNode *lcd, u8 sta)
{
	if(lcd == NULL)
		return -1;

	return lcd->drv->onoff(lcd, sta);
}

s32 dev_lcd_flush(DevLcdNode *lcd, u16 *color, u32 len)
{
	if(lcd == NULL)
		return -1;

	return lcd->drv->flush(lcd, color, len);	
}
s32 dev_lcd_update(DevLcdNode *lcd)
{
	if(lcd == NULL)
		return -1;

	return lcd->drv->update(lcd);
}
/**
 *@brief:      dev_lcd_setdir
 *@details:    设置横屏或竖屏，扫描方向
 *@param[in]   u8 dir       0 竖屏1横屏
               u8 scan_dir  参考宏定义L2R_U2D       
 *@param[out]  无
 *@retval:     
 */
s32 dev_lcd_setdir(DevLcdNode *node, u8 dir, u8 scan_dir)
{
	u16 temp;
	u8 scan_dir_tmp;

	if(node == NULL)
		return -1;

	
	if(dir != node->dir)//切换屏幕方向	
	{
		
		node->dir = node->dir^0x01;
		temp = node->width;
		node->width = node->height;
		node->height = temp;
		LCD_DEBUG(LOG_DEBUG, "set dir w:%d, h:%d\r\n", node->width, node->height);
	}
	
	
	if(node->dir == W_LCD)//横屏，扫描方向映射转换
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
	
	node->scandir = scan_dir_tmp;
	
	node->drv->set_dir(node, node->scandir);
	
	return 0;
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
void line (DevLcdNode *lcd, int x1, int y1, int x2, int y2, unsigned colidx)
{
	int tmp;
	int dx = x2 - x1;
	int dy = y2 - y1;

	if(lcd == NULL)
		return;

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
			dev_lcd_drawpoint(lcd, x1 >> 16, y1, colidx);
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
			dev_lcd_drawpoint(lcd, x1, y1 >> 16, colidx);
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
void put_cross(DevLcdNode *lcd, int x, int y, unsigned colidx)
{
	if(lcd == NULL)
		return;
	
	line (lcd, x - 10, y, x - 2, y, colidx);
	line (lcd, x + 2, y, x + 10, y, colidx);
	line (lcd, x, y - 10, x, y - 2, colidx);
	line (lcd, x, y + 2, x, y + 10, colidx);

	line (lcd, x - 6, y - 9, x - 9, y - 9, colidx + 1);
	line (lcd, x - 9, y - 8, x - 9, y - 6, colidx + 1);
	line (lcd, x - 9, y + 6, x - 9, y + 9, colidx + 1);
	line (lcd, x - 8, y + 9, x - 6, y + 9, colidx + 1);
	line (lcd, x + 6, y + 9, x + 9, y + 9, colidx + 1);
	line (lcd, x + 9, y + 8, x + 9, y + 6, colidx + 1);
	line (lcd, x + 9, y - 6, x + 9, y - 9, colidx + 1);
	line (lcd, x + 8, y - 9, x + 6, y - 9, colidx + 1);

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
void put_char(DevLcdNode *lcd, int x, int y, int c, int colidx)
{
	int i,j,bits;
	u8* p;
	
	if(lcd == NULL)
		return;	
	
	p = (u8*)font_vga_8x8.data;
	for (i = 0; i < font_vga_8x8.height; i++) 
	{
		bits =  p[font_vga_8x8.height * c + i];
		for (j = 0; j < font_vga_8x8.width; j++, bits <<= 1)
		{
			if (bits & 0x80)
			{
				lcd->drv->draw_point(lcd, x + j, y + i, colidx);
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
void put_string(DevLcdNode *lcd, int x, int y, char *s, unsigned colidx)
{
	int i;
	
	if(lcd == NULL)
		return;	
	
	for (i = 0; *s; i++, x += font_vga_8x8.width, s++)
		put_char(lcd, x, y, *s, colidx);
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
void put_string_center(DevLcdNode *lcd, int x, int y, char *s, unsigned colidx)
{
	int sl = strlen (s);
	
	if(lcd == NULL)
		return;	
	
    put_string (lcd, x - (sl / 2) * font_vga_8x8.width,
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
void rect (DevLcdNode *lcd, int x1, int y1, int x2, int y2, unsigned colidx)
{
	if(lcd == NULL)
		return;

	line (lcd, x1, y1, x2, y1, colidx);
	line (lcd, x2, y1, x2, y2, colidx);
	line (lcd, x2, y2, x1, y2, colidx);
	line (lcd, x1, y2, x1, y1, colidx);
}

/**
 *@brief:      dev_lcd_put_string
 *@details:    显示字符串，支持中文
 *@param[in]   无
 *@param[out]  无
 *@retval:     	
 */
s32 dev_lcd_put_string(DevLcdNode *lcd, FontType font, int x, int y, char *s, unsigned colidx)
{
	u16 slen;
	u16 xlen,ylen;
	u16 *framebuff;//样点缓冲，按照L2R_U2D格式填充
	u8 *dotbuf;//字符点阵缓冲
	s32 res;
	u16 sidx;
	u16 i,j;
	u32 xbase;//显示在x轴偏移量

	if(lcd == NULL)
		return -1;
	
	/* 通过刷一整块，提高显示速度 */
	slen = strlen(s);
	//uart_printf("str len:%d\r\n", slen);

	/*
		根据字符串长度计算刷新区域长宽
	*/
	xlen = slen*FontAscList[font]->width;
	ylen = FontAscList[font]->height;

	framebuff = (u16*)wjq_malloc(xlen*ylen*sizeof(u16));//样点缓冲
	dotbuf = (u8*)wjq_malloc(32);//要改为根据字库类型申请
	sidx = 0;

	/*获取点阵，并转化为LCD像素*/
	while(1)
	{
		if(*(s+sidx) < 0x81)//英文字母
		{
			//uart_printf("eng\r\n");
			u8 ch;
			/*获取点阵*/
			ch = *(s+sidx);
			
			res = font_get_asc(font, &ch, dotbuf);
			//PrintFormat(dotbuf, 16);
			/*asc是横库*/

			for(j=0;j<FontAscList[font]->height;j++)
			{

				xbase = xlen*j + sidx*FontAscList[font]->width;//当前字符X轴偏移量
				for(i=0;i<FontAscList[font]->width;i++)
				{
					/*暂时只处理6*12，8*16的ASC，每一列1个字节*/
					if((dotbuf[j*1+i/8]&(0x80>>(i%8)))!= 0)
					{
						//uart_printf("* ");
						framebuff[xbase + i] = colidx;
					}
					else
					{
						//uart_printf("- ");
						framebuff[xbase + i] = BackColor;
					}
				}
				//uart_printf("\r\n");
			}	
			
			sidx++;
		}
		else//汉字
		{
			//uart_printf("ch\r\n");
			res = font_get_hz(font, s+sidx, dotbuf);//从SD卡读取一个1616汉字的点阵要1ms
			//PrintFormat(dotbuf, 32);

			/*仅仅支持纵库，取模方式2,16*16*/
			for(j=0; j<FontList[font]->height; j++)
			{
				xbase = xlen*j + sidx*FontAscList[font]->width;//当前字符X轴偏移量
				for(i=0;i<FontList[font]->width;i++)
				{
					/*暂时只做1212，1616，每一列2个字节数据*/
					if((dotbuf[i*2+j/8]&(0x80>>(j%8)))!= 0)
					{
						//uart_printf("* ");
						framebuff[xbase + i] = colidx;
					}
					else
					{
						//uart_printf("- ");
						framebuff[xbase + i] = BackColor;
					}
				}
				//uart_printf("\r\n");
			}	
			
			sidx+= 2;
		}

		if(sidx >= slen)
		{
			//uart_printf("finish");
			break;
		}
	}


	if( y + ylen > lcd->height)
	{
		/*显示超出屏幕*/
		ylen = lcd->height - y+1;//假设height = 240,y = 240, 也就意味着只显示一行
	}
	
	if(x + xlen >= lcd->width)
	{
		/*显示超出屏幕宽度*/
		i = lcd->width - x + 1;
		
		/*调整数据*/
		j = 1;
		while(1)
		{
			if(j >= ylen)
				break;
			memcpy(framebuff+j*i, framebuff+ j*xlen, 2*i);
			j++;
		}
		xlen = i;
	}

	dev_lcd_fill(lcd, x, x + xlen-1, y, y + ylen-1, framebuff);

	wjq_free(framebuff);
	wjq_free(dotbuf);

	return 0;	
}
extern void Delay(__IO uint32_t nTime);

#if 1
typedef struct tagBITMAPFILEHEADER  //文件头  14B  
{ 
    u16  bfType;   //0x424d, "BM"
    u32  bfSize;   //文件大小，包含文件头
    u16  bfReserved1;   //保留字节
    u16  bfReserved2;   //保留字节
    u32  bfOffBits;   	//从文件头到实际位图数据的偏移
} BITMAPFILEHEADER; 

typedef struct tagBITMAPINFOHEADER  //位图信息头
{ 
    u32 biSize;   //本结构长度，也即是40
    s32 biWidth;  //图像宽度   
    s32 biHeight; //图像高度    
    u16 biPlanes; //1  
    u16 biBitCount;//1黑白二色图，4 16位色，8 256色，24 真彩色 
    u32 biCompression;   //是否压缩
    u32 biSizeImage;   //实际位图数据字节数
    s32 biXPelsPerMeter;  //目标设备水平分辨率 
    s32 biYPelsPerMeter;   //目标设备垂直分辨率
    u32 biClrUsed;  //图像实际用到颜色数，如为0，则用到的颜色数为2的biBitCount次方
    u32 biClrImportant;  //指定本图象中重要的颜色数，如果该值为零，则认为所有的颜色都是重要的
} BITMAPINFOHEADER;

/*调色板每个元素*/
typedef struct tagRGBQUAD
{ 
	u8    rgbBlue; //蓝色分量  
	u8    rgbGreen; //绿色分量    
	u8    rgbRed;   //红色分量  
	u8    rgbReserved;    
} RGBQUAD; 

#include "ff.h"

#define WIDTHBYTES(i) ((i+31)/32*4)

/*

	4种图片，刷屏时间，图片保存在SD卡

			FSMC	读数据		SPI（其中，将U16拆为U8，花10ms）
	1bit	53ms 	30  	316
	4bit	74	 	50		340
	8bit	79	 	51		344
	24bit	111	 	91		378


	要提速，还可以有下面方法：
	申请双缓冲，用DMA，启动DMA后，不等传输完成，
	就出来准备下一包数据，发送下一包前，查询上一包是否发送完成。
	这样，原来的时间：
		数据准备时间+数据传输时间
	现在时间变为：
		数据准备时间/数据传输时间，两者较长的为需要时间。
*/
s32 dev_lcd_show_bmp(DevLcdNode *lcd, u16 x, u16 y, u16 xlen, u16 ylen, s8 *BmpFileName)
{
	BITMAPFILEHEADER    bf;
    BITMAPINFOHEADER    bi;
	
	FRESULT res;
	FIL bmpfile;
	
	u32 rlen;
    u16 LineBytes;
	u16 NumColors;
    u32 ImgSize;
	u16 buf[40];

	u32 i,j;
	u8 *palatte;
	volatile u16 color;
	u32 k, m;
	u16 r,g,b;
	u8 c;
	u16 *pcc;
	u8 *pdata;
	u8 linecnt = 20;//一次读多行，加快速度
	u8 l;
	
	wjq_log(LOG_DEBUG, "bmp open file:%s\r\n", BmpFileName);
	
	res = f_open(&bmpfile, BmpFileName, FA_READ);
	if(res != FR_OK)
	{
		wjq_log(LOG_DEBUG, "bmp open file err:%d\r\n", res);
		return -1;
	}

    res = f_read(&bmpfile, (void *)buf, 14, &rlen);

	bf.bfType      = buf[0];
    bf.bfSize      = buf[2];
    bf.bfSize = (bf.bfSize<<16)+buf[1];
    bf.bfReserved1 = buf[3];
    bf.bfReserved2 = buf[4];
    bf.bfOffBits   = buf[6];
    bf.bfOffBits = (bf.bfOffBits<<16)+buf[5];
	
	wjq_log(LOG_DEBUG, "bf.bfType:%x\r\n", bf.bfType);	
	wjq_log(LOG_DEBUG, "bf.bfSize:%d\r\n", bf.bfSize);
	wjq_log(LOG_DEBUG, "bf.bfOffBits:%d\r\n", bf.bfOffBits);

	res = f_read(&bmpfile, (void *)buf, 40, &rlen);

	bi.biSize           = (unsigned long) buf[0];
    bi.biWidth          = (long) buf[2];
    bi.biHeight         = (long) buf[4];
    bi.biPlanes         = buf[6];
    bi.biBitCount       = buf[7];
    bi.biCompression    = (unsigned long) buf[8];
    bi.biSizeImage      = (unsigned long) buf[10];
    bi.biXPelsPerMeter  = (long) buf[12];
    bi.biYPelsPerMeter  = (long) buf[14];
    bi.biClrUsed        = (unsigned long) buf[16];
    bi.biClrImportant   = (unsigned long) buf[18];

	wjq_log(LOG_DEBUG, "bi.biSize:%d\r\n", bi.biSize);	
	wjq_log(LOG_DEBUG, "bi.biWidth:%d\r\n", bi.biWidth);
	wjq_log(LOG_DEBUG, "bi.biHeight:%d\r\n", bi.biHeight);
	wjq_log(LOG_DEBUG, "bi.biPlanes:%d\r\n", bi.biPlanes);
	wjq_log(LOG_DEBUG, "bi.biBitCount:%d\r\n", bi.biBitCount);
	wjq_log(LOG_DEBUG, "bi.biCompression:%d\r\n", bi.biCompression);
	wjq_log(LOG_DEBUG, "bi.biSizeImage:%d\r\n", bi.biSizeImage);
	wjq_log(LOG_DEBUG, "bi.biXPelsPerMeter:%d\r\n", bi.biXPelsPerMeter);
	wjq_log(LOG_DEBUG, "bi.biYPelsPerMeter:%d\r\n", bi.biYPelsPerMeter);
	wjq_log(LOG_DEBUG, "bi.biClrUsed:%d\r\n", bi.biClrUsed);
	wjq_log(LOG_DEBUG, "bi.biClrImportant:%d\r\n", bi.biClrImportant);

	/*8个像素占用一个字节，不足一个字节补足一个字节*/
	/*单色图片四字节对齐*/
	LineBytes = WIDTHBYTES(bi.biWidth * bi.biBitCount);
    ImgSize   = (unsigned long) LineBytes  * bi.biHeight;

    wjq_log(LOG_DEBUG, "bmp w:%d,h:%d, bitcount:%d, linebytes:%d\r\n", bi.biWidth, bi.biHeight, bi.biBitCount, LineBytes);
	
	if(bi.biClrUsed!=0)
		NumColors=(DWORD)bi.biClrUsed;//如果 bi.biClrUsed 不为零，就是本图象实际用到的颜色
	else
	{
	    switch(bi.biBitCount)
	    {
	    case 1:
	        NumColors=2;//黑白屏用到两个调色板，一个是黑一个是白
	        break;
	        
	    case 4:
	        NumColors=16;
	        break;
	        
	    case 8:
	        NumColors=256;
	        break;
	        
	    case 24:
	        NumColors=0;
	        break;
	        
	    default:
	        f_close(&bmpfile);
	        return 2;
	    }
	}

	/* 读调色板 */
	if(NumColors != 0)
	{
		palatte = wjq_malloc(4*NumColors);
		f_read(&bmpfile, (void *)palatte, 4*NumColors, &rlen);
	}

	if(xlen > bi.biWidth)
        xlen = bi.biWidth;

    if(ylen > bi.biHeight)
        ylen = bi.biHeight;

	pdata = wjq_malloc(LineBytes*linecnt);
	
	dev_lcd_prepare_display(lcd, x, x+xlen-1, y, y+ylen-1);
			
    switch(bi.biBitCount)
    {
    case 1:
		GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
		
		pcc = wjq_malloc(xlen*sizeof(u16));
	
		for(j=0; j<ylen;) //图片取模:横向,左高右低
        {
        	if(j+linecnt >= ylen)
				linecnt = ylen-j;
						
			f_read(&bmpfile, (void *)pdata, LineBytes*linecnt, &rlen);
			if(rlen != LineBytes*linecnt)
			{
				wjq_log(LOG_DEBUG, "bmp read data err!\r\n");
			}
			l = 0;
			while(l < linecnt)
			{
				k = l*LineBytes;
				#if 0//不用除法， 测试
				i = 0;				
				while(1)
	            {
					/*
            		一个字节8个像素，高位在前
            		调色板有256种颜色
					*/
		            c = pdata[k];
					
					m = 0;
					while(m <8)
					{
						if((c &(0x80>>m)) != 0)
	                		*(pcc+i)  = WHITE;
						else
							*(pcc+i)  = BLACK;	

						i++;
						if(i>= xlen)
							break;
						m++;
					}
					
					if(i>= xlen)
						break;

					k++;
	            }
				#else
				for(i=0; i<xlen; i++)
		        {
		        	/*
		        		一个字节8个像素，高位在前
		        		调色板有256种颜色
					*/
		            c = pdata[k+(i/8)]&(0x80>>(i%8));
		            
		            if(c != 0)
						*(pcc+i) = WHITE;
		            else
						*(pcc+i) = BLACK;
		        }
				#endif
				
				dev_lcd_flush(lcd, pcc, xlen);
				l++;
			}

			j += linecnt;
        }
		wjq_free(pcc);
		GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
        break;
        
    case 4:
		GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
		pcc = wjq_malloc(xlen*sizeof(u16));
	
		for(j=0; j<ylen;) //图片取模:横向,左高右低
        {
        	if(j+linecnt >= ylen)
				linecnt = ylen-j;
						
			f_read(&bmpfile, (void *)pdata, LineBytes*linecnt, &rlen);
			if(rlen != LineBytes*linecnt)
			{
				wjq_log(LOG_DEBUG, "bmp read data err!\r\n");
			}
			l = 0;
			while(l < linecnt)
			{
				k = l*LineBytes;

				#if 0//不用除法,测试
				i = 0;
				while(1)
	            {
					/*4个bit 1个像素，要进行对U16的转换
					rgb565
					#define BLUE         	 0x001F  
					#define GREEN         	 0x07E0
					#define RED           	 0xF800
					*/
					c = *(pdata+k);
					
					m = ((c>>4)&0x0f)*4;
					
					b = (palatte[m++] & 0xF8)>>3;
					g = (palatte[m++] & 0xFC)>>2;
					r = (palatte[m] & 0xF8)>>3;
	                                
	                *(pcc+i) = (r<<11)|(g<<5)|(b<<0);
					
					i++;
					if(i>= xlen)
						break;
					
					m = (c&0x0f)*4;
					
					b = (palatte[m++] & 0xF8)>>3;
					g = (palatte[m++] & 0xFC)>>2;
					r = (palatte[m] & 0xF8)>>3;

	                *(pcc+i) = (r<<11)|(g<<5)|(b<<0);
					
					i++;
					if(i>= xlen)
						break;
					k++;
	            }
				#else
				for(i=0; i < xlen; i++)
				{
									/*4个bit 1个像素，要进行对U16的转换
					rgb565
	#define BLUE			 0x001F  
	#define GREEN			 0x07E0
	#define RED 			 0xF800
					*/
					m = *(pdata+k+i/2);
					
					if(i%2 == 0)
						m = ((m>>4)&0x0f);
					else
						m = (m&0x0f);
					m = m*4;
					
					r = (palatte[m+2] & 0xF8)>>3;
					g = (palatte[m+1] & 0xFC)>>2;
					b = (palatte[m] & 0xF8)>>3;
					
					*(pcc+i) = (r<<11)|(g<<5)|(b<<0);
				}
				#endif
				
				dev_lcd_flush(lcd, pcc, xlen);
				l++;
			}

			j += linecnt;
        }
		wjq_free(pcc);
		GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
        break;

    case 8:
		GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
		pcc = wjq_malloc(xlen*sizeof(u16));	
		for(j=0; j<ylen;) //图片取模:横向,左高右低
        {
        	if(j+linecnt >= ylen)
				linecnt = ylen-j;
						
			f_read(&bmpfile, (void *)pdata, LineBytes*linecnt, &rlen);
			if(rlen != LineBytes*linecnt)
			{
				wjq_log(LOG_DEBUG, "bmp read data err!\r\n");
			}
			
			l = 0;
			while(l < linecnt)
			{
				k = l*LineBytes;
				
            	for(i=0; i < xlen; i++)
	            {
					/*1个字节1个像素，要进行对U16的转换
					rgb565
					#define BLUE         	 0x001F  
					#define GREEN         	 0x07E0
					#define RED           	 0xF800
					*/
					m = *(pdata+k);
					k++;
					m = m*4;
					
					r = (palatte[m+2] & 0xF8)>>3;
	                g = (palatte[m+1] & 0xFC)>>2;
	                b = (palatte[m] & 0xF8)>>3;
					
	                *(pcc+i)  = (r<<11)|(g<<5)|(b<<0);
	            }
				
				dev_lcd_flush(lcd, pcc, xlen);
				
				l++;
			}

			j += linecnt;
        }
		wjq_free(pcc);
		GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
        break;

	case 16:

		break;
		
    case 24://65K真彩色		
    	GPIO_ResetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
		pcc = (u16 *)pdata;
	
        for(j=0; j<ylen;) //图片取模:横向,左高右低
        {
			if(j+linecnt >= ylen)
				linecnt = ylen-j;
			
			res = f_read(&bmpfile, (void *)pdata, LineBytes*linecnt, &rlen);
			if(res != FR_OK)
			{
				wjq_log(LOG_DEBUG, "bmp read data err!\r\n");	
			}
			
			if(rlen != LineBytes*linecnt)
			{
				wjq_log(LOG_DEBUG, "bmp read data err!\r\n");
			}
			
			l = 0;
			while(l < linecnt)
			{
				k = l*LineBytes;
				
            	for(i=0; i < xlen; i++)
	            {
	            	/*3个字节1个像素，要进行对U16的转换
						rgb565
						#define BLUE         	 0x001F  
						#define GREEN         	 0x07E0
						#define RED           	 0xF800
					*/
					b = pdata[k++];
					g = pdata[k++];
					r = pdata[k++];
					
					r = ((r<<8)&0xf800);
					g = ((g<<3)&0x07e0);
					b = ((b>>3)&0x001f);
					*(pcc+i) = r+g+b;

	            }
				
				dev_lcd_flush(lcd, pcc, xlen);
				l++;
			}

			j += linecnt;
        }

		GPIO_SetBits(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_3);
			
        break;

	case 32:
		break;
		
    default:
        break;
    } 

	dev_lcd_update(lcd);
	
	wjq_free(pdata);
	if(NumColors != 0)
	{
		wjq_free(palatte);
	}
	
	f_close(&bmpfile);
    return 0;
}
#endif
/**
 *@brief:      dev_lcd_test
 *@details:    LCD测试函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void dev_lcd_test(void)
{
	DevLcdNode *LcdCog = NULL;
	DevLcdNode *LcdOled = NULL;
	DevLcdNode *LcdOledI2C = NULL;
	DevLcdNode *LcdTft = NULL;

	/*  打开三个设备 */
	LcdCog = dev_lcd_open("spicoglcd");
	if(LcdCog==NULL)
		wjq_log(LOG_FUN, "open cog lcd err\r\n");

	LcdOled = dev_lcd_open("vspioledlcd");
	if(LcdOled==NULL)
		wjq_log(LOG_FUN, "open oled lcd err\r\n");
	
	LcdTft = dev_lcd_open("tftlcd");
	if(LcdTft==NULL)
		wjq_log(LOG_FUN, "open tft lcd err\r\n");

	LcdOledI2C = dev_lcd_open("i2coledlcd");
	if(LcdOledI2C==NULL)
		wjq_log(LOG_FUN, "open oled i2c lcd err\r\n");
	
	/*打开背光*/
	dev_lcd_backlight(LcdCog, 1);
	dev_lcd_backlight(LcdOled, 1);
	dev_lcd_backlight(LcdOledI2C, 1);
	dev_lcd_backlight(LcdTft, 1);

	#if 0/*不支持汉字时*/
	put_string(LcdCog, 5, 5, "spi cog lcd", BLACK);
	put_string(LcdOled, 5, 5, "vspi oled lcd", BLACK);
	put_string(LcdOledI2C, 5, 5, "i2c oled lcd", BLACK);
	put_string(LcdTft, 5, 5, "2.8 tft lcd", BLACK);
	#endif

	#if 1
	dev_lcd_put_string(LcdOled, FONT_SONGTI_1212, 10,1, "ABC-abc，", BLACK);
	dev_lcd_put_string(LcdOled, FONT_SIYUAN_1616, 1, 13, "这是oled lcd", BLACK);
	dev_lcd_put_string(LcdOled, FONT_SONGTI_1212, 10,30, "www.wujique.com", BLACK);
	dev_lcd_put_string(LcdOled, FONT_SIYUAN_1616, 1, 47, "屋脊雀工作室", BLACK);
	dev_lcd_update(LcdOled);
	dev_lcd_put_string(LcdCog, FONT_SONGTI_1212, 10,1, "ABC-abc，", BLACK);
	dev_lcd_put_string(LcdCog, FONT_SIYUAN_1616, 1, 13, "这是cog lcd", BLACK);
	dev_lcd_put_string(LcdCog, FONT_SONGTI_1212, 10,30, "www.wujique.com", BLACK);
	dev_lcd_put_string(LcdCog, FONT_SIYUAN_1616, 1, 47, "屋脊雀工作室", BLACK);
	dev_lcd_update(LcdCog);
	dev_lcd_put_string(LcdTft, FONT_SONGTI_1212, 20,30, "ABC-abc，", RED);
	dev_lcd_put_string(LcdTft, FONT_SIYUAN_1616, 20,60, "这是tft lcd", RED);
	dev_lcd_put_string(LcdTft, FONT_SONGTI_1212, 20,100, "www.wujique.com", RED);
	dev_lcd_put_string(LcdTft, FONT_SIYUAN_1616, 20,150, "屋脊雀工作室", RED);
	dev_lcd_update(LcdTft);
	dev_lcd_put_string(LcdOledI2C, FONT_SONGTI_1212, 10,1, "ABC-abc，", BLACK);
	dev_lcd_put_string(LcdOledI2C, FONT_SIYUAN_1616, 1,13, "这是LcdOledI2C", BLACK);
	dev_lcd_put_string(LcdOledI2C, FONT_SONGTI_1212, 10,30, "www.wujique.com", BLACK);
	dev_lcd_put_string(LcdOledI2C, FONT_SIYUAN_1616, 1,47, "屋脊雀工作室", BLACK);
	dev_lcd_update(LcdOledI2C);
	#endif
	
	while(1);
}



void dev_i2coledlcd_test(void)
{

	DevLcdNode *LcdOledI2C = NULL;

	LcdOledI2C = dev_lcd_open("i2coledlcd");
	if(LcdOledI2C==NULL)
		wjq_log(LOG_FUN, "open oled i2c lcd err\r\n");
	
	/*打开背光*/
	dev_lcd_backlight(LcdOledI2C, 1);

	dev_lcd_put_string(LcdOledI2C, FONT_SONGTI_1212, 10,1, "ABC-abc，", BLACK);
	dev_lcd_put_string(LcdOledI2C, FONT_SIYUAN_1616, 1,13, "这是LcdOledI2C", BLACK);
	dev_lcd_put_string(LcdOledI2C, FONT_SONGTI_1212, 10,30, "www.wujique.com", BLACK);
	dev_lcd_put_string(LcdOledI2C, FONT_SIYUAN_1616, 1,47, "屋脊雀工作室", BLACK);
	dev_lcd_update(LcdOledI2C);

	LcdOledI2C = dev_lcd_open("i2coledlcd2");
	if(LcdOledI2C==NULL)
		wjq_log(LOG_FUN, "open oled i2c2 lcd err\r\n");
	
	/*打开背光*/
	dev_lcd_backlight(LcdOledI2C, 1);
	while(1)
	{
		dev_lcd_put_string(LcdOledI2C, FONT_SONGTI_1212, 10,1, "ABC-abc，", BLACK);
		dev_lcd_put_string(LcdOledI2C, FONT_SIYUAN_1616, 1,13, "这是LcdOledI2C", BLACK);
		dev_lcd_put_string(LcdOledI2C, FONT_SONGTI_1212, 10,30, "www.wujique.com", BLACK);
		dev_lcd_put_string(LcdOledI2C, FONT_SIYUAN_1616, 1,47, "屋脊雀工作室", BLACK);
		dev_lcd_update(LcdOledI2C);
		Delay(1000);
		dev_lcd_color_fill(LcdOledI2C, 1, 1000, 1, 1000, WHITE);
		dev_lcd_update(LcdOledI2C);
		Delay(1000);
	}

}

