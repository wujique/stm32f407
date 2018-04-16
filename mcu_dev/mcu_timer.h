#ifndef __MCU_TIMER_H__
#define __MCU_TIMER_H__

extern s32 mcu_timer_init(void);
extern void mcu_tim4_pwm_init(u32 arr,u32 psc);
extern void mcu_timer_cap_init(u32 arr,u16 psc);
extern u32 mcu_timer_get_cap(void);

extern void mcu_tim3_init(void);
extern s32 mcu_tim3_start(void);
extern s32 mcu_tim3_stop(void);

extern void mcu_timer7_init(void);
extern s32 mcu_tim7_start(u32 Delay_10us);


#endif
