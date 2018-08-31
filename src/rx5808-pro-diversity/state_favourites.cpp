#include <Arduino.h>
#include <avr/pgmspace.h>

#include "settings_eeprom.h"

#include "state_favourites.h"

#include "receiver.h"
#include "channels.h"
#include "buttons.h"
#include "state.h"
#include "ui.h"
#include "pstr_helper.h"
#include "voltage.h"


using StateMachine::FavouritesStateHandler;

uint8_t favouritesSelectedChannel = 0;

bool changeingChannel = false;
            
void FavouritesStateHandler::onEnter() {
  changeingChannel = false;
  Receiver::setChannel(EepromSettings.favouriteChannels[favouritesSelectedChannel]);
  
  this->onUpdateDraw();
}

void FavouritesStateHandler::onUpdate() {

    if ( !changeingChannel ) {
        Ui::clear();
        Ui::setTextSize(1);
        
        for (int i=0; i<8; i++) {
          Ui::setCursor(20+22*(i%4), 17+20*(i/4));
          Ui::display.print(Channels::getName(EepromSettings.favouriteChannels[i]));
            if (i == favouritesSelectedChannel) {
              Ui::drawRoundRect(20+22*(i%4)-2, 17+20*(i/4)-2, 15, 11, 2, WHITE);
            }
        }    
            
        Ui::needDisplay(); 
    }
    if ( changeingChannel ) {
        Ui::setTextSize(1);
        
        int i = favouritesSelectedChannel;
        Ui::setCursor(20+22*(i%4), 17+20*(i/4));
        
        Ui::fillRect(20+22*(i%4)-2, 17+20*(i/4)-2, 15, 11, WHITE);
            
        Ui::display.setTextColor(BLACK);
        Ui::display.print(Channels::getName(EepromSettings.favouriteChannels[i]));
        Ui::display.setTextColor(WHITE);
        
        Ui::needDisplay(); 

    }
}

void FavouritesStateHandler::onInitialDraw() {
    this->onUpdateDraw();
}

void FavouritesStateHandler::onUpdateDraw() {

    Ui::needDisplay();
}

void FavouritesStateHandler::onButtonChange(
    Button button,
    Buttons::PressType pressType
) {
  
  if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::MODE_PRESSED
     ) {
          if ( !changeingChannel ) {
            Receiver::setChannelByFreq(
              Channels::getCenterFreq(
                Channels::getFrequency(
                  EepromSettings.favouriteChannels[favouritesSelectedChannel]
                )
              )
            );
          } 
          if ( changeingChannel ) {
              EepromSettings.favouriteChannels[favouritesSelectedChannel] += 8;
              if ( EepromSettings.favouriteChannels[favouritesSelectedChannel] > CHANNELS_SIZE - 1 ) {
                EepromSettings.favouriteChannels[favouritesSelectedChannel] -= CHANNELS_SIZE;
              }
              Receiver::setChannel(EepromSettings.favouriteChannels[favouritesSelectedChannel]);
              EepromSettings.markDirty();
          }
        }
  else if (
      pressType == Buttons::PressType::LONG &&
      button == Button::MODE_PRESSED
     ) {
          changeingChannel = !changeingChannel;
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::UP_PRESSED
     ) {          
          if ( !changeingChannel ) {
              favouritesSelectedChannel--;
              if ( favouritesSelectedChannel == 255) {
                favouritesSelectedChannel = 7;
              }
              Receiver::setChannel(EepromSettings.favouriteChannels[favouritesSelectedChannel]);
              EepromSettings.markDirty();
          }       
          if ( changeingChannel ) {
              EepromSettings.favouriteChannels[favouritesSelectedChannel]--;
              if ( EepromSettings.favouriteChannels[favouritesSelectedChannel] == 255) {
                EepromSettings.favouriteChannels[favouritesSelectedChannel] = CHANNELS_SIZE - 1;
              }
              Receiver::setChannel(EepromSettings.favouriteChannels[favouritesSelectedChannel]);
              EepromSettings.markDirty();
          }
        }
  else if (
      pressType == Buttons::PressType::SHORT &&
      button == Button::DOWN_PRESSED
     ) {          
          if ( !changeingChannel ) {
              favouritesSelectedChannel++;
              if ( favouritesSelectedChannel > 7 ) {
                favouritesSelectedChannel = 0;
              }
              Receiver::setChannel(EepromSettings.favouriteChannels[favouritesSelectedChannel]);
              EepromSettings.markDirty();
          }      
          if ( changeingChannel ) {
              EepromSettings.favouriteChannels[favouritesSelectedChannel]++;
              if ( EepromSettings.favouriteChannels[favouritesSelectedChannel] > CHANNELS_SIZE - 1 ) {
                EepromSettings.favouriteChannels[favouritesSelectedChannel] = 0;
              }
              Receiver::setChannel(EepromSettings.favouriteChannels[favouritesSelectedChannel]);
              EepromSettings.markDirty();
          }
        }
}

