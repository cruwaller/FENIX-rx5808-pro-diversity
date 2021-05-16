#include <Arduino.h>
#include "task_prios.h"
#include "settings.h"
#include "settings_eeprom.h"
#include "receiver.h"
#include "receiver_spi.h"
#include "channels.h"
#include "state.h"
#include "ui.h"

#include "timer.h"

#include <esp32-hal-adc.h>

#define RSSI_READS 4 // 3;

#define RSSI_MIN 0
#define RSSI_MAX 1000

namespace Receiver {
    ReceiverId DMA_ATTR activeReceiver = ReceiverId::ALL; // Init to ALL to make sure the initial value is set correctly!
    uint8_t DMA_ATTR activeChannel;

    uint16_t DMA_ATTR rssiA = 0;
    uint32_t DMA_ATTR rssiARaw = 0;
    uint16_t DMA_ATTR rssiALast[RECEIVER_LAST_DATA_SIZE] = { 0 };

    uint16_t DMA_ATTR rssiB = 0;
    uint32_t DMA_ATTR rssiBRaw = 0;
    uint16_t DMA_ATTR rssiBLast[RECEIVER_LAST_DATA_SIZE] = { 0 };

    uint32_t DMA_ATTR antennaAOnTime = 0;
    uint32_t DMA_ATTR antennaBOnTime = 0;

    static uint32_t DMA_ATTR previousSwitchTime = 0;

    static ReceiverId DMA_ATTR diversityTargetReceiver = activeReceiver;
    static Timer DMA_ATTR diversityHysteresisTimer = Timer(5); // default value and is replce by value stored in eeprom during setup

    static Timer DMA_ATTR rssiStableTimer = Timer(30); // default value and is replce by value stored in eeprom during setup
    static Timer DMA_ATTR rssiLogTimer = Timer(RECEIVER_LAST_DELAY);

    static bool DMA_ATTR hasRssiUpdated = false;

    static TaskHandle_t rssi_task;

    static QueueHandle_t set_ch_queue;


    static uint32_t IRAM_ATTR updateRssiPin(uint8_t const pin, uint8_t const oversample)
    {
        uint32_t _rssiARaw = 0;
        uint8_t iter;
        adcAttachPin(pin);
        for (iter = 0; iter < oversample; iter++) {
            _rssiARaw += analogRead(pin);
        }
        _rssiARaw /= oversample;
        return _rssiARaw;
    }


    static void IRAM_ATTR setActiveReceiverSwitch(ReceiverId receiver)
    {
        if (receiver == activeReceiver)
            return;

        switch (receiver) {
            case ReceiverId::A:
                digitalWrite(PIN_RX_SWITCH, LOW);
                break;

            case ReceiverId::B:
                digitalWrite(PIN_RX_SWITCH, HIGH);
                break;

            default:
                return;
        }

        activeReceiver = receiver;
    }

    static void updateRssiValues(void)
    {
        DiversityMode const mode = EepromSettings.diversityMode;
        uint8_t const not_rssi_state =
            StateMachine::currentState != StateMachine::State::SETTINGS_RSSI;

        if (mode == Receiver::DiversityMode::ANTENNA_A ||
            mode == Receiver::DiversityMode::DIVERSITY) {
            rssiARaw = updateRssiPin(PIN_RSSI_A, RSSI_READS);
            if (not_rssi_state)
                rssiA = constrain(
                    map(
                        rssiARaw,
                        EepromSettings.rssiAMin,
                        EepromSettings.rssiAMax,
                        RSSI_MIN,
                        RSSI_MAX
                    ),
                    RSSI_MIN,
                    RSSI_MAX
                );
        }
        if (mode == Receiver::DiversityMode::ANTENNA_B ||
            mode == Receiver::DiversityMode::DIVERSITY) {
            rssiBRaw = updateRssiPin(PIN_RSSI_B, RSSI_READS);
            if (not_rssi_state)
                rssiB = constrain(
                    map(
                        rssiBRaw,
                        EepromSettings.rssiBMin,
                        EepromSettings.rssiBMax,
                        RSSI_MIN,
                        RSSI_MAX
                    ),
                    RSSI_MIN,
                    RSSI_MAX
                );
        }

        if (rssiLogTimer.hasTicked()) {
#if !HOME_SHOW_LAPTIMES
            for (uint8_t i = 0; i < RECEIVER_LAST_DATA_SIZE - 1; i++) {
                rssiALast[i] = rssiALast[i + 1];
                rssiBLast[i] = rssiBLast[i + 1];
            }
            rssiALast[RECEIVER_LAST_DATA_SIZE - 1] = rssiA;
            rssiBLast[RECEIVER_LAST_DATA_SIZE - 1] = rssiB;
#endif // HOME_SHOW_LAPTIMES

            rssiLogTimer.reset();
            hasRssiUpdated = true;
        }
    }

    static void IRAM_ATTR switchDiversity()
    {
        ReceiverId nextReceiver = activeReceiver;
        ReceiverId currentBestReceiver = activeReceiver;
        int32_t const rssiDiff = (int32_t)(rssiA - rssiB);

        if (abs(rssiDiff) >= EepromSettings.rssiHysteresis) {
            if (rssiDiff > 0) {
                currentBestReceiver = ReceiverId::A;
            } else if (rssiDiff < 0) {
                currentBestReceiver = ReceiverId::B;
            }

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

        setActiveReceiverSwitch(nextReceiver);
    }

    static void IRAM_ATTR updateAntenaOnTime()
    {
        uint32_t const _sec_now = (millis() / 1000);
        uint32_t const secs = _sec_now - previousSwitchTime;
        switch (activeReceiver) {
            case ReceiverId::A:
                antennaAOnTime += secs;
                break;
            case ReceiverId::B:
                antennaBOnTime += secs;
                break;
            default:
                break;
        }
        previousSwitchTime = _sec_now;
    }


    void IRAM_ATTR setChannel(uint8_t const channel, ReceiverId const rcvr_id)
    {
        hasRssiUpdated = false;
        xQueueSend(set_ch_queue, &channel, portMAX_DELAY);
    }

    void IRAM_ATTR setChannelByFreq(uint16_t const freq)
    {
        setChannel(Channels::getClosestChannel(freq));
    }

    void IRAM_ATTR setDiversityMode(DiversityMode const mode)
    {
        EepromSettings.diversityMode = mode;
        ReceiverId receiver = activeReceiver;

#if POWER_OFF_RX
        ReceiverSpi::rxStandby(Receiver::ReceiverId::ALL);
#endif

        switch (mode) {
            case Receiver::DiversityMode::ANTENNA_A:
                receiver = ReceiverId::A;
                break;

            case Receiver::DiversityMode::ANTENNA_B:
                receiver = ReceiverId::B;
                break;
            case Receiver::DiversityMode::DIVERSITY:
            default:
                receiver = ReceiverId::ALL;
                break;
        }

#if POWER_OFF_RX
        ReceiverSpi::rxWakeup(receiver);
#endif // POWER_OFF_RX

        setChannel(activeChannel, receiver);

        if (receiver != ReceiverId::ALL)
            setActiveReceiverSwitch(receiver);
    }

    bool IRAM_ATTR isRssiStable()
    {
        return rssiStableTimer.hasTicked();
    }

    bool IRAM_ATTR isRssiStableAndUpdated()
    {
        return isRssiStable() && hasRssiUpdated;
    }

    void updateRssiTask(void*)
    {
        uint8_t next_ch;
        while(1) {
            if (pdTRUE == xQueueReceive(set_ch_queue, &next_ch, EepromSettings.rssiMinTuneTime / portTICK_PERIOD_MS)) {
                activeChannel = next_ch;
                hasRssiUpdated = false;
                ReceiverSpi::setSynthRegisterB(Channels::getFrequency(next_ch), ReceiverId::ALL);
                rssiStableTimer.reset();
                rssiLogTimer.reset();
                continue;
            }
            //vTaskDelay(EepromSettings.rssiMinTuneTime / portTICK_PERIOD_MS);
            if (isRssiStable()) {
                updateAntenaOnTime();
                updateRssiValues();
                /* Switch only if mode is selected */
                if (EepromSettings.diversityMode == Receiver::DiversityMode::DIVERSITY)
                    switchDiversity();
            }
        }
        vTaskDelete(NULL);
        rssi_task = NULL;
    }

    void setup()
    {
        set_ch_queue = xQueueCreate(8, sizeof(uint8_t));
        diversityHysteresisTimer = Timer(EepromSettings.rssiHysteresisPeriod);
        rssiStableTimer = Timer(EepromSettings.rssiMinTuneTime);
        activeChannel = EepromSettings.startChannel;
        setDiversityMode(EepromSettings.diversityMode);

        if (!rssi_task)
            xTaskCreatePinnedToCore(updateRssiTask, "rssi",
                1024, NULL, TASK_PRIO_RSSI, &rssi_task, 1);
    }
}
