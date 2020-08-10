#include <Arduino.h>
#include <WString.h>

#include "settings_eeprom.h"
#include "settings.h"

#include "state_home.h"

#include "receiver.h"
#include "receiver_spi.h"
#include "channels.h"
#include "state.h"
#include "ui.h"
#include "temperature.h"
#include "voltage.h"
#include "touchpad.h"
#include "ExpressLRS_Protocol.h"
#include "comm_espnow.h"

#include "lap_times.h"


// For scalling graphics accross screen
#if (CHANNELS_SIZE == 72)
    #define CHANNELS_SIZE_DIVIDER 4
#elif (CHANNELS_SIZE == 48)
    #define CHANNELS_SIZE_DIVIDER 6
#endif

using StateMachine::HomeStateHandler;

void HomeStateHandler::onEnter() {

    displayActiveChannel = Receiver::activeChannel;

}

void HomeStateHandler::onUpdate() {
    if (TouchPad::touchData.buttonPrimary) {
      TouchPad::touchData.buttonPrimary = false;
      this->doTapAction();
    }
    this->onUpdateDraw();
}

void HomeStateHandler::onInitialDraw() {
    this->onUpdateDraw();
}

void HomeStateHandler::onUpdateDraw() {
    uint32_t sec_now;

    if (isInBandScanRegion()) {
        bandScanUpdate();
        wasInBandScanRegion = true;
    } else {
        if (wasInBandScanRegion) {
            Receiver::setChannel(displayActiveChannel);
        }
        displayActiveChannel = Receiver::activeChannel;
        wasInBandScanRegion = false;
    }

#if 0
    /*************************************************/
    /*********     PRINT HEADER     ******************/
    // Mode
    Ui::display.setTextColor(100);
    Ui::display.setCursor( 8, 0);
    Ui::display.print("Mode: ");
    if (EepromSettings.diversityMode == Receiver::DiversityMode::ANTENNA_A) {
        Ui::display.print("Antenna A");
    }
    else if (EepromSettings.diversityMode == Receiver::DiversityMode::ANTENNA_B) {
        Ui::display.print("Antenna B");
    }
    else if (EepromSettings.diversityMode == Receiver::DiversityMode::ANTENNA_C) {
        Ui::display.print("Antenna C");
    }
    else if (EepromSettings.diversityMode == Receiver::DiversityMode::ANTENNA_D) {
        Ui::display.print("Antenna D");
    }
    else if (EepromSettings.diversityMode == Receiver::DiversityMode::DIVERSITY) {
        Ui::display.print("Diversity");
    }
    else if (EepromSettings.diversityMode == Receiver::DiversityMode::QUADVERSITY) {
        Ui::display.print("Quadversity");
    }

#ifdef USE_VOLTAGE_MONITORING
    // Voltage
    if (Voltage::voltage > 9) {
        Ui::display.setCursor( 173, 0);
    } else {
        Ui::display.setCursor( 181, 0);
    }
    Ui::display.print(Voltage::voltage);
    Ui::display.print(".");
    Ui::display.print(Voltage::voltageDec);
    Ui::display.print("V ");
#else
    Ui::display.setCursor( 221, 0);
#endif

#ifdef USE_TEMPERATURE_MONITORING
    // Temperature // Doesnt currently work within ESP32 Arduino.
    Ui::display.print(Temperature::getTemperature());
    Ui::display.print("C ");
#else
    Ui::display.setCursor( 221+4*8, 0);
#endif

    // On Time
    sec_now = millis() / 1000;
    uint8_t hours = sec_now / 60 / 60;
    uint8_t mins  = sec_now / 60 - hours * 60 * 60;
    uint8_t secs  = sec_now - hours * 60 * 60 - mins * 60;
    Ui::display.print(hours);
    Ui::display.print(":");
    if(mins < 10) {
        Ui::display.print("0");
    }
    Ui::display.print(mins);
    Ui::display.print(":");
    if(secs < 10) {
        Ui::display.print("0");
    }
    Ui::display.print(secs);

    // Menu Icon
    Ui::display.line( 315, 1, 322, 1, 100);
    Ui::display.line( 315, 4, 322, 4, 100);
    Ui::display.line( 315, 7, 322, 7, 100);

    // Horixontal line
    Ui::display.line( 0, 9, SCREEN_WIDTH, 9, 100);
#else
    drawHeader();
#endif

    /*************************************************/
    /*********      PRINT HOME      ******************/

    // Display Band and Channel
    Ui::display.setCursor( 2, 15);
    Ui::display.printLarge(Channels::getName(displayActiveChannel), 8, 12);

    // Display Frequency
    Ui::display.setCursor( 0, 105);
    Ui::display.printLarge(Channels::getFrequency(displayActiveChannel), 4, 3);

    // Channel labels
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
    if (EepromSettings.quadversity) {
        Ui::drawBigCharacter( 190, 23,
                              'A',
                              3, 2);
        Ui::drawBigCharacter( 190, 23 + 28,
                              'B',
                              3, 2);
        Ui::drawBigCharacter( 190, 23 + 28*2,
                              'C',
                              3, 2);
        Ui::drawBigCharacter( 190, 23 + 28*3,
                              'D',
                              3, 2);
    }
    else
#else
    {
        Ui::display.setCursor( 130, 12 + 28*0 + 3);
        Ui::display.printLarge("A", 2, 2);
        Ui::display.setCursor( 130, 12 + 28*2 + 3);
        Ui::display.printLarge("B", 2, 2);//
    }
#endif

    // Channel selected square
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
    if (EepromSettings.quadversity) {
      if (Receiver::activeReceiver == Receiver::ReceiverId::A) {
          Ui::drawRoundRect(185, 20, 34, 24, 2, WHITE);
      }
      if (Receiver::activeReceiver == Receiver::ReceiverId::B) {
          Ui::drawRoundRect(185, 20 + 28, 34, 24, 2, WHITE);
      }
      if (Receiver::activeReceiver == Receiver::ReceiverId::C) {
          Ui::drawRoundRect(185, 20 + 28*2, 34, 24, 2, WHITE);
      }
      if (Receiver::activeReceiver == Receiver::ReceiverId::D) {
          Ui::drawRoundRect(185, 20 + 28*3, 34, 24, 2, WHITE);
      }
    }
    else
#else
    {
      if (Receiver::activeReceiver == Receiver::ReceiverId::A) {
          Ui::display.rect(128, 10 + 28*0 + 3, 19, 19, 100);
      }
      if (Receiver::activeReceiver == Receiver::ReceiverId::B) {
          Ui::display.rect(128, 10 + 28*2 + 3, 18, 18, 100);
      }
    }
#endif

    sec_now = millis() / 1000;
    // On percentage
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
    if (EepromSettings.quadversity) {
        Ui::setCursor(225, 25);
        Ui::display.print( (100 * Receiver::antennaAOnTime) / (sec_now) );
        Ui::display.print(PSTR2("%"));
        Ui::setCursor(225, 25+28);
        Ui::display.print( (100 * Receiver::antennaBOnTime) / (sec_now) );
        Ui::display.print(PSTR2("%"));
        Ui::setCursor(225, 25+28*2);
        Ui::display.print( (100 * Receiver::antennaCOnTime) / (sec_now) );
        Ui::display.print(PSTR2("%"));
        Ui::setCursor(225, 25+28*3);
        Ui::display.print( (100 * Receiver::antennaDOnTime) / (sec_now) );
        Ui::display.print(PSTR2("%"));
    }
    else
#else
    {
        Ui::display.setCursor(128, 30 + 28*0 + 3);
        Ui::display.print( (100.0 * Receiver::antennaAOnTime) / (sec_now) );
        Ui::display.print("%");
        Ui::display.setCursor(128, 32 + 28*2 + 3);
        Ui::display.print( (100.0 * Receiver::antennaBOnTime) / (sec_now) );
        Ui::display.print("%");
    }
#endif

#if HOME_SHOW_LAPTIMES
    Ui::display.setCursor(195, 12 + 28 + 3);
    Ui::display.print("LAP TIMES (");
    Ui::display.print(lapt_time_race_idx_get());
    Ui::display.print(")");
    uint8_t node_idx = 0;
    uint32_t lap_time;
    for (uint8_t iter = 1; iter <= 10; iter++) {
        lap_time = lapt_time_laptime_get(node_idx, iter);
        (void)lap_time;

        Ui::display.setCursor(195, 12 + 28 + 3 + iter*9);
        Ui::display.print("00:");
        if (iter < 10) {
            Ui::display.print("0");
        }
        Ui::display.print(iter);
        Ui::display.print(":1234");
    }

#else // !HOME_SHOW_LAPTIMES
    // Draw RSSI Plots
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
    if (EepromSettings.quadversity) {
        Ui::drawRect(250, 20 + 28*0 + 3, 64*3-1, 28*1-1, WHITE);
        Ui::drawRect(250, 20 + 28*1 + 3, 64*3-1, 28*1-1, WHITE);
        Ui::drawRect(250, 20 + 28*2 + 3, 64*3-1, 28*1-1, WHITE);
        Ui::drawRect(250, 20 + 28*3 + 3, 64*3-1, 28*1-1, WHITE);
        for (uint8_t i=0; i < RECEIVER_LAST_DATA_SIZE; i++) {
            Ui::drawLine(250+3*i, (20 + 28*1)-Receiver::rssiALast[i]/4, 250+3*(i+1), (20 + 28*1)-Receiver::rssiALast[i+1]/4, WHITE);
            Ui::drawLine(250+3*i, (20 + 28*2)-Receiver::rssiBLast[i]/4, 250+3*(i+1), (20 + 28*2)-Receiver::rssiBLast[i+1]/4, WHITE);
            Ui::drawLine(250+3*i, (20 + 28*3)-Receiver::rssiBLast[i]/4, 250+3*(i+1), (20 + 28*3)-Receiver::rssiBLast[i+1]/4, WHITE);
            Ui::drawLine(250+3*i, (20 + 28*4)-Receiver::rssiBLast[i]/4, 250+3*(i+1), (20 + 28*4)-Receiver::rssiBLast[i+1]/4, WHITE);
        }
    }
    else
#else
    {
        Ui::display.rect(195, 12 + 28*0 + 3, 64*2, 28*2-1, 100);
        Ui::display.rect(195, 12 + 28*2 + 3, 64*2, 28*2-1, 100);
        for (uint8_t i=0; i < RECEIVER_LAST_DATA_SIZE-1; i++) {
            Ui::display.line(195+1*i, (12 + 28*2)-Receiver::rssiALast[i]/20, 195+1*(i+1), (12 + 28*2)-Receiver::rssiALast[i+1]/20, 100);
            Ui::display.line(195+1*i, (12 + 28*4)-Receiver::rssiBLast[i]/20, 195+1*(i+1), (12 + 28*4)-Receiver::rssiBLast[i+1]/20, 100);
        }
    }
#endif
#endif // HOME_SHOW_LAPTIMES

    // Plot Spectrum 324 x 224
    for (uint8_t i=0; i<CHANNELS_SIZE; i++) {
        Ui::display.fillRect(18+CHANNELS_SIZE_DIVIDER*i, 214 - Receiver::rssiBandScanData[i]*8/100, CHANNELS_SIZE_DIVIDER, Receiver::rssiBandScanData[i]*8/100, Receiver::rssiBandScanData[i]/10);
    }
    Ui::display.line(0, 213, 323, 213, 100);
    Ui::display.setCursor( 1, 215);
    Ui::display.print(Channels::getFrequency(Channels::getOrderedIndex(0)));
    Ui::display.setCursor( 290, 215);
    Ui::display.print(Channels::getFrequency(Channels::getOrderedIndex(CHANNELS_SIZE-1)));

    // Marker triangle
    uint8_t markerX = Channels::getOrderedIndexFromIndex(Receiver::activeChannel);
    for (int i = 0; i < 7; i++) {
        Ui::display.line(18+CHANNELS_SIZE_DIVIDER*markerX, 214, 18+CHANNELS_SIZE_DIVIDER*markerX+(-3+i), 219, 100);
    }

    if (HomeStateHandler::isInBandScanRegion() && TouchPad::touchData.cursorX > 18 && TouchPad::touchData.cursorX < (324-18)) {
        Ui::display.fillRect( TouchPad::touchData.cursorX - 33, TouchPad::touchData.cursorY - 17, 33, 17, 10);
        Ui::display.setCursor( TouchPad::touchData.cursorX - 32, TouchPad::touchData.cursorY - 16 );
        Ui::display.print(Channels::getName(
                                            Channels::getOrderedIndex(
                                                                     (TouchPad::touchData.cursorX-18) / CHANNELS_SIZE_DIVIDER
                                                                     )
                                            )
                          );
        Ui::display.setCursor( TouchPad::touchData.cursorX - 32, TouchPad::touchData.cursorY - 8 );
        Ui::display.print(Channels::getFrequency(
                                            Channels::getOrderedIndex(
                                                                     (TouchPad::touchData.cursorX-18) / CHANNELS_SIZE_DIVIDER
                                                                     )
                                            )
                          );
    }

}

void HomeStateHandler::doTapAction() {

  if ( // Up band
      TouchPad::touchData.cursorX >= 0  && TouchPad::touchData.cursorX < 61 &&
      TouchPad::touchData.cursorY > 8 && TouchPad::touchData.cursorY < 54
     ) {
          this->setChannel(8);
        }
  else if ( // Down band
      TouchPad::touchData.cursorX >= 0  && TouchPad::touchData.cursorX < 61 &&
      TouchPad::touchData.cursorY > 54 && TouchPad::touchData.cursorY < 99
     ) {
          this->setChannel(-8);
        }
  else if ( // Up channel
      TouchPad::touchData.cursorX > 61  && TouchPad::touchData.cursorX < 122 &&
      TouchPad::touchData.cursorY > 8 && TouchPad::touchData.cursorY < 54
     ) {
          this->setChannel(1);
        }
  else if ( // Down channel
      TouchPad::touchData.cursorX > 61  && TouchPad::touchData.cursorX < 122 &&
      TouchPad::touchData.cursorY > 54 && TouchPad::touchData.cursorY < 99
     ) {
          this->setChannel(-1);
        }
  else if ( // Menu
      TouchPad::touchData.cursorX > 314  && TouchPad::touchData.cursorY < 8
     ) {
          EepromSettings.save();
          StateMachine::switchState(StateMachine::State::MENU);
        }
  else if ( // Change mode
      TouchPad::touchData.cursorX < 130 &&
      TouchPad::touchData.cursorY < 8
     ) {
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
          if (EepromSettings.quadversity) {
              switch ( EepromSettings.diversityMode )
              {
                  case Receiver::DiversityMode::ANTENNA_A:
                      EepromSettings.diversityMode = Receiver::DiversityMode::ANTENNA_B;
                      break;
                  case Receiver::DiversityMode::ANTENNA_B:
                      EepromSettings.diversityMode = Receiver::DiversityMode::ANTENNA_C;
                      break;
                  case Receiver::DiversityMode::ANTENNA_C:
                      EepromSettings.diversityMode = Receiver::DiversityMode::ANTENNA_D;
                      break;
                  case Receiver::DiversityMode::ANTENNA_D:
                      EepromSettings.diversityMode = Receiver::DiversityMode::DIVERSITY;
                      break;
                  case Receiver::DiversityMode::DIVERSITY:
                      EepromSettings.diversityMode = Receiver::DiversityMode::QUADVERSITY;
                      break;
                  case Receiver::DiversityMode::QUADVERSITY:
                      EepromSettings.diversityMode = Receiver::DiversityMode::ANTENNA_A;
                      break;
                  default:
                      break;
              }
          } else
#endif
          {
              switch ( EepromSettings.diversityMode )
              {
                  case Receiver::DiversityMode::ANTENNA_A:
                      EepromSettings.diversityMode = Receiver::DiversityMode::ANTENNA_B;
#if POWER_OFF_RX
                      ReceiverSpi::rxStandby(Receiver::ReceiverId::A);
                      ReceiverSpi::rxPowerOn(Receiver::ReceiverId::B);
#endif // POWER_OFF_RX
                      break;
                  case Receiver::DiversityMode::ANTENNA_B:
                      EepromSettings.diversityMode = Receiver::DiversityMode::DIVERSITY;
#if POWER_OFF_RX
                      ReceiverSpi::rxPowerOn(Receiver::ReceiverId::A);
#endif // POWER_OFF_RX
                      break;
                  case Receiver::DiversityMode::DIVERSITY:
                      EepromSettings.diversityMode = Receiver::DiversityMode::ANTENNA_A;
#if POWER_OFF_RX
                      ReceiverSpi::rxStandby(Receiver::ReceiverId::B);
#endif // POWER_OFF_RX
                      break;
                  default:
                      break;
              }
          }

          EepromSettings.markDirty();

        }
  else if ( // Select channel from spectrum
          HomeStateHandler::isInBandScanRegion()
        ) {
            setChannel(0, Channels::getOrderedIndex( (TouchPad::touchData.cursorX-18) / CHANNELS_SIZE_DIVIDER ));
#if 0
            Receiver::setChannel(
                                Channels::getOrderedIndex( (TouchPad::touchData.cursorX-18) / CHANNELS_SIZE_DIVIDER )
                                );
            HomeStateHandler::centreFrequency();
            displayActiveChannel = Receiver::activeChannel;

            EepromSettings.startChannel = displayActiveChannel;
            EepromSettings.markDirty();
#endif
        }
}

void HomeStateHandler::setChannel(int channelIncrement, int setChannel) {

    uint8_t activeChannel = (setChannel < 0) ? Receiver::activeChannel : setChannel;

    int band = activeChannel / 8;
    int channel = activeChannel % 8;

    if (channelIncrement == 8) {
      band = band + 1;
    } else if (channelIncrement == -8) {
      band = band - 1;
    } else if (channelIncrement == 1 ) {
      channel = channel + 1;
      if (channel > 7) {
        channel = 0;
      }
    } else if (channelIncrement == -1 ) {
      channel = channel - 1;
      if (channel < 0) {
        channel = 7;
      }
    }

    int newChannelIndex = band * 8 + channel;

    // Check wraparounds
    if (newChannelIndex >= CHANNELS_SIZE) {
      newChannelIndex = newChannelIndex - CHANNELS_SIZE;
    } else if (newChannelIndex < 0) {
      newChannelIndex = newChannelIndex + CHANNELS_SIZE;
    }

    Receiver::setChannel(newChannelIndex);
    EepromSettings.startChannel = newChannelIndex;
    EepromSettings.markDirty();
    centred = false;

    displayActiveChannel = Receiver::activeChannel;
}

// Frequency 'Centring' function.
// The function walks up and then down from the currently Rx frequency
// in 1 MHz steps until RSSI < threshold.  The Rx is then set to the
// centre of these 2 frequencies.
void HomeStateHandler::centreFrequency() {

  uint16_t activeChannelFreq = Channels::getFrequency(Receiver::activeChannel);
  uint16_t centerFreq = Channels::getCenterFreq(activeChannelFreq);
  Receiver::setChannel(Channels::getClosestChannel(centerFreq));

  wasInBandScanRegion = false;
}

bool HomeStateHandler::isInBandScanRegion() {
    if (TouchPad::touchData.cursorY > 130 ) {
        return true;
    } else {
        return false;
    }
}

void HomeStateHandler::bandScanUpdate() {

    Ui::UiTimeOut.reset();

    if (!wasInBandScanRegion) {
        orderedChanelIndex = Channels::getOrderedIndexFromIndex(displayActiveChannel); // Start from currently selected channel to prevent initial spike artifact.
    }

    if (Receiver::isRssiStableAndUpdated()) {

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
        if (EepromSettings.quadversity) {
            Receiver::rssiBandScanData[orderedChanelIndex] = max(Receiver::rssiA, max(Receiver::rssiB, max(Receiver::rssiC, Receiver::rssiD)));
        }
        else
#endif
        {
            Receiver::rssiBandScanData[orderedChanelIndex] = max(Receiver::rssiA, Receiver::rssiB);
        }

        orderedChanelIndex = orderedChanelIndex + 1;
        if (orderedChanelIndex == CHANNELS_SIZE) {
            orderedChanelIndex = 0;
        }
        Receiver::setChannel(Channels::getOrderedIndex(orderedChanelIndex));

    }

}
