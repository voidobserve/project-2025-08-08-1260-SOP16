#ifndef __LIGHT_HANDLE_H
#define __LIGHT_HANDLE_H

#include "include.h"
#include "user_config.h"

void light_init(void);
void light_blink(u8 blink_cnt);
void light_handle(void);
// void light_set_pwm_duty(u16 pwm_duty_val);

#endif


