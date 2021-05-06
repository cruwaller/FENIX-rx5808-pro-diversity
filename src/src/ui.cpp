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

#if NUM_OSD_BUFF
    QueueHandle_t queue_free;
    QueueHandle_t queue_ready;
#endif

    static void IRAM_ATTR compositeCore(void *data)
    {
#if !NUM_OSD_BUFF
        while (true) {
            //just send the graphics frontbuffer whithout any interruption
            composite.sendFrameHalfResolution(display.get_frame());
            // Notify other task to draw the next buffer
            xTaskNotify(uitask, 0, eNoAction);
        }
#else
        char ** frame;
        while (true) {
            xQueueReceive(queue_ready, &frame, portMAX_DELAY);
            //just send the graphics frontbuffer whithout any interruption
            composite.sendFrameHalfResolution(frame);
            xQueueSend(queue_free, &frame, portMAX_DELAY);
        }
#endif
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

#if NUM_OSD_BUFF
        queue_free = xQueueCreate(NUM_OSD_BUFF, sizeof(char**));
        queue_ready = xQueueCreate(NUM_OSD_BUFF, sizeof(char**));
        display.get_frames(queue_free);
#endif

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

    void IRAM_ATTR reset()
    {
#if NUM_OSD_BUFF
        char ** buff = NULL;
        do {
            xQueueReceive(queue_free, &buff, portMAX_DELAY);
        } while(buff == NULL);
        display.set_backbuff(buff);
#endif
        // reset OSD to black
        display.begin(BLACK);
    }

    void IRAM_ATTR draw()
    {
#if NUM_OSD_BUFF
        char ** buff = display.get_backbuff();
        xQueueSend(queue_ready, &buff, portMAX_DELAY);
#else
        display.end();
        // Wait until current buffer is fetched
        if (pdTRUE == xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(200))) {
            //display.end();
        }
#endif
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
