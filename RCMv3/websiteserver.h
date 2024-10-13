#ifndef WEBSITESERVER_H
#define WEBSITESERVER_H
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
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

    server.begin();
}

#endif // WEBSITESERVER_H
