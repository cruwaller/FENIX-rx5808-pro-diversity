//#include <Arduino.h>
//
//#include "state_search.h"
//
//#include "settings.h"
//#include "settings_internal.h"
//#include "settings_eeprom.h"
//#include "receiver.h"
//#include "channels.h"
//#include "buttons.h"
//#include "ui.h"
//#include "pstr_helper.h"
//#include "bitmaps.h"
//
//
//using StateMachine::SearchStateHandler;
//
//
//static const unsigned char* menuModeIcon(void* state) {
//    SearchStateHandler* search = static_cast<SearchStateHandler*>(state);
//    return search->manual ? manualIcon : autoIcon;
//}
//
//static const unsigned char* menuOrderIcon(void* state) {
//    SearchStateHandler* search = static_cast<SearchStateHandler*>(state);
//    switch (search->order) {
//        case SearchStateHandler::ScanOrder::FREQUENCY:
//            return freqOrderIcon;
//            break;
//
//        case SearchStateHandler::ScanOrder::CHANNEL:
//            return channelOrderIcon;
//            break;
//    }
//}
//
//static void menuModeHandler(void* state) {
//    SearchStateHandler* search = static_cast<SearchStateHandler*>(state);
//    search->manual = !search->manual;
//
//    EepromSettings.searchManual = search->manual;
//    EepromSettings.markDirty();
//}
//
//static void menuOrderHandler(void* state) {
//    SearchStateHandler* search = static_cast<SearchStateHandler*>(state);
//    if (search->order == SearchStateHandler::ScanOrder::FREQUENCY) {
//        search->order = SearchStateHandler::ScanOrder::CHANNEL;
//        search->orderedChanelIndex =
//            Channels::getOrderedIndex(search->orderedChanelIndex);
//        EepromSettings.searchOrderByChannel = true;
//    } else {
//        search->order = SearchStateHandler::ScanOrder::FREQUENCY;
//        search->orderedChanelIndex =
//            Channels::getOrderedIndexFromIndex(search->orderedChanelIndex);
//        EepromSettings.searchOrderByChannel = false;
//    }
//
//    EepromSettings.markDirty();
//}
//
//
//void SearchStateHandler::onEnter() {
//    menu.addItem(menuModeIcon, menuModeHandler);
//    menu.addItem(menuOrderIcon, menuOrderHandler);
//
//    this->manual = EepromSettings.searchManual;
//    this->order = EepromSettings.searchOrderByChannel ?
//        ScanOrder::CHANNEL :
//        ScanOrder::FREQUENCY;
//
//    switch (this->order) {
//        case ScanOrder::CHANNEL:
//            this->orderedChanelIndex = EepromSettings.startChannel;
//            break;
//
//        case ScanOrder::FREQUENCY:
//            this->orderedChanelIndex =
//                Channels::getOrderedIndexFromIndex(EepromSettings.startChannel);
//            break;
//    }
//}
//
//void SearchStateHandler::onUpdate() {
//    if (!this->manual && scanning) {
//        onUpdateAuto();
//    }
//
//    Ui::needUpdate();
//}
//
//void SearchStateHandler::onUpdateAuto() {
//
//    if (direction == ScanDirection::SCANUP) {
//        orderedChanelIndex += 1;
//    } else {
//        orderedChanelIndex -= 1;
//    }
//        
//    if (orderedChanelIndex == 255)
//        orderedChanelIndex = CHANNELS_SIZE - 1;
//    else if (orderedChanelIndex >= CHANNELS_SIZE)
//        orderedChanelIndex = 0;
//  
//    this->setChannel();
//        
//    while (!Receiver::isRssiStable()) {
//      delay(1);
//    }
//    Receiver::updateRssi();
//    
//    if (!EepromSettings.quadversity) {
//        rssi = max(Receiver::rssiA, Receiver::rssiB);
//    }
//    if (EepromSettings.quadversity) {
//        rssi = max(Receiver::rssiA, max(Receiver::rssiB, max(Receiver::rssiC, Receiver::rssiD)));
//    }
//    
//    if (rssi < EepromSettings.rssiSeekTreshold && onPeak) {
//        onPeak = false;
//    } else if ( rssi >= EepromSettings.rssiSeekTreshold && !onPeak) {
//        scanning = false;
//  
//        if (this->order == ScanOrder::FREQUENCY) {
//          
//                orderedChanelIndex = Channels::getOrderedIndexFromIndex(
//                  Channels::getClosestChannel(
//                       Channels::getCenterFreq(
//                         Channels::getFrequency(
//                           Channels::getOrderedIndex(orderedChanelIndex)
//                ))));
//            
//        } else 
//        if (this->order == ScanOrder::CHANNEL) {
//          
//                orderedChanelIndex = Channels::getClosestChannel(
//                       Channels::getCenterFreq(
//                         Channels::getFrequency(orderedChanelIndex)
//                ));
//            
//        }
//    
//        this->setChannel();
//  
//        onPeak = true;
//        SearchStateHandler::onInitialDraw(); // Needed to correct UI after Centering graphic.
//        
//        EepromSettings.startChannel = orderedChanelIndex;
//        EepromSettings.markDirty();
//  
//    } 
//}
//
//void SearchStateHandler::onButtonChange(
//    Button button,
//    Buttons::PressType pressType
//) {
//    if (this->menu.handleButtons(button, pressType))
//        return;
//        
//    if (!this->manual) {
//        if (
//            pressType == Buttons::PressType::SHORT &&
//            button == Button::MODE_PRESSED
//           ) {
//
//                if (this->order == ScanOrder::FREQUENCY) {
//                  
//                        orderedChanelIndex = Channels::getOrderedIndexFromIndex(
//                          Channels::getClosestChannel(
//                               Channels::getCenterFreq(
//                                 Channels::getFrequency(
//                                   Channels::getOrderedIndex(orderedChanelIndex)
//                        ))));
//                    
//                } else 
//                if (this->order == ScanOrder::CHANNEL) {
//                  
//                        orderedChanelIndex = Channels::getClosestChannel(
//                               Channels::getCenterFreq(
//                                 Channels::getFrequency(orderedChanelIndex)
//                        ));
//                    
//                }
//
//                this->setChannel();
//                
//                SearchStateHandler::onInitialDraw(); // Needed to correct UI after Centering graphic.
//              }
//        else if (
//            pressType == Buttons::PressType::SHORT &&
//            button == Button::UP_PRESSED
//           ) {
//                scanning = true;
//                direction = ScanDirection::SCANUP;
//              }
//        else if (
//            pressType == Buttons::PressType::SHORT &&
//            button == Button::DOWN_PRESSED
//           ) {
//                scanning = true;
//                direction = ScanDirection::SCANDOWN;
//              }
//        else if (
//            pressType == Buttons::PressType::LONG &&
//            button == Button::MODE_PRESSED
//           ) {
//              }   
//        else {
//          return;   
//        }
//        
//    } else {
//        if (
//            pressType == Buttons::PressType::SHORT &&
//            button == Button::MODE_PRESSED
//           ) {
//
//                if (this->order == ScanOrder::FREQUENCY) {
//                  
//                        orderedChanelIndex = Channels::getOrderedIndexFromIndex(
//                          Channels::getClosestChannel(
//                               Channels::getCenterFreq(
//                                 Channels::getFrequency(
//                                   Channels::getOrderedIndex(orderedChanelIndex)
//                        ))));
//                    
//                } else 
//                if (this->order == ScanOrder::CHANNEL) {
//                  
//                        orderedChanelIndex = Channels::getClosestChannel(
//                               Channels::getCenterFreq(
//                                 Channels::getFrequency(orderedChanelIndex)
//                        ));
//                    
//                }
//
//                this->setChannel();
//                
//                SearchStateHandler::onInitialDraw(); // Needed to correct UI after Centering graphic.
//              }
//        else if (
//            pressType == Buttons::PressType::SHORT &&
//            button == Button::UP_PRESSED
//           ) {
//              orderedChanelIndex += 1;
//              if (orderedChanelIndex >= CHANNELS_SIZE)
//                orderedChanelIndex = 0;
//              this->setChannel();
//              EepromSettings.startChannel = orderedChanelIndex;
//              EepromSettings.markDirty();
//              }
//        else if (
//            pressType == Buttons::PressType::SHORT &&
//            button == Button::DOWN_PRESSED
//           ) {
//              orderedChanelIndex -= 1;
//              if (orderedChanelIndex == 255)
//                orderedChanelIndex = CHANNELS_SIZE - 1;
//              this->setChannel();
//              EepromSettings.startChannel = orderedChanelIndex;
//              EepromSettings.markDirty();
//              }
//        else if (
//            pressType == Buttons::PressType::LONG &&
//            button == Button::MODE_PRESSED
//           ) {
//              }    
//        else {
//          return;   
//        }    
//    }
//}
//
//void SearchStateHandler::setChannel() {
//    uint8_t actualChannelIndex;
//    if (this->order == ScanOrder::FREQUENCY) {
//        actualChannelIndex = Channels::getOrderedIndex(orderedChanelIndex);
//    } else {
//        actualChannelIndex = orderedChanelIndex;
//    }
//
//    Receiver::setChannel(actualChannelIndex);
//}
