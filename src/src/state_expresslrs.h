#ifndef STATE_EXLRS_H
#define STATE_EXLRS_H


#include "state.h"


namespace StateMachine {
    class ExLRSStateHandler : public StateMachine::StateHandler {
        private:
            void onUpdateDraw(uint8_t tapAction);

        public:
            void onEnter();
            void onUpdate();
    };
}


#endif
