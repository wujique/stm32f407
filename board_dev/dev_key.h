#ifndef __DEV_KEY_H__
#define __DEV_KEY_H__


extern s32 dev_key_init(void);
extern s32 dev_key_scan(void);
extern s32 dev_key_read(u8 *key, u8 len);
extern s32 dev_key_open(void);
extern s32 dev_key_close(void);

#define DEV_KEY_PR_MASK	(0X80)/*按键按下松开标志*/
#define DEV_KEY_PRESS	(0X01)/*键值，只有一个键，直接定义*/
#define DEV_KEY_REL		(DEV_KEY_PRESS|DEV_KEY_PR_MASK)
#endif
