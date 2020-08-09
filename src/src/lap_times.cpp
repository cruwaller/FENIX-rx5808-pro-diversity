#include "lap_times.h"
#include "settings.h"
#include <Arduino.h>

static uint8_t race_id = 0;
static uint32_t _lap_times[6][100];


void lap_times_reset(void)
{
    memset(_lap_times, 0, sizeof(_lap_times));
}


void lap_times_handle(esp_now_send_lap_s * lap_info)
{
    switch (lap_info->type) {
        case ESPNOW_TYPE_RECE_START:
            lap_times_reset();
            race_id = lap_info->race_id;
            break;
        case ESPNOW_TYPE_LAP_TIME: {
#if DEBUG_ENABLED
            Serial.printf("Race ID: %u, Node: %u, Lap: %u, time: %u\n",
                        lap_info->race_id, lap_info->node, lap_info->lap, lap_info->lap_time);
#endif
            if (race_id == lap_info->race_id) {
                if (lap_info->node < 6 && lap_info->lap < 100)
                    _lap_times[lap_info->node][lap_info->lap] = lap_info->lap_time;
            }
            break;
        }
        default:
            break;
    }
}

uint32_t lapt_time_race_idx_get(void)
{
    return race_id;
}

uint32_t lapt_time_laptime_get(uint8_t node_index, uint8_t lap)
{
    if (6 <= node_index || 100 <= lap)
        return UINT32_MAX;
    return _lap_times[node_index][lap];
}
