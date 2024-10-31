#ifndef RCMV3_BOARDS_H
#define RCMV3_BOARDS_H

/*
*
* {
  "components": [
    {
      "type": 0,
      "name": "TMC7300 IC",
      "username": "AB",
      "inputs": [],
      "outputs": [],
      "parameters": [
        7,
        3
      ]
    },
    {
      "type": 1,
      "name": "Motor Driver TMC7300",
      "username": "B",
      "inputs": [
        1
      ],
      "outputs": [],
      "parameters": [
        0,
        true,
        6
      ]
    },
    {
      "type": 2,
      "name": "Motor Driver Servo ESP32",
      "username": "1",
      "inputs": [
        1
      ],
      "outputs": [],
      "parameters": [
        0,
        5
      ]
    },
    {
      "type": 1,
      "name": "Motor Driver TMC7300",
      "username": "A",
      "inputs": [
        1
      ],
      "outputs": [],
      "parameters": [
        0,
        false,
        16
      ]
    }
  ]
}
* */

/*
    "default_components":[
        //CommponentListToJsonString style
    ],
    "data_type_presets":[
        {"type": RC_DATA_Pin, "name": "pin1", "val": 1}
    ],
    "component_presets":[

    ]
    }
    */

#ifdef RCM_MCU_ESP32

#elif defined(RCM_MCU_QT_PY)

#endif // RCM_MCU_ESP32
#endif // RCMV3_BOARDS_H
