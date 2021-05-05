#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <Arduino.h>
#include <stdlib.h>

#include "settings.h"
#include "timer.h"
#include "CompositeGraphics.h"


#define UI_MID_X            ((Ui::XRES / 2) - 1)
#define UI_MID_Y            ((Ui::YRES / 2) - 1)
#define UI_GET_MID_X(_c)    ((Ui::XRES - 8 * (_c)) / 2)

#define WHITE (100u)
#define BLACK (0u)

#define SPACE_BEF 3 //4
#define SPACE_AFT 2 //3

#define AREA_X_START(_x)        ((_x) - SPACE_BEF)
#define AREA_X_END(_x, _cnt)    ((_x) + SPACE_AFT + ((_cnt) * Ui::CHAR_W))
#define AREA_X_LEN(_cnt)        (SPACE_BEF + SPACE_AFT + ((_cnt) * Ui::CHAR_W))
#define AREA_Y_START(_y)        ((_y) - SPACE_BEF)
#define AREA_Y_END(_y, _cnt)    ((_y) + SPACE_AFT + ((_cnt) * Ui::CHAR_H))
#define AREA_Y_LEN(_cnt)        (SPACE_BEF + SPACE_AFT + ((_cnt) * Ui::CHAR_H))


namespace Ui {

#if VIDEO_MODE == PAL
    constexpr int XRES = 324;
    constexpr int YRES = 269; // 576 are visible
#else
    constexpr int XRES = 324;
    constexpr int YRES = 224; // 486 are visible
#endif
    constexpr int CHAR_W = 8;
    constexpr int CHAR_H = 8;

    extern CompositeGraphics DMA_ATTR display;

    extern bool DMA_ATTR isTvOn;
    extern Timer DMA_ATTR UiTimeOut;

    void setup();
    void deinit();

    void reset();
    void draw();

    void tvOn();
    void tvOff();
}

#endif
