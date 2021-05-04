#include "settings.h"
#include "settings_eeprom.h"
#include "ui.h"
#include "receiver_spi.h"
#include "receiver.h"
#include "touchpad.h"
#include "font8x8.h"
#include "cursor.h"

#include <soc/rtc.h>
#include <stdint.h>

namespace Ui {

    bool DMA_ATTR isTvOn = false;

    Timer DMA_ATTR UiTimeOut = Timer(2000);

    CompositeGraphics DMA_ATTR display(XRES, YRES);
    CompositeOutput DMA_ATTR composite(CompositeOutput::VIDEO_MODE, XRES * 2, YRES * 2);
    Font<CompositeGraphics> DMA_ATTR font(8, 8, font8x8::pixels);

    TaskHandle_t uitask;
    TaskHandle_t compositeTask;

    void setup() {

        // highest clockspeed needed
        //rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
        rtc_cpu_freq_config_t config;
        rtc_clk_cpu_freq_to_config(RTC_CPU_FREQ_240M, &config);
        rtc_clk_cpu_freq_set_config(&config);


        //initializing DMA buffers and I2S
        composite.init();
        //initializing graphics double buffer
        display.init();
        composite.stopOutput(); //stop i2s driver (no video output)
        //select font
        display.setFont(font);

        uitask = xTaskGetCurrentTaskHandle();

        //running composite output pinned to first core
        //  rendering the actual graphics in the main loop is done on the second core by default
        xTaskCreatePinnedToCore(compositeCore, "c",
            1024, NULL, ((configMAX_PRIORITIES-1) | portPRIVILEGE_BIT),
            &compositeTask, 0);
    }

    void deinit() {
        if (compositeTask)
            vTaskSuspend(compositeTask);
        composite.deinit();
        display.deinit();
    }

    void reset()
    {
        // reset OSD to black
        display.begin(BLACK);
    }

    void update()
    {
        drawCursor();
    }

    void draw()
    {
        // Wait until current buffer is fetched
        xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(200));
        //if (pdTRUE == xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(100)))
        //    display.end();
    }

    void compositeCore(void *data)
    {
        char ** frame = display.end();
      while (true)
      {
        //just send the graphics frontbuffer whithout any interruption
        composite.sendFrameHalfResolution(frame);
        // Swap ping-pong buffer
        frame = display.end();
        // Notify other task to draw the next buffer
        xTaskNotify(uitask, 0, eNoAction);
      }
    }

    void tvOn() {

        ReceiverSpi::rxVideoOff(Receiver::ReceiverId::ALL);

        composite.startOutput();

        isTvOn = true;
    }

    void tvOff() {

        ReceiverSpi::rxPowerUp(Receiver::ReceiverId::ALL);

        Receiver::setChannel(Receiver::activeChannel);

        composite.stopOutput();

        isTvOn = false;
    }

    void drawCursor() {

        uint8_t i = 0, py, px, pixelValue;
        int16_t cursor_x = TouchPad::touchData.cursorX, cursor_y = TouchPad::touchData.cursorY;

        for (py = 0; py < Cursor::yres; py++) {
            for (px = 0; px < Cursor::xres; px++) {
                pixelValue = Cursor::pixels[i++];
                if (pixelValue == 255) {
                    display.dot(px + cursor_x, py + cursor_y, 100);
                } else if (pixelValue == 1) {
                    display.dot(px + cursor_x, py + cursor_y, 0);
                }
            }
        }

    //    // For testing print the cursor location.  Helps with positioning widgits.
    //    display.setCursor(cursor_x, cursor_y);
    //    display.print("  [");
    //    display.print(cursor_x);
    //    display.print(", ");
    //    display.print(cursor_y);
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
