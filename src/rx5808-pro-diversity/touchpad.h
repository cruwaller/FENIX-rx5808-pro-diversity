#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <stdint.h>
#include <SPI.h>

namespace TouchPad {

    typedef struct _relData
    {
      bool isActive;
      bool buttonPrimary;
      bool buttonSecondary;
      bool buttonAuxiliary;
      int8_t xDelta;
      int8_t yDelta;
      bool xSign;
      bool ySign; 
      int16_t cursorX;
      int16_t cursorY;
    } relData_t;

    extern relData_t touchData;
    
    void setup();
    void update();
    void clearTouchData();
    void Pinnacle_Init();
    void Pinnacle_getRelative(relData_t * result);
    void Pinnacle_ClearFlags();
    void RAP_ReadBytes(uint8_t address, uint8_t * data, uint8_t count);
    void RAP_Write(uint8_t address, uint8_t data);
    void Assert_SS();
    void DeAssert_SS();
    bool isDataAvailable();
    
    
}

#endif
