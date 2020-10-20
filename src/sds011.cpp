#include "sds011.h"

uint8_t read_buffer[255];
uint8_t read_position;

measurement incoming_measurement; 
measurement average_measurement; 

uint8_t current_state;
uint16_t time_spent_in_state; 
uint8_t current_measurement_count;

void sds011_send_command(uint8_t command_id, uint8_t data1, uint8_t data2, uint8_t data3)
{
	uint8_t checksum = 0; 

	checksum = data1 + data2 + data3 + 0xFF + 0xFF;

	Serial2.write(0xAA);
	Serial2.write(command_id);
	Serial2.write(data1);
	Serial2.write(data2);
	Serial2.write(data3);
	for (uint8_t i = 0; i < 10; i++)
	{
		Serial2.write(0x00);
	}
	Serial2.write(0xFF);
	Serial2.write(0xFF);
	Serial2.write(checksum);
	Serial2.write(0xAB);
}

void sds011_init()
{
    // UART for SDS011 - RX 13, TX 21 (connect SDS011 TX to 13, SDS011 RX to 21!) 
	Serial2.begin(9600, SERIAL_8N1, 13, 21);

    // Wake SDS011 up (if the MCU was reset while in sleep mode) 
	sds011_send_command(0xB4, 0x06, 0x01, 0x01);

    current_measurement_count = 0;
    time_spent_in_state = 0;
    current_state = STATE_WAITING_FOR_STABILIZE;
    read_position = 0;
}

void sds011_measurement_received(measurement data)
{
	#ifdef DEBUG
		Serial.print("Measurement: PM2.5: ");
		Serial.print(data.pm25);
		Serial.print(" µg/m³ - PM10: "); 
		Serial.print(data.pm10);
		Serial.println(" µg/m³");
	#endif

	average_measurement.pm25 += incoming_measurement.pm25;
	average_measurement.pm10 += incoming_measurement.pm10; 		

	if (current_measurement_count == MEASUREMENTS_PER_TX)
	{
		// Send SDS011 to sleep
		sds011_send_command(0xB4, 0x06, 0x01, 0x00);	

		// Calculate the average particle count
		average_measurement.pm25 /= MEASUREMENTS_PER_TX;
		average_measurement.pm10 /= MEASUREMENTS_PER_TX;

		// Transmit packet
		/*Serial.println("TX!");
		Serial.print("Measurement: PM2.5: ");
		Serial.print(average_measurement.pm25);
		Serial.print(" µg/m³ - PM10: "); 
		Serial.print(average_measurement.pm10);
		Serial.println(" µg/m³");*/

        payload[1] = average_measurement.pm25 & 0xFF;
        payload[2] = (average_measurement.pm25 >> 8) & 0xFF;
        payload[3] = average_measurement.pm10 & 0xFF;
        payload[4] = (average_measurement.pm10 >> 8) & 0xFF;

        payloadSent = 0;
        do_send(&sendjob);

		// Reset state and go back to sleep
		current_measurement_count = 0;
		average_measurement.pm25 = 0;
		average_measurement.pm10 = 0; 
		time_spent_in_state = 0; 
		current_state = STATE_SLEEP;
	}
	
	current_measurement_count++; 
}

uint8_t sds011_parse_packet(uint8_t *packet_buffer)
{
	// Check for message header, command number and message tail
	if (packet_buffer[0] == 0xAA && packet_buffer[1] == 0xC0 && packet_buffer[9] == 0xAB)
	{
		// Calculate checksum
		uint8_t checksum = 0x00; 
		for (uint8_t i = 2; i <= 7; i++)
		{
			checksum += packet_buffer[i];
		}

		// Checksum invalid? 
		if (packet_buffer[8] != checksum)
		{
			return 0; 
		}

		// If we're not actively measuring, drop packet.
		if (current_state != STATE_WAITING_FOR_MEASUREMENTS)
		{
			return 1; 
		}

		#ifdef DEBUG
			Serial.print("Received a valid packet: ");
			for (uint8_t i = 0; i < 10; i++)
			{
				char strbuf[255];
				snprintf(strbuf, 250, "%02x", packet_buffer[i]);
				Serial.print(strbuf);
			}
			Serial.println();
		#endif

		incoming_measurement.pm25 = packet_buffer[2];
		incoming_measurement.pm25 |= (packet_buffer[3] << 8);

		incoming_measurement.pm10 = packet_buffer[4];
		incoming_measurement.pm10 |= (packet_buffer[5] << 8);

		sds011_measurement_received(incoming_measurement);

		return 1;
	}
	return 0;
}

void sds011_read_loop()
{
    if (Serial2.available())
	{
		uint8_t incoming_byte = Serial2.read();

		// Wait for the start of a new packet (0xAA) or just save all incoming bytes if we've already seen the start.
		if (read_position != 0 || incoming_byte == 0xAA)
		{
			read_buffer[read_position++] = incoming_byte;
		}

		// If we're reading 0xAB and have at least 10 bytes in our buffer, this might be a valid packet
		if (incoming_byte == 0xAB && read_position >= 10)
		{		
			// Set the read_position counter back to 0, if we've received a valid packet. 
			// Otherwise keep listening for a bit, we might have started listening mid-packet. 
			if (sds011_parse_packet(read_buffer + read_position - 10))
			{
				read_position = 0;
			}
		}

		// Limit read_position to 250
		read_position %= 250; 
	}
}