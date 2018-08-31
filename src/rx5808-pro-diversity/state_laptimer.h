#ifndef STATE_LAPTIMER_H
#define STATE_LAPTIMER_H


#include "state.h"
#include "settings.h"


namespace StateMachine {
    class LaptimerStateHandler : public StateMachine::StateHandler {

        public:
            void onEnter();
            void onUpdate();

            void onInitialDraw();
            void onUpdateDraw();

            void onButtonChange(Button button, Buttons::PressType pressType);

    };
}

#endif
