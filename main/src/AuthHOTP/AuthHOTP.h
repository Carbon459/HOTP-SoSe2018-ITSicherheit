#ifndef AuthHOTP_h
#define AuthHOTP_h

#include "Arduino.h"

#define DEBUG


#define THROTTLE 3

class AuthHOTP
{
	public:
		AuthHOTP(char* secret, size_t secretSize, uint8_t digits);
		
		/*	Calculates the OTP for the current secret and counter
		*	@return	A OneTimePassword
		*/
		uint32_t calcOTP();
		

		String authServer(String msg);
		String authClient(String msg);
		void setCounter(int c);
		
	private:
		String padOTP(uint32_t otp);
		void printHash(uint8_t* hash);
		

		uint64_t _counter = 0;
		char* _secret;
		size_t _secretSize;		//Size of the secret in bytes
		uint8_t _digits = 6;	//Number of digits the OTP should have. (Valid values: 6,7,8)
		
		/*throttling parameter: the server will refuse connections
		* from a user after T unsuccessful authentication attempts.
		*/
		const int throttle = 3;
		int8_t _remTries = throttle;
		
		/*resynchronization parameter: the server will attempt to
        * verify a received authenticator across s consecutive
        * counter values
		*/
		uint8_t _resync = 2;
};

#endif