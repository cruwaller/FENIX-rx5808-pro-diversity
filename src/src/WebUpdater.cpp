#include "WebUpdater.h"
#include "settings.h"
#include "comm_espnow.h"
#include "ui.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>
#include <esp_heap_caps_init.h>


#ifndef HTTP_SERVER_PORT
#define HTTP_SERVER_PORT 80
#endif


#define MDNS_HOST "fenix"

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


static void http_setup(IPAddress &ipaddr)
{
    WiFi.setHostname(MDNS_HOST);
    MDNS.begin(MDNS_HOST);
    MDNS.addService("http", "tcp", HTTP_SERVER_PORT);

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
            Serial.printf("Update: %s\r\n", upload.filename.c_str());
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
                Serial.printf("Update Success: %u\nRebooting...\r\n", upload.totalSize);
#endif
                server.send(200);
                server.send(200);
            } else {
#if DEBUG_ENABLED
                Update.printError(Serial);
#endif
                server.send(500);
            }
#if DEBUG_ENABLED
            Serial.setDebugOutput(false);
#endif
        } else {
#if DEBUG_ENABLED
            Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\r\n", upload.status);
#endif
        }
    });

    server.on("/mac", handleMacAddress);
    server.begin();

#if DEBUG_ENABLED
    Serial.println("HTTP task started");
    Serial.printf("Open http://" MDNS_HOST ".local in your browser.\r\n");
    Serial.print("My IP: ");
    Serial.println(ipaddr);
#endif
}


void WiFiConnect(void)
{
    IPAddress ipaddr;
    comm_espnow_deinit();
    Ui::deinit();

    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

#if defined(WIFI_SSID) && defined(WIFI_PSK)
    uint32_t iter = 0, timeout;
#if DEBUG_ENABLED
    Serial.print("Connecting to WiFi " WIFI_SSID " > ");
#endif

    timeout = WIFI_TIMEOUT * 10;

    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);

    while (!WiFi.isConnected() && (iter++ < timeout)) {
        esp_task_wdt_reset();
        vTaskDelay(100 / portTICK_PERIOD_MS);
#if DEBUG_ENABLED
        if ((iter % 10) == 0) {
            Serial.print(".");
        }
#endif
    }

    if (WiFi.isConnected()) {
        ipaddr = WiFi.localIP();
#if DEBUG_ENABLED
        Serial.println(" CONNECTED!");
#endif
    } else
#endif // defined(WIFI_SSID) && defined(WIFI_PSK)
    {
#if DEBUG_ENABLED
        Serial.println("WiFi: Starting AP " WIFI_AP_SSID);
#endif
        WiFi.mode(WIFI_AP);
        WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PSK);
        ipaddr = WiFi.softAPIP();
    }

    http_setup(ipaddr);
}


void BeginWebUpdate(void)
{
    WiFiConnect();
}


void HandleWebUpdate(void)
{
    server.handleClient();
    yield();
}
