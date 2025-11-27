#include "QProtocol.h"
#include "SPI.h"
#include "NRFLite.h"


#if defined(ESP8266) || defined(ESP32)
#include "Print.h"
#include "core_esp8266_features.h" 
#include "HardwareSerial.h"
#endif
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#endif
#if defined(ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#endif
QNetworkInterface::QNetworkInterface()
{
    outputMode = false;
    publicKey = 0;
    privateKey = 0;
    onRouteMode = false;
    availableFn = nullptr;
    readFn      = nullptr;
    writeFn     = nullptr;
    writeESPNOWFn = nullptr;
    writeNRFFn  = nullptr;
}

//SERIAL_FUNCTIONS/////////////////////////////////////////
void QPROTOCOL_SERIAL_SENDER(const QProtocolDataPacket& qp){
    Serial.write((const uint8_t*)&qp, QPACKET_SIZE);
}

QProtocolDataPacket QPROTOCOL_SERIAL_RECEIVER(){
    QProtocolDataPacket qp;
    Serial.readBytes((uint8_t*)&qp, QPACKET_SIZE);
    return qp;
}

bool QPROTOCOL_SERIAL_AVAILABLE(){
    return Serial.available()>=QPACKET_SIZE;
}
///////////////////////////////////////////////////////////

//NRF_FUNCTIONS////////////////////////////////////////////
NRFLite _radio;
void QPROTOCOL_NRF_SETUP(uint8_t RADIO_ID, uint8_t PIN_RADIO_CE, uint8_t PIN_RADIO_CSN){
  _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN);
}
void QPROTOCOL_NRF_SENDER(const QProtocolDataPacket& qp, uint8_t TARGET_RADIO_ID){
    _radio.send(TARGET_RADIO_ID, const_cast<QProtocolDataPacket*>(&qp), QPACKET_SIZE);
}
QProtocolDataPacket QPROTOCOL_NRF_RECEIVER(){
  QProtocolDataPacket qp;
  _radio.readData(&qp);
  return qp;
}
bool QPROTOCOL_NRF_AVAILABLE(){
  return _radio.hasData();
}
///////////////////////////////////////////////////////////

#if defined(ESP8266) || defined(ESP32)
//ESP-NOW_FUNCTIONS/////////////////////////////////////
void QPROTOCOL_ESPNOW_WIFI_SETUP(){WiFi.mode(WIFI_STA);if (esp_now_init() != 0) {return;}}
QProtocolDataPacket TempQP;
bool availableOnRecv;
void QPROTOCOL_ESPNOW_ADD_PEER(uint8_t broadcastAddress[]){
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}
void QPROTOCOL_ESPNOW_SENDER(const QProtocolDataPacket& qp, uint8_t broadcastAddress[]){
    esp_now_send(broadcastAddress, (uint8_t *) &qp, QPACKET_SIZE);
}
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
QProtocolDataPacket QPROTOCOL_ESPNOW_RECEIVER() {
    availableOnRecv = false;
    return TempQP;
}
bool QPROTOCOL_ESPNOW_AVAILABLE(){
    return availableOnRecv;
}
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
    availableOnRecv = true;
    memcpy(&TempQP, incomingData, QPACKET_SIZE);
}
///////////////////////////////////////////////////////////
#endif

void QNetworkInterface::setAvailable(bool (*fn)()) {availableFn = fn;}

void QNetworkInterface::setRead(QProtocolDataPacket (*fn)()) {
        #if defined(ESP8266) || defined(ESP32)
        if(fn == QPROTOCOL_ESPNOW_RECEIVER){esp_now_set_self_role(ESP_NOW_ROLE_COMBO);esp_now_register_recv_cb(OnDataRecv);}
        #endif
        readFn = fn;}

void QNetworkInterface::setWrite(void (*fn)(const QProtocolDataPacket&)) {
        writeFn = fn;}

void QNetworkInterface::setESPNOW_SENDER(void (*fn)(const QProtocolDataPacket&, uint8_t broadcastAddress[])) {
        #if defined(ESP8266) || defined(ESP32)
        if(fn == QPROTOCOL_ESPNOW_SENDER){esp_now_set_self_role(ESP_NOW_ROLE_COMBO);esp_now_register_send_cb(OnDataSent);}
        #endif
        writeESPNOWFn = fn;}

void QNetworkInterface::setNRF_SENDER(void (*fn)(const QProtocolDataPacket&, uint8_t TARGET_RADIO_ID)) {
        writeNRFFn = fn;}

void QNetworkInterface::clearSerialInput() {while (Serial.available() > 0) {Serial.read();}}

void QNetworkInterface::route()
{
    if(availableFn && readFn){ //onRead Receiver
        if (availableFn()) {
            QProtocolDataPacket qp = readFn();
            //Output
            if (outputMode){
                if (qp.key == 0x10) {
                    // Packet is valid! Process it.
                    Serial.print("SUCCESS | Key: ");
                    Serial.println(qp.key);
                    Serial.print("SUCCESS | pKey: ");
                    Serial.println(qp.pkey);
                    Serial.print("SUCCESS | Purpose: ");
                    Serial.println(qp.purpose);
                    // ... continue processing ...
                    uint8_t broadcastAddressCOM7[] = {0xa4, 0xcf, 0x12, 0xf0, 0x8e, 0x04};
                    qp.key = 0x15;
                    QPROTOCOL_ESPNOW_SENDER(qp, broadcastAddressCOM7);

                    clearSerialInput();
                } 
                else if (qp.key == 0x15) {
                    // Packet is valid! Process it.
                    Serial.print("SUCCESS | Key: ");
                    Serial.println(qp.key);
                    Serial.print("SUCCESS | pKey: ");
                    Serial.println(qp.pkey);
                    Serial.print("SUCCESS | Purpose: ");
                    Serial.println(qp.purpose);
                    // ... continue processing ...

                    clearSerialInput();
                }
                else {
                    // Data is corrupt. Discard one byte to shift the buffer contents.
                    // Next loop, availableFn() will see the remaining bytes.
                    clearSerialInput();
                    Serial.println("SYNC FAILED. Discarding byte.");
                    Serial.print("Received Key:");
                    Serial.println(qp.key, HEX);
                }
            }
        }   
    }
}
