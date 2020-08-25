#include "lap_times.h"
#include "settings.h"
#include "protocol_chorus.h"
#include <Arduino.h>

static uint8_t  DRAM_ATTR _my_node_idx;
static uint8_t  DRAM_ATTR _race_id;
static lap_time_t DRAM_ATTR _lap_times[MAX_LAP_TIMES];
static uint32_t DRAM_ATTR _best_lap_time;
static uint8_t  DRAM_ATTR _best_lap_idx;
static uint16_t  DRAM_ATTR _last_lap_idx;

static int8_t  DRAM_ATTR _error;



void lap_times_nodeidx_set(uint8_t nodeidx)
{
    if (nodeidx < MAX_NODES)
        _my_node_idx = nodeidx;
}


void lap_times_nodeidx_roll(int8_t dir)
{
    if ((_my_node_idx == 0 && dir < 0) || (_my_node_idx == (MAX_NODES-1) && 0 < dir))
        return;
    _my_node_idx += dir;
}


uint8_t lap_times_nodeidx_get(void)
{
    return _my_node_idx;
}


void lap_times_reset(void)
{
    memset(_lap_times, 0, sizeof(_lap_times));
    _best_lap_idx = UINT8_MAX;
    _best_lap_time = UINT32_MAX;
    _last_lap_idx = 0;
    _error = 0;

#if 0
    for (uint8_t iter = 0; iter < 16; iter++) {
        // first should be skipped
        _lap_times[iter].m = (iter+1);
        _lap_times[iter].s = (iter+1);
        _lap_times[iter].ms = (iter+1);
        _last_lap_idx = iter;
    }
    _best_lap_idx = 6;
#endif
}


void lap_times_handle(esp_now_send_lap_s * lap_info)
{
    uint32_t secs;
    uint16_t ms;
    uint8_t hours, mins;
    switch (lap_info->type) {
        case ESPNOW_TYPE_RACE_START: {
            lap_times_reset();
            chorus_race_state_set(1);
            _race_id = lap_info->race_id;
            break;
        }
        case ESPNOW_TYPE_RACE_STOP: {
            chorus_race_state_set(0);
            break;
        }
        case ESPNOW_TYPE_LAP_TIME: {
#if DEBUG_ENABLED
            Serial.printf("Race ID: %u, Node: %u, Lap: %u, time: %u\n",
                          lap_info->race_id, lap_info->node, lap_info->lap, lap_info->lap_time);
#endif
            if (_race_id == lap_info->race_id && _my_node_idx == lap_info->node) {
                if (lap_info->lap < MAX_LAP_TIMES) {
                    if ((_last_lap_idx + 1) != lap_info->lap)
                        _error = 1;
                    _last_lap_idx = lap_info->lap;

                    /* Collect best lap */
                    if (0 < lap_info->lap && lap_info->lap_time < _best_lap_time) {
                        _best_lap_idx = lap_info->lap;
                        _best_lap_time = lap_info->lap_time;
                    }
                    /* Convert time */
                    secs = lap_info->lap_time / 1000;

                    hours = secs / 3600;
                    mins  = (secs - (hours * 3600)) / 60;
                    secs  = secs - (hours * 3600) - (mins * 60);
                    ms = lap_info->lap_time % 1000;

                    /* Store lap time */
                    _lap_times[lap_info->lap].ms = ms;
                    _lap_times[lap_info->lap].s = secs;
                    _lap_times[lap_info->lap].m = mins;
                }
            }
            break;
        }
        default:
            break;
    }
}

uint8_t lapt_time_race_num_laps(void)
{
    if (_last_lap_idx == 0)
        return 0;
    return (_last_lap_idx + 1);
}

uint32_t lapt_time_race_idx_get(void)
{
    return _race_id;
}

lap_time_t lapt_time_laptime_get(uint8_t lap, uint8_t &fastest)
{
    fastest = (lap == _best_lap_idx);
    if (MAX_LAP_TIMES <= lap)
        return (lap_time_t){.ms = 0, .s = 0, .m = 0};
    return _lap_times[lap];
}
