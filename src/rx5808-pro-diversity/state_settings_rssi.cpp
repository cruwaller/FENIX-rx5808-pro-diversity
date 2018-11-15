#include <stdint.h>

#include "state_settings_rssi.h"

#include "receiver.h"
#include "channels.h"

#include "settings.h"
#include "settings_internal.h"
#include "settings_eeprom.h"

#include "ui.h"
#include "pstr_helper.h"

#include "touchpad.h"
    
void StateMachine::SettingsRssiStateHandler::onEnter() {
    internalState = InternalState::WAIT_FOR_LOW;
}

void StateMachine::SettingsRssiStateHandler::onUpdate() {

//    Ui::UiTimeOut.reset();
    onUpdateDraw();
    
    if (TouchPad::touchData.buttonPrimary && internalState!=InternalState::SCANNING_LOW) {
      doTapAction();
    }
  
    if (!Receiver::isRssiStable())
        return;

    switch (internalState) {
        case InternalState::SCANNING_LOW:
            if (Receiver::rssiARaw < EepromSettings.rssiAMin)
                EepromSettings.rssiAMin = Receiver::rssiARaw;

            if (Receiver::rssiARaw > EepromSettings.rssiAMax) {
                EepromSettings.rssiAMax = Receiver::rssiARaw;
                bestChannel = Receiver::activeChannel;
            }
            if (Receiver::rssiBRaw < EepromSettings.rssiBMin)
                EepromSettings.rssiBMin = Receiver::rssiBRaw;
                
            if (Receiver::rssiBRaw > EepromSettings.rssiBMax)
                EepromSettings.rssiBMax = Receiver::rssiBRaw;

            if (EepromSettings.quadversity) {
                if (Receiver::rssiCRaw < EepromSettings.rssiCMin)
                    EepromSettings.rssiCMin = Receiver::rssiCRaw;
                if (Receiver::rssiCRaw > EepromSettings.rssiCMax)
                    EepromSettings.rssiCMax = Receiver::rssiCRaw;
                    
                if (Receiver::rssiDRaw < EepromSettings.rssiDMin)
                    EepromSettings.rssiDMin = Receiver::rssiDRaw;
                if (Receiver::rssiDRaw > EepromSettings.rssiDMax)
                    EepromSettings.rssiDMax = Receiver::rssiDRaw;
            }
        break;
    }

    Receiver::setChannel((Receiver::activeChannel + 1) % CHANNELS_SIZE);
    
    if (internalState==InternalState::SCANNING_LOW || internalState==InternalState::SCANNING_HIGH) {
//        Ui::setTextSize(1);
//        Ui::setTextColor(WHITE);
//        Ui::drawBigCharacter( 120, 55, 
//                              Channels::getName(Receiver::activeChannel)[0], 
//                              11, 10);
//        Ui::drawBigCharacter( 200, 55, 
//                              Channels::getName(Receiver::activeChannel)[1], 
//                              11, 10);     
//        Ui::needDisplay(); 
//        Ui::update();      
    }
          
    if (Receiver::activeChannel == 0) {
        currentSweep++;

        if (currentSweep == RSSI_SETUP_RUN && internalState == InternalState::SCANNING_LOW) {
            internalState = InternalState::DONE;
//            Ui::needUpdate();
        }
    }
}

void StateMachine::SettingsRssiStateHandler::doTapAction() {

    switch (internalState) {
        case InternalState::WAIT_FOR_LOW:
            internalState = InternalState::SCANNING_LOW;
            currentSweep = 0;
            Receiver::setChannel(0);
            bestChannel = 0;

            EepromSettings.rssiAMin = UINT16_MAX;
            EepromSettings.rssiAMax = 0;
            EepromSettings.rssiBMin = UINT16_MAX;
            EepromSettings.rssiBMax = 0;
            EepromSettings.rssiCMin = UINT16_MAX;
            EepromSettings.rssiCMax = 0;
            EepromSettings.rssiDMin = UINT16_MAX;
            EepromSettings.rssiDMax = 0;
        break;

        case InternalState::DONE:
            EepromSettings.isCalibrated = true;
            
            EepromSettings.save();

            Receiver::setChannel(
              Channels::getClosestChannel(
                Channels::getCenterFreq(
                  Channels::getFrequency(bestChannel))));
            
            EepromSettings.lastKnownMenuItem = 0;
            EepromSettings.markDirty();

            StateMachine::switchState(StateMachine::State::HOME);
            
        break;
    }

//    Ui::needUpdate();
}


void StateMachine::SettingsRssiStateHandler::onInitialDraw() {
//    Ui::needUpdate(); // Lazy. :(
    onUpdateDraw(); // Lazier. :(
}

void StateMachine::SettingsRssiStateHandler::onUpdateDraw() {
//    Ui::clear();

    switch (internalState) {
      case InternalState::WAIT_FOR_LOW:
    
//          Ui::setTextSize(1);
//          Ui::setTextColor(WHITE);
//          Ui::setCursor( 80, 40);
//          Ui::display.println(PSTR2("Your module is not calibrated."));
//          Ui::setCursor( 80, 50);
//          Ui::display.println(PSTR2("Follow the below steps."));
//          Ui::setCursor( 80, 70);
//          Ui::display.println(PSTR2("- Turn on a VTx at 25mW & place 1m away."));
//          Ui::setCursor( 80, 80);
//          Ui::display.println(PSTR2("- Remove Rx antennas."));
//          Ui::setCursor( 80, 100);
//          Ui::display.println(PSTR2("Press MODE when ready."));
      break;

      case InternalState::SCANNING_LOW:
//          Ui::setTextSize(1);
//          Ui::setTextColor(WHITE);
//          Ui::setCursor( 80, 40);
//          Ui::display.println(PSTR2("Scanning for lowest & highest RSSI..."));
      break;

      case InternalState::DONE:
//          Ui::setTextSize(1);
//          Ui::setTextColor(WHITE);
//          Ui::setCursor( 80, 40);
//          Ui::display.print(PSTR2("All done!"));
//
//          Ui::setCursor(0, CHAR_HEIGHT * 2);
//          
//          Ui::setCursor( 80, 60);
//          Ui::display.print(PSTR2("A: "));
//          Ui::display.print(EepromSettings.rssiAMin);
//          Ui::display.print(PSTR2(" -> "));
//          Ui::display.println(EepromSettings.rssiAMax);
//          Ui::setCursor( 80, 70);
//          Ui::display.print(PSTR2("B: "));
//          Ui::display.print(EepromSettings.rssiBMin);
//          Ui::display.print(PSTR2(" -> "));
//          Ui::display.println(EepromSettings.rssiBMax);
          if (EepromSettings.quadversity) {
//              Ui::setCursor( 80, 80);
//              Ui::display.print(PSTR2("C: "));
//              Ui::display.print(EepromSettings.rssiCMin);
//              Ui::display.print(PSTR2(" -> "));
//              Ui::display.println(EepromSettings.rssiCMax);
//              Ui::setCursor( 80, 90);
//              Ui::display.print(PSTR2("D: "));
//              Ui::display.print(EepromSettings.rssiDMin);
//              Ui::display.print(PSTR2(" -> "));
//              Ui::display.println(EepromSettings.rssiDMax);
          }

//          Ui::setCursor( 80, 110);
//          Ui::display.print(PSTR2("Press MODE to save."));
      break;
    }

//    Ui::needDisplay();
}
