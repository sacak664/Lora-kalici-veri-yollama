#ifndef CONFIG_H
#define CONFIG_H

// -------------------------------
// LoRa Ayarları
// -------------------------------

#define LORA_BUFFER_SIZE      10      // Buffer'da tutulacak maksimum paket
#define LORA_MAX_DATA_LENGTH  32      // Bir paketin maksimum veri uzunluğu

// ACK zaman aşımı (ms)
#define LORA_ACK_TIMEOUT      500

// Maksimum tekrar gönderme
#define LORA_MAX_RETRY        5

// Paket başlangıç karakterleri
#define PACKET_START          '#'
#define ACK_START             "ACK"

#endif
