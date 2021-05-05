#include "settings.h"
#include "settings_eeprom.h"
#include "ui.h"
#include "receiver_spi.h"
#include "receiver.h"
#include "touchpad.h"
#include "font8x8.h"

#include "CompositeOutput.h"

#include <soc/rtc.h>
#include <stdint.h>

namespace Ui {

    bool DMA_ATTR isTvOn;

    Timer DMA_ATTR UiTimeOut = Timer(2000);

    CompositeGraphics DMA_ATTR display(XRES, YRES);
    CompositeOutput DMA_ATTR composite(CompositeOutput::VIDEO_MODE, XRES * 2, YRES * 2);
    Font<CompositeGraphics> DMA_ATTR font(8, 8, font8x8::pixels);

    static TaskHandle_t uitask;
    static TaskHandle_t compositeTask;


    static void IRAM_ATTR compositeCore(void *data)
    {
        char ** frame = display.get_frame();
        while (true) {
            //just send the graphics frontbuffer whithout any interruption
            composite.sendFrameHalfResolution(frame);
            frame = display.end();
            // Notify other task to draw the next buffer
            xTaskNotify(uitask, 0, eNoAction);
        }
    }


    void setup()
    {
        isTvOn = false;

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

    void IRAM_ATTR draw()
    {
        // Wait until current buffer is fetched
        if (pdTRUE == xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(200))) {
            //display.end();
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
}
