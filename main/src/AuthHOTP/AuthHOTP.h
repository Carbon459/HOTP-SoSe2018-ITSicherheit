#ifndef AuthHOTP_h
#define AuthHOTP_h

#include "Arduino.h"

#define DEBUG

/*throttling parameter: the server will refuse connections
* from a user after T unsuccessful authentication attempts.
*/
#define THROTTLE 3

class AuthHOTP
{
	public:
		AuthHOTP(char* secret, size_t secretSize, uint8_t digits);
		
		/*	Calculates the OTP for the current secret and counter
		*	@return	A OneTimePassword
		*/
		uint32_t calcOTP();
		
		/*	
		*	Implements the protocol on the server side
		*	@msg A protocol message received from the client
		*	@return A protocol message to send to the client
		*/
		String authServer(String msg);
		
		/*	
		*	Implements the protocol on the client side
		*	@msg A protocol message received from the server
		*	@return A protocol message to send to the server
		*/
		String authClient(String msg);
	private:
		void printHash(uint8_t* hash);
		
		uint64_t _counter = 0;
		char* _secret;
		size_t _secretSize;		//Size of the secret in bytes
		uint8_t _digits = 6;	//Number of digits the OTP should have. (Valid values: 6,7,8)
	
		int8_t _remTries = THROTTLE;
		
		/*resynchronization parameter: the server will attempt to
        * verify a received authenticator across s consecutive
        * counter values
		*/
		uint8_t _resync = 2;
};

#endif