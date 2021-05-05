#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <stdint.h>


#define GESTURES_ENABLED 0

namespace TouchPad {

    typedef struct _TouchData {
      int16_t cursorX;
      int16_t cursorY;
      uint8_t buttonPrimary   :1,
              buttonSecondary :1,
              buttonAuxiliary :1;
    } TouchData;

    enum class Gesture : uint8_t {
        Up,
        Down,
        Left,
        Right,
        Nope
    };

    void setup();
    void clearTouchData();
    TouchData get();
}

#endif
