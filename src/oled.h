#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "lora.h"
#include "sds011.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern uint8_t repaint_display;

void oled_init();
void oled_draw_frame();