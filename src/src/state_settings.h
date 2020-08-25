#ifndef STATE_SETTINGS_H
#define STATE_SETTINGS_H


#include "state.h"


namespace StateMachine {
    class SettingsStateHandler : public StateMachine::StateHandler {
        private:
            void onUpdateDraw(uint8_t tapAction);

        public:
            void onEnter();
            void onUpdate();
    };
}


#endif
