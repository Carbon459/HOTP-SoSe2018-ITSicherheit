/*
 * Version 1.0
 * Author: Friedrich Hudinjan
 */

#include <AuthHOTP.h>
#include "src/arduino-cc1101-master/cc1101.h"
#include "src/arduino-cc1101-master/ccpacket.h"

const bool serverMode = true;

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define CC1101Interrupt 4 // Pin 19
#define CC1101_GDO0 19
#elif defined(__MK64FX512__)
// Teensy 3.5
#define CC1101Interrupt 9 // Pin 9
#define CC1101_GDO0 9
#else
#define CC1101Interrupt 1 // Pin 2
#define CC1101_GDO0 3
#endif

CC1101 radio;

byte syncWord[2] = {199, 10};
bool packetWaiting;

unsigned long lastSend = 0;
unsigned int sendDelay = 5000;

void messageReceived() {
    packetWaiting = true;
}

AuthHOTP server("12345678901234567890", 20, 6);
AuthHOTP client("12345678901234567890", 20, 6);
  
void setup() {
  radio.init();
  radio.setSyncWord(syncWord);
  radio.setCarrierFreq(CFREQ_433);
  radio.disableAddressCheck();
  radio.setTxPowerAmp(PA_LongDistance);

  Serial.begin(9600);

  if(serverMode) {
    Serial.println("Server Mode:");
  } else {
    Serial.println("Client Mode:");
  }
  Serial.println(F("CC1101 radio initialized."));
  Serial.println("-------------------------------------------");
  attachInterrupt(CC1101Interrupt, messageReceived, FALLING);
  
  /*String s = "";
  do {
    s = client.authClient("authRequest");
    s = server.authServer(s);
    s = client.authClient(s);
    delay(1000);
  } while(!s.indexOf("authSuccess") >= 0);*/
}

void loop() {
  if(serverMode) {
    serverLoop();
  } else {
    clientLoop();
  }
}

void serverLoop() {
  bool answer = false;
  String s;
      if (packetWaiting) {
        detachInterrupt(CC1101Interrupt);
        packetWaiting = false;
        CCPACKET packet;
        if (radio.receiveData(&packet) > 0) {
            Serial.println(F("Received packet..."));
            if (!packet.crc_ok) {
                Serial.println(F("crc not ok"));
            }
            if (packet.crc_ok && packet.length > 0) {
              if(server.isAuthenticated()) {
                Serial.println("Client successfully authenticated");
              } else {
                Serial.println("Client is not authenticated yet");
              }
                const char * data = (const char *) packet.data;
                Serial.println((const char *) packet.data);

                s = server.authServer(String(data));
                answer = true;
            }
        }

        attachInterrupt(CC1101Interrupt, messageReceived, FALLING);
    }

    if (answer) {
        detachInterrupt(CC1101Interrupt);
        
        char buffer[50];
        s.toCharArray(buffer, 50);
        const char *message = buffer;
        CCPACKET packet;
        // We also need to include the 0 byte at the end of the string
        packet.length = strlen(message)  + 1;
        strncpy((char *) packet.data, message, packet.length);

        radio.sendData(packet);
        Serial.println(F("Sent packet..."));
        answer = false;
        Serial.println("-------------------------------------------");
        
        attachInterrupt(CC1101Interrupt, messageReceived, FALLING);
    }  
}

void clientLoop() {
      if (packetWaiting) {
        detachInterrupt(CC1101Interrupt);
        packetWaiting = false;
        CCPACKET packet;
        if (radio.receiveData(&packet) > 0) {
            Serial.println(F("Received packet..."));
            if (!packet.crc_ok) {
                Serial.println(F("crc not ok"));
            }
            if (packet.crc_ok && packet.length > 0) {
                const char * data = (const char *) packet.data;
                Serial.println((const char *) packet.data);
                if(strcmp(data, "ready") == 0) {
                  Serial.println("READY RECIEVED");
                }
                client.authClient(String(data));
                Serial.println("-------------------------------------------");
            }
        }

        attachInterrupt(CC1101Interrupt, messageReceived, FALLING);
    }
    unsigned long now = millis();
    if (now > lastSend + sendDelay) {
        detachInterrupt(CC1101Interrupt);
        
        lastSend = now;
        char buffer[50];
        client.authClient("authRequest").toCharArray(buffer, 50);
        const char *message = buffer;
        CCPACKET packet;
        // We also need to include the 0 byte at the end of the string
        packet.length = strlen(message)  + 1;
        strncpy((char *) packet.data, message, packet.length);

        radio.sendData(packet);
        Serial.println(F("Sent packet..."));

        attachInterrupt(CC1101Interrupt, messageReceived, FALLING);
    }  
}

// Get signal strength indicator in dBm.
// See: http://www.ti.com/lit/an/swra114d/swra114d.pdf
int rssi(char raw) {
    uint8_t rssi_dec;
    // TODO: This rssi_offset is dependent on baud and MHz; this is for 38.4kbps and 433 MHz.
    uint8_t rssi_offset = 74;
    rssi_dec = (uint8_t) raw;
    if (rssi_dec >= 128)
        return ((int)( rssi_dec - 256) / 2) - rssi_offset;
    else
        return (rssi_dec / 2) - rssi_offset;
}

// Get link quality indicator.
int lqi(char raw) {
    return 0x3F - raw;
}
