#include "lora.h"

// Begin LoRaWAN
uint8_t payload[5];
uint8_t payloadSent;
osjob_t sendjob;
hw_timer_t * timer;
const char *lora_state_string;

#define PAYLOAD_PROTOCOL_VERSION 1 

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = { 34, 26, 35 },
};

#define CFG_sx1276_radio 1

// LSB-format! Last bytes should be 0xD5B370! 
static const u1_t PROGMEM cAPPEUI[8]=APPEUI;
void os_getArtEui (u1_t* buf) { memcpy_P(buf, cAPPEUI, 8);}

// LSB-format! 
static const u1_t PROGMEM cDEVEUI[8]=DEVEUI;
void os_getDevEui (u1_t* buf) { memcpy_P(buf, cDEVEUI, 8);}

// MSB-format!
static const u1_t PROGMEM cAPPKEY[16] = APPKEY;
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, cAPPKEY, 16);}


// End LoRaWAN 

// called every second to count up our clock
void second_count_timer() 
{
    repaint_display = 1;
    time_spent_in_state++; 
}

void lora_init()
{
    payloadSent = 1;
    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);

    // Send the packet version information
    payload[0] = PAYLOAD_PROTOCOL_VERSION;

    timer = timerBegin(0, 80, true); // timer_id = 0; divider=80; countUp = true;
    timerAttachInterrupt(timer, &second_count_timer, true); // edge = true
    timerAlarmWrite(timer, 1000000, true);  //1000 ms
    timerAlarmEnable(timer);
}

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void onEvent (ev_t ev) {
    //Serial.print(os_getTime());
    //Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            lora_state_string = "EV_SCAN_TIMEOUT";
            break;
        case EV_BEACON_FOUND:
            lora_state_string = "EV_BEACON_FOUND";
            break;
        case EV_BEACON_MISSED:
            lora_state_string = "EV_BEACON_MISSED";
            break;
        case EV_BEACON_TRACKED:
            lora_state_string = "EV_BEACON_TRACKED";
            break;
        case EV_JOINING:
            lora_state_string = "EV_JOINING";
            break;
        case EV_JOINED:
            lora_state_string = "EV_JOINED";
            {
             /* u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();*/
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	        // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        case EV_JOIN_FAILED:
            lora_state_string = "EV_JOIN_FAILED";
            break;
        case EV_REJOIN_FAILED:
            lora_state_string = "EV_REJOIN_FAILED";
            break;
        case EV_TXCOMPLETE:
            lora_state_string = "EV_TXCOMPLETE";
            /*if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }*/

            payloadSent = 1; 

            if (current_state == STATE_SLEEP)
            {
                u8g2.setPowerSave(1);
            }
            // Schedule next transmission
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            lora_state_string = "EV_LOST_TSYNC";
            break;
        case EV_RESET:
            lora_state_string = "EV_RESET";
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            lora_state_string = "EV_RXCOMPLETE";
            break;
        case EV_LINK_DEAD:
            lora_state_string = "EV_LINK_DEAD";
            break;
        case EV_LINK_ALIVE:
            lora_state_string = "EV_LINK_ALIVE";
            break;
        case EV_TXSTART:
            lora_state_string = "EV_TXSTART";
            break;
        case EV_TXCANCELED:
            lora_state_string = "EV_TXCANCELED";
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            lora_state_string = "EV: no JoinAccept";
            break;
        default:
            lora_state_string = "Unknown event ";
            //Serial.println((unsigned) ev);
            break;
    }
}

void do_send(osjob_t* j)
{
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, payload, 5, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}
