#include "oled.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 15, /* data=*/ 4);
uint8_t repaint_display;

void oled_init()
{
    u8g2.begin();
    u8g2.enableUTF8Print();

    repaint_display = 1;
}

char displaybuffer[250];
void oled_draw_frame()
{
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.setFontDirection(0);
    u8g2.clearBuffer();

    u8g2.setCursor(0, (1 * 7));
    u8g2.print("FeinstaubLoRa - dna-ev.de");

    u8g2.drawHLine(0, 8, 128);

    if (time_spent_in_state && current_state == STATE_WAITING_FOR_MEASUREMENTS)
    {
        u8g2.setCursor(0, (2 * 7) + 3);
        snprintf(displaybuffer, 250, "PM2.5  %d µg/m³", average_measurement.pm25 / time_spent_in_state);
        u8g2.print(displaybuffer);
        u8g2.setCursor(0, (3 * 7) + 3);
        snprintf(displaybuffer, 250, "PM10   %d µg/m³", average_measurement.pm10 / time_spent_in_state);
        u8g2.print(displaybuffer);
    }
    else
    {
        u8g2.setCursor(0, (2 * 7) + 3);
        snprintf(displaybuffer, 250, "PM2.5  - µg/m³");
        u8g2.print(displaybuffer);
        u8g2.setCursor(0, (3 * 7) + 3);
        snprintf(displaybuffer, 250, "PM10   - µg/m³");
        u8g2.print(displaybuffer);
    }
   

    switch (current_state)
	{
		case STATE_SLEEP:
            u8g2.setCursor(0, (4 * 7) + 3);
            u8g2.print("Status: Warten...");
			break;
		case STATE_WAITING_FOR_STABILIZE:
			u8g2.setCursor(0, (4 * 7) + 3);
            snprintf(displaybuffer, 250, "Status: Aufwärmen %ds", (WAIT_TIME_STABILIZE_S - time_spent_in_state));
            u8g2.print(displaybuffer);
			break; 
		case STATE_WAITING_FOR_MEASUREMENTS: 
			u8g2.setCursor(0, (4 * 7) + 3);
            snprintf(displaybuffer, 250, "Status: Messung %ds", (MEASUREMENTS_PER_TX - time_spent_in_state));
            u8g2.print(displaybuffer);
			break;
		default:
			break;
	}

    u8g2.setCursor(0, (5 * 7) + 3);
    u8g2.print("LoRaWAN: ");
    u8g2.setCursor(45, (5 * 7) + 3);
    u8g2.print(lora_state_string);

    u8g2.sendBuffer();
}
