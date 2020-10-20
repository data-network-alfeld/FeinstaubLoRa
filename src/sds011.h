#pragma once
#include <Arduino.h>
#include "lora.h"

typedef struct
{
	uint32_t pm25; 
	uint32_t pm10;
} measurement;

enum state
{
	STATE_SLEEP = 0,
	STATE_WAITING_FOR_STABILIZE = 1,
	STATE_WAITING_FOR_MEASUREMENTS = 2
};

extern measurement incoming_measurement; 
extern measurement average_measurement; 

extern uint8_t current_state;
extern uint16_t time_spent_in_state; 
extern uint8_t current_measurement_count;

uint8_t sds011_parse_packet(uint8_t *packet_buffer);
void sds011_measurement_received(measurement data);
void sds011_send_command(uint8_t command_id, uint8_t data1, uint8_t data2, uint8_t data3);
void sds011_init();
void sds011_read_loop();

