#ifndef WEBSITESERVER_H
#define WEBSITESERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <Preferences.h>

#include "rcmv3.h"

AsyncWebServer server(80);
Preferences prefs;

const unsigned long wifiConnectionTimeout = 10000;
enum WifiMethod {
    STA,
    AP,
    FALLBACK_AP
};
WifiMethod wifiMethod = FALLBACK_AP;
boolean hasWifiConnected = false;

boolean hasWebsiteLoaded = false;

extern void rslOn();
extern void rslOff();

void connectToWifi()
{
    wifiMethod = FALLBACK_AP;

    hasWifiConnected = false;

    delay(100);
    prefs.begin("wifiSettings", true, nvsPartition);

    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_MODE_NULL);

    if (prefs.isKey("ssid") && prefs.isKey("password") && prefs.isKey("mode") && prefs.isKey("hostname")) {
        WiFi.setHostname(prefs.getString("hostname").c_str());
        if (prefs.getInt("mode") == WIFI_STA) {
            wifiMethod = STA;
        } else {
            wifiMethod = AP;
        }
    } else {
        wifiMethod = FALLBACK_AP;
    }

    if (digitalRead(0) == LOW) { // holding IO0 on boot forces fallback AP mode
        wifiMethod = FALLBACK_AP;
    }

    while (!hasWifiConnected) {
        switch (wifiMethod) {
        case STA: {
            WiFi.mode(WIFI_STA);
            WiFi.begin(prefs.getString("ssid").c_str(), prefs.getString("password").c_str());
#ifdef RCM_MCU_NOU3
            WiFi.setTxPower(WIFI_POWER_8_5dBm); // fix for wifi on nou3 thanks @torchtopher from mini FRC
#endif
            unsigned long startConnectionMillis = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - startConnectionMillis) <= wifiConnectionTimeout) {
                rslOn();
                delay(34);
                rslOff();
                delay(66);
            }

            if (WiFi.status() == WL_CONNECTED) {
                hasWifiConnected = true;
                Serial.print("\nConnected to network ");
                Serial.print(prefs.getString("ssid"));
                Serial.print(" with password ");
                Serial.print(prefs.getString("password"));
                Serial.print(" IP address ");
                Serial.println(WiFi.localIP());
            } else {
                wifiMethod = FALLBACK_AP;
            }
            break;
        }
        case AP: {
            WiFi.mode(WIFI_AP);
            WiFi.softAP(prefs.getString("ssid").c_str(), prefs.getString("password").c_str());
#ifdef RCM_MCU_NOU3
            WiFi.setTxPower(WIFI_POWER_8_5dBm); // fix for wifi on nou3 thanks @torchtopher from mini FRC
#endif
            hasWifiConnected = true;
            Serial.print("\nCreated AP with SSID ");
            Serial.print(prefs.getString("ssid"));
            Serial.print(" and password ");
            Serial.print(prefs.getString("password"));
            Serial.print(" IP address ");
            Serial.println(WiFi.softAPIP());
            break;
        }
        case FALLBACK_AP:
        default: {
            WiFi.mode(WIFI_AP);
            WiFi.softAP("http://rcmv3.local");
#ifdef RCM_MCU_NOU3
            WiFi.setTxPower(WIFI_POWER_8_5dBm); // fix for wifi on nou3 thanks @torchtopher from mini FRC
#endif
            hasWifiConnected = true;
            Serial.print("\nCreated open AP with SSID http://rcmv3.local and IP address ");
            Serial.println(WiFi.softAPIP());
            break;
        }
        }
    }

    if (wifiMethod == FALLBACK_AP) {
        MDNS.begin("rcmv3");
        Serial.print("Hostname: ");
        Serial.println("rcmv3");
    } else {
        MDNS.begin(prefs.getString("hostname").c_str());
        Serial.print("Hostname: ");
        Serial.println(prefs.getString("hostname"));
    }

    prefs.end();
}

void startWebServer()
{
    // Initialize SPIFFS
    if (!LittleFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
    }

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        hasWebsiteLoaded = true;
        request->send(LittleFS, "/index.html", "text/html");
    });

    // Route to load script.js file
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/script.js", "text/javascript");
    });

    // Route to load style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/style.css", "text/css");
    });

    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/favicon.ico", "image/x-icon");
    });

    // save driverstation data received in post request and save the json to LittleFS
    server.on("/saveUI", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("UIdata", true)) {
            const AsyncWebParameter* p = request->getParam("UIdata", true);
            prefs.begin("uiSettings", false, nvsPartition);
            boolean success = prefs.putBytes("uidata", p->value().c_str(), p->value().length());
            prefs.end();
            if (success) {
                request->send(200, "text/plain", "OK");
            } else {
                request->send(200, "text/plain", "FAIL to save data");
            }
        } else {
            request->send(200, "text/plain", "FAIL to receive data");
        }
    });

    server.on("/saveWifiSettings", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("ssid", true)) {
            const AsyncWebParameter* p = request->getParam("ssid", true);
            prefs.begin("wifiSettings", false, nvsPartition);
            prefs.putString("ssid", p->value().c_str());
            prefs.end();
        }
        if (request->hasParam("password", true)) {
            const AsyncWebParameter* p = request->getParam("password", true);
            prefs.begin("wifiSettings", false, nvsPartition);
            prefs.putString("password", p->value().c_str());
            prefs.end();
        }
        if (request->hasParam("hostname", true)) {
            const AsyncWebParameter* p = request->getParam("hostname", true);
            prefs.begin("wifiSettings", false, nvsPartition);
            prefs.putString("hostname", p->value().c_str());
            prefs.end();
        }
        if (request->hasParam("mode", true)) {
            const AsyncWebParameter* p = request->getParam("mode", true);
            prefs.begin("wifiSettings", false, nvsPartition);
            prefs.putInt("mode", p->value().toInt());
            prefs.end();
        }
        request->send(200, "text/plain", "OK");
        delay(1000);
        ESP.restart();
    });

    server.on("/loadWifiSettings.json", HTTP_GET, [](AsyncWebServerRequest* request) {
        prefs.begin("wifiSettings", true, nvsPartition);
        if (prefs.isKey("ssid") && prefs.isKey("password") && prefs.isKey("mode") && prefs.isKey("hostname")) {
            String ssid = prefs.getString("ssid");
            String password = prefs.getString("password");
            String hostname = prefs.getString("hostname");
            int mode = prefs.getInt("mode");
            String json = "{\"ssid\":\"" + ssid + "\",\"password\":\"" + password + "\",\"hostname\":\"" + hostname + "\",\"mode\":" + String(mode) + "}";
            request->send(200, "application/json", json);
        } else {
            request->send(200, "application/json", "{}");
        }
        prefs.end();
    });
    server.on("/loadUI.json", HTTP_GET, [](AsyncWebServerRequest* request) {
        prefs.begin("uiSettings", true, nvsPartition);
        if (prefs.isKey("uidata")) {
            size_t len = prefs.getBytesLength("uidata"); // Preferences library allows long byte arrays but limits strings
            char buf[len];
            prefs.getBytes("uidata", buf, len);
            request->send(200, "application/json", buf);
        } else {
            request->send(200, "application/json", "{}");
        }
        prefs.end();
    });

    server.on("/saveMiscConfigInfo", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("miscConfigInfo", true)) {
            const AsyncWebParameter* p = request->getParam("miscConfigInfo", true);
            prefs.begin("miscConfigInfo", false, nvsPartition);
            boolean success = prefs.putBytes("miscConfigInfo", p->value().c_str(), p->value().length());
            prefs.end();
            if (success) {
                request->send(200, "text/plain", "OK");
            } else {
                request->send(200, "text/plain", "FAIL to save data");
            }
        } else {
            request->send(200, "text/plain", "FAIL to receive data");
        }
    });

    server.on("/loadMiscConfigInfo.json", HTTP_GET, [](AsyncWebServerRequest* request) {
        prefs.begin("miscConfigInfo", true, nvsPartition);
        if (prefs.isKey("miscConfigInfo")) {
            size_t len = prefs.getBytesLength("miscConfigInfo"); // Preferences library allows long byte arrays but limits strings
            char buf[len];
            prefs.getBytes("miscConfigInfo", buf, len);
            request->send(200, "application/json", buf);
        } else {
            request->send(200, "application/json", "{}");
        }
        prefs.end();
    });

    server.on("/saveConfig", HTTP_POST, [](AsyncWebServerRequest* request) {
        RCMV3_website_save_config(request);
    });

    server.on("/loadConfig.json", HTTP_GET, [](AsyncWebServerRequest* request) {
        RCMV3_website_load_config(request);
    });

    server.on("/loadBoardInfo.json", HTTP_GET, [](AsyncWebServerRequest* request) {
        RCMV3_website_load_board_info(request);
    });

    server.on("/saveConfigToMemory", HTTP_GET, [](AsyncWebServerRequest* request) {
        save_config_to_memory();
        request->send(200, "text/plain", "OK");
    });

    server.serveStatic("/presets", LittleFS, "/presets");

    server.begin();
}

#endif // WEBSITESERVER_H
