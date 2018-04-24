/**
 * @file            soundplay.c
 * @brief           声音播放功能
 * @author          wujique
 * @date            2018年1月6日 星期六
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年1月6日 星期六
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
/*

	本函数功能是，提供接口，播放指定路径下的声音文件，通过指定的设备输出

	备注：
		1 暂时只做16BIT WAV文件播放
		2 单声道通过数据填充做成双声道

*/
#include "stm32f4xx.h"
#include "ff.h"
#include "stm32f4xx_spi.h"
#include "dev_wm8978.h"
#include "mcu_i2s.h"
#include "soundplay.h"
#include "wujique_log.h"
#include "alloc.h"
#include "dev_dacsound.h"

#define FUN_SOUND_DEBUG

#ifdef FUN_SOUND_DEBUG
#define SOUND_DEBUG	wjq_log 
#else
#define SOUND_DEBUG(a, ...)
#endif



/*wav 文件结构*/
typedef struct _TWavHeader 
{          
	/*RIFF块*/
    int rId;    //标志符（RIFF）  0x46464952
    int rLen;   //数据大小,包括数据头的大小和音频文件的大小   (文件总大小-8)      
    int wId;    //格式类型（"WAVE"）   0x45564157
    
    /*Format Chunk*/
    int fId;    //"fmt " 带一个空格 0X20746D66     
    int fLen;   //Sizeof(WAVEFORMATEX)          
    short wFormatTag;       //编码格式，包括 1 WAVE_FORMAT_PCM，WAVEFORMAT_ADPCM等         
    short nChannels;        //声道数，单声道为1，双声道为2         
    int nSamplesPerSec;   //采样频率         
    int nAvgBytesPerSec;  //每秒的数据量         
    short nBlockAlign;      //块对齐          
    short wBitsPerSample;   //WAVE文件的采样大小         
    int dId;              //"data"     有可能是FACT块     
    int wSampleLength;    //音频数据的大小 
    /*紧跟后面可能有一个fact 块，跟压缩有关，如果没有，就是data块*/
}TWavHeader;


TWavHeader *wav_header;	
FIL SoundFile;//声音文件

/*
播放SD卡的音乐，只要2*4K缓冲
播放U盘中的音乐，却要2*8K
*/
#define I2S_DMA_BUFF_SIZE1   (1024*8)
#define DAC_SOUND_BUFF_SIZE2 (1024*1)

u32 SoundBufSize = I2S_DMA_BUFF_SIZE1;//采样频率小，就开小一点缓冲。

volatile u8 SoundBufIndex=0xff;//双缓冲索引，取值0和1，都填充后赋值0XFF
u16 *SoundBufP[2];

volatile SOUND_State SoundSta = SOUND_IDLE;
u32 playlen;
SOUND_DEV_TYPE SoundDevType = SOUND_DEV_NULL;



/*------------------------------------------*/

s32 fun_sound_stop(void);
/**
 *@brief:      fun_sound_set_free_buf
 *@details:    设置空闲缓冲索引
 			   这个函数提供给I2S或者DAC SOUND模块调用
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
 *@brief:      fun_sound_play
 *@details:    通过指定设备播放指定声音
 *@param[in]   char *name  
               char *dev   
 *@param[out]  无
 *@retval:     
 */
int fun_sound_play(char *name, char *dev)
{
	FRESULT res;
	unsigned int len;

	SoundSta = SOUND_BUSY;
	/*
		打开文件是否需要关闭？
		同时打开很多文件事发后会内存泄漏。
	*/
	res = f_open(&SoundFile, name, FA_READ);
	if(res != FR_OK)
	{
		SOUND_DEBUG(LOG_DEBUG, "sound open file err:%d\r\n", res);
		SoundSta = SOUND_IDLE;
		return -1;
	}

	SOUND_DEBUG(LOG_DEBUG, "sound open file ok\r\n");

	wav_header = (TWavHeader *)wjq_malloc(sizeof(TWavHeader));
	if(wav_header == 0)
	{
		SOUND_DEBUG(LOG_DEBUG, "sound malloc err!\r\n");
		SoundSta = SOUND_IDLE;
		return -1;
	}
	SOUND_DEBUG(LOG_DEBUG, "sound malloc ok\r\n");

	res = f_read(&SoundFile, (void *)wav_header, sizeof(TWavHeader), &len);
	if(res != FR_OK)
	{
		SOUND_DEBUG(LOG_DEBUG, "sound read err\r\n");
		SoundSta = SOUND_IDLE;
		return -1;
	}

	SOUND_DEBUG(LOG_DEBUG, "sound read ok\r\n");
	if(len != sizeof(TWavHeader))
	{
		SOUND_DEBUG(LOG_DEBUG, "read wav header err %d\r\n", len);
		SoundSta = SOUND_IDLE;
		return -1;
	}
	
	SOUND_DEBUG(LOG_DEBUG, "---%x\r\n", wav_header->rId);
	SOUND_DEBUG(LOG_DEBUG, "---%x\r\n", wav_header->rLen);
	SOUND_DEBUG(LOG_DEBUG, "---%x\r\n", wav_header->wId);//等于WAVE(0X45564157)，就说明是wave格式
	SOUND_DEBUG(LOG_DEBUG, "---%x\r\n", wav_header->fId);
	SOUND_DEBUG(LOG_DEBUG, "---%x\r\n", wav_header->fLen);
	SOUND_DEBUG(LOG_DEBUG, "---wave 格式 %x\r\n", wav_header->wFormatTag);
	SOUND_DEBUG(LOG_DEBUG, "---声道      %x\r\n", wav_header->nChannels);
	SOUND_DEBUG(LOG_DEBUG, "---采样频率  %d\r\n", wav_header->nSamplesPerSec);
	SOUND_DEBUG(LOG_DEBUG, "---每秒数据量 %d\r\n", wav_header->nAvgBytesPerSec);
	SOUND_DEBUG(LOG_DEBUG, "---样点字节数 %d\r\n", wav_header->nBlockAlign);
	SOUND_DEBUG(LOG_DEBUG, "---位宽 :    %d bit\r\n", wav_header->wBitsPerSample);
	SOUND_DEBUG(LOG_DEBUG, "---data =    %x\r\n", wav_header->dId);
	SOUND_DEBUG(LOG_DEBUG, "---数据长度: %x\r\n", wav_header->wSampleLength);

	if(wav_header->nSamplesPerSec <= I2S_AudioFreq_16k)
	{
		SoundBufSize = DAC_SOUND_BUFF_SIZE2;
	}
	else
	{
		SoundBufSize = I2S_DMA_BUFF_SIZE1;
	}
	/*
	
	*/
	SoundBufP[0] = (u16 *)wjq_malloc(SoundBufSize*2); 
	SoundBufP[1] = (u16 *)wjq_malloc(SoundBufSize*2); 
	
	SOUND_DEBUG(LOG_DEBUG, "%08x, %08x\r\n", SoundBufP[0], SoundBufP[1]);
	if(SoundBufP[0] == NULL)
	{

		SOUND_DEBUG(LOG_DEBUG, "sound malloc err\r\n");
		SoundSta = SOUND_IDLE;
		return -1;
	}

	if(SoundBufP[1] == NULL )
	{
		wjq_free(SoundBufP[0]);
		SoundSta = SOUND_IDLE;
		return -1;
	}
		

	/*根据文件内容设置采样频率跟样点格式*/
	u8 format;
	if(wav_header->wBitsPerSample == 16)
	{
		format =	WM8978_I2S_Data_16b; 	
	}
	else if(wav_header->wBitsPerSample == 24)
	{
		format =	WM8978_I2S_Data_24b; 	
	}
	else if(wav_header->wBitsPerSample == 32)
	{
		format =	WM8978_I2S_Data_32b; 	
	}

	/*打开指定设备*/
	if(0 == strcmp(dev, "wm8978"))
	{

	}

	playlen = 0;

	u32 rlen;

	/*音源单声道，设备双声道，对数据复制一份到另外一个声道*/
	if((wav_header->nChannels == 1) && (SoundDevType == SOUND_DEV_2CH))
	{
		rlen = SoundBufSize;
		f_read(&SoundFile, (void *)SoundBufP[0], rlen, &len);
		fun_sound_deal_1ch_data((u8*)SoundBufP[0]);
		f_read(&SoundFile, (void *)SoundBufP[1], rlen, &len);
		fun_sound_deal_1ch_data((u8*)SoundBufP[1]);

	}
	else
	{
		rlen = SoundBufSize*2;
		f_read(&SoundFile, (void *)SoundBufP[0], rlen, &len);
		f_read(&SoundFile, (void *)SoundBufP[1], rlen, &len);
	}
	
	playlen += rlen*2;

	if(0 == strcmp(dev, "wm8978"))
	{
		dev_wm8978_open();
		dev_wm8978_dataformat(wav_header->nSamplesPerSec, WM8978_I2S_Phillips, format);
		mcu_i2s_dma_init(SoundBufP[0], SoundBufP[1], SoundBufSize);
		SoundDevType = SOUND_DEV_2CH;
		dev_wm8978_transfer(1);//启动I2S传输
	}
	else if(0 == strcmp(dev, "dacsound"))
	{
		dev_dacsound_open();
		dev_dacsound_dataformat(wav_header->nSamplesPerSec, WM8978_I2S_Phillips, format);
		dev_dacsound_setbuf(SoundBufP[0], SoundBufP[1], SoundBufSize);
		SoundDevType = SOUND_DEV_1CH;
		dev_dacsound_transfer(1);
	}
	

	SoundSta = SOUND_PLAY;
	
	return 0;
}

/*

	单声道数据使用WM8978播放要经过处理，
	造成双声道

*/
static s32 fun_sound_deal_1ch_data(u8 *p)
{
	u8 ch1,ch2;
	u16 shift;
	u16 i;
	
	for(i=SoundBufSize;i>0;)
	{
		i--;
		//uart_printf("%d-",i);
		ch1 = *(p+i);
		i--;
		ch2 = *(p+i);
		
		shift = i*2;
		
		*(p+shift) = ch2;	
		*(p+shift+1) = ch1;
		*(p+shift+2) = ch2;	
		*(p+shift+3) = ch1;

	}
	
	return 0;
}

/**
 *@brief:      fun_sound_task
 *@details:    声音播放轮询任务，执行间隔不可以太久，-
               否则声音会有杂音，也就是断续
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void fun_sound_task(void)
{
	FRESULT res;
	unsigned int len;
	volatile s32 buf_index = 0;
	int rlen;
	u16 i;

	if(SoundSta == SOUND_BUSY
		|| SoundSta == SOUND_IDLE)
		return;
	
	buf_index = fun_sound_get_buff_index();
	if(0xff != buf_index)
	{
		if(SoundSta == SOUND_PAUSE)//暂停
		{
			for(i=0;i<SoundBufSize;i++)
			{
				*(SoundBufP[buf_index]+i)= 0x0000;
			}	
		}
		else
		{

			if((wav_header->nChannels == 1) && (SoundDevType == SOUND_DEV_2CH))
			{
				rlen = SoundBufSize;
				res = f_read(&SoundFile, (void *)SoundBufP[buf_index], rlen, &len);
				fun_sound_deal_1ch_data((u8*)SoundBufP[buf_index]);
			}
			else
			{
				rlen = SoundBufSize*2;
				res = f_read(&SoundFile, (void *)SoundBufP[buf_index], rlen, &len);
			}
			
			//memset(SoundBufP[buf_index], 0, SoundRecBufSize*2);
			
			playlen += len;


			/*
				u盘有BUG，有时候读到的数据长度不对
				稳健的做法是用已经播放的长度跟音源长度比较。
			*/
			if(len < rlen)
			{
				SOUND_DEBUG(LOG_DEBUG, "play finish %d, playlen:%x\r\n", len, playlen);
				fun_sound_stop();
				
			}	
		}
			
	}

}
/**
 *@brief:      fun_sound_get_sta
 *@details:    查询音乐播放状态
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
SOUND_State fun_sound_get_sta(void)
{
	return SoundSta;

}
/**
 *@brief:      fun_sound_stop
 *@details:    停止音乐播放
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 fun_sound_stop(void)
{
	u32 res;
	
	if(SoundSta == SOUND_PLAY
		|| SoundSta == SOUND_PAUSE)	
	{
		if(SoundDevType == SOUND_DEV_2CH)
		{
			dev_wm8978_transfer(0);
		}
		else if(SoundDevType == SOUND_DEV_1CH)
		{
			dev_dacsound_transfer(0);
			dev_dacsound_close();
		}
		
		wjq_free(SoundBufP[0]);
		wjq_free(SoundBufP[1]);
		res = f_close(&SoundFile);
		SOUND_DEBUG(LOG_DEBUG, "f_close:%d\r\n", res);
		wjq_free(wav_header);
		SoundSta = SOUND_IDLE;	
	}
	return 0;
}
/**
 *@brief:      fun_sound_pause
 *@details:    暂停播放
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 fun_sound_pause(void)
{
	if(SoundSta == SOUND_PLAY)
	{
		SoundSta = SOUND_PAUSE;
	}
	return 0;
}
/**
 *@brief:      fun_sound_resume
 *@details:    恢复播放
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 fun_sound_resume(void)
{
	if(SoundSta == SOUND_PAUSE)
	{
		SoundSta = SOUND_PLAY;
	}
	return 0;
}
/**
 *@brief:      fun_sound_setvol
 *@details:    设置音量
 *@param[in]   u8 vol  
 *@param[out]  无
 *@retval:     
 */
s32 fun_sound_setvol(u8 vol)
{
	return 0;
}

/**
 *@brief:      fun_sound_test
 *@details:    测试播放
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void fun_sound_test(void)
{
	SOUND_DEBUG(LOG_DEBUG, "play sound\r\n");
	fun_sound_play("1:/mono_16bit_8k.wav", "dacsound");		

}
/*
测试音源名称
mono_16bit_8k.wav
mono_16bit_44k.wav
stereo_16bit_32k.wav
十送红军.wav

*/

/*

	通过I2S利用WM8978录音
	跟播放一样，通过双缓冲DMA录音。
	需要注意的一点是，录音属于I2S_EXIT，不产生时钟，需要I2S才能产生通信时钟。
	也就是说要播音才能录音。
	所以录音时，也要配置I2S播放，我们只配置一个字节的DMA缓冲，以便I2S产生通信时钟，
	
*/
/*---录音跟播音缓冲要差不多，否则会互相卡顿----*/
u32 SoundRecBufSize;
u16 *SoundRecBufP[2];
static u8 SoundRecBufIndex=0xff;//双缓冲索引，取值0和1，都填充后赋值0XFF
TWavHeader *recwav_header;	
FIL SoundRecFile;//声音文件
u32 RecWavSize = 0;
u16 RecPlayTmp[8];

/*
	录音频率，如果考虑播音跟录音一起工作，
	需要综合考虑如何配置录音频率
*/
#define SOUND_REC_FRE 32000
/**
 *@brief:      fun_rec_set_free_buf
 *@details:    设置录音缓冲索引，在I2S exit中断中使用
 *@param[in]   u8 index  
 *@param[out]  无
 *@retval:     
 */
s32 fun_rec_set_free_buf(u8 index)
{
	SoundRecBufIndex = index;
	return 0;
}
/**
 *@brief:      fun_rec_get_buff_index
 *@details:    查询录音满的缓冲，满就要读走
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static s32 fun_rec_get_buff_index(void)
{
	s32 res;

	res = SoundRecBufIndex;
	SoundRecBufIndex = 0xff;
	return res;
}
/**
 *@brief:      fun_sound_rec
 *@details:    启动录音
 *@param[in]   char *name  
 *@param[out]  无
 *@retval:     
 */
s32 fun_sound_rec(char *name)
{
	FRESULT fres;
	u32 len;

	SOUND_DEBUG(LOG_DEBUG, "sound rec\r\n");
	RecWavSize = 0;
	SoundRecBufSize = SoundBufSize;

	/*  创建WAV文件 */
	fres=f_open(&SoundRecFile,(const TCHAR*)name, FA_CREATE_ALWAYS | FA_WRITE); 
	if(fres != FR_OK)			//文件创建失败
	{
		SOUND_DEBUG(LOG_DEBUG, "create rec file err!\r\n");
		return -1;
	}

	recwav_header = (TWavHeader *)wjq_malloc(sizeof(TWavHeader));
	if(recwav_header == NULL)
	{
		SOUND_DEBUG(LOG_DEBUG, "rec malloc err!\r\n");
		return -1;	
	}
	
	recwav_header->rId=0X46464952;
	recwav_header->rLen = 0;//录音结束后填
	recwav_header->wId = 0X45564157;//wave
	recwav_header->fId=0X20746D66;
	recwav_header->fLen = 16;
	recwav_header->wFormatTag = 0X01;
	recwav_header->nChannels = 2;
	recwav_header->nSamplesPerSec = SOUND_REC_FRE;//这个采样频率需要特殊处理，暂时不做。
	recwav_header->nAvgBytesPerSec = (recwav_header->nSamplesPerSec)*(recwav_header->nChannels)*(16/8);
	recwav_header->nBlockAlign = recwav_header->nChannels*(16/8);
	recwav_header->wBitsPerSample = 16;
	recwav_header->dId = 0X61746164;
	recwav_header->wSampleLength = 0;

	fres=f_write(&SoundRecFile,(const void*)recwav_header,sizeof(TWavHeader), &len);
	if((fres!= FR_OK)
		|| (len != sizeof(TWavHeader)))
	{

		SOUND_DEBUG(LOG_DEBUG, "rec write err!\r\n");
		wjq_free(recwav_header);
		return -1;		
	}
	else
	{
		SOUND_DEBUG(LOG_DEBUG, "create rec wav ok!\r\n");
	}

	/*  测试录音     */
	SoundRecBufP[0] = (u16 *)wjq_malloc(SoundRecBufSize*2); 
	SoundRecBufP[1] = (u16 *)wjq_malloc(SoundRecBufSize*2); 
	
	SOUND_DEBUG(LOG_DEBUG, "%08x, %08x\r\n", SoundRecBufP[0], SoundRecBufP[1]);
	if(SoundRecBufP[0] == NULL)
	{

		SOUND_DEBUG(LOG_DEBUG, "sound malloc err\r\n");
		return -1;
	}

	if(SoundRecBufP[1] == NULL )
	{
		wjq_free(SoundRecBufP[0]);
		return -1;
	}
	
	dev_wm8978_open();
	dev_wm8978_dataformat(SOUND_REC_FRE, WM8978_I2S_Phillips, WM8978_I2S_Data_16b);
	
	mcu_i2s_dma_init(RecPlayTmp, RecPlayTmp, 1);
	dev_wm8978_transfer(1);//启动I2S传输
	
	mcu_i2sext_dma_init(SoundRecBufP[0], SoundRecBufP[1], SoundRecBufSize);
	mcu_i2sext_dma_start();
	
	SOUND_DEBUG(LOG_DEBUG, "rec--------------------\r\n");
	
	return 0;
}
/**
 *@brief:      fun_rec_stop
 *@details:    停止录音
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 fun_rec_stop(void)
{
	u32 len;
	dev_wm8978_transfer(0);
	mcu_i2sext_dma_stop();
	
	recwav_header->rLen = RecWavSize+36;
	recwav_header->wSampleLength = RecWavSize;
	f_lseek(&SoundRecFile,0);
	
	f_write(&SoundRecFile,(const void*)recwav_header,sizeof(TWavHeader),&len);//写入头数据
	f_close(&SoundRecFile);

	wjq_free(SoundRecBufP[0]);
	wjq_free(SoundRecBufP[1]);
	wjq_free(recwav_header);
	return 0;
}
/**
 *@brief:      fun_rec_task
 *@details:    录音线程
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void fun_rec_task(void)
{
	int buf_index = 0;
	u32 len;
	FRESULT fres;
	
	buf_index = fun_rec_get_buff_index();
	if(0xff != buf_index)
	{
		//uart_printf("rec buf full:%d!\r\n", buf_index);
		RecWavSize += SoundRecBufSize*2;
	
		fres = f_write(&SoundRecFile,(const void*)SoundRecBufP[buf_index], 2*SoundRecBufSize, &len);
		if(fres != FR_OK)
		{
			SOUND_DEBUG(LOG_DEBUG, "write err\r\n");
		}
		
		if(len!= 2*SoundRecBufSize)
		{
			SOUND_DEBUG(LOG_DEBUG, "len err\r\n");
		}
		
	}
}

/**
 *@brief:      fun_rec_test
 *@details:    开始录音
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void fun_rec_test(void)
{
	fun_sound_rec("1:/rec9.wav");
}

void fun_play_rec_test(void)
{
	fun_sound_play("1:/rec9.wav", "wm8978");	
}

