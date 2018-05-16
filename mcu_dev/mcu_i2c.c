/**
 * @file            mcu_i2c.c
 * @brief           IO模拟I2C
 * @author          test
 * @date            2017年10月30日 星期一
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:      2017年10月30日 星期一
 *   作    者:      屋脊雀工作室
 *   修改内容:      创建文件
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
#include "list.h"
#include "mcu_i2c.h"
#include "alloc.h"

#define MCU_I2C_DEBUG

#ifdef MCU_I2C_DEBUG
#define I2C_DEBUG	wjq_log 
#else
#define I2C_DEBUG(a, ...)
#endif


#define MCU_I2C_TIMEOUT 250


/**
 *@brief:      mcu_i2c_delay
 *@details:    I2C信号延时函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_delay(void)
{
    //Delay(1);//延时，I2C时钟
    volatile u32 i = 5;

    for(;i>0;i--);
}

/**
 *@brief:      mcu_i2c_sda_input
 *@details:    将I2C sda IO设置为输入
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_i2c_sda_input(DevI2c *dev)
{
	mcu_io_config_in(dev->sdaport, dev->sdapin);
}
/**
 *@brief:      mcu_i2c_sda_output
 *@details:       将I2C sda IO设置为输出
 *@param[in]  void  
 *@param[out]  无
 *@retval:     
 */
void mcu_i2c_sda_output(DevI2c *dev)
{
	mcu_io_config_out(dev->sdaport, dev->sdapin);
}
/**
 *@brief:      mcu_i2c_readsda
 *@details:    读SDA数据
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static s32 mcu_i2c_readsda(DevI2c *dev)
{

    if(Bit_SET == mcu_io_input_readbit(dev->sdaport, dev->sdapin))
        return 1;
    else
        return 0;
}
/**
 *@brief:      mcu_i2c_sda
 *@details:       SDA输出高低电平
 *@param[in]  u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_sda(DevI2c *dev, u8 sta)
{

    if(sta == 1)
    {
		mcu_io_output_setbit(dev->sdaport, dev->sdapin);
    }
    else if(sta == 0)
    {
    	mcu_io_output_resetbit(dev->sdaport, dev->sdapin);
    }

}

/**
 *@brief:      mcu_i2c_scl
 *@details:    时钟SCL输出高低电平
 *@param[in]   u8 sta  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_scl(DevI2c *dev, u8 sta)
{

	if(sta == 1)
    {
		mcu_io_output_setbit(dev->sclport, dev->sclpin);
    }
    else if(sta == 0)
    {
    	mcu_io_output_resetbit(dev->sclport, dev->sclpin);
    }
}
/**
 *@brief:      mcu_i2c_start
 *@details:    发送start时序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_start(DevI2c *dev)
{
    mcu_i2c_sda_output(dev);
    
    mcu_i2c_sda(dev, 1);  
    mcu_i2c_scl(dev, 1);

    mcu_i2c_delay();
    mcu_i2c_sda(dev, 0);

    mcu_i2c_delay();
    mcu_i2c_scl(dev, 0);
}
/**
 *@brief:      mcu_i2c_stop
 *@details:    发送I2C STOP时序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_stop(DevI2c *dev)
{
    mcu_i2c_sda_output(dev);

    mcu_i2c_scl(dev, 0);
    mcu_i2c_sda(dev, 0);   
    mcu_i2c_delay();
    
    mcu_i2c_scl(dev, 1);
    mcu_i2c_delay();
    
    mcu_i2c_sda(dev, 1);
    mcu_i2c_delay();
}

/**
 *@brief:      mcu_i2c_wait_ack
 *@details:       等待ACK信号
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static s32 mcu_i2c_wait_ack(DevI2c *dev)
{
    u8 time_out = 0;
    
    //sda转输入
    mcu_i2c_sda_input(dev);
    mcu_i2c_sda(dev, 1);
    mcu_i2c_delay();
    
    mcu_i2c_scl(dev, 1);
    mcu_i2c_delay();
    
    while(1)
    {
        time_out++;
        if(time_out > MCU_I2C_TIMEOUT)
        {
            mcu_i2c_stop(dev);
            //wjq_log(LOG_ERR, "i2c:wait ack time out!\r\n");
            return 1;
        }

        if(0 == mcu_i2c_readsda(dev))
        {   
            break;
        }
    }
    
    mcu_i2c_scl(dev, 0);
    
    return 0;
}
/**
 *@brief:      mcu_i2c_ack
 *@details:       发送ACK信号
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_ack(DevI2c *dev)
{
    mcu_i2c_scl(dev, 0);
    mcu_i2c_sda_output(dev);
    
    mcu_i2c_sda(dev, 0);
    mcu_i2c_delay();
    
    mcu_i2c_scl(dev, 1);
    mcu_i2c_delay();
    
    mcu_i2c_scl(dev, 0);
}
/**
 *@brief:      mcu_i2c_writebyte
 *@details:       I2C总线写一个字节数据
 *@param[in]  u8 data  
 *@param[out]  无
 *@retval:     static
 */
static s32 mcu_i2c_writebyte(DevI2c *dev, u8 data)
{
    u8 i = 0;

    mcu_i2c_sda_output(dev);

    mcu_i2c_scl(dev, 0);
    
    while(i<8)
    {
    
        if((0x80 & data) == 0x80)
        {
            mcu_i2c_sda(dev, 1);   
        }
        else
        {
            mcu_i2c_sda(dev, 0);
        }
        
        mcu_i2c_delay();

        mcu_i2c_scl(dev, 1);
        mcu_i2c_delay();
        
        mcu_i2c_scl(dev, 0);
        mcu_i2c_delay();
        
        data = data <<1;
        i++;
    }
		return 0;
}
/**
 *@brief:      mcu_i2c_readbyte
 *@details:       I2C总线读一个字节数据
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static u8 mcu_i2c_readbyte(DevI2c *dev)
{
    u8 i = 0;
    u8 data = 0;

    mcu_i2c_sda_input(dev);
    
    while(i<8)
    {
        mcu_i2c_scl(dev, 0);
        mcu_i2c_delay();
        
        mcu_i2c_scl(dev, 1);

        data = (data <<1);

        if(1 == mcu_i2c_readsda(dev))
        {
            data =data|0x01;    
        }
        else
        {
            data = data & 0xfe;
        }

        mcu_i2c_delay();
        
        i++;
    }

    return data;
}

/**
 *@brief:      mcu_i2c_transfer
 *@details:    中间无重新开始位的传输流程
 *@param[in]   u8 addr   
               u8 rw    0 写，1 读    
               u8* data  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_i2c_transfer(DevI2cNode *node, u8 addr, u8 rw, u8* data, s32 datalen)
{
    s32 i;
    u8 ch;
	DevI2c *dev;
	s32 res;
	
    #if 0//测试IO口是否连通
    while(1)
    {
        uart_printf("test \r\n");
        mcu_i2c_scl(1);
        mcu_i2c_sda(1); 
        Delay(5);
        mcu_i2c_scl(0);
        mcu_i2c_sda(0); 
        Delay(5);
    }   
    #endif

	if(node == NULL)
		return -1;

	dev = &node->dev;
	
	//I2C_DEBUG(LOG_DEBUG, "i2c trf %s\r\n", dev->name);
    //发送起始
    mcu_i2c_start(dev);
    //发送地址+读写标志
    //处理ADDR
    if(rw == MCU_I2C_MODE_W)
    {
        addr = ((addr<<1)&0xfe);
        //uart_printf("write\r\n");
    }
    else
    {
        addr = ((addr<<1)|0x01);
        //uart_printf("read\r\n");
    }
    
    mcu_i2c_writebyte(dev, addr);
	
    res = mcu_i2c_wait_ack(dev);
	if(res == 1)
		return 1;
	
    i = 0;

    //数据传输
    if(rw == MCU_I2C_MODE_W)//写
    {
	    while(i < datalen)
	 	{
            ch = *(data+i);
            mcu_i2c_writebyte(dev, ch);
            res = mcu_i2c_wait_ack(dev);
			if(res == 1)
				return 1;
			
			i++;
	    }
    }
    else if(rw == MCU_I2C_MODE_R)//读
    {
       	while(i < datalen)
	 	{
            ch = mcu_i2c_readbyte(dev);  
            mcu_i2c_ack(dev);
            *(data+i) = ch;
			i++;
	    }
    }

    //发送结束
    mcu_i2c_stop(dev);
    return 0;
}


/*

	I2C模块维护一个链表，记录有当前初始化的I2C控制器

*/
struct list_head DevI2cGdRoot = {&DevI2cGdRoot, &DevI2cGdRoot};


/**
 *@brief:      mcu_i2c_register
 *@details:    初始化I2C接口， 相当于注册一个I2C设备
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_i2c_register(const DevI2c * dev)
{
    
	struct list_head *listp;
	DevI2cNode *p;
	
	wjq_log(LOG_INFO, "[register]  i2c:%s!\r\n", dev->name);

	/*
		先要查询当前I2C控制器，防止重名
	*/
	listp = DevI2cGdRoot.next;
	while(1)
	{
		if(listp == &DevI2cGdRoot)
			break;

		p = list_entry(listp, DevI2cNode, list);
		//wjq_log(LOG_INFO, "i2c dev name:%s!\r\n", p->dev.name);
		
		if(strcmp(dev->name, p->dev.name) == 0)
		{
			wjq_log(LOG_INFO, "i2c dev name err!\r\n");
			return -1;
		}
		
		listp = listp->next;
	}

	/* 
		申请一个节点空间 
		
	*/
	p = (DevI2cNode *)wjq_malloc(sizeof(DevI2cNode));
	list_add(&(p->list), &DevI2cGdRoot);
	
	/*
		初始化设备节点
	*/
	memcpy((u8 *)&p->dev, (u8 *)dev, sizeof(DevI2c));
	p->gd = -1;

	/*初始化IO口状态*/
	mcu_io_config_out(dev->sclport, dev->sclpin);
	mcu_io_config_out(dev->sdaport, dev->sdapin);

	mcu_io_output_setbit(dev->sdaport, dev->sdapin);
	mcu_io_output_setbit(dev->sclport, dev->sclpin); 

	return 0;
}

DevI2cNode *mcu_i2c_open(char *name)
{

	DevI2cNode *node;
	struct list_head *listp;
	
	//I2C_DEBUG(LOG_INFO, "i2c open:%s!\r\n", name);

	listp = DevI2cGdRoot.next;
	node = NULL;
	
	while(1)
	{
		if(listp == &DevI2cGdRoot)
			break;

		node = list_entry(listp, DevI2cNode, list);
		//I2C_DEBUG(LOG_INFO, "i2c dev name:%s!\r\n", node->dev.name);
		
		if(strcmp(name, node->dev.name) == 0)
		{
			//I2C_DEBUG(LOG_INFO, "i2c dev open ok!\r\n");
			break;
		}
		
		listp = listp->next;
	}

	if(node != NULL)
	{
		if(node->gd == 0)
		{
			node = NULL;
		}
		else
		{
			node->gd = 0;
		}
	}
	return node;
}

s32 mcu_i2c_close(DevI2cNode *node)
{
	if(node == NULL)
		return -1;

	if(node->gd != 0)
		return -1;

	node->gd = -1; 

	return 0;
}

/*
	摄像头使用SCCB接口，其实就是I2C
	从ST官方摄像头移植，使用的是硬件I2C
*/
#define CAREMA_SCCB_I2C 	I2C2 //使用I2C2控制器
#define SCCB_TIMEOUT_MAX    10000
	
void SCCB_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef  I2C_InitStruct;	  
	/****** Configures the I2C1 used for OV9655 camera module configuration *****/
	/* I2C1 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	
	/* GPIOB clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE); 
	
	/* Connect I2C2 pins to AF4 */
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource0, GPIO_AF_I2C2);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource1, GPIO_AF_I2C2);
	
	/* Configure I2C2 GPIOs */	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	/* Configure I2C2 */
	/* I2C DeInit */
	I2C_DeInit(CAREMA_SCCB_I2C);
	  
	///* Enable the I2C peripheral */
	//I2C_Cmd(CAREMA_SCCB_I2C, ENABLE);
	
	/* Set the I2C structure parameters */
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_OwnAddress1 = 0xFE;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_ClockSpeed = 10000;
	
	/* Initialize the I2C peripheral w/ selected parameters */
	I2C_Init(CAREMA_SCCB_I2C, &I2C_InitStruct);
	
	I2C_Cmd(CAREMA_SCCB_I2C, ENABLE);
	
	I2C_AcknowledgeConfig(CAREMA_SCCB_I2C, ENABLE);

}
	
uint8_t bus_sccb_writereg(uint8_t DeviceAddr, uint16_t Addr, uint8_t Data)
{
	
	uint32_t timeout = SCCB_TIMEOUT_MAX;
	
	/* Generate the Start Condition */
	I2C_GenerateSTART(CAREMA_SCCB_I2C, ENABLE);
	
	/* Test on I2C1 EV5 and clear it */
	timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
	while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
	  /* If the timeout delay is exceeded, exit with error code */
	  if ((timeout--) == 0) return 0xFF;
	}
	 
	/* Send DCMI selected device slave Address for write */
	I2C_Send7bitAddress(CAREMA_SCCB_I2C, DeviceAddr, I2C_Direction_Transmitter);
	
	/* Test on I2C1 EV6 and clear it */
	timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
	while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	  /* If the timeout delay is exceeded, exit with error code */
	  if ((timeout--) == 0) return 0xFF;
	}
	
	/* Send I2C1 location address LSB */
	I2C_SendData(CAREMA_SCCB_I2C, (uint8_t)(Addr));
	
	/* Test on I2C1 EV8 and clear it */
	timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
	while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	  /* If the timeout delay is exceeded, exit with error code */
	  if ((timeout--) == 0) return 0xFF;
	}
	
	/* Send Data */
	I2C_SendData(CAREMA_SCCB_I2C, Data);
	
	/* Test on I2C1 EV8 and clear it */
	timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
	while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	  /* If the timeout delay is exceeded, exit with error code */
	  if ((timeout--) == 0) return 0xFF;
	}  
	
	/* Send I2C1 STOP Condition */
	I2C_GenerateSTOP(CAREMA_SCCB_I2C, ENABLE);
	
	/* If operation is OK, return 0 */
	return 0;

}
	
uint8_t bus_sccb_readreg(uint8_t DeviceAddr, uint16_t Addr)
{
  uint32_t timeout = SCCB_TIMEOUT_MAX;
  uint8_t Data = 0;

  /* Generate the Start Condition */
  I2C_GenerateSTART(CAREMA_SCCB_I2C, ENABLE);

  /* Test on I2C1 EV5 and clear it */
  timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
  while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
	/* If the timeout delay is exceeded, exit with error code */
	if ((timeout--) == 0) return 0xFF;
  }
  
  /* Send DCMI selected device slave Address for write */
  I2C_Send7bitAddress(CAREMA_SCCB_I2C, DeviceAddr, I2C_Direction_Transmitter);
 
  /* Test on I2C1 EV6 and clear it */
  timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
  while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
	/* If the timeout delay is exceeded, exit with error code */
	if ((timeout--) == 0) return 0xFF;
  }

  /* Send I2C1 location address LSB */
  I2C_SendData(CAREMA_SCCB_I2C, (uint8_t)(Addr));

  /* Test on I2C1 EV8 and clear it */
  timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
  while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
  {
	/* If the timeout delay is exceeded, exit with error code */
	if ((timeout--) == 0) return 0xFF;
  } 
  
  /* Clear AF flag if arised */
  CAREMA_SCCB_I2C->SR1 |= (uint16_t)0x0400;

  /* Generate the Start Condition */
  I2C_GenerateSTART(CAREMA_SCCB_I2C, ENABLE);
  
  /* Test on I2C1 EV6 and clear it */
  timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
  while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
	/* If the timeout delay is exceeded, exit with error code */
	if ((timeout--) == 0) return 0xFF;
  } 
  
  /* Send DCMI selected device slave Address for write */
  I2C_Send7bitAddress(CAREMA_SCCB_I2C, DeviceAddr, I2C_Direction_Receiver);

  /* Test on I2C1 EV6 and clear it */
  timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
  while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
  {
	/* If the timeout delay is exceeded, exit with error code */
	if ((timeout--) == 0) return 0xFF;
  }
 
  /* Prepare an NACK for the next data received */
  I2C_AcknowledgeConfig(CAREMA_SCCB_I2C, DISABLE);

  /* Test on I2C1 EV7 and clear it */
  timeout = SCCB_TIMEOUT_MAX; /* Initialize timeout value */
  while(!I2C_CheckEvent(CAREMA_SCCB_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
  {
	/* If the timeout delay is exceeded, exit with error code */
	if ((timeout--) == 0) return 0xFF;
  }

  /* Prepare Stop after receiving data */
  I2C_GenerateSTOP(CAREMA_SCCB_I2C, ENABLE);

  /* Receive the Data */
  Data = I2C_ReceiveData(CAREMA_SCCB_I2C);

  /* return the read data */
  return Data;
}
	

