////#include <avr/pgmspace.h>
//
//#include "settings_eeprom.h"
//
//#include "state_spectator.h"
//
//#include "receiver.h"
//#include "channels.h"
//#include "buttons.h"
//#include "state.h"
//#include "ui.h"
//#include "pstr_helper.h"
//
//
//using StateMachine::SpectatorStateHandler;
//
//
//uint8_t selectedChannel = 0;
//
//void SpectatorStateHandler::onEnter() {
//  if (EepromSettings.spectatorChannels[0] != -1) {
//      Receiver::setChannel(EepromSettings.spectatorChannels[selectedChannel]);
//  }
//  this->onUpdateDraw();
//}
//
//void SpectatorStateHandler::onUpdate() {
//  this->onUpdateDraw();
//}
//
//void SpectatorStateHandler::onInitialDraw() {  
//}
//
//void SpectatorStateHandler::onUpdateDraw() {
//
//    if (EepromSettings.spectatorChannels[0] == -1) {
//        Ui::clear();
//        Ui::setTextSize(1);
//        Ui::setCursor(12, 15);
//        Ui::display.print(PSTR2("No channels stored.")); 
//        Ui::setCursor(22, 30);
//        Ui::display.print(PSTR2("Long press Mode")); 
//        Ui::setCursor(29, 45);
//        Ui::display.print(PSTR2("to search.")); 
//    } else {        
//        Ui::clear();
//        Ui::setTextSize(1);
//        
//        for (int i=0; i<8; i++) {
//          if (EepromSettings.spectatorChannels[i] > -1) {
//            Ui::setCursor(20+22*(i%4), 17+20*(i/4));
//            Ui::display.print(Channels::getName(EepromSettings.spectatorChannels[i]));
//              if (i == selectedChannel) {
//                Ui::drawRoundRect(20+22*(i%4)-2, 17+20*(i/4)-2, 15, 11, 2, WHITE);
//              }
//           }
//        }      
//    }    
//        
//    Ui::needDisplay(); 
//
//}
//
//void SpectatorStateHandler::onExit() {
//    
//}
//
//void SpectatorStateHandler::onButtonChange(
//    Button button,
//    Buttons::PressType pressType
//) {
//  
//  if (
//      pressType == Buttons::PressType::SHORT &&
//      button == Button::MODE_PRESSED
//     ) {
//          EepromSettings.spectatorChannels[selectedChannel] = Channels::getClosestChannel(
//                                             Channels::getCenterFreq(
//                                               Channels::getFrequency(
//                                                 EepromSettings.spectatorChannels[selectedChannel]
//                                           )));
//          Receiver::setChannel(EepromSettings.spectatorChannels[selectedChannel]);
//        }
//  else if (
//      pressType == Buttons::PressType::LONG &&
//      button == Button::MODE_PRESSED
//     ) {
//          this->findChannels();
//        }
//  else if (
//      pressType == Buttons::PressType::SHORT &&
//      button == Button::UP_PRESSED
//     ) {
//          selectedChannel--;
//          if ( selectedChannel == 255) {
//            selectedChannel = 7;
//            while (EepromSettings.spectatorChannels[selectedChannel] == -1) {
//              selectedChannel--;
//            }
//          }
//          if (EepromSettings.spectatorChannels[selectedChannel] > -1) {
//            Receiver::setChannel(EepromSettings.spectatorChannels[selectedChannel]);
//          }
//        }
//  else if (
//      pressType == Buttons::PressType::SHORT &&
//      button == Button::DOWN_PRESSED
//     ) {
//          selectedChannel++;
//          if ( selectedChannel > 7 || EepromSettings.spectatorChannels[selectedChannel] == -1 ) {
//            selectedChannel = 0;
//          }
//          if (EepromSettings.spectatorChannels[selectedChannel] > -1) {
//            Receiver::setChannel(EepromSettings.spectatorChannels[selectedChannel]);
//          }
//        }
//}
//
//void SpectatorStateHandler::findChannels() {
//  
//    EepromSettings.spectatorChannels[0] = -1;
//    EepromSettings.spectatorChannels[1] = -1;
//    EepromSettings.spectatorChannels[2] = -1;
//    EepromSettings.spectatorChannels[3] = -1;
//    EepromSettings.spectatorChannels[4] = -1;
//    EepromSettings.spectatorChannels[5] = -1;
//    EepromSettings.spectatorChannels[6] = -1;
//    EepromSettings.spectatorChannels[7] = -1;
//    
//    selectedChannel = 0;
//    
//    uint8_t progressBarLength;
//    uint8_t rssi;
//    bool upSideFound = false;
//    uint16_t upSideFoundFreq;
//    uint16_t foundFreq;
//    uint8_t numFound = 0;
//    uint16_t scanLowerFreq = Channels::getFrequency(Channels::getOrderedIndex(0)) - 40;
//    uint16_t scanUpperFreq = Channels::getFrequency(Channels::getOrderedIndex(CHANNELS_SIZE - 1)) + 40;
//    uint8_t scanStepSize = EepromSettings.spectatorFreqScanStep;
//    uint8_t minPeakFWHM = EepromSettings.spectatorFWHM;
//
//    for (uint16_t i=scanLowerFreq; i<scanUpperFreq; i += scanStepSize) {
//      
//      Ui::clear();
//      Ui::setTextSize(1);
//      Ui::setCursor(0, 0);
//      Ui::display.print(PSTR2("Scanning...")); 
//      Ui::drawRoundRect(0, 53, 128, 10, 2, WHITE);
//    
//      Ui::setTextSize(2);
//      Ui::clearRect(16, 22, 48, 20);
//      Ui::setCursor(16, 22);
//      Ui::display.print(i);
//      Ui::display.print(PSTR2(" MHz")); 
//
//      progressBarLength = 126 * (i - scanLowerFreq) / (scanUpperFreq - scanLowerFreq);
//      Ui::fillRect(2, 55, progressBarLength, 6, WHITE);
//
//      Receiver::setChannelByFreq(i);
//      while (!Receiver::isRssiStable()) {
//        delay(1);
//      }
//      Receiver::updateRssi();
//
//      //  Testing required to see if average or max is best.
//      if (!EepromSettings.quadversity) {
////        rssi = (Receiver::rssiA + Receiver::rssiB) / 2;
//        rssi = max(Receiver::rssiA, Receiver::rssiB);
//      }
//      if (EepromSettings.quadversity) {
////        rssi = ( ( Receiver::rssiA + Receiver::rssiB ) / 2  + ( Receiver::rssiC + Receiver::rssiD ) / 2 ) / 2;
//        rssi = max( max( max(Receiver::rssiA, Receiver::rssiB), Receiver::rssiC), Receiver::rssiD);
//      }
//
//      Ui::setTextSize(1);
//      Ui::setCursor(51, 42);
//      Ui::display.print(rssi);
//      Ui::setCursor(76, 42);
//      Ui::display.print(PSTR2("RSSI")); 
//     
//      if (rssi > EepromSettings.rssiSeekTreshold && !upSideFound) {
//          upSideFound = true;
//          upSideFoundFreq = i - scanStepSize;
//      }
//      
//      if (rssi < EepromSettings.rssiSeekTreshold && upSideFound && numFound < 8) {
//        
//          if (i - upSideFoundFreq > minPeakFWHM) { // Peak must have minimum FWHM to help rule out noisy peaks.
//            foundFreq = ( upSideFoundFreq + i ) / 2;
//            
//            EepromSettings.spectatorChannels[numFound] = Channels::getClosestChannel(Channels::getCenterFreq(foundFreq));
//            
//            numFound++;            
//          }
//          
//          upSideFound = false;
//      }
//    
//      Ui::needDisplay(); 
//      Ui::update();
//    }
//
//    if (EepromSettings.spectatorChannels[0] > -1) {
//      Receiver::setChannel(EepromSettings.spectatorChannels[0]);
//    }
//
//    EepromSettings.markDirty();
//    
//    this->onUpdateDraw();
//}

