#ifndef STATE_FINDER_H
#define STATE_FINDER_H


#include "state.h"
#include "settings.h"


namespace StateMachine {
    class FinderStateHandler : public StateMachine::StateHandler {

        public:
            void onEnter();
            void onUpdate();

            void onInitialDraw();
            void onUpdateDraw();

            void onButtonChange(Button button, Buttons::PressType pressType);

    };
}

#endif
