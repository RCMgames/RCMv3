//   This program is template code for programming small esp32 powered wifi controlled robots.
//   https://github.com/rcmgames/RCMv3
//   for information see this page: https://github.com/RCMgames

#include "rcmv3.h"
#include "websocketcomms.h"

#include <vector>

void Enabled()
{
    // code to run while enabled, put your main code here
}

void Enable()
{
    // turn on outputs
    RCMV3_enable();
}

void Disable()
{
    // turn off outputs
    RCMV3_disable();
}

void PowerOn()
{
    // runs once on robot startup, set pin modes and use begin() if applicable here
    load_config_from_memory();
    RCMV3_begin();
}

void Always()
{
    // always runs if void loop is running, JMotor run() functions should be put here
    // (but only the "top level", for example if you call drivetrainController.run() you shouldn't also call leftMotorController.run())
    componentMutex.lock();
    RCMV3_run(inputs, outputs);
    componentMutex.unlock();

    delay(1);
}

void WifiDataToParse()
{
    enabled = WSC::recvBl();
    byte numInputs = WSC::recvBy();
    inputs.resize(numInputs, 0);
    // add data to read here: (EWD::recvBl, EWD::recvBy, EWD::recvIn, EWD::recvFl)(boolean, byte, int, float)
    for (int i = 0; i < numInputs; i++) {
        inputs[i] = WSC::recvFl();
    }
}
void WifiDataToSend()
{
    for (int i = 0; i < outputs.size(); i++) {
        WSC::sendFl(outputs[i]);
    }
}
// wifi name and password is set through the website

#include "rcmutil.h"
