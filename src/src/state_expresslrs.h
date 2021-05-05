#ifndef STATE_EXLRS_H
#define STATE_EXLRS_H


#include "state.h"


namespace StateMachine {
    class ExLRSStateHandler : public StateMachine::StateHandler {
        public:
            void onUpdate(TouchPad::TouchData const &touch);
    };
}


#endif
