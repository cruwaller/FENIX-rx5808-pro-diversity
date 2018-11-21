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

#define VERSION_NUMBER 0001

// === Features ===============================================================

// You can use any of the arduino analog pins to measure the voltage of the
// battery. See additional configuration below.

//#define USE_VOLTAGE_MONITORING

// Can enable this to powerdown the audio blocks on the RX58xx if you don't
// need it. Save a tiny bit of power, make your videos less noisy.

// WARNING: Makes RSSI act a little weird.
//#define DISABLE_AUDIO

// === Pins ====================================================================


#define PIN_SPI_SLAVE_SELECT_RX_A       22
#define PIN_SPI_SLAVE_SELECT_RX_B       35

#define PIN_RX_SWICTH                   32

#define PIN_RSSI_A                      36
#define PIN_RSSI_B                      39
#define PIN_RSSI_C                      39
#define PIN_RSSI_D                      39

#define PIN_TOUCHPAD_SLAVE_SELECT       21
#define PIN_TOUCHPAD_DATA_READY         34

//#define PIN_VBAT PA6 // not tested

// === Voltage Monitoring ======================================================

// Voltage levels
#define WARNING_VOLTAGE 72 // 3.6V per cell for 2S
#define CRITICAL_VOLTAGE 66 // 3.3V per cell for 2S
#define VBAT_SCALE 110
#define VBAT_OFFSET 0

// Alarm sounds
#define ALARM_EVERY_SEC 20000
#define CRITICAL_BEEP_EVERY_MSEC 400
#define CRITICAL_BEEPS 3
#define WARNING_BEEP_EVERY_MSEC 200
#define WARNING_BEEPS 2

// === RSSI ====================================================================

// RSSI default raw range.
#define RSSI_MIN_VAL 500
#define RSSI_MAX_VAL 2000

// 75% threshold, when channel is printed in spectrum.
#define RSSI_SEEK_FOUND 75

// 80% under max value for RSSI.
//#define RSSI_SEEK_TRESHOLD 50

// Scan loops for setup run.
#define RSSI_SETUP_RUN 3

// === Misc ====================================================================

#define EEPROM_SAVE_TIME 1000

#define BEEPER_CHIRP 25

#define SPEED_TEST

#endif // file_defined
