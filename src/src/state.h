#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <esp_attr.h>


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
            virtual void onUpdate() {};
            virtual void onExit() {};
        protected:
            uint8_t drawHeader(void);
    };

    extern State DMA_ATTR currentState;
    extern State DMA_ATTR lastState;

    void setup();
    void update();

    void switchState(State newState);
}

#endif
