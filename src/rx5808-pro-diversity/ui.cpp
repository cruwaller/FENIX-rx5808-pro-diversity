#include <stdint.h>

#include "settings.h"
#include "settings_eeprom.h"
#include "ui.h"
#include "receiver_spi.h"
#include "receiver.h"
#include "touchpad.h"

namespace Ui {
  
    bool isTvOn = false;

    Timer UiTimeOut = Timer(3000);

    int XRES = 324;
    int YRES = 224;
    CompositeGraphics display(XRES, YRES);
    CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);
    Font<CompositeGraphics> font(8, 8, font8x8::pixels);

    void setup() {

    //highest clockspeed needed
    rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
    
    //initializing DMA buffers and I2S
    composite.init();
    //initializing graphics double buffer
    display.init();
    composite.stopOutput(); //stop i2s driver (no video output)
    //select font
    display.setFont(font);

    //running composite output pinned to first core
    xTaskCreatePinnedToCore(compositeCore, "c", 1024, NULL, 1, NULL, 0);
    //rendering the actual graphics in the main loop is done on the second core by default
    
    }    

    void update() {

        drawCursor();
        
    }

    void compositeCore(void *data)
    {
      while (true)
      {
        //just send the graphics frontbuffer whithout any interruption 
        composite.sendFrameHalfResolution(&display.frame);
      }
    }

    void tvOn() {

        ReceiverSpi::setPowerDownRegister(0b01010000110000010011);
        
        composite.startOutput();
        
        isTvOn = true;
    }

    void tvOff() { 

        ReceiverSpi::setPowerDownRegister(0b00010000110000010011);

        Receiver::setChannel(Receiver::activeChannel);
        
        composite.stopOutput();
         
        isTvOn = false;
    }

    // This needs to be redone with a fill triangle... which needs to be added to the TTVout lib.
    void drawCursor() {
        
        // Black inner
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX, TouchPad::touchData.cursorY + 16, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+1, TouchPad::touchData.cursorY + 16, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+2, TouchPad::touchData.cursorY + 15, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+3, TouchPad::touchData.cursorY + 14, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+4, TouchPad::touchData.cursorY + 13, 10);
        
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+6, TouchPad::touchData.cursorY + 18, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+7, TouchPad::touchData.cursorY + 18, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+8, TouchPad::touchData.cursorY + 17, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+9, TouchPad::touchData.cursorY + 17, 10);
        
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+7, TouchPad::touchData.cursorY + 13, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+8, TouchPad::touchData.cursorY + 13, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+9, TouchPad::touchData.cursorY + 13, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+10, TouchPad::touchData.cursorY + 13, 10);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX+11, TouchPad::touchData.cursorY + 13, 10);
        
        // White boarder
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, TouchPad::touchData.cursorX, TouchPad::touchData.cursorY + 16, 100);
        display.line(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY + 16, TouchPad::touchData.cursorX + 4, TouchPad::touchData.cursorY + 13, 100);
        display.line(TouchPad::touchData.cursorX + 4, TouchPad::touchData.cursorY + 13, TouchPad::touchData.cursorX + 6, TouchPad::touchData.cursorY + 18, 100);
        display.line(TouchPad::touchData.cursorX + 6, TouchPad::touchData.cursorY + 18, TouchPad::touchData.cursorX + 9, TouchPad::touchData.cursorY + 17, 100);
        display.line(TouchPad::touchData.cursorX + 9, TouchPad::touchData.cursorY + 17, TouchPad::touchData.cursorX + 7, TouchPad::touchData.cursorY + 13, 100);
        display.line(TouchPad::touchData.cursorX + 7, TouchPad::touchData.cursorY + 13, TouchPad::touchData.cursorX + 11, TouchPad::touchData.cursorY + 12, 100);
        display.line(TouchPad::touchData.cursorX + 11, TouchPad::touchData.cursorY + 12, TouchPad::touchData.cursorX, TouchPad::touchData.cursorY, 100);

        display.setCursor(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY);
        display.print("[");
        display.print(TouchPad::touchData.cursorX);
        display.print(", ");
        display.print(TouchPad::touchData.cursorY);
        display.print("]");
        
    }

//    void beep() { 
//        uint16_t freq = 5000; // frequency in Hz
//        display.tone(freq, BEEPER_CHIRP);
//    }
//    void beep(uint16_t freq) { 
//        display.tone(freq, BEEPER_CHIRP);
//    }

//    void drawBullseye(const int x, const int y,const int r, const int color) {
//      drawCircle(x, y, r-2, color, BLACK);
//      drawFastHLine(x, y, 0, color);
//      drawFastHLine(x-r, y, 2*r+1, color);
//      drawFastVLine(x, y-r, 2*r+1, color);
//    }
    
}


