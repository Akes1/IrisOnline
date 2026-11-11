#pragma once

#include <windows.h>
#include <wininet.h>

// #include "resource.h"

#define LPR_CONTINUE   0
#define LPR_STOP       1
#define LPR_ERROR      2
#define LPR_COMPLETE   LPR_CONTINUE

typedef DWORD (WINAPI *LOADER_PROGRESS_ROUTINE)(DWORD TotalFileSize, DWORD TotalBytesTransferred, void* data);

#define LIBVERSION			101
#define LIBNUMBERCLASSES	1

class ClassDesc
{
public:
	virtual DWORD    Open  ( char* url , unsigned int port = 0 )=0;
	virtual void     Close ( )=0;
	virtual DWORD    Get   ( char* url, LOADER_PROGRESS_ROUTINE routine, DWORD* exitCode, void* data, char** buffer, DWORD* bufferLen )=0;

public:
	virtual ~ClassDesc( ) {}
};

class Loader : public ClassDesc
{
private:
	HINTERNET m_internet;
	HINTERNET m_session;
	char      m_url[ 1024 ];
	int       m_port;

public:
	Loader(void);

	DWORD     Open  ( char* url , unsigned int port = 0 );
	void      Close ( );
	DWORD     Get   ( char* url, LOADER_PROGRESS_ROUTINE routine, DWORD* exitCode, void* data, char** buffer, DWORD* bufferLen );

public:
	virtual ~Loader(void);
};