#ifndef STATE_SETTINGS_H
#define STATE_SETTINGS_H


#include "state.h"


namespace StateMachine {
    class SettingsStateHandler : public StateMachine::StateHandler {
        public:
            void onEnter();
            void onUpdate(TouchPad::TouchData const &touch);
    };
}


#endif
