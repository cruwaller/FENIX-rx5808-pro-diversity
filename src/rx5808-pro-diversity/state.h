#ifndef STATE_H
#define STATE_H


#include <stdint.h>
#include "buttons.h"


namespace StateMachine {
//    #define STATE_COUNT 11
    enum class State : uint8_t {
        BOOT,
        HOME,
//        HOME_SIMPLE,
//        HOME_STATS,
//        SEARCH,
//        BANDSCAN,
//        SPECTATOR,
//        SCREENSAVER,
//        FAVOURITES, // https://github.com/piodabro/WPro58 I pinched your star :)
//        FINDER,
//        LAPTIMER,
//        MENU,
        SETTINGS,
        SETTINGS_INTERNAL,
        SETTINGS_RSSI
//        CUSTOMLOGO
    };

    class StateHandler {
        public:
            virtual void onInitialDraw() {};
            virtual void onUpdateDraw() {};

            virtual void onEnter() {};
            virtual void onUpdate() {};
            virtual void onExit() {};

            virtual void onButtonChange(
                Button button,
                Buttons::PressType pressType) {};
    };

    extern State currentState;
    extern State lastState;

    void setup();
    void update();

    void switchState(State newState);
}


#endif
