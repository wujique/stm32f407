/**
 * @file            
 * @brief           数学函数
 * @author          wujique
 * @date            2018年06月26日 星期二
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:      
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

#include "stm32f4xx.h"
#include "wujique_log.h"

/*

非递归判断一个数是2的多少次方

*/
int log2(int value) 
{ 
  int x=0; 
  while(value>1) 
  { 
	value>>=1; 
	x++; 
  } 
  return x; 
} 

/*
	对输入的U16 进行希尔排序
*/
void ShellSort(u16 *pSrc, s32 Len)
{
    u32 d;//增量
    s32 j, k;

    u16 temp;

    //uart_printf("shell:%d\r\n", Len);

    for(d = Len/2; d >0; d=d/2)
    {
        for(j = d; j < Len; j++)
        {
            temp = *(pSrc + j);

            for(k = j - d; (k >=0 && temp <(*(pSrc + k))); k -=d)
            {
                *(pSrc + k + d) = *(pSrc + k);
            }

            *(pSrc + k + d) = temp;
        }
    }
}

/*
	函数名称: 
	函数功能: 二分法查找u16
	入口参数: 
	返 回 值: 返回-1没找到，其他为索引值
*/
int BinarySearch(u16 *pArray, u16 data, int front, int end)
{
    int low, high, mid;
    low = front;
    high = end;
    while(low <= high)
    {

        mid = (low + high) / 2;
        if (*(pArray + mid) < data)
        {
            low = mid + 1;
        }
        else if (*(pArray + mid) > data)
        {
            high = mid - 1;
        }
        else 
        {
            return mid;
        }
    }
    return -1;
}

