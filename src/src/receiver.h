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
        ALL,
    };

    enum class DiversityMode : uint8_t {
        ANTENNA_A,
        ANTENNA_B,
        DIVERSITY,
    };

    extern ReceiverId DMA_ATTR activeReceiver;
    extern uint8_t DMA_ATTR activeChannel;

    extern uint16_t DMA_ATTR rssiA;
    extern uint32_t DMA_ATTR rssiARaw;
    extern uint16_t DMA_ATTR rssiALast[RECEIVER_LAST_DATA_SIZE];
    extern uint16_t DMA_ATTR rssiB;
    extern uint32_t DMA_ATTR rssiBRaw;
    extern uint16_t DMA_ATTR rssiBLast[RECEIVER_LAST_DATA_SIZE];

    extern uint32_t DMA_ATTR antennaAOnTime;
    extern uint32_t DMA_ATTR antennaBOnTime;

    void IRAM_ATTR setDiversityMode(DiversityMode mode);

    void IRAM_ATTR setChannel(uint8_t channel, ReceiverId rcvr_id=ReceiverId::ALL);
    void IRAM_ATTR setChannelByFreq(uint16_t freq);
    void IRAM_ATTR updateRssi();

    bool IRAM_ATTR isRssiStable();
    bool IRAM_ATTR isRssiStableAndUpdated();

    void setup();
    void IRAM_ATTR update();
}
#endif
