#include <Arduino.h>

#include "settings.h"
#include "settings_eeprom.h"
#include "receiver.h"
#include "receiver_spi.h"
#include "channels.h"
#include "state.h"
#include "ui.h"

#include "timer.h"

namespace Receiver {
    ReceiverId DMA_ATTR activeReceiver = ReceiverId::ALL; // Init to ALL to make sure the initial value is set correctly!
    uint8_t DMA_ATTR activeChannel;

    uint16_t DMA_ATTR rssiA = 0;
    uint32_t DMA_ATTR rssiARaw = 0;
    uint16_t DMA_ATTR rssiALast[RECEIVER_LAST_DATA_SIZE] = { 0 };

    uint16_t DMA_ATTR rssiB = 0;
    uint32_t DMA_ATTR rssiBRaw = 0;
    uint16_t DMA_ATTR rssiBLast[RECEIVER_LAST_DATA_SIZE] = { 0 };

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
    uint16_t DMA_ATTR rssiC = 0;
    uint32_t DMA_ATTR rssiCRaw = 0;
    uint16_t DMA_ATTR rssiCLast[RECEIVER_LAST_DATA_SIZE] = { 0 };

    uint16_t DMA_ATTR rssiD = 0;
    uint32_t DMA_ATTR rssiDRaw = 0;
    uint16_t DMA_ATTR rssiDLast[RECEIVER_LAST_DATA_SIZE] = { 0 };
#endif

    uint16_t DMA_ATTR previousSwitchTime = 0;
    uint16_t DMA_ATTR antennaAOnTime = 0;
    uint16_t DMA_ATTR antennaBOnTime = 0;
    uint16_t DMA_ATTR antennaCOnTime = 0;
    uint16_t DMA_ATTR antennaDOnTime = 0;

    ReceiverId DMA_ATTR diversityTargetReceiver = activeReceiver;
    static Timer DMA_ATTR diversityHysteresisTimer = Timer(5); // default value and is replce by value stored in eeprom during setup

    static Timer DMA_ATTR rssiStableTimer = Timer(30); // default value and is replce by value stored in eeprom during setup
    static Timer DMA_ATTR rssiLogTimer = Timer(RECEIVER_LAST_DELAY);

    static bool DMA_ATTR hasRssiUpdated = false;

    void setChannel(uint8_t channel)
    {
        ReceiverSpi::setSynthRegisterB(Channels::getFrequency(channel));

        rssiStableTimer.reset();
        activeChannel = channel;

        hasRssiUpdated = false;
    }

    void setChannelByFreq(uint16_t freq)
    {
        setChannel(Channels::getClosestChannel(freq));
    }

    static void setActiveReceiver(ReceiverId receiver)
    {
        // TODO FIXME!
        //if (receiver == activeReceiver)
        //    return;

        switch (EepromSettings.diversityMode) {
            case Receiver::DiversityMode::ANTENNA_A:
                receiver = ReceiverId::A;
                digitalWrite(PIN_RX_SWITCH, LOW);
                break;

            case Receiver::DiversityMode::ANTENNA_B:
                receiver = ReceiverId::B;
                digitalWrite(PIN_RX_SWITCH, HIGH);
                break;

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
            case Receiver::DiversityMode::ANTENNA_C:
                receiver = ReceiverId::C;
                break;

            case Receiver::DiversityMode::ANTENNA_D:
                receiver = ReceiverId::D;
                break;
#endif
            case Receiver::DiversityMode::DIVERSITY:
                if (receiver == ReceiverId::A) {
                    digitalWrite(PIN_RX_SWITCH, LOW);
                } else if (receiver == ReceiverId::B) {
                    digitalWrite(PIN_RX_SWITCH, HIGH);
                }
                break;

            case Receiver::DiversityMode::QUADVERSITY:
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
                digitalWrite(PIN_LED_A, receiver == ReceiverId::A);
                digitalWrite(PIN_LED_B, receiver == ReceiverId::B);
                digitalWrite(PIN_LED_C, receiver == ReceiverId::C);
                digitalWrite(PIN_LED_D, receiver == ReceiverId::D);
                break;
#endif
            default:
                break;
        }

        activeReceiver = receiver;
    }

    bool isRssiStable()
    {
        return rssiStableTimer.hasTicked();
    }

    bool isRssiStableAndUpdated()
    {
        return isRssiStable() && hasRssiUpdated;
    }

    void updateRssi()
    {
        uint8_t iter;

#define RSSI_READS 4 // 3;

        rssiARaw = rssiBRaw = 0;
        for (iter = 0; iter < RSSI_READS; iter++) {
            rssiARaw += analogRead(PIN_RSSI_A);
            rssiBRaw += analogRead(PIN_RSSI_B);
        }
        rssiARaw /= RSSI_READS;
        rssiBRaw /= RSSI_READS;

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
        if (EepromSettings.quadversity) {
            rssiCRaw = rssiDRaw = 0;
            for (iter = 0; iter < RSSI_READS; iter++) {
                rssiCRaw += analogRead(PIN_RSSI_C);
                rssiDRaw += analogRead(PIN_RSSI_D);
            }
            rssiCRaw /= RSSI_READS;
            rssiDRaw /= RSSI_READS;
        }
#endif

        if (StateMachine::currentState != StateMachine::State::SETTINGS_RSSI) {

            rssiA = constrain(
                map(
                    rssiARaw,
                    EepromSettings.rssiAMin,
                    EepromSettings.rssiAMax,
                    0,
                    1000
                ),
                0,
                1000
            );

            rssiB = constrain(
                map(
                    rssiBRaw,
                    EepromSettings.rssiBMin,
                    EepromSettings.rssiBMax,
                    0,
                    1000
                ),
                0,
                1000
            );

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
            if (EepromSettings.quadversity) {
                rssiC = constrain(
                    map(
                        rssiCRaw,
                        EepromSettings.rssiCMin,
                        EepromSettings.rssiCMax,
                        0,
                        1000
                    ),
                    0,
                    1000
                );
                rssiD = constrain(
                    map(
                        rssiDRaw,
                        EepromSettings.rssiDMin,
                        EepromSettings.rssiDMax,
                        0,
                        1000
                    ),
                    0,
                    1000
                );
            }
#endif
        }

#if 1 || !HOME_SHOW_LAPTIMES
        if (rssiLogTimer.hasTicked()) {
            for (uint8_t i = 0; i < RECEIVER_LAST_DATA_SIZE - 1; i++) {
                rssiALast[i] = rssiALast[i + 1];
                rssiBLast[i] = rssiBLast[i + 1];
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
                if (EepromSettings.quadversity) {
                    rssiCLast[i] = rssiCLast[i + 1];
                    rssiDLast[i] = rssiDLast[i + 1];
                }
#endif
            }

            rssiALast[RECEIVER_LAST_DATA_SIZE - 1] = rssiA;
            rssiBLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiB;
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
            if (EepromSettings.quadversity) {
                rssiCLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiC;
                rssiDLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiD;
            }
#endif
            rssiLogTimer.reset();
            hasRssiUpdated = true;
        }
#endif // !HOME_SHOW_LAPTIMES
    }

    static void switchDiversity() {
        int32_t rssiDiff = 0;
        ReceiverId nextReceiver = activeReceiver;
        ReceiverId currentBestReceiver = activeReceiver;

#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
        if (EepromSettings.quadversity) {
            int32_t rssiMax = max(max(rssiA, rssiB), max(rssiC, rssiD));

            // Find which Rx has the highest RSSI.
            if (rssiA == rssiMax) {
                currentBestReceiver = ReceiverId::A;
            } else if (rssiB == rssiMax) {
                currentBestReceiver = ReceiverId::B;
            } else if (rssiC == rssiMax) {
                currentBestReceiver = ReceiverId::C;
            } else if (rssiD == rssiMax) {
                currentBestReceiver = ReceiverId::D;
            }

            // Difference against currently active Rx.
            if (ReceiverId::A == activeReceiver) {
                rssiDiff = rssiMax - rssiA;
            } else if (ReceiverId::B == activeReceiver) {
                rssiDiff = rssiMax - rssiB;
            } else if (ReceiverId::C == activeReceiver) {
                rssiDiff = rssiMax - rssiC;
            } else if (ReceiverId::D == activeReceiver) {
                rssiDiff = rssiMax - rssiD;
            }
        }
        else
#endif // PIN_RSSI_C && PIN_RSSI_D
        {
            rssiDiff = (int32_t)(rssiA - rssiB);

            if (rssiDiff > 0) {
                currentBestReceiver = ReceiverId::A;
            } else if (rssiDiff < 0) {
                currentBestReceiver = ReceiverId::B;
            }
        }

        if (abs(rssiDiff) >= EepromSettings.rssiHysteresis) {
            if (currentBestReceiver == diversityTargetReceiver) {
                if (diversityHysteresisTimer.hasTicked()) {
                    nextReceiver = diversityTargetReceiver;
                }
            } else {
                diversityTargetReceiver = currentBestReceiver;
                diversityHysteresisTimer.reset();
            }
        } else {
            diversityHysteresisTimer.reset();
        }

        setActiveReceiver(nextReceiver);
    }

    void updateAntenaOnTime()
    {
        uint32_t _sec_now = (millis() / 1000);
        uint32_t secs = _sec_now - previousSwitchTime;
        switch (activeReceiver) {
            case ReceiverId::A:
                antennaAOnTime += secs;
                break;
            case ReceiverId::B:
                antennaBOnTime += secs;
                break;
#if defined(PIN_RSSI_C) && defined(PIN_RSSI_D)
            case ReceiverId::C:
                antennaCOnTime += secs;
                break;
            case ReceiverId::D:
                antennaDOnTime += secs;
                break;
#endif
            default:
                break;
        }
        previousSwitchTime = _sec_now;
    }

    void setup()
    {
        diversityHysteresisTimer = Timer(EepromSettings.rssiHysteresisPeriod);
        rssiStableTimer = Timer(EepromSettings.rssiMinTuneTime);
        setChannel(EepromSettings.startChannel);
        setActiveReceiver(ReceiverId::A);
    }

    void update()
    {
        if (isRssiStable()) {

            updateAntenaOnTime();

            updateRssi();

            // TODO FIXME!
            /* Switch only if mode is selected */
            //if (EepromSettings.diversityMode == Receiver::DiversityMode::DIVERSITY ||
            //    EepromSettings.diversityMode == Receiver::DiversityMode::QUADVERSITY)
                switchDiversity();
        }

    }
}
