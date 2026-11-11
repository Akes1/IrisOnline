#pragma once

#include "rc5.h"

#define MAX_BUFFER	0x10000
#define FD_CONNECTED	(1<<11)
#define FD_DISCONNECT	(1<<12)

struct Buffer
{
	unsigned long offset;
	unsigned long length;
	char*         data;
};

class cDummyClient;

class cAsyncSelect
{
private:
	HWND    m_wnd;
	DWORD   m_msg;
	SOCKET  m_socket;
	HANDLE  m_connectThread;
	bool    m_connected;

	u_long  m_addr;
	u_short m_port;

	bool    m_rc5;
	DWORD   m_seq;
	long    m_loginIdx;
	long    m_memberIdx;

	Buffer  m_backBuffer;
	Buffer  m_frontBuffer;
	Buffer  m_sendBuffer;

	cDummyClient*	mDummyClient;

	DWORD mLastTickCount;

private:
	void    Decrypt         ( BYTE* ptr, RC5_32_KEY key );
	void    Encrypt         ( BYTE* ptr, RC5_32_KEY key );

	void    Decrypt         ( char* ptr, BYTE key );
	void    Encrypt         ( char* ptr, BYTE key );

	void    FdReceive       ( SOCKET socket );
	void    FdWrite         ( SOCKET socket );
	void    FdClose         ( SOCKET socket );

	void    FdConnected     ( SOCKET socket );
	void    FdDisconnect    ( SOCKET socket );

public:
	bool    NmUserComeInGameReq ( );

	bool    NmUserGameSrvReq       ( );
	bool    NmUserCharacterListReq ( );

public:
	cAsyncSelect(HWND wnd, unsigned int msg, cDummyClient* client);

	HRESULT NetWorkProc         ( WPARAM wParam, LPARAM lParam );

	bool    IsConnect       ( ) { return m_connected; }
	SOCKET  GetSocket       ( ) { return m_socket; }

	bool    Connect         ( char* addr, unsigned short port );
	bool    Connect         ( unsigned long ipv4, unsigned short port );
	void    Disconnect      ( );

	bool    SendHeartbeat   ( );
	bool    SendNetworkMsg  ( char* ptr, int len );
	bool    Batcomplete     ( char* ptr, int len );

	void    Process         ( );

	DWORD   ConnectThread   ( void* ptr );
	void    ConnectComplete ( bool result );

public:
	virtual ~cAsyncSelect(void);

public:
	static DWORD WINAPI ConnectThreadStartingPoint( void* ptr );
};
