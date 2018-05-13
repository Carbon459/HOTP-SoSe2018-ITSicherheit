#ifndef AuthHOTP_h
#define AuthHOTP_h

#include "Arduino.h"

#define DEBUG

class AuthHOTP
{
	public:
		AuthHOTP(char* secret, size_t secretSize);
		uint32_t calcOTP(uint8_t digits);
	private:
		void printHash(uint8_t* hash);
		uint64_t _counter = 0;
		char* _secret;
		size_t _secretSize;
		uint8_t _throttle = 0;
		uint8_t _resync = 0;
};

#endif