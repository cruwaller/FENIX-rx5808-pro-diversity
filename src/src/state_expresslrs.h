#ifndef STATE_EXLRS_H
#define STATE_EXLRS_H


#include "state.h"


namespace StateMachine {
    class ExLRSStateHandler : public StateMachine::StateHandler {
        private:
            void doTapAction();

        public:
            void onEnter();
            void onUpdate();

            void onInitialDraw();
            void onUpdateDraw();

    };
}


#endif
