#include "comm_espnow.h"
#include "lap_times.h"
#include "settings.h"
#include "protocol_ExpressLRS.h"
#include "protocol_chorus.h"
#include "channels.h"
#include "receiver.h"
#include "settings_eeprom.h"
#include "msptypes.h"
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#ifndef ESP_NOW_CHANNEL
#define ESP_NOW_CHANNEL 1
#endif


uint8_t DMA_ATTR elrs_peers[][ESP_NOW_ETH_ALEN] = {
#if defined(ESP_NOW_PEERS_ELRS)
    ESP_NOW_PEERS_ELRS
#endif
};
constexpr uint8_t ELRS_PEERS_CNT = sizeof(elrs_peers) / ESP_NOW_ETH_ALEN;

uint8_t DMA_ATTR chorus_peers[][ESP_NOW_ETH_ALEN] = {
#if defined(ESP_NOW_PEERS_CHORUS)
    ESP_NOW_PEERS_CHORUS
#endif
};
constexpr uint8_t CHORUS_PEERS_CNT = sizeof(chorus_peers) / ESP_NOW_ETH_ALEN;

// This is used to for parse ongoing MSP packet
uint8_t DMA_ATTR msp_tx_buffer[250];

MSP msp_parser;


static void esp_now_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    int iter;
    bool msp_rcvd = 0;

    /* No data or peer is unknown => ignore */
    if (!data_len || !esp_now_is_peer_exist(mac_addr))
        return;

#if DEBUG_ENABLED
    Serial.printf("ESP NOW: ");
#endif

    msp_rcvd = msp_parser.processReceivedByte(data[0]);

    // Check if message is MSP
    for (iter = 1; (iter < data_len) && (msp_rcvd == 0) && msp_parser.mspOngoing(); iter++) {
        msp_rcvd = msp_parser.processReceivedByte(data[iter]);
    }

    // Only MSP packet or laptime is expected
    if (msp_rcvd) {
        /* Process the received MSP packet */
        mspPacket_t &msp_in = msp_parser.getPacket();
        if (msp_in.type == MSP_PACKET_V1_ELRS) {
            uint8_t * payload = (uint8_t*)msp_in.payload;
            switch (msp_in.function) {
                case ELRS_INT_MSP_PARAMS: {
#if DEBUG_ENABLED
                    Serial.println("ELRS_INT_MSP_PARAMS");
#endif
                    expresslrs_params_update(payload[0], payload[1], payload[2], payload[3], payload[4]);
                    break;
                }
            };
        } else if (msp_in.type == MSP_PACKET_V2_COMMAND) {
            switch (msp_in.function) {
                case MSP_SET_VTX_CONFIG: {
                    uint16_t freq = msp_in.payload[1];
                    freq <<= 8;
                    freq += msp_in.payload[0];
                    Receiver::setChannelByFreq(freq);
                    EepromSettings.startChannel = Receiver::activeChannel;
                    EepromSettings.markDirty();
                    break;
                }
            }
        }

    // Check if string command sent by Chorus
    } else if ((0 > chorus_command_handle(data, data_len)) &&
               (sizeof(esp_now_send_lap_s) == data_len)) {
        // command is specific ESP-NOW command
        esp_now_send_lap_s * lap_info = (esp_now_send_lap_s*)data;
        lap_times_handle(lap_info);

#if DEBUG_ENABLED
    } else {
        if (strnlen((char*)data, 250) < 250) {
            Serial.print("[TXT] '");
            Serial.print((char*)data);
            Serial.println("'");
        } else {
            Serial.println("message unknown!");
        }
#endif
    }

    msp_parser.markPacketFree();
}


void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
#if DEBUG_ENABLED && 1
  Serial.print("ESPNOW Sent: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
#endif
}

void comm_espnow_init(void)
{
    // Init values
    expresslrs_params_update(0xff, 0xff, 0xff, 0xff, 0xff);
    lap_times_reset();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg); //initiate and allocate wifi resources (does not matter if connection fails)

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);
    WiFi.enableSTA(false);
    WiFi.enableAP(false);
    WiFi.disconnect(true, true);
    delay(1000);

    //WiFi.mode(WIFI_MODE_APSTA);
    WiFi.mode(WIFI_MODE_STA);
#if DEBUG_ENABLED
    // STA MAC address: D8:A0:1D:4C:72:18

    //Serial.print("My MAC address: ");
    //Serial.println(WiFi.macAddress());
    Serial.printf("STA MAC Address: %s\n", WiFi.macAddress().c_str());
    Serial.printf("AP MAC Address: %s\n", WiFi.softAPmacAddress().c_str());
#endif

#if DEBUG_ENABLED
    Serial.println("ESPNOW initialize...");
#endif
    if (esp_now_init() == ESP_OK) {
        esp_now_register_send_cb(esp_now_send_cb);
        esp_now_register_recv_cb(esp_now_recv_cb);

        esp_now_peer_info_t peer_info = {
            .peer_addr = {0},
            .lmk = {0},
            .channel = ESP_NOW_CHANNEL,
            .ifidx = ESP_IF_WIFI_STA,
            .encrypt = 0,
            .priv = NULL
        };

        uint8_t iter;

        for (iter = 0; iter < ELRS_PEERS_CNT; iter++) {
            memcpy(peer_info.peer_addr, elrs_peers[iter], ESP_NOW_ETH_ALEN);
            esp_err_t err = esp_now_add_peer(&peer_info);
            if (ESP_OK != err) {
#if DEBUG_ENABLED
                Serial.printf("Failed to add ELRS peer[%u], error: %d\r\\n", iter, (int)err);
#endif
            }
        }

        for (iter = 0; iter < CHORUS_PEERS_CNT; iter++) {
            memcpy(peer_info.peer_addr, chorus_peers[iter], ESP_NOW_ETH_ALEN);
            esp_err_t err = esp_now_add_peer(&peer_info);
            if (ESP_OK != err) {
#if DEBUG_ENABLED
                Serial.printf("Failed to add Chorus peer[%u], error: %d\r\\n", iter, (int)err);
#endif
            }
        }

        expresslrs_params_get();    // Get params from ELRS
        chorus_race_init();         // Init Chorus32 communication
#if DEBUG_ENABLED
        Serial.println(" ... init DONE");
    } else {
        Serial.println("ESPNOW init failed!");
#endif
    }
}

void comm_espnow_deinit(void)
{
    uint8_t iter;
    for (iter = 0; iter < ELRS_PEERS_CNT; iter++) {
        if (ESP_OK != esp_now_del_peer(elrs_peers[iter])) {
#if DEBUG_ENABLED
            Serial.printf("Failed to remove ELRS peer[%u], error: %d\r\n", iter);
#endif
        }
    }
    for (iter = 0; iter < CHORUS_PEERS_CNT; iter++) {
        if (ESP_OK != esp_now_del_peer(chorus_peers[iter])) {
#if DEBUG_ENABLED
            Serial.printf("Failed to remove Chorus peer[%u], error: %d\r\n", iter);
#endif
        }
    }
    if (ESP_OK != esp_now_deinit()) {
#if DEBUG_ENABLED
        Serial.printf("ESPNOW deinit failed\n");
#endif
    }
}

void comm_espnow_send_msp(mspPacket_t * packet, comm_espnow_receiver_e rcvr)
{
    // Pack and send packet
    uint8_t len = msp_parser.sendPacket(packet, msp_tx_buffer);
    comm_espnow_send((uint8_t*)msp_tx_buffer, len, rcvr);
}

void comm_espnow_send(uint8_t * buffer, uint8_t len, comm_espnow_receiver_e rcvr)
{
    uint8_t iter;
    if (len && len <= 250) {
        if (rcvr == ESPNOW_ALL) {
            esp_now_send(NULL, buffer, len);
        } else if (rcvr == ESPNOW_ELRS) {
            for (iter = 0; iter < ELRS_PEERS_CNT; iter++) {
                esp_now_send(elrs_peers[iter], buffer, len);
            }
        } else if (rcvr == ESPNOW_CHORUS) {
            for (iter = 0; iter < CHORUS_PEERS_CNT; iter++) {
                esp_now_send(chorus_peers[iter], buffer, len);
            }
        }
    }
}
