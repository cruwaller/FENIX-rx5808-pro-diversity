#ifndef STATE_H
#define STATE_H

#include <esp_attr.h>
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
        protected:
            void drawHeader(void);
    };

    extern State DMA_ATTR currentState;
    extern State DMA_ATTR lastState;

    void setup();
    void update();

    void switchState(State newState);
}

#endif
