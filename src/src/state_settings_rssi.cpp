#include <stdint.h>

#include "state_settings_rssi.h"

#include "receiver.h"
#include "channels.h"

#include "settings.h"
#include "settings_eeprom.h"

#include "ui.h"

#include "touchpad.h"

static Receiver::DiversityMode DRAM_ATTR start_mode;

void StateMachine::SettingsRssiStateHandler::onEnter()
{
    // Save mode and change to diversity
    start_mode = EepromSettings.diversityMode;
    EepromSettings.diversityMode = Receiver::DiversityMode::DIVERSITY;
    // Init internal state
    internalState = InternalState::WAIT_FOR_LOW;
    // Draw initial view
    //onUpdateDraw();
}

void StateMachine::SettingsRssiStateHandler::onExit()
{
    // restore mode
    EepromSettings.diversityMode = start_mode;
}

void StateMachine::SettingsRssiStateHandler::onUpdate()
{
    uint32_t rssiARaw = 0, rssiBRaw = 0, iter;

    onUpdateDraw();

    if (TouchPad::touchData.buttonPrimary && internalState != InternalState::SCANNING_LOW) {
        TouchPad::touchData.buttonPrimary = false;
        doTapAction();
    }

    if (!Receiver::isRssiStableAndUpdated())
        return;

    rssiARaw = Receiver::rssiARaw;
    rssiBRaw = Receiver::rssiBRaw;

    adcAttachPin(PIN_RSSI_A);
    for (iter = 0; iter < 100; iter++) {
        adcStart(PIN_RSSI_A);
        rssiARaw = (rssiARaw * 9) / 10;
        rssiARaw += adcEnd(PIN_RSSI_A) / 10;
    }

    adcAttachPin(PIN_RSSI_B);
    for (iter = 0; iter < 100; iter++) {
        adcStart(PIN_RSSI_B);
        rssiBRaw = (rssiBRaw * 9) / 10;
        rssiBRaw += adcEnd(PIN_RSSI_B) / 10;
    }

    switch (internalState) {
        case InternalState::SCANNING_LOW:
            if ( Channels::getFrequency(Receiver::activeChannel) >= 5658) { // Only use min max above R1 to stay within RX5808 freq range
                if (rssiARaw < EepromSettings.rssiAMin) {
                    EepromSettings.rssiAMin = rssiARaw;
                } else if (rssiARaw > EepromSettings.rssiAMax) {
                    EepromSettings.rssiAMax = rssiARaw;
                    bestChannel = Receiver::activeChannel;
                }

                if (rssiBRaw < EepromSettings.rssiBMin) {
                    EepromSettings.rssiBMin = rssiBRaw;
                } else if (rssiBRaw > EepromSettings.rssiBMax) {
                    EepromSettings.rssiBMax = rssiBRaw;
                }
            }
            break;
        default:
            break;
    }

    Receiver::setChannel((Receiver::activeChannel + 1) % CHANNELS_SIZE);

    if (internalState == InternalState::SCANNING_LOW || internalState == InternalState::SCANNING_HIGH) {

        Ui::display.setTextColor(WHITE);
        Ui::display.setCursor( 100, 80);
        Ui::display.printLarge(Channels::getName(Receiver::activeChannel), 6, 6);

        uint8_t progressBar = (Ui::XRES-100-2) * (currentSweep * CHANNELS_SIZE + Receiver::activeChannel) / (RSSI_SETUP_RUN * CHANNELS_SIZE);
        Ui::display.fillRect(52, 152, progressBar, 20, WHITE);

    }

    if (Receiver::activeChannel == 0) {
        currentSweep++;

        if (currentSweep == RSSI_SETUP_RUN && internalState == InternalState::SCANNING_LOW) {
            internalState = InternalState::DONE;
        }
    }
}


void StateMachine::SettingsRssiStateHandler::doTapAction()
{
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
            break;

        case InternalState::DONE:
            EepromSettings.isCalibrated = true;

            EepromSettings.save();

            Receiver::setChannel(
              Channels::getClosestChannel(
                Channels::getCenterFreq(
                  Channels::getFrequency(bestChannel))));

            StateMachine::switchState(StateMachine::State::HOME);
            break;

        default:
            break;
    }
}


void StateMachine::SettingsRssiStateHandler::onUpdateDraw()
{
    switch (internalState) {
        case InternalState::WAIT_FOR_LOW:

            Ui::display.setTextColor(WHITE);
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
            Ui::display.setTextColor(WHITE);
            Ui::display.setCursor( 40, 40);
            Ui::display.print("Scanning for lowest & highest");
            Ui::display.setCursor( 40, 50);
            Ui::display.print("RSSI...");
            // Progress bar outer rect
            Ui::display.rect(50, 150, Ui::XRES-100, 24, WHITE);
            break;

        case InternalState::DONE:
            Ui::display.setTextColor(WHITE);
            Ui::display.setCursor( 60, 40);
            Ui::display.print("All done!");

            //Ui::display.setCursor(0, Ui::CHAR_H * 2);

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
            Ui::display.setCursor( 60, 90);
            Ui::display.print("Tap to save.");
            break;

        default:
            break;
    }
}
