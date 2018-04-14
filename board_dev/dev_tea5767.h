#ifndef __DEV_TEA5767_H__
#define __DEV_TEA5767_H__


extern s32 dev_tea5767_test(void);
extern s32 dev_tea5767_init(void);
extern s32 dev_tea5767_open(void);
extern s32 dev_tea5767_close(void);
extern void dev_tea5767_setfre(unsigned long fre);
extern s32 dev_tea5767_search(u8 mode);

#endif
