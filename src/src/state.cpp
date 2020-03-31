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

    static uint8_t stateBuffer[STATE_BUFFER_SIZE];
    static StateHandler* currentHandler = nullptr;
    State currentState = State::BOOT;
    State lastState = currentState;


    void setup() {

    }

    void update() {
  
        if (currentHandler) {
            currentHandler->onUpdate();

            // FIXME: This should probably be handled in the UI module but not
            // 100% on how to decouple them at this stage
            if (currentHandler

            ) {

                currentHandler->onUpdateDraw();

            }
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

}
