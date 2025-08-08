#ifndef __ADC_H
#define __ADC_H

#include "include.h"
#include "user_config.h"

enum
{
    ADC_PIN_NONE = 0x00,
    ADC_PIN_DETECT_CHARGE,  // 检测充电分压后的电压（1/11分压） P12 AN7
    ADC_PIN_DETECT_BATTERY, // 检测电池分压后的电压（1/2分压）  P05 AN4
    ADC_PIN_DETECT_CURRENT, // 检测充电电流的引脚 P31(HCK) AN25
};
 

extern volatile u16 adc_val;

void adc_config(void);
void adc_sel_ref_voltage(u8 adc_ref_voltage);
void adc_sel_pin(u8 adc_pin);
u16 adc_getval(void);

void adc_update_bat_adc_val(void);
void adc_update_charge_adc_val(u8 adc_ref_voltage);
void adc_update_current_adc_val(void);

#endif
