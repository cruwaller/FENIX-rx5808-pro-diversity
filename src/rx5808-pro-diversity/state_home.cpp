#include <Arduino.h>
//#include <avr/pgmspace.h>

#include "settings_eeprom.h"

#include "state_home.h"

#include "receiver.h"
#include "channels.h"
#include "buttons.h"
#include "state.h"
#include "ui.h"
#include "pstr_helper.h"
#include "voltage.h"

#define COLUMN_WIDTH 7

#define CHANNEL_TEXT_SIZE 4
#define CHANNEL_TEXT_X 3
#define CHANNEL_TEXT_Y 4
#define CHANNEL_TEXT_H (CHAR_HEIGHT * CHANNEL_TEXT_SIZE)

#define FREQUENCY_TEXT_SIZE 2
#define FREQUENCY_TEXT_X 2
#define FREQUENCY_TEXT_Y (SCREEN_HEIGHT - (CHAR_HEIGHT * 3)-4)
#define FREQUENCY_TEXT_H (CHAR_HEIGHT * FREQUENCY_TEXT_SIZE)

using StateMachine::HomeStateHandler;

void HomeStateHandler::onEnter() {
    Ui::clear();
}

void HomeStateHandler::onUpdate() {
    Ui::needUpdate();
}

void HomeStateHandler::onInitialDraw() {
    this->onUpdateDraw();
}

void HomeStateHandler::onUpdateDraw() {

    // Channel name
    Ui::clearRect(
        CHANNEL_TEXT_X,
        CHANNEL_TEXT_Y,
        SCREEN_WIDTH - 4*(COLUMN_WIDTH+2),
        CHANNEL_TEXT_H
    );

    Ui::setTextSize(CHANNEL_TEXT_SIZE);
    Ui::setTextColor(WHITE);
    Ui::setCursor(CHANNEL_TEXT_X, CHANNEL_TEXT_Y);
    Ui::display.print(Channels::getName(Receiver::activeChannel));

    // Frequency
    Ui::clearRect(
        FREQUENCY_TEXT_X,
        FREQUENCY_TEXT_Y,
        SCREEN_WIDTH - 4*(COLUMN_WIDTH+2),
        CHAR_HEIGHT * 2
    );
    
    Ui::setTextSize(FREQUENCY_TEXT_SIZE);
    Ui::setTextColor(WHITE);
    Ui::setCursor(FREQUENCY_TEXT_X, FREQUENCY_TEXT_Y);
    Ui::display.print(Channels::getFrequency(Receiver::activeChannel));

    // Clear Centered frequency
    if (!centred) {
      Ui::clearRect(
        0, 
        FREQUENCY_TEXT_Y + CHAR_HEIGHT * 2,
        SCREEN_WIDTH - 4*(COLUMN_WIDTH+2),
        CHAR_HEIGHT * 2);
    }

    // Voltage 
    #ifdef FENIX_QUADVERSITY
//        Ui::clearRect(
//          SCREEN_WIDTH - 15,
//          0,
//          15,
//          SCREEN_HEIGHT - 1
//        );
//    
//        Ui::clearRect(45, 56, 36, 7);
//        Ui::setTextSize(1);
//        Ui::setTextColor(WHITE);
//        Ui::setCursor(45, 56);
//        Ui::display.print(Voltage::voltage);        
//        Ui::display.print(PSTR2("."));        
//        Ui::display.print(Voltage::voltageDec);
//        Ui::display.print(PSTR2("V"));

        // Trying to determine if batter is 2, 3, or 4 cell.
        Ui::setTextSize(1);
        Ui::setTextColor(WHITE);
        Ui::setCursor(115, 54);

        uint8_t maxVoltage;
        if (EepromSettings.vbatWarning < 80) {
          maxVoltage = 2*42;
          Ui::display.print(PSTR2("2S"));
        } else if (EepromSettings.vbatWarning < 120) {
          maxVoltage = 3*42;
          Ui::display.print(PSTR2("3S"));
        } else {
          maxVoltage = 4*42;
          Ui::display.print(PSTR2("4S"));
        }
        
        Ui::drawRoundRect(SCREEN_WIDTH - 13 + 2, 0, 6, 4, 2, WHITE);
        Ui::drawRoundRect(SCREEN_WIDTH - 15, 3, 14, SCREEN_HEIGHT-17, 2, WHITE);
        
        uint8_t batteryHeight = constrain(
          map(
            Voltage::voltage * 10 + Voltage::voltageDec,
            EepromSettings.vbatWarning,
            maxVoltage,
            1,
            SCREEN_HEIGHT-21
            ),
            1,
            SCREEN_HEIGHT-21
            );
        Ui::fillRect(SCREEN_WIDTH - 15 + 2, SCREEN_HEIGHT-16-batteryHeight, 10, batteryHeight, WHITE);
    #endif

    uint8_t rssiAHeight = 0;
    uint8_t rssiBHeight = 0;
    uint8_t rssiCHeight = 0;
    uint8_t rssiDHeight = 0;
    
    // RSSI bars
    rssiAHeight = constrain(map(Receiver::rssiA, 0, 100, 1, (SCREEN_HEIGHT-14)), 1, (SCREEN_HEIGHT-14));
    rssiBHeight = constrain(map(Receiver::rssiB, 0, 100, 1, (SCREEN_HEIGHT-14)), 1, (SCREEN_HEIGHT-14));
    if (EepromSettings.quadversity) {
      rssiCHeight = constrain(map(Receiver::rssiC, 0, 100, 1, (SCREEN_HEIGHT-14)), 1, (SCREEN_HEIGHT-14));
      rssiDHeight = constrain(map(Receiver::rssiD, 0, 100, 1, (SCREEN_HEIGHT-14)), 1, (SCREEN_HEIGHT-14));
    }

    int8_t xOffset = -16;

    Ui::clearRect(
        xOffset + SCREEN_WIDTH - 4*(COLUMN_WIDTH+2) - 1,
        0,
        4*(COLUMN_WIDTH+2),
        SCREEN_HEIGHT - 1
    );
    
    Ui::fillRect(xOffset + SCREEN_WIDTH - 4*(COLUMN_WIDTH+2), (SCREEN_HEIGHT-14) - rssiAHeight, COLUMN_WIDTH, rssiAHeight, WHITE);
    Ui::fillRect(xOffset + SCREEN_WIDTH - 3*(COLUMN_WIDTH+2), (SCREEN_HEIGHT-14) - rssiBHeight, COLUMN_WIDTH, rssiBHeight, WHITE);
    if (EepromSettings.quadversity) {
      Ui::fillRect(xOffset + SCREEN_WIDTH - 2*(COLUMN_WIDTH+2), (SCREEN_HEIGHT-14) - rssiCHeight, COLUMN_WIDTH, rssiCHeight, WHITE);
      Ui::fillRect(xOffset + SCREEN_WIDTH - 1*(COLUMN_WIDTH+2), (SCREEN_HEIGHT-14) - rssiDHeight, COLUMN_WIDTH, rssiDHeight, WHITE);
    }
    
    Ui::setTextSize(1);
    Ui::setTextColor(WHITE);
    
    Ui::setCursor(xOffset + SCREEN_WIDTH - 4*(COLUMN_WIDTH+2)+1, 54);
    Ui::display.print(PSTR2("A"));
    Ui::setCursor(xOffset + SCREEN_WIDTH - 3*(COLUMN_WIDTH+2)+1, 54);
    Ui::display.print(PSTR2("B"));
    if (EepromSettings.quadversity) {
      Ui::setCursor(xOffset + SCREEN_WIDTH - 2*(COLUMN_WIDTH+2)+1, 54);
      Ui::display.print(PSTR2("C"));
      Ui::setCursor(xOffset + SCREEN_WIDTH - 1*(COLUMN_WIDTH+2)+1, 54);
      Ui::display.print(PSTR2("D"));
    }

    if (Receiver::activeReceiver == Receiver::ReceiverId::A) {
        Ui::drawRoundRect(xOffset + SCREEN_WIDTH - 4*(COLUMN_WIDTH+2)-1, 52, 9, 11, 2, WHITE);
    }
    if (Receiver::activeReceiver == Receiver::ReceiverId::B) {
        Ui::drawRoundRect(xOffset + SCREEN_WIDTH - 3*(COLUMN_WIDTH+2)-1, 52, 9, 11, 2, WHITE);
    }
    if (EepromSettings.quadversity) {
      if (Receiver::activeReceiver == Receiver::ReceiverId::C) {
          Ui::drawRoundRect(xOffset + SCREEN_WIDTH - 2*(COLUMN_WIDTH+2)-1, 52, 9, 11, 2, WHITE);
      }
      if (Receiver::activeReceiver == Receiver::ReceiverId::D) {
          Ui::drawRoundRect(xOffset + SCREEN_WIDTH - 1*(COLUMN_WIDTH+2)-1, 52, 9, 11, 2, WHITE);
      }
    }
      
    Ui::needDisplay();
}

void HomeStateHandler::onButtonChange(
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

void HomeStateHandler::setChannel(int channelIncrement) {

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
void HomeStateHandler::centreFrequency() {

  uint16_t activeChannelFreq = Channels::getFrequency(Receiver::activeChannel);
  uint16_t centerFreq = Channels::getCenterFreq(activeChannelFreq);
  Receiver::setChannelByFreq(centerFreq);

  Ui::clearRect(
  0, 
  FREQUENCY_TEXT_Y + CHAR_HEIGHT * 2,
  SCREEN_WIDTH - 4*(COLUMN_WIDTH+2),
  CHAR_HEIGHT * 2);
        
  Ui::setCursor(9, 56);
  Ui::setTextSize(1);
  Ui::setTextColor(WHITE);
  Ui::display.print( centerFreq );
  Ui::drawBullseye(39, 59, 4, WHITE);
  
  centred = true;

  // Clear centering notification
  Ui::clearRect(26, 27, 76, 12);
}
