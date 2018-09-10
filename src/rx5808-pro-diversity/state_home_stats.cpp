#include <Arduino.h>
//#include <avr/pgmspace.h>

#include "settings_eeprom.h"

#include "state_home_stats.h"

#include "receiver.h"
#include "channels.h"
#include "buttons.h"
#include "state.h"
#include "ui.h"
#include "pstr_helper.h"
#include "voltage.h"
#include "temperature.h"

using StateMachine::HomeStatsStateHandler;

void HomeStatsStateHandler::onEnter() {
    Ui::clear();
}

void HomeStatsStateHandler::onUpdate() {
    Ui::needUpdate();
}

void HomeStatsStateHandler::onInitialDraw() {
    this->onUpdateDraw();
}

void HomeStatsStateHandler::onUpdateDraw() {
  
    // Channel name
    Ui::clearRect(0, 0, 60, 38);
    Ui::setTextSize(3);
    Ui::setTextColor(WHITE);
    Ui::setCursor(6, 0);
    Ui::display.print(Channels::getName(Receiver::activeChannel));

    // Frequency    
    Ui::setTextSize(2);
    Ui::setTextColor(WHITE);
    Ui::setCursor(0, 24);
    Ui::display.print(Channels::getFrequency(Receiver::activeChannel));

    // Clear Centered frequency
    if (!centred) {
        Ui::clearRect(0, 39, 43, 9);
    }
    
    // Temperature
    Ui::clearRect(0, 52, 21, 7);
    Ui::setCursor(0, 52);
    Ui::setTextSize(1);
    Ui::display.print((uint8_t)Temperature::temperature);
    Ui::display.print(PSTR2("C"));        

    // Voltage 
    #ifdef FENIX_QUADVERSITY  
        Ui::clearRect(28, 52, 18, 7);
        Ui::setTextSize(1);
        Ui::setTextColor(WHITE);
        Ui::setCursor(28, 52);
        Ui::display.print(Voltage::voltage); 
        Ui::display.print(PSTR2("."));        
        Ui::display.print(Voltage::voltageDec);
        Ui::setTextSize(1);
        Ui::display.print(PSTR2("V"));
//        Ui::clearRect(0, 49, 60, 14);
//        Ui::setTextSize(2);
//        Ui::setTextColor(WHITE);
//        Ui::setCursor(0, 49);
//        Ui::display.print(Voltage::voltage); 
//        Ui::display.print(PSTR2("."));        
//        Ui::display.print(Voltage::voltageDec);
//        Ui::setTextSize(1);
//        Ui::display.print(PSTR2("V"));
    #endif

    Ui::clearRect(60, 0, 67, 64);

    Ui::setTextSize(1);
    Ui::setTextColor(WHITE);
    
    Ui::drawFastHLine(61, 10, 66, WHITE);
    
    Ui::setCursor(62,13);
    Ui::display.print(PSTR2("A"));
    Ui::setCursor(62,26);
    Ui::display.print(PSTR2("B"));
    Ui::setCursor(62,39);
    Ui::display.print(PSTR2("C"));
    Ui::setCursor(62,52);
    Ui::display.print(PSTR2("D"));
    
    if (Receiver::activeReceiver == Receiver::ReceiverId::A) {
        Ui::drawRoundRect(60, 11, 9, 11, 2, WHITE);
    }
    if (Receiver::activeReceiver == Receiver::ReceiverId::B) {
        Ui::drawRoundRect(60, 24, 9, 11, 2, WHITE);
    }
    if (EepromSettings.quadversity) {
      if (Receiver::activeReceiver == Receiver::ReceiverId::C) {
          Ui::drawRoundRect(60, 37, 9, 11, 2, WHITE);
      }
      if (Receiver::activeReceiver == Receiver::ReceiverId::D) {
          Ui::drawRoundRect(60, 50, 9, 11, 2, WHITE);
      }
    }
    
    Ui::drawFastVLine(71, 0, 64, WHITE);
    
    Ui::setCursor(73,0);
    Ui::display.print(PSTR2("RSSI"));
    Ui::setCursor(73,13);
    Ui::display.print(Receiver::rssiA);
    Ui::setCursor(73,26);
    Ui::display.print(Receiver::rssiB);
    if (EepromSettings.quadversity) {
        Ui::setCursor(73,39);
        Ui::display.print(Receiver::rssiC);
        Ui::setCursor(73,52);
        Ui::display.print(Receiver::rssiD);
    }
    
    Ui::drawFastVLine(99, 0, 64, WHITE);
    
    Ui::setCursor(101,0);
    Ui::display.print(PSTR2("On %"));
    Ui::setCursor(101,13);
    Ui::display.print( (100 * Receiver::antennaAOnTime) / (millis() / 1000) );
    Ui::setCursor(101,26);
    Ui::display.print( (100 * Receiver::antennaBOnTime) / (millis() / 1000) );
    if (EepromSettings.quadversity) {
        Ui::setCursor(101,39);
        Ui::display.print( (100 * Receiver::antennaCOnTime) / (millis() / 1000) );
        Ui::setCursor(101,52);
        Ui::display.print( (100 * Receiver::antennaDOnTime) / (millis() / 1000) );
    }
    
    Ui::needDisplay();
}

void HomeStatsStateHandler::onButtonChange(
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

void HomeStatsStateHandler::setChannel(int channelIncrement) {

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
void HomeStatsStateHandler::centreFrequency() {

  uint16_t activeChannelFreq = Channels::getFrequency(Receiver::activeChannel);
  uint16_t centerFreq = Channels::getCenterFreq(activeChannelFreq);
  Receiver::setChannelByFreq(centerFreq);

  Ui::clearRect(0, 39, 43, 9);

  Ui::setTextSize(1);
  Ui::setTextColor(WHITE);
  Ui::setCursor(6, 40);
  Ui::display.print( centerFreq );
  
  Ui::drawBullseye(38, 43, 4, WHITE);
  
  centred = true;

}
