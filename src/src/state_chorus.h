#ifndef STATE_CHORUS_H
#define STATE_CHORUS_H

#include "state.h"

namespace StateMachine {
    class ChorusStateHandler : public StateMachine::StateHandler {
        private:
            void doTapAction();
            void onUpdateDraw(uint8_t tapAction);

        public:
            void onEnter();
            void onUpdate();

            void onInitialDraw();
    };
}

#endif
