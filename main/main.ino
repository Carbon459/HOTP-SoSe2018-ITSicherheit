#include "src/Cryptosuite-master/sha1.h"

#define DEBUG

void printHash(uint8_t* hash) {
  int i;
  for (i=0; i<20; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

uint32_t gCounter = 0; //Eigentlich 8byte
uint8_t hmacKey[]= {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30};

uint8_t hashedhmac[]= {0x1f, 0x86, 0x98, 0x69, 0x0e, 0x02, 0xca, 0x16, 0x61, 0x85, 0x50, 0xef, 0x7f, 0x19, 0xda, 0x8e, 0x94, 0x5b, 0x55, 0x5a}; //OTP: 872921

/**
 * Erstellt ein 6 bis 8-stelligen Code nach dem HOTP Verfahren(RFC4226).
 * @counter Der Zählstand, der synchron gehalten werden muss mit der gegenstelle
 * @secret Ein gemeinsamer geheimer Schlüssel
 * @secretSize Länge dieses Schlüssels
 * @digits Gültige Werte: 6,7,8. Gibt die Anzahl der Stellen des ausgegeben OTPs an.
 * 
 * @returns 6-8 stelliges Einmalpasswort. Bei ungültiger Anzahl an stellen wird 0 zurückgegeben.
 */
uint32_t hotp(uint32_t counter, uint8_t* secret, size_t secretSize, uint8_t digits) {
  
  uint64_t divider;
  if(digits == 6) {divider = 1000000;}          //10^6
  else if(digits == 7) {divider = 10000000;}    //10^7
  else if(digits == 8) { divider = 100000000;}  //10^8
  else {return 0;}

  //SHA1 HMAC Hash
  uint8_t *hash;
  Sha1.initHmac(secret,secretSize); // key, and length of key in bytes
  Sha1.print(counter);
  hash = Sha1.resultHmac();
  
  #ifdef DEBUG
  printHash(hash); //cc93cf18508d94934c64b65d8ba7667fb7cde4b0 gewünschte ergebnis
  #endif
  
  uint8_t offset   =  hashedhmac[19] & 0xf;

  uint32_t otp = 0;
  for(int i = 0; i < 4; i++) { //Byte Offset bis Offset+3 zusammensetzen
    otp = (otp << 8) | hashedhmac[offset + i];
  }
  otp = otp & 0x7FFFFFFF; //Erste Byte maskieren, aufgrund unsigned vs signed modulo berechnung auf verschiedenen prozessoren.
  otp = otp % divider;
  return otp;
}

void setup() {
  Serial.begin(9600);
  uint32_t otp = hotp(gCounter, hmacKey, 20, 6);
  Serial.println(otp);
}

void loop() {

}
/*
   The following test data uses the ASCII string
   "12345678901234567890" for the secret:

   Secret = 0x3132333435363738393031323334353637383930

   Table 1 details for each count, the intermediate HMAC value.

   Count    Hexadecimal HMAC-SHA-1(secret, count)
   0        cc93cf18508d94934c64b65d8ba7667fb7cde4b0
   1        75a48a19d4cbe100644e8ac1397eea747a2d33ab
   2        0bacb7fa082fef30782211938bc1c5e70416ff44
   3        66c28227d03a2d5529262ff016a1e6ef76557ece
   4        a904c900a64b35909874b33e61c5938a8e15ed1c
   5        a37e783d7b7233c083d4f62926c7a25f238d0316
   6        bc9cd28561042c83f219324d3c607256c03272ae
   7        a4fb960c0bc06e1eabb804e5b397cdc4b45596fa
   8        1b3c89f65e6c9e883012052823443f048b4332db
   9        1637409809a679dc698207310c8c7fc07290d9e5

   Table 2 details for each count the truncated values (both in
   hexadecimal and decimal) and then the HOTP value.

                     Truncated
   Count    Hexadecimal    Decimal        HOTP
   0        4c93cf18       1284755224     755224
   1        41397eea       1094287082     287082
   2         82fef30        137359152     359152
   3        66ef7655       1726969429     969429
   4        61c5938a       1640338314     338314
   5        33c083d4        868254676     254676
   6        7256c032       1918287922     287922
   7         4e5b397         82162583     162583
   8        2823443f        673399871     399871
   9        2679dc69        645520489     520489
 */
