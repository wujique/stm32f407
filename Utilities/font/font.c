/**
 * @file            font.c
 * @brief           字库管理
 * @author          wujique
 * @date            2018年3月2日 星期五
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年3月2日 星期五
 *   作    者:       屋脊雀工作室
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
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "font.h"
#include "ff.h"

/*
	最好的方案其实是让字库自举，也就是在字库的头部包含字库信息。
	暂时不做这么复杂了。
*/

/*

	汉字字库只使用多国文字点阵字库生成器生成的点阵字库，
	使用模式二生成，也就是纵库。

*/
/*宋体*/
FIL FontFileST1616;
FIL FontFileST1212;

/*思源*/
FIL FontFileSY1616;
FIL FontFileSY1212;

struct fbcon_font_desc font_songti_16x16 = {
	HZ_1616_IDX,
	"1:/songti1616.DZK",//保存在SD卡
	16,
	16,
	(char *)&FontFileST1616,
	0,
	32
};
	
struct fbcon_font_desc font_songti_12x12 = {
	HZ_1212_IDX,
	"1:/songti1212.DZK",
	12,
	12,
	(char *)&FontFileST1212,
	0,
	24
};

struct fbcon_font_desc font_siyuan_16x16 = {
	HZ_1616_IDX,
	"1:/shscn1616.DZK",//保存在SD卡
	16,
	16,
	(char *)&FontFileSY1616,
	0,
	32
};
	
struct fbcon_font_desc font_siyuan_12x12 = {
	HZ_1212_IDX,
	"1:/shscn1212.DZK",
	12,
	12,
	(char *)&FontFileSY1212,
	0,
	24
};

const struct fbcon_font_desc *FontList[FONT_LIST_MAX]=
	{
		&font_songti_16x16,
		&font_songti_12x12,
		&font_siyuan_16x16,
		&font_siyuan_12x12
	};

/*

	asc字符点阵，使用横库。
	ASC字库内置在源代码中。
*/
const struct fbcon_font_desc *FontAscList[FONT_LIST_MAX]=
		{
			&font_vga_8x16,
			&font_vga_6x12,
			&font_vga_8x16,
			&font_vga_6x12
		};

s32 FontInit = -1;		
/**
 *@brief:      font_check_hzfont
 *@details:    检查字库，主要是汉字库
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 font_check_hzfont(void)
{
	u8 i;
	FRESULT res;
	FIL* fp;
	
	i = 0;
	while(1)
	{
		if(i>= FONT_LIST_MAX)
			break;
		
		fp = (FIL*)(FontList[i]->data);		
		res = f_open(fp, FontList[i]->name, FA_READ);

		
		if(res != FR_OK)
		{
			wjq_log(LOG_INFO, "font open file:%s, err:%d!\r\n", FontList[i]->name, res);
		}
		else
		{
			wjq_log(LOG_INFO, "font open file :%s ok!\r\n", FontList[i]->name);	
		}
		
		i++;
	}

	FontInit = 0;
	return 0;
}

/**
 *@brief:      font_get_hz
 *@details:    获取汉字点阵
 *@param[in]   FontType type  
               char *ch       
               char *buf      
 *@param[out]  无
 *@retval:     
 */
s32 font_get_hz(FontType type, u8 *ch, u8 *buf)
{
	FRESULT res;
	unsigned int len;
	u32 addr;
	u8 hcode,lcode;

	FIL* fontp;

	if(type >= FONT_LIST_MAX)
		return -1;

	if(FontInit == -1)
		font_check_hzfont();
	
	hcode = *ch;
	lcode = *(ch+1);
	
	if((hcode < 0x81) 
		|| (hcode > 0xfe)
		)
	{
		//uart_printf("no china hz\r\n");
		return -1;
	}

	//uart_printf("hz code:%02x, %02x\r\n", hcode, lcode);

	addr = (hcode-0x81)*190;
	if(lcode<0x7f)
	{
		addr = addr+lcode-0x40;	
	}
	else
	{
		addr = addr+lcode-0x41;	
	}
	addr = addr*FontList[type]->size;
	//uart_printf("adr:%08x\r\n", addr);
	
	fontp = (FIL*)FontList[type]->data;
	if(fontp->fs == 0)
	{
		//uart_printf("no font\r\n");
		return -1;
	}
	
	res = f_lseek(fontp, addr);
	if(res != FR_OK)
	{
		return 0;
	}

	res = f_read(fontp, (void *)buf, FontList[type]->size, &len);
	if((res != FR_OK) || (len!= FontList[type]->size))
	{
		//uart_printf("font read err\r\n");
		return -1;
	}
	
	return 0;
	
}
/**
 *@brief:      font_get_asc
 *@details:    获取ASC字符点阵数据
 *@param[in]   FontType type  
               char *ch       
               char *buf      
 *@param[out]  无
 *@retval:     
 */
s32 font_get_asc(FontType type, u8 *ch, u8 *buf)
{
	u8* fp;
	
	if(*ch >= 0x80)
		return -1;

	if(type > FONT_LIST_MAX)
		return -1;

	fp = (u8*)FontAscList[type]->data + (*ch)*FontAscList[type]->size;
	//wjq_log(LOG_DEBUG, "dot data\r\n");
	//PrintFormat(fp, 16);
	
	memcpy(buf, fp, FontAscList[type]->size);

	return 0;
}



