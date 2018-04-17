#ifndef __DEV_RS485_H__
#define __DEV_RS485_H__


extern s32 dev_rs485_init(void);
extern s32 dev_rs485_open(void);
extern s32 dev_rs485_close(void);
extern s32 dev_rs485_read(u8 *buf, s32 len);
extern s32 dev_rs485_write(u8 *buf, s32 len);
extern s32 dev_rs485_ioctl(void);
extern s32 dev_rs485_test(u8 mode);


#endif
