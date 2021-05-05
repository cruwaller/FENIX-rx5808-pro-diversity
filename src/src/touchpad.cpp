#include <Arduino.h>
#include <SPI.h>

#include "ui.h"
#include "channels.h"
#include "receiver.h"
#include "settings.h"
#include "settings_eeprom.h"
#include "touchpad.h"
#include "task_prios.h"

#define DEBUG_TOUCHPAD 0

// Masks for Cirque Register Access Protocol (RAP)
#define TOUCHPAD_WRITE_MASK  0x80
#define TOUCHPAD_READ_MASK   0xA0

#define GESTURE_ARRAY_SIZE 8


extern SemaphoreHandle_t mutex_spi;

static TaskHandle_t task_handle;


namespace TouchPad
{
    void IRAM_ATTR Pinnacle_getRelative(relData_t * result);
    void IRAM_ATTR RAP_ReadBytes(uint8_t address, uint8_t * data, uint8_t count);
    void IRAM_ATTR RAP_Write(uint8_t address, uint8_t data);

#if GESTURES_ENABLED
    Gesture isGesture();
    void doGesture(Gesture currentGesture);
    void setChannel(int channelIncrement);

    int DMA_ATTR xGestureArray[GESTURE_ARRAY_SIZE] = {0};
    int DMA_ATTR yGestureArray[GESTURE_ARRAY_SIZE] = {0};
    int DMA_ATTR xSwipeThreshold = 130;
    int DMA_ATTR ySwipeThreshold = 200;
#endif // GESTURES_ENABLED

    relData_t DMA_ATTR touchData;

    SPISettings DMA_ATTR spi_settings(10000000, MSBFIRST, SPI_MODE1);


    static void IRAM_ATTR _data_ready_isr_handler()
    {
        BaseType_t pxHigherPriorityTaskWoken = pdFALSE;;
        if (task_handle)
            xTaskNotifyFromISR(task_handle, 0, eNoAction, &pxHigherPriorityTaskWoken);
        if (pxHigherPriorityTaskWoken)
            portYIELD_FROM_ISR();
    }

    static inline void Assert_SS()
    {
        digitalWrite(PIN_TOUCHPAD_SLAVE_SELECT, LOW);
    }

    static inline void DeAssert_SS()
    {
        digitalWrite(PIN_TOUCHPAD_SLAVE_SELECT, HIGH);
    }

    static inline bool isDataAvailable()
    {
        return digitalRead(PIN_TOUCHPAD_DATA_READY);
    }

    static void touchpad_task(void*)
    {
        while(1) {
            if (pdTRUE == xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(50))) {
                //Serial.println("touch");
                Pinnacle_getRelative(&touchData);
                //if (touchData.buttonPrimary)
                //    Serial.println("touch");

                Ui::UiTimeOut.reset(); // reset since touch is active

#if GESTURES_ENABLED
                if (Ui::isTvOn) {
#endif // GESTURES_ENABLED
                    int16_t cursorX = touchData.cursorX;
                    int16_t cursorY = touchData.cursorY;
                    cursorX += touchData.xDelta;
                    cursorY -= touchData.yDelta;
                    //cursorX -= touchData.xDelta;
                    //cursorY += touchData.yDelta;

                    if (cursorX < 1) {
                        cursorX = 1;
                    } else if (cursorX > Ui::XRES - 1) {
                        cursorX = Ui::XRES - 1;
                    }
                    if (cursorY < 1) {
                        cursorY = 1;
                    } else if (cursorY > Ui::YRES - 1) {
                        cursorY = Ui::YRES - 1;
                    }
                    touchData.cursorX = cursorX;
                    touchData.cursorY = cursorY;
#if GESTURES_ENABLED
                } else {
                    Gesture currentGesture = isGesture();
                    if (currentGesture != Gesture::Nope) {
                        doGesture(currentGesture);
                    }
                }
#endif // GESTURES_ENABLED
            }

#if 0
            if (isDataAvailable()) {
                Pinnacle_getRelative(&touchData);

                Ui::UiTimeOut.reset(); // reset since touch is active

#if GESTURES_ENABLED
                if (Ui::isTvOn) {
#endif // GESTURES_ENABLED
                    int16_t cursorX = touchData.cursorX;
                    int16_t cursorY = touchData.cursorY;
                    cursorX += touchData.xDelta;
                    cursorY -= touchData.yDelta;
                    //cursorX -= touchData.xDelta;
                    //cursorY += touchData.yDelta;

                    if (cursorX < 1) {
                        cursorX = 1;
                    } else if (cursorX > Ui::XRES - 1) {
                        cursorX = Ui::XRES - 1;
                    }
                    if (cursorY < 1) {
                        cursorY = 1;
                    } else if (cursorY > Ui::YRES - 1) {
                        cursorY = Ui::YRES - 1;
                    }
                    touchData.cursorX = cursorX;
                    touchData.cursorY = cursorY;
#if GESTURES_ENABLED
                } else {
                    Gesture currentGesture = isGesture();
                    if (currentGesture != Gesture::Nope) {
                        doGesture(currentGesture);
                    }
                }
#endif // GESTURES_ENABLED

            } else {
                vTaskDelay(5 / portTICK_PERIOD_MS);
            }
#endif
        }
        vTaskDelete(NULL);
        task_handle = NULL;
    }


    // Clears Status1 register flags (SW_CC and SW_DR)
    static inline void Pinnacle_ClearFlags()
    {
        RAP_Write(0x02, 0x00);
        delayMicroseconds(50);
    }


    void setup()
    {
        // Host clears SW_CC flag
        Pinnacle_ClearFlags();
        // Feed Enable
        RAP_Write(0x04, 0b00000001);

        pinMode(PIN_TOUCHPAD_DATA_READY, INPUT);
        attachInterrupt(digitalPinToInterrupt(PIN_TOUCHPAD_DATA_READY),
                        _data_ready_isr_handler, RISING);

        touchData.cursorX = 200;
        touchData.cursorY = 100;
        touchData.dataUpdated = false;

        xTaskCreatePinnedToCore(touchpad_task, "touch",
            1024, NULL, TASK_PRIO_RSSI, &task_handle, 1);
    }


    void IRAM_ATTR update()
    {
    }


    void IRAM_ATTR clearTouchData()
    {
        touchData.buttonPrimary = 0;
        //touchData.buttonSecondary = 0;
        //touchData.buttonAuxiliary = 0;
        touchData.xDelta = 0;
        touchData.yDelta = 0;
        //touchData.xSign = 0;
        //touchData.ySign = 0;
        touchData.dataUpdated = false;
    }


    // Reads X, Y, and Scroll-Wheel deltas from Pinnacle, as well as button states
    // NOTE: this function should be called immediately after DR is asserted (HIGH)
    void IRAM_ATTR Pinnacle_getRelative(relData_t * result)
    {
        uint8_t data[3] = { 0,0,0 };

        RAP_ReadBytes(0x12, data, sizeof(data));

        Pinnacle_ClearFlags();

        if (!result->dataUpdated)
        {
            result->buttonPrimary   = !!(data[0] & 0b00000001);
            //result->buttonSecondary = !!(data[0] & 0b00000010);
            //result->buttonAuxiliary = !!(data[0] & 0b00000100);
            result->dataUpdated     = true;
        }
        //result->xSign           = !!(data[0] & 0b00010000);
        //result->ySign           = !!(data[0] & 0b00100000);
        result->xDelta          = (int8_t)data[2];
        result->yDelta          = (int8_t)data[1];
    }

    /*  RAP Functions */
    // Reads <count> Pinnacle registers starting at <address>
    //void RAP_ReadBytes(byte address, byte * data, byte count)
    void IRAM_ATTR RAP_ReadBytes(uint8_t address, uint8_t * data, uint8_t count) {
        byte cmdByte = TOUCHPAD_READ_MASK | address;   // Form the READ command byte
        uint8_t buff[3 + count];
        memset(buff, 0xFC, sizeof(buff));
        buff[0] = cmdByte; // Signal a RAP-read operation starting at <address>

        xSemaphoreTake(mutex_spi, portMAX_DELAY);
        SPI.beginTransaction(spi_settings);
        Assert_SS();
        SPI.transferBytes(buff, buff, sizeof(buff));
        DeAssert_SS();
        SPI.endTransaction();
        xSemaphoreGive(mutex_spi);

        memcpy(data, &buff[3], count);
    }

    // Writes single-byte <data> to <address>
    void IRAM_ATTR RAP_Write(uint8_t address, uint8_t data)
    {
        uint8_t buff[] = {(uint8_t)(TOUCHPAD_WRITE_MASK | address), data};
        xSemaphoreTake(mutex_spi, portMAX_DELAY);
        SPI.beginTransaction(spi_settings);
        Assert_SS();
        SPI.transferBytes(buff, buff, sizeof(buff));
        DeAssert_SS();
        SPI.endTransaction();
        xSemaphoreGive(mutex_spi);
    }

#if GESTURES_ENABLED
    Gesture isGesture() {

        for (int i = 0; i < GESTURE_ARRAY_SIZE - 1; i++) {
            xGestureArray[GESTURE_ARRAY_SIZE-1-i] = xGestureArray[GESTURE_ARRAY_SIZE-2-i];
            yGestureArray[GESTURE_ARRAY_SIZE-1-i] = yGestureArray[GESTURE_ARRAY_SIZE-2-i];
        }
        xGestureArray[0] = TouchPad::touchData.xDelta;
        yGestureArray[0] = TouchPad::touchData.yDelta;

        int xSumArray = 0;
        int ySumArray = 0;
        for (int i = 0; i < GESTURE_ARRAY_SIZE - 1; i++) {
            xSumArray += xGestureArray[i];
            ySumArray += yGestureArray[i];
        }

        if (xSumArray > xSwipeThreshold) {
#if DEBUG_ENABLED && DEBUG_TOUCHPAD
            Serial.println("Swipe Left");
#endif
            for (int i = 0; i < GESTURE_ARRAY_SIZE - 1; i++) {
                xGestureArray[i] = 0;
                yGestureArray[i] = 0;
            }

            return Gesture::Left;
        } else if (xSumArray < -xSwipeThreshold) {
#if DEBUG_ENABLED && DEBUG_TOUCHPAD
            Serial.println("Swipe Right");
#endif
            for (int i = 0; i < GESTURE_ARRAY_SIZE - 1; i++) {
                xGestureArray[i] = 0;
                yGestureArray[i] = 0;
            }

            return Gesture::Right;
        } else if (ySumArray > ySwipeThreshold) {
#if DEBUG_ENABLED && DEBUG_TOUCHPAD
            Serial.println("Swipe Up");
#endif
            for (int i = 0; i < GESTURE_ARRAY_SIZE - 1; i++) {
                xGestureArray[i] = 0;
                yGestureArray[i] = 0;
            }

            return Gesture::Up;
        } else if (ySumArray < -ySwipeThreshold) {
#if DEBUG_ENABLED && DEBUG_TOUCHPAD
            Serial.println("Swipe Down");
#endif
            for (int i = 0; i < GESTURE_ARRAY_SIZE - 1; i++) {
                xGestureArray[i] = 0;
                yGestureArray[i] = 0;
            }

            return Gesture::Down;
        } else {
            return Gesture::Nope;
        }

    }

    void doGesture(Gesture currentGesture) {
        switch (currentGesture) {
          case Gesture::Left:
              break;
          case Gesture::Right:
              break;
          case Gesture::Up:
              setChannel(-1);
              break;
          case Gesture::Down:
              setChannel(1);
              break;
          default:
              break;
        }
    }

    void setChannel(int channelIncrement) {

        int band = Receiver::activeChannel / 8;
        int channel = Receiver::activeChannel % 8;

        if (channelIncrement == 8) {
          band = band + 1;
        }

        if (channelIncrement == -8) {
          band = band - 1;
        }

        if (channelIncrement == 1 ) {
          channel = channel + 1;
          if (channel > 7) {
            channel = 0;
          }
        }

        if (channelIncrement == -1 ) {
          channel = channel - 1;
          if (channel < 0) {
            channel = 7;
          }
        }

        int newChannelIndex = band * 8 + channel;

        if (newChannelIndex >= CHANNELS_SIZE) {
          newChannelIndex = newChannelIndex - CHANNELS_SIZE;
        }
        if (newChannelIndex < 0) {
          newChannelIndex = newChannelIndex + CHANNELS_SIZE;
        }
        Receiver::setChannel(newChannelIndex);
        EepromSettings.startChannel = newChannelIndex;
        EepromSettings.save();
    }
#endif // GESTURES_ENABLED
}
