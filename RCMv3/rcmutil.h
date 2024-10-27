#ifndef RCMUTIL_H
#define RCMUTIL_H

// contains functions common to all RCMv2 projects

#include <Arduino.h>
#include <FastLED.h>

#include "rcm.h"
#include "websiteserver.h"
#include "websocketcomms.h"

extern void PowerOn();
extern void Enable();
extern void Disable();
extern void Enabled();
extern void Always();
extern void configWifi();
extern void WifiDataToParse();
extern void WifiDataToSend();
extern void setupMotors();

CRGB RSL_leds[1] = { CRGB(0, 0, 0) };
#define RSL_LED RSL_leds[0]
#define RSL_COLOR CRGB(250, 45, 0) // orange
#define RSL_OFF CRGB(0, 0, 0)

void setupRSL()
{
#ifndef PIN_NEOPIXEL
#define ONBOARD_LED 2
    pinMode(ONBOARD_LED, OUTPUT);
#else
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, HIGH);
    FastLED.addLeds<NEOPIXEL, PIN_NEOPIXEL>(RSL_leds, 1);
    RSL_LED = CRGB(0, 0, 0);
    FastLED.show();
#endif
}
void enabledRSL()
{
#ifndef PIN_NEOPIXEL
    digitalWrite(ONBOARD_LED, millis() % 500 < 250); // flash, enabled
#else
    if (millis() % 500 < 250) {
        RSL_LED = RSL_COLOR;
        FastLED.show();
    } else {
        RSL_LED = RSL_OFF;
        FastLED.show();
    }
#endif
}
void wifiFailRSL()
{
#ifndef PIN_NEOPIXEL
    digitalWrite(ONBOARD_LED, millis() % 1000 <= 100); // short flash, wifi connection fail
#else
    if (millis() % 1000 <= 100) {
        RSL_LED = RSL_COLOR;
        FastLED.show();
    } else {
        RSL_LED = RSL_OFF;
        FastLED.show();
    }
#endif
}
void wifiDisconnectedRSL()
{
#ifndef PIN_NEOPIXEL
    digitalWrite(ONBOARD_LED, millis() % 1000 >= 100); // long flash, no driver station connected
#else
    if (millis() % 1000 >= 100) {
        RSL_LED = RSL_COLOR;
        FastLED.show();
    } else {
        RSL_LED = RSL_OFF;
        FastLED.show();
    }
#endif
}
void disabledRSL()
{
#ifndef PIN_NEOPIXEL
    digitalWrite(ONBOARD_LED, HIGH); // on, disabled
#else
    RSL_LED = RSL_COLOR;
    FastLED.show();
#endif
}

boolean connectedToWifi()
{
    return !WSC::timedOut() || hasWebsiteLoaded;
}
boolean connectionTimedOut()
{
    return WSC::timedOut();
}

void setup()
{
    Serial.begin(115200);
    setupRSL();

    PowerOn();

    Disable();

    connectToWifi();
    WSC::startWebSocketComms(WifiDataToParse, WifiDataToSend);
    startWebServer();
}

void loop()
{
    WSC::runWebSocketComms();
    if (!connectedToWifi() || connectionTimedOut()) {
        enabled = false;
    }
    Always();

    if (enabled && !wasEnabled) {
        Enable();
    }
    if (!enabled && wasEnabled) {
        Disable();
    }
    if (enabled) {
        Enabled();
        enabledRSL();
    } else {
        if (!connectedToWifi())
            wifiFailRSL();
        else if (connectionTimedOut())
            wifiDisconnectedRSL();
        else
            disabledRSL();
    }
    wasEnabled = enabled;
}

#endif // RCMUTIL_H
