#include <Arduino.h>

uint8_t read_buffer[255];
uint8_t read_position = 0;

typedef struct
{
	uint16_t pm25; 
	uint16_t pm10;
} measurement;

measurement incoming_measurement; 


void setup()
{
	// Debug Serial
	Serial.begin(9600);

	// UART for SDS011 - RX 13, TX 21 (connect SDS011 TX to 13, SDS011 RX to 21!) 
	Serial2.begin(9600, SERIAL_8N1, 13, 21);
}


void measurement_received(measurement data)
{
	#ifdef DEBUG
		Serial.print("Measurement: PM2.5: ");
		Serial.print(data.pm25);
		Serial.print(" µg/m³ - PM10: "); 
		Serial.print(data.pm10);
		Serial.println(" µg/m³");
	#endif

}

uint8_t parse_packet(uint8_t *packet_buffer)
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

		measurement_received(incoming_measurement);

		return 1;
	}
	return 0;
}

void loop()
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
			if (parse_packet(read_buffer + read_position - 10))
			{
				read_position = 0;
			}
		}

		// Limit read_position to 250
		read_position %= 250; 
	}

}