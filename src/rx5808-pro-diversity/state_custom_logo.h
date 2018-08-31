#ifndef STATE_CUSTOM_LOGO_H
#define STATE_CUSTOM_LOGO_H


#include "state.h"
#include "settings.h"


namespace StateMachine {
    class CustomLogoStateHandler : public StateMachine::StateHandler {

        public:

            uint8_t x = 64;
            uint8_t y = 32;
            
            void onEnter();
            void onUpdate();

            void onInitialDraw();
            void onUpdateDraw();

            void onButtonChange(Button button, Buttons::PressType pressType);

    };
}

#endif
