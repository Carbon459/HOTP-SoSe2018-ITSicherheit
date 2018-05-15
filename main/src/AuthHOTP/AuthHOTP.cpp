#include "Arduino.h"
#include "AuthHOTP.h"
#include "src/Cryptosuite-master/sha1.h"

AuthHOTP::AuthHOTP(char* secret, size_t secretSize) {
	_secret = secret;
	_secretSize = secretSize;
}

void AuthHOTP::printHash(uint8_t* hash) {
  int i;
  Serial.print("SHA1-HMAC: ");
  for (i=0; i<20; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}

char* AuthHOTP::authServer(char* msg) {
	char* backmsg;
	switch(msg) {
		case "check":
			//TODO cooldown(throttle)
			char buffer[11];
			itoa(otp, buffer, 10);
			uint8_t digits = strlen(buffer);
			if(digits > 5 && digits < 9) {
				if(otp == calcOTP(digits)) {
				#ifdef DEBUG
				Serial.println("Auth: C: " + String((uint32_t)_counter) + " Success");
				#endif
				_counter++;
				} else {
					//TODO Resync
				}
			}
			#ifdef DEBUG
			Serial.println("Auth: C: " + String((uint32_t)_counter) + " Failure");
			#endif
			break;
		default:
			break;
	}
	return backmsg;
}

char* AuthHOTP::authClient(char* msg) {
	char* backmsg;
	switch(msg) {
		case "resync":
			uint32_t otps[s];
			for(int i = 0; i < s; i++) {
			otps[i] = calcOTP(6); //TODO digits
			}
			break;
		default:
			break;
	}
	return backmsg;
}

uint32_t AuthHOTP::calcOTP(uint8_t digits) {
  
  uint64_t divider;
  if(digits == 6) {divider = 1000000;}         //10^6
  else if(digits == 7) {divider = 10000000;}   //10^7
  else if(digits == 8) {divider = 100000000;}  //10^8
  else {return 0;}

  //SHA1 HMAC Hash
  uint8_t *hash;
  Sha1.initHmac(_secret,_secretSize); // key, and length of key in bytes
  for(int i=7; i>=0; i--) {
    Sha1.write(_counter >> (i * 8));
  }
  hash = Sha1.resultHmac();
  
  #ifdef DEBUG
  printHash(hash);
  #endif
  
  uint8_t offset   =  hash[19] & 0xf;
  uint32_t otp = 0;
  for(int i = 0; i < 4; i++) { //Byte Offset bis Offset+3 zusammensetzen
    otp = (otp << 8) | hash[offset + i];
  }
  
  otp = otp & 0x7FFFFFFF; //Vorzeichen Byte maskieren, aufgrund unsigned vs signed modulo berechnung auf verschiedenen prozessoren.
  otp = otp % divider;

  #ifdef DEBUG
  Serial.print("OTP: " + String(otp) + "      C: " + String((uint32_t)_counter) + "      K: ");
  for(int i = 0; i < _secretSize; i++) {
    Serial.print((char)_secret[i]);
  }
  Serial.println();
  #endif
  
  return otp;
}