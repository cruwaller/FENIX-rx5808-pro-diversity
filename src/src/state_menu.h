#ifndef STATE_MENU_H
#define STATE_MENU_H


#include "state.h"


namespace StateMachine {
    class MenuStateHandler : public StateMachine::StateHandler {
        private:
            void onUpdateDraw(uint8_t tapAction);

        public:
            void onEnter();
            void onUpdate();
    };
}


#endif
