#ifndef WEBSITESERVER_H
#define WEBSITESERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Preferences.h>

AsyncWebServer server(80);
Preferences wifiSettings;

void connectToWifi()
{
    // connect to ssid "router" with password "password"
    wifiSettings.begin("wifiSettings", false);
    wifiSettings.putString("ssid", "router");
    wifiSettings.putString("password", "password");
    wifiSettings.putInt("mode", WIFI_STA);
    wifiSettings.end();

    Serial.println("Connecting to wifi");
    Serial.println(wifiSettings.getString("ssid"));
    Serial.println(wifiSettings.getString("password"));

    wifiSettings.begin("wifiSettings", false);
    if (wifiSettings.isKey("ssid") && wifiSettings.isKey("password") && wifiSettings.isKey("mode")) {
        if (wifiSettings.getInt("mode") == WIFI_STA) {
            WiFi.mode(WIFI_STA);
            WiFi.begin(wifiSettings.getString("ssid").c_str(), wifiSettings.getString("password").c_str());
        }
    } else {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("RCMv3");
    }
    if (wifiSettings.getInt("mode") == WIFI_STA) {
        Serial.println("Connecting to WiFi ");
        while (WiFi.status() != WL_CONNECTED) {
            delay(100); // TODO:
            Serial.print(".");
        }
        Serial.println();

        Serial.println(WiFi.localIP());
    } else {
        Serial.println(WiFi.softAPIP());
    }
    wifiSettings.end(); // TODO
}

void startWebServer()
{
    // Initialize SPIFFS
    if (!LittleFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
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
            Serial.println("post of UIdata");
            AsyncWebParameter* p = request->getParam("UIdata", true);
            File f = LittleFS.open("/UIdata", "w");
            Serial.println(p->value().c_str());
            f.print(p->value().c_str());
            f.close();
            request->send(200, "text/plain", "OK");
        } else {
            request->send(200, "text/plain", "FAIL");
        }
    });

    server.on("/loadUI.json", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/UIdata", "application/json");
    });

    server.begin();
}

#endif // WEBSITESERVER_H
