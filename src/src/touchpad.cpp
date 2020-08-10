#include <Arduino.h>

#include "ui.h"
#include "channels.h"
#include "receiver.h"
#include "settings.h"
#include "settings_eeprom.h"
#include "touchpad.h"

#define DEBUG_TOUCHPAD 0
#define USE_ISR 0

//// Cirque's 7-bit I2C Slave Address
//#define TOUCHPAD_SLAVE_ADDR  0x2A

// Masks for Cirque Register Access Protocol (RAP)
#define TOUCHPAD_WRITE_MASK  0x80
#define TOUCHPAD_READ_MASK   0xA0

namespace TouchPad {

    const int sizeOfGestureArray = 8;
    int xGestureArray[sizeOfGestureArray] = {0};
    int yGestureArray[sizeOfGestureArray] = {0};
    int xSwipeThreshold = 130;
    int ySwipeThreshold = 200;

    relData_t DMA_ATTR touchData;

    SPISettings DMA_ATTR spi_settings(10000000, MSBFIRST, SPI_MODE1);

#if USE_ISR
    static volatile uint8_t DMA_ATTR _data_ready_state = LOW;
    static void ICACHE_RAM_ATTR _data_ready_isr_handler(void)
    {
      Pinnacle_getRelative(&touchData);
      _data_ready_state = HIGH;
    }
#endif

    void setup() {

        Pinnacle_Init();

        pinMode(PIN_TOUCHPAD_DATA_READY, INPUT);
#if USE_ISR
        attachInterrupt(digitalPinToInterrupt(PIN_TOUCHPAD_DATA_READY),
                        _data_ready_isr_handler, RISING);
#endif
        touchData.cursorX = 200;
        touchData.cursorY = 100;
        touchData.timeLastButtonPress = 0;
        touchData.switchButtonOrder = false;
        touchData.switchButtonOrder = false;
    }

    void update() {

        if(isDataAvailable()) {

#if DEBUG_ENABLED && DEBUG_TOUCHPAD
            Serial.println("Touchpad data ready!");
#endif
#if !USE_ISR
            Pinnacle_getRelative(&touchData);
#endif
            Ui::UiTimeOut.reset(); // reset since touch is active

            if (Ui::isTvOn) {
                touchData.cursorX += touchData.xDelta;
                touchData.cursorY -= touchData.yDelta;
//                touchData.cursorX -= touchData.xDelta;
//                touchData.cursorY += touchData.yDelta;

                if (touchData.cursorX < 1) {
                    touchData.cursorX = 1;
                }
                if (touchData.cursorX > SCREEN_WIDTH - 1) {
                    touchData.cursorX = SCREEN_WIDTH - 1;
                }
                if (touchData.cursorY < 1) {
                    touchData.cursorY = 1;
                }
                if (touchData.cursorY > SCREEN_HEIGHT - 1) {
                    touchData.cursorY = SCREEN_HEIGHT - 1;
                }
            } else {
                Gesture currentGesture = isGesture();
                if (currentGesture != Gesture::Nope) {
                    doGesture(currentGesture);
                }
            }

//            if (touchData.buttonPrimary) {
////                Ui::beep();
//            }

//            Serial.print(touchData.buttonPrimary);
//            Serial.print('\t');
//            Serial.print(touchData.buttonSecondary);
//            Serial.print('\t');
//            Serial.print(touchData.buttonAuxiliary);
//            Serial.print('\t');
//            Serial.print(touchData.xDelta);
//            Serial.print('\t');
//            Serial.print(touchData.yDelta);
//            Serial.print('\t');
//            Serial.print(touchData.xSign);
//            Serial.print('\t');
//            Serial.println(touchData.ySign);

        }

    }

    void clearTouchData() {
        touchData.buttonPrimary = 0;
        touchData.buttonSecondary = 0;
        touchData.buttonAuxiliary = 0;
        touchData.xDelta = 0;
        touchData.yDelta = 0;
        touchData.xSign = 0;
        touchData.ySign = 0;
    }

    /*  Pinnacle-based TM0XX0XX Functions  */
    void Pinnacle_Init() {

      // Host clears SW_CC flag
      Pinnacle_ClearFlags();

      // Feed Enable
      RAP_Write(0x04, 0b00000001);

    }

    // Reads X, Y, and Scroll-Wheel deltas from Pinnacle, as well as button states
    // NOTE: this function should be called immediately after DR is asserted (HIGH)
    void ICACHE_RAM_ATTR Pinnacle_getRelative(relData_t * result) {

      uint8_t data[3] = { 0,0,0 };

      RAP_ReadBytes(0x12, data, 3);

      Pinnacle_ClearFlags();

      //bool switchButtonOrder;

      result->buttonPrimary = data[0] & 0b00000001;
      result->buttonSecondary = data[0] & 0b00000010;

        // // Some touch pads reverse the primary/secondard order.
        // // Hacky fix to detect rapis button pressing and reverse order.
        // if(!switchButtonOrder && (millis() - result->timeLastButtonPress < 10))
        // {
        //   switchButtonOrder = true;
        //   result->timeLastButtonPress = millis();
        // }
        // if(switchButtonOrder)
        // {
        //   result->buttonPrimary = data[0] & 0b00000010;
        //   result->buttonSecondary = data[0] & 0b00000001;
        // }
        // ///////////////////////////////////////////////////////////////

      result->buttonAuxiliary = data[0] & 0b00000100;
      result->xDelta = (int8_t)data[2];
      result->yDelta = (int8_t)data[1];
      result->xSign = data[0] & 0b00010000;
      result->ySign = data[0] & 0b00100000;

    }

    // Clears Status1 register flags (SW_CC and SW_DR)
    void ICACHE_RAM_ATTR Pinnacle_ClearFlags() {

      RAP_Write(0x02, 0x00);

      delayMicroseconds(50); // TODO: needed??

    }

    /*  RAP Functions */
    // Reads <count> Pinnacle registers starting at <address>
    //void RAP_ReadBytes(byte address, byte * data, byte count)
    void ICACHE_RAM_ATTR RAP_ReadBytes(uint8_t address, uint8_t * data, uint8_t count) {
        byte cmdByte = TOUCHPAD_READ_MASK | address;   // Form the READ command byte

        SPI.beginTransaction(spi_settings);

        Assert_SS();
        SPI.transfer(cmdByte);  // Signal a RAP-read operation starting at <address>
        SPI.transfer(0xFC);     // Filler byte
        SPI.transfer(0xFC);     // Filler byte
        for(byte i = 0; i < count; i++)
        {
          data[i] =  SPI.transfer(0xFC);  // Each subsequent SPI transfer gets another register's contents
        }
        DeAssert_SS();

        SPI.endTransaction();
    }

    // Writes single-byte <data> to <address>
    void ICACHE_RAM_ATTR RAP_Write(uint8_t address, uint8_t data) {
        byte cmdByte = TOUCHPAD_WRITE_MASK | address;  // Form the WRITE command byte

        SPI.beginTransaction(spi_settings);

        Assert_SS();
        SPI.transfer(cmdByte);  // Signal a write to register at <address>
        SPI.transfer(data);    // Send <value> to be written to register
        DeAssert_SS();

        SPI.endTransaction();
    }

    void ICACHE_RAM_ATTR Assert_SS()
    {
      digitalWrite(PIN_TOUCHPAD_SLAVE_SELECT, LOW);
    }

    void ICACHE_RAM_ATTR DeAssert_SS()
    {
      digitalWrite(PIN_TOUCHPAD_SLAVE_SELECT, HIGH);
    }

    bool ICACHE_RAM_ATTR isDataAvailable()
    {
#if USE_ISR
      return _data_ready_state;
#else
      return digitalRead(PIN_TOUCHPAD_DATA_READY);
#endif
    }

    Gesture isGesture() {

        for (int i = 0; i < sizeOfGestureArray - 1; i++) {
            xGestureArray[sizeOfGestureArray-1-i] = xGestureArray[sizeOfGestureArray-2-i];
            yGestureArray[sizeOfGestureArray-1-i] = yGestureArray[sizeOfGestureArray-2-i];
        }
        xGestureArray[0] = TouchPad::touchData.xDelta;
        yGestureArray[0] = TouchPad::touchData.yDelta;

        int xSumArray = 0;
        int ySumArray = 0;
        for (int i = 0; i < sizeOfGestureArray - 1; i++) {
            xSumArray += xGestureArray[i];
            ySumArray += yGestureArray[i];
        }

        if (xSumArray > xSwipeThreshold) {
#if DEBUG_ENABLED && DEBUG_TOUCHPAD
            Serial.println("Swipe Left");
#endif
            for (int i = 0; i < sizeOfGestureArray - 1; i++) {
                xGestureArray[i] = 0;
                yGestureArray[i] = 0;
            }

            return Gesture::Left;
        } else if (xSumArray < -xSwipeThreshold) {
#if DEBUG_ENABLED && DEBUG_TOUCHPAD
            Serial.println("Swipe Right");
#endif
            for (int i = 0; i < sizeOfGestureArray - 1; i++) {
                xGestureArray[i] = 0;
                yGestureArray[i] = 0;
            }

            return Gesture::Right;
        } else if (ySumArray > ySwipeThreshold) {
#if DEBUG_ENABLED && DEBUG_TOUCHPAD
            Serial.println("Swipe Up");
#endif
            for (int i = 0; i < sizeOfGestureArray - 1; i++) {
                xGestureArray[i] = 0;
                yGestureArray[i] = 0;
            }

            return Gesture::Up;
        } else if (ySumArray < -ySwipeThreshold) {
#if DEBUG_ENABLED && DEBUG_TOUCHPAD
            Serial.println("Swipe Down");
#endif
            for (int i = 0; i < sizeOfGestureArray - 1; i++) {
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

}
