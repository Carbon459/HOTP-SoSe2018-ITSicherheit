#ifndef AuthHOTP_h
#define AuthHOTP_h

#include "Arduino.h"

#define DEBUG

class AuthHOTP
{
	public:
		AuthHOTP(char* secret, size_t secretSize, uint8_t digits);
		
		/**
		* 	Calculates the OTP for the current secret and counter
		*	@returns	A OneTimePassword
		**/
		uint32_t calcOTP();
		
		/**
		*	Process a Protocol Message recieved from the Client
		*	@returns	The Protocol Message to be forwarded to the Client
		**/
		String authServer(String msg);
		/**
		*	Process a Protocol Message recieved from the Server
		*	@returns	The Protocol Message to be forwarded to the Server
		**/
		String authClient(String msg);
		/**
		*	Set the Counter, which is used by the HOTP algorithm manually
		**/
		void setCounter(int c);
		/**
		*	Check if the Client has the same secret as you
		*	This function doesnt have a use if this is used as a client only.
		**/
		bool isAuthenticated();
		/**
		*	Reset the flag, which is retrievable with isAuthenticated()
		*	This function doesnt have a use if this is used as a client only.
		**/
		void resetAuthSuccess();
		
	private:
		String padOTP(uint32_t otp);
		void printHash(uint8_t* hash);
		

		uint64_t _counter = 0;
		char* _secret;
		size_t _secretSize;		//Size of the secret in bytes
		uint8_t _digits = 6;	//Number of digits the OTP should have. (Valid values: 6,7,8)
		
		/*throttling parameter: the server will refuse connections
		* from a user after T unsuccessful authentication attempts.*/
		const int throttle = 3;
		
		//Currently remaining tries 
		int8_t _remTries = throttle;
		
		/*resynchronization parameter: the server will attempt to
        * verify a received authenticator across s consecutive
        * counter values*/
		uint8_t _resync = 2;
		
		bool _authSuccess = false;
};

#endif