/*
   SPI driver based on fs_skyrf_58g-main.c Written by Simon Chambers
   TVOUT by Myles Metzel
   Scanner by Johan Hermen
   Inital 2 Button version by Peter (pete1990)
   Refactored and GUI reworked by Marko Hoepken
   Universal version my Marko Hoepken
   Diversity Receiver Mode and GUI improvements by Shea Ivey
   OLED Version by Shea Ivey
   Seperating display concerns by Shea Ivey

  The MIT License (MIT)

  Copyright (c) 2015 Marko Hoepken

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), bto deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/




#include "settings.h"
#include "settings_internal.h"
#include "settings_eeprom.h"

#include "channels.h"
#include "receiver.h"
#include "receiver_spi.h"
#include "state.h"
#include "ui.h"
#include "voltage.h"
#include "temperature.h"
#include "touchpad.h"




#include <soc/rtc.h>
#include "CompositeGraphics.h"
#include "Image.h"
#include "CompositeOutput.h"
#include "luni.h"
#include "font6x8.h"

const int XRES = 324;
const int YRES = 224;
CompositeGraphics graphics(XRES, YRES);
CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);
Image<CompositeGraphics> luni0(luni::xres, luni::yres, luni::pixels);
Font<CompositeGraphics> font(6, 8, font6x8::pixels);

void setup()
{

    Serial.begin(9600);

    SPI.begin();
    
    EepromSettings.load();

    setupPins();
//    Temperature::setup();
    StateMachine::setup();
    Ui::setup(); 
    TouchPad::setup(); 

  // Has to be last setup() otherwise channel may not be set.
  // RX possibly not botting quick enough if setup() is called earler.
  Receiver::setup(); 

//  if (!EepromSettings.isCalibrated) {
//      StateMachine::switchState(StateMachine::State::SETTINGS_RSSI); 
//      Ui::switchOSDOutputState();    
//  } else {
      StateMachine::switchState(StateMachine::State::HOME); 
//  }   


  

    //highest clockspeed needed
    rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
    
    //initializing DMA buffers and I2S
    composite.init();
    //initializing graphics double buffer
    graphics.init();
    //select font
    graphics.setFont(font);
  
    //running composite output pinned to first core
    xTaskCreatePinnedToCore(compositeCore, "c", 1024, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(compositeCore, "c", 1024, NULL, 1, NULL, 0);
    //rendering the actual graphics in the main loop is done on the second core by default
        
}

void setupPins() {

    pinMode(PIN_SPI_SLAVE_SELECT_RX_A, OUTPUT);
    digitalWrite(PIN_SPI_SLAVE_SELECT_RX_A, HIGH);
    
    pinMode(PIN_SPI_SLAVE_SELECT_RX_B, OUTPUT);
    digitalWrite(PIN_SPI_SLAVE_SELECT_RX_B, HIGH);

    pinMode(PIN_RX_SWICTH, OUTPUT);
    
    pinMode(PIN_TOUCHPAD_SLAVE_SELECT, OUTPUT);
    digitalWrite(PIN_TOUCHPAD_SLAVE_SELECT, HIGH);

    pinMode(PIN_TOUCHPAD_DATA_READY, INPUT);

}

void loop() {
  
    Receiver::update();
    
//    #ifdef FENIX_QUADVERSITY  
//        Voltage::update();
//    #endif
  
//    if (Ui::UiRefreshTimer.hasTicked()) {
//        Ui::UiRefreshTimer.reset();
        TouchPad::update(); 
        
        if (Ui::isTvOn) {
//            Temperature::update();
//            StateMachine::update();
            Ui::update();
            EepromSettings.update();
        }
//    }
    
    if (TouchPad::touchData.isActive) {
        Ui::UiTimeOut.reset();
    }
    if (Ui::isTvOn && Ui::UiTimeOut.hasTicked()) {
        Ui::switchOSDOutputState();    
    }
    if (!Ui::isTvOn && TouchPad::touchData.buttonPrimary) {
        Ui::switchOSDOutputState();
    }
  
    TouchPad::clearTouchData(); 

    
    draw();

}

void draw()
{
  //clearing background and starting to draw
  graphics.begin(0);
  //drawing an image
  luni0.draw(graphics, 200, 10);

  //drawing a frame
  graphics.fillRect(27, 18, 160, 30, 10);
  graphics.rect(27, 18, 160, 30, 20);

  //setting text color, transparent background
  graphics.setTextColor(50);
  //text starting position
  graphics.setCursor(30, 20);
  //printing some lines of text
  graphics.print("hello!");
  graphics.print(" free memory: ");
  graphics.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  graphics.print("\nrendered frame: ");
  static int frame = 0;
  graphics.print(frame, 10, 4); //base 10, 6 characters 
  graphics.print("\n        in hex: ");
  graphics.print(frame, 16, 4);
  frame++;

  //drawing some lines
  for(int i = 0; i <= 100; i++)
  {
    graphics.line(50, i + 60, 50 + i, 160, i / 2);
    graphics.line(150, 160 - i, 50 + i, 60, i / 2);
  }
  
  //draw single pixel
  graphics.dot(20, 190, 10);
  
  //finished drawing, swap back and front buffer to display it
  graphics.end();
}

void compositeCore(void *data)
{
  while (true)
  {
    //just send the graphics frontbuffer whithout any interruption 
    composite.sendFrameHalfResolution(&graphics.frame);
  }
}
