#ifndef RCM_V3_H
#define RCM_V3_H

// this file allows for runtime configuration of robots using ArduinoJson and a factory for JMotor components

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <JMotor.h>
#include <Preferences.h>

JsonDocument config;

// website is asking for config
void RCMV3_load_config(AsyncWebServerRequest* request, Preferences& prefs)
{
    // TODO
}
// website is writing config
void RCMV3_save_config(AsyncWebServerRequest* request, Preferences& prefs)
{
    // TODO
}

enum RCMv3ComponentType {
    RC_TYPE_JMotorDriverTMC7300,
    RC_TYPE_JMotorDriverEsp32Servo
};

const char* RCMv3ComponentTypeNames[] = {
    "Motor Driver TMC7300",
    "Motor Driver Servo ESP32"
};

class RCMv3Component {
protected:
    void* internalInstance = nullptr;
    RCMv3ComponentType type;

public:
    virtual void begin() = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual void run() = 0;
    virtual int getNumInputs() = 0;
    virtual float readInput(int index) = 0;
    virtual int getNumOutputs() = 0;
    virtual void setOutput(int index, float value) = 0;

    void* getInternalInstance()
    {
        return internalInstance;
    }
    RCMv3ComponentType getType()
    {
        return type;
    }
    const char* getTypeName()
    {
        return RCMv3ComponentTypeNames[type];
    }
    virtual ~RCMv3Component() { }
};

class RCMv3ComponentJMotorDriver : public RCMv3Component {
public:
    void begin()
    {
    }
    void enable()
    {
        ((JMotorDriver*)internalInstance)->enable();
    }
    void disable()
    {
        ((JMotorDriver*)internalInstance)->disable();
    }
    void run()
    {
    }
    int getNumInputs()
    {
        return 0;
    }
    float readInput(int index)
    {
        return 0;
    }
    int getNumOutputs()
    {
        return 1;
    }
    virtual ~RCMv3ComponentJMotorDriver() { }
};

class RCMv3ComponentJMotorDriverTMC7300 : public RCMv3ComponentJMotorDriver {
public:
    RCMv3ComponentJMotorDriverTMC7300(TMC7300IC& ic, boolean channel)
    {
        type = RC_TYPE_JMotorDriverTMC7300;
        internalInstance = new JMotorDriverTMC7300(ic, channel);
    }
    void setOutput(int index, float value)
    {
        if (index == 0) {
            ((JMotorDriverTMC7300*)internalInstance)->set(value);
        }
    }
    ~RCMv3ComponentJMotorDriverTMC7300()
    {
        delete (JMotorDriverTMC7300*)internalInstance;
    }
};

class RCMv3ComponentJMotorDriverEsp32Servo : public RCMv3ComponentJMotorDriver {
public:
    RCMv3ComponentJMotorDriverEsp32Servo(int pwmChannel, int servoPin)
    {
        type = RC_TYPE_JMotorDriverEsp32Servo;
        internalInstance = new JMotorDriverEsp32Servo(pwmChannel, servoPin);
    }
    void setOutput(int index, float value)
    {
        if (index == 0) {
            ((JMotorDriverEsp32Servo*)internalInstance)->set(value);
        }
    }
    ~RCMv3ComponentJMotorDriverEsp32Servo()
    {
        delete (JMotorDriverEsp32Servo*)internalInstance;
    }
};

class RCMv3ComponentFactory {
public:
    static RCMv3Component* createComponent(RCMv3ComponentType type, JsonObject data)
    {
        switch (type) {
        case RC_TYPE_JMotorDriverTMC7300:
            return new RCMv3ComponentJMotorDriverTMC7300();
            break;
        case RC_TYPE_JMotorDriverEsp32Servo:
            return new RCMv3ComponentJMotorDriverEsp32Servo();
            break;
        default:
            return nullptr;
            break;
        }
    }
    static void deleteComponent(RCMv3Component* component)
    {
        delete component;
    }
};
#endif // RCM_V3_H