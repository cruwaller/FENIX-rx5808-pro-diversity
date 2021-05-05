#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <esp_attr.h>
#include "touchpad.h"


namespace StateMachine {

    enum class State : uint8_t {
        BOOT,
        HOME,
        SETTINGS,
        SETTINGS_INTERNAL,
        SETTINGS_RSSI,
        MENU,
        EXPRESSLRS,
        CHORUS
    };

    class StateHandler {
        public:
            virtual void onEnter() {};
            virtual void onUpdate(TouchPad::TouchData const &touch) {};
            virtual void onExit() {};
        protected:
            uint8_t drawHeader(int16_t cursorX, int16_t cursorY, uint8_t tap);
    };

    extern State DMA_ATTR currentState;
    extern State DMA_ATTR lastState;

    void setup();
    void update();

    void switchState(State newState);
}

#endif
