#include "src/Cryptosuite-master/sha1.h"

void printHash(uint8_t* hash) {
  int i;
  for (i=0; i<20; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

void setup() {
  uint8_t *hash;
  Sha1.initHmac("testhashkey",8); // key, and length of key in bytes
  Sha1.print("This is a message to hash");
  hash = Sha1.resultHmac();
  Serial.begin(9600);
  printHash(Sha1.result());
}

void loop() {

}
