#include <Arduino.h>
#include <avr/pgmspace.h>

#include "settings_eeprom.h"
#include "state_finder.h"
#include "receiver.h"
#include "channels.h"
#include "buttons.h"
#include "state.h"
#include "ui.h"
#include "pstr_helper.h"
#include "timer.h"


using StateMachine::FinderStateHandler;

uint8_t antennaSelection = 0;
uint8_t rssi = 50;
uint8_t currentRssi;

Timer finderBeeper = Timer(0);

void FinderStateHandler::onEnter() {
    Ui::clear();
}

void FinderStateHandler::onUpdate() {
    Ui::needUpdate();
}

void FinderStateHandler::onInitialDraw() {
    this->onUpdateDraw();
}

void FinderStateHandler::onUpdateDraw() {

    Ui::clear();
    
    Ui::setTextSize(1);
    Ui::setTextColor(WHITE);
    
    Ui::setCursor(3, 8);
    Ui::display.print(PSTR2("A"));
    if (!EepromSettings.quadversity) {
      Ui::setCursor(3, 22);
      Ui::display.print(PSTR2("B"));
    }
    if (EepromSettings.quadversity) {
      Ui::setCursor(3, 22);
      Ui::display.print(PSTR2("B"));
      Ui::setCursor(3, 36);
      Ui::display.print(PSTR2("C"));
      Ui::setCursor(3, 50);
      Ui::display.print(PSTR2("D"));
    }

    Ui::setCursor(15, 8);
    Ui::display.print(Channels::getName(Receiver::activeChannel));


    if (!EepromSettings.quadversity) {
      if (antennaSelection == 0) {
        Ui::drawRoundRect(1, 6, 9, 11, 2, WHITE);
        currentRssi = Receiver::rssiA;
      }
      if (antennaSelection == 1) {
        Ui::drawRoundRect(1, 20, 9, 11, 2, WHITE);
        currentRssi = Receiver::rssiB;
      }
      if (antennaSelection == 2) {
        Ui::drawRoundRect(1, 6, 9, 25, 2, WHITE);
        currentRssi = (Receiver::rssiA + Receiver::rssiB)/2; 
      }
    }
    if (EepromSettings.quadversity) {
      if (antennaSelection == 0) {
        Ui::drawRoundRect(1, 6, 9, 11, 2, WHITE);
        currentRssi = Receiver::rssiA;
      }
      if (antennaSelection == 1) {
        Ui::drawRoundRect(1, 20, 9, 11, 2, WHITE);
        currentRssi = Receiver::rssiB;
      }
      if (antennaSelection == 2) {
        Ui::drawRoundRect(1, 34, 9, 11, 2, WHITE);
        currentRssi = Receiver::rssiC;
      }
      if (antennaSelection == 3) {
        Ui::drawRoundRect(1, 48, 9, 11, 2, WHITE);
        currentRssi = Receiver::rssiD;
      }
      if (antennaSelection == 4) {
        Ui::drawRoundRect(1, 6, 9, 53, 2, WHITE);
        currentRssi = ((Receiver::rssiA + Receiver::rssiB)/2 + (Receiver::rssiC +  Receiver::rssiD)/2) / 2; 
      }
    }

    // An attempt to slow the rapid RSSI change
    if (currentRssi > rssi) {
      rssi = rssi + 2;
      if (rssi > 100) rssi = 100;
    } else {
      rssi = rssi - 2;
      if (rssi > 100) rssi = 0;
    }
      
    Ui::setCursor(15, 50);
    Ui::display.print(rssi);
    Ui::display.print(PSTR2("%"));   

    Ui::drawCircle(64, 32, 30.0*(float)rssi/100.0, WHITE, BLACK);
    
    Ui::drawBullseye(64, 32, 30, WHITE);

    uint8_t barHeight = 64.0*(float)rssi/100.0;
    Ui::fillRect(107, 64-barHeight, 20, barHeight, WHITE);
      
    if (finderBeeper.hasTicked()) {
      uint16_t beepInterval = 2000 - 2000*((float)rssi/100.0);
      finderBeeper = Timer(beepInterval);
      Ui::beep(8000-2*beepInterval);
    }

    Ui::needDisplay();
}

void FinderStateHandler::onButtonChange(
    Button button,
    Buttons::PressType pressType
) {
  
  if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::MODE_PRESSED
     ) {
          if (!EepromSettings.quadversity) {
              antennaSelection++;
              if (antennaSelection > 2)
                antennaSelection = 0;
          }
          if (EepromSettings.quadversity) {
              antennaSelection++;
              if (antennaSelection > 4)
                antennaSelection = 0;
          }
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::UP_PRESSED
     ) {
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::DOWN_PRESSED
     ) {
        }
  else if (
      pressType == Buttons::PressType::LONG &&
      button == Button::MODE_PRESSED
     ) {
        }
}

