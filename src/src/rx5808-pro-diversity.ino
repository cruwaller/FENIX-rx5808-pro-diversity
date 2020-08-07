/*
   SPI driver based on fs_skyrf_58g-main.c Written by Simon Chambers
   TVOUT by Myles Metzel
   Scanner by Johan Hermen
   Inital 2 Button version by Peter (pete1990)
   Refactored and GUI reworked by Marko Hoepken
   Universal version my Marko Hoepken
   Diversity Receiver Mode and GUI improvements by Shea Ivey
   OLED Version by Shea Ivey
   Seperating display concerns by Shea Ivey

  The MIT License (MIT)

  Copyright (c) 2015 Marko Hoepken

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), bto deal
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

#include <EEPROM.h>
#include "settings.h"
#include "settings_eeprom.h"
#include "state_home.h"
#include "channels.h"
#include "receiver.h"
#include "receiver_spi.h"
#include "state.h"
#include "ui.h"
#include "voltage.h"
#include "temperature.h"
#include "touchpad.h"
#include "receiver_spi.h"
#include <esp_now.h>
#include <WiFi.h>
#include "ExpressLRS_Protocol.h"
#include "WebUpdater.h"

#ifdef SPEED_TEST
    uint32_t speed_test_hz = 0;
    uint32_t speed_test_previousTime = 0;
#endif

/*
    esp-now setup for communicating to https://github.com/AlessandroAU/ExpressLRS
    broadcastAddress is the mac of your receiving esp8266
*/
uint8_t broadcastAddress[][ESP_NOW_ETH_ALEN] = {
    {0x50, 0x02, 0x91, 0xDA, 0x56, 0xCA},   // esp32 tx 50:02:91:DA:56:CA
    {0x50, 0x02, 0x91, 0xDA, 0x37, 0x84},   // r9 tx    50:02:91:DA:37:84
};

bool updatingOTA = false;
uint32_t previousLEDTime = 0;

void setup()
{

#ifdef SPEED_TEST
    Serial.begin(115200);
#endif

    EEPROM.begin(2048);
    SPI.begin();
    SPI.setHwCs(false);

    EepromSettings.setup();
    setupPins();
    StateMachine::setup();
    Ui::setup();
    TouchPad::setup();

    // Has to be last setup() otherwise channel may not be set.
    // RX possibly not booting quick enough if setup() is called earler.
    // delay() may be needed.
    Receiver::setup();

    if (!EepromSettings.isCalibrated) {
        StateMachine::switchState(StateMachine::State::SETTINGS_RSSI);
        Ui::tvOn();
    } else {
        StateMachine::switchState(StateMachine::State::HOME);
    }


    if (EepromSettings.otaUpdateRequested)
    {
        BeginWebUpdate();
        EepromSettings.otaUpdateRequested = false;
        EepromSettings.save();
        updatingOTA = true;
    } else
    /*
        esp-now setup for communicating to https://github.com/AlessandroAU/ExpressLRS
    */
    {
        WiFi.mode(WIFI_STA);

        if (esp_now_init() != ESP_OK) {
            // Serial.println("Error initializing ESP-NOW");
            return;
        }

        // Adds broadcastAddress
        esp_now_peer_info_t injectorInfo;
        injectorInfo.channel = 0;
        injectorInfo.encrypt = false;
        for (int i = 0; i < sizeof(broadcastAddress) / ESP_NOW_ETH_ALEN; i++) {
            memcpy(injectorInfo.peer_addr, broadcastAddress[i], ESP_NOW_ETH_ALEN);
            if (esp_now_add_peer(&injectorInfo) != ESP_OK) {
                // Serial.println("Failed to add peer");
                //return;
            }
        }
    }
}

void setupPins() {

    // Rx and Tx set as input so that they are high impedance when conencted to goggles.
    pinMode(1, INPUT);
    pinMode(3, INPUT);

    pinMode(PIN_SPI_SLAVE_SELECT_RX_A, OUTPUT);
    digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, HIGH);

    pinMode(PIN_SPI_SLAVE_SELECT_RX_B, OUTPUT);
    digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, HIGH);

    pinMode(PIN_TOUCHPAD_SLAVE_SELECT, OUTPUT);
    digitalWrite(PIN_TOUCHPAD_SLAVE_SELECT, HIGH);

    pinMode(PIN_RX_SWITCH, OUTPUT);
    digitalWrite(PIN_RX_SWITCH, LOW);

    pinMode(PIN_TOUCHPAD_DATA_READY, INPUT);

    pinMode(PIN_RSSI_A, INPUT);
    pinMode(PIN_RSSI_B, INPUT);
#ifdef PIN_RSSI_C
    pinMode(PIN_RSSI_C, INPUT);
#endif
#ifdef PIN_RSSI_D
    pinMode(PIN_RSSI_D, INPUT);
#endif
#ifdef PIN_VBAT
    analogSetPinAttenuation(PIN_VBAT, ADC_2_5db);
#endif
}

void loop() {
    uint32_t now = millis();
    if (updatingOTA)
    {
        HandleWebUpdate();
        if (100u <= (now - previousLEDTime))
        {
            digitalWrite(PIN_RX_SWITCH, !digitalRead(PIN_RX_SWITCH));
            previousLEDTime = now;
        }
    }
    else
    {
        Receiver::update();
        TouchPad::update();

        if (Ui::isTvOn) {

#ifdef USE_VOLTAGE_MONITORING
            Voltage::update();
#endif
            Ui::display.begin(0); // reset OSD to black
            StateMachine::update();
            Ui::update();
            Ui::display.end(); // draw OSD

            // Useless call, checks if !Ui::isTvOn ==> removed!
            // EepromSettings.update();

            if (Ui::UiTimeOut.hasTicked() &&
                StateMachine::currentState != StateMachine::State::SETTINGS_RSSI )
            {
                Ui::tvOff();
                EepromSettings.update();
            }
        }
        else if (TouchPad::touchData.buttonPrimary) // TV is off, check if touch has happened
        {
            TouchPad::touchData.buttonPrimary = false;
            Ui::tvOn();
        }

        TouchPad::clearTouchData();

#ifdef SPEED_TEST
            speed_test_hz++;
            uint32_t nowTime = millis();
            if (1000u <= (nowTime - speed_test_previousTime)) {
                Serial.print(speed_test_hz);
                Serial.println(" Hz");
                speed_test_previousTime = nowTime;
                speed_test_hz = 0;
            }
#endif // SPEED_TEST
    }
}

uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

void sendToExLRS(uint16_t function, uint16_t payloadSize, const uint8_t *payload)
{
    uint8_t nowDataOutput[9 + payloadSize] = {0};
    uint8_t crc = 0, iter, data;

    // MSP header
    nowDataOutput[0] = '$';
    nowDataOutput[1] = 'X';
    nowDataOutput[2] = '<';
    nowDataOutput[3] = '0';
    nowDataOutput[4] = function & 0xff;
    nowDataOutput[5] = (function >> 8) & 0xff;
    nowDataOutput[6] = payloadSize & 0xff;
    nowDataOutput[7] = (payloadSize >> 8) & 0xff;

    // Calc CRC, starts from [3]
    for(iter = 3; iter < 8; iter++)
        crc = crc8_dvb_s2(crc, nowDataOutput[iter]);

    // Add payload and calc CRC on the fly
    while (payloadSize--) {
        data = *payload++;
        nowDataOutput[iter++] = data;
        crc = crc8_dvb_s2(crc, data);
    }

    // Add CRC
    nowDataOutput[iter++] = crc;

#if 0
    for (int i = 0; i < sizeof(broadcastAddress) / 6; i++)
    {
        uint8_t tempBroadcastAddress[6];
        memcpy(tempBroadcastAddress, broadcastAddress + (6 * i), 6);
        esp_now_send(tempBroadcastAddress, (uint8_t *) &nowDataOutput, sizeof(nowDataOutput));
    }
#else
    // Send to all listed peers
    esp_now_send(NULL, (uint8_t *) &nowDataOutput, iter);
#endif
}
