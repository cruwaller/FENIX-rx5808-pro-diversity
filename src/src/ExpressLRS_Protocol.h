/*
    esp-now setup for communicating to https://github.com/AlessandroAU/ExpressLRS

    3 bytes transferred {group, task, data}
*/

#ifndef EXPRESSLRS_PROTOCOL_H
#define EXPRESSLRS_PROTOCOL_H

#include <stdint.h>

// Modes
#define ExLRS_50Hz          2
#define ExLRS_100Hz         1
#define ExLRS_200Hz         0

// TLM
#define ExLRS_TLM_OFF       0
#define ExLRS_TLM_ON        0xFF

enum
{
    ExLRS_PWR_DYNAMIC = 0,
    ExLRS_PWR_10mW,
    ExLRS_PWR_25mW,
    ExLRS_PWR_50mW,
    ExLRS_PWR_100mW,
    ExLRS_PWR_250mW,
    ExLRS_PWR_500mW,
    ExLRS_PWR_1000mW,
    ExLRS_PWR_2000mW,
    ExLRS_PWR_UNKNOWN
};


void expresslrs_vtx_channel_send(uint16_t channel);
void expresslrs_rate_send(uint8_t rate);
void expresslrs_power_send(uint8_t power);
void expresslrs_tlm_send(uint8_t tlm);

void expresslrs_params_update(uint8_t rate, uint8_t tlm, uint8_t pwr, uint8_t pwr_max, uint8_t region);
void expresslrs_params_get(void);
uint8_t expresslrs_params_get_rate(void);
uint8_t expresslrs_params_get_tlm(void);
uint8_t expresslrs_params_get_power(void);
uint8_t expresslrs_params_get_region(void);

#endif
