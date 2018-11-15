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
#include "settings_internal.h"
#include "settings_eeprom.h"

#include "channels.h"
#include "receiver.h"
#include "receiver_spi.h"
#include "state.h"
#include "ui.h"
#include "voltage.h"
#include "temperature.h"
#include "touchpad.h"

void setup()
{
  
//  EepromSettings.load();

//  setupPins();
//  Temperature::setup();
  StateMachine::setup();
//  Ui::setup(); 
//  TouchPad::setup(); 

//  // Flash lights as startup sequency
//  for (int x=0; x<20; x++) {
//    digitalWrite(PIN_LED_A,!digitalRead(PIN_LED_A));
//    digitalWrite(PIN_LED_B,!digitalRead(PIN_LED_B));
//    #ifdef FENIX_QUADVERSITY
//      digitalWrite(PIN_LED_C,!digitalRead(PIN_LED_C));
//      digitalWrite(PIN_LED_D,!digitalRead(PIN_LED_D));
//      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
//    #endif
//    Ui::beep(x*500);
//  }

//  // Has to be last setup() otherwise channel may not be set.
//  // RX possibly not botting quick enough if setup() is called earler.
//  Receiver::setup(); 
//
//  if (!EepromSettings.isCalibrated) {
//      StateMachine::switchState(StateMachine::State::SETTINGS_RSSI); 
//      Ui::switchOSDOutputState();    
//  } else {
//      StateMachine::switchState(StateMachine::State::HOME); 
//  }   
//
//  #ifdef FENIX_QUADVERSITY
//    digitalWrite(PIN_LED, HIGH);  // ON
//  #endif

}

//void setupPins() {
//  
//  #ifdef FENIX_QUADVERSITY
//    pinMode(PIN_OSDCONTROL, OUTPUT);
//    digitalWrite(PIN_OSDCONTROL, HIGH);
//    pinMode(PIN_LED, OUTPUT);
//    digitalWrite(PIN_LED, LOW);
//  #endif
//  
//  #ifdef REALACC_RX5808_PRO_PLUS_OSD
//    pinMode(TS5A3159, OUTPUT);
//  #endif
//  
//    pinMode(PIN_LED_A, OUTPUT);
//    digitalWrite(PIN_LED_A, LOW);
//    pinMode(PIN_LED_B, OUTPUT);
//    digitalWrite(PIN_LED_B, LOW);
//    pinMode(PIN_LED_C, OUTPUT);
//    digitalWrite(PIN_LED_C, LOW);
//    pinMode(PIN_LED_D, OUTPUT);
//    digitalWrite(PIN_LED_D, LOW);
//  
//    pinMode(PIN_RSSI_A, INPUT_ANALOG);
//    pinMode(PIN_RSSI_B, INPUT_ANALOG);
//    pinMode(PIN_RSSI_C, INPUT_ANALOG);
//    pinMode(PIN_RSSI_D, INPUT_ANALOG);
//  
//  #ifdef FENIX_QUADVERSITY
//    pinMode(PIN_SPI_SLAVE_SELECT, OUTPUT);
//  #endif
//  #ifdef REALACC_RX5808_PRO_PLUS_OSD
//    pinMode(PIN_SPI_SLAVE_SELECT_A, OUTPUT);
//    pinMode(PIN_SPI_SLAVE_SELECT_B, OUTPUT);
//  #endif
//    pinMode(PIN_SPI_DATA, OUTPUT);
//    pinMode(PIN_SPI_CLOCK, OUTPUT);
//    
//  #ifdef FENIX_QUADVERSITY
//    digitalWrite(PIN_SPI_SLAVE_SELECT, HIGH);
//  #endif
//  #ifdef REALACC_RX5808_PRO_PLUS_OSD
//    digitalWrite(PIN_SPI_SLAVE_SELECT_A, HIGH);
//    digitalWrite(PIN_SPI_SLAVE_SELECT_B, HIGH);
//  #endif
//    digitalWrite(PIN_SPI_CLOCK, LOW);
//    digitalWrite(PIN_SPI_DATA, LOW);
//    
//  #ifdef FENIX_QUADVERSITY
//    pinMode(PIN_VBAT, INPUT_ANALOG);
//  #endif
//}

void loop() {
  
    Receiver::update();
    
//    #ifdef FENIX_QUADVERSITY  
//        Voltage::update();
//    #endif
  
//    if (Ui::UiRefreshTimer.hasTicked()) {
//        Ui::UiRefreshTimer.reset();
//        TouchPad::update(); 
//        
//        if (Ui::isTvOn) {
//            Temperature::update();
//            StateMachine::update();
//            Ui::update();
//            EepromSettings.update();
//        }
//    }
    
//    if (TouchPad::touchData.isActive) {
//        Ui::UiTimeOut.reset();
//    }
//    if (Ui::isTvOn && Ui::UiTimeOut.hasTicked()) {
//        Ui::switchOSDOutputState();    
//    }
//    if (!Ui::isTvOn && TouchPad::touchData.buttonPrimary) {
//        Ui::switchOSDOutputState();
//    }
  
//    TouchPad::clearTouchData(); 
}
