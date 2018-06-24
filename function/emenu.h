
#ifndef __EASY_MENU_H__
#define __EASY_MENU_H__


typedef enum{
	MENU_TYPE_NULL = 0,
/*
功能菜单，功能菜单没有子菜单，在执行的函数就是实际功能
*/
	MENU_TYPE_FUN,
/*
//<列表菜单> 菜单名显示在顶部，下级菜单通过列表形式显示在下方
通过上下按键选择子菜单, 通过确认键选择进入下一级菜单
*/
	MENU_TYPE_LIST,
/*
//<天顶菜单> 本级菜单与统计菜单显示在顶部，通过左右选择本级菜单，
下级菜单通过列表形式显示在下方，通过上下选择子菜单
只有第一级菜单才能做为天顶菜单，天顶菜单的下一级，一般是列表菜单
*/
	MENU_TYPE_B,
/*
//<按键菜单> 本级菜单名显示在顶部，子菜单行列式显示在下方，通过按键选择进入菜单
  上下用于翻页子菜单 
  MENU_TYPE_KEY_1COL 单列 MENU_TYPE_KEY_2COL双列
  双列菜单通过数字按键选择，通常数字键盘只有10个按键，一般只使用1-8
*/
	MENU_TYPE_KEY_1COL,
	MENU_TYPE_KEY_2COL
}MenuType;

typedef enum{
	MENU_L_0 = 0,
	MENU_L_1,
	MENU_L_2,
	MENU_L_3,
	MENU_L_MAX
}MenuLel;

typedef enum{
	MENU_LANG_CHA = 0,
	MENU_LANG_ENG,
}MenuLang;

#define MENU_LANG_BUF_SIZE 16
/**
 * @brief  菜单对象
*/
typedef struct _strMenu
{
    MenuLel l;     ///<菜单等级
    char cha[MENU_LANG_BUF_SIZE];   ///中文
    char eng[MENU_LANG_BUF_SIZE];   ///英文
    MenuType type;  ///
    s32 (*fun)(void);  ///测试函数

} MENU;

#include "dev_lcd.h"
extern s32 emenu_run(DevLcdNode *lcd, MENU *p, u16 len, FontType font, u8 spaced);


#endif

