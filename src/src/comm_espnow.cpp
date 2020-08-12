#include "comm_espnow.h"
#include "lap_times.h"
#include "settings.h"
#include "ExpressLRS_Protocol.h"
#include <esp_now.h>
#include <WiFi.h>


#define WIFI_CHANNEL 1

uint8_t broadcastAddress[][ESP_NOW_ETH_ALEN] = {
    {0x5C, 0xCF, 0x7F, 0xAC, 0xD9, 0x0F},   // R9M LOGGER STA: 5C:CF:7F:AC:D9:0F (ESP8266)
    {0x5E, 0xCF, 0x7F, 0xAC, 0xD9, 0x0F},   // R9M LOGGER  AP: 5E:CF:7F:AC:D9:0F (ESP8266)
    //{0xF0, 0x08, 0xD1, 0xD4, 0xED, 0x7C},   // Chorus32 STA: F0:08:D1:D4:ED:7C (ESP32)
    {0xF0, 0x08, 0xD1, 0xD4, 0xED, 0x7D},   // Chorus32  AP: F0:08:D1:D4:ED:7D (ESP32)
};

// This is used to for parse ongoing MSP packet
uint8_t DMA_ATTR msp_tx_buffer[250];

MSP msp_parser;


static void esp_now_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    int iter;
    bool msp_rcvd = 0;

#if DEBUG_ENABLED
    Serial.printf("ESP NOW: ");
#endif

    /* No data, return */
    if (!data_len)
        return;
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
        }

    } else if (data_len == sizeof(esp_now_send_lap_s)) {
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

#if 0
    char hello[] = "FENIX_CB\n";
    esp_now_send(mac_addr, (uint8_t*)hello, strlen(hello)); // send to all registered peers
#endif
}


void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
#if DEBUG_ENABLED && 1
  Serial.print("ESPNOW Sent: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
#endif
}

void comm_espnow_init(void)
{
    expresslrs_params_update(0xff, 0xff, 0xff, 0xff, 0xff);
    lap_times_reset(); // just for debug

    WiFi.mode(WIFI_MODE_APSTA);
#if DEBUG_ENABLED
    // STA MAC address: D8:A0:1D:4C:72:18

    //Serial.print("My MAC address: ");
    //Serial.println(WiFi.macAddress());
    Serial.printf("STA MAC Address: %s\n", WiFi.macAddress().c_str());
    Serial.printf("AP MAC Address: %s\n", WiFi.softAPmacAddress().c_str());
#endif
    WiFi.disconnect();

    Serial.println("ESPNOW initialize...");
    if (esp_now_init() == ESP_OK) {
        esp_now_register_send_cb(esp_now_send_cb);
        esp_now_register_recv_cb(esp_now_recv_cb);

        esp_now_peer_info_t peer_info = {
            .peer_addr = {0},
            .lmk = {0},
            .channel = WIFI_CHANNEL,
            .ifidx = ESP_IF_WIFI_STA,
            .encrypt = 0,
            .priv = NULL
        };

        uint8_t num_peers = sizeof(broadcastAddress) / ESP_NOW_ETH_ALEN;
        for (uint8_t iter = 0; iter < num_peers; iter++) {
            memcpy(peer_info.peer_addr, broadcastAddress[iter], ESP_NOW_ETH_ALEN);
            esp_err_t err = esp_now_add_peer(&peer_info);
            if (ESP_OK != err) {
#if DEBUG_ENABLED
                Serial.printf("Failed to add peer[%u], error: %d\n", iter, (int)err);
#endif
            }
        }

        expresslrs_params_get(); // Get params from ELRS
#if 0
        char hello[] = "FENIX_HELLO\n";
        esp_now_send(NULL, (uint8_t*)hello, strlen(hello)); // send to all registered peers
#endif
#if DEBUG_ENABLED
        Serial.println(" ... init DONE");
    } else {
        Serial.println("ESPNOW init failed!");
#endif
    }
}

void comm_espnow_deinit(void)
{
    if (ESP_OK != esp_now_deinit()) {
#if DEBUG_ENABLED
        Serial.printf("ESPNOW deinit failed\n");
#endif
    }
}

void comm_espnow_send_msp(mspPacket_t * packet)
{
    // Pack and send packet
    uint8_t len = msp_parser.sendPacket(packet, msp_tx_buffer);
    if (len)
        esp_now_send(NULL, (uint8_t*)msp_tx_buffer, len);
}
