![DNA e.V.](https://screenshot.tbspace.de/kzmdiwjhayn.png) &nbsp; SDS011 LoRaWAN sensor
=============================

![Photo of SDS011 Heltec setup](https://screenshot.tbspace.de/scnqtfadejv.jpg)


### Overview

FeinstaubLoRa uses an ESP32-based Heltec Lora 32 module and a SDS011 particle sensor to measure the amount of particles in air and reports these measurements to TheThingsNetwork.org. 
It uses MCCI / LMIC for LoRaWAN communication and u8g2 to control the OLED screen.

The source code can be compiled using PlatformIO.  
The code does not support proper deep sleep and requires a constant power supply.

### Pinout 

| SDS011 | ESP32   |
| ------------- |-------------|
| 5V | 5V / Vin |
| GND | GND |
| TX | GPIO 13 |
| RX | GPIO 21 |

Make sure to use the TX/RX labels on the back of the SDS011.  
The SDS011 takes a lot of power to start the fan motor.  
It is beneficial to add a 1000µF (or similar) capacitor between 5V and GND directly at the sensor.  
You can also desolder or short the 1N4148 diode on the ESP32 board.   

### TTN setup
![TTN Screenshot](https://screenshot.tbspace.de/clejynuhrmo.png)

Configure a new application and register a new device.  
Set the device to OTAA mode, with "Frame Counter Checks" disabled (settings-page on the device-menu).   
Frame Counter Checks are _not_ supported, because the ESP32 can't store the last frame counter value permanently (without constant flash writes or external battery-backed SRAM/RTC).

Device EUI, Application EUI and App Key need to be copied into config.h. (copy config.sample.h and rename it config.h first). 
Make sure to copy the right endianess (MSB/LSB), this can be switched in the TTN web interface. 

The [TTN Decoder](ttn_decoder.js) needs to be copied into the "Payload Formats" tab page (in the application-menu).

### Additional info

[TTN Decoder](ttn_decoder.js)  
[Video of a measurement cycle](https://www.youtube.com/watch?v=NoSQvhXs800)  
