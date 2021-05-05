#pragma once

#include <freertos/task.h>

enum {
    TASK_PRIO_RSSI = (tskIDLE_PRIORITY+2),
    TASK_PRIO_VOLT = TASK_PRIO_RSSI,
    TASK_PRIO_TOUCH = (tskIDLE_PRIORITY+3),
    TASK_PRIO_OSD = (tskIDLE_PRIORITY+4),
};
