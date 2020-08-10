#include <stddef.h>

#include "state.h"

#include "state_home.h"
#include "state_settings.h"
#include "state_settings_internal.h"
#include "state_settings_rssi.h"
#include "state_menu.h"
#include "state_expresslrs.h"

#include "ui.h"
#include "settings_eeprom.h"
#include "timer.h"

#include "temperature.h"
#include "voltage.h"

//void *operator new(size_t size, void *ptr){
//  return ptr;
//}

#define MAX(a, b) (a > b ? a : b)

#define STATE_BUFFER_SIZE \
    MAX(sizeof(HomeStateHandler), \
    MAX(sizeof(SettingsStateHandler), \
    MAX(sizeof(SettingsInternalStateHandler), \
    MAX(sizeof(SettingsRssiStateHandler), \
    MAX(sizeof(MenuStateHandler), \
        sizeof(ExLRSStateHandler) \
    )))))
;

namespace StateMachine {

    static StateHandler *getStateHandler(State stateType);

    static uint8_t DMA_ATTR stateBuffer[STATE_BUFFER_SIZE];
    static StateHandler* DMA_ATTR currentHandler = nullptr;
    State DMA_ATTR currentState = State::BOOT;
    State DMA_ATTR lastState = currentState;


    void setup() {

    }

    void update() {
        if (currentHandler != nullptr) {
            currentHandler->onUpdate();
        }
    }

    void switchState(State newState) {
        if (currentHandler != nullptr) {
            currentHandler->onExit();
        }

        lastState = currentState;
        currentState = newState;
        currentHandler = getStateHandler(newState);

        if (currentHandler != nullptr) {
            currentHandler->onEnter();
            currentHandler->onInitialDraw();
        }
    }

    static StateHandler *getStateHandler(State state) {
        #define STATE_FACTORY(s, c) \
            case s: \
                return new (&stateBuffer) c(); \
                break;

        switch (state) {
            STATE_FACTORY(State::HOME, HomeStateHandler);
            STATE_FACTORY(State::SETTINGS, SettingsStateHandler);
            STATE_FACTORY(State::SETTINGS_INTERNAL, SettingsInternalStateHandler);
            STATE_FACTORY(State::SETTINGS_RSSI, SettingsRssiStateHandler);
            STATE_FACTORY(State::MENU, MenuStateHandler);
            STATE_FACTORY(State::EXPRESSLRS, ExLRSStateHandler);

            default:
                return nullptr;
        }

        #undef STATE_FACTORY
    }

    void StateHandler::drawHeader(void) {
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

        // Voltage
    #ifdef USE_VOLTAGE_MONITORING
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
        uint32_t sec_now = millis() / 1000;
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
    }
}
