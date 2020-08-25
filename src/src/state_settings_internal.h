#ifndef STATE_SETTINGS_INTERNAL_H
#define STATE_SETTINGS_INTERNAL_H


#include "state.h"


namespace StateMachine {
    class SettingsInternalStateHandler : public StateMachine::StateHandler {
        private:
            void onUpdateDraw(uint8_t tapAction);

        public:
            void onEnter();
            void onExit();
            void onUpdate();
    };
}


#endif
