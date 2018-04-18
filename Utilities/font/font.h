/*
 *  font.h -- `Soft' font definitions
 *
 *  Created 1995 by Geert Uytterhoeven
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License.  See the file COPYING in the main directory of this archive
 *  for more details.
 */

#ifndef _VIDEO_FONT_H
#define _VIDEO_FONT_H

#include "stm32f4xx.h"

struct fbcon_font_desc 
{
    int idx;
    char *name;
    u16 width, height;
    void *data;
    int pref;
	u16 size;//每个字符字节数
};

#define VGA8x8_IDX		0
#define VGA8x16_IDX		1
#define PEARL8x8_IDX	2
#define VGA6x11_IDX		3
#define SUN8x16_IDX		4
#define SUN12x22_IDX	5
#define ACORN8x8_IDX	6
#define HZ_1616_IDX		7
#define HZ_1212_IDX		8

/*
	字体类型定义
*/
typedef enum{
	FONT_SONGTI_1616 = 0,//1616字体，对应的ASC则是8*16
	FONT_SONGTI_1212,	//1212字体，对应的ASC是6*12	
	FONT_SIYUAN_1616,
	FONT_SIYUAN_1212,
	FONT_LIST_MAX
}FontType;

/* Max. length for the name of a predefined font */
#define MAX_FONT_NAME	32

extern struct fbcon_font_desc font_vga_8x8;
extern struct fbcon_font_desc font_vga_8x16;
extern struct fbcon_font_desc font_vga_6x12;

extern const struct fbcon_font_desc *FontAscList[FONT_LIST_MAX];
extern const struct fbcon_font_desc *FontList[FONT_LIST_MAX];

extern s32 font_get_asc(FontType type, u8 *ch, u8 *buf);
extern s32 font_get_hz(FontType type, u8 *ch, u8 *buf);



#endif /* _VIDEO_FONT_H */

