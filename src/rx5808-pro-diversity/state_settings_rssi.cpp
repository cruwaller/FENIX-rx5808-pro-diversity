#include <stdint.h>

#include "state_settings_rssi.h"

#include "receiver.h"
#include "channels.h"

#include "settings.h"
#include "settings_internal.h"
#include "settings_eeprom.h"
#include "buttons.h"

#include "ui.h"
#include "pstr_helper.h"


void StateMachine::SettingsRssiStateHandler::onEnter() {
    internalState = InternalState::WAIT_FOR_LOW;
}

void StateMachine::SettingsRssiStateHandler::onUpdate() {
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
        Ui::setTextSize(2);
        Ui::clearRect(52, 32, 24, 20);
        Ui::setCursor(52, 32);
        Ui::display.print(Channels::getName(Receiver::activeChannel));       
        Ui::needDisplay(); 
        Ui::update();      
        Ui::clearRect(52, 32, 24, 20);
    }
          
    if (Receiver::activeChannel == 0) {
        currentSweep++;

        if (currentSweep == RSSI_SETUP_RUN && internalState == InternalState::SCANNING_LOW) {
            internalState = InternalState::DONE;
            Ui::needUpdate();
        }
    }
}

void StateMachine::SettingsRssiStateHandler::onButtonChange(
    Button button,
    Buttons::PressType pressType
) {
    if (button != Button::MODE_PRESSED || pressType != Buttons::PressType::SHORT)
        return;

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

            switch(EepromSettings.selectedHomePage) {
                case 0:
                  StateMachine::switchState(StateMachine::State::HOME);
                break;
                case 1:
                  StateMachine::switchState(StateMachine::State::HOME_SIMPLE);
                break;
                case 2:
                  StateMachine::switchState(StateMachine::State::HOME_STATS);
                break;
            }  
            
        break;
    }

    Ui::needUpdate();
}


void StateMachine::SettingsRssiStateHandler::onInitialDraw() {
//    Ui::needUpdate(); // Lazy. :(
    onUpdateDraw(); // Lazier. :(
}

void StateMachine::SettingsRssiStateHandler::onUpdateDraw() {
    Ui::clear();

    switch (internalState) {
      case InternalState::WAIT_FOR_LOW:
          Ui::setTextSize(1);
          Ui::setCursor(0, 0);
          Ui::display.println(PSTR2("- Turn on VTx & place"));
          Ui::display.println(PSTR2("  1m away."));
          Ui::display.println(PSTR2(" "));
          Ui::display.println(PSTR2("- Remove Rx antennas."));

          Ui::setCursor(0, SCREEN_HEIGHT - CHAR_HEIGHT - 1);
          Ui::display.print(PSTR2("Press MODE when ready."));
      break;

      case InternalState::SCANNING_LOW:
          Ui::setTextSize(1);
          Ui::setCursor(0, 0);
          Ui::display.println(PSTR2("Scanning for lowest &"));
          Ui::display.println(PSTR2("highest RSSI..."));
      break;

      case InternalState::DONE:
          Ui::setTextSize(1);
          Ui::setCursor(0, 0);
          Ui::display.print(PSTR2("All done!"));

          Ui::setCursor(0, CHAR_HEIGHT * 2);
          
          Ui::display.print(PSTR2("A: "));
          Ui::display.print(EepromSettings.rssiAMin);
          Ui::display.print(PSTR2(" -> "));
          Ui::display.println(EepromSettings.rssiAMax);
          Ui::display.print(PSTR2("B: "));
          Ui::display.print(EepromSettings.rssiBMin);
          Ui::display.print(PSTR2(" -> "));
          Ui::display.println(EepromSettings.rssiBMax);
          if (EepromSettings.quadversity) {
              Ui::display.print(PSTR2("C: "));
              Ui::display.print(EepromSettings.rssiCMin);
              Ui::display.print(PSTR2(" -> "));
              Ui::display.println(EepromSettings.rssiCMax);
              Ui::display.print(PSTR2("D: "));
              Ui::display.print(EepromSettings.rssiDMin);
              Ui::display.print(PSTR2(" -> "));
              Ui::display.println(EepromSettings.rssiDMax);
          }

          Ui::setCursor(0, SCREEN_HEIGHT - CHAR_HEIGHT - 1);
          Ui::display.print(PSTR2("Press MODE to save."));
      break;
    }

    Ui::needDisplay();
}
