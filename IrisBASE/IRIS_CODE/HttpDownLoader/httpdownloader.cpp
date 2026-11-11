/*-- Include
*/
#include "httpdownloader.h"

/*-- Global data
*/
extern HINSTANCE g_instance;
Loader           g_classDesc;

/*-- Prototype
*/
ClassDesc* GetClassDesc( ) { return &g_classDesc; }

/*-- Loader Constructor
*/
Loader::Loader(void)
: m_internet( NULL ),
m_session( NULL ),
m_port( 80 )
{
}

/*-- Loader Destructor
*/
Loader::~Loader(void)
{
	Close( );
}

/*-- Open Method - 인터넷 연결 - HTTP 방식
*/
DWORD Loader::Open(char* url, unsigned int port)
{
	Close( );

	try {
		m_internet = InternetOpen( NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );
		if ( m_internet == NULL )
			throw GetLastError( );
		if( port != 0)
			m_session = InternetConnect( m_internet, url, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
		else
			m_session = InternetConnect( m_internet, url, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );

		if ( m_session == NULL )
			throw GetLastError( );

		strcpy_s( m_url, url );
		m_port = (port == 0) ? 80 : port ;
		return 0;
	} catch ( DWORD error ) {
		Close( );
		return error;
	}
}

/*-- Close Method
*/
void Loader::Close( )
{
	if ( m_session != NULL )
	{
		InternetCloseHandle( m_session );
		m_session = NULL;
	}
	if ( m_internet != NULL )
	{
		InternetCloseHandle( m_internet );
		m_internet = NULL;
	}
}

/*-- Get Method
*/
DWORD Loader::Get(char* url, LOADER_PROGRESS_ROUTINE routine, DWORD* exitCode, void* data, char** buffer, DWORD* bufferLen)
{
	HINTERNET request = NULL;
	DWORD     retcode = 0;

	try {
		// HTTP 형식의 Request 핸들 생성 - URL 참조.
		// 서버에서 보낼 준비가 된 크기.`
		if( (*exitCode) == LPR_CONTINUE )
		{
			request = HttpOpenRequest( m_session
									  ,"GET"
									  ,url
									  ,HTTP_VERSION
									  ,NULL
									  ,NULL
									  ,INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_AUTO_REDIRECT
									  ,0 );
			if ( request == NULL )
				throw GetLastError( );
		}
		else
			throw ERROR_INTERNET_HANDLE_EXISTS;

		// HTTP 서버에 요청 보내기.
		if( (*exitCode) == LPR_CONTINUE )
		{
			DWORD timeout = (DWORD)-1;
			InternetSetOption( request, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout) );

			timeout = 10000;
			InternetSetOption( request, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout) );
			InternetSetOption( request, INTERNET_OPTION_SEND_TIMEOUT,    &timeout, sizeof(timeout) );

			if ( HttpSendRequest( request, NULL, 0, NULL, 0 ) == FALSE )
				throw GetLastError( );
		}
		else
			throw ERROR_INTERNET_HANDLE_EXISTS;

		char  query[ 32 ];
		memset( query, 0, sizeof(query) );
		DWORD cbLengthQuery = sizeof(query);

		// HTTP 서버의 컨텐츠 크기 받기.
		if( (*exitCode) == LPR_CONTINUE )
		{
			if ( HttpQueryInfo( request, HTTP_QUERY_CONTENT_LENGTH, query, &cbLengthQuery, NULL ) == FALSE )
				throw GetLastError( );
		}
		else
			throw ERROR_INTERNET_HANDLE_EXISTS;

		// 메모리 할당 및 변수 초기화.
		DWORD offset = 0;

		(*bufferLen) = (DWORD)atol( query );
		(*buffer)    = (char*)GlobalAlloc( GPTR, (*bufferLen)+1 );

		if ( (*buffer) == NULL )
			throw GetLastError( );

		DWORD numOfByteAvaiable   = 0;
		DWORD cbNumOfByteAvaiable = 0;
		char* ptr                 = (*buffer);

		//(*exitCode) = LPR_CONTINUE;
		do {
			// 서버에서 보낼 준비가 된 크기.`
			if( (*exitCode) == LPR_CONTINUE )
			{
				if ( InternetQueryDataAvailable( request, &numOfByteAvaiable, 0, 0 ) == FALSE )
					throw GetLastError( );
			}
			else
			{
				retcode = ERROR_INTERNET_HANDLE_EXISTS;
				break;
			}

			// 서버에서 데이터 받기.
			if( (*exitCode) == LPR_CONTINUE )
			{
				if ( InternetReadFile( request, ptr, numOfByteAvaiable, &cbNumOfByteAvaiable ) == FALSE )
					throw GetLastError( );
			}
			else
			{
				retcode = ERROR_INTERNET_HANDLE_EXISTS;
				break;
			}

			// 서버에서 받은 데이터 누적.
			offset += cbNumOfByteAvaiable;
			ptr    += cbNumOfByteAvaiable;

			if ( routine != NULL && (*exitCode) == LPR_CONTINUE )
				(*exitCode) = (*routine)( (*bufferLen), offset, data );

		} while ( offset < (*bufferLen) );
	} catch ( DWORD error ) {
		retcode = error;
	}

	if( (*bufferLen) == 1466 )
	{
		char* defaultPage =  "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">";

		if( _strnicmp( *buffer, defaultPage, strlen( defaultPage ) ) == 0 )
		{	
			(*bufferLen) = 0;
			retcode = ERROR_INTERNET_INVALID_URL;
		}
	}
	else if( (*bufferLen) == 345 )
	{
		char* defaultPage =  "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN";
		
		if( _strnicmp( *buffer, defaultPage, strlen( defaultPage ) ) == 0 )
		{	
			(*bufferLen) = 0;
			retcode = ERROR_INTERNET_INVALID_URL;
		}
	}

	if ( request != NULL )
	{
		InternetCloseHandle( request );
		request = NULL;
	}

	return retcode;
}
