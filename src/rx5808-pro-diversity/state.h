#ifndef STATE_H
#define STATE_H


#include <stdint.h>


namespace StateMachine {
//    #define STATE_COUNT 11
    enum class State : uint8_t {
        BOOT,
        HOME,
        SETTINGS,
        SETTINGS_INTERNAL,
        SETTINGS_RSSI
    };

    class StateHandler {
        public:
            virtual void onInitialDraw() {};
            virtual void onUpdateDraw() {};

            virtual void onEnter() {};
            virtual void onUpdate() {};
            virtual void onExit() {};

//            virtual void onButtonChange(
//                Button button,
//                Buttons::PressType pressType) {};
    };

    extern State currentState;
    extern State lastState;

    void setup();
    void update();

    void switchState(State newState);
}


#endif
