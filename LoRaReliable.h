#ifndef LORARELIABLE_H
#define LORARELIABLE_H

#include <Arduino.h>
#include <Stream.h>
#include "config.h"

struct Packet
{
    uint16_t id;
    char data[LORA_MAX_DATA_LENGTH];

    bool used;
    bool ackReceived;

    uint8_t retry;

    unsigned long sendTime;
};

class LoRaReliable
{
public:

    LoRaReliable();

    void begin(Stream &serial,
               uint8_t myAddress,
               uint8_t peerAddress,
               uint8_t channel);

    void update();

    bool send(const char *msg);

    bool available();

    String read();

private:

    Stream *port;

    uint8_t myAddr;
    uint8_t peerAddr;
    uint8_t channel;
    uint16_t lastReceivedID;

    Packet txBuffer[LORA_BUFFER_SIZE];

    uint16_t nextID;

    String rxData;

    bool enqueue(const char *msg);

    Packet* getFirstPacket();

    void sendPacket(Packet *p);

    void processReceive();

    void processACK();

    void checkTimeout();

};

#endif
