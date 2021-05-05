#ifndef STATE_CHORUS_H
#define STATE_CHORUS_H

#include "state.h"

namespace StateMachine {
    class ChorusStateHandler : public StateMachine::StateHandler {
        public:
            void onUpdate(TouchPad::TouchData const &touch);
    };
}

#endif
