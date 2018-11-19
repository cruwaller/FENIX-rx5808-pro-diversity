#ifndef UI_H
#define UI_H

//#include <SPI.h>
//#include <Wire.h>


#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>

//#include "TTVout.h"
////#include <fontALL.h>
//#include <font8x8.h>

#include "settings.h"
#include "settings_internal.h"
#include "timer.h"


#include "CompositeGraphics.h"
#include "Image.h"
#include "CompositeOutput.h"
#include "luni.h"
#include "font6x8.h"
#include "font8x8.h"
#include <soc/rtc.h>

#define SCREEN_WIDTH 324
#define SCREEN_HEIGHT 224

#define SCREEN_WIDTH_MID ((SCREEN_WIDTH / 2) - 1)
#define SCREEN_HEIGHT_MID ((SCREEN_HEIGHT / 2) - 1)

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7

namespace Ui {
    
//    extern TTVout display;
    
//    extern bool shouldDrawUpdate;
//    extern bool shouldDisplay;
//    extern bool shouldFullRedraw;  
    extern int XRES;
    extern int YRES;
    extern CompositeGraphics display;
    extern CompositeOutput composite;
    extern Image<CompositeGraphics> luni0;
    extern Font<CompositeGraphics> font;

    void compositeCore(void *data);
    void draw();
    
    extern bool isTvOn;
    
    // Refresh rate is set to match touchpad rate.  Refresh every 50ms (20 FPS)
//    extern Timer UiRefreshTimer;
    // UI time for return to FPV
    extern Timer UiTimeOut;

    void setup();
//    void sdToTtvout();
    void update();

    void tvOn();
    void tvOff();
//    void beep();
//    void beep(uint16_t freq);

    void drawCursor();
    
//    void drawGraph(
//        const uint8_t data[],
//        const uint8_t dataSize,
//        const uint8_t dataScale,
//        const uint8_t x,
//        const uint8_t y,
//        const uint8_t w,
//        const uint8_t h
//    );
//
//    void drawSolidGraph(
//        const uint8_t data[],
//        const uint8_t dataSize,
//        const uint8_t dataScale,
//        const uint8_t x,
//        const uint8_t y,
//        const uint8_t w,
//        const uint8_t h
//    );
//
    void drawLine(const int x1, const int y1, const int x2, const int y2, const int color);
//    void drawDashedHLine(const int x, const int y, const int w, const int step);
//    void drawDashedVLine(const int x, const int y, const int w, const int step);
//    void drawFastHLine(const int x, const int y, const int w, const int color);
//    void drawFastVLine(const int x, const int y, const int h, const int color);
//    void drawRect(const int x, const int y,const int w, const int h, const int color);
//    void drawRoundRect(const int x, const int y,const int w, const int h, const int r, const int color);
//    void fillRect(const int x, const int y,const int w, const int h, const int color);
//    void fillTriangle(const int x0, const int y0, const int x1, const int y1, const int x2, const int y2, const int color);
//    void drawBitmap(const int x, const int y, const unsigned char *image, const int w, const int h, const int color);
//    void drawCircle(const int x, const int y,const int r, const int color, const int fillcolor);
//    void drawBullseye(const int x, const int y,const int r, const int color);
//  
//    void setTextColor(const int color);
//    void setTextSize(const int size);
//    void setCursor(const int x, const int y);
//    void print(const char text);
    void drawBigCharacter( const int x, const int y, const char text, const int xMultiplier, const int yMultiplier);
//    void drawBigNumber( const int x, const int y, const int number, const int xMultiplier, const int yMultiplier);
//      
//    void clear();
//    void clearRect(const int x, const int y, const int w, const int h);
//
//    void needUpdate();
//    void needDisplay();
//    void needFullRedraw();
}

#endif
