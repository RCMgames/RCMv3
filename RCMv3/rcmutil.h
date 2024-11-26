#ifndef RCMUTIL_H
#define RCMUTIL_H

// contains functions common to all RCMv2 projects

#include <Arduino.h>
#include <FastLED.h>

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
void rslOn()
{
#ifndef PIN_NEOPIXEL
    digitalWrite(ONBOARD_LED, HIGH);
#else
    RSL_LED = RSL_COLOR;
    FastLED.show();
#endif
}
void rslOff()
{
#ifndef PIN_NEOPIXEL
    digitalWrite(ONBOARD_LED, LOW);
#else
    RSL_LED = RSL_OFF;
    FastLED.show();
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
    if (WSC::timedOut()) {
        enabled = false;
    }

    Always();
    if (disableEnabled) {
        enabled = false;
    }

    if (enabled && !wasEnabled) {
        Enable();
    }
    if (!enabled && wasEnabled) {
        Disable();
    }
    if (enabled) {
        enabledRSL();
        Enabled();
    } else {
        if (WSC::timedOut()) {
            boolean rslOffVal = true;
            int blinkTime = millis() % 1500;
            for (byte i = 0; i < wifiMethod + 1; i++) {
                if (blinkTime > i * 300 && blinkTime <= i * 300 + 100) {
                    rslOffVal = false;
                }
            }
            rslOffVal ? rslOff() : rslOn();
        } else {
            disabledRSL();
        }
    }
    wasEnabled = enabled;
}

#endif // RCMUTIL_H
