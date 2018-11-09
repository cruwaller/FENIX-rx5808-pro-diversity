#ifndef TOUCHPAD_H
#define TOUCHPAD_H

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
    void RAP_ReadBytes(byte address, byte * data, uint8_t count);
    void RAP_Write(byte address, byte data);
    bool isDataAvailable();
    
}

#endif
