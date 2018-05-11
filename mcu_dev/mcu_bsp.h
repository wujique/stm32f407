#ifndef _MCU_BSP_H_
#define _MCU_BSP_H_

typedef enum{
	MCU_PORT_A = 0x00,
	MCU_PORT_B,
	MCU_PORT_C,
	MCU_PORT_D,
	MCU_PORT_E,
	MCU_PORT_F,
	MCU_PORT_G,
	MCU_PORT_H,
	MCU_PORT_I,
	MCU_PORT_J,
	MCU_PORT_K,
}MCU_PORT;

extern void mcu_io_config_in(MCU_PORT port, u16 pin);
extern void mcu_io_config_out(MCU_PORT port, u16 pin);
extern void mcu_io_output_setbit(MCU_PORT port, u16 pin);
extern void mcu_io_output_resetbit(MCU_PORT port, u16 pin);
extern u8 mcu_io_input_readbit(MCU_PORT port, u16 pin);

#endif

