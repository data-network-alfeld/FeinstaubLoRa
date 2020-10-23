#pragma once
/*
FeinstaubLoRa
Copyright (c) 2020 by Tobias Mädel <feinstaublora@tbspace.de>
Written for Data Network Alfeld e.V. - https://dna-ev.de 
All rights reserved.

--- Rename this file to config.h and fill out the parameters below: ---
*/

// LoRaWAN parameters

// LSB-format! 
#define DEVEUI { 0x65, 0x95, 0x80, 0x8D, 0x9E, 0x13, 0x0C, 0x00 }

// LSB-format! Last bytes should be 0xD5B370! 
#define APPEUI { 0xD9, 0x6A, 0x03, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 }

// MSB-format!
#define APPKEY { 0xAB, 0x07, 0x0F, 0x04, 0xBC, 0x0E, 0x75, 0x55, 0x8F, 0xC8, 0x19, 0xF7, 0x5C, 0x0C, 0xE7, 0x6B }

// Measurement parameters

// How many measurements of the SDS011 should be taken and averaged? 
#define MEASUREMENTS_PER_TX 10
// How many seconds to wait after turning the SDS011 to ensure the measurements are stable? 
#define WAIT_TIME_STABILIZE_S 30
// How low should the sensor sleep? 
#define MEASUREMENT_INTERVAL_S (5 * 60)
