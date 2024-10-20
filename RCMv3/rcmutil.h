#ifndef RCMUTIL_H
#define RCMUTIL_H

// contains functions common to all RCMv2 projects

#include <Arduino.h>

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

#if RCM_HARDWARE_VERSION == RCM_ORIGINAL || RCM_HARDWARE_VERSION == RCM_4_V1

void setupRSL()
{
    pinMode(ONBOARD_LED, OUTPUT);
}
void enabledRSL()
{
    digitalWrite(ONBOARD_LED, millis() % 500 < 250); // flash, enabled
}
void wifiFailRSL()
{
    digitalWrite(ONBOARD_LED, millis() % 1000 <= 100); // short flash, wifi connection fail
}
void wifiDisconnectedRSL()
{
    digitalWrite(ONBOARD_LED, millis() % 1000 >= 100); // long flash, no driver station connected
}
void disabledRSL()
{
    digitalWrite(ONBOARD_LED, HIGH); // on, disabled
}

#elif RCM_HARDWARE_VERSION == RCM_D1_V1

void setupRSL()
{
    pinMode(ONBOARD_LED, OUTPUT);
    digitalWrite(ONBOARD_LED, HIGH);
}
void enabledRSL()
{
    digitalWrite(ONBOARD_LED, millis() % 500 > 250); // flash, enabled
}
void wifiFailRSL()
{
    digitalWrite(ONBOARD_LED, millis() % 1000 >= 100); // short flash, wifi connection fail
}
void wifiDisconnectedRSL()
{
    digitalWrite(ONBOARD_LED, millis() % 1000 >= 100); // long flash, no driver station connected
}
void disabledRSL()
{
    digitalWrite(ONBOARD_LED, LOW); // on, disabled
}

#elif RCM_HARDWARE_VERSION == RCM_BYTE_V2 || RCM_HARDWARE_VERSION == RCM_NIBBLE_V1

void setupRSL()
{
#if defined(NEOPIXEL_POWER)
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, HIGH);
#endif
    FastLED.addLeds<NEOPIXEL, PIN_NEOPIXEL>(RSL_leds, 1);
    RSL_LED = CRGB(0, 0, 0);
    FastLED.show();
}
void enabledRSL()
{
#ifndef OVERWRITE_RSL
    if (millis() % 500 < 250) {
        setRSL(RSLcolor);
    } else {
        setRSL(CRGB(0, 0, 0));
    }
#endif
}
void wifiFailRSL()
{
#ifndef OVERWRITE_RSL
    if (millis() % 1000 <= 100) {
        setRSL(RSLcolor);
    } else {
        setRSL(CRGB(0, 0, 0));
    }
#endif
}
void wifiDisconnectedRSL()
{
#ifndef OVERWRITE_RSL
    if (millis() % 1000 >= 100) {
        setRSL(RSLcolor);
    } else {
        setRSL(CRGB(0, 0, 0));
    }
#endif
}
void disabledRSL()
{
#ifndef OVERWRITE_RSL
    setRSL(RSLcolor);
#endif
}

#endif

void setup()
{
    Serial.begin(115200);
    setupRSL();
    setupMotors();
#if RCM_HARDWARE_VERSION == RCM_BYTE_V2 || RCM_HARDWARE_VERSION == RCM_NIBBLE_V1
#ifndef RCM_BYTE_DO_NOT_USE_SAFE_DISABLE
    digitalWrite(motorsEnablePin, LOW); // if using safe disable, disable all motors
#endif
#endif
    PowerOn();
    Disable();
#if RCM_COMM_METHOD == RCM_COMM_EWD
    configWifi();
    EWD::setupWifi(WifiDataToParse, WifiDataToSend);
#elif RCM_COMM_METHOD == RCM_COMM_ROS
    setupROS();
#elif RCM_COMM_METHOD == RCM_COMM_WEBSITE
    connectToWifi();
    WSC::startWebSocketComms(WifiDataToParse, WifiDataToSend);
#endif
    startWebServer(); // TODO:
}

boolean connectedToWifi()
{
#if RCM_COMM_METHOD == RCM_COMM_EWD
    return EWD::wifiConnected;
#elif RCM_COMM_METHOD == RCM_COMM_ROS
    return !ROSCheckFail;
#elif RCM_COMM_METHOD == RCM_COMM_WEBSITE
    return !WSC::timedOut() || hasWebsiteLoaded;
#endif
}
boolean connectionTimedOut()
{
#if RCM_COMM_METHOD == RCM_COMM_EWD
    return EWD::timedOut();
#elif RCM_COMM_METHOD == RCM_COMM_ROS
    return (millis() - lastEnableSentMillis) > rosWifiTimeout;
#elif RCM_COMM_METHOD == RCM_COMM_WEBSITE
    return WSC::timedOut();
#endif
}

extern void ROSrun();

void loop()
{
#if RCM_COMM_METHOD == RCM_COMM_EWD
    EWD::runWifiCommunication();
#elif RCM_COMM_METHOD == RCM_COMM_ROS
    ROSrun();
#elif RCM_COMM_METHOD == RCM_COMM_WEBSITE
    WSC::runWebSocketComms();
#endif
    if (!connectedToWifi() || connectionTimedOut()) {
        enabled = false;
    }
    Always();
    if (enabled && !wasEnabled) {
#if RCM_HARDWARE_VERSION == RCM_BYTE_V2 || RCM_HARDWARE_VERSION == RCM_NIBBLE_V1
#ifndef RCM_BYTE_DO_NOT_USE_SAFE_DISABLE
        digitalWrite(motorsEnablePin, HIGH);
#endif
#endif

        Enable();
    }
    if (!enabled && wasEnabled) {
        Disable();

#if RCM_HARDWARE_VERSION == RCM_BYTE_V2 || RCM_HARDWARE_VERSION == RCM_NIBBLE_V1
#ifndef RCM_BYTE_DO_NOT_USE_SAFE_DISABLE
        digitalWrite(motorsEnablePin, LOW);
#endif
#endif
    }
    if (enabled) {
        Enabled();
        enabledRSL(); //        digitalWrite(ONBOARD_LED, millis() % 500 < 250); // flash, enabled
    } else {
        if (!connectedToWifi())
            wifiFailRSL(); //            digitalWrite(ONBOARD_LED, millis() % 1000 <= 100); // short flash, wifi connection fail
        else if (connectionTimedOut())
            wifiDisconnectedRSL(); //            digitalWrite(ONBOARD_LED, millis() % 1000 >= 100); // long flash, no driver station connected
        else
            disabledRSL(); //            digitalWrite(ONBOARD_LED, HIGH); // on, disabled
    }
    wasEnabled = enabled;
}

#endif // RCMUTIL_H
