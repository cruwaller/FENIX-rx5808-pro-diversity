#include <stdint.h>

#include "state_settings_rssi.h"

#include "receiver.h"
#include "channels.h"

#include "settings.h"
#include "settings_eeprom.h"

#include "ui.h"

#include "touchpad.h"
    
void StateMachine::SettingsRssiStateHandler::onEnter() {
    internalState = InternalState::WAIT_FOR_LOW;
}

void StateMachine::SettingsRssiStateHandler::onUpdate() {

    onUpdateDraw();
    
    if (TouchPad::touchData.buttonPrimary && internalState!=InternalState::SCANNING_LOW) {
      TouchPad::touchData.buttonPrimary = false;
      doTapAction();
    }
  
    if (!Receiver::isRssiStable() || !Receiver::hasRssiUpdated)
        return;

    for (int i = 0; i < 100; i++) {
        Receiver::rssiARaw = 0.9 * Receiver::rssiARaw + 0.1 * analogRead(PIN_RSSI_A);
        Receiver::rssiBRaw = 0.9 * Receiver::rssiBRaw + 0.1 * analogRead(PIN_RSSI_B);
    }

    switch (internalState) {
        case InternalState::SCANNING_LOW:
            if ( Channels::getFrequency(Receiver::activeChannel) >= 5658) { // Only use min max above R1 to stay within RX5808 freq range
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
            }
        break;
    }

    Receiver::setChannel((Receiver::activeChannel + 1) % CHANNELS_SIZE);
    
    if (internalState==InternalState::SCANNING_LOW || internalState==InternalState::SCANNING_HIGH) {

        Ui::display.setTextColor(100);
        Ui::display.setCursor( 100, 80);
        Ui::display.printLarge(Channels::getName(Receiver::activeChannel), 6, 6);

        uint8_t progressBar = (SCREEN_WIDTH-100-2) * (currentSweep * CHANNELS_SIZE + Receiver::activeChannel) / (RSSI_SETUP_RUN * CHANNELS_SIZE);
        Ui::display.fillRect(52, 152, progressBar, 20, 100);
  
    }
          
    if (Receiver::activeChannel == 0) {
        currentSweep++;

        if (currentSweep == RSSI_SETUP_RUN && internalState == InternalState::SCANNING_LOW) {
            internalState = InternalState::DONE;
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

}


void StateMachine::SettingsRssiStateHandler::onInitialDraw() {
//    Ui::needUpdate(); // Lazy. :(
    onUpdateDraw(); // Lazier. :(
}

void StateMachine::SettingsRssiStateHandler::onUpdateDraw() {

    switch (internalState) {
      case InternalState::WAIT_FOR_LOW:
    
          Ui::display.setTextColor(100);
          Ui::display.setCursor( 40, 40);
          Ui::display.print("Your module is not calibrated.");
          Ui::display.setCursor( 40, 50);
          Ui::display.print("Follow the below steps.");
          Ui::display.setCursor( 40, 70);
          Ui::display.print("- Turn on a VTx at 25mW & place");
          Ui::display.setCursor( 40, 80);
          Ui::display.print("  1m away.");
          Ui::display.setCursor( 40, 90);
          Ui::display.print("- Remove Rx antennas.");
          Ui::display.setCursor( 40, 110);
          Ui::display.print("Tap to continue.");
      break;

      case InternalState::SCANNING_LOW:
          Ui::display.setTextColor(100);
          Ui::display.setCursor( 40, 40);
          Ui::display.print("Scanning for lowest & highest");
          Ui::display.setCursor( 40, 50);
          Ui::display.print("RSSI...");
          // Progress bar outer rect
          Ui::display.rect(50, 150, SCREEN_WIDTH-100, 24, 100);
      break;

      case InternalState::DONE:
          Ui::display.setTextColor(100);
          Ui::display.setCursor( 60, 40);
          Ui::display.print("All done!");

          Ui::display.setCursor(0, CHAR_HEIGHT * 2);
          
          Ui::display.setCursor( 60, 60);
          Ui::display.print("A: ");
          Ui::display.print(EepromSettings.rssiAMin);
          Ui::display.print(" -> ");
          Ui::display.print(EepromSettings.rssiAMax);
          Ui::display.setCursor( 60, 70);
          Ui::display.print("B: ");
          Ui::display.print(EepromSettings.rssiBMin);
          Ui::display.print(" -> ");
          Ui::display.print(EepromSettings.rssiBMax);
          if (EepromSettings.quadversity) {
              Ui::display.setCursor( 60, 80);
              Ui::display.print("C: ");
              Ui::display.print(EepromSettings.rssiCMin);
              Ui::display.print(" -> ");
              Ui::display.print(EepromSettings.rssiCMax);
              Ui::display.setCursor( 60, 90);
              Ui::display.print("D: ");
              Ui::display.print(EepromSettings.rssiDMin);
              Ui::display.print(" -> ");
              Ui::display.print(EepromSettings.rssiDMax);
          }

          Ui::display.setCursor( 60, 90);
          Ui::display.print("Tap to save.");
      break;
    }
}
