/**
 * @file            mcu_spi.c
 * @brief           片上SPI控制器驱动
 * @author          test
 * @date            2017年10月26日 星期四
 * @version         初稿
 * @par             
 * @par History:
 * 1.日    期:      2017年10月26日 星期四
 *   作    者:      test
 *   修改内容:      创建文件
		版权说明：
		1 源码归屋脊雀工作室所有。
		2 可以用于的其他商业用途（配套开发板销售除外），不须授权。
		3 屋脊雀工作室不对代码功能做任何保证，请使用者自行测试，后果自负。
		4 可随意修改源码并分发，但不可直接销售本代码获利，并且保留版权说明。
		5 如发现BUG或有优化，欢迎发布更新。请联系：code@wujique.com
		6 使用本源码则相当于认同本版权说明。
		7 如侵犯你的权利，请联系：code@wujique.com
		8 一切解释权归屋脊雀工作室所有。
*/
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "list.h"
#include "mcu_spi.h"
#include "wujique_sysconf.h"

#define MCU_SPI_DEBUG

#ifdef MCU_SPI_DEBUG
#define SPI_DEBUG	wjq_log 
#else
#define SPI_DEBUG(a, ...)
#endif


#define MCU_SPI_WAIT_TIMEOUT 0x40000
/*
	硬件SPI使用控制器SPI3
*/
#define SPI_DEVICE SPI3
/*
	相位配置，一共四种模式
*/
typedef struct
{
	u16 CPOL;
	u16 CPHA;	
}_strSpiModeSet;

const _strSpiModeSet SpiModeSet[SPI_MODE_MAX]=
	{
		{SPI_CPOL_Low, SPI_CPHA_1Edge},
		{SPI_CPOL_Low, SPI_CPHA_2Edge},
		{SPI_CPOL_High, SPI_CPHA_1Edge},
		{SPI_CPOL_High, SPI_CPHA_2Edge}
	};

/**
 *@brief:      mcu_spi_init
 *@details:    初始化SPI控制器，暂时支持SPI3
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_hspi_init(const DevSpi *dev)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStruct;


    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;//---PB3~5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//---复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//---推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//---100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//---上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//---初始化

    //配置引脚复用映射
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3); //PB3 复用为 SPI3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3); //PB4 复用为 SPI3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3); //PB5 复用为 SPI3

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);// ---使能 SPI3 时钟
    // 复位SPI模块
    SPI_I2S_DeInit(SPI_DEVICE);

    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//---双线双向全双工
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;//---主模式
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;//---8bit帧结构
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;//----串行同步时钟的空闲状态为低电平
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;//---数据捕获于第1个时钟沿
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; //---SPI_NSS_Hard; 片选由硬件管理，SPI控制器不管理
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;  //---预分频
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;//---数据传输从 MSB 位开始
    SPI_InitStruct.SPI_CRCPolynomial = 7;//---CRC 值计算的多项式

    SPI_Init(SPI_DEVICE, &SPI_InitStruct);
    
    return 0;
}

/**
 *@brief:      mcu_spi_open
 *@details:       打开SPI
 *@param[in]   SPI_DEV dev  ：SPI号
               u8 mode      模式
               u16 pre      预分频
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_hspi_open(DevSpiNode *node, SPI_MODE mode, u16 pre)
{
	SPI_InitTypeDef SPI_InitStruct;

	if(node->gd != -1)
		return -1;

	if(mode >= SPI_MODE_MAX)
		return -1;

	SPI_I2S_DeInit(SPI_DEVICE);
	SPI_Cmd(SPI_DEVICE, DISABLE); 
	
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//---双线双向全双工
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;//---主模式
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;//---8bit帧结构
    SPI_InitStruct.SPI_CPOL = SpiModeSet[mode].CPOL;
    SPI_InitStruct.SPI_CPHA = SpiModeSet[mode].CPHA;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; //---SPI_NSS_Hard; 片选由硬件管理，SPI控制器不管理
    SPI_InitStruct.SPI_BaudRatePrescaler = pre;  //---预分频
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;//---数据传输从 MSB 位开始
    SPI_InitStruct.SPI_CRCPolynomial = 7;//---CRC 值计算的多项式

    SPI_Init(SPI_DEVICE, &SPI_InitStruct);

	SPI_Cmd(SPI_DEVICE, ENABLE);
	
	node->gd = 0;
		
    return 0;
}
/**
 *@brief:      mcu_spi_close
 *@details:    关闭SPI 控制器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_hspi_close(DevSpiNode *node)
{
    
	if(node->gd != 0)
		return -1;

	SPI_Cmd(SPI_DEVICE, DISABLE);
	node->gd = -1;
    return 0;
}
/**
 *@brief:      mcu_spi_transfer
 *@details:    SPI 传输
 *@param[in]   u8 *snd  
               u8 *rsv  
               s32 len  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_hspi_transfer(DevSpiNode *node, u8 *snd, u8 *rsv, s32 len)
{
    s32 i = 0;
    s32 pos = 0;
    u32 time_out = 0;
    u16 ch;

	if(node == NULL)
		return -1;

	if(node->gd != 0)
	{
		SPI_DEBUG(LOG_DEBUG, "spi dev err\r\n");
		return -1;
	}
	
    if( ((snd == NULL) && (rsv == NULL)) || (len < 0) )
    {
        return -1;
    }
    
    /* 忙等待 */
    time_out = 0;
    while(SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_BSY) == SET)
    {
        if(time_out++ > MCU_SPI_WAIT_TIMEOUT)
        {
            return(-1);
        }
    }

    /* 清空SPI缓冲数据，防止读到上次传输遗留的数据 */
    time_out = 0;
    while(SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == SET)
    {
        SPI_I2S_ReceiveData(SPI_DEVICE);
        if(time_out++ > 2)
        {
            return(-1);
        }
    }

    /* 开始传输 */
    for(i=0; i < len; )
    {
        // 写数据
        if(snd == NULL)/*发送指针为NULL，说明仅仅是读数据 */
        {
            SPI_I2S_SendData(SPI_DEVICE, 0xff);
        }
        else
        {
            ch = (u16)snd[i];
            SPI_I2S_SendData(SPI_DEVICE, ch);
        }
        i++;
        
        // 等待接收结束
        time_out = 0;
        while(SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == RESET)
        {
            time_out++;
            if(time_out > MCU_SPI_WAIT_TIMEOUT)
            {
                return -1;
            }    
        }
        // 读数据
        if(rsv == NULL)/* 接收指针为空，读数据后丢弃 */
        {
            SPI_I2S_ReceiveData(SPI_DEVICE);
        }
        else
        {
            ch = SPI_I2S_ReceiveData(SPI_DEVICE);
            rsv[pos] = (u8)ch;
        } 
        pos++;

    }

    return i;
}

/**
 *@brief:      mcu_vspi_init
 *@details:       初始化虚拟SPI
 *@param[in]  void  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_vspi_init(const DevSpi *dev)
{

	wjq_log(LOG_DEBUG, "vspi init:%s\r\n", dev->name);

	mcu_io_config_out(dev->clkport, dev->clkpin);
	mcu_io_output_setbit(dev->clkport,dev->clkpin);

	mcu_io_config_out(dev->mosiport, dev->mosipin);
	mcu_io_output_setbit(dev->mosiport, dev->mosipin);


	mcu_io_config_in(dev->misoport, dev->misopin);
	mcu_io_output_setbit(dev->misoport, dev->misopin);
	
	return 0;
}


/**
 *@brief:      vspi_delay
 *@details:    虚拟SPI时钟延时
 *@param[in]   u32 delay  
 *@param[out]  无
 *@retval:     
 */
void vspi_delay(u32 delay)
{
	volatile u32 i=delay;

	while(i>0)
	{
		i--;	
	}

}

u32 VspiDelay = 0;

/**
 *@brief:      mcu_vspi_open
 *@details:    打开虚拟SPI
 *@param[in]   SPI_DEV dev    
               SPI_MODE mode  
               u16 pre        
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_vspi_open(DevSpiNode *node, SPI_MODE mode, u16 pre)
{

	if(node == NULL)
		return -1;
	
	if(node->gd != -1)
		return -1;

	node->clk = pre;
	node->gd = 0;
		
    return 0;
}
/**
 *@brief:      mcu_vspi_close
 *@details:    关闭虚拟SPI
 *@param[in]   SPI_DEV dev  
 *@param[out]  无
 *@retval:     
 */
static s32 mcu_vspi_close(DevSpiNode *node)
{
	node->gd = -1;
	
    return 0;
}
/**
 *@brief:      mcu_vspi_transfer
 *@details:       虚拟SPI通信
 *@param[in]   SPI_DEV dev  
               u8 *snd      
               u8 *rsv      
               s32 len      
 *@param[out]  无
 *@retval:     

 		node->clk = 0, CLK时钟1.5M 2018.06.02
 */
static s32 mcu_vspi_transfer(DevSpiNode *node, u8 *snd, u8 *rsv, s32 len)
{
	u8 i;
	u8 data;
	s32 slen;
	u8 misosta;

	volatile u16 delay;
	
	DevSpi *dev;
	
	if(node == NULL)
		return -1;

    if( ((snd == NULL) && (rsv == NULL)) || (len < 0) )
    {
        return -1;
    }

	dev = &(node->dev);

	slen = 0;

	while(1)
	{
		if(slen >= len)
			break;

		if(snd == NULL)
			data = 0xff;
		else
			data = *(snd+slen);
		
		for(i=0; i<8; i++)
		{
			mcu_io_output_resetbit(dev->clkport, dev->clkpin);

			delay = node->clk;
			while(delay>0)
			{
				delay--;	
			}
			
			if(data&0x80)
				mcu_io_output_setbit(dev->mosiport, dev->mosipin);
			else
				mcu_io_output_resetbit(dev->mosiport, dev->mosipin);
			
			delay = node->clk;
			while(delay>0)
			{
				delay--;	
			}
			
			data<<=1;
			mcu_io_output_setbit(dev->clkport, dev->clkpin);
			
			delay = node->clk;
			while(delay>0)
			{
				delay--;	
			}
			
			misosta = mcu_io_input_readbit(dev->misoport, dev->misopin);
			if(misosta == Bit_SET)
			{
				data |=0x01;
			}
			else
			{
				data &=0xfe;
			}
			
			delay = node->clk;
			while(delay>0)
			{
				delay--;	
			}
			
		}

		if(rsv != NULL)
			*(rsv+slen) = data;
		
		slen++;
	}

	return slen;
}

/*


	所有SPI统一对外接口


*/
struct list_head DevSpiRoot = {&DevSpiRoot, &DevSpiRoot};
/**
 *@brief:      mcu_spi_register
 *@details:    注册SPI控制器设备
 *@param[in]   DevSpi *dev      
 *@param[out]  无
 *@retval:     
 */
s32 mcu_spi_register(const DevSpi *dev)
{

	struct list_head *listp;
	DevSpiNode *p;
	
	wjq_log(LOG_INFO, "[register] spi :%s!\r\n", dev->name);

	/*
		先要查询当前，防止重名
	*/
	listp = DevSpiRoot.next;
	while(1)
	{
		if(listp == &DevSpiRoot)
			break;

		p = list_entry(listp, DevSpiNode, list);

		if(strcmp(dev->name, p->dev.name) == 0)
		{
			wjq_log(LOG_INFO, "spi dev name err!\r\n");
			return -1;
		}
		
		listp = listp->next;
	}

	/* 
		申请一个节点空间 
		
	*/
	p = (DevSpiNode *)wjq_malloc(sizeof(DevSpiNode));
	list_add(&(p->list), &DevSpiRoot);

	memcpy((u8 *)&p->dev, (u8 *)dev, sizeof(DevSpi));
	p->gd = -1;

	/*初始化*/
	if(dev->type == DEV_SPI_V)
		mcu_vspi_init(dev);
	else if(dev->type == DEV_SPI_H)
		mcu_hspi_init(dev);
	
	return 0;
}

struct list_head DevSpiChRoot = {&DevSpiChRoot, &DevSpiChRoot};
/**
 *@brief:      mcu_spich_register
 *@details:    注册SPI通道
 *@param[in]   DevSpiCh *dev     
 *@param[out]  无
 *@retval:     
 */
s32 mcu_spich_register(const DevSpiCh *dev)
{
	struct list_head *listp;
	DevSpiChNode *p;
	DevSpiNode *p_spi;
	
	wjq_log(LOG_INFO, "[register] spi ch :%s!\r\n", dev->name);

	/*
		先要查询当前，防止重名
	*/
	listp = DevSpiChRoot.next;
	while(1)
	{
		if(listp == &DevSpiChRoot)
			break;

		p = list_entry(listp, DevSpiChNode, list);
		
		if(strcmp(dev->name, p->dev.name) == 0)
		{
			wjq_log(LOG_INFO, ">--------------------spi ch dev name err!\r\n");
			return -1;
		}
		
		listp = listp->next;
	}

	/* 寻找SPI控制器*/
	listp = DevSpiRoot.next;
	while(1)
	{
		if(listp == &DevSpiRoot)
		{
			wjq_log(LOG_INFO, ">---------------------spi ch reg err:no spi!\r\n");
			return -1;
		}


		p_spi = list_entry(listp, DevSpiNode, list);

		if(strcmp(dev->spi, p_spi->dev.name) == 0)
		{
			//wjq_log(LOG_INFO, "spi ch find spi!\r\n");
			break;
		}
		
		listp = listp->next;
	}
	/* 
		申请一个节点空间 
		
	*/
	p = (DevSpiChNode *)wjq_malloc(sizeof(DevSpiChNode));
	list_add(&(p->list), &DevSpiChRoot);
	memcpy((u8 *)&p->dev, (u8 *)dev, sizeof(DevSpiCh));
	p->gd = -1;
	p->spi = p_spi;

	/* 初始化管脚 */
	mcu_io_config_out(dev->csport,dev->cspin);
	mcu_io_output_setbit(dev->csport,dev->cspin);

	return 0;
}


/**
 *@brief:      mcu_spi_open
 *@details:    打开SPI
 *@param[in]   SPI_DEV dev  ：SPI号
               u8 mode      模式
               u16 pre      预分频
 *@param[out]  无
 *@retval:     
 */
DevSpiChNode *mcu_spi_open(char *name, SPI_MODE mode, u16 pre)
{

	s32 res;
	DevSpiChNode *node;
	struct list_head *listp;
	
	//SPI_DEBUG(LOG_INFO, "spi ch open:%s!\r\n", name);

	listp = DevSpiChRoot.next;
	node = NULL;
	
	while(1)
	{
		if(listp == &DevSpiChRoot)
			break;

		node = list_entry(listp, DevSpiChNode, list);
		//SPI_DEBUG(LOG_INFO, "spi ch name%s!\r\n", node->dev.name);
		
		if(strcmp(name, node->dev.name) == 0)
		{
			//SPI_DEBUG(LOG_INFO, "spi ch dev get ok!\r\n");
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
		
		if(node->gd == 0)
		{
			SPI_DEBUG(LOG_INFO, "spi ch open err:using!\r\n");
			node = NULL;
		}
		else
		{
			/*打开SPI控制器*/
			if(node->spi->dev.type == DEV_SPI_H)
			{
				res = mcu_hspi_open(node->spi, mode, pre);	
			}
			else if(node->spi->dev.type == DEV_SPI_V)
			{
				res = mcu_vspi_open(node->spi, mode, pre);	
			}

			if(res == 0)
			{
				node->gd = 0;
				//SPI_DEBUG(LOG_INFO, "spi dev open ok: %s!\r\n", name);
				mcu_io_output_resetbit(node->dev.csport, node->dev.cspin);
			}
			else
			{
				SPI_DEBUG(LOG_INFO, "spi dev open err!\r\n");
				node = NULL;
			}
		}
	}
	else
	{
		SPI_DEBUG(LOG_INFO, "spi ch no exist!\r\n");	
	}
	
	return node;
}


/**
 *@brief:      mcu_spi_close
 *@details:    关闭SPI 控制器
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_spi_close(DevSpiChNode * node)
{
	if(node->spi->dev.type == DEV_SPI_H)
	{
		mcu_hspi_close(node->spi);
	}
	else
		mcu_vspi_close(node->spi);
	
	/*拉高CS*/
	mcu_io_output_setbit(node->dev.csport, node->dev.cspin);
	node->gd = -1;
 
	return 0;
}
/**
 *@brief:      mcu_spi_transfer
 *@details:    SPI 传输
 *@param[in]   u8 *snd  
               u8 *rsv  
               s32 len  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_spi_transfer(DevSpiChNode * node, u8 *snd, u8 *rsv, s32 len)
{
	if(node == NULL)
		return -1;

	if(node->spi->dev.type == DEV_SPI_H)
		return mcu_hspi_transfer(node->spi, snd, rsv, len);
	else	
		return mcu_vspi_transfer(node->spi, snd, rsv, len);
}
/**
 *@brief:      mcu_spi_cs
 *@details:    操控对应SPI的CS
 *@param[in]   SPI_DEV dev  
               u8 sta       
 *@param[out]  无
 *@retval:     
 */
s32 mcu_spi_cs(DevSpiChNode * node, u8 sta)
{
	switch(sta)
	{
		case 1:
			mcu_io_output_setbit(node->dev.csport, node->dev.cspin);
			break;
			
		case 0:
			mcu_io_output_resetbit(node->dev.csport, node->dev.cspin);
			break;
			
		default:
			return -1;

	}

	return 0;
}



