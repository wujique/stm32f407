/**
 * @file            soundplay.h
 * @brief           声音播放对外定义
 * @author          wujique
 * @date            2018年1月7日 星期日
 * @version         初稿
 * @par             版权所有 (C), 2013-2023
 * @par History:
 * 1.日    期:        2018年1月7日 星期日
 *   作    者:         wujique
 *   修改内容:   创建文件
*/
#ifndef _SOUNDPLAY_H_
#define _SOUNDPLAY_H_

typedef enum {
  SOUND_IDLE =0,
  SOUND_BUSY,
  SOUND_PLAY,
  SOUND_PAUSE
}
SOUND_State;

typedef enum{
	SOUND_DEV_NULL = 0,	
	SOUND_DEV_1CH,
	SOUND_DEV_2CH
}SOUND_DEV_TYPE;

#endif
