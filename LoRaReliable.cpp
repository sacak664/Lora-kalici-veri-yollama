#include "LoRaReliable.h"

LoRaReliable::LoRaReliable()
{
    port = NULL;

    myAddr = 0;
    peerAddr = 0;
    channel = 0;

    nextID = 1;
    lastReceivedID = 0;

    rxData = "";

    for (int i = 0; i < LORA_BUFFER_SIZE; i++)
    {
        txBuffer[i].used = false;
        txBuffer[i].ackReceived = false;
        txBuffer[i].retry = 0;
        txBuffer[i].sendTime = 0;
    }
}

void LoRaReliable::begin(Stream &serial,
                         uint8_t myAddress,
                         uint8_t peerAddress,
                         uint8_t ch)
{
    port = &serial;

    myAddr = myAddress;
    peerAddr = peerAddress;
    channel = ch;
}

bool LoRaReliable::enqueue(const char *msg)
{
    for (int i = 0; i < LORA_BUFFER_SIZE; i++)
    {
        if (!txBuffer[i].used)
        {
            txBuffer[i].used = true;

            txBuffer[i].ackReceived = false;

            txBuffer[i].retry = 0;

            txBuffer[i].sendTime = 0;

            txBuffer[i].id = nextID++;

            strncpy(txBuffer[i].data,
                    msg,
                    LORA_MAX_DATA_LENGTH - 1);

            txBuffer[i].data[LORA_MAX_DATA_LENGTH - 1] = '\0';

            return true;
        }
    }

    return false;
}

bool LoRaReliable::send(const char *msg)
{
    return enqueue(msg);
}

Packet *LoRaReliable::getFirstPacket()
{
    for (int i = 0; i < LORA_BUFFER_SIZE; i++)
    {
        if (txBuffer[i].used && !txBuffer[i].ackReceived)
            return &txBuffer[i];
    }

    return NULL;
}

void LoRaReliable::sendPacket(Packet *p)
{
    port->write((byte)0);
    port->write(peerAddr);
    port->write(channel);

    port->print('#');

    port->print(p->id);

    port->print('|');
    

    port->print(strlen(p->data));

    port->print('|');

    port->print('D');

    port->print('|');

    port->println(p->data);

    p->sendTime = millis();
    Serial.print("SEND ID=");
    Serial.print(p->id);
    Serial.print(" Retry=");
    Serial.println(p->retry);
}

void LoRaReliable::processReceive()
{
    if (!port->available())
        return;

    String packet = port->readStringUntil('\n');
    Serial.print("RX RAW = ");
    Serial.println(packet);

    packet.trim();

    if (packet.length() == 0)
        return;

    // Paket # ile başlamıyorsa geçersiz
    if (packet[0] != '#')
        return;

    int p1 = packet.indexOf('|');
int p2 = packet.indexOf('|', p1 + 1);

if (p1 == -1 || p2 == -1)
    return;

uint16_t id = packet.substring(1, p1).toInt();
uint8_t len = packet.substring(p1 + 1, p2).toInt();

// ACK paketi mi?
if (packet.endsWith("|A"))
{
    Serial.print("ACK GELDI ID=");
    Serial.println(id);

    for (int i = 0; i < LORA_BUFFER_SIZE; i++)
    {
        if (txBuffer[i].used && txBuffer[i].id == id)
        {
            txBuffer[i].ackReceived = true;
            txBuffer[i].used = false;

            Serial.println("PAKET TAMAMLANDI");
            break;
        }
    }

    return;
}

// DATA paketi için üçüncü | gerekli
int p3 = packet.indexOf('|', p2 + 1);

if (p3 == -1)
    return;

char type = packet.substring(p2 + 1, p3).charAt(0);

String payload = packet.substring(p3 + 1);

    // ---------- DATA ----------
    if (type == 'D')
 {

    // Daha önce gelen paket mi?
    if(id == lastReceivedID)
    {

        Serial.print("DUPLICATE PAKET ID=");
        Serial.println(id);

        // ACK yine gönder
        port->write((byte)0);
        port->write(peerAddr);
        port->write(channel);

        port->print('#');
        port->print(id);
        port->println("|0|A");

        return;
    }


    // Yeni paket
    lastReceivedID = id;

    rxData = payload;


    Serial.print("YENI DATA ID=");
    Serial.println(id);


    // ACK gönder
    port->write((byte)0);
    port->write(peerAddr);
    port->write(channel);

    port->print('#');
    port->print(id);
    port->println("|0|A");

 }

    // ---------- ACK ----------
    else if (type == 'A')
    {
        Serial.print("ACK GELDI ID=");
        Serial.println(id);

        for (int i = 0; i < LORA_BUFFER_SIZE; i++)
        {
          if (txBuffer[i].used && txBuffer[i].id == id)
          {
              txBuffer[i].ackReceived = true;
              txBuffer[i].used = false;

              Serial.println("PAKET TAMAMLANDI");
              break;
          }
        }
    }
}
void LoRaReliable::processACK()
{
    // Şimdilik boş
}

void LoRaReliable::checkTimeout()
{
    for (int i = 0; i < LORA_BUFFER_SIZE; i++)
    {
        if (!txBuffer[i].used)
            continue;

        if (txBuffer[i].ackReceived)
            continue;

        if (txBuffer[i].sendTime == 0)
            continue;

        unsigned long timeout;

        // İlk 5 deneme hızlı
        if (txBuffer[i].retry < LORA_MAX_RETRY)
        {
           timeout = LORA_ACK_TIMEOUT;     // örn. 500 ms
        }
        else
        {
           timeout = 5000;                 // sonra 5 saniyede bir
        }

        if (millis() - txBuffer[i].sendTime >= timeout)
        {
           txBuffer[i].retry++;

           if (txBuffer[i].retry == LORA_MAX_RETRY + 1)
           { 
              Serial.println("ALICI YOK - YAVAS YENIDEN DENEME MODU");
           }  
           sendPacket(&txBuffer[i]);
        }
    }
}

void LoRaReliable::update()
{
    processReceive();

    static unsigned long last = 0;

    if (millis() - last > 50)
    {
        last = millis();

        Packet *p = getFirstPacket();

        if (p != NULL)
        {
            // Daha önce hiç gönderilmediyse gönder
            if (p->sendTime == 0)
            {
                sendPacket(p);
            }
        }
    }

    checkTimeout();
}

bool LoRaReliable::available()
{
    return rxData.length() > 0;
}

String LoRaReliable::read()
{
    String temp = rxData;

    rxData = "";

    return temp;
}
