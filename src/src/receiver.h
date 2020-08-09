#ifndef RECEIVER_H
#define RECEIVER_H


#include <stdint.h>
#include "channels.h"


#define RECEIVER_LAST_DELAY 50
#define RECEIVER_LAST_DATA_SIZE 128 //64


namespace Receiver {

    enum class ReceiverId : uint8_t {
        A,
        B,
        C,
        D,
        ALL,
    };

    enum class DiversityMode : uint8_t {
        ANTENNA_A,
        ANTENNA_B,
        ANTENNA_C,
        ANTENNA_D,
        DIVERSITY,
        QUADVERSITY
    };

    extern ReceiverId activeReceiver;
    extern uint8_t activeChannel;

    extern uint16_t  rssiA;
    extern uint32_t rssiARaw;
    extern uint16_t  rssiALast[RECEIVER_LAST_DATA_SIZE];
    extern uint16_t  rssiB;
    extern uint32_t rssiBRaw;
    extern uint16_t  rssiBLast[RECEIVER_LAST_DATA_SIZE];
    extern uint16_t  rssiC;
    extern uint32_t rssiCRaw;
    extern uint16_t  rssiCLast[RECEIVER_LAST_DATA_SIZE];
    extern uint16_t  rssiD;
    extern uint32_t rssiDRaw;
    extern uint16_t  rssiDLast[RECEIVER_LAST_DATA_SIZE];

    extern uint16_t rssiBandScanData[CHANNELS_SIZE];

    extern uint16_t previousSwitchTime;
    extern uint16_t antennaAOnTime;
    extern uint16_t antennaBOnTime;
    extern uint16_t antennaCOnTime;
    extern uint16_t antennaDOnTime;

    void setChannel(uint8_t channel);
    void setChannelByFreq(uint16_t freq);
    void updateRssi();
    void setActiveReceiver(ReceiverId receiver = ReceiverId::A);

    void antenaOnTime();

    void switchDiversity();

    bool isRssiStable();
    extern bool hasRssiUpdated;

    void setup();
    void update();
}
#endif
