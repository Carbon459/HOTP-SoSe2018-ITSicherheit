#ifndef AuthHOTP_h
#define AuthHOTP_h

#include "Arduino.h"

#define DEBUG

class AuthHOTP
{
	public:
		AuthHOTP(char* secret, size_t secretSize);
		
		/*	Calculates the OTP for the current secret and counter
		*	@digits	Number of digits the OTP should have. (Valid values: 6,7,8)
		*	@return	A OneTimePassword
		*/
		uint32_t calcOTP(uint8_t digits);
		
		/*	
		*	
		*/
		char* authServer(char* msg);
		
		/*	
		*
		*/
		char* authClient(char* msg);
	private:
		void printHash(uint8_t* hash);
		
		uint64_t _counter = 0;
		char* _secret;
		size_t _secretSize;		//Size of the secret in bytes
		
		/*throttling parameter: the server will refuse connections
        * from a user after T unsuccessful authentication attempts.
		*/
		uint8_t _throttle = 0;
		
		/*resynchronization parameter: the server will attempt to
        * verify a received authenticator across s consecutive
        * counter values
		*/
		uint8_t _resync = 3;
};

#endif