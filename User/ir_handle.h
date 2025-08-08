#ifndef __IR_HANDLE_H
#define __IR_HANDLE_H

#include "include.h"

#define IR_RECV_PIN P23

// 定义红外按键键值
enum
{
    IR_KEY_NONE = 0,

    /* 大摇控器的按键 */
    IR_KEY_ON = 0x62,
    IR_KEY_OFF = 0x22,

    IR_KEY_RED = 0xA2,             // 大摇控器的红色按键
    IR_KEY_FULL_BRIGHTNESS = 0xC2, // 全亮
    IR_KEY_HALF_BRIGHTNESS = 0xB0, // 半亮
    IR_KEY_BRIGHTNESS_ADD_OR_NUM_4 = 0xE0,  // 亮度加，也是小遥控器的数字4
    IR_KEY_BRIGHTNESS_SUB_OR_NUM_2 = 0x90,  // 亮度减，也是小遥控器的数字2
    // IR_KEY_M = 0x10,               // M

    IR_KEY_AUTO_OR_NUM_5 = 0x98, // 自动模式 ，也是小遥控器的数字5
    IR_KEY_3H_OR_NUM_3 = 0xA8,   // 3H，也是小遥控器的数字3
    IR_KEY_5H_OR_M1 = 0x68,   // 5H，也是小遥控器的M1
    IR_KEY_8H_OR_M3 = 0x18,   // 8H，也是小遥控器的M3

    /* 小遥控器的按键，有些键值是跟大摇控器一样的，都归类为大摇控器的按键 */
    IR_KEY_SET = 0xE2,   // SET 模式设置
    IR_KEY_NUM_1 = 0x02, // 数字1
    // IR_KEY_NUM_2 = 0x90, // 数字2
    // IR_KEY_NUM_3 = 0xA8, // 数字3
    // IR_KEY_NUM_4 = 0xE0, // 数字4
    // IR_KEY_NUM_5 = 0x98, // 数字5
    // IR_KEY_M1 = 0x68,
    IR_KEY_M2 = 0x30,
    // IR_KEY_M3 = 0x18,
};





void ir_handle(void);

// void refresh_led_mode_status(void);

#endif