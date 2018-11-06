#include <Arduino.h>
//#include <avr/pgmspace.h>

#include "settings_eeprom.h"

#include "state_home_simple.h"

#include "receiver.h"
#include "channels.h"
#include "buttons.h"
#include "state.h"
#include "ui.h"
#include "pstr_helper.h"
#include "voltage.h"

using StateMachine::HomeSimpleStateHandler;

void HomeSimpleStateHandler::onEnter() {
//    Ui::clear();
}

void HomeSimpleStateHandler::onUpdate() {
    Ui::needUpdate();
}

void HomeSimpleStateHandler::onInitialDraw() {
    this->onUpdateDraw();
}

void HomeSimpleStateHandler::onUpdateDraw() {
  
    Ui::clear();

    Ui::setTextColor(WHITE);

    Ui::setTextSize(6);
    Ui::setCursor(
        SCREEN_WIDTH_MID - ((CHAR_WIDTH) * 6) / 2 * 2 - 3,
        2);

    Ui::display.print(Channels::getName(Receiver::activeChannel));

    Ui::setTextSize(2);
    Ui::setCursor(
        SCREEN_WIDTH_MID - ((CHAR_WIDTH + 1) * 2) / 2 * 4 - 1,
        SCREEN_HEIGHT - CHAR_HEIGHT * 2 - 2);
    Ui::display.print(Channels::getFrequency(Receiver::activeChannel));

    // Show Bullseye if Centered frequency
    if (centred) {
      Ui::drawBullseye(92, 55, 6, WHITE);
    }
      
    Ui::needDisplay();
}

void HomeSimpleStateHandler::onButtonChange(
    Button button,
    Buttons::PressType pressType
) {
  
  if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::MODE_PRESSED
     ) {
        #ifdef FENIX_QUADVERSITY 
          this->centreFrequency();
        #else
          this->setChannel(8);
        #endif  
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::UP_PRESSED
     ) {
        this->setChannel(-1);
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::DOWN_PRESSED
     ) {
        this->setChannel(1);
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::LEFT_PRESSED
     ) {
        this->setChannel(-8);
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::RIGHT_PRESSED
     ) {
        this->setChannel(8);
        }
  else if (
      pressType == Buttons::PressType::LONG &&
      button == Button::MODE_PRESSED
     ) {
        #ifdef FENIX_QUADVERSITY  
          // Future menu ???
        #else
          this->centreFrequency();
        #endif  
        }
}

void HomeSimpleStateHandler::setChannel(int channelIncrement) {

    int band = Receiver::activeChannel / 8;
    int channel = Receiver::activeChannel % 8;
    
    if (channelIncrement == 8) {
      band = band + 1;
    }
    
    if (channelIncrement == -8) {
      band = band - 1;
    }
    
    if (channelIncrement == 1 ) {
      channel = channel + 1;
      if (channel > 7) {
        channel = 0;
      }
    }
    
    if (channelIncrement == -1 ) {
      channel = channel - 1;
      if (channel < 0) {
        channel = 7;
      }
    }
    
    int newChannelIndex = band * 8 + channel;

    if (newChannelIndex >= CHANNELS_SIZE) {
      newChannelIndex = newChannelIndex - CHANNELS_SIZE;
    }
    if (newChannelIndex < 0) {
      newChannelIndex = newChannelIndex + CHANNELS_SIZE;
    }
    Receiver::setChannel(newChannelIndex);
    EepromSettings.startChannel = newChannelIndex;
    EepromSettings.markDirty();
    centred = false;
}

// Frequency 'Centring' function.
// The function walks up and then down from the currently Rx frequency 
// in 1 MHz steps until RSSI < threshold.  The Rx is then set to the 
// centre of these 2 frequencies.
void HomeSimpleStateHandler::centreFrequency() {

  uint16_t activeChannelFreq = Channels::getFrequency(Receiver::activeChannel);
  uint16_t centerFreq = Channels::getCenterFreq(activeChannelFreq);
  Receiver::setChannelByFreq(centerFreq);
  
  centred = true;
}
