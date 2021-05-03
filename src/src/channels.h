#ifndef CHANNELS_H
#define CHANNELS_H

#include "settings.h"
#include <stdint.h>
#include <esp_attr.h>

#if (CHANNELS_SIZE != 48) && (CHANNELS_SIZE != 72)
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
