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
#include "comm_espnow.h"
#include "WebUpdater.h"
#include "task_prios.h"

#include <SPI.h>
#include <soc/rtc_wdt.h>

SemaphoreHandle_t mutex_spi = NULL;


#ifdef SPEED_TEST
uint32_t speed_test_hz = 0;
uint32_t speed_test_previousTime = 0;
#endif


static void setupPins()
{
#if !DEBUG_ENABLED && !defined(SPEED_TEST)
    // Rx and Tx set as input so that they are high impedance when conencted to goggles.
    pinMode(1, INPUT);
    pinMode(3, INPUT);
#endif

    // Init CS pins here to make sure those are in correct state before configuration is performed
    pinMode(PIN_TOUCHPAD_SLAVE_SELECT, OUTPUT);
    digitalWrite(PIN_TOUCHPAD_SLAVE_SELECT, HIGH);

    pinMode(PIN_SPI_SLAVE_SELECT_RX_A, OUTPUT);
    digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, HIGH);

    pinMode(PIN_SPI_SLAVE_SELECT_RX_B, OUTPUT);
    digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, HIGH);

    pinMode(PIN_RX_SWITCH, OUTPUT);
    digitalWrite(PIN_RX_SWITCH, LOW);

    pinMode(PIN_RSSI_A, INPUT);
    pinMode(PIN_RSSI_B, INPUT);
#ifdef PIN_VBAT
    analogSetPinAttenuation(PIN_VBAT, ADC_2_5db);
#endif
}


void setup()
{
    rtc_wdt_protect_off();
    rtc_wdt_disable();
    disableCore0WDT();
    //disableCore1WDT();

#if DEBUG_ENABLED || defined(SPEED_TEST)
    Serial.begin(115200);
#endif

    EepromSettings.setup();
    setupPins();

    mutex_spi = xSemaphoreCreateMutex();

    SPI.begin();
    SPI.setHwCs(false);

    StateMachine::setup();
    Ui::setup();
    TouchPad::setup();

    /* Setup receiver(s) and configure registers */
    ReceiverSpi::setup();

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

#ifdef USE_VOLTAGE_MONITORING
    Voltage::setup();
#endif

    comm_espnow_init();
    //WiFiConnect();
    //while(1) HandleWebUpdate();

    // Set loop task priority
    vTaskPrioritySet(NULL, TASK_PRIO_OSD);
}


void loop()
{
    if (Ui::isTvOn) {
        if (Ui::UiTimeOut.hasTicked() &&
                (StateMachine::currentState != StateMachine::State::SETTINGS_RSSI)) {
            Ui::tvOff();
            EepromSettings.update();
        } else {
            StateMachine::update();
        }
        return;
    }

    TouchPad::TouchData const touch = TouchPad::get();
    if (touch.buttonPrimary) {
        // Start TV, touch has happened
        Ui::tvOn();
        return;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);

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
