#include <SoftwareSerial.h>
#include "config.h"
#include "LoRaReliable.h"

SoftwareSerial LoRa(10, 11);   // RX, TX

LoRaReliable Radio;

void setup()
{
    Serial.begin(9600);
    LoRa.begin(9600);

    Radio.begin(
        LoRa,
        11,     // Benim adresim
        1,      // Karşı taraf
        18      // Kanal
    );

    Serial.println("LoRaReliable V1 BASLADI");
}

void loop()
{
    Radio.update();

    if (Serial.available())
    {
        String mesaj = Serial.readStringUntil('\n');

        if (Radio.send(mesaj.c_str()))
        {
            Serial.print("BUFFER -> ");
            Serial.println(mesaj);
        }
        else
        {
            Serial.println("BUFFER DOLU");
        }
    }

    if (Radio.available())
    {
        Serial.print("GELEN -> ");
        Serial.println(Radio.read());
    }
}
