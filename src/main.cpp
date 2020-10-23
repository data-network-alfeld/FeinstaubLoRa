/*
FeinstaubLoRa

Pinout: 
- SDS011 5V  to ESP32 5V / Vin
- SDS011 GND to ESP32 GND
- SDS011 TX  to ESP32 13
- SDS011 RX  to ESP32 21

Copyright (c) 2020 by Tobias Mädel <feinstaublora@tbspace.de>
Written for Data Network Alfeld e.V. - https://dna-ev.de 
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this 
list of conditions and the following disclaimer in the documentation and/or other 
materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/
#include "main.h"

void setup()
{
	// Debug Serial
	Serial.begin(9600);

	Serial.println("FeinstaubLoRa starting...");

    oled_init();
    oled_draw_frame();

    lora_init();

	delay(1000);

    sds011_init();
}

void loop()
{
	// MCCI LoRaWAN main loop
	os_runloop_once();

	sds011_read_loop();

	// state machine for sensor state
	switch (current_state)
	{
		case STATE_SLEEP:
			if (time_spent_in_state >= (MEASUREMENT_INTERVAL_S - (WAIT_TIME_STABILIZE_S + MEASUREMENTS_PER_TX)))
			{        
                u8g2.setPowerSave(0);

				// Wake SDS011 up (twice, to be safe) 
				sds011_send_command(0xB4, 0x06, 0x01, 0x01);
				delay(100);
				sds011_send_command(0xB4, 0x06, 0x01, 0x01);
				
				//Serial.println("Waking SDS011 up, waiting for sensor to stabilize.");
				time_spent_in_state = 0; 
				current_state = STATE_WAITING_FOR_STABILIZE;
			}
			break;
		case STATE_WAITING_FOR_STABILIZE:
			if (time_spent_in_state >= WAIT_TIME_STABILIZE_S)
			{
				//Serial.println("Measurements stable. Taking measurements.");
				time_spent_in_state = 0; 
				current_state = STATE_WAITING_FOR_MEASUREMENTS;
			}
			break; 
		case STATE_WAITING_FOR_MEASUREMENTS: 
			// Waiting for measurements...
            if (time_spent_in_state >= 60)
            {
                // We haven't received enough measurements in 60 seconds to send a packet and advance the state machine. 
                // This is probably a sensor communication issue or another error. 
                Serial.println("Haven't received enough packets in 60 seconds to transmit a message. Rebooting ESP!");
                delay(500);
                ESP.restart();
            }
			break;
		default:
			break;
	}

    if (repaint_display)
    {
        oled_draw_frame();
        repaint_display = 0;
    }
}