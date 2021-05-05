#ifndef STATE_HOME_H
#define STATE_HOME_H

#include "channels.h"
#include "state.h"
#include "settings.h"
#include "receiver.h"

namespace StateMachine {
    class HomeStateHandler : public StateMachine::StateHandler {
        private:
            uint8_t displayActiveChannel = 0;
            bool wasInBandScanRegion = false;
            bool isInBandScanRegion(int16_t const posy) const {
                return (130 < posy);
            }
            void bandScanUpdate();

            void setChannel(int channelIncrement, int setChannel = -1);
            void centreFrequency();
            bool centred = false;

            uint8_t orderedChanelIndex = 0;
            uint8_t lastChannelIndex = 0;

        public:
            void onEnter();
            void onUpdate(TouchPad::TouchData const &touch);
    };
}

#endif
