#include <stdint.h>
#include <avr/pgmspace.h>
#ifndef QPROTOCOL_H
#define QPROTOCOL_H

#include <Arduino.h>

#if defined(ESP8266) || defined(ESP32)
#include "c_types.h"
#endif


// struct __attribute__((packed)) QProtocolDataPacket
// {
//   int32_t key;
//   uint8_t purpose;
//   int32_t data[10];
//   uint8_t  method;
//   int32_t target[10];
// };

struct __attribute__((packed)) QProtocolDataPacket
{
  uint8_t key;
  int32_t pkey;
  uint8_t purpose;
};

const size_t QPACKET_SIZE = sizeof(QProtocolDataPacket);


//QPROTOCOL BUILT-IN SERIAL FUNCTIONS
void QPROTOCOL_SERIAL_SENDER(const QProtocolDataPacket& qp);
QProtocolDataPacket QPROTOCOL_SERIAL_RECEIVER();
bool QPROTOCOL_SERIAL_AVAILABLE();
////////////////////////////////////

//QPROTOCOL BUILT-IN NRF24 FUNCTIONS
void QPROTOCOL_NRF_SETUP(uint8_t RADIO_ID, uint8_t PIN_RADIO_CE, uint8_t PIN_RADIO_CSN);
void QPROTOCOL_NRF_SENDER(const QProtocolDataPacket& qp, uint8_t TARGET_RADIO_ID);
QProtocolDataPacket QPROTOCOL_NRF_RECEIVER();
bool QPROTOCOL_NRF_AVAILABLE();
////////////////////////////////////

//QPROTOCOL BUILT-IN ESP-NOW FUNCTIONS
void QPROTOCOL_ESPNOW_WIFI_SETUP();
bool QPROTOCOL_ESPNOW_AVAILABLE();
void QPROTOCOL_ESPNOW_SENDER(const QProtocolDataPacket& qp, uint8_t broadcastAddress[]);
QProtocolDataPacket QPROTOCOL_ESPNOW_RECEIVER();
void QPROTOCOL_ESPNOW_RECEIVER_SRC(uint8_t * mac, uint8_t *incomingData, uint8_t len);
void QPROTOCOL_ESPNOW_ADD_PEER(uint8_t broadcastAddress[]);
////////////////////////////////////

class QNetworkInterface
{
  public:

    QNetworkInterface();
    int32_t publicKey;
    int32_t privateKey;

    void clearSerialInput();
    void setAvailable(bool (*fn)());
    void setRead(QProtocolDataPacket (*fn)());
    void setWrite(void (*fn)(const QProtocolDataPacket&));
    void setESPNOW_SENDER(void (*fn)(const QProtocolDataPacket&, uint8_t broadcastAddress[]));
    void setNRF_SENDER(void (*fn)(const QProtocolDataPacket&, uint8_t TARGET_RADIO_ID));
    void route();
    bool    outputMode;

  private:

    
    bool    onRouteMode;
    int32_t targetInfo[10];
    int32_t feedbackInfo[10];
    bool    (*availableFn)();
    void    (*writeFn)(const QProtocolDataPacket&);
    void    (*writeESPNOWFn)(const QProtocolDataPacket&, uint8_t broadcastAddress[]);
    void    (*writeNRFFn)(const QProtocolDataPacket&, uint8_t TARGET_RADIO_ID);

    QProtocolDataPacket (*readFn)();
};

#endif
