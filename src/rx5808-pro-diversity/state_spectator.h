#ifndef STATE_SPECTATOR_H
#define STATE_SPECTATOR_H


#include "state.h"
#include "settings.h"


namespace StateMachine {
    class SpectatorStateHandler : public StateMachine::StateHandler {
        private:

        public:
            void onEnter();
            void onUpdate();

            void onInitialDraw();
            void onUpdateDraw();
            
            void onExit();
            
            void onButtonChange(Button button, Buttons::PressType pressType);

            void findChannels();

    };
}

#endif
