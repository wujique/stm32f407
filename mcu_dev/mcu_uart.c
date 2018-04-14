/**
 * @file            mcu_uart.c
 * @brief           串口驱动
 * @author          wujique
 * @date            2017年10月24日 星期二
 * @version         初稿
 * @par             
 * @par History:
 * 1.日    期:      2017年10月24日 星期二
 *   作    者:     
 *   修改内容:        创建文件
		版权说明：
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
#include "stm32f4xx_usart.h"
#include "mcu_uart.h"
#include "wujique_log.h"


#define RX3_TEMP_BUF_LEN_MAX       4096/*串口接收缓冲长度*/

volatile u8 UartBuf3[RX3_TEMP_BUF_LEN_MAX];//串口接收缓冲
volatile u16 UartHead3;//串口接收缓冲写指针
volatile u16 UartEnd3;//串口接收缓冲读指针
volatile u8 UartBuf3OverFg = 0;//缓冲溢出标志

/**
 *@brief:      mcu_dev_uart_open
 *@details:    初始化串口
 *@param[in]   s32 comport  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_uart_open(s32 comport)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    UartHead3 = 0;
    UartEnd3 = 0;

    // 打开GPIO时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    // 打开串口时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	//GPIOB10 复用为 USART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    //GPIOB11 复用为 USART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);
    
    // TXD ---- PB10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // RXD ---- PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //USART 初始化设置
    USART_InitStructure.USART_BaudRate = 115200;//波特率;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为 8 位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发
    USART_Init(USART3, &USART_InitStructure); //初始化串口
    USART_Cmd(USART3, ENABLE); //使能串口
    USART_ClearFlag(USART3, USART_FLAG_TC);//清中断标志

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
    //Usart3 NVIC 配置，主要是配置中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级 3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =3; //响应优先级 3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能中断
    NVIC_Init(&NVIC_InitStructure); //执行NVIC配置
 
    return (0);
}
/**
 *@brief:      mcu_uart_close
 *@details:    关闭串口
 *@param[in]   s32 comport  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_uart_close (s32 comport)
{

    UartHead3 = 0;
    UartEnd3 = 0;
    //根据硬件决定是否要重新配置IO口。
    USART_Cmd(USART3, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);

    return(0);
}
/**
 *@brief:      mcu_uart_tcflush
 *@details:    清串口接收缓冲
 *@param[in]   s32 comport  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_uart_tcflush(s32 comport)
{ 
    UartHead3 = UartEnd3;

    return 0;
}
/**
 *@brief:      mcu_uart_set_baud
 *@details:       设置串口波特率
 *@param[in]  s32 comport   
               s32 baud      
               s32 databits  
               s32 parity    
               s32 stopbits  
               s32 flowctl   
 *@param[out]  无
 *@retval:     
 */
s32 mcu_uart_set_baud (s32 comport, s32 baud, s32 databits, s32 parity, s32 stopbits, s32 flowctl)
{
    USART_InitTypeDef USART_InitStructure;

    if( mcu_uart_open (comport) == -1) return(-1);
    
    // 关闭串口
    USART_Cmd(USART3, DISABLE);
    USART_InitStructure.USART_BaudRate = baud; 
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART3, &USART_InitStructure);
    // 打开串口
    mcu_uart_tcflush(comport);
    USART_Cmd(USART3, ENABLE); 

    return 0;
}
/**
 *@brief:      mcu_uart_read
 *@details:    读串口数据
 *@param[in]   s32 comport  
               u8 *buf      
               s32 len      
 *@param[out]  无
 *@retval:     
 */
s32 mcu_uart_read (s32 comport, u8 *buf, s32 len)
{
    s32 i;
    
    if(len <= 0) return(-1);
    if(buf == NULL) return(-1);

    i = 0;

    //uart_printf("rec index:%d, %d\r\n", UartHead3, rec_end3);
    while(UartHead3 != UartEnd3)
    {
        *buf = UartBuf3[UartHead3++];
        if(UartHead3 >= RX3_TEMP_BUF_LEN_MAX) 
            UartHead3 = 0;

        buf ++;
        i ++;
        if(i >= len)
        {
            break;
        }
  }
  return (i);
}
/**
 *@brief:      mcu_uart_write
 *@details:    写串口数据
 *@param[in]   s32 comport  
               u8 *buf      
               s32 len      
 *@param[out]  无
 *@retval:     
 */
s32 mcu_uart_write (s32 comport, u8 *buf, s32 len)
{
    u32 t;
    u16 ch;
  
    if (len <= 0) 
        return(-1);
        
    if(buf == NULL) 
        return(-1);
 
    while(len != 0)
    {
        
        t = 0;
		// 等待串口发送完毕
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
        {
            if(t++ >= 0x1000000)//超时
                return(-1);
        }  
        ch = (u16)(*buf & 0xff);
        USART_SendData(USART3, ch);
        buf++;
        len--;
    }
    
    return(0);
}
/**
 *@brief:      mcu_uart3_IRQhandler
 *@details:    串口中断处理函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_uart3_IRQhandler(void)
{
    unsigned short ch;

    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//判断是不是RXNE中断
    {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);//清RX中断
        if(USART_GetITStatus(USART3, USART_IT_ORE) != RESET)
        {
            // 串口数据接收字符有丢失
            //wjq_log(LOG_DEBUG, "1");
            ch = USART_ReceiveData(USART3);
            USART_GetITStatus(USART3, USART_IT_ORE); // 清除ORE标记
            UartEnd3 = UartHead3;
            UartBuf3OverFg = 1;
        }
        else
        {
            ch = USART_ReceiveData(USART3);
            //uart_printf("%02x", ch);
            UartBuf3[UartEnd3++] = (unsigned char)(ch&0xff);
            if(UartEnd3 >= RX3_TEMP_BUF_LEN_MAX)
                UartEnd3 = 0;
                
            if(UartEnd3 == UartHead3)       // 串口接收缓冲溢出了
                UartBuf3OverFg = 1;
        }
    }
    
    if(USART_GetITStatus(USART3, USART_IT_FE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART3, USART_IT_FE);
        USART_ReceiveData(USART3);

    }
#if 0
    /* If the USART3 detects a parity error */
    if(USART_GetITStatus(USART3, USART_IT_PE) != RESET)
    {
        while(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET)
        {
        }
        /* Clear the USART3 Parity error pending bit */
        USART_ClearITPendingBit(USART3, USART_IT_PE);
        USART_ReceiveData(USART3);
    }
    /* If a Overrun error is signaled by the card */
    if(USART_GetITStatus(USART3, USART_IT_ORE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART3, USART_IT_ORE);
        USART_ReceiveData(USART3);
    }
    /* If a Noise error is signaled by the card */
    if(USART_GetITStatus(USART3, USART_IT_NE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART3, USART_IT_NE);
        USART_ReceiveData(USART3);
    }
#endif    
}

/**
 *@brief:      mcu_uart_test
 *@details:    串口测试
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_uart_test(void)
{
    u8 buf[12];
    s32 len;
    s32 res;
    
    len =  mcu_uart_read (3, buf, 10);
    wjq_log(LOG_FUN, "mcu_dev_uart_read :%d\r\n", len);
    res = mcu_uart_write(3, buf, len);
    wjq_log(LOG_FUN, "mcu_dev_uart_write res: %d\r\n", res);
    wjq_log(LOG_FUN, "%s,%s,%d,%s\r\n", __FUNCTION__,__FILE__,__LINE__,__DATE__);
    
}

