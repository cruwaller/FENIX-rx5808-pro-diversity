#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>

#include "settings.h"
#include "timer.h"


#include "CompositeGraphics.h"
#include "Image.h"
#include "CompositeOutput.h"
#include "font8x8.h"
#include "cursor.h"
#include <soc/rtc.h>

#define SCREEN_WIDTH 324
#define SCREEN_HEIGHT 224

#define SCREEN_WIDTH_MID ((SCREEN_WIDTH / 2) - 1)
#define SCREEN_HEIGHT_MID ((SCREEN_HEIGHT / 2) - 1)

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7

namespace Ui {

    extern CompositeGraphics DMA_ATTR display;
    extern CompositeOutput DMA_ATTR composite;
    //extern Image<CompositeGraphics> luni0;
    extern Font<CompositeGraphics> DMA_ATTR font;

    void compositeCore(void *data);
    //void draw();

    extern bool DMA_ATTR isTvOn;

    extern Timer DMA_ATTR UiTimeOut;

    void setup();
//    void sdToTtvout();
    void update();

    void tvOn();
    void tvOff();
//    void beep();
//    void beep(uint16_t freq);

    void drawCursor();

}

#endif
