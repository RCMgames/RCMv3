# RCMv3 - Program and Control RCM Robots through a Robot-Hosted Web Interface

# work in progress
expect breaking changes

## Questions?
post [here](https://github.com/orgs/RCMgames/discussions/categories/rcmv3) or email robot.control.modules@gmail.com

## Programmer Website (start here)
https://rcmgames.github.io/RCMv3/programmer

## Driverstation
https://rcmgames.github.io/RCMv3/ds/

## Compatibility
* [RCM_V4](https://github.com/RCMgames/RCM-Hardware-V4)
* [RCM_BYTE](https://github.com/RCMgames/RCM-Hardware-BYTE)
* [RCM_Nibble](https://github.com/RCMgames/RCM-Hardware-Nibble)
* [Alfredo Systems NoU2](https://www.alfredosys.com/products/alfredo-nou2/)
* [Alfredo Systems NoU3](https://www.alfredosys.com/products/alfredo-nou3/)

### ESP32 Microcontrollers
* ESP32 DevKitC with 38 pins
* Adafruit ESP32s3 QT Py with no psram (#5426)
* Adafruit ESP32s3 QT Py with 2MB psram (#5700)
* ESP32 DevKit with 30 pins (NoU2)
* Alfredo Systems NoU3

## References
* [Information about all hardware and software for Robot Control Modules](https://github.com/rcmgames?view_as=public)
* [JMotor library documentation](https://joshua-8.github.io/JMotor/md_readme.html) (documentation for many of the motor control components)

## LED blink codes
* 2Hz: enabled
* solid on: disabled but connected
* flashes 1 time per 1.5 seconds: connected to wifi router with previously saved name and password
* flashes 2 times per 1.5 seconds: RCM has created a wifi network with previously saved name and password
* flashes 3 times per 1.5 seconds: RCM has created a wifi network named "http://rcmv3.local" with no password
    * pressing then releasing the reset/en button then immediately pressing then holding the IO0 button will force the RCM into this mode (if you forgot the wifi password)
* 10Hz: searching for wifi router on boot

## Acknowledgements
* [PlatformIO](https://registry.platformio.org/platforms/platformio/espressif32) for compiling
* [arduino-esp32](https://github.com/espressif/arduino-esp32)
* [esptool-js](https://github.com/espressif/esptool-js)
* [ArduinoJson](https://arduinojson.org/) library for using JSON data in Arduino by Benoit Blanchon
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) web server and web sockets library for ESP32 by @me-no-dev
* [Alfredo Systems](https://www.alfredosys.com/) for making opensource libraries for their NoU2 and NoU3 boards
* [FastLED](https://github.com/FastLED/FastLED) library for controlling the RGB led on QT Py
* [gobabygocarswithjoysticks/programmer](https://github.com/gobabygocarswithjoysticks/programmer) the RCM programmer reuses code from this project for programming ESP32s
* [JMotor](https://github.com/joshua-8/JMotor) library for motor control by @joshua-8
