#ifndef __DEV_TOUCHKEY_H__
#define __DEV_TOUCHKEY_H__


#define DEV_TOUCHKEY_IDLE 		0X00
#define DEV_TOUCHKEY_TOUCH 		0X01
#define DEV_TOUCHKEY_RELEASE 	0X02

extern s32 dev_touchkey_task(void);
extern s32 dev_touchkey_test(void);
extern s32 dev_touchkey_open(void);
extern s32 dev_touchkey_init(void);
extern s32 dev_touchkey_close(void);
extern s32 dev_touchkey_read(u8 *buf, u32 count);


#endif
