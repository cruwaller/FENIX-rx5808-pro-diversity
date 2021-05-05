#include "lap_times.h"
#include "settings.h"
#include "protocol_chorus.h"
#include <Arduino.h>

#if (FIRST_LAP_IDX < 1) || (MAX_LAP_TIMES < FIRST_LAP_IDX)
#error "Invalid first lap index!"
#endif


typedef struct {
    uint32_t    ms;
    lap_time_t  time;
} lap_times_store_t;

static lap_times_store_t DRAM_ATTR _lap_times[MAX_LAP_TIMES];
static uint32_t DRAM_ATTR _best_lap_time;
static uint8_t  DRAM_ATTR _best_lap_idx;
static uint16_t  DRAM_ATTR _last_lap_idx;

static int8_t  DRAM_ATTR _error;


static lap_time_t IRAM_ATTR convert_ms_time(uint32_t lap_time)
{
    uint32_t secs = lap_time / 1000;
    uint16_t ms = lap_time % 1000;
    uint8_t hours, mins;
    hours = secs / 3600;
    mins  = (secs - (hours * 3600)) / 60;
    secs  = secs - (hours * 3600) - (mins * 60);
    return (lap_time_t){.ms = ms, .s = (uint8_t)secs, .m = mins};
}


void lap_times_reset(void)
{
    memset(_lap_times, 0, sizeof(_lap_times));
    _best_lap_idx = UINT8_MAX;
    _best_lap_time = UINT32_MAX;
    _last_lap_idx = 0;
    _error = 0;

#if 0
    _lap_times[0].ms = 61001;
    _lap_times[1].ms = 8891;
    _lap_times[2].ms = 11899;
    _lap_times[3].ms = 10681;
    _lap_times[4].ms = 11612;
    _lap_times[5].ms = 13550;
    _lap_times[6].ms = 10697;
    _lap_times[7].ms = 10983;
    _lap_times[8].ms = 11148;
    _lap_times[9].ms = 10988;
    _lap_times[10].ms = 10913;
    _lap_times[11].ms = 671011;
    _lap_times[12].ms = 732012;


    _lap_times[0].time.m = 1;
    _lap_times[0].time.s = 1;
    _lap_times[0].time.ms = 1;

    _lap_times[1].time.m = 0;
    _lap_times[1].time.s = 8;
    _lap_times[1].time.ms = 891;

    _lap_times[2].time.m = 0;
    _lap_times[2].time.s = 11;
    _lap_times[2].time.ms = 899;

    _lap_times[3].time.m = 0;
    _lap_times[3].time.s = 10;
    _lap_times[3].time.ms = 681;

    _lap_times[4].time.m = 0;
    _lap_times[4].time.s = 11;
    _lap_times[4].time.ms = 612;

    _lap_times[5].time.m = 0;
    _lap_times[5].time.s = 13;
    _lap_times[5].time.ms = 550;

    _lap_times[6].time.m = 0;
    _lap_times[6].time.s = 10;
    _lap_times[6].time.ms = 697;

    _lap_times[7].time.m = 0;
    _lap_times[7].time.s = 10;
    _lap_times[7].time.ms = 983;

    _lap_times[8].time.m = 0;
    _lap_times[8].time.s = 11;
    _lap_times[8].time.ms = 148;

    _lap_times[9].time.m = 0;
    _lap_times[9].time.s = 10;
    _lap_times[9].time.ms = 988;

    _lap_times[10].time.m = 0;
    _lap_times[10].time.s = 10;
    _lap_times[10].time.ms = 913;

    _last_lap_idx = 10;

    // extra to check bounds
    _lap_times[11].time.m = 11;
    _lap_times[11].time.s = 11;
    _lap_times[11].time.ms = 11;
    _lap_times[12].time.m = 12;
    _lap_times[12].time.s = 12;
    _lap_times[12].time.ms = 12;

#elif 0
    for (uint8_t iter = 0; iter < 16; iter++) {
        // first should be skipped
        _lap_times[iter].time.m = (iter+1);
        _lap_times[iter].time.s = (iter+1);
        _lap_times[iter].time.ms = (iter+1);
        _last_lap_idx = iter;
    }
    _best_lap_idx = 6;
#endif
}


void lap_times_handle(esp_now_send_lap_s * lap_info)
{
    uint32_t lap_time;
#if 0
    uint32_t secs;
    uint16_t ms;
    uint8_t hours, mins;
#endif
    uint8_t node_idx = chorus_nodeidx_get();

    /* Accept only correct node index */
    if (node_idx != lap_info->node)
        return;

    switch (lap_info->type) {
        case ESPNOW_TYPE_RACE_START: {
            chorus_race_state_set(1);
            chorus_race_idx_set(lap_info->race_id);
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
            if (chorus_race_idx_get() == lap_info->race_id) {
                if (lap_info->lap < MAX_LAP_TIMES) {
                    if ((_last_lap_idx + 1) != lap_info->lap)
                        _error = 1;
                    _last_lap_idx = lap_info->lap;

                    lap_time = lap_info->lap_time;

                    /* Collect best lap, ignore initial time */
                    if (1 < lap_info->lap && lap_time < _best_lap_time) {
                        _best_lap_idx = lap_info->lap;
                        _best_lap_time = lap_time;
                    }
#if 0
                    /* Convert time */
                    secs = lap_time / 1000;

                    hours = secs / 3600;
                    mins  = (secs - (hours * 3600)) / 60;
                    secs  = secs - (hours * 3600) - (mins * 60);
                    ms = lap_time % 1000;

                    /* Store lap time */
                    _lap_times[lap_info->lap].ms = lap_time;
                    _lap_times[lap_info->lap].time.ms = ms;
                    _lap_times[lap_info->lap].time.s = secs;
                    _lap_times[lap_info->lap].time.m = mins;
#else
                    _lap_times[lap_info->lap].ms = lap_time;
                    _lap_times[lap_info->lap].time = convert_ms_time(lap_time);
#endif
                }
            }
            break;
        }
        default:
            break;
    }
}

uint8_t IRAM_ATTR lapt_time_race_num_laps(void)
{
    if (_last_lap_idx == 0)
        return 0;
    //return (_last_lap_idx + 1);
    return _last_lap_idx;
}

lap_time_t IRAM_ATTR lapt_time_laptime_get(uint8_t lap, uint8_t &fastest)
{
    fastest = (lap == _best_lap_idx);
    if (MAX_LAP_TIMES <= lap) // range check
        return (lap_time_t){.ms = 0, .s = 0, .m = 0};
    return _lap_times[lap].time;
}

lap_time_t IRAM_ATTR  lapt_time_best_consecutives_get(uint8_t const consecutives, uint8_t &first)
{
    uint32_t jter, sum_avg = 0, best = UINT32_MAX;
    int32_t iter, num_laps = (int32_t)lapt_time_race_num_laps() - consecutives;
    for (iter = 2; iter <= num_laps; iter++) {
        for (jter = 0, sum_avg = 0; iter < consecutives; jter++) {
            sum_avg += _lap_times[iter + jter].ms;
        }
        sum_avg /= consecutives;
        if (0 < sum_avg && sum_avg < best) {
            best = sum_avg;
            first = iter;
        }
    }
    if (best < UINT32_MAX)
        return convert_ms_time(best);
    return (lap_time_t){.ms = 0, .s = 0, .m = 0};
}
