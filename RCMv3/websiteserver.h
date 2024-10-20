#ifndef WEBSITESERVER_H
#define WEBSITESERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <Preferences.h>

AsyncWebServer server(80);
Preferences prefs;

const unsigned long wifiConnectionTimeout = 5000;
enum WifiMethod {
    FALLBACK_AP,
    STA,
    AP
};
WifiMethod wifiMethod = FALLBACK_AP;
boolean hasWifiConnected = false;

boolean hasWebsiteLoaded = false;

void connectToWifi()
{
    wifiMethod = FALLBACK_AP;

    hasWifiConnected = false;

    delay(100);
    prefs.begin("wifiSettings", true);

    if (prefs.isKey("ssid") && prefs.isKey("password") && prefs.isKey("mode") && prefs.isKey("hostname")) {
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
            unsigned long startConnectionMillis = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - startConnectionMillis) <= wifiConnectionTimeout) {
                delay(10);
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
            AsyncWebParameter* p = request->getParam("UIdata", true);
            prefs.begin("uiSettings", false);
            prefs.putString("uidata", p->value().c_str());
            prefs.end();
            request->send(200, "text/plain", "OK");
        } else {
            request->send(200, "text/plain", "FAIL to receive data");
        }
    });

    server.on("/saveWifiSettings", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (request->hasParam("ssid", true)) {
            AsyncWebParameter* p = request->getParam("ssid", true);
            prefs.begin("wifiSettings", false);
            prefs.putString("ssid", p->value().c_str());
            prefs.end();
        }
        if (request->hasParam("password", true)) {
            AsyncWebParameter* p = request->getParam("password", true);
            prefs.begin("wifiSettings", false);
            prefs.putString("password", p->value().c_str());
            prefs.end();
        }
        if (request->hasParam("hostname", true)) {
            AsyncWebParameter* p = request->getParam("hostname", true);
            prefs.begin("wifiSettings", false);
            prefs.putString("hostname", p->value().c_str());
            prefs.end();
        }
        if (request->hasParam("mode", true)) {
            AsyncWebParameter* p = request->getParam("mode", true);
            prefs.begin("wifiSettings", false);
            prefs.putInt("mode", p->value().toInt());
            prefs.end();
        }
        request->send(200, "text/plain", "OK");
        delay(1000);
        ESP.restart();
    });

    server.on("/loadWifiSettings.json", HTTP_GET, [](AsyncWebServerRequest* request) {
        prefs.begin("wifiSettings", true);
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
        prefs.begin("uiSettings", true);
        if (prefs.isKey("uidata")) {
            request->send(200, "application/json", prefs.getString("uidata"));
        } else {
            request->send(200, "application/json", "{}");
        }
        prefs.end();
    });

    server.begin();
}

#endif // WEBSITESERVER_H
