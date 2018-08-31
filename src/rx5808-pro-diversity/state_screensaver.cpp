//#include <avr/pgmspace.h>

#include "state_screensaver.h"

#include "receiver.h"
#include "channels.h"
#include "buttons.h"
#include "state.h"
#include "ui.h"
#include "bitmaps.h"
#include "settings_eeprom.h"


void StateMachine::ScreensaverStateHandler::onEnter() {
    showLogo = true;
}

void StateMachine::ScreensaverStateHandler::onUpdate() {
    if (this->displaySwapTimer.hasTicked()) {
        this->displaySwapTimer.reset();
        showLogo = !showLogo;

        Ui::needUpdate();
    }
}


void StateMachine::ScreensaverStateHandler::onButtonChange(
    Button button,
    Buttons::PressType pressType
) {
    StateMachine::switchState(StateMachine::lastState);
}


void StateMachine::ScreensaverStateHandler::onInitialDraw() {
    Ui::clear();

    if (showLogo) {  
      switch (EepromSettings.saveScreenOn) {
        case 1:
            Ui::drawBitmap(
                0,
                0,
                logo,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                WHITE
            );
          break;
        case 2:
            Ui::drawBitmap(
                0,
                0,
                logoBetaflight,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                WHITE
            );
          break;
        case 3:
            Ui::drawBitmap(
                0,
                0,
                logoRotorRiot,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                WHITE
            );
          break;
        case 4:
            Ui::drawBitmap(
                0,
                0,
                EepromSettings.customLogo,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                WHITE
            );
          break;    
      }
      
    } else {
      Ui::setTextColor(WHITE);

      Ui::setTextSize(6);
      Ui::setCursor(
          SCREEN_WIDTH_MID - ((CHAR_WIDTH) * 6) / 2 * 2 - 3,
          2);

      Ui::display.print(Channels::getName(Receiver::activeChannel));

      Ui::setTextSize(2);
      Ui::setCursor(
          SCREEN_WIDTH_MID - ((CHAR_WIDTH + 1) * 2) / 2 * 4 - 1,
          SCREEN_HEIGHT - CHAR_HEIGHT * 2 - 2);
      Ui::display.print(Channels::getFrequency(Receiver::activeChannel));
    }

    Ui::needDisplay();
}

void StateMachine::ScreensaverStateHandler::onUpdateDraw() {
    this->onInitialDraw();
}
