#include "protocol_chorus.h"
#include "comm_espnow.h"
#include "lap_times.h"

static uint8_t started;

uint8_t chorus_race_is_start(void)
{
    return started;
}

void chorus_race_state_set(uint8_t state)
{
    started = state;
    if (!state) // end, get lap times
        chorus_race_laps_get();
}

void chorus_number_of_nodes_get(void)
{
    char command[] = "N\n";
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
}

void chorus_race_start(void)
{
    char command[] = "R*R1\n";
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
    //started = 1;
}

void chorus_race_end(void)
{
    char command[] = "R*R0\n";
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
    //started = 0;
}

void chorus_race_laps_get(void)
{
    char command[3] = {'l', '*', '\n'};
    // Node: 0...MAX or '*' for all
    command[1] = '0' + lap_times_nodeidx_get();
    comm_espnow_send((uint8_t*)command, sizeof(command), ESPNOW_CHORUS);
}
