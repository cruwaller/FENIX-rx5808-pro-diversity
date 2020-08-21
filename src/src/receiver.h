#ifndef RECEIVER_H
#define RECEIVER_H


#include <stdint.h>
#include <esp_attr.h>
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

    extern ReceiverId DMA_ATTR activeReceiver;
    extern uint8_t DMA_ATTR activeChannel;

    extern uint16_t DMA_ATTR rssiA;
    extern uint32_t DMA_ATTR rssiARaw;
    extern uint16_t DMA_ATTR rssiALast[RECEIVER_LAST_DATA_SIZE];
    extern uint16_t DMA_ATTR rssiB;
    extern uint32_t DMA_ATTR rssiBRaw;
    extern uint16_t DMA_ATTR rssiBLast[RECEIVER_LAST_DATA_SIZE];
    extern uint16_t DMA_ATTR rssiC;
    extern uint32_t DMA_ATTR rssiCRaw;
    extern uint16_t DMA_ATTR rssiCLast[RECEIVER_LAST_DATA_SIZE];
    extern uint16_t DMA_ATTR rssiD;
    extern uint32_t DMA_ATTR rssiDRaw;
    extern uint16_t DMA_ATTR rssiDLast[RECEIVER_LAST_DATA_SIZE];

    extern uint16_t DMA_ATTR antennaAOnTime;
    extern uint16_t DMA_ATTR antennaBOnTime;
    extern uint16_t DMA_ATTR antennaCOnTime;
    extern uint16_t DMA_ATTR antennaDOnTime;

    void setChannel(uint8_t channel);
    void setChannelByFreq(uint16_t freq);
    void updateRssi();

    bool isRssiStable();
    bool isRssiStableAndUpdated();

    void setup();
    void update();
}
#endif
