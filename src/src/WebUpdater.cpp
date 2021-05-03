#include "WebUpdater.h"
#include "settings.h"
#include "comm_espnow.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <esp_wifi.h>

#ifndef HTTP_SERVER_PORT
#define HTTP_SERVER_PORT 80
#endif

const char *host = "fenix";

WebServer server(HTTP_SERVER_PORT);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";


static void handleMacAddress(void)
{
    uint8_t primaryChan;
    wifi_second_chan_t secondChan;
    esp_wifi_get_channel(&primaryChan, &secondChan);
    (void)secondChan;

    String message = "WiFi STA MAC: ";
    message += WiFi.macAddress();
    message += "\n  - channel in use: ";
    message += primaryChan;
    message += "\n  - mode: ";
    message += (uint8_t)WiFi.getMode();
    message += "\n\nWiFi SoftAP MAC: ";
    message += WiFi.softAPmacAddress();
    message += "\n  - IP: ";
    message += WiFi.softAPIP().toString();
    message += "\n";
    server.send(200, "text/plain", message);
}

static TaskHandle_t wifi_http_task = NULL;

static void httpsTask(void *pvParameters)
{
#if DEBUG_ENABLED
    Serial.println("HTTP task started...");
#endif

    server.begin();
    for(;;) {
        server.handleClient();
        vTaskDelay(10);
    }
    wifi_http_task = NULL;

#if DEBUG_ENABLED
    Serial.println("HTTP task exit");
#endif
}


uint8_t WiFiConnect(void)
{
#if defined(WIFI_SSID) && defined(WIFI_PSK)
    uint32_t iter = 0, timeout;
#if DEBUG_ENABLED
    Serial.println("Connecting to WiFi " WIFI_SSID);
#endif

    timeout = WIFI_TIMEOUT * 2;
    timeout /= 10;

    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);

    while (!WiFi.isConnected() && (iter++ < timeout)) {
        yield();
        vTaskDelay(100);
        if ((iter % 10) == 0) {
            Serial.print(".");
        }
    }

    if (WiFi.isConnected()) {
#if DEBUG_ENABLED
        Serial.println("    CONNECTED!");
#endif
        /* Start update service only when connected */
        BeginWebUpdate();
        return 1;
    }
#if DEBUG_ENABLED
    Serial.println("    FAILED!");
#endif
#endif // defined(WIFI_SSID) && defined(WIFI_PSK)
    return 0;
}


void BeginWebUpdate(void)
{
    if (wifi_http_task != NULL) {
#if DEBUG_ENABLED
        Serial.println("HTTP task already started");
#endif
        return;
    }

    if (!WiFi.isConnected()) {
#if DEBUG_ENABLED
        Serial.println("Not connected! Starting AP...");
#endif
        comm_espnow_deinit();
        WiFi.mode(WIFI_AP);
        WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PSK);
    }

    if (MDNS.begin(host)) {
        MDNS.addService("http", "tcp", HTTP_SERVER_PORT);
    }

    server.on("/", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", serverIndex);
    });
    server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
#if DEBUG_ENABLED
            Serial.setDebugOutput(true);
            Serial.printf("Update: %s\n", upload.filename.c_str());
#endif
            if (!Update.begin()) { //start with max available size
#if DEBUG_ENABLED
                Update.printError(Serial);
#endif
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
#if DEBUG_ENABLED
                Update.printError(Serial);
#endif
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
#if DEBUG_ENABLED
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
#endif
            } else {
#if DEBUG_ENABLED
                Update.printError(Serial);
#endif
            }
#if DEBUG_ENABLED
            Serial.setDebugOutput(false);
#endif
        } else {
#if DEBUG_ENABLED
            Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
#endif
        }
    });

    server.on("/mac", handleMacAddress);

    xTaskCreatePinnedToCore(
        httpsTask,              // Function to implement the task
        "wifiTask",             // Name of the task
        4096,                   // Stack size in bytes
        NULL,                   // Task input parameter
        (tskIDLE_PRIORITY + 1), // Priority of the task
        &wifi_http_task, 0);

#if DEBUG_ENABLED
    Serial.println("HTTP task started");

    Serial.printf("Open http://%s.local in your browser.\r\n", host);
#endif
}

void HandleWebUpdate(void)
{
}
