#ifndef __DEV_UART_H__
#define __DEV_UART_H__

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>   
                  
#define PC_PORT  3 

extern s32 mcu_uart_open (s32 comport);
extern s32 mcu_uart_close (s32 comport);
extern s32 mcu_uart_write (s32 comport, u8 *buf, s32 len);
extern s32 mcu_uart_read (s32 comport, u8 *buf, s32 len);
extern s32 mcu_uart_set_baud (s32 comport, s32 baud, s32 databits, s32 parity, s32 stopbits, s32 flowctl);
extern s32 mcu_uart_tcflush (s32 comport);
extern void mcu_uart_test(void);

#endif

