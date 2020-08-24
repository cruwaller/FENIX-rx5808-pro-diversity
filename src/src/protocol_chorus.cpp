#include "protocol_chorus.h"
#include "comm_espnow.h"
#include "lap_times.h"

#include <esp_attr.h>

// output id byte constants
#define RESPONSE_WAIT_FIRST_LAP      '1'
#define RESPONSE_BAND                'B'
#define RESPONSE_CHANNEL             'C'
#define RESPONSE_FREQUENCY           'F'
#define RESPONSE_THRESHOLD_SETUP     'H'
#define RESPONSE_RSSI_MON_INTERVAL   'I'
#define RESPONSE_TIME_ADJUSTMENT     'J'
#define RESPONSE_LAPTIME             'L'
#define RESPONSE_RACE_MODE           'R'
#define RESPONSE_MIN_LAP_TIME        'M'
#define RESPONSE_SOUND               'S'
#define RESPONSE_THRESHOLD           'T'
#define RESPONSE_PILOT_ACTIVE        'A'
#define RESPONSE_EXPERIMENTAL_MODE   'E'

#define RESPONSE_API_VERSION         '#'
#define RESPONSE_RSSI                'r'
#define RESPONSE_TIME                't'
#define RESPONSE_VOLTAGE             'v'
#define RESPONSE_END_SEQUENCE        'x'
#define RESPONSE_IS_CONFIGURED       'y'
#define RESPONSE_PING                '%'

/*************** HELPERS *****************/

#define TO_BYTE(i) (i <= '9' ? i - 0x30 : i - 0x41 + 10)
#define TO_HEX(i) (i <= 9 ? 0x30 + i : 0x41 + i - 10)

uint8_t IRAM_ATTR HEX_TO_BYTE (uint8_t hi, uint8_t lo) {
  return TO_BYTE(hi) * 16 + TO_BYTE(lo);
}

uint16_t IRAM_ATTR HEX_TO_UINT16 (uint8_t * buf) {
  return (HEX_TO_BYTE(buf[0], buf[1]) << 8) + (HEX_TO_BYTE(buf[2], buf[3]));
}

uint32_t IRAM_ATTR HEX_TO_UINT32 (uint8_t * buf) {
  return (HEX_TO_BYTE(buf[0], buf[1]) << 24) | (HEX_TO_BYTE(buf[2], buf[3]) << 16) | (HEX_TO_BYTE(buf[4], buf[5]) << 8) | (HEX_TO_BYTE(buf[6], buf[7]));
}

int32_t IRAM_ATTR HEX_TO_SIGNED_LONG (uint8_t * buf) {
#define LEN 8
  int32_t temp = 0;
  for (int i = 0; i < LEN; i++) {
    temp += TO_BYTE(buf[LEN - 1 - i]) * (int32_t)1 << (i * 4);
  }
  return temp;
}

/*************** FUNCS *****************/

static uint8_t started;
static uint8_t min_lap_time = 5;

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
    uint8_t command[3] = {'l', '*', '\n'};
    // Node: 0...MAX or '*' for all
    command[1] = '0' + lap_times_nodeidx_get();
    comm_espnow_send(command, sizeof(command), ESPNOW_CHORUS);
}


uint8_t chorus_race_lap_time_min(void)
{
    return min_lap_time;
}

void chorus_race_lap_time_min_get(void)
{
    uint8_t command[4] = {'R', (uint8_t)('0'+lap_times_nodeidx_get()), RESPONSE_MIN_LAP_TIME, '\n'};
    comm_espnow_send(command, sizeof(command), ESPNOW_CHORUS);
}

void chorus_race_lap_time_min_change(int val)
{
    char command[8];
    min_lap_time += val;
    snprintf(command, sizeof(command), "R%uM%02X\n", lap_times_nodeidx_get(), min_lap_time);
    comm_espnow_send((uint8_t*)command, strlen(command), ESPNOW_CHORUS);
}

void chorus_command_handle(uint8_t const * buff, uint8_t const len)
{
    // Chorus commands
    switch (buff[2]) {
        case RESPONSE_MIN_LAP_TIME: {
            min_lap_time = HEX_TO_BYTE(buff[3], buff[4]);
            break;
        }
        default:
            // just ignore
            break;
    }
}
