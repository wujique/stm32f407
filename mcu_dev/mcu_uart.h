#ifndef __DEV_UART_H__
#define __DEV_UART_H__

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>   

typedef enum {
  MCU_UART_1 =0,
  MCU_UART_2,
  MCU_UART_3,
  MCU_UART_MAX,
}
McuUartNum; 


#define PC_PORT  MCU_UART_3 

extern s32 mcu_uart_init(void);
extern s32 mcu_uart_open (McuUartNum comport);
extern s32 mcu_uart_close (McuUartNum comport);
extern s32 mcu_uart_write (McuUartNum comport, u8 *buf, s32 len);
extern s32 mcu_uart_read (McuUartNum comport, u8 *buf, s32 len);
extern s32 mcu_uart_set_baud (McuUartNum comport, s32 baud);
extern s32 mcu_uart_tcflush (McuUartNum comport);
extern void mcu_uart_test(void);

#endif

