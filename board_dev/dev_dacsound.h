#ifndef _DEV_DACSOUND_H_
#define _DEV_DACSOUND_H_

extern s32 dev_dacsound_init(void);
extern s32 dev_dacsound_open(void);
extern s32 dev_dacsound_dataformat(u32 Freq, u8 Standard, u8 Format);
extern s32 dev_dacsound_setbuf(u16 *buffer0,u16 *buffer1,u32 len);
extern s32 dev_dacsound_transfer(u8 sta);
extern s32 dev_dacsound_close(void);


#endif
