#ifndef STATE_SETTINGS_INTERNAL_H
#define STATE_SETTINGS_INTERNAL_H


#include "state.h"


namespace StateMachine {
    class SettingsInternalStateHandler : public StateMachine::StateHandler {
        public:
            void onEnter();
            void onUpdate(TouchPad::TouchData const &touch);
    };
}


#endif
