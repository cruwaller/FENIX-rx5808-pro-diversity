#ifndef STATE_SETTINGS_INTERNAL_H
#define STATE_SETTINGS_INTERNAL_H


#include "state.h"


namespace StateMachine {
    class SettingsInternalStateHandler : public StateMachine::StateHandler {
        public:
            void onEnter();
            void onExit();
            void onUpdate();

            void onInitialDraw();
            void onUpdateDraw();

            void onButtonChange(Button button, Buttons::PressType pressType);
    };
}


#endif
