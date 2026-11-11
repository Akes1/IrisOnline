#include "stdafx.h"
#include "OAuth.h"
#include "hmac.h"	// 암호화
#include "sha1.h"	// 암호화

bool ParameterSort( void *arg0, void* arg1 ) 
{ 
	sQueryParameter* param0 = (sQueryParameter*)arg0;
	sQueryParameter* param1 = (sQueryParameter*)arg1;

	if( param0->mName == param1->mName )
	{
		if( param0->mValue.Compare( param1->mValue ) )
			return true;

		return false;
	}
	
	if( param0->mName.Compare( param1->mName ) )
		return true;

	return false;
} 


cOAuth::cOAuth()
{

}

cOAuth::~cOAuth()
{

}


char	cOAuth::B64_Encode(unsigned char u)
{
	if(u < 26)  return 'A'+u;
	if(u < 52)  return 'a'+(u-26);
	if(u < 62)  return '0'+(u-52);
	if(u == 62) return '+';
	return '/';
}

unsigned char	cOAuth::B64_Decode(char c)
{
	if(c >= 'A' && c <= 'Z') return(c - 'A');
	if(c >= 'a' && c <= 'z') return(c - 'a' + 26);
	if(c >= '0' && c <= '9') return(c - '0' + 52);
	if(c == '+')             return 62;
	return 63;
}

int		cOAuth::B64_Is_Base64(char c)
{
	if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
		(c >= '0' && c <= '9') || (c == '+')             ||
		(c == '/')             || (c == '=')) {
			return 1;
	}
	return 0;
}

char*	cOAuth::Encode_Base64(int size, const unsigned char *src)
{
	int i;
	char *out, *p;

	if(!src) return NULL;
	if(!size) size= strlen((char *)src);
	out = (char*) new char[ size*4/3+4 ];
	p = out;

	for(i=0; i<size; i+=3) {
		unsigned char b1=0, b2=0, b3=0, b4=0, b5=0, b6=0, b7=0;
		b1= src[i];
		if(i+1<size) b2= src[i+1];
		if(i+2<size) b3= src[i+2];

		b4= b1>>2;
		b5= ((b1&0x3)<<4)|(b2>>4);
		b6= ((b2&0xf)<<2)|(b3>>6);
		b7= b3&0x3f;

		*p++= B64_Encode(b4);
		*p++= B64_Encode(b5);

		if(i+1<size) *p++= B64_Encode(b6);
		else *p++= '=';

		if(i+2<size) *p++= B64_Encode(b7);
		else *p++= '=';
	}
	return out;
}

int		cOAuth::Decode_Base64(unsigned char *dest, const char *src)
{
	if(src && *src) {
		unsigned char *p= dest;
		int k, l= strlen(src)+1;
		unsigned char *buf= new unsigned char[ l ];

		/* Ignore non base64 chars as per the POSIX standard */
		for(k=0, l=0; src[k]; k++) {
			if(B64_Is_Base64(src[k])) {
				buf[l++]= src[k];
			}
		} 

		for(k=0; k<l; k+=4) {
			char c1='A', c2='A', c3='A', c4='A';
			unsigned char b1=0, b2=0, b3=0, b4=0;
			c1= buf[k];

			if(k+1<l) c2= buf[k+1];
			if(k+2<l) c3= buf[k+2];
			if(k+3<l) c4= buf[k+3];

			b1= B64_Decode(c1);
			b2= B64_Decode(c2);
			b3= B64_Decode(c3);
			b4= B64_Decode(c4);

			*p++=((b1<<2)|(b2>>4) );

			if(c3 != '=') *p++=(((b2&0xf)<<4)|(b3>>2) );
			if(c4 != '=') *p++=(((b3&0x3)<<6)|b4 );
		}
		delete[]buf;
		dest[p-dest]='\0';
		return(p-dest);
	}
	return 0;
}

void cOAuth::GetNonce( cString& outNonce )
{
	static int rndinit = 1;
	
	cString	chars =	TWIT_NONCE_TEXT;

	unsigned int max = chars.GetLength();
	
	if( rndinit ) 
	{
		srand( (unsigned int)time(NULL) ); 
		rndinit=0;
	} 

	int len = 15 + (int)floor( rand() * 16.0 / (double)RAND_MAX );

	outNonce.Clear();
	outNonce.Reserve( len + 1 );

	for(int i = 0; i < len; i++) 
	{
		outNonce[i] = chars[ rand() % max ];
	}

	outNonce[len] = 0;
}

void cOAuth::GetTimeStamp( cString& outTimeStamp )
{
	outTimeStamp.Format( "%d", (unsigned long) time(NULL) );
}

void cOAuth::GetSignature(	unsigned char method, cString& cUrl, cString& cKey, cString& cSecret, cString& token, 
							cString& tSecret, cString& timeStamp, cString& nonce, cString& siganatureType )
{

	if( cKey.GetLength()			<= 0 ||
		cSecret.GetLength()			<= 0 ||	
		siganatureType.GetLength()	<= 0 )
	{
		return;
	}

	//====================================
	//	Signature Base String 만들기
	//====================================
	typedef tArray<void*> cParameterArray;
	cParameterArray mParameterArray;
	
	// nonce
	sQueryParameter* parameter = new sQueryParameter;	
	parameter->mName	= TWITCURL_OAUTH_NONCE;
	parameter->mValue	= nonce;	
	mParameterArray.PushBack( parameter );

	// timestamp
	parameter = new sQueryParameter;	
	parameter->mName	= TWITCURL_OAUTH_TIMESTAMP;
	parameter->mValue	= timeStamp;	
	mParameterArray.PushBack( parameter );

	// token
	parameter = new sQueryParameter;
	parameter->mName	= TWITCURL_OAUTH_TOKEN;
	parameter->mValue	= token;	
	mParameterArray.PushBack( parameter );

	// consumerkey
	parameter = new sQueryParameter;
	parameter->mName	= TWITCURL_OAUTH_CONSUMERKEY;
	parameter->mValue	= cKey;
	mParameterArray.PushBack( parameter );

	// signature method
	parameter = new sQueryParameter;
	parameter->mName	= TWITCURL_OAUTH_SIGNATUREMETHOD;
	parameter->mValue	= method == eHMAC_SHA1 ? "HMAC-SHA1" : method == eRSA_SHA1 ? "RSA-SHA1" : "PLAINTEXT";
	mParameterArray.PushBack( parameter );

	// version
	parameter = new sQueryParameter;
	parameter->mName	= TWITCURL_OAUTH_VERSION;
	parameter->mValue	= "1.0";
	mParameterArray.PushBack( parameter );
	
	// 파라미터 소팅
	::Sort( mParameterArray.Begin(), mParameterArray.End(), ParameterSort );

	cString normalizeReqParameters;
	NormalizeReqParameters( mParameterArray, normalizeReqParameters );

	if( normalizeReqParameters.GetLength() <= 0 )
	{
		return; 
	}


	/*
	//====================================
	//	Secret 키 만들기
	//====================================
	cString sigKey;
	sigKey.Append( cSecret );
	sigKey.Append( "&" );
	sigKey.Append( tSecret );

	cString sigBaseString;
	sigBaseString.Append( method == ePOST ? "POST" : "GET" );
	sigBaseString.Append( "&" );
	sigBaseString.Append( cUrl );
	sigBaseString.Append( "&" );
	sigBaseString.Append( normalizeReqParameters );
	*/

}

bool cOAuth::NormalizeReqParameters( tArray<void*>& parameterArray, cString& reqPrameters )
{
	unsigned int nSize = parameterArray.GetSize();
	for( unsigned int i = 0; i < nSize; i++ )
	{
		sQueryParameter* pPrameter = (sQueryParameter*)parameterArray[ i ];
		if( pPrameter == NULL )
		{
			assert(0);
			return false;
		}
		
		pPrameter->mName.Append("=");
		pPrameter->mName.Append( pPrameter->mValue );

		reqPrameters.Append( pPrameter->mName );

		if( i < nSize - 1 )
		{
			reqPrameters.Append("&");
		}

		SAFE_DELETE(pPrameter);
	}

	parameterArray.Clear();

	return true;	
}

bool cOAuth::GetRequestToken( cString& tURL, cString& cKey, cString& cSecret, cString& outReply )
{
	cString tokenURL;

	tokenURL += tURL;
	tokenURL.Append( "?" );

	return true;
}

void cOAuth::AuthWebRequest( unsigned char httpType, cString& url, cString& postData )
{
	cString outUrl = "";
	cString querystring = "";
	cString ret = "";
/*
	if (httpType == ePOST )
	{
		if (postData.GetLength() > 0)
		{
			//Decode the parameters and re-encode using the oAuth UrlEncode method.
			NameValueCollection qs = HttpUtility.ParseQueryString(postData);
			postData = "";
			foreach (string key in qs.AllKeys)
			{
				if (postData.Length > 0)
				{
					postData += "&";
				}
				qs[key] = HttpUtility.UrlDecode(qs[key]);
				qs[key] = this.UrlEncode(qs[key]);
				postData += key + "=" + qs[key];

			}
			if (url.IndexOf("?") > 0)
			{
				url += "&";
			}
			else
			{
				url += "?";
			}
			url += postData;
		}
	}

	Uri uri = new Uri(url);

	string nonce = this.GenerateNonce();
	string timeStamp = this.GenerateTimeStamp();

	//Generate Signature
	string sig = this.GenerateSignature(uri,
		this.ConsumerKey,
		this.ConsumerSecret,
		this.Token,
		this.TokenSecret,
		method.ToString(),
		timeStamp,
		nonce,
		out outUrl,
		out querystring);

	querystring += "&oauth_signature=" + HttpUtility.UrlEncode(sig);

	//Convert the querystring to postData
	if (method == Method.POST)
	{
		postData = querystring;
		querystring = "";
	}

	if (querystring.Length > 0)
	{
		outUrl += "?";
	}

	ret = WebRequest(method, outUrl +  querystring, postData);

	return ret;
	*/

}

int cOAuth::CompareParameter(const void *p1, const void *p2)
{/*
	char *v1,*v2;
	char *t1,*t2;
	int rv;
	
	// array elements (once) before sorting.
	v1=oauth_url_escape(* (char * const *)p1);
	v2=oauth_url_escape(* (char * const *)p2);

	// '=' signs are not "%3D" !
	if ((t1=strstr(v1,"%3D"))) {
		t1[0]='\0'; t1[1]='='; t1[2]='=';
	}
	if ((t2=strstr(v2,"%3D"))) {
		t2[0]='\0'; t2[1]='='; t2[2]='=';
	}

	// compare parameter names
	rv=strcmp(v1,v2);
	if (rv!=0) {
		if (v1) free(v1);
		if (v2) free(v2);
		return rv;
	}

	// if parameter names are equal, sort by value.
	if (t1) t1[0]='='; 
	if (t2) t2[0]='='; 
	rv=strcmp(t1,t2);
	if (v1) free(v1);
	if (v2) free(v2);
	return rv;*/
	return 0;
}
