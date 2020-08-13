#include "lap_times.h"
#include "settings.h"
#include <Arduino.h>

static uint8_t  DRAM_ATTR _my_node_idx;
static uint8_t  DRAM_ATTR _race_id;
static uint32_t DRAM_ATTR _lap_times[MAX_NODES][MAX_LAP_TIMES];
static uint32_t DRAM_ATTR _best_lap_time;
static uint8_t  DRAM_ATTR _best_lap_idx;


void lap_times_set_nodeidx(uint8_t nodeidx)
{
    if (nodeidx < MAX_NODES)
        _my_node_idx = nodeidx;
}


void lap_times_reset(void)
{
    memset(_lap_times, 0, sizeof(_lap_times));
    _best_lap_idx = UINT8_MAX;
    _best_lap_time = UINT32_MAX;

#if 0
    // Debug:
    _race_id = 5;
    _lap_times[0][0] = 12345;
    _lap_times[0][1] = 57*1000 + 1234;
    _lap_times[0][2] = 2 * 60000 + 32*1000 + 1234;
    _lap_times[0][3] = 3600000 + 45 * 60000 + 32*1000 + 1234;
    _lap_times[0][4] = 2*3600000 + 12 * 60000 + 55*1000 + 432;
#endif
}


void lap_times_handle(esp_now_send_lap_s * lap_info)
{
    switch (lap_info->type) {
        case ESPNOW_TYPE_RECE_START:
            lap_times_reset();
            _race_id = lap_info->race_id;
            break;
        case ESPNOW_TYPE_LAP_TIME: {
#if DEBUG_ENABLED
            Serial.printf("Race ID: %u, Node: %u, Lap: %u, time: %u\n",
                        lap_info->race_id, lap_info->node, lap_info->lap, lap_info->lap_time);
#endif
            if (_race_id == lap_info->race_id && _my_node_idx == lap_info->node) {
                if (lap_info->lap < MAX_LAP_TIMES) {
                    /* Collect best lap */
                    if (0 < lap_info->lap && lap_info->lap_time < _best_lap_time) {
                        _best_lap_idx = lap_info->lap;
                        _best_lap_time = lap_info->lap_time;
                    }
                    /* Store lap time */
                    _lap_times[lap_info->node][lap_info->lap] = lap_info->lap_time;
                }
            }
            break;
        }
        default:
            break;
    }
}

uint32_t lapt_time_race_idx_get(void)
{
    return _race_id;
}

uint32_t lapt_time_laptime_get(uint8_t lap)
{
    if (MAX_LAP_TIMES <= lap)
        return UINT32_MAX;
    return _lap_times[_my_node_idx][lap];
}
