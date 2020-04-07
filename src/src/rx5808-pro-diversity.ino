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
    uint32_t n = 0; 
    uint32_t previousTime = millis();
#endif

/* 
    esp-now setup for communicating to https://github.com/AlessandroAU/ExpressLRS
    broadcastAddress is the mac of your receiving esp8266
*/
uint8_t broadcastAddress[] = {0x50, 0x02, 0x91, 0xDA, 0x56, 0xCA}; // 50:02:91:DA:56:CA
bool updatingOTA = false;
uint32_t previousLEDTime = 0;

void setup()
{

    #ifdef SPEED_TEST
        Serial.begin(115200);
    #endif

    EEPROM.begin(2048);
    SPI.begin();
    
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

        esp_now_peer_info_t injectorInfo;
        memcpy(injectorInfo.peer_addr, broadcastAddress, 6);
        injectorInfo.channel = 0;  
        injectorInfo.encrypt = false;

        if (esp_now_add_peer(&injectorInfo) != ESP_OK){
            // Serial.println("Failed to add peer");
            return;
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
//    pinMode(PIN_RSSI_C, INPUT);
//    pinMode(PIN_RSSI_D, INPUT);

}

void loop() {

    if (updatingOTA)
    {
        HandleWebUpdate();
        if (millis() > previousLEDTime+100)
        {
            digitalWrite(PIN_RX_SWITCH, !digitalRead(PIN_RX_SWITCH));
            previousLEDTime = millis();
        }
    } else
    {
        Receiver::update();

        #ifdef USE_VOLTAGE_MONITORING  
            Voltage::update();
        #endif
    
        TouchPad::update();

        if (Ui::isTvOn) {
        
            Ui::display.begin(0);
            StateMachine::update();
            Ui::update();
            Ui::display.end();
    
            EepromSettings.update();
        }
        
        if (TouchPad::touchData.isActive) {
            Ui::UiTimeOut.reset();
        }
        
        if (Ui::isTvOn &&
            Ui::UiTimeOut.hasTicked() &&
            StateMachine::currentState != StateMachine::State::SETTINGS_RSSI ) 
        {
            Ui::tvOff();  
            EepromSettings.update();
        }
        
        if (!Ui::isTvOn &&
            TouchPad::touchData.buttonPrimary)
        {
            Ui::tvOn();
        }
    
        TouchPad::clearTouchData();  

        #ifdef SPEED_TEST
            n++;
            uint32_t nowTime = millis();
            if (nowTime > previousTime + 1000) {
                Serial.print(n);
                Serial.println(" Hz");
                previousTime = nowTime;
                n = 0;
            }
        #endif
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

// sendToExLRS(function, payloadSize, (uint8_t *) &payload);
void sendToExLRS(uint16_t function, uint16_t payloadSize, const uint8_t *payload)
{
    uint8_t nowDataOutput[9 + payloadSize] = {0};
    
    nowDataOutput[0] = '$';
    nowDataOutput[1] = 'X';
    nowDataOutput[2] = '<';
    nowDataOutput[3] = '0';
    nowDataOutput[4] = function & 0xff;
    nowDataOutput[5] = (function >> 8) & 0xff;
    nowDataOutput[6] = payloadSize & 0xff;
    nowDataOutput[7] = (payloadSize >> 8) & 0xff;

    for (int i = 0; i < payloadSize; i++)
    {
        nowDataOutput[8 + i] = payload[i];
    }

    uint8_t ck2 = 0;
    for(int i = 3; i < payloadSize+8; i++)
    {
        ck2=crc8_dvb_s2(ck2, nowDataOutput[i]);
    }
    nowDataOutput[payloadSize+8] = ck2;

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &nowDataOutput, sizeof(nowDataOutput));
    // esp_err_t esp_now_send(const uint8_t *peer_addr, const uint8_t *data, size_t len);
}

/*
uint16 Function - ['E', 'x'] (e.g. Dec = 17784, Hex = [0x45, 0x78], Bytes = [69, 120])

ExLRS function (uint8) and payload

    Mode - 0x00, uint8 mode number 0, 1, 2, 3, 4
    Tx power - 0x01, uint16 mW
    TLM rate - 0x02, uint8 0, 2, 4, 8, 16, 32, 64, 128

E.g. MSP V2 for setting ExLRS Tx power to 1W.

[$, X, <, flag, function, function, payload size, payload size, ExLRS function, Tx power, Tx power, CRC]
[$, X, <, 0x00, E, x, 0x03, 0x00, 0x01, 0xE8, 0x03, CRC]
*/