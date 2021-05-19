/*
    esp-now setup for communicating to https://github.com/AlessandroAU/ExpressLRS

    3 bytes transferred {group, task, data}
*/

#ifndef EXPRESSLRS_PROTOCOL_H
#define EXPRESSLRS_PROTOCOL_H

#include <stdint.h>

// RF mode
enum {
    ExLRS_RF_MODE_915_AU_FCC = 0,
    ExLRS_RF_MODE_868_EU,
    ExLRS_RF_MODE_433_AU_EU,
    ExLRS_RF_MODE_2400_ISM,
    ExLRS_RF_MODE_2400_ISM_500,
    ExLRS_RF_MODE_2400_ISM_FLRC,
    ExLRS_RF_MODE_MAX,
    ExLRS_RF_MODE_HANDSET   = 0x40,
    ExLRS_RF_MODE_DUAL      = 0x80,
    ExLRS_RF_MODE_MASK      = 0x3F,
    ExLRS_RF_MODE_INVALID   = 0xff,
};

// RC rate
#define ExLRS_RATE_INVALID 0xff
enum {
    ExLRS_RATE_200 = 0, // fastest
    ExLRS_RATE_100,
    ExLRS_RATE_50,
    ExLRS_RATE_MAX,
};
enum {
    ExLRS_RATE_ISM_500 = 0, // fastest
    ExLRS_RATE_ISM_250,
    ExLRS_RATE_ISM_125,
    ExLRS_RATE_ISM_50,
    ExLRS_RATE_ISM_MAX,
};

// TLM
enum
{
    ExLRS_TLM_OFF = 0,
    ExLRS_TLM_RATIO_1_128,
    ExLRS_TLM_RATIO_1_64,
    ExLRS_TLM_RATIO_1_32,
    ExLRS_TLM_RATIO_1_16,
    ExLRS_TLM_RATIO_1_8,
    ExLRS_TLM_RATIO_1_4,
    ExLRS_TLM_RATIO_1_2,
    ExLRS_TLM_RATIO_MAX,
    ExLRS_TLM_ON = 0xff,
};

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
    ExLRS_PWR_MAX
};


void expresslrs_vtx_freq_send(uint16_t freq);
void expresslrs_rate_send(uint8_t rate);
void expresslrs_power_send(uint8_t power);
void expresslrs_tlm_send(uint8_t tlm);
void expresslrs_domain_send(uint8_t rate);

void expresslrs_params_update(uint8_t rate, uint8_t tlm, uint8_t pwr, uint8_t pwr_max, uint8_t region);
void expresslrs_params_get(void);
uint8_t expresslrs_params_get_rate(void);
uint8_t expresslrs_params_get_tlm(void);
uint8_t expresslrs_params_get_power(void);
uint8_t expresslrs_params_get_region(void);

#endif
