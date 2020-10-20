#pragma once
#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>

#include "config.h"
#include "oled.h"
#include "sds011.h"

extern const char *lora_state_string;
extern uint8_t payload[5];
extern uint8_t payloadSent;
extern osjob_t sendjob;
extern hw_timer_t * timer;

void do_send(osjob_t* j);
void lora_init();