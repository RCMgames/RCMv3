#ifndef RCMROS_H
#define RCMROS_H
#include "rcm.h"

// rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rclc_executor_t executor;
rcl_node_t node;

void error_loop()
{
    // error in ROS communication
    while (1) {
#if (RCM_HARDWARE_VERSION == RCM_BYTE_V2 || RCM_HARDWARE_VERSION == RCM_NIBBLE_V1)
        // bytes and nibbles have QT PYs with neopixel built in leds
        delay(50);
        setRSL(RSLcolor);
        delay(50);
        setRSL(CRGB(0, 0, 0));
#else
        digitalWrite(ONBOARD_LED, !digitalRead(ONBOARD_LED));
        delay(50);
#endif
    }
}

/**
 * @brief  macro to declare a publisher and message
 * @param name the name of the publisher and message
 * @param type the type of ROS message
 */
#define declarePub(name, type) \
    rcl_publisher_t name##Pub; \
    type name##Msg;

/**
 * @brief  macro to declare a subscriber and start the definition of the callback function
 * @note this macro opens a curly brace for the function that needs to be closed outside the macro
 * @param name the name of the subscriber and message
 * @param type the type of ROS message
 */
#define declareSubAndCallback(name, type)                \
    rcl_subscription_t name##Sub;                        \
    type name##Msg;                                      \
    void name##_subscription_callback(const void* msgin) \
    {                                                    \
        const type* name##Msg = (const type*)msgin;

/**
 * @brief  macro to create a publisher
 * @param name the name of the publisher
 * @param type the type of ROS message
 * @param topic the topic to publish to
 */
#define createPublisher(name, type, topic)                               \
    RCCHECK(rclc_publisher_init_best_effort(                             \
        &name##Pub,                                                      \
        &node,                                                           \
        rosidl_typesupport_c__get_message_type_support_handle__##type(), \
        topic));

/**
 * @brief  macro to create a subscriber
 * @param name the name of the subscriber
 * @param type the type of ROS message
 * @param topic the topic to subscribe to
 */
#define addSub(name, type, topic)                                        \
    RCCHECK(rclc_subscription_init_default(                              \
        &name##Sub,                                                      \
        &node,                                                           \
        rosidl_typesupport_c__get_message_type_support_handle__##type(), \
        topic));                                                         \
    RCCHECK(rclc_executor_add_subscription(&executor, &name##Sub, &name##Msg, &name##_subscription_callback, ON_NEW_DATA));

#define RCCHECK(fn)                    \
    {                                  \
        rcl_ret_t temp_rc = fn;        \
        if ((temp_rc != RCL_RET_OK)) { \
            error_loop();              \
        }                              \
    }
#define RCSOFTCHECK(fn)                \
    {                                  \
        rcl_ret_t temp_rc = fn;        \
        if ((temp_rc != RCL_RET_OK)) { \
            ROSCheckFail = true;       \
        }                              \
    }

#define publish(name) RCSOFTCHECK(rcl_publish(&name##Pub, &name##Msg, NULL))

#define rosSpin(time) rclc_executor_spin_some(&executor, RCL_MS_TO_NS(time))

void ROSWifiSettings();
void ROSbegin();
char* nodeName;
char* namespaceVar = "";
int rosWifiTimeout = 1500;
int numSubscribers = 10;
#include <example_interfaces/msg/bool.h>

declareSubAndCallback(enabled, example_interfaces__msg__Bool);
enabled = enabledMsg->data;
ROSCheckFail = false;
lastEnableSentMillis = millis();
} // end of callback

void setupROS()
{
    ROSWifiSettings();
    allocator = rcl_get_default_allocator();
    RCCHECK(rclc_executor_init(&executor, &support.context, numSubscribers, &allocator));
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    RCCHECK(rclc_node_init_default(&node, nodeName, namespaceVar, &support));

    addSub(enabled, example_interfaces__msg__Bool, "/rcm/enabled");

    ROSbegin();
}

#endif // RCMROS_H
