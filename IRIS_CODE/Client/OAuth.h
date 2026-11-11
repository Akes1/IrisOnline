#pragma once


// OAuth Define
const cString			TWIT_CONSUMERKEY		= "3tV5mGcNHpFPwry071bw";	
const cString			TWIT_CONSUMERSECRET		= "0TcSAfHXxzmQCuC213mO3DvTeMCB72b0ya2ay740Y";
const cString			TWIT_TOKEN				= "";	
const cString			TWIT_TOKENSECRET		= "";

const cString			TWIT_NONCE_TEXT			= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.~";


const cString			TWITCURL_OAUTH_CONSUMERKEY		= "oauth_consumer_key=";
const cString			TWITCURL_OAUTH_TOKEN			= "oauth_token=";
const cString			TWITCURL_OAUTH_SIGNATUREMETHOD	= "oauth_signature_method=";
const cString			TWITCURL_OAUTH_SIGNATURE		= "oauth_signature=";
const cString			TWITCURL_OAUTH_TIMESTAMP		= "oauth_timestamp=";
const cString			TWITCURL_OAUTH_NONCE			= "oauth_nonce=";
const cString			TWITCURL_OAUTH_VERSION			= "oauth_version=";

struct sQueryParameter
{
	cString	mName;
	cString	mValue;
};


class cOAuth
{

public:

	// Http 전송 타입
	enum
	{
		ePOST,
		eGET,
	};

	// 암호화 타입
	enum
	{
		eHMAC_SHA1,
		eRSA_SHA1,
		ePLAINTEXT,
	};

	cOAuth();
	virtual ~cOAuth();

	

protected:
	
	char			B64_Encode(unsigned char u); 		
	unsigned char	B64_Decode(char c);
	int				B64_Is_Base64(char c);
	char*			Encode_Base64(int size, const unsigned char *src);
	int				Decode_Base64(unsigned char *dest, const char *src);

	int				CompareParameter(const void *p1, const void *p2);

	void			GetNonce( cString& outNonce );
	void			GetTimeStamp( cString& outTimeStamp );
	void			GetSignature(	unsigned char method, cString& cUrl, cString& cKey, cString& cSecret, cString& token, 
									cString& tSecret, cString& timeStatemp, cString& nonce, cString& siganatureType );

	bool			NormalizeReqParameters( tArray<void*>& parameterArray, cString& reqPrameters );


public:
	
	bool			GetRequestToken( cString& tURL, cString& cKey, cString& cSecret, cString& outReply );			
	void			AuthWebRequest( unsigned char httpType, cString& url, cString& postData );
	

};