#ifndef WEBSOCKETCOMMS_H
#define WEBSOCKETCOMMS_H
#include "websiteserver.h" // uses the same AsyncWebServer server

#include <Arduino.h>
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer

namespace WSC {

AsyncWebSocket ws("/control");

boolean debugPrint = true;

unsigned long signalLossTimeout = 1000;

int maxWifiRecvBufSize = 41;
byte recvdData[41];

int maxWifiSendBufSize = 12;
byte dataToSend[12];

unsigned long lastMessageTimeMillis = 0;
boolean wifiConnected = false;
boolean receivedNewData = false;

int wifiArrayCounter = 0;

void (*sendCallback)(void);
void (*receiveCallback)(void);

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
{
    if (type == WS_EVT_CONNECT) {
        if (debugPrint) {
            Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        }
        client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
        // client disconnected
        if (debugPrint) {
            Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
        }
        wifiConnected = false;
    } else if (type == WS_EVT_ERROR) {
        // error was received from the other end
        if (debugPrint) {
            Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
        }
    } else if (type == WS_EVT_PONG) {
        // pong message was received (in response to a ping request maybe)

        if (debugPrint) {
            Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
        }
        wifiConnected = true;
    } else if (type == WS_EVT_DATA) {
        // data packet
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
            // the whole message is in a single frame and we got all of its data
            if (debugPrint) {
                Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
            }
            if (info->opcode == WS_BINARY) {
                lastMessageTimeMillis = millis();
                receivedNewData = true;
                if (debugPrint) {
                    for (size_t i = 0; i < info->len; i++) {
                        Serial.printf("%02x ", data[i]);
                    }
                    Serial.println();
                }
                for (size_t i = 0; i < info->len; i++) {
                    if (i < maxWifiRecvBufSize) {
                        recvdData[i] = data[i];
                    }
                }
                wifiArrayCounter = 0;
                receiveCallback();
                wifiArrayCounter = 0;
                sendCallback();
                ws.binary(client->id(), dataToSend, wifiArrayCounter);
            }
        }
    }
}

void startWebSocketComms(void (*_recvCB)(void), void (*_sendCB)(void))
{
    sendCallback = _sendCB;
    receiveCallback = _recvCB;

    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void runWebSocketComms()
{
    receivedNewData = false;
    ws.cleanupClients();
}

boolean recvBl()
{ // return boolean at pos position in recvdData
    byte msg = recvdData[min(wifiArrayCounter, maxWifiRecvBufSize - 1)];
    wifiArrayCounter++; // increment the counter for the next value
    return (msg == 1);
}

byte recvBy()
{ // return byte at pos position in recvdData
    byte msg = recvdData[min(wifiArrayCounter, maxWifiRecvBufSize - 1)];
    wifiArrayCounter++; // increment the counter for the next value
    return msg;
}

//  great reference on unions, which make this code work: https://www.mcgurrin.info/robots/127/
int recvIn()
{ // return int from four bytes starting at pos position in recvdData (esp32s use 4 byte ints)
    union { // this lets us translate between two variable types (equal size, but one's four bytes in an array, and one's a four byte int)  Reference for unions: https://www.mcgurrin.info/robots/127/
        byte b[4];
        int v;
    } d; // d is the union, d.b accesses the byte array, d.v accesses the int

    for (int i = 0; i < 4; i++) {
        d.b[i] = recvdData[min(wifiArrayCounter, maxWifiRecvBufSize - 1)];
        wifiArrayCounter++;
    }

    return d.v; // return the int form of union d
}

float recvFl()
{ // return float from 4 bytes starting at pos position in recvdData
    union { // this lets us translate between two variable types (equal size, but one's 4 bytes in an array, and one's a 4 byte float) Reference for unions: https://www.mcgurrin.info/robots/127/
        byte b[4];
        float v;
    } d; // d is the union, d.b accesses the byte array, d.v accesses the float

    for (int i = 0; i < 4; i++) {
        d.b[i] = recvdData[min(wifiArrayCounter, maxWifiRecvBufSize - 1)];
        wifiArrayCounter++;
    }

    return d.v;
}

void sendBl(boolean msg)
{ // add a boolean to the dataToSend array
    dataToSend[min(wifiArrayCounter, maxWifiSendBufSize - 1)] = msg;
    wifiArrayCounter++;
}

void sendBy(byte msg)
{ // add a byte to the dataToSend array
    dataToSend[min(wifiArrayCounter, maxWifiSendBufSize - 1)] = msg;
    wifiArrayCounter++;
}

//  great reference on unions, which make this code work: https://www.mcgurrin.info/robots/127/
void sendIn(int msg)
{ // add an int to the dataToSend array (four bytes, esp32s use 4 byte ints)
    union {
        byte b[4];
        int v;
    } d; // d is the union, d.b accesses the byte array, d.v accesses the int (equal size, but one's 4 bytes in an array, and one's a 4 byte int Reference for unions: https://www.mcgurrin.info/robots/127/

    d.v = msg; // put the value into the union as an int

    for (int i = 0; i < 4; i++) {
        dataToSend[min(wifiArrayCounter, maxWifiSendBufSize - 1)] = d.b[i];
        wifiArrayCounter++;
    }
}

void sendFl(float msg)
{ // add a float to the dataToSend array (four bytes)
    union { // this lets us translate between two variables (equal size, but one's 4 bytes in an array, and one's a 4 byte float Reference for unions: https://www.mcgurrin.info/robots/127/
        byte b[4];
        float v;
    } d; // d is the union, d.b accesses the byte array, d.v accesses the float

    d.v = msg;

    for (int i = 0; i < 4; i++) {
        dataToSend[min(wifiArrayCounter, maxWifiSendBufSize - 1)] = d.b[i];
        wifiArrayCounter++;
    }
}

unsigned long millisSinceMessage()
{
    return millis() - lastMessageTimeMillis;
}
bool notTimedOut()
{
    return millis() - lastMessageTimeMillis < signalLossTimeout;
}
bool timedOut()
{
    return millis() - lastMessageTimeMillis >= signalLossTimeout;
}
bool newData()
{
    return receivedNewData;
}
bool getWifiConnected()
{
    return wifiConnected;
}

}; // namespace WSC

#endif // WEBSOCKETCOMMS_H
