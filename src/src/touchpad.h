#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <stdint.h>
#include <esp_attr.h>
#include <esp8266-compat.h>


#define GESTURES_ENABLED 0

namespace TouchPad {

    typedef struct _relData
    {
      int16_t cursorX;
      int16_t cursorY;
      int8_t xDelta;
      int8_t yDelta;
      //bool xSign;
      //bool ySign;
      bool buttonPrimary;
      //bool buttonSecondary;
      //bool buttonAuxiliary;
      bool dataUpdated;
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
}

#endif
