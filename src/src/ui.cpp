#include <stdint.h>

#include "settings.h"
#include "settings_eeprom.h"
#include "ui.h"
#include "receiver_spi.h"
#include "receiver.h"
#include "touchpad.h"

namespace Ui {
  
    bool isTvOn = false;

    Timer UiTimeOut = Timer(2000);

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
    xTaskCreatePinnedToCore(compositeCore, "c", 1024, NULL, 255, NULL, 0); //increase priority to remove osd jitter with esp-now
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

    void drawCursor() {

        int i = 0;
        for(int py = 0; py < Cursor::yres; py++) {
            for(int px = 0; px < Cursor::xres; px++) {
                int pixelValue = Cursor::pixels[i++];
                if (pixelValue == 255) {
                    display.dot(px + TouchPad::touchData.cursorX, py + TouchPad::touchData.cursorY, 100);
                }
                if (pixelValue == 1) {
                    display.dot(px + TouchPad::touchData.cursorX, py + TouchPad::touchData.cursorY, 0);
                }
            }
        }

    //    // For testing print the cursor location.  Helps with positioning widgits.
    //    display.setCursor(TouchPad::touchData.cursorX, TouchPad::touchData.cursorY);
    //    display.print("  [");
    //    display.print(TouchPad::touchData.cursorX);
    //    display.print(", ");
    //    display.print(TouchPad::touchData.cursorY);
    //    display.print("]");
        
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
