/**
 * @file            dev_wm8978.c
 * @brief           wm8978驱动
 * @author          wujique
 * @date            2017年11月16日 星期四
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2017年11月16日 星期四
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

#include "mcu_i2c.h"
#include "mcu_i2s.h"
#include "dev_wm8978.h"  

/*
	wm8978通过I2C接口控制，但是只能写，不能读
	利用内存记录写到WM8978寄存器的值

*/
#define DEV_WM8978_I2CBUS DEV_VI2C_1
#define WM8978_SLAVE_ADDRESS    0x1A

static u16 WM8978RegVaule[] = {
	0X0000,0X0000,0X0000,0X0000, 0X0050,0X0000,0X0140,0X0000,
	0X0000,0X0000,0X0000,0X00FF, 0X00FF,0X0000,0X0100,0X00FF,
	0X00FF,0X0000,0X012C,0X002C, 0X002C,0X002C,0X002C,0X0000,
	0X0032,0X0000,0X0000,0X0000, 0X0000,0X0000,0X0000,0X0000,
	0X0038,0X000B,0X0032,0X0000, 0X0008,0X000C,0X0093,0X00E9,
	0X0000,0X0000,0X0000,0X0000, 0X0003,0X0010,0X0010,0X0100,
	0X0100,0X0002,0X0001,0X0001, 0X0039,0X0039,0X0039,0X0039,
	0X0001,0X0001
};

/**
 *@brief:      dev_wm8978_writereg
 *@details:    写WM8978寄存器
 *@param[in]   u8 reg     寄存器
               u16 vaule  数值，低九位为有效值
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_writereg(u8 reg, u16 vaule)
{
	s32 ret = -1;
	u8 data[2];

	data[0] = (reg<<1) | ((vaule>>8)&0x01);
	data[1] = vaule & 0xff;
	ret = mcu_i2c_transfer(DEV_WM8978_I2CBUS, WM8978_SLAVE_ADDRESS, MCU_I2C_MODE_W, data, 2);
	if(ret == 0)
	{
		WM8978RegVaule[reg] = vaule;
	}
	
	return ret;	
}
/**
 *@brief:      dev_wm8978_readreg
 *@details:    读回当前WM8978寄存器设定值
 *@param[in]   u8 addr    
               u16 *data  
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_readreg(u8 addr, u16 *data)
{
	*data = WM8978RegVaule[addr];
	return 0;	
}
/**
 *@brief:      dev_wm8978_set_dataformat
 *@details:    设置I2S格式
 *@param[in]   u8 Standard  
               u8 Format    
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_set_dataformat(u8 Standard, u8 Format)
{
	u16 RegValue = 0;
	RegValue  = Standard<<3;	
	RegValue |= Format<<5;
	dev_wm8978_writereg(4, RegValue);	// 设置IIS接口为 I2S Phillips 格式，数据长度16位	
	return 0;
}

/**
 *@brief:      dev_wm8978_set_phone_vol
 *@details:    设置耳机音量
 *@param[in]   u8 volume  0-63
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_set_phone_vol(u8 volume)
{
	volume = volume & 0x3F;	

	dev_wm8978_writereg(52, volume);
	dev_wm8978_writereg(53, volume | 0x100);

	return 0;
}

/**
 *@brief:      dev_wm8978_set_spk_vol
 *@details:    设置喇叭音量
 *@param[in]   u8 volume  0-63
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_set_spk_vol(u8 volume)
{
	volume = volume & 0x3F;	

	dev_wm8978_writereg(54, volume);
	dev_wm8978_writereg(55, volume | 0x100);
	return 0;
}

/**
 *@brief:      dev_wm8978_set_mic_gain
 *@details:    设置麦克风增益
 *@param[in]   u8 gain   0-63
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_set_mic_gain(u8 gain)
{
	gain = gain & 0x3F;

	dev_wm8978_writereg(45, gain);
	dev_wm8978_writereg(46, gain | (1 << 8));

	return 0;
}
/**
 *@brief:      dev_wm8978_set_line_gain
 *@details:    设置线性输入增益
 *@param[in]   u8 gain  0-7
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_set_line_gain(u8 gain)
{
	u16 RegValue;

	gain = gain & 0x07;

	dev_wm8978_readreg(47, &RegValue);
	RegValue &= 0x8F;						
	RegValue |= (gain << 4);			
	dev_wm8978_writereg(47, RegValue);	

	dev_wm8978_readreg(48, &RegValue);  
	RegValue &= 0x8F;					
	RegValue |= (gain << 4);           
	dev_wm8978_writereg(48, RegValue);	

	return 0;
}
/**
 *@brief:      dev_wm8978_set_aux_gain
 *@details:    设置AUX输入增益
 *@param[in]   u8 gain  
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_set_aux_gain(u8 gain)
{
	u16 RegValue;

	gain&=0X07;
	dev_wm8978_readreg(47, &RegValue);
	RegValue&=~(7<<0);			 
 	dev_wm8978_writereg(47,RegValue|gain<<0);

	dev_wm8978_readreg(48, &RegValue);
	RegValue&=~(7<<0);			
 	dev_wm8978_writereg(48,RegValue|gain<<0);
	return 0;
}
/**
 *@brief:      dev_wm8978_inout
 *@details:    配置WM8978输入输出通道
 *@param[in]   u16 In   
               u16 Out  
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_inout(u8 In, u8 Out)
{
	u16 RegValue;

	if ((In == WM8978_INPUT_NULL) && (Out == WM8978_OUTPUT_NULL))
	{
		return 0;
	}

	RegValue = (1 << 3) | (3 << 0);
	
	if (Out & WM8978_OUTPUT_LINE) 
	{
		RegValue |= ((1 << 7) | (1 << 6));
	}
	
	if ((In & WM8978_INPUT_LMIC) || (In & WM8978_INPUT_RMIC))
	{
		RegValue |= (1 << 4);
	}
	dev_wm8978_writereg(1, RegValue);

	RegValue = 0;
	if (Out & WM8978_OUTPUT_PHONE)	{
		RegValue |= (1 << 7);
		RegValue |= (1 << 8);
	}

	if (In & WM8978_INPUT_LMIC){
		RegValue |= ((1 << 4) | (1 << 2));
	}
	if (In & WM8978_INPUT_RMIC){
		RegValue |= ((1 << 5) | (1 << 3));
	}
	
	if (In & WM8978_INPUT_LINE){
		RegValue |= ((1 << 4) | (1 << 5));
	}
	if (In & WM8978_INPUT_RMIC){
		RegValue |= ((1 << 5) | (1 << 3));
	}
	if (In & WM8978_INPUT_ADC){
		RegValue |= ((1 << 1) | (1 << 0));
	}
	dev_wm8978_writereg(2, RegValue);

	RegValue = 0;
	if (Out & WM8978_OUTPUT_LINE){
		RegValue |= ((1 << 8) | (1 << 7));
	}
	if (Out & WM8978_OUTPUT_SPK){
		RegValue |= ((1 << 6) | (1 << 5));
	}
	if (Out != WM8978_OUTPUT_NULL){
		RegValue |= ((1 << 3) | (1 << 2));
	}
	
	if (In & WM8978_INPUT_DAC){
		RegValue |= ((1 << 1) | (1 << 0));
	}
	dev_wm8978_writereg(3, RegValue);

	RegValue = 0 << 8;
	if (In & WM8978_INPUT_LINE){
		RegValue |= ((1 << 6) | (1 << 2));
	}
	if (In & WM8978_INPUT_RMIC){
		RegValue |= ((1 << 5) | (1 << 4));
	}
	if (In & WM8978_INPUT_LMIC){
		RegValue |= ((1 << 1) | (1 << 0));
	}
	dev_wm8978_writereg(44, RegValue);	

	if (In & WM8978_INPUT_ADC){
		RegValue = (1 << 3) | (0 << 8) | (4 << 0);/* 禁止ADC高通滤波器, 设置截至频率 */
	}
	else{
		RegValue = 0;
	}
	dev_wm8978_writereg(14, RegValue);	

	if (In & WM8978_INPUT_ADC)
	{
		RegValue = (0 << 7);
		dev_wm8978_writereg(27, RegValue);	
		RegValue = 0;
		dev_wm8978_writereg(28, RegValue);	
		dev_wm8978_writereg(29, RegValue);	
		dev_wm8978_writereg(30, RegValue);	
	}

	RegValue = 0;		/* 禁止自动增益控制 */
	dev_wm8978_writereg(32, RegValue);
	dev_wm8978_writereg(33, RegValue);
	dev_wm8978_writereg(34, RegValue);


	RegValue = (3 << 1) | (7 << 0);		/* 禁止自动增益控制 */
	dev_wm8978_writereg(35, RegValue);

	RegValue = 0;
	if ((In & WM8978_INPUT_LMIC) || (In & WM8978_INPUT_RMIC))
	{
		RegValue |= (1 << 8);	/* MIC增益取+20dB */
	}
	if (In & WM8978_INPUT_AUX)
	{
		RegValue |= (3 << 0);	/* Aux增益固定取3，用户可以自行调整 */
	}
	if (In & WM8978_INPUT_LINE)
	{
		RegValue |= (3 << 4);	/* Line增益固定取3，用户可以自行调整 */
	}
	dev_wm8978_writereg(47, RegValue);	/* 写左声道输入增益控制寄存器 */
	dev_wm8978_writereg(48, RegValue);	/* 写右声道输入增益控制寄存器 */

	RegValue = 0xFF;
	dev_wm8978_writereg(15, RegValue);	/* 选择0dB，先缓存左声道 */
	RegValue = 0x1FF;
	dev_wm8978_writereg(16, RegValue);	/* 同步更新左右声道 */

	RegValue = 0;
	if (Out & WM8978_OUTPUT_SPK)
	{
		RegValue |= (1 << 4);	/* ROUT2 反相, 用于驱动扬声器 */
	}
	if (In & WM8978_INPUT_AUX)
	{
		RegValue |= ((7 << 1) | (1 << 0));
	}
	dev_wm8978_writereg(43, RegValue);

	RegValue = 0;
	if (In & WM8978_INPUT_DAC){
		RegValue |= ((1 << 6) | (1 << 5));
	}
	if (Out & WM8978_OUTPUT_SPK){
		RegValue |=  ((1 << 2) | (1 << 1));	/* SPK 1.5x增益,  热保护使能 */
	}
	if (Out & WM8978_OUTPUT_LINE){
		RegValue |=  ((1 << 4) | (1 << 3));	/* BOOT3  BOOT4  1.5x增益 */
	}
	dev_wm8978_writereg(49, RegValue);

	RegValue = 0;
	if (In & WM8978_INPUT_AUX){
		RegValue |= ((7 << 6) | (1 << 5));
	}
	if ((In & WM8978_INPUT_LINE) 
		|| (In & WM8978_INPUT_LMIC) 
		|| (In & WM8978_INPUT_RMIC)){
		RegValue |= ((7 << 2) | (1 << 1));
	}
	if (In & WM8978_INPUT_DAC){
		RegValue |= (1 << 0);
	}
	dev_wm8978_writereg(50, RegValue);
	dev_wm8978_writereg(51, RegValue);

	RegValue = 0;
	if (Out & WM8978_OUTPUT_LINE){
		RegValue |= (1 << 3);
	}
	dev_wm8978_writereg(56, RegValue);

	RegValue = 0;
	if (Out & WM8978_OUTPUT_LINE){
		RegValue |= ((1 << 4) |  (1 << 1));
	}
	dev_wm8978_writereg(57, RegValue);

	if (In & WM8978_INPUT_DAC){
		dev_wm8978_writereg(11, 255);
		dev_wm8978_writereg(12, 255 | 0x100);
	}
	else{
		dev_wm8978_writereg(11, 0);
		dev_wm8978_writereg(12, 0 | 0x100);
	}

	if (In & WM8978_INPUT_DAC){
		dev_wm8978_writereg(10, 0);
	}
	
	return 0;
}

/**
 *@brief:      dev_wm8978_init
 *@details:    初始化wm8978配置
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static s32 dev_wm8978_setting_init(void)
{
	s32 ret = -1;

	ret = dev_wm8978_writereg(0,0x00);	// 复位WM8978
	if(ret == -1)		// 复位失败
		return ret;
	
	dev_wm8978_writereg(1,0x1B);	
	
	dev_wm8978_writereg(2,0x1B0);
	dev_wm8978_writereg(3, 0x000C);	// 使能左右声道混合
	dev_wm8978_writereg(6, 0x0000);	// 由处理器提供时钟信号
	dev_wm8978_writereg(43, 0x0010);	// 设置ROUT2反相,驱动扬声器所必须
	dev_wm8978_writereg(49, 0x0006);	// 扬声器 1.5x 增益, 开启热保护

	dev_wm8978_inout(WM8978_INPUT_NULL,
						WM8978_OUTPUT_NULL);

	dev_wm8978_set_mic_gain(45);
	dev_wm8978_set_phone_vol(40);
	dev_wm8978_set_spk_vol(40);
	dev_wm8978_set_aux_gain(5);
	return ret;
}
/**
 *@brief:      dev_wm8978_init
 *@details:    初始化WM8978芯片
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_init(void)
{
	mcu_i2s_init();//初始化I2S接口
	dev_wm8978_setting_init();//配置WM8978初始化状态
	return 0;
}
/**
 *@brief:      dev_wm8978_open
 *@details:    打开WM8978，配置默认输入输出通道
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_open(void)
{
	dev_wm8978_inout(WM8978_INPUT_DAC|WM8978_INPUT_LMIC|WM8978_INPUT_RMIC|WM8978_INPUT_ADC,
					WM8978_OUTPUT_PHONE|WM8978_OUTPUT_SPK);
	return 0;
}
/**
 *@brief:      dev_wm8978_dataformat
 *@details:    配置WM8978跟I2S控制器的数据格式
 *@param[in]   u32 Freq     
               u8 Standard  
               u8 Format    
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_dataformat(u32 Freq, u8 Standard, u8 Format)
{
	u16 standard;
	u16 dataformat;
	
	dev_wm8978_set_dataformat(Standard, Format);

	if(Standard == WM8978_I2S_LSB)
	{
		standard = 	I2S_Standard_LSB;
	}
	else if(Standard == WM8978_I2S_MSB)
	{
		standard = 	I2S_Standard_MSB;
	}
	else if(Standard == WM8978_I2S_Phillips)
	{
		standard = 	I2S_Standard_Phillips;
	}
	else if(Standard == WM8978_I2S_PCM)
	{
		standard = 	I2S_Standard_PCMLong;
	}
	else
	{
		standard = 	I2S_Standard_Phillips;	
	}

	if(Format == WM8978_I2S_Data_16b)
	{
		dataformat = 	I2S_DataFormat_16b;
	}
	else if(Format == WM8978_I2S_Data_20b)
	{
		dataformat = 	I2S_DataFormat_16bextended;
	}
	else if(Format == WM8978_I2S_Data_24b)
	{
		dataformat = 	I2S_DataFormat_24b;
	}
	else if(Format == WM8978_I2S_Data_32b)
	{
		dataformat = 	I2S_DataFormat_32b;
	}
	
	mcu_i2s_config(Freq, standard, dataformat);
	mcu_i2sext_config(Freq, standard, dataformat);
	
	return 0;
}
/**
 *@brief:      dev_wm8978_transfer
 *@details:    启动、停止I2S数据传输
 *@param[in]   u8 onoff  
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_transfer(u8 onoff)
{
	if(onoff == 1)
	{
		mcu_i2s_dma_start();
	}
	else
	{
		mcu_i2s_dma_stop();	
	}
	return 0;
}

#if 0
/*   
	仅仅测试使用，后续改为动态申请 
*/
volatile u8 SoundBufIndex=0xff;//双缓冲索引，取值0和1，都填充后赋值0XFF

#define I2S_DMA_BUFF_SIZE (4096*4)
uint16_t I2sDmaBuf[2][I2S_DMA_BUFF_SIZE];

extern const u8 AUDIO_SAMPLE[291632];
/**
 *@brief:      fun_sound_set_free_buf
 *@details:    设置空闲缓冲索引
 		
 *@param[in]   u8 *index  
 *@param[out]  无
 *@retval:     
 */
s32 fun_sound_set_free_buf(u8 index)
{
	SoundBufIndex = index;
	return 0;
}
/**
 *@brief:      fun_sound_get_buff_index
 *@details:    查询当前需要填充的BUF
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
static s32 fun_sound_get_buff_index(void)
{
	s32 res;

	res = SoundBufIndex;
	SoundBufIndex = 0xff;
	return res;
}

/**
 *@brief:      dev_wm8978_test
 *@details:    WM8978测试程序，播放内嵌的WAV数据
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 dev_wm8978_test(void)
{
	u32 sample_index=0;
	u32 buf_index = 0;
	u32 i;
	
	wjq_log(LOG_FUN, "test wm8978\r\n");
	
	dev_wm8978_dataformat(I2S_AudioFreq_8k, WM8978_I2S_Phillips, WM8978_I2S_Data_16b);
	
	mcu_i2s_dma_init(&I2sDmaBuf[0][0], &I2sDmaBuf[1][0], I2S_DMA_BUFF_SIZE);

	sample_index = 0;
	for(i = 0; i<I2S_DMA_BUFF_SIZE; i++)
	{
		I2sDmaBuf[0][i]	= AUDIO_SAMPLE[sample_index++];
		I2sDmaBuf[0][i] += (AUDIO_SAMPLE[sample_index++]<<8);
	}
	
	for(i = 0; i<I2S_DMA_BUFF_SIZE; i++)
	{
		I2sDmaBuf[1][i]	= AUDIO_SAMPLE[sample_index++];
		I2sDmaBuf[1][i] += (AUDIO_SAMPLE[sample_index++]<<8);
	}
	
	dev_wm8978_transfer(1);
	
	while(1)
	{
		buf_index = fun_sound_get_buff_index();
		if(0xff != buf_index)
		{
			for(i = 0; i<I2S_DMA_BUFF_SIZE; i++)
			{
				I2sDmaBuf[buf_index][i]	= AUDIO_SAMPLE[sample_index++];
				I2sDmaBuf[buf_index][i] += (AUDIO_SAMPLE[sample_index++]<<8);
			}
			
			if((sample_index + I2S_DMA_BUFF_SIZE) > (291632))
			{
				wjq_log(LOG_FUN, "play finish\r\n");
				break;
			}
		}

	}
	dev_wm8978_transfer(0);
	return 0;

}
#endif


