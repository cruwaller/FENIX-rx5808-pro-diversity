#ifndef STATE_FAVOURITES_H
#define STATE_FAVOURITES_H


#include "state.h"
#include "settings.h"


namespace StateMachine {
    class FavouritesStateHandler : public StateMachine::StateHandler {

        public:
            void onEnter();
            void onUpdate();

            void onInitialDraw();
            void onUpdateDraw();

            void onButtonChange(Button button, Buttons::PressType pressType);

    };
}

#endif
