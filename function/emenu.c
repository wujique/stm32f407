/**
 * @file            emenu.c
 * @brief           简易菜单模块
 * @author          wujique
 * @date            2018年3月10日 星期六
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年3月10日 星期六
 *   作    者:         wujique屋脊雀工作室
 *   修改内容:   创建文件
*/
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "emenu.h"
#include "dev_lcd.h"
#include "font.h"
#include "dev_keypad.h"

extern u16 PenColor;
extern u16 BackColor;


/*按键定义*/
#define EMENU_KEYU_0	'0'
#define EMENU_KEYU_1	'1'
#define EMENU_KEYU_2	'2'
#define EMENU_KEYU_3	'3'
#define EMENU_KEYU_4	'4'
#define EMENU_KEYU_5	'5'
#define EMENU_KEYU_6	'6'
#define EMENU_KEYU_7	'7'
#define EMENU_KEYU_8	'8'
#define EMENU_KEYU_9	'9'
#define EMENU_KEYU_ESC	0x1b
#define EMENU_KEYU_F	0x70
#define EMENU_KEYU_DEL	0x2e
#define EMENU_KEYU_ENTER	0x0d
#define EMENU_KEYU_STAR 0x2a
#define EMENU_KEYU_HASH 0x23
#define EMENU_KEYU_UP	EMENU_KEYU_STAR
#define EMENU_KEYU_DOWM EMENU_KEYU_HASH

/*按键转换表*/
const u8 emunu_key_chg[17]=
{
	0, EMENU_KEYU_1,EMENU_KEYU_2,EMENU_KEYU_3,EMENU_KEYU_F,
	EMENU_KEYU_4,EMENU_KEYU_5,EMENU_KEYU_6,EMENU_KEYU_DEL,
	EMENU_KEYU_7,EMENU_KEYU_8,EMENU_KEYU_9,EMENU_KEYU_ESC,
	EMENU_KEYU_STAR,EMENU_KEYU_0,EMENU_KEYU_HASH,EMENU_KEYU_ENTER
};
	
u8 emenu_get_key(void)
{
	u8 key;
	s32 res;
	
	res = dev_keypad_read(&key, 1);
	if(res == 1)
	{
		//wjq_log(LOG_DEBUG,"key:%02x\r\n", key);
		
		if((key & KEYPAD_PR_MASK)!= KEYPAD_PR_MASK  )
		{
			return emunu_key_chg[key];
		}
	}
	return 0;

}

/**
 *@brief:      emenu_check_list
 *@details:    检测菜单列表
 *@param[in]   MENU *p  
               u16 len  
 *@param[out]  无
 *@retval:     
 */
s32 emenu_check_list(MENU *list, u16 len)
{
	u16 i = 0;
	s32 err = 0;;
	MenuLel now_lel = 0;//当前菜单等级
	MENU *p = list;
	
	while(1)
	{
		if(i>= len)
			return err;

		/*菜单等级不能跳级*/
		if((p->l != now_lel)
			&&(p->l != (now_lel-1))
			&&(p->l != (now_lel+1))
			)
		{
			wjq_log(LOG_DEBUG,"level err!\r\n");
			return err;
		}
		/*功能菜单函数不能为空*/
		if(p->type == MENU_TYPE_FUN)
		{
		
		}

		p++;
		i++;
	}
	 
}
/**
 * @brief  菜单程序控制
*/

typedef struct _strMenuCtrl
{
	MENU *fa;	//父菜单，也就是当前显示的菜单
	MENU *dis;	//显示起始
	MENU *sel;	//当前选中的菜单
	u8 d;		//记录dis跟SEL的同级偏移，不用每次去找
	
	u8 lang;	//语言指针偏移量
	
	/*
		LCD跟字体规格
	*/
	u16 lcdh;
	u16 lcdw;
	
	FontType font;
	u8 fonth;
	u8 fontw;

	/*
		显示控制
	*/
	u8 spaced;//行间隔
	u8 line;//一屏能显示的行数
} MenuCtrl;

MenuCtrl menu_ctrl;

/*
	记录dis跟sel、d，以便返回
*/
MENU *emenu_stack[2*MENU_L_MAX];
u8 d_rec[MENU_L_MAX];

MENU *emenu_find_fa(MENU *fp)
{
	MENU *p;
	p = fp;

	/*根节点没父亲*/
	if(p->l == MENU_L_0)
			return NULL;
	
	while(1)
	{
		p--;
		if(p->l == ((menu_ctrl.fa->l)-1))
		{
			/*找到父节点*/
			return p;	
		}
	}
}

MENU *emunu_find_next(MENU *fp, u8 mode)
{
	MENU *p;
	p = fp;

	while(1)
	{
		if(mode == 1)
			p++;
		else
			p--;
		
		/*菜单等级比父节点低一级（大1）*/
		if(p->l == ((menu_ctrl.fa->l)+1))
		{
			return p;	
		}
		else if(p->l == (menu_ctrl.fa->l))
		{
			/*跟父菜单等级一致，说明没子菜单了*/	
			return 0;
		}
		else if(p->l == MENU_L_0)
		{
			wjq_log(LOG_DEBUG,"menu end\r\n");
			return 0;
		}
		else
		{
			
		}

	}
}
/**
 *@brief:    
 *@details:   
 *@param[in]    
 *@param[out] 
 *@retval:     
 */
s32 emenu_display(DevLcdNode *lcd)
{
	MENU *p;
	u16 disy = 1;
	u16 disx;
	u8 discol = 0;
	char disbuf[32];
	u8 menu_num = 0;
	u16 lang;

	/*计算不同语言显示内容的偏移*/
	lang = menu_ctrl.lang*MENU_LANG_BUF_SIZE;
	
	wjq_log(LOG_DEBUG,"emenu display:%s\r\n", menu_ctrl.fa->cha + lang);
	
	dev_lcd_color_fill(lcd, 1, menu_ctrl.lcdw, 1, menu_ctrl.lcdh, BackColor);
	
	/*顶行居中显示父菜单*/
	disx = (menu_ctrl.lcdw - strlen(menu_ctrl.fa->cha)*menu_ctrl.fontw)/2;//居中显示
	dev_lcd_put_string(lcd, menu_ctrl.font, disx, disy, menu_ctrl.fa->cha + lang, PenColor);
	
	
	/* 显示子菜单*/
	switch(menu_ctrl.fa->type)
	{
		case MENU_TYPE_LIST:
			disy += menu_ctrl.fonth + menu_ctrl.spaced;//这些参数要根据选择的字体选择

			/* 
				从父节点后一个节点开始分析，以便统计子节点数量
				在菜单前面显示序号
			*/
			
			p =  menu_ctrl.fa +1;
			while(1)
			{
				/*菜单等级比父节点低一级（大1）*/
				if(p->l == ((menu_ctrl.fa->l)+1))
				{
					menu_num++;
					/*开始显示，之前的菜单不显示*/
					if(p>=menu_ctrl.dis)
					{
						if(disy + menu_ctrl.fonth-1 > menu_ctrl.lcdh)//显示12的字符，从1，开始，只要到dot11就可以显示完了。
						{
							wjq_log(LOG_DEBUG,"over the lcd\r\n");
							break;
						}
						
						memset(disbuf,0,sizeof(disbuf));
						/*菜单被选中，加上效果*/
						if(p == menu_ctrl.sel)
						{
							//uart_printf("sel menu\r\n");	
							sprintf(disbuf, ">%d.", menu_num);
						}
						else
						{
							sprintf(disbuf, "%d.", menu_num);	
						}
						strcat(disbuf, p->cha + lang);
						
						dev_lcd_put_string(lcd, menu_ctrl.font, 1, disy, disbuf, PenColor);

						disy += (menu_ctrl.fonth + menu_ctrl.spaced);//每行间隔
					}
				}
				else if(p->l == (menu_ctrl.fa->l))
				{
					/*跟父菜单等级一致，说明没子菜单了*/	
					break;
				}
				else if(p->l == MENU_L_0)
				{
					/* 末尾节点*/
					wjq_log(LOG_DEBUG, "menu end\r\n");
					break;
				}
				else
				{
					/*比当前父节点低两级，说明是孙菜单*/
				}
				p++;
			}
			break;
			
		case MENU_TYPE_KEY_2COL:
			u8 col2_dis_num;
			/*
				数字键只有0-9，因此双列菜单最多显示8个菜单一页，用1-8按键选择
			*/
			col2_dis_num = (menu_ctrl.line-1)*2;
			if(col2_dis_num > 8)
				col2_dis_num = 8;
			
			p = menu_ctrl.dis;//直接从显示节点开始分析，不用统计子节点数量
			
			disy += menu_ctrl.fonth + menu_ctrl.spaced;
		
			menu_ctrl.d = 0;
			while(1)
			{

				if(p->l == ((menu_ctrl.fa->l)+1))
				{
					menu_num++;
					
					if(menu_num > col2_dis_num)
						break;
					/*处理显示位置*/
					if(menu_num  == (col2_dis_num/2+1))
					{
						disy = menu_ctrl.fonth + menu_ctrl.spaced + 1;
						discol++;
						if(discol>=2)
						{
							wjq_log(LOG_DEBUG,"over the lcd\r\n");
							break;
						}
					}

					menu_ctrl.sel = p;//sel指向显示的最后一个菜单
					menu_ctrl.d++;
					
					memset(disbuf,0,sizeof(disbuf));
					sprintf(disbuf, "%d.", menu_num);	
					strcat(disbuf, p->cha + lang);
					
					dev_lcd_put_string(lcd, menu_ctrl.font, menu_ctrl.lcdw/2*discol+1, disy, disbuf, PenColor);

					disy += (menu_ctrl.fonth + menu_ctrl.spaced);
				}
				else if(p->l == (menu_ctrl.fa->l))
				{
					/*跟父菜单等级一致，说明没子菜单了*/	
					break;
				}
				else if(p->l == MENU_L_0)
				{
					/* 末尾节点*/
					wjq_log(LOG_DEBUG,"menu end\r\n");
					break;
				}
				else
				{
					/*比当前父节点低两级，说明是孙菜单*/
				}
				p++;
			}
			break;
			
		case MENU_TYPE_B:
			break;
		case MENU_TYPE_KEY_1COL:
			break;
		
		default:
			break;
	}

	dev_lcd_update(lcd);
	return 0;
}
/**
 *@brief:      emenu_deal_key_list
 *@details:    列表菜单按键处理
 *@param[in]   u8 key  
 *@param[out]  无
 *@retval:     
 */
s32 emenu_deal_key_list(u8 key)
{
	MENU *menup;
	
	switch(key)
	{
	
		case EMENU_KEYU_DOWM://down
			menup = emunu_find_next(menu_ctrl.sel, 1);
			if(menup!=0)
			{
				menu_ctrl.sel = menup;	
				wjq_log(LOG_DEBUG,"%");
				if(menu_ctrl.d>= menu_ctrl.line-2)//如果一屏能显示5行，顶行是显示内容，4行菜单，间距为3，因此-2
				{
					menup = emunu_find_next(menu_ctrl.dis, 1);
					if(menup!=0)
					{
						menu_ctrl.dis = menup;	
					}
				}
				else
				{
					menu_ctrl.d++;
				}
			}
			else
			{
				/*已经是最后一个菜单，从头开始显示第一个菜单	*/
				menu_ctrl.sel = menu_ctrl.fa+1;
				menu_ctrl.dis = menu_ctrl.sel;
				menu_ctrl.d = 0;
			}
			break;
			
		case EMENU_KEYU_UP://up
			menup = emunu_find_next(menu_ctrl.sel, 0);
			
			if(menup!=0)
			{
				menu_ctrl.sel = menup;	
				wjq_log(LOG_DEBUG,"%");
				if(menu_ctrl.d == 0)
				{
						menu_ctrl.dis = menu_ctrl.sel;	
				}
				else
				{
					menu_ctrl.d--;
				}
			}
			else
			{
				/*第一个节点，那么翻到最后一个节点*/
				while(1)
				{
					menup = emunu_find_next(menu_ctrl.sel, 1);
					if(menup!=0)
					{
						menu_ctrl.sel = menup;	

						if(menu_ctrl.d>= menu_ctrl.line-2)
						{
							menup = emunu_find_next(menu_ctrl.dis, 1);
							if(menup!=0)
							{
								menu_ctrl.dis = menup;	
							}
						}
						else
						{
							/*显示菜单不变，只是移动了选择*/
							menu_ctrl.d++;
						}
						}
						else
						{
							/*已经是最后一个菜单，从头开始显示第一个菜单	*/
							break;
						}
				}
			}
			break;
			
		case EMENU_KEYU_ENTER://enter
			/*保证子菜单有效*/
			if(menu_ctrl.sel->l == (menu_ctrl.fa->l)+1)
			{
				if(menu_ctrl.sel->type == MENU_TYPE_FUN)
				{
					if(menu_ctrl.sel->fun != NULL)
						/*执行真实测试函数*/
						menu_ctrl.sel->fun();
				}
				else
				{
					wjq_log(LOG_DEBUG, "sel:%s\r\n", menu_ctrl.sel->cha);
					wjq_log(LOG_DEBUG, "dis:%s\r\n", menu_ctrl.dis->cha);
					
					emenu_stack[(menu_ctrl.fa->l)*2] = menu_ctrl.sel;
					emenu_stack[(menu_ctrl.fa->l)*2+1] = menu_ctrl.dis;
					
					d_rec[menu_ctrl.fa->l] = menu_ctrl.d;
					/* 进入下级菜单 */
					menu_ctrl.fa = 	menu_ctrl.sel;
					menu_ctrl.sel = menu_ctrl.fa+1;
					menu_ctrl.dis = menu_ctrl.sel;
					menu_ctrl.d = 0;
				}
			}
			break;
			
		case EMENU_KEYU_ESC://ESC
			menup = emenu_find_fa(menu_ctrl.fa);
			if(menup != NULL)
			{
				menu_ctrl.fa = 	menup;
				
				menu_ctrl.sel = emenu_stack[(menu_ctrl.fa->l)*2];
				menu_ctrl.dis = emenu_stack[(menu_ctrl.fa->l)*2+1];
				menu_ctrl.d = d_rec[menu_ctrl.fa->l];

				wjq_log(LOG_DEBUG, "sel:%s\r\n", menu_ctrl.sel->cha);
				wjq_log(LOG_DEBUG, "dis:%s\r\n", menu_ctrl.dis->cha);

			}
			break;
		default:
			return -1;
			break;
	}
	return 0;
}

s32 emenu_deal_key_2col(u8 key)
{
	MENU *menup;
	u8 index;
	
	switch(key)
	{
	
		case EMENU_KEYU_DOWM://down
			menup = emunu_find_next(menu_ctrl.sel, 1);
			if(menup != 0)
			{
				menu_ctrl.dis = menup;	
			}
			break;
			
		case EMENU_KEYU_UP://up
			wjq_log(LOG_DEBUG, "2COL UP\r\n");
			u8 i = 0;
			while(1)
			{
				menup = emunu_find_next(menu_ctrl.dis, 0);
				if(menup != 0)
				{
					menu_ctrl.dis = menup;
				}
				else
					break;
				
				u8 col2_dis_num;
				/*
					数字键只有0-9，因此双列菜单最多显示8个菜单一页，用1-8按键选择
				*/
				col2_dis_num = (menu_ctrl.line-1)*2;
				if(col2_dis_num > 8)
					col2_dis_num = 8;
				
				i++;
				if( i>= col2_dis_num)
					break;
			}
			break;
			
		case EMENU_KEYU_ESC://ESC
			menup = emenu_find_fa(menu_ctrl.fa);
			if(menup != NULL)
			{
				menu_ctrl.fa = 	menup;
				
				menu_ctrl.sel = emenu_stack[(menu_ctrl.fa->l)*2];
				menu_ctrl.dis = emenu_stack[(menu_ctrl.fa->l)*2+1];
				menu_ctrl.d = d_rec[menu_ctrl.fa->l];

				wjq_log(LOG_DEBUG, "sel:%s\r\n", menu_ctrl.sel->cha);
				wjq_log(LOG_DEBUG, "dis:%s\r\n", menu_ctrl.dis->cha);
			}
			break;

		case EMENU_KEYU_1:
		case EMENU_KEYU_2:
		case EMENU_KEYU_3:
		case EMENU_KEYU_4:
		case EMENU_KEYU_5:
		case EMENU_KEYU_6:
		case EMENU_KEYU_7:
		case EMENU_KEYU_8:
		case EMENU_KEYU_9:	
			index = key-'0';
			wjq_log(LOG_DEBUG, "index:%d\r\n", index);
			if(index <= menu_ctrl.d)
			{
				menup = menu_ctrl.dis;
				while(1)
				{
					index--;
					if(index == 0)
						break;
					
					menup = emunu_find_next(menup, 1);
					if(menup == 0)
						return -1;
				}
				
				if(menup->l == (menu_ctrl.fa->l)+1)
				{
					if(menup->type == MENU_TYPE_FUN)
					{
						if(menup->fun != NULL)
							/*执行真实测试函数*/
							menup->fun();
					}
					else
					{
						wjq_log(LOG_DEBUG, "sel:%s\r\n", menu_ctrl.sel->cha);
						wjq_log(LOG_DEBUG, "dis:%s\r\n", menu_ctrl.dis->cha);
						
						emenu_stack[(menu_ctrl.fa->l)*2] = menu_ctrl.sel;
						emenu_stack[(menu_ctrl.fa->l)*2+1] = menu_ctrl.dis;
						d_rec[menu_ctrl.fa->l] = menu_ctrl.d;
						
						/* 进入下级菜单 */
						menu_ctrl.fa = 	menup;
						menu_ctrl.sel = menu_ctrl.fa+1;
						menu_ctrl.dis = menu_ctrl.sel;
						menu_ctrl.d = 0;
					}
				}	
				
			}
			else
			{
				wjq_log(LOG_DEBUG, "out list\r\n");
			}
			break;
		
		default:
			return -1;
			break;
	}

	return 0;
}


/**
 *@brief:      emenu_run
 *@details:    运行一个菜单
 *@param[in]   MENU *p  菜单首指针  
               u16 len  菜单长度
 *@param[out]  无
 *@retval:     
 */
s32 emenu_run(DevLcdNode *lcd, MENU *p, u16 len, FontType font, u8 spaced)
{
	u8 disflag = 1;
	s32 ret;

	#if 0
	MENU *menup = p;
	u16 i;
	uart_printf("menu len:%d\r\n", len);
	for(i=0; i< len; i++)
	{
		uart_printf("%s\r\n", menup->cha);
		menup++;
	}
	#endif
	
	menu_ctrl.lcdw = lcd->width;
	menu_ctrl.lcdh = lcd->height;
	menu_ctrl.font = font;
	font_get_hw(font, &(menu_ctrl.fonth), &(menu_ctrl.fontw));
	
	menu_ctrl.spaced = spaced;
	
	menu_ctrl.lang = MENU_LANG_CHA;//语言选择
	
	menu_ctrl.fa = p;//初始化，显示根菜单，也就是唯一MENU_L_0菜单
	menu_ctrl.sel = p+1;//第二个菜单肯定是第一个MENU_L_1
	menu_ctrl.dis = menu_ctrl.sel;
	menu_ctrl.d = 0;
	menu_ctrl.line = (menu_ctrl.lcdh+1)/(menu_ctrl.fonth + menu_ctrl.spaced);//+1，因为最后一行可以不要间距，
	
	wjq_log(LOG_DEBUG, "line:%d\r\n", menu_ctrl.line);
	
	while(1)
	{
		if(1==disflag)
		{
			emenu_display(lcd);
			disflag = 0;
		}
		
		u8 key;

		key = emenu_get_key();
		
		if(key != 0)
		{
			//wjq_log(LOG_DEBUG, "KEY:%02x\r\n", key);
			
			if(menu_ctrl.fa->type == MENU_TYPE_LIST)
			{
				ret = emenu_deal_key_list(key);
			}
			else if(menu_ctrl.fa->type == MENU_TYPE_KEY_2COL)
			{
				ret = emenu_deal_key_2col(key);
			}
			//wjq_log(LOG_DEBUG,"get a key:%02x\r\n", key);
			if(ret == 0)
				disflag = 1;

		}
	}
		
}

