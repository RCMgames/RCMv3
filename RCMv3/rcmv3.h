#ifndef RCMV3_H
#define RCMV3_H

// this file allows for runtime configuration of robots using ArduinoJson and a factory for JMotor components
#include <Arduino.h>

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <JMotor.h>
#include <Preferences.h>
#include <mutex>
#include <vector>

#define nvsPartition "nvs2"

boolean enabled = false;
boolean wasEnabled = false;

boolean disableEnabled = false;

String parse_config_error_msg = "";
String create_component_error_msg = "";

std::mutex componentMutex;

std::vector<float> inputs;
std::vector<float> outputs;

// DO NOT REARRANGE arrays, it breaks old config files
enum RCMv3DataType {
    RC_DATA_Int,
    RC_DATA_Float,
    RC_DATA_Bool,
    RC_DATA_Pin,
    RC_DATA_TMC7300IC,
    RC_DATA_BSED,
    RC_DATA_TMCChipAddress,
    RC_DATA_WhichWire,
    RC_DATA_VoltageMonitorCalibrationVal,
    RC_DATA_ComponentIndex,
    RC_DATA_ComponentInputIndex, /*which input of a component should be accessed*/
    RC_DATA_ServoDriver,
    RC_DATA_COUNT
};
const char* RCMv3DataTypeNames[] = {
    "int",
    "float",
    "bool",
    "pin",
    "TMC7300IC",
    "BSED",
    "TMCChipAddress",
    "WhichWire",
    "VoltageMonitorCalibrationVal", // TODO: ADD METHOD FOR ADDING HELPERS THAT GIVE DEFAULT VALUES WITHOUT CREATING A NEW DATA TYPE?
    "ComponentIndex",
    "ComponentInputIndex",
    "Servo Driver"
};

typedef struct {
    const char* name;
    RCMv3DataType type;
} RCMv3Parameter;

const char* RCMv3ComponentTypeNames[] = {
    "JVoltageCompMeasure",
    "TMC7300 IC",
    "Motor Driver TMC7300",
    "Motor Driver Servo ESP32",
    "Motor Driver HBridge ESP32",
    "ByteSizedEncoderDecoder",
    "Encoder BSED",
    "Mixer",
    "Servo Controller"
};

enum RCMv3ComponentType {
    RC_TYPE_JVoltageCompMeasure,
    RC_TYPE_TMC7300IC,
    RC_TYPE_JMotorDriverTMC7300,
    RC_TYPE_JMotorDriverEsp32Servo,
    RC_Type_JMotorDriverEsp32HBridge,
    RC_TYPE_BSED,
    RC_TYPE_JEncoderBSED,
    RC_TYPE_MIXER,
    RC_TYPE_JServoController,
    RC_TYPE_COUNT
};

int RCMv3ComponentNumInputs[] = {
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    3,
    3
};

/**
 * @brief
 * @note
 * @param  type:
 * @param  input: should be less than RCMv3ComponentNumInputs[type]
 * @retval
 */
const char* RCMv3ComponentInputNames(RCMv3ComponentType type, uint8_t input)
{
    switch (type) {
    case RC_TYPE_MIXER:
        switch (input) {
        case 0:
            return "x";
        case 1:
            return "y";
        case 2:
            return "z";
        }
        break;
    case RC_TYPE_TMC7300IC:
        return "";
    case RC_TYPE_JMotorDriverTMC7300:
        return "power";
    case RC_TYPE_JMotorDriverEsp32Servo:
        return "position";
    case RC_Type_JMotorDriverEsp32HBridge:
        return "power";
    case RC_TYPE_JVoltageCompMeasure:
        return "";
    case RC_TYPE_BSED:
        return "";
    case RC_TYPE_JEncoderBSED:
        return "";
    case RC_TYPE_JServoController:
        switch (input) {
        case 0:
            return "angle immediate";
        case 1:
            return "angle smoothed";
        case 2:
            return "angle increment";
        }
        return "";
    }
    return "";
};

int RCMv3ComponentNumOutputs[] = {
    1,
    0,
    0,
    0,
    0,
    0,
    2,
    1,
    2
};

const char* RCMv3ComponentOutputNames(RCMv3ComponentType type, uint8_t output)
{
    switch (type) {
    case RC_TYPE_MIXER:
        return "output";
    case RC_TYPE_TMC7300IC:
        return "";
    case RC_TYPE_JMotorDriverTMC7300:
        return "";
    case RC_TYPE_JMotorDriverEsp32Servo:
        return "";
    case RC_Type_JMotorDriverEsp32HBridge:
        return "";
    case RC_TYPE_JVoltageCompMeasure:
        return "voltage";
    case RC_TYPE_BSED:
        return "";
    case RC_TYPE_JEncoderBSED:
        switch (output) {
        case 0:
            return "position";
        case 1:
            return "velocity";
        }
        return "";
    case RC_TYPE_JServoController:
        switch (output) {
        case 0:
            return "position";
        case 1:
            return "velocity";
        }
        return "";
    }
    return "";
};

class RCMv3ParameterHelper {
public:
    static std::vector<RCMv3Parameter> getParameterInfo(RCMv3ComponentType type)
    {
        switch (type) {
        case RC_TYPE_MIXER:
            return {
                { "A", RC_DATA_Float },
                { "B", RC_DATA_Float },
                { "C", RC_DATA_Float },
                { "component to send value to", RC_DATA_ComponentIndex },
                { "input index to send value to", RC_DATA_ComponentInputIndex }
            };
        case RC_TYPE_TMC7300IC:
            return {
                { "pin", RC_DATA_Pin },
                { "chipAddress", RC_DATA_TMCChipAddress }
            };
        case RC_TYPE_JMotorDriverTMC7300:
            return {
                { "TMC7300IC index", RC_DATA_TMC7300IC },
                { "motor1", RC_DATA_Bool },
                { "enablePin", RC_DATA_Pin }
            };
        case RC_TYPE_JMotorDriverEsp32Servo:
            return {
                { "pwmChannel", RC_DATA_Int },
                { "servoPin", RC_DATA_Pin }
            };
        case RC_Type_JMotorDriverEsp32HBridge:
            return {
                { "pwmChannel", RC_DATA_Int },
                { "in1", RC_DATA_Int },
                { "in2", RC_DATA_Int }
            };
        case RC_TYPE_JVoltageCompMeasure:
            return {
                { "measurePin", RC_DATA_Pin },
                { "adcUnitsPerVolt", RC_DATA_VoltageMonitorCalibrationVal },
                { "batteryDisableVoltage", RC_DATA_Float },
                { "hysteresis", RC_DATA_Float }
            };
        case RC_TYPE_BSED:
            return {
                { "wire", RC_DATA_WhichWire },
                { "address", RC_DATA_Int },
                { "encoderSlowestInterval", RC_DATA_Int },
                { "encoderEnoughCounts", RC_DATA_Int }
            };
        case RC_TYPE_JEncoderBSED:
            return {
                { "BSED index", RC_DATA_BSED },
                { "encoderChannel", RC_DATA_Int },
                { "reverse", RC_DATA_Bool },
                { "distPerCountFactor", RC_DATA_Float },
                { "slowestIntervalMicros", RC_DATA_Int },
                { "encoderEnoughCounts", RC_DATA_Int }
            };
        case RC_TYPE_JServoController:
            return {
                { "servo", RC_DATA_ServoDriver },
                { "reverse", RC_DATA_Bool },
                { "velLimit", RC_DATA_Float },
                { "accelLimit", RC_DATA_Float },
                { "decelLimit", RC_DATA_Float },
                { "minAngleLimit", RC_DATA_Float },
                { "maxAngleLimit", RC_DATA_Float },
                { "defaultAngle", RC_DATA_Float },
                { "minSetAngle", RC_DATA_Float },
                { "maxSetAngle", RC_DATA_Float },
                { "minServoVal", RC_DATA_Int },
                { "maxServoVal", RC_DATA_Int }
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
        // child classes should call disable() in their destructor if needed
        jsonData.clear();
        delete inputs;
        delete outputs;
    }
};

std::vector<RCMv3Component*> components;

// class RCMv3ComponentMixer : public RCMv3Component
// performs A*x + B*y + C*z and sets the value into the specified component and input index

class RCMv3ComponentMixer : public RCMv3Component {
protected:
    float A;
    float B;
    float C;
    int componentIndex;
    int inputIndex;

    float value;

    float X;
    float Y;
    float Z;

public:
    RCMv3ComponentMixer(float A_, float B_, float C_, int componentIndex_, int inputIndex_)
        : RCMv3Component(RC_TYPE_MIXER)
    {
        X = 0;
        Y = 0;
        Z = 0;
        value = 0;
        A = A_;
        B = B_;
        C = C_;
        componentIndex = componentIndex_;
        inputIndex = inputIndex_;
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
        value = A * X + B * Y + C * Z;

        if (componentIndex >= 0 && componentIndex < components.size()) {
            components[componentIndex]->write(inputIndex, value);
        }
    }
    void write(int index, float value)
    {
        switch (index) {
        case 0:
            X = value;
            break;
        case 1:
            Y = value;
            break;
        case 2:
            Z = value;
            break;
        }
    }
    float read(int index)
    {
        return value;
    }
    ~RCMv3ComponentMixer()
    {
    }
};

#define RCMV3_COMPONENT_J_VOLTAGE_COMP_MEASURE_N 50
class RCMv3ComponentJVoltageCompMeasure : public RCMv3Component {
protected:
    float batteryDisableVoltage;
    float hysteresis;
    boolean allowEnabled;

public:
    RCMv3ComponentJVoltageCompMeasure(uint8_t _measurePin, float _DACUnitsPerVolt, float _batteryDisableVoltage, float _hysteresis = 0.25)
        : RCMv3Component(RC_TYPE_JVoltageCompMeasure)
    {
        internalInstance = new JVoltageCompMeasure<RCMV3_COMPONENT_J_VOLTAGE_COMP_MEASURE_N>(_measurePin, _DACUnitsPerVolt);
        batteryDisableVoltage = _batteryDisableVoltage;
        hysteresis = _hysteresis;
        allowEnabled = true;
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
        float batteryVoltage = ((JVoltageCompMeasure<RCMV3_COMPONENT_J_VOLTAGE_COMP_MEASURE_N>*)internalInstance)->getSupplyVoltage();
        if (allowEnabled) {
            if (batteryVoltage < batteryDisableVoltage) {
                allowEnabled = false;
            }
        } else {
            if (batteryVoltage > batteryDisableVoltage + hysteresis) {
                allowEnabled = true;
            }
        }
        if (allowEnabled == false) {
            disableEnabled = true;
        }
    }
    void write(int index, float value)
    {
    }
    float read(int index)
    {
        return ((JVoltageCompMeasure<RCMV3_COMPONENT_J_VOLTAGE_COMP_MEASURE_N>*)internalInstance)->getSupplyVoltage();
    }
    ~RCMv3ComponentJVoltageCompMeasure()
    {
        delete (JVoltageCompMeasure<RCMV3_COMPONENT_J_VOLTAGE_COMP_MEASURE_N>*)internalInstance;
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
            ((JMotorDriver*)internalInstance)->set(value);
        }
    }
    virtual ~RCMv3ComponentJMotorDriver()
    {
        disable();
    }
};

class RCMv3ComponentJMotorDriverTMC7300 : public RCMv3ComponentJMotorDriver {
public:
    RCMv3ComponentJMotorDriverTMC7300(TMC7300IC& ic, boolean channel, byte enPin)
        : RCMv3ComponentJMotorDriver(RC_TYPE_JMotorDriverTMC7300)
    {
        internalInstance = new JMotorDriverTMC7300(ic, channel, true, enPin);
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

class RCMv3ComponentJMotorDriverEsp32HBridge : public RCMv3ComponentJMotorDriver {
public:
    RCMv3ComponentJMotorDriverEsp32HBridge(int pwmChannel, int in1, int in2)
        : RCMv3ComponentJMotorDriver(RC_Type_JMotorDriverEsp32HBridge)
    {
        internalInstance = new JMotorDriverEsp32HBridge(pwmChannel, in1, in2);
    }
    ~RCMv3ComponentJMotorDriverEsp32HBridge()
    {
        delete (JMotorDriverEsp32HBridge*)internalInstance;
    }
};

class RCMv3ComponentBSED : public RCMv3Component {
protected:
    TwoWire* wire;

public:
    RCMv3ComponentBSED(TwoWire* _wire, int address)
        : RCMv3Component(RC_TYPE_BSED)
    {
        wire = _wire;
        internalInstance = new ByteSizedEncoderDecoder(wire, address);
    }
    void begin()
    {
        wire->begin();
        ((ByteSizedEncoderDecoder*)internalInstance)->begin();
    }
    void enable()
    {
    }
    void disable()
    {
    }
    void run()
    {
        ((ByteSizedEncoderDecoder*)internalInstance)->run();
    }
    void write(int index, float value)
    {
    }
    float read(int index)
    {
        return 0;
    }
    ~RCMv3ComponentBSED()
    {
        delete (ByteSizedEncoderDecoder*)internalInstance;
    }
};

class RCMv3ComponentJEncoderBSED : public RCMv3Component {
public:
    RCMv3ComponentJEncoderBSED(ByteSizedEncoderDecoder* _bsed, byte _encoderChannel, bool _reverse, float _distPerCountFactor, int16_t _slowestIntervalMicros, int16_t _encoderEnoughCounts)
        : RCMv3Component(RC_TYPE_JEncoderBSED)
    {
        internalInstance = new JEncoderBSED(*_bsed, _encoderChannel, _reverse, _distPerCountFactor, _slowestIntervalMicros, _encoderEnoughCounts);
    }
    void begin()
    {
        ((JEncoderBSED*)internalInstance)->zeroCounter();
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
        switch (index) {
        case 0:
            return ((JEncoderBSED*)internalInstance)->getPos();
        case 1:
            return ((JEncoderBSED*)internalInstance)->getVel();
        }
        return 0;
    }
    ~RCMv3ComponentJEncoderBSED()
    {
        delete (JEncoderBSED*)internalInstance;
    }
};

class RCMv3ComponentJServoController : public RCMv3Component {
public:
    RCMv3ComponentJServoController(JMotorDriverServo& servo, boolean reverse, float velLimit, float accelLimit, float decelLimit, float minAngleLimit, float maxAngleLimit, float defaultAngle, float minSetAngle, float maxSetAngle, int minServoVal, int maxServoVal)
        : RCMv3Component(RC_TYPE_JServoController)
    {
        internalInstance = new JServoController(servo, reverse, velLimit, accelLimit, decelLimit, 0, minAngleLimit, maxAngleLimit, defaultAngle, minSetAngle, maxSetAngle, minServoVal, maxServoVal);
    }
    void begin()
    {
    }
    void enable()
    {
        ((JServoController*)internalInstance)->enable();
    }
    void disable()
    {
        ((JServoController*)internalInstance)->disable();
    }
    void run()
    {
        ((JServoController*)internalInstance)->run();
    }
    void write(int index, float value)
    {
        switch (index) {
        case 0: {
            ((JServoController*)internalInstance)->setAngleImmediate(value);
        } break;
        case 1: {
            ((JServoController*)internalInstance)->setAngleSmoothed(value);
        } break;
        case 2: {
            ((JServoController*)internalInstance)->setAngleImmediateInc(value);
        } break;
        }
    }
    float read(int index)
    {
        switch (index) {
        case 0:
            return ((JServoController*)internalInstance)->getPos();
        case 1:
            return ((JServoController*)internalInstance)->getVelocity();
        }
        return 0;
    }
};

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
            create_component_error_msg += " invalid number of parameters ";
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
            case RC_DATA_VoltageMonitorCalibrationVal:
            case RC_DATA_Float: {
                if (!data[i].is<float>()) {
                    create_component_error_msg += " invalid float value for parameter " + String(i);
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
                if (icIndex < 0 || icIndex >= components.size()) {
                    create_component_error_msg += " invalid TMC7300IC index (" + String(icIndex) + ") ";
                    return false;
                }
                if (components[icIndex]->getType() != RC_TYPE_TMC7300IC) {
                    create_component_error_msg += " component specified with the TMC7300IC index parameter (" + String(icIndex) + ") is not a TMC7300IC";
                    return false;
                }
            } break;
            case RC_DATA_TMCChipAddress: {
                if (!data[i].is<int>()) {
                    return false;
                }
            } break;
            case RC_DATA_ComponentIndex: {
                if (!data[i].is<int>()) {
                    return false;
                }
                int componentIndex = data[i];
                if ((componentIndex < 0 || componentIndex >= components.size()) && !(type == RC_TYPE_MIXER)) { // mixer only optionally needs a component to send the value to
                    create_component_error_msg += " invalid component index (" + String(componentIndex) + ") ";
                    return false;
                }
            } break;
            case RC_DATA_ComponentInputIndex: {
                if (!data[i].is<int>()) {
                    return false;
                }
            } break;
            case RC_DATA_WhichWire: {
                if (!data[i].is<int>()) {
                    return false;
                }
            } break;
            case RC_DATA_BSED: {
                if (!data[i].is<int>()) {
                    return false;
                }
                int icIndex = data[i];
                if (icIndex < 0 || icIndex >= components.size()) {
                    create_component_error_msg += " invalid BSED index (" + String(icIndex) + ") ";
                    return false;
                }
                if (components[icIndex]->getType() != RC_TYPE_BSED) {
                    create_component_error_msg += " component specified with the BSED index parameter (" + String(icIndex) + ") is not a ByteSizedEncoderDecoder";
                    return false;
                }
            } break;
            case RC_DATA_ServoDriver: {
                if (!data[i].is<int>()) {
                    create_component_error_msg += " invalid Servo Driver index (" + String(i) + ") ";
                    return false;
                }
                int icIndex = data[i];
                if (icIndex < 0 || icIndex >= components.size()) {
                    create_component_error_msg += " invalid Servo Driver index (" + String(icIndex) + ") ";
                    return false;
                }
                if (!(components[icIndex]->getType() == RC_TYPE_JMotorDriverEsp32Servo)) {
                    create_component_error_msg += " component specified with the Servo Driver index parameter (" + String(icIndex) + ") is not a Servo Driver";
                    return false;
                }
            } break;
            default:
                create_component_error_msg += " invalid data type for parameter " + String(i);
                return false;
                break;
            }
        }
        // parameters validated
        // create component!
        switch (type) {
        case RC_TYPE_MIXER: {
            Serial.printf("creating Mixer with A %f, B %f, C %f, componentIndex %d, and inputIndex %d\n", (float)data[0], (float)data[1], (float)data[2], (int)data[3], (int)data[4]);
            components.push_back(new RCMv3ComponentMixer((float)data[0], (float)data[1], (float)data[2], (int)data[3], (int)data[4]));
        } break;
        case RC_TYPE_JVoltageCompMeasure: {
            Serial.printf("creating JVoltageCompMeasure with pin %d and DACUnitsPerVolt %f and batteryDisableVoltage %f and hysteresis %f\n", (int)data[0], (float)data[1], (float)data[2], (float)data[3]);
            components.push_back(new RCMv3ComponentJVoltageCompMeasure((int)data[0], (float)data[1], (float)data[2], (float)data[3]));
        } break;
        case RC_TYPE_TMC7300IC: {
            Serial.printf("creating TMC7300IC with pin %d and chipAddress %d\n", (int)data[0], (int)data[1]);
            if ((int)data[1] < 0 || (int)data[1] > 3) { // invalid chip address
                create_component_error_msg += " chipAddress must be between 0 and 3";
                return false;
            }
            components.push_back(new RCMv3ComponentTMC7300IC((int)data[0], (int)data[1]));
        } break;
        case RC_TYPE_JMotorDriverTMC7300: {
            int icIndex = data[0];
            TMC7300IC* ic = (TMC7300IC*)components[icIndex]->getInternalInstance();
            Serial.printf("creating JMotorDriverTMC7300 with icIndex %d and channel %d and enPin %d\n", icIndex, (boolean)data[1], (int)data[2]);
            components.push_back(new RCMv3ComponentJMotorDriverTMC7300(*ic, (boolean)data[1], (int)data[2]));
        } break;
        case RC_TYPE_JMotorDriverEsp32Servo: {
            Serial.printf("creating JMotorDriverEsp32Servo with pwmChannel %d and servoPin %d\n", (int)data[0], (int)data[1]);
            components.push_back(new RCMv3ComponentJMotorDriverEsp32Servo((int)data[0], (int)data[1]));
        } break;
        case RC_Type_JMotorDriverEsp32HBridge: {
            Serial.printf("creating JMotorDriverEsp32HBridge with pwmChannel %d and in1 %d and in2 %d\n", (int)data[0], (int)data[1], (int)data[2]);
            components.push_back(new RCMv3ComponentJMotorDriverEsp32HBridge((int)data[0], (int)data[1], (int)data[2]));
        } break;
        case RC_TYPE_BSED: {
            Serial.printf("creating BSED with wire %d and address %d\n", (int)data[0], (int)data[1]);
            TwoWire* selectedWire = nullptr;
            switch ((int)data[0]) {
            case 0:
                selectedWire = &Wire;
                break;
            case 1:
                selectedWire = &Wire1;
                break;
            default:
                create_component_error_msg += " invalid wire selection (0=Wire, 1=Wire1)";
                return false;
            }
            components.push_back(new RCMv3ComponentBSED(selectedWire, (int)data[1]));
        } break;
        case RC_TYPE_JEncoderBSED: {
            if (data[1] < 1 || data[1] > 8) {
                create_component_error_msg += " encoderChannel must be between 1 and 8";
                return false;
            }
            Serial.printf("creating JEncoderBSED with bsed %d and encoderChannel %d and reverse %d and distPerCountFactor %f and slowestInterval %d and enoughCounts %d\n", (int)data[0], (int)data[1], (int)data[2], (float)data[3], (int)data[4], (int)data[5]);
            ByteSizedEncoderDecoder* bsed = (ByteSizedEncoderDecoder*)components[(int)data[0]]->getInternalInstance();
            components.push_back(new RCMv3ComponentJEncoderBSED(bsed, (int)data[1], (int)data[2], (float)data[3], (int)data[4], (int)data[5]));
        } break;
        case RC_TYPE_JServoController: {
            JMotorDriverServo* servo = (JMotorDriverServo*)components[(int)data[0]]->getInternalInstance();
            Serial.printf("creating JServoController with servo %d and reverse %d and velLimit %f and accelLimit %f and decelLimit %f and minAngleLimit %f and maxAngleLimit %f and defaultAngle %f and minSetAngle %f and maxSetAngle %f and minServoVal %d and maxServoVal %d\n", (int)data[0], (int)data[1], (float)data[2], (float)data[3], (float)data[4], (float)data[5], (float)data[6], (float)data[7], (float)data[8], (float)data[9], (int)data[10], (int)data[11]);
            components.push_back(new RCMv3ComponentJServoController(*servo, (int)data[1], (float)data[2], (float)data[3], (float)data[4], (float)data[5], (float)data[6], (float)data[7], (float)data[8], (float)data[9], (int)data[10], (int)data[11]));
        } break;
        } // end switch

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
        int maxOutputIndex = -1;
        for (int i = 0; i < components.size(); i++) {
            for (int j = 0; j < RCMv3ComponentNumOutputs[components[i]->getType()]; j++) {
                maxOutputIndex = max(maxOutputIndex, components[i]->outputs[j]);
            }
        }
        outputVars.resize(min(maxOutputIndex, 255) + 1, 0);
    }

    for (int i = 0; i < components.size(); i++) {
        for (int j = 0; j < RCMv3ComponentNumInputs[components[i]->getType()]; j++) {
            if (components[i]->inputs[j] >= 0) {
                if (components[i]->inputs[j] < inputVars.size()) {
                    components[i]->write(j, inputVars[components[i]->inputs[j]]);
                } else {
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
    JsonDocument json;
    DeserializationError error = deserializeJson(json, str);
    if (error) {
        return false;
    }
    if (json.is<JsonArray>() == false) {
        return false;
    }

    std::vector<RCMv3Component*> tempComponents;
    boolean success = true;
    int jsonComponentNum = -1;
    for (JsonObject jsonComponent : json.as<JsonArray>()) {
        jsonComponentNum++;
        if (jsonComponent["parameters"].is<JsonArray>() == false || jsonComponent["inputs"].is<JsonArray>() == false || jsonComponent["outputs"].is<JsonArray>() == false) {
            success = false;
        } else {
            if (jsonComponent["type"].is<int>() == false) {
                success = false;
            } else {
                create_component_error_msg = "";
                if (RCMv3ComponentFactory::createComponent(tempComponents, (RCMv3ComponentType)(int)jsonComponent["type"], jsonComponent["parameters"], jsonComponent["inputs"], jsonComponent["outputs"]) == true) {
                    tempComponents[tempComponents.size() - 1]->jsonData.set(jsonComponent);
                } else {
                    parse_config_error_msg += "Error in component " + String(jsonComponentNum) + ": " + create_component_error_msg + "<br>";
                    success = false;
                }
            }
        }
    }

    if (success == false) {
        for (RCMv3Component* component : tempComponents) {
            delete component;
        }
    } else {
        componentMutex.lock();
        for (RCMv3Component* component : components) {
            delete component;
        }
        components.clear();
        for (RCMv3Component* component : tempComponents) {
            components.push_back(component);
        }
        RCMV3_begin();

        if (enabled) {
            RCMV3_enable();
        }

        refreshOutputListSize = true;
        componentMutex.unlock();
    }
    return success;
}

boolean RCMV3_ComponentList_To_JSON_String(String& output)
{
    JsonDocument json;
    JsonArray jsonComponentList = json["components"].to<JsonArray>();
    for (RCMv3Component* component : components) {
        jsonComponentList.add(component->jsonData);
    }
    serializeJson(json, output);
    return true;
}

boolean RCMv3_Board_Info_To_JSON_String(String& output)
{
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
            jsonInput["name"] = RCMv3ComponentInputNames((RCMv3ComponentType)i, j);
        }

        JsonArray jsonOutputList = jsonComponent["outputs"].to<JsonArray>();
        for (int j = 0; j < RCMv3ComponentNumOutputs[i]; j++) {
            JsonObject jsonOutput = jsonOutputList.add<JsonObject>();
            jsonOutput["name"] = RCMv3ComponentOutputNames((RCMv3ComponentType)i, j);
        }

        JsonArray jsonParameterList = jsonComponent["parameters"].to<JsonArray>();
        for (RCMv3Parameter& param : RCMv3ParameterHelper::getParameterInfo((RCMv3ComponentType)i)) {
            JsonObject jsonParameter = jsonParameterList.add<JsonObject>();
            jsonParameter["name"] = param.name;
            jsonParameter["type"] = RCMv3DataTypeNames[param.type];
        }
    }
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
    if (request->hasParam("components", true)) {
        AsyncWebParameter* p = request->getParam("components", true);
        parse_config_error_msg = "";
        if (RCMV3_parse_config(p->value())) {
            request->send(200, "text/plain", "OK");
        } else {
            request->send(200, "text/plain", parse_config_error_msg);
        }
    } else {
        request->send(200, "text/plain", "Error");
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

void save_config_to_memory()
{
    Preferences preferences;
    preferences.begin("config", false, nvsPartition);
    String configData;
    if (RCMV3_ComponentList_To_JSON_String(configData)) {
        preferences.putString("config", configData);
    }
    preferences.end();
}
void load_config_from_memory()
{
    Preferences preferences;
    preferences.begin("config", true, nvsPartition);
    String configData = preferences.getString("config", "[]");
    configData = configData.substring(strlen("{\"components\":"), configData.length() - 1);
    RCMV3_parse_config(configData);
    preferences.end();
}

#endif // RCMV3_H
