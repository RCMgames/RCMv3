#ifndef RCMV3_H
#define RCMV3_H

// this file allows for runtime configuration of robots using ArduinoJson and a factory for JMotor components
#include <Arduino.h>

#define ARDUINOJSON_ENABLE_NAN 1
#define ARDUINOJSON_ENABLE_INFINITY 1
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
    RC_DATA_ComponentIndex,
    RC_DATA_ComponentInputIndex, /*which input of a component should be accessed*/
    RC_DATA_ServoDriver,
    RC_DATA_VoltageComp,
    RC_DATA_JMotorDriver,
    RC_DATA_JMotorCompensator,
    RC_DATA_JEncoder,
    RC_DATA_JControlLoop,
    RC_DATA_JMotorController,
    RC_DATA_PCA9685,
    RC_DATA_JDrivetrain,
    RC_DATA_COUNT
};
// DO NOT RENAME without updating docs/ds/script.js and all presets
const char* RCMv3DataTypeNames[] = {
    "int",
    "float",
    "bool",
    "pin",
    "TMC7300IC",
    "BSED",
    "TMCChipAddress",
    "WhichWire",
    "ComponentIndex",
    "ComponentInputIndex",
    "Servo Driver",
    "VoltageComp",
    "JMotorDriver",
    "JMotorCompensator",
    "JEncoder",
    "JControlLoop",
    "JMotorController",
    "PCA9685",
    "JDrivetrain"
};
typedef struct {
    const char* name;
    RCMv3DataType type;
} RCMv3Parameter;

// DO NOT RENAME without updating docs/ds/script.js and all presets
const char* RCMv3ComponentTypeNames[] = {
    "VoltageCompMeasure",
    "TMC7300IC",
    "MotorDriverTMC7300",
    "MotorDriverEsp32Servo",
    "MotorDriverEsp32HBridge",
    "ByteSizedEncoderDecoder",
    "EncoderBSED",
    "Mixer",
    "ServoController",
    "MotorCompBasic",
    "ControlLoopBasic",
    "MotorControllerOpen",
    "MotorControllerClosed",
    "VoltageCompConst",
    "PCA9685",
    "MotorDriverPCA9685HBridge",
    "EncoderQuadrature",
    "DrivetrainTwoSide"
};
// DO NOT REARRANGE arrays, it breaks old config files
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
    RC_TYPE_JMotorCompBasic,
    RC_TYPE_JControlLoopBasic,
    RC_TYPE_JMotorControllerOpen,
    RC_TYPE_JMotorControllerClosed,
    RC_TYPE_JVoltageCompConst,
    RC_TYPE_PCA9685,
    RC_TYPE_JMotorDriverPCA9685HBridge,
    RC_TYPE_JEncoderQuadrature,
    RC_TYPE_JDrivetrainTwoSide,
    RC_TYPE_JDrivetrainMecanum,
    RC_TYPE_COUNT
};

int RCMv3ComponentNumInputs[] = {
    0, // JVoltageCompMeasure
    0, // TMC7300 IC
    1, // Motor Driver TMC7300
    1, // Motor Driver Servo ESP32
    1, // Motor Driver HBridge ESP32
    0, // ByteSizedEncoderDecoder
    0, // Encoder BSED
    3, // Mixer
    2, // Servo Controller
    0, // JMotorCompBasic
    0, // JControlLoopBasic
    2, // JMotorControllerOpen
    2, // JMotorControllerClosed
    0, // JVoltageCompConst
    0, // PCA9685
    1, // Motor Driver PCA9685 HBridge
    0, // Encoder Quadrature
    3, // Drivetrain Two Side
    3, // Drivetrain Mecanum
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
            return "servo controller mode";
        case 1:
            return "value";
        }
        return "";
    case RC_TYPE_JMotorCompBasic:
        return "";
    case RC_TYPE_JControlLoopBasic:
        return "";
    case RC_TYPE_JMotorControllerOpen:
        switch (input) {
        case 0:
            return "motor controller mode";
        case 1:
            return "value";
        }
    case RC_TYPE_JMotorControllerClosed:
        switch (input) {
        case 0:
            return "motor controller mode";
        case 1:
            return "value";
        }
    case RC_TYPE_JVoltageCompConst:
        return "";
    case RC_TYPE_PCA9685:
        return "";
    case RC_TYPE_JMotorDriverPCA9685HBridge:
        return "power";
    case RC_TYPE_JEncoderQuadrature:
        return "";
    case RC_TYPE_JDrivetrainTwoSide:
        switch (input) {
        case 0:
            return "velocity turning";
        case 1:
            return "velocity forwards";
        case 2:
            return "velocity left";
        }
    case RC_TYPE_JDrivetrainMecanum:
        switch (input) {
        case 0:
            return "velocity turning";
        case 1:
            return "velocity forwards";
        case 2:
            return "velocity left";
        }
    } // end of switch
    return "";
};

int RCMv3ComponentNumOutputs[] = {
    1, // JVoltageCompMeasure
    0, // TMC7300 IC
    0, // Motor Driver TMC7300
    0, // Motor Driver Servo ESP32
    0, // Motor Driver HBridge ESP32
    0, // ByteSizedEncoderDecoder
    2, // Encoder BSED
    1, // Mixer
    2, // Servo Controller
    0, // JMotorCompBasic
    1, // JControlLoopBasic
    2, // JMotorControllerOpen
    2, // JMotorControllerClosed
    1, // JVoltageCompConst
    0, // PCA9685
    0, // Motor Driver PCA9685 HBridge
    2, // Encoder Quadrature
    3, // Drivetrain Two Side
    3, // Drivetrain Mecanum
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
    case RC_TYPE_JMotorCompBasic:
        return "";
    case RC_TYPE_JControlLoopBasic:
        return "control loop output";
    case RC_TYPE_JMotorControllerOpen:
        switch (output) {
        case 0:
            return "velocity";
        case 1:
            return "position";
        }
    case RC_TYPE_JMotorControllerClosed:
        switch (output) {
        case 0:
            return "velocity";
        case 1:
            return "position";
        }
    case RC_TYPE_JVoltageCompConst:
        return "voltage";
    case RC_TYPE_PCA9685:
        return "";
    case RC_TYPE_JMotorDriverPCA9685HBridge:
        return "";
    case RC_TYPE_JEncoderQuadrature:
        switch (output) {
        case 0:
            return "position";
        case 1:
            return "velocity";
        }
    case RC_TYPE_JDrivetrainTwoSide:
        switch (output) {
        case 0:
            return "velocity turning";
        case 1:
            return "velocity forwards";
        case 2:
            return "velocity left";
        }
    case RC_TYPE_JDrivetrainMecanum:
        switch (output) {
        case 0:
            return "velocity turning";
        case 1:
            return "velocity forwards";
        case 2:
            return "velocity left";
        }
    } // end of switch
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
                { "adcUnitsPerVolt", RC_DATA_Float },
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
                { "ByteSizedEncoderDecoder", RC_DATA_BSED },
                { "encoderChannel", RC_DATA_Int },
                { "reverse", RC_DATA_Bool },
                { "distPerCountFactor", RC_DATA_Float },
                { "slowestIntervalMicros", RC_DATA_Int },
                { "encoderEnoughCounts", RC_DATA_Int }
            };
        case RC_TYPE_JServoController:
            return {
                { "defaultInputMode", RC_DATA_Int },
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
        case RC_TYPE_JMotorCompBasic:
            return {
                { "voltageComp", RC_DATA_VoltageComp },
                { "volts per speed", RC_DATA_Float },
                { "min speed", RC_DATA_Float }
            };
        case RC_TYPE_JControlLoopBasic:
            return {
                { "kP", RC_DATA_Float },
                { "timeout", RC_DATA_Int },
                { "noReverseVoltage", RC_DATA_Bool }
            };
        case RC_TYPE_JMotorControllerOpen:
            return {
                { "defaultInputMode", RC_DATA_Int },
                { "driver", RC_DATA_JMotorDriver },
                { "compensator", RC_DATA_JMotorCompensator },
                { "velLimit", RC_DATA_Float },
                { "accelLimit", RC_DATA_Float },
                { "minMotorPulseTime", RC_DATA_Int }
            };
        case RC_TYPE_JMotorControllerClosed:
            return {
                { "defaultInputMode", RC_DATA_Int },
                { "driver", RC_DATA_JMotorDriver },
                { "compensator", RC_DATA_JMotorCompensator },
                { "encoder", RC_DATA_JEncoder },
                { "controlLoop", RC_DATA_JControlLoop },
                { "velLimit", RC_DATA_Float },
                { "accelLimit", RC_DATA_Float },
                { "distFromSetpointLimit", RC_DATA_Float },
                { "preventGoingWrongWay", RC_DATA_Bool },
                { "maxStoppingDecel", RC_DATA_Float }
            };
        case RC_TYPE_JVoltageCompConst:
            return {
                { "voltage", RC_DATA_Float }
            };
        case RC_TYPE_PCA9685:
            return {
                { "wire", RC_DATA_WhichWire },
                { "address", RC_DATA_Int },
                { "pinSDA", RC_DATA_Pin },
                { "pinSCL", RC_DATA_Pin },
                { "outputEnablePin", RC_DATA_Pin },
                { "pwmFrequency", RC_DATA_Int }
            };
        case RC_TYPE_JMotorDriverPCA9685HBridge:
            return {
                { "PCA9685", RC_DATA_PCA9685 },
                { "channelPos", RC_DATA_Int },
                { "channelNeg", RC_DATA_Int },
                { "reverse", RC_DATA_Bool },
                { "brakeWhenEnabled", RC_DATA_Bool },
                { "brakeWhenDisabled", RC_DATA_Bool }
            };
        case RC_TYPE_JEncoderQuadrature:
            return {
                { "pinA", RC_DATA_Pin },
                { "pinB", RC_DATA_Pin },
                { "distPerCountFactor", RC_DATA_Float },
                { "reverse", RC_DATA_Bool },
                { "slowestIntervalMicros", RC_DATA_Int }
            };
        case RC_TYPE_JDrivetrainTwoSide:
            return {
                { "leftMotor", RC_DATA_JMotorController },
                { "rightMotor", RC_DATA_JMotorController },
                { "width", RC_DATA_Float }
            };
        case RC_TYPE_JDrivetrainMecanum:
            return {
                { "FRmotor", RC_DATA_JMotorController },
                { "FLmotor", RC_DATA_JMotorController },
                { "BLmotor", RC_DATA_JMotorController },
                { "BRmotor", RC_DATA_JMotorController },
                { "forwardsScalar", RC_DATA_Float },
                { "rightScalar", RC_DATA_Float },
                { "CCWScalar", RC_DATA_Float }
            };
        } // end of switch
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
    virtual void begin() { }
    virtual void enable() { }
    virtual void disable() { }
    virtual void run() { }
    virtual void write(int index, float value) { }
    virtual float read(int index) { return 0; }
    virtual void callback(byte id) { }

    virtual ~RCMv3Component()
    {
        disable();
        jsonData.clear();
        delete[] inputs;
        delete[] outputs;
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

#define RCMV3_COMPONENT_J_VOLTAGE_COMP_MEASURE_N 25
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
    void run()
    {
        float batteryVoltage = ((JVoltageCompMeasure<RCMV3_COMPONENT_J_VOLTAGE_COMP_MEASURE_N>*)internalInstance)->getSupplyVoltage();
        if (batteryVoltage < batteryDisableVoltage) {
            allowEnabled = false;
        }
        if (batteryVoltage > batteryDisableVoltage + hysteresis) {
            allowEnabled = true;
        }
        if (allowEnabled == false) {
            disableEnabled = true;
        }
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
    void enable()
    {
        ((JMotorDriver*)internalInstance)->enable();
    }
    void disable()
    {
        ((JMotorDriver*)internalInstance)->disable();
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

class RCMv3ComponentPCA9685 : public RCMv3Component {
public:
    RCMv3ComponentPCA9685(TwoWire* wire, int address, byte pinSDA, byte pinSCL, int outputEnablePin, int pwmFrequency)
        : RCMv3Component(RC_TYPE_PCA9685)
    {
        internalInstance = new PCA9685();
        ((PCA9685*)internalInstance)->setupOutputEnablePin(outputEnablePin);
        wire->setPins(pinSDA, pinSCL);
        ((PCA9685*)internalInstance)->setWire(*wire, false);
        wire->setClock(400000);
        ((PCA9685*)internalInstance)->addDevice(address);
        ((PCA9685*)internalInstance)->resetAllDevices();
        ((PCA9685*)internalInstance)->enableOutputs(outputEnablePin);
        ((PCA9685*)internalInstance)->setToFrequency(pwmFrequency);

        for (int i = 0; i < 16; i++) {
            ((PCA9685*)internalInstance)->setChannelDutyCycle(i, 0);
        }
    }
};

class RCMv3ComponentJMotorDriverPCA9685HBridge : public RCMv3ComponentJMotorDriver {
public:
    RCMv3ComponentJMotorDriverPCA9685HBridge(PCA9685& pca9685, int channelPos, int channelNeg, bool reverse, bool breakWhenEnabled, bool breakWhenDisabled)
        : RCMv3ComponentJMotorDriver(RC_TYPE_JMotorDriverPCA9685HBridge)
    {
        internalInstance = new JMotorDriverPCA9685HBridge(pca9685, channelPos, channelNeg, reverse, breakWhenEnabled, breakWhenDisabled);
    }
    ~RCMv3ComponentJMotorDriverPCA9685HBridge()
    {
        delete (JMotorDriverPCA9685HBridge*)internalInstance;
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
    void run()
    {
        ((ByteSizedEncoderDecoder*)internalInstance)->run();
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

void isrManager(byte i);
// clang-format off
void ISR0(){isrManager(0);}
void ISR1(){isrManager(1);}
void ISR2(){isrManager(2);}
void ISR3(){isrManager(3);}
void ISR4(){isrManager(4);}
void ISR5(){isrManager(5);}
void ISR6(){isrManager(6);}
void ISR7(){isrManager(7);}
void ISR8(){isrManager(8);}
void ISR9(){isrManager(9);}
void ISR10(){isrManager(10);}
void ISR11(){isrManager(11);}
void ISR12(){isrManager(12);}
void ISR13(){isrManager(13);}
void ISR14(){isrManager(14);}
void ISR15(){isrManager(15);}
void ISR16(){isrManager(16);}
void ISR17(){isrManager(17);}
void ISR18(){isrManager(18);}
void ISR19(){isrManager(19);}
void ISR20(){isrManager(20);}
void ISR21(){isrManager(21);}
void ISR22(){isrManager(22);}
void ISR23(){isrManager(23);}
void ISR24(){isrManager(24);}
void ISR25(){isrManager(25);}
void ISR26(){isrManager(26);}
void ISR27(){isrManager(27);}
void ISR28(){isrManager(28);}
void ISR29(){isrManager(29);}
void ISR30(){isrManager(30);}
void ISR31(){isrManager(31);}
// clang-format on

#define NUM_ISRS 32 // only half can really be used, since components are created before they are deleted
void (*isrArrayVoids[NUM_ISRS])(void) = { &ISR0, &ISR1, &ISR2, &ISR3, &ISR4, &ISR5, &ISR6, &ISR7, &ISR8, &ISR9, &ISR10, &ISR11, &ISR12, &ISR13, &ISR14, &ISR15, &ISR16, &ISR17, &ISR18, &ISR19, &ISR20, &ISR21, &ISR22, &ISR23, &ISR24, &ISR25, &ISR26, &ISR27, &ISR28, &ISR29, &ISR30, &ISR31 };
boolean isrArrayElementAssigned[NUM_ISRS] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
RCMv3Component* isrArrayComponents[NUM_ISRS] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
byte isrArrayCallbackIndex[NUM_ISRS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void isrManager(byte i)
{
    if (isrArrayElementAssigned[i] == true && isrArrayComponents[i] != nullptr) {
        isrArrayComponents[i]->callback(isrArrayCallbackIndex[i]);
    }
}

boolean isrManagerClaimPointer(void (**func)(void), int* pointerIndex, RCMv3Component* component, byte callbackIndex)
{
    for (int i = 0; i < NUM_ISRS; i++) {
        if (isrArrayElementAssigned[i] == false) {
            isrArrayElementAssigned[i] = true;
            *func = isrArrayVoids[i];
            *pointerIndex = i;
            isrArrayComponents[i] = component;
            isrArrayCallbackIndex[i] = callbackIndex;
            return true;
        }
    }
    return false;
}

boolean isrManagerSetComponent(int num, RCMv3Component* component)
{
    if (isrArrayElementAssigned[num] == true && isrArrayComponents[num] == nullptr) {
        isrArrayComponents[num] = component;
        return true;
    }
    return false;
}

void isrManagerFreePointer(int num)
{
    isrArrayElementAssigned[num] = false;
    isrArrayComponents[num] = nullptr;
}

class RCMv3ComponentJEncoderQuadrature : public RCMv3Component {
protected:
    void (*isra)(void);
    void (*isrb)(void);
    int isrAI;
    int isrBI;
    boolean successfullyCreatedISRsVal;

public:
    /**
     * @note if successfullyCreatedISRs() returns false, the component must be deleted
     */
    RCMv3ComponentJEncoderQuadrature(byte pinA, byte pinB, float distPerCountFactor, bool reverse, int slowestIntervalMicros)
        : RCMv3Component(RC_TYPE_JEncoderQuadrature)
    {
        successfullyCreatedISRsVal = false;
        if (isrManagerClaimPointer(&isra, &isrAI, this, 0)) {
            if (isrManagerClaimPointer(&isrb, &isrBI, this, 1)) {
                successfullyCreatedISRsVal = true;
            } else {
                isrManagerFreePointer(isrAI);
            }
        }
        internalInstance = new JEncoderQuadratureAttachInterrupt(pinA, pinB, distPerCountFactor, reverse, slowestIntervalMicros);
    }
    boolean successfullyCreatedISRs()
    {
        return successfullyCreatedISRsVal;
    }
    void callback(byte id)
    {
        switch (id) {
        case 0:
            ((JEncoderQuadratureAttachInterrupt*)internalInstance)->ISRA();
            break;
        case 1:
            ((JEncoderQuadratureAttachInterrupt*)internalInstance)->ISRB();
            break;
        }
    }
    void begin()
    {
        ((JEncoderQuadratureAttachInterrupt*)internalInstance)->setUpInterrupts(isra, isrb);
    }
    void run()
    {
        ((JEncoderQuadratureAttachInterrupt*)internalInstance)->run();
    }
    float read(int index)
    {
        switch (index) {
        case 0:
            return ((JEncoderQuadratureAttachInterrupt*)internalInstance)->getPos();
        case 1:
            return ((JEncoderQuadratureAttachInterrupt*)internalInstance)->getVel();
        }
        return 0;
    }
    ~RCMv3ComponentJEncoderQuadrature()
    {
        if (successfullyCreatedISRsVal) {
            isrManagerFreePointer(isrAI);
            isrManagerFreePointer(isrBI);
        }
        delete (JEncoderQuadratureAttachInterrupt*)internalInstance;
    }
};

class RCMv3ComponentJServoController : public RCMv3Component {
protected:
    int inputMode;

public:
    RCMv3ComponentJServoController(int defaultInputMode, JMotorDriverServo& servo, boolean reverse, float velLimit, float accelLimit, float decelLimit, float minAngleLimit, float maxAngleLimit, float defaultAngle, float minSetAngle, float maxSetAngle, int minServoVal, int maxServoVal)
        : RCMv3Component(RC_TYPE_JServoController)
    {
        internalInstance = new JServoController(servo, reverse, velLimit, accelLimit, decelLimit, 0, minAngleLimit, maxAngleLimit, defaultAngle, minSetAngle, maxSetAngle, minServoVal, maxServoVal);
        inputMode = defaultInputMode;
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
        if (index == 0) {
            inputMode = value;
        } else if (index == 1) {
            switch (inputMode) {
            case 0: {
                ((JServoController*)internalInstance)->setAngleSmoothed(value);
            } break;
            case 1: {
                ((JServoController*)internalInstance)->setAngleImmediate(value);
            } break;
            case 2: {
                ((JServoController*)internalInstance)->setAngleImmediateInc(value);
            } break;
            }
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
class RCMv3ComponentJMotorCompBasic : public RCMv3Component {
public:
    RCMv3ComponentJMotorCompBasic(JVoltageCompensator& voltageComp, float voltsPerSpeed, float minSpeed)
        : RCMv3Component(RC_TYPE_JMotorCompBasic)
    {
        internalInstance = new JMotorCompBasic(voltageComp, voltsPerSpeed, minSpeed);
    }
    ~RCMv3ComponentJMotorCompBasic()
    {
        delete (JMotorCompBasic*)internalInstance;
    }
};

class RCMv3ComponentJControlLoopBasic : public RCMv3Component {
public:
    RCMv3ComponentJControlLoopBasic(float kP, unsigned long timeout, bool noReverseVoltage)
        : RCMv3Component(RC_TYPE_JControlLoopBasic)
    {
        internalInstance = new JControlLoopBasic(kP, timeout, noReverseVoltage);
    }
    float read(int index)
    {
        return ((JControlLoopBasic*)internalInstance)->getCtrlLoopOut();
    }
    ~RCMv3ComponentJControlLoopBasic()
    {
        delete (JControlLoopBasic*)internalInstance;
    }
};

class RCMv3ComponentJMotorControllerOpen : public RCMv3Component {
protected:
    int inputMode;

public:
    RCMv3ComponentJMotorControllerOpen(int defaultInputMode, JMotorDriver& driver, JMotorCompensator& compensator, float velLimit, float accelLimit, int minMotorPulseTime)
        : RCMv3Component(RC_TYPE_JMotorControllerOpen)
    {
        internalInstance = new JMotorControllerOpen(driver, compensator, velLimit, accelLimit, minMotorPulseTime);
        inputMode = defaultInputMode;
    }
    void enable()
    {
        ((JMotorControllerOpen*)internalInstance)->enable();
        ((JMotorControllerOpen*)internalInstance)->dL.setVelocity(0);
    }
    void disable()
    {
        ((JMotorControllerOpen*)internalInstance)->disable();
    }
    void run()
    {
        ((JMotorControllerOpen*)internalInstance)->run();
    }
    void write(int index, float value)
    {
        if (index == 0) {
            inputMode = value;
        } else if (index == 1) {
            switch (inputMode) {
            case 0:
                ((JMotorControllerOpen*)internalInstance)->setVelTarget(value, false);
                break;
            case 1:
                ((JMotorControllerOpen*)internalInstance)->setPosTarget(value, false);
                break;
            }
        }
    }
    float read(int index)
    {
        switch (index) {
        case 0:
            return ((JMotorControllerOpen*)internalInstance)->getVel();
        case 1:
            return ((JMotorControllerOpen*)internalInstance)->getPos();
        }
        return 0;
    }
    ~RCMv3ComponentJMotorControllerOpen()
    {
        delete (JMotorControllerOpen*)internalInstance;
    }
};
// TODO: think about when/how position is reset. avoid the float getting big, but also allow for moving specific distances (most important for when drivetrains are added)
class RCMv3ComponentJMotorControllerClosed : public RCMv3Component {
protected:
    int inputMode;

public:
    RCMv3ComponentJMotorControllerClosed(int defaultInputMode, JMotorDriver& driver, JMotorCompensator& compensator, JEncoder& encoder, JControlLoop& controlLoop, float velLimit, float accelLimit, float distFromSetpointLimit, bool preventGoingWrongWay, float maxStoppingDecel)
        : RCMv3Component(RC_TYPE_JMotorControllerClosed)
    {
        internalInstance = new JMotorControllerClosed(driver, compensator, encoder, controlLoop, velLimit, accelLimit, distFromSetpointLimit, preventGoingWrongWay, maxStoppingDecel);
        inputMode = defaultInputMode;
    }
    void enable()
    {
        ((JMotorControllerClosed*)internalInstance)->enable();
    }
    void disable()
    {
        ((JMotorControllerClosed*)internalInstance)->disable();
    }
    void run()
    {
        ((JMotorControllerClosed*)internalInstance)->run();
    }
    void write(int index, float value)
    {
        if (index == 0) {
            inputMode = value;
        } else if (index == 1) {
            switch (inputMode) {
            case 0:
                ((JMotorControllerClosed*)internalInstance)->setAccelPosDelta(value, false, true);
                break;
            case 1:
                ((JMotorControllerClosed*)internalInstance)->setPosTarget(value, false);
                break;
            }
        }
    }
    float read(int index)
    {
        switch (index) {
        case 0:
            return ((JMotorControllerClosed*)internalInstance)->getVel();
        case 1:
            return ((JMotorControllerClosed*)internalInstance)->getPos();
        }
        return 0;
    }
    ~RCMv3ComponentJMotorControllerClosed()
    {
        delete (JMotorControllerClosed*)internalInstance;
    }
};

class RCMv3ComponentJVoltageCompConst : public RCMv3Component {
public:
    RCMv3ComponentJVoltageCompConst(float voltage)
        : RCMv3Component(RC_TYPE_JVoltageCompConst)
    {
        internalInstance = new JVoltageCompConst(voltage);
    }
    float read(int index)
    {
        return ((JVoltageCompConst*)internalInstance)->getSupplyVoltage();
    }
    ~RCMv3ComponentJVoltageCompConst()
    {
        delete (JVoltageCompConst*)internalInstance;
    }
};

class RCMv3ComponentJDrivetrainTwoSide : public RCMv3Component {
protected:
    JTwoDTransform twoDTransform;

public:
    RCMv3ComponentJDrivetrainTwoSide(JMotorController& left, JMotorController& right, float width)
        : RCMv3Component(RC_TYPE_JDrivetrainTwoSide)
    {
        internalInstance = new JDrivetrainTwoSide(left, right, width);
    }
    void enable()
    {
        ((JDrivetrainTwoSide*)internalInstance)->enable();
    }
    void disable()
    {
        ((JDrivetrainTwoSide*)internalInstance)->disable();
    }
    void run()
    {
        ((JDrivetrainTwoSide*)internalInstance)->setVel(twoDTransform, false);
        ((JDrivetrainTwoSide*)internalInstance)->run();
    }
    void write(int index, float value)
    {
        if (index == 0) {
            twoDTransform.theta = value;
        } else if (index == 1) {
            twoDTransform.x = value;
        } else if (index == 2) {
            twoDTransform.y = value;
        }
    }
    float read(int index)
    {
        switch (index) {
        case 0:
            return ((JDrivetrainTwoSide*)internalInstance)->getVel().theta;
        case 1:
            return ((JDrivetrainTwoSide*)internalInstance)->getVel().x;
        case 2:
            return ((JDrivetrainTwoSide*)internalInstance)->getVel().y;
        }
        return 0;
    }
    ~RCMv3ComponentJDrivetrainTwoSide()
    {
        delete (JDrivetrainTwoSide*)internalInstance;
    }
};

class RCMv3ComponentJDrivetrainMecanum : public RCMv3Component {
protected:
    JTwoDTransform twoDTransform;

public:
    RCMv3ComponentJDrivetrainMecanum(JMotorController& FRmotor, JMotorController& FLmotor, JMotorController& BLmotor, JMotorController& BRmotor, float forwardsScalar, float rightScalar, float CCWScalar)
        : RCMv3Component(RC_TYPE_JDrivetrainMecanum)
    {
        internalInstance = new JDrivetrainMecanum(FRmotor, FLmotor, BLmotor, BRmotor, { forwardsScalar, rightScalar, CCWScalar });
    }
    void enable()
    {
        ((JDrivetrainMecanum*)internalInstance)->enable();
    }
    void disable()
    {
        ((JDrivetrainMecanum*)internalInstance)->disable();
    }
    void run()
    {
        ((JDrivetrainMecanum*)internalInstance)->setVel(twoDTransform, false);
        ((JDrivetrainMecanum*)internalInstance)->run();
    }
    void write(int index, float value)
    {
        if (index == 0) {
            twoDTransform.theta = value;
        } else if (index == 1) {
            twoDTransform.x = value;
        } else if (index == 2) {
            twoDTransform.y = value;
        }
    }
    float read(int index)
    {
        switch (index) {
        case 0:
            return ((JDrivetrainMecanum*)internalInstance)->getVel().theta;
        case 1:
            return ((JDrivetrainMecanum*)internalInstance)->getVel().x;
        case 2:
            return ((JDrivetrainMecanum*)internalInstance)->getVel().y;
        }
        return 0;
    }
    ~RCMv3ComponentJDrivetrainMecanum()
    {
        delete (JDrivetrainMecanum*)internalInstance;
    }
};

boolean refreshOutputListSize = true;

class RCMv3ComponentFactory {
protected:
    static boolean verifyThatDataIsComponent(std::vector<RCMv3Component*>& components, JsonVariant data, const std::vector<RCMv3ComponentType>& allowedComponentType)
    {
        if (!data.is<int>()) {
            return false;
        }
        int componentIndex = data;
        if (componentIndex < 0 || componentIndex >= components.size()) {
            create_component_error_msg += " there is no component #" + String(componentIndex) + " above the current component ";
            return false;
        }
        boolean validType = false;
        for (int i = 0; i < allowedComponentType.size(); i++) {
            if (components[componentIndex]->getType() == allowedComponentType[i]) {
                validType = true;
                break;
            }
        }
        if (validType == false) {
            create_component_error_msg += " component #" + String(componentIndex) + " is not an allowed component type";
            return false;
        }
        return true;
    }

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
                create_component_error_msg += " invalid data for parameter " + String(i);
                return false;
            }
            switch (params[i].type) {
            case RC_DATA_Int: {
                if (!data[i].is<int>()) {
                    create_component_error_msg += " invalid int value for parameter " + String(i);
                    return false;
                }
            } break;
            case RC_DATA_Float: {
                if (!data[i].is<float>()) {
                    if (data[i].is<String>()) { // JSON can't handle Infinity, -Infinity, or NaN, so they are sent as strings and handled in special cases
                        String str = data[i].as<String>();
                        if (str == "Infinity") {
                            data[i].set((float)INFINITY);
                        } else if (str == "-Infinity") {
                            data[i].set((float)-INFINITY);
                        } else if (str == "NaN") {
                            data[i].set((float)NAN);
                        } else {
                            create_component_error_msg += " invalid 'float' value for parameter " + String(i);
                            return false;
                        }
                    } else {
                        create_component_error_msg += " invalid float value for parameter " + String(i);
                        return false;
                    }
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
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_TMC7300IC }) == false) {
                    return false;
                }
            } break;
            case RC_DATA_BSED: {
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_BSED }) == false) {
                    return false;
                }
            } break;
            case RC_DATA_ServoDriver: {
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_JMotorDriverEsp32Servo }) == false) {
                    return false;
                }
            } break;
            case RC_DATA_VoltageComp: {
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_JVoltageCompMeasure, RC_TYPE_JVoltageCompConst }) == false) {
                    return false;
                }
            } break;
            case RC_DATA_JMotorDriver: {
                if (verifyThatDataIsComponent(components, data[i],
                        { RC_TYPE_JMotorDriverTMC7300, RC_TYPE_JMotorDriverEsp32Servo, RC_Type_JMotorDriverEsp32HBridge })
                    == false) {
                    return false;
                }
            } break;
            case RC_DATA_JMotorCompensator: {
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_JMotorCompBasic }) == false) {
                    return false;
                }
            } break;
            case RC_DATA_JEncoder: {
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_JEncoderBSED, RC_TYPE_JEncoderQuadrature }) == false) {
                    return false;
                }
            } break;
            case RC_DATA_JControlLoop: {
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_JControlLoopBasic }) == false) {
                    return false;
                }
            } break;
            case RC_DATA_JMotorController: {
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_JMotorControllerOpen, RC_TYPE_JMotorControllerClosed }) == false) {
                    return false;
                }
            } break;
            case RC_DATA_JDrivetrain: {
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_JDrivetrainTwoSide, RC_TYPE_JDrivetrainMecanum }) == false) {
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
                    create_component_error_msg += " invalid input index type ";
                    return false;
                }
            } break;
            case RC_DATA_WhichWire: {
                if (!data[i].is<int>()) {
                    return false;
                }
            } break;
            case RC_DATA_PCA9685: {
                if (verifyThatDataIsComponent(components, data[i], { RC_TYPE_PCA9685 }) == false) {
                    return false;
                }
            } break;
            default:
                create_component_error_msg += " unknown data type for parameter " + String(i);
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
            JMotorDriverServo* servo = (JMotorDriverServo*)components[(int)data[1]]->getInternalInstance();
            Serial.printf("creating JServoController with defaultInputMode %d and servo %d and reverse %d and velLimit %f and accelLimit %f and decelLimit %f and minAngleLimit %f and maxAngleLimit %f and defaultAngle %f and minSetAngle %f and maxSetAngle %f and minServoVal %d and maxServoVal %d\n", (int)data[0], (int)data[1], (int)data[2], (float)data[3], (float)data[4], (float)data[5], (float)data[6], (float)data[7], (float)data[8], (float)data[9], (int)data[10], (int)data[11], (int)data[12]);
            components.push_back(new RCMv3ComponentJServoController((int)data[0], *servo, (boolean)data[2], (float)data[3], (float)data[4], (float)data[5], (float)data[6], (float)data[7], (float)data[8], (float)data[9], (float)data[10], (int)data[11], (int)data[12]));
        } break;
        case RC_TYPE_JMotorCompBasic: {
            JVoltageCompensator* voltageComp = (JVoltageCompensator*)components[(int)data[0]]->getInternalInstance();
            Serial.printf("creating JMotorCompBasic with voltageComp %d and volts per speed %f and min speed %f\n", (int)data[0], (float)data[1], (float)data[2]);
            components.push_back(new RCMv3ComponentJMotorCompBasic(*voltageComp, (float)data[1], (float)data[2]));
        } break;
        case RC_TYPE_JControlLoopBasic: {
            Serial.printf("creating JControlLoopBasic with kP %f and timeout %lu and noReverseVoltage %d\n", (float)data[0], (unsigned long)max(0, (int)data[1]), (int)data[2]);
            components.push_back(new RCMv3ComponentJControlLoopBasic((float)data[0], (unsigned long)data[1], (boolean)data[2]));
        } break;
        case RC_TYPE_JMotorControllerOpen: {
            JMotorDriver* driver = (JMotorDriver*)components[(int)data[1]]->getInternalInstance();
            JMotorCompensator* compensator = (JMotorCompensator*)components[(int)data[2]]->getInternalInstance();
            Serial.printf("creating JMotorControllerOpen with defaultInputMode %d and driver %d and compensator %d and velLimit %f and accelLimit %f and minMotorPulseTime %d\n", (int)data[0], (int)data[1], (int)data[2], (float)data[3], (float)data[4], (int)data[5]);
            components.push_back(new RCMv3ComponentJMotorControllerOpen((int)data[0], *driver, *compensator, (float)data[3], (float)data[4], (int)data[5]));
        } break;
        case RC_TYPE_JMotorControllerClosed: {
            JMotorDriver* driver = (JMotorDriver*)components[(int)data[1]]->getInternalInstance();
            JMotorCompensator* compensator = (JMotorCompensator*)components[(int)data[2]]->getInternalInstance();
            JEncoder* encoder = (JEncoder*)components[(int)data[3]]->getInternalInstance();
            JControlLoop* controlLoop = (JControlLoop*)components[(int)data[4]]->getInternalInstance();
            Serial.printf("creating JMotorControllerClosed with defaultInputMode %d, driver %d, compensator %d, encoder %d, controlLoop %d, velLimit %f, accelLimit %f, distFromSetpointLimit %f, preventGoingWrongWay %d, maxStoppingDecel %f\n", (int)data[0], (int)data[1], (int)data[2], (int)data[3], (int)data[4], (float)data[5], (float)data[6], (float)data[7], (int)data[8], (float)data[9]);
            components.push_back(new RCMv3ComponentJMotorControllerClosed((int)data[0], *driver, *compensator, *encoder, *controlLoop, (float)data[5], (float)data[6], (float)data[7], (bool)data[8], (float)data[9]));
        } break;
        case RC_TYPE_JVoltageCompConst: {
            Serial.printf("creating JVoltageCompConst with voltage %f\n", (float)data[0]);
            components.push_back(new RCMv3ComponentJVoltageCompConst((float)data[0]));
        } break;
        case RC_TYPE_PCA9685: {
            Serial.printf("creating PCA9685 with wire %d and address %d and pinSDA %d and pinSCL %d and outputEnablePin %d and pwmFrequency %d\n", (int)data[0], (int)data[1], (int)data[2], (int)data[3], (int)data[4], (int)data[5]);
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
            components.push_back(new RCMv3ComponentPCA9685(selectedWire, (int)data[1], (int)data[2], (int)data[3], (int)data[4], (int)data[5]));
        } break;
        case RC_TYPE_JMotorDriverPCA9685HBridge: {
            PCA9685* pca9685 = (PCA9685*)components[(int)data[0]]->getInternalInstance();
            Serial.printf("creating JMotorDriverPCA9685HBridge with pca9685 %d and channelPos %d and channelNeg %d and reverse %d and breakWhenEnabled %d and breakWhenDisabled %d\n", (int)data[0], (int)data[1], (int)data[2], (int)data[3], (int)data[4], (int)data[5]);
            components.push_back(new RCMv3ComponentJMotorDriverPCA9685HBridge(*pca9685, (int)data[1], (int)data[2], (bool)data[3], (bool)data[4], (bool)data[5]));
        } break;
        case RC_TYPE_JEncoderQuadrature: {
            Serial.printf("creating JEncoderQuadrature with pinA %d and pinB %d and distPerCountFactor %f and reverse %d and slowestIntervalMicros %d\n", (int)data[0], (int)data[1], (float)data[2], (bool)data[3], (int)data[4]);
            RCMv3ComponentJEncoderQuadrature* newEncoder = new RCMv3ComponentJEncoderQuadrature((byte)data[0], (byte)data[1], (float)data[2], (bool)data[3], (int)data[4]);
            if (newEncoder && newEncoder->successfullyCreatedISRs()) {
                Serial.println("        encoder created successfully");
                components.push_back(newEncoder);
            } else {
                create_component_error_msg += " failed to create ISRs";
                delete newEncoder;
                return false;
            }
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
