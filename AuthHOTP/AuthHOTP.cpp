/*
 * Version 1.0
 * Author: Friedrich Hudinjan
 */

#include "Arduino.h"
#include "AuthHOTP.h"
#include "src/Cryptosuite-master/sha1.h"

AuthHOTP::AuthHOTP(char* secret, size_t secretSize, uint8_t digits) {
	_secret = secret;
	_secretSize = secretSize;
	_digits = digits;
}

void AuthHOTP::setCounter(int c) {
	_counter = c;
}
		
bool AuthHOTP::isAuthenticated() {
	return _authSuccess;
}
void AuthHOTP::resetAuthSuccess() {
	_authSuccess = false;
}
void AuthHOTP::setResyncParam(uint8_t s) {
	_resync = s;
}
void AuthHOTP::setThrottleParam(uint8_t t) {
	_remTries = t;
}

String AuthHOTP::authServer(String msg) {
	String cmd = msg.substring(0,msg.indexOf(' '));
	String param1 = msg.substring(msg.indexOf(' ') + 1);
	if(cmd == String("authRequest")) {
		uint8_t digits = param1.length();
		if(digits > 8 || digits < 6) {
			#ifdef DEBUG
			Serial.println("Server: Auth: C: " + String((uint32_t)_counter) + " Failure: Invalidparam " + param1);
			#endif
			return "authFailure invalidparam";
		}
		uint32_t otp = param1.toInt();
		
		if(_remTries <= 0) {
			#ifdef DEBUG
			Serial.println("Server: Auth: C: " + String((uint32_t)_counter) + " Failure: No more retries available");
			#endif
			return "authFailure throttling";
		}
		
		if(_counter == 0) {_counter++;} //Falls erste Authentifikation auf Serverseite: counter auf 1 setzen um in Sync zu bleiben mit client
		
		if(_digits == digits) {
			if(otp == calcOTP()) {
			#ifdef DEBUG
			Serial.println("Server: Auth: C: " + String((uint32_t)_counter) + " Success");
			#endif
			_counter++;
			_authSuccess = true;
			return "authSuccess " + padOTP(otp);
			} else { //Resync probieren
				uint64_t counterSave = _counter; //Counter wert sichern falls resync nicht erfolgreich
				for(int i = 1; i <= _resync; i++) {
					#ifdef DEBUG
					Serial.println("Server: Auth: C: " + String((uint32_t)_counter) + " Failure: Trying resync...");
					#endif
					_counter++;
					if(calcOTP() == otp) {
						#ifdef DEBUG
						Serial.println("Server: Auth: C: " + String((uint32_t)_counter) + " Success after " + String(i+1) + " resync attempt(s)");
						#endif
						_authSuccess = true;
						return "authSuccess " + padOTP(otp);
					}
				}
				#ifdef DEBUG
				Serial.println("Server: Auth: C: " + String((uint32_t)_counter) + " Failure: Resync not successfull");
				#endif
				_counter = counterSave; //Counterwert wiederherstellen
			}
		}
		_remTries--;
		#ifdef DEBUG
		Serial.println("Server: Auth: C: " + String((uint32_t)_counter) + " Failure: Remaining Tries: " + String(_remTries));
		#endif
		return "authFailure " + padOTP(otp);
	}
}

String AuthHOTP::authClient(String msg) {
	String cmd = msg.substring(0,msg.indexOf(' '));
	String param1 = msg.substring(msg.indexOf(' ') + 1);
	if(cmd == String("authRequest")) {
		_counter++;
		return "authRequest " + padOTP(calcOTP());
	} else if(cmd == String("authSuccess")) {
		#ifdef DEBUG
		Serial.println("Client: Success: " + param1);
		#endif
		_authSuccess = true;
		return "";
	} else if(cmd == String("authFailure")) {
		#ifdef DEBUG
		Serial.println("Client: Failure: " + param1);
		#endif
		return "";
	}
}

uint32_t AuthHOTP::calcOTP() {
  
  uint64_t divider;
  if(_digits == 6) {divider = 1000000;}         //10^6
  else if(_digits == 7) {divider = 10000000;}   //10^7
  else if(_digits == 8) {divider = 100000000;}  //10^8
  else {return 0;}

  //SHA1 HMAC Hash
  uint8_t *hash;
  Sha1.initHmac(_secret,_secretSize); // key, and length of key in bytes
  for(int i=7; i>=0; i--) {
    Sha1.write(_counter >> (i * 8));
  }
  hash = Sha1.resultHmac();
  
  /*#ifdef DEBUG
  printHash(hash);
  #endif*/
  
  uint8_t offset   =  hash[19] & 0xf;
  uint32_t otp = 0;
  for(int i = 0; i < 4; i++) { //Byte Offset bis Offset+3 zusammensetzen
    otp = (otp << 8) | hash[offset + i];
  }
  
  otp = otp & 0x7FFFFFFF; //Vorzeichen Bit maskieren, aufgrund unsigned vs signed modulo berechnung auf verschiedenen prozessoren.
  
  otp = otp % divider;

  /*#ifdef DEBUG
  Serial.print("OTP: " + padOTP(otp) + "      C: " + String((uint32_t)_counter) + "      K: ");
  for(int i = 0; i < _secretSize; i++) {
    Serial.print((char)_secret[i]);
  }
  Serial.println();
  #endif*/
  return otp;
}

String AuthHOTP::padOTP(uint32_t otp) {
	if(String(otp).length() == _digits) return String(otp);
	else {
		int toPad = _digits - String(otp).length();
		String append = "";
		for(int i = 0; i < toPad; i++) {
			append.concat("0");
		}
		return append+String(otp);
	}
}

//Funktion aus Beispiel der SHA1 Library übernommen
void AuthHOTP::printHash(uint8_t* hash) {
  int i;
  Serial.print("SHA1-HMAC: ");
  for (i=0; i<20; i++) {
    Serial.print("0123456789abcdef"[hash[i]>>4]);
    Serial.print("0123456789abcdef"[hash[i]&0xf]);
  }
  Serial.println();
}
