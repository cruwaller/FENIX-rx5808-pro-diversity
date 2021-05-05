#include "channels.h"
#include "settings.h"
#include "receiver.h"
#include "ui.h"
#include "settings_eeprom.h"
#include <Arduino.h>
//#include <esp_attr.h>


#if (CHANNELS_SIZE != 48) && (CHANNELS_SIZE != 72)
    #error "Invalid CHANNELS_SIZE"
#endif


// Channels with their Mhz Values
static const uint16_t DMA_ATTR channelFreqTable[] = {
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // A
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // B
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // E
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // F / Airwave
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // R / Immersion Raceband
    5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621, // L
    5325, 5348, 5366, 5384, 5402, 5420, 5438, 5456, // U
    5474, 5492, 5510, 5528, 5546, 5564, 5582, 5600, // O
    5653, 5693, 5733, 5773, 5813, 5853, 5893, 5933  // H
};

#if (CHANNELS_SIZE == 48)
// All Channels of the above List ordered by Mhz
static const uint8_t DMA_ATTR channelFreqOrderedIndex[] = {
        40, // 5362
        41, // 5399
        42, // 5436
        43, // 5473
        44, // 5510
        45, // 5547
        46, // 5584
        47, // 5621
        19, // 5645
        32, // 5658
        18, // 5665
        17, // 5685
        33, // 5695
        16, // 5705
         7, // 5725
        34, // 5732
         8, // 5733
        24, // 5740
         6, // 5745
         9, // 5752
        25, // 5760
         5, // 5765
        35, // 5769
        10, // 5771
        26, // 5780
         4, // 5785
        11, // 5790
        27, // 5800
         3, // 5805
        36, // 5806
        12, // 5809
        28, // 5820
         2, // 5825
        13, // 5828
        29, // 5840
        37, // 5843
         1, // 5845
        14, // 5847
        30, // 5860
         0, // 5865
        15, // 5866
        31, // 5880
        38, // 5880
        20, // 5885
        21, // 5905
        39, // 5917
        22, // 5925
        23  // 5945
};

static const uint8_t DMA_ATTR channelIndexToOrderedIndex[] = {
        39,
        36,
        32,
        28,
        25,
        21,
        18,
        14,
        16,
        19,
        23,
        26,
        30,
        33,
        37,
        40,
        13,
        11,
        10,
         8,
        43,
        44,
        46,
        47,
        17,
        20,
        24,
        27,
        31,
        34,
        38,
        41,
         9,
        12,
        15,
        22,
        29,
        35,
        42,
        45,
         0,
         1,
         2,
         3,
         4,
         5,
         6,
         7
};

#elif (CHANNELS_SIZE == 72)
// All Channels of the above List ordered by Mhz
static const uint8_t DMA_ATTR channelFreqOrderedIndex[] = {
        48, //5325
        49, //5348
        40, //5362
        50, //5366
        51, //5384
        41, //5399
        52, //5402
        53, //5420
        42, //5436
        54, //5438
        55, //5456
        43, //5473
        56, //5474
        57, //5492
        44, //5510
        58, //5510
        59, //5528
        60, //5546
        45, //5547
        61, //5564
        62, //5582
        46, //5584
        63, //5600
        47, //5621
        19, //5645
        64, //5653
        32, //5658
        18, //5665
        17, //5685
        65, //5693
        33, //5695
        16, //5705
        7, //5725
        34, //5732
        8, //5733
        66, //5733
        24, //5740
        6, //5745
        9, //5752
        25, //5760
        5, //5765
        35, //5769
        10, //5771
        67, //5773
        26, //5780
        4, //5785
        11, //5790
        27, //5800
        3, //5805
        36, //5806
        12, //5809
        68, //5813
        28, //5820
        2, //5825
        13, //5828
        29, //5840
        37, //5843
        1, //5845
        14, //5847
        69, //5853
        30, //5860
        0, //5865
        15, //5866
        31, //5880
        38, //5880
        20, //5885
        70, //5893
        21, //5905
        39, //5917
        22, //5925
        71, //5933
        23  //5945
};

static const uint8_t DMA_ATTR channelIndexToOrderedIndex[] = {
        61,
        57,
        53,
        48,
        45,
        40,
        37,
        32,
        34,
        38,
        42,
        46,
        50,
        54,
        58,
        62,
        31,
        28,
        27,
        24,
        65,
        67,
        69,
        71,
        36,
        39,
        44,
        47,
        52,
        55,
        60,
        63,
        26,
        30,
        33,
        41,
        49,
        56,
        64,
        68,
        2,
        5,
        8,
        11,
        14,
        18,
        21,
        23,
        0,
        1,
        3,
        4,
        6,
        7,
        9,
        10,
        12,
        13,
        15,
        16,
        17,
        19,
        20,
        22,
        25,
        29,
        35,
        43,
        51,
        59,
        66,
        70
};
#endif

namespace Channels {
    const uint16_t IRAM_ATTR getFrequency(uint8_t const index) {
        return channelFreqTable[index];
    }

    // Returns channel name as a string.
    void IRAM_ATTR getName(uint8_t const index, char * const nameBuffer) {
        const char bandNames[] = {
            'A', 'B', 'E', 'F', 'R', 'L', 'U', 'O', 'H',};
        uint8_t band = index / 8;
        uint8_t channel = 48 + (index % 8) + 1;   // https://www.arduino.cc/en/Reference/ASCIIchart
        if (band < sizeof(bandNames))
            nameBuffer[0] = bandNames[band];
        else
            nameBuffer[0] = '_';
        nameBuffer[1] = channel;
    }

    const uint8_t IRAM_ATTR getOrderedIndex(uint8_t index) {
        if (sizeof(channelFreqOrderedIndex) <= index)
            return channelFreqOrderedIndex[sizeof(channelFreqOrderedIndex)-1];
        return channelFreqOrderedIndex[index];
    }

    const uint8_t IRAM_ATTR getOrderedIndexFromIndex(uint8_t index) {
        if (sizeof(channelIndexToOrderedIndex) <= index)
            return channelIndexToOrderedIndex[sizeof(channelIndexToOrderedIndex)-1];
        return channelIndexToOrderedIndex[index];
    }

    const uint16_t IRAM_ATTR getCenterFreq(uint16_t const freq) {
        uint16_t upperFreq = freq;
        uint16_t lowerFreq = freq;
        uint16_t rssi = 1000;

        while (rssi > EepromSettings.rssiSeekTreshold) {
          upperFreq = upperFreq + 1;
          Receiver::setChannelByFreq(upperFreq);
          while (!Receiver::isRssiStableAndUpdated()) {
            delay(1);
          }
          rssi = (Receiver::rssiA + Receiver::rssiB)/2;
        }

        rssi  = 1000;
        while (rssi > EepromSettings.rssiSeekTreshold) {
          lowerFreq = lowerFreq - 1;
          Receiver::setChannelByFreq(lowerFreq);
          while (!Receiver::isRssiStableAndUpdated()) {
            delay(1);
          }
          rssi = (Receiver::rssiA + Receiver::rssiB)/2;
        }

        return (lowerFreq + upperFreq) / 2;
    }

    const uint8_t IRAM_ATTR getClosestChannel(uint16_t const freq) {

        uint8_t closestChannel = 0;
        for (int j=0; j<CHANNELS_SIZE; j++) {
            if (
                max( Channels::getFrequency(j), freq ) - min( Channels::getFrequency(j), freq ) <=
                max( Channels::getFrequency(closestChannel), freq ) - min( Channels::getFrequency(closestChannel), freq )
                ) {
              closestChannel = j;
            }
        }

        return closestChannel;
    }
}
