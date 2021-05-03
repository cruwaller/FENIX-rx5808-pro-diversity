/*
 * Setings file by Shea Ivey

The MIT License (MIT)

Copyright (c) 2015 Shea Ivey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#define VERSION_NUMBER 0003


// === Features ===============================================================

// You can use any of the arduino analog pins to measure the voltage of the
// battery. See additional configuration below.

//#define USE_VOLTAGE_MONITORING

//#define USE_TEMPERATURE_MONITORING

//#define USE_PRINT_ON_TIME

// Can enable this to powerdown the audio blocks on the RX58xx if you don't
// need it. Save a tiny bit of power, make your videos less noisy.

// WARNING: Makes RSSI act a little weird.
#define DISABLE_AUDIO

/* How many channel receiver has */
#ifndef CHANNELS_SIZE
#define CHANNELS_SIZE 48
#endif

/* Video output mode: PAL or NTSC */
#ifndef VIDEO_MODE
#define VIDEO_MODE NTSC
#endif

// === Pins ====================================================================

#define PIN_SPI_SLAVE_SELECT_RX_A       32
#define PIN_SPI_SLAVE_SELECT_RX_B       33

#define PIN_RX_SWITCH                   27

#define PIN_RSSI_A                      36
#define PIN_RSSI_B                      39

#define PIN_TOUCHPAD_SLAVE_SELECT       5
#define PIN_TOUCHPAD_DATA_READY         34

//#define PIN_VBAT                        35

// === Voltage Monitoring ======================================================

// Voltage levels
#define VBAT_SCALE 26 // 100k 10k resistor potential divider


// === RSSI ====================================================================

// RSSI default raw range.
#define RSSI_MIN_VAL 500
#define RSSI_MAX_VAL 2000

// Scan loops for setup run.
#define RSSI_SETUP_RUN 1

// === Misc ====================================================================

#define EEPROM_SAVE_TIME 1000

#define BEEPER_CHIRP 25

//#define SPEED_TEST

//#define DEBUG_ENABLED 1

#define POWER_OFF_RX 1

#define HOME_SHOW_LAPTIMES 1

// === Sanity checks and defaults ==============================================

#ifndef PIN_VBAT
#undef USE_VOLTAGE_MONITORING
#endif
#ifndef WIFI_AP_SSID
#define WIFI_AP_SSID "FENIX"
#endif
#ifndef WIFI_AP_PSK
#define WIFI_AP_PSK NULL
#endif
#ifndef WIFI_TIMEOUT
#define WIFI_TIMEOUT 60
#endif

#endif // file_defined
