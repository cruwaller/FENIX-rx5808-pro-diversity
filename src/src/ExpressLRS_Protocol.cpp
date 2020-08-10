#include "ExpressLRS_Protocol.h"
#include "comm_espnow.h"
#include "settings.h"


void expresslrs_vtx_channel_send(uint8_t channel)
{
    // channel format is 8 * BAND + CHANNEL
    uint8_t payload[4] = {channel, 0, 1, 0};
    comm_espnow_send_msp(MSP_SET_VTX_CONFIG, sizeof(payload), (uint8_t *) &payload);
}

void expresslrs_rate_send(uint8_t rate)
{
    uint8_t payload[] = {ExLRS_MODE, rate};
    comm_espnow_send_msp(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
}

void expresslrs_power_send(uint16_t power)
{
    uint8_t payload[] = {ExLRS_TX_POWER, (uint8_t)(power), (uint8_t)(power >> 8)};
    comm_espnow_send_msp(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
}

void expresslrs_tlm_send(uint8_t tlm)
{
    uint8_t payload[] = {ExLRS_TLM, (uint8_t)(tlm ? ExLRS_TLM_ON : ExLRS_TLM_OFF)};
    comm_espnow_send_msp(FUNC_Ex, sizeof(payload), (uint8_t *) &payload);
}
