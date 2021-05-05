#ifndef STATE_SETTINGS_RSSI_H
#define STATE_SETTINGS_RSSI_H


#include "state.h"


namespace StateMachine {
    class SettingsRssiStateHandler : public StateMachine::StateHandler {
        private:
            enum class InternalState : uint8_t {
                WAIT_FOR_LOW,
                SCANNING_LOW,
                WAIT_FOR_HIGH,
                SCANNING_HIGH,
                DONE
            };

            void doTapAction();
            void onUpdateDraw();

            InternalState internalState = InternalState::WAIT_FOR_LOW;
            uint8_t currentSweep = 0;
            uint8_t bestChannel = 0;


        public:
            void onEnter();
            void onUpdate(TouchPad::TouchData const &touch);
            void onExit();
    };
}


#endif
