#ifndef STATE_H
#define STATE_H


#include <stdint.h>


namespace StateMachine {

    enum class State : uint8_t {
        BOOT,
        HOME,
        SETTINGS,
        SETTINGS_INTERNAL,
        SETTINGS_RSSI,
        MENU,
        EXPRESSLRS
    };

    class StateHandler {
        public:
            virtual void onInitialDraw() {};
            virtual void onUpdateDraw() {};

            virtual void onEnter() {};
            virtual void onUpdate() {};
            virtual void onExit() {};

    };

    extern State currentState;
    extern State lastState;

    void setup();
    void update();

    void switchState(State newState);
}


#endif
