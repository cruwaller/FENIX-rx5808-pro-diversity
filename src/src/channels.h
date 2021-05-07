#ifndef CHANNELS_H
#define CHANNELS_H

#include <stdint.h>



namespace Channels {
    constexpr uint8_t getnamesize = 3;

    const uint16_t getFrequency(uint8_t index);
    const int8_t getIndexByFrequency(uint16_t freq);
    extern void getName(uint8_t index, char * const nameBuffer);
    const uint8_t getOrderedIndex(uint8_t index);
    const uint8_t getOrderedIndexFromIndex(uint8_t index);

    const uint16_t getCenterFreq(uint16_t freq);
    const uint8_t getClosestChannel(uint16_t freq);
}

#endif
