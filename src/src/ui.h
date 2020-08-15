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

#define UI_MID_X            ((Ui::XRES / 2) - 1)
#define UI_MID_Y            ((Ui::YRES / 2) - 1)
#define UI_GET_MID_X(_c)    ((Ui::XRES - 8 * (_c)) / 2)

#define WHITE (100u)
#define BLACK (0u)

namespace Ui {

    constexpr int XRES = 324;
    constexpr int YRES = 224;

    constexpr int CHAR_W = 8;
    constexpr int CHAR_H = 8;

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
