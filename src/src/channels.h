#ifndef CHANNELS_H
#define CHANNELS_H

#include "settings.h"
#include <stdint.h>

// #define CHANNELS_72
#define CHANNELS_48

#ifdef CHANNELS_72
    #define CHANNELS_SIZE 72
#endif
#ifdef CHANNELS_48
    #define CHANNELS_SIZE 48
#endif

namespace Channels {
    const uint16_t getSynthRegisterB(uint8_t index);
    const uint16_t getSynthRegisterBFreq(uint16_t freq);
    const uint16_t getFrequency(uint8_t index);
    extern char *getName(uint8_t index);
    const uint8_t getOrderedIndex(uint8_t index);
    const uint8_t getOrderedIndexFromIndex(uint8_t index);
    
    const uint16_t getCenterFreq(uint16_t freq);
    const uint8_t getClosestChannel(uint16_t freq);
}

#endif