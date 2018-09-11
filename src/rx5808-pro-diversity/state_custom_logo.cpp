#include <Arduino.h>
#include <avr/pgmspace.h>

#include "settings_eeprom.h"

#include "state_custom_logo.h"

#include "receiver.h"
#include "channels.h"
#include "buttons.h"
#include "state.h"
#include "ui.h"
#include "pstr_helper.h"
#include "voltage.h"


using StateMachine::CustomLogoStateHandler;


void CustomLogoStateHandler::onEnter() {
    Ui::clear();
}

void CustomLogoStateHandler::onUpdate() {

    Ui::clear();

    #ifndef EEPROM_AT24C02
        Ui::drawBitmap(
            0,
            0,
            EepromSettings.customLogo,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            WHITE
        );
    #endif
    
    Ui::drawCircle(x, y, 3, WHITE, BLACK);
    
    Ui::needUpdate();
}

void CustomLogoStateHandler::onInitialDraw() {
    this->onUpdateDraw();
}

void CustomLogoStateHandler::onUpdateDraw() {

    Ui::needDisplay();
}

void CustomLogoStateHandler::onButtonChange(
    Button button,
    Buttons::PressType pressType
) {
  
  if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::MODE_PRESSED
     ) {
          uint16_t byteIndex = (128*y+x)/8;
          #ifndef EEPROM_AT24C02
              byte byteToChange = EepromSettings.customLogo[byteIndex];
          #endif
          uint8_t bitToChange = 7 - (128*y+x) % 8;

          #ifndef EEPROM_AT24C02
              EepromSettings.customLogo[byteIndex] = bitWrite(byteToChange, bitToChange, !bitRead(byteToChange, bitToChange));
          #endif
          
          EepromSettings.markDirty();
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::UP_PRESSED
     ) {
          y--;
          if (y > 63) {
            y = 63;
          }
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::DOWN_PRESSED
     ) {
          y++;
          if (y > 63) {
            y = 0;
          }
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::LEFT_PRESSED
     ) {
          x--;
          if (x > 127) {
            x = 127;
          }
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::RIGHT_PRESSED
     ) {
          x++;
          if (x > 127) {
            x = 0;
          }
        }
  else if (
      pressType == Buttons::PressType::LONG &&
      button == Button::MODE_PRESSED
     ) {
          // Future menu???
        }
  else if (
      pressType == Buttons::PressType::LONG &&
      button == Button::UP_PRESSED
     ) {
          y -= 10;
          if (y > 63 ) {
            y -= 256-64;
          }
        }
  else if (
      pressType == Buttons::PressType::LONG &&
      button == Button::DOWN_PRESSED
     ) {
          // This function is reserved for TVout!
        }
  else if (
      pressType == Buttons::PressType::LONG &&
      button == Button::LEFT_PRESSED
     ) {
          x -= 10;
          if (x > 127) {
            x -= 256-128;
          }
        }
  else if (
      pressType == Buttons::PressType::LONG &&
      button == Button::RIGHT_PRESSED
     ) {
          x += 10;
          if (x > 127) {
            x -= 128;
          }
        }
}

