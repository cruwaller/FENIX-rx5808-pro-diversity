#ifndef CHANNELS_H
#define CHANNELS_H

#include "settings.h"
#include <stdint.h>
#include <esp_attr.h>

#ifdef CHANNELS_72
    #define CHANNELS_SIZE 72
#elif defined(CHANNELS_48)
    #define CHANNELS_SIZE 48
#else
    #error "Invalid CHANNELS_SIZE"
#endif

namespace Channels {
    const uint16_t IRAM_ATTR getFrequency(uint8_t index);
    extern char * IRAM_ATTR getName(uint8_t index);
    const uint8_t IRAM_ATTR getOrderedIndex(uint8_t index);
    const uint8_t IRAM_ATTR getOrderedIndexFromIndex(uint8_t index);

    const uint16_t IRAM_ATTR getCenterFreq(uint16_t freq);
    const uint8_t IRAM_ATTR getClosestChannel(uint16_t freq);
}

#endif
