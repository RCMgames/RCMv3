#ifndef RCMV3_H
#define RCMV3_H

// this file allows for runtime configuration of robots using ArduinoJson and a factory for JMotor components

#include "rcmv3board.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <JMotor.h>
#include <Preferences.h>
#include <mutex>
#include <vector>

std::mutex componentMutex;

#define DEBUG_PRINT_COMPONENT_FACTORY

enum RCMv3DataType {
    RC_DATA_Int,
    RC_DATA_Float,
    RC_DATA_Bool,
    RC_DATA_Pin,
    RC_DATA_TMC7300IC,
    RC_DATA_Servo_Port,
    RC_DATA_COUNT
};
const char* RCMv3DataTypeNames[] = {
    "int",
    "float",
    "bool",
    "int",
    "TMC7300IC",
    "int"
};

typedef struct {
    const char* name;
    RCMv3DataType type;
} RCMv3Parameter;

const char* RCMv3ComponentTypeNames[] = {
    "TMC7300 IC",
    "Motor Driver TMC7300",
    "Motor Driver Servo ESP32"
};

enum RCMv3ComponentType {
    RC_TYPE_TMC7300IC,
    RC_TYPE_JMotorDriverTMC7300,
    RC_TYPE_JMotorDriverEsp32Servo,
    RC_TYPE_COUNT
};

int RCMv3ComponentNumInputs[] = {
    0,
    1,
    1
};
const char* RCMv3ComponentInputNames[] = { // TODO: it needs to be a 2d array (type, input number), or a function in the class
    "",
    "power",
    "power"
};
int RCMv3ComponentNumOutputs[] = {
    0,
    0,
    0
};
const char* RCMv3ComponentOutputNames[] = { // TODO: FIX
    "",
    "",
    ""
};

class RCMv3ParameterHelper {
public:
    static std::vector<RCMv3Parameter> getParameterInfo(RCMv3ComponentType type)
    {
        switch (type) {
        case RC_TYPE_TMC7300IC:
            return {
                { "pin", RC_DATA_Pin },
                { "chipAddress", RC_DATA_Int }
            };
        case RC_TYPE_JMotorDriverTMC7300:
            return {
                { "TMC7300IC index", RC_DATA_TMC7300IC },
                { "channel", RC_DATA_Bool }
            };
        case RC_TYPE_JMotorDriverEsp32Servo:
            return {
                { "pwmChannel", RC_DATA_Int },
                { "servoPin", RC_DATA_Pin }
            };
        }
        return {};
    }
};

class RCMv3Component {
protected:
    void* internalInstance = nullptr;
    RCMv3ComponentType type;

public:
    int* inputs;
    int* outputs;

    JsonDocument jsonData;

    RCMv3Component(RCMv3ComponentType _type)
    {
        type = _type;
        Serial.println("RCMv3Component constructor");
        inputs = new int[RCMv3ComponentNumInputs[type]];
        outputs = new int[RCMv3ComponentNumOutputs[type]];
    }

    virtual void begin() = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual void run() = 0;

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

    virtual void write(int index, float value) = 0;
    virtual float read(int index) = 0;

    virtual ~RCMv3Component()
    {
        Serial.println("RCMv3Component destructor");
        jsonData.clear();
        delete inputs;
        delete outputs;
    }
};

class RCMv3ComponentTMC7300IC : public RCMv3Component {
public:
    RCMv3ComponentTMC7300IC(uint8_t pin, uint8_t chipAddress)
        : RCMv3Component(RC_TYPE_TMC7300IC)
    {
        internalInstance = new TMC7300IC(pin, chipAddress);
    }
    void begin()
    {
    }
    void enable()
    {
    }
    void disable()
    {
    }
    void run()
    {
    }
    void write(int index, float value)
    {
    }
    float read(int index)
    {
        return 0;
    }
    ~RCMv3ComponentTMC7300IC()
    {
        delete (TMC7300IC*)internalInstance;
    }
};

class RCMv3ComponentJMotorDriver : public RCMv3Component {
public:
    RCMv3ComponentJMotorDriver(RCMv3ComponentType type)
        : RCMv3Component(type)
    {
        Serial.println("RCMv3ComponentJMotorDriver constructor");
    }
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
    float read(int index)
    {
        return 0;
    }
    void write(int index, float value)
    {
        if (index == 0) {
            Serial.print("setting JMotorDriver value to: ");
            Serial.println(value);
            ((JMotorDriver*)internalInstance)->set(value);
        }
    }
    virtual ~RCMv3ComponentJMotorDriver() { }
};

class RCMv3ComponentJMotorDriverTMC7300 : public RCMv3ComponentJMotorDriver {
public:
    RCMv3ComponentJMotorDriverTMC7300(TMC7300IC& ic, boolean channel)
        : RCMv3ComponentJMotorDriver(RC_TYPE_JMotorDriverTMC7300)
    {
        internalInstance = new JMotorDriverTMC7300(ic, channel);
    }
    ~RCMv3ComponentJMotorDriverTMC7300()
    {
        delete (JMotorDriverTMC7300*)internalInstance;
    }
};

class RCMv3ComponentJMotorDriverEsp32Servo : public RCMv3ComponentJMotorDriver {
public:
    RCMv3ComponentJMotorDriverEsp32Servo(int pwmChannel, int servoPin)
        : RCMv3ComponentJMotorDriver(RC_TYPE_JMotorDriverEsp32Servo)
    {
        internalInstance = new JMotorDriverEsp32Servo(pwmChannel, servoPin);
    }
    ~RCMv3ComponentJMotorDriverEsp32Servo()
    {
        delete (JMotorDriverEsp32Servo*)internalInstance;
    }
};

std::vector<RCMv3Component*> components;

boolean refreshOutputListSize = true;

class RCMv3ComponentFactory {
public:
    /**
     * @note data, inputs, and outputs must be JsonArrays (check before calling this function)
     */
    static boolean createComponent(std::vector<RCMv3Component*>& components, RCMv3ComponentType type, JsonArray data, JsonArray inputs, JsonArray outputs)
    {
        std::vector<RCMv3Parameter> params = RCMv3ParameterHelper::getParameterInfo(type);
        // for each RCMv3Parameter this type should have, check that it is in data and valid
        if (params.size() != data.size()) {
            return false;
        }
        for (int i = 0; i < params.size(); i++) {
            if (!data[i].is<JsonVariant>()) {
                return false;
            }
            switch (params[i].type) {
            case RC_DATA_Int: {
                if (!data[i].is<int>()) {
                    return false;
                }
            } break;
            case RC_DATA_Float: {
                if (!data[i].is<float>()) {
                    return false;
                }
            } break;
            case RC_DATA_Bool: {
                if (!data[i].is<bool>()) {
                    return false;
                }
            } break;
            case RC_DATA_Pin: {
                if (!data[i].is<int>()) {
                    return false;
                }
            } break;
            case RC_DATA_TMC7300IC: {
                if (!data[i].is<int>()) {
                    return false;
                }
                int icIndex = data[i];
                if (components[icIndex]->getType() != RC_TYPE_TMC7300IC) {
                    return false;
                }
            } break;
            case RC_DATA_Servo_Port: {
                if (!data[i].is<int>()) {
                    return false;
                }
            } break;
            default:
                return false;
                break;
            }
        }
        Serial.println("parameters validated");
        // parameters validated
        // create component!
        switch (type) {
        case RC_TYPE_TMC7300IC: {
            Serial.printf("creating TMC7300IC with pin %d and chipAddress %d\n", (int)data[0], (int)data[1]);
            components.push_back(new RCMv3ComponentTMC7300IC((int)data[0], (int)data[1]));
        } break;
        case RC_TYPE_JMotorDriverTMC7300: {
            int icIndex = data[0];
            TMC7300IC* ic = (TMC7300IC*)components[icIndex]->getInternalInstance();
            Serial.printf("creating JMotorDriverTMC7300 with icIndex %d and channel %d\n", icIndex, (boolean)data[1]);
            components.push_back(new RCMv3ComponentJMotorDriverTMC7300(*ic, (boolean)data[1]));
        } break;
        case RC_TYPE_JMotorDriverEsp32Servo: {
            Serial.printf("creating JMotorDriverEsp32Servo with pwmChannel %d and servoPin %d\n", (int)data[0], (int)data[1]);
            components.push_back(new RCMv3ComponentJMotorDriverEsp32Servo((int)data[0], (int)data[1]));
        } break;
        }

        // validate inputs and outputs
        if (inputs.size() == RCMv3ComponentNumInputs[type]) {
            for (int i = 0; i < inputs.size(); i++) {
                if (!inputs[i].is<int>()) {
                    return false;
                }
            }
        } else {
            return false;
        }
        if (outputs.size() == RCMv3ComponentNumOutputs[type]) {
            for (int i = 0; i < outputs.size(); i++) {
                if (!outputs[i].is<int>()) {
                    return false;
                }
            }
        } else {
            return false;
        }
        Serial.println("inputs validated");

        // inputs
        for (int i = 0; i < inputs.size(); i++) {
            components[components.size() - 1]->inputs[i] = inputs[i].as<int>();
        }
        // outputs
        for (int i = 0; i < outputs.size(); i++) {
            components[components.size() - 1]->outputs[i] = outputs[i].as<int>();
        }

        return true;
    }
};

void RCMV3_begin()
{
    for (int i = 0; i < components.size(); i++) {
        components[i]->begin();
    }
}
void RCMV3_enable()
{
    for (int i = 0; i < components.size(); i++) {
        components[i]->enable();
    }
}
void RCMV3_disable()
{
    for (int i = 0; i < components.size(); i++) {
        components[i]->disable();
    }
}
void RCMV3_run(const std::vector<float>& inputVars, std::vector<float>& outputVars)
{

    if (refreshOutputListSize) {
        refreshOutputListSize = false;
        // resize outputVars if a component has a output index greater than the current size and less than 256
        int maxOutputIndex = 0;
        for (int i = 0; i < components.size(); i++) {
            for (int j = 0; j < RCMv3ComponentNumOutputs[components[i]->getType()]; j++) {
                if (components[i]->outputs[j] >= outputVars.size()) {
                    maxOutputIndex = max(maxOutputIndex, components[i]->outputs[j]);
                }
            }
        }
        Serial.println("resizing outputVars");
        outputVars.resize(min(maxOutputIndex, 255) + 1, 0);
    }

    for (int i = 0; i < components.size(); i++) {
        for (int j = 0; j < RCMv3ComponentNumInputs[components[i]->getType()]; j++) {
            if (components[i]->inputs[j] >= 0) {
                if (components[i]->inputs[j] < inputVars.size()) {
                    components[i]->write(j, inputVars[components[i]->inputs[j]]);
                } else {
                    Serial.print("invalid input index ");
                    Serial.println(components[i]->inputs[j]);
                    components[i]->write(j, 0);
                }
            }
        }
        components[i]->run();
        for (int j = 0; j < RCMv3ComponentNumOutputs[components[i]->getType()]; j++) {
            if (components[i]->outputs[j] >= 0) {
                outputVars[components[i]->outputs[j]] = components[i]->read(j);
            }
        }
    }
}

boolean RCMV3_parse_config(const String& str)
{
    Serial.println("RCMV3_parse_config");
    Serial.println(str);
    JsonDocument json;
    DeserializationError error = deserializeJson(json, str);
    if (error) {
        Serial.println(F("deserializeJson() failed: "));
        return false;
    }
    if (json.is<JsonArray>() == false) {
        Serial.println("json is not an array");
        return false;
    }

    std::vector<RCMv3Component*> tempComponents;
    boolean success = true;
    for (JsonObject jsonComponent : json.as<JsonArray>()) {
        if (jsonComponent["parameters"].is<JsonArray>() == false || jsonComponent["inputs"].is<JsonArray>() == false || jsonComponent["outputs"].is<JsonArray>() == false) {
            Serial.println("jsonComponent is not valid");
            success = false;
        } else {
            if (jsonComponent["type"].is<int>() == false) {
                Serial.println("jsonComponent type is not valid");
                success = false;
            } else {
                Serial.print("trying to make jsonComponent, type: ");
                Serial.println((int)jsonComponent["type"]);
                if (RCMv3ComponentFactory::createComponent(tempComponents, (RCMv3ComponentType)(int)jsonComponent["type"], jsonComponent["parameters"], jsonComponent["inputs"], jsonComponent["outputs"]) == true) {
                    Serial.println(tempComponents.size());
                    Serial.println("created component");
                    tempComponents[tempComponents.size() - 1]->jsonData.set(jsonComponent);
                } else {
                    Serial.println("error when creating a component");
                    success = false;
                }
            }
        }
    }

    if (success == false) {
        for (RCMv3Component* component : tempComponents) {
            delete component;
        }
        Serial.println("ERROR! components not set");
        Serial.println(components.size());
    } else {
        Serial.println("locking componentMutex");
        componentMutex.lock();
        for (RCMv3Component* component : components) {
            delete component;
        }
        components.clear();
        for (RCMv3Component* component : tempComponents) {
            components.push_back(component);
        }
        refreshOutputListSize = true;
        componentMutex.unlock();
        Serial.print(components.size());
        Serial.println(" components set");
    }
    return success;
}

boolean RCMV3_ComponentList_To_JSON_String(String& output)
{
    Serial.println("RCMV3_ComponentList_To_JSON_String");
    JsonDocument json;
    JsonArray jsonComponentList = json["components"].to<JsonArray>();
    for (RCMv3Component* component : components) {
        jsonComponentList.add(component->jsonData);
    }
    serializeJson(json, output);
    Serial.println(output);
    return true;
}

boolean RCMv3_Board_Info_To_JSON_String(String& output)
{
    Serial.println("RCMv3_PotentialComponentInfo_To_JSON_String");
    JsonDocument json;
    JsonArray jsonComponentList = json["potential_components"].to<JsonArray>();
    for (int i = 0; i < RC_TYPE_COUNT; i++) {
        JsonObject jsonComponent = jsonComponentList.add<JsonObject>();
        jsonComponent["type"] = i;
        jsonComponent["name"] = RCMv3ComponentTypeNames[i];
        jsonComponent["num_inputs"] = RCMv3ComponentNumInputs[i];
        jsonComponent["num_outputs"] = RCMv3ComponentNumOutputs[i];

        JsonArray jsonInputList = jsonComponent["inputs"].to<JsonArray>();
        for (int j = 0; j < RCMv3ComponentNumInputs[i]; j++) {
            JsonObject jsonInput = jsonInputList.add<JsonObject>();
            jsonInput["name"] = RCMv3ComponentInputNames[j];
        }

        JsonArray jsonOutputList = jsonComponent["outputs"].to<JsonArray>();

        JsonArray jsonParameterList = jsonComponent["parameters"].to<JsonArray>();
        for (RCMv3Parameter& param : RCMv3ParameterHelper::getParameterInfo((RCMv3ComponentType)i)) {
            JsonObject jsonParameter = jsonParameterList.add<JsonObject>();
            jsonParameter["name"] = param.name;
            jsonParameter["type"] = RCMv3DataTypeNames[param.type];
        }
    }
    JsonArray jsonPresetList = json["component_presets"].to<JsonArray>();
    serializeJson(json, output);
    return true;
}

// website is asking for config
void RCMV3_website_load_config(AsyncWebServerRequest* request)
{
    String jsonToSend;
    if (RCMV3_ComponentList_To_JSON_String(jsonToSend)) {
        request->send(200, "application/json", jsonToSend);
    } else {
        request->send(200, "application/json", "{}");
    }
}
// website is writing config
void RCMV3_website_save_config(AsyncWebServerRequest* request)
{
    // print request to Serial
    Serial.println("RCMV3_website_save_config");

    if (request->hasParam("components", true)) {
        AsyncWebParameter* p = request->getParam("components", true);
        Serial.println(p->value());
        if (RCMV3_parse_config(p->value())) {
            Serial.println(" parsed config ");
            request->send(200, "text/plain", "OK");
        } else {
            request->send(200, "text/plain", "Error");
        }
    } else {
        request->send(200, "text/plain", "Error2");
    }
}

void RCMV3_website_load_board_info(AsyncWebServerRequest* request)
{
    String jsonToSend;
    if (RCMv3_Board_Info_To_JSON_String(jsonToSend)) {
        request->send(200, "application/json", jsonToSend);
    } else {
        request->send(200, "application/json", "{}");
    }
}
#endif // RCMV3_H
