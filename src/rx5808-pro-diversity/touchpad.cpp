#include <Arduino.h>

#include "ui.h"
#include "settings.h"
#include "touchpad.h"

//// Cirque's 7-bit I2C Slave Address
//#define TOUCHPAD_SLAVE_ADDR  0x2A

// Masks for Cirque Register Access Protocol (RAP)
#define TOUCHPAD_WRITE_MASK  0x80
#define TOUCHPAD_READ_MASK   0xA0
        
namespace TouchPad {

    relData_t touchData;
      
    void setup() {

        Pinnacle_Init();
    
        touchData.cursorX = 200;
        touchData.cursorY = 100;

    }  
    
    void update() {

        if(isDataAvailable()) {
          
            Pinnacle_getRelative(&touchData);

            touchData.cursorX -= touchData.xDelta; // Reversed for testing
//            touchData.cursorX += touchData.xDelta;
            touchData.cursorY -= touchData.yDelta;
            
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

            if (touchData.buttonPrimary) {
//                Ui::beep();
            }
            
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
        touchData.isActive = false;
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

      SPI.begin();
  
      // Host clears SW_CC flag
      Pinnacle_ClearFlags();
    
      // Feed Enable
      RAP_Write(0x04, 0b00000001);
      
    }
    
    // Reads X, Y, and Scroll-Wheel deltas from Pinnacle, as well as button states
    // NOTE: this function should be called immediately after DR is asserted (HIGH)
    void Pinnacle_getRelative(relData_t * result) {
      
      uint8_t data[3] = { 0,0,0 };
    
      RAP_ReadBytes(0x12, data, 3);
    
      Pinnacle_ClearFlags();

      
      result->isActive = true;
      result->buttonPrimary = data[0] & 0b00000001;
      result->buttonSecondary = data[0] & 0b00000010;
      result->buttonAuxiliary = data[0] & 0b00000100;
      result->xDelta = (int8_t)data[2];
      result->yDelta = (int8_t)data[1];
      result->xSign = data[0] & 0b00010000;
      result->ySign = data[0] & 0b00100000;
    
    }
    
    // Clears Status1 register flags (SW_CC and SW_DR)
    void Pinnacle_ClearFlags() {
      
      RAP_Write(0x02, 0x00);
      
      delayMicroseconds(50);
      
    }
    
    /*  RAP Functions */
    // Reads <count> Pinnacle registers starting at <address>
    //void RAP_ReadBytes(byte address, byte * data, byte count)
    void RAP_ReadBytes(uint8_t address, uint8_t * data, uint8_t count) {
        byte cmdByte = TOUCHPAD_READ_MASK | address;   // Form the READ command byte
      
        SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE1));
      
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
    void RAP_Write(uint8_t address, uint8_t data) {
        byte cmdByte = TOUCHPAD_WRITE_MASK | address;  // Form the WRITE command byte
      
        SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE1));
      
        Assert_SS();
        SPI.transfer(cmdByte);  // Signal a write to register at <address>
        SPI.transfer(data);    // Send <value> to be written to register
        DeAssert_SS();
      
        SPI.endTransaction();
    }

    void Assert_SS()
    {
      digitalWrite(PIN_TOUCHPAD_SLAVE_SELECT, LOW);
    }
    
    void DeAssert_SS()
    {
      digitalWrite(PIN_TOUCHPAD_SLAVE_SELECT, HIGH);
    }
    
    bool isDataAvailable()
    {
      return digitalRead(PIN_TOUCHPAD_DATA_READY);
    }
    
}
