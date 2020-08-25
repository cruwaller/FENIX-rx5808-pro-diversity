#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <stdint.h>
#include <esp_attr.h>
#include <esp8266-compat.h>

namespace TouchPad {

    typedef struct _relData
    {
      bool buttonPrimary;
      bool buttonSecondary;
      bool buttonAuxiliary;
      int8_t xDelta;
      int8_t yDelta;
      bool xSign;
      bool ySign;
      int16_t cursorX;
      int16_t cursorY;

      int32_t timeLastButtonPress;
      bool buttonOrderChecked;
      bool switchButtonOrder;
    } relData_t;

    extern relData_t DMA_ATTR touchData;

    enum class Gesture : uint8_t {
        Up,
        Down,
        Left,
        Right,
        Nope
    };

    void setup();
    void update();
    void clearTouchData();
    void Pinnacle_Init();
    void ICACHE_RAM_ATTR Pinnacle_getRelative(relData_t * result);
    void ICACHE_RAM_ATTR Pinnacle_ClearFlags();
    void ICACHE_RAM_ATTR RAP_ReadBytes(uint8_t address, uint8_t * data, uint8_t count);
    void ICACHE_RAM_ATTR RAP_Write(uint8_t address, uint8_t data);
    void ICACHE_RAM_ATTR Assert_SS();
    void ICACHE_RAM_ATTR DeAssert_SS();
    bool ICACHE_RAM_ATTR isDataAvailable();

    Gesture isGesture();
    void doGesture(Gesture currentGesture);
    void setChannel(int channelIncrement);

}

#endif
