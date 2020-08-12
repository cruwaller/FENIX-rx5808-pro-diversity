#include "ExpressLRS_Protocol.h"
#include "comm_espnow.h"
#include "settings.h"
#include "msp.h"
#include "msptypes.h"

struct elrs_params {
    uint8_t rate;
    uint8_t tlm;
    uint8_t pwr;
    uint8_t pwr_max;
    uint8_t region;
};

struct elrs_params DRAM_ATTR elrs_params;
mspPacket_t DRAM_ATTR msp_out;


void expresslrs_vtx_channel_send(uint16_t channel)
{
    // channel format is 8 * BAND + CHANNEL

    msp_out.type = MSP_PACKET_V2_COMMAND;
    msp_out.flags = MSP_VERSION | MSP_STARTFLAG; // TODO: validate!
    msp_out.function = MSP_SET_VTX_CONFIG;
    msp_out.payloadSize = 4;
    msp_out.payload[0] = (channel & 0xff);
    msp_out.payload[1] = (channel >> 8);
    msp_out.payload[2] = 1; // power (25mW)
    msp_out.payload[3] = 0; // pitmode == (power == 0)
    comm_espnow_send_msp(&msp_out);
}

void expresslrs_rate_send(uint8_t rate)
{
    msp_out.type = MSP_PACKET_V1_ELRS;
    msp_out.flags = MSP_ELRS_INT;
    msp_out.function = ELRS_INT_MSP_PARAMS;
    msp_out.payloadSize = 2;
    msp_out.payload[0] = MSP_ELRS_RF_MODE;
    msp_out.payload[1] = rate;
    comm_espnow_send_msp(&msp_out);
}

void expresslrs_power_send(uint16_t power)
{
    msp_out.type = MSP_PACKET_V1_ELRS;
    msp_out.flags = MSP_ELRS_INT;
    msp_out.function = ELRS_INT_MSP_PARAMS;
    msp_out.payloadSize = 3;
    msp_out.payload[0] = MSP_ELRS_TX_PWR;
    msp_out.payload[1] = (uint8_t)(power);
    msp_out.payload[2] = (uint8_t)(power >> 8);
    comm_espnow_send_msp(&msp_out);
}

void expresslrs_tlm_send(uint8_t tlm)
{
    msp_out.type = MSP_PACKET_V1_ELRS;
    msp_out.flags = MSP_ELRS_INT;
    msp_out.function = ELRS_INT_MSP_PARAMS;
    msp_out.payloadSize = 2;
    msp_out.payload[0] = MSP_ELRS_TLM_RATE;
    //msp_out.payload[1] = tlm;
    msp_out.payload[1] = (uint8_t)(tlm ? ExLRS_TLM_ON : ExLRS_TLM_OFF);
    comm_espnow_send_msp(&msp_out);
}

void expresslrs_params_update(uint8_t rate, uint8_t tlm, uint8_t pwr, uint8_t pwr_max, uint8_t region)
{
    elrs_params.rate = rate;
    elrs_params.tlm = tlm;
    elrs_params.pwr = pwr;
    elrs_params.pwr_max = pwr_max;
    elrs_params.region = region;
}

uint8_t expresslrs_params_get_rate(void)
{
    return elrs_params.rate;
}

uint8_t expresslrs_params_get_tlm(void)
{
    return elrs_params.tlm;
}


uint8_t expresslrs_params_get_power(void)
{
    return elrs_params.pwr;
}


uint8_t expresslrs_params_get_region(void)
{
    return elrs_params.region;
}
