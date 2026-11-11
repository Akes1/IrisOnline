#pragma once

#include "../Common/Packet.h"

const unsigned long MAX_RECV_BUFFER_SIZE = 0x1000;
const unsigned long MAX_SEND_BUFFER_SIZE = 0x1000;


extern bool WriteLog(const char*  msg);

class cAsyncSelect
{
public:
	cAsyncSelect( );
	~cAsyncSelect(void);

	bool Init();
	bool CreateSocket( HWND hWnd );
	void CloseSocket();

	bool Connect( HWND hWnd );
	bool Connect( HWND hwnd, const char* ip, int port );
	bool ConnectComplete( HWND hwnd );
	int	 Recv( char* recvMsg, unsigned long length );
	bool Send( char* sendMsg, unsigned long length );


	SOCKET			GetSocket()		{	return mSocket;		}
	const char*		GetIp()			{	return mIP;			}
	const int		GetPort()		{	return mPort;		}

private:

	SOCKET			mSocket;
	SOCKADDR_IN     mAddr;

	char			mIP[50];
	int				mPort;

public:

	bool			mIsConnect;
	bool			mIsConnectResult;

	u_long			mSeq;
};
