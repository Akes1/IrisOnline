// Include
#include "loginsrv.h"
#include "packet.h"

// Local definitions
#define SafeDelete(P)       if (P!=NULL) { delete(P);      (P)=NULL; } // memory를 안전하게 해제

#pragma warning( disable: 4127 )

// Global data

// cRudpProcess Constructor
cRudpProcess::cRudpProcess(void) : mPacketPool(NULL)
{
}

// ~cRudpProcess Destructor
cRudpProcess::~cRudpProcess(void)
{
	SafeDelete( mPacketPool );
}

// Initialize Method
bool cRudpProcess::Initialize(char* recv, char* send, unsigned short port, unsigned short numWorkerThreads)
{
	mPacketPool = new PacketPool( );

	PHOSTENT phe;

	// GameServer IPv4 주소를 만든다.
	ZeroMemory( (void*)&mGame, sizeof(SOCKADDR_IN) );
	mGame.sin_family = AF_INET;
	mGame.sin_port   = htons( U_GAME_SPORT );
	mGame.sin_addr.s_addr = inet_addr( send );

	if ( mGame.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( send )) != NULL )
			CopyMemory( &mGame.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}
	mGameSeq = 0;
	mGameBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );

	// LogDemon IPv4 주소를 만든다.
	ZeroMemory( (void*)&mLog, sizeof(SOCKADDR_IN) );
	mLog.sin_family = AF_INET;
	mLog.sin_port   = htons( U_LOG_SPORT );
	mLog.sin_addr.s_addr = inet_addr( send );

	if ( mLog.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( send )) != NULL )
			CopyMemory( &mLog.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}
	mLogSeq = 0;
	mLogBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );

	return IocpRudp::Initialize( recv, port, numWorkerThreads );
}

// Shutdown Method
void cRudpProcess::Shutdown(DWORD maxWait)
{
	Sleep( 50 );
	IocpRudp::Shutdown( maxWait );

	if ( mLogBuffer )
	{
		GlobalFree( mLogBuffer );
		mLogBuffer = NULL;
	}
	if ( mGameBuffer )
	{
		GlobalFree( mGameBuffer );
		mGameBuffer = NULL;
	}

	mPacketPool->Shutdown( );
}

// ReSendExec Method
bool cRudpProcess::ReSendExec(u_long addr, u_short port, u_long seq)
{
	cCSLock       lock( &m_cs );
	PerIoContext* ioContext = NULL;
	long          mask      = (seq & SM_ClassD);
	bool          retvalue  = false;

	if ( htons( U_GAME_SPORT ) == port )
	{
		ioContext = mGameBuffer->buffer[ mask ];
	}
	else if ( htons( U_LOG_SPORT ) == port )
	{
		ioContext = mLogBuffer->buffer[ mask ];
	}

	if ( ioContext != NULL )
	{
		RUDP* rudp = (RUDP*)ioContext->buffer;
		if ( rudp->seq == seq )
		{
			SOCKADDR_IN saddr;

			ZeroMemory( (void*)&saddr, sizeof(SOCKADDR_IN) );
			saddr.sin_family = AF_INET;
			saddr.sin_port   = port;
			saddr.sin_addr.s_addr = addr;

			PerIoContext* sendIoContext = m_ioContextPool->GetIoContext( m_send, IOCP_REQUEST_WRITE, saddr );

			memcpy( sendIoContext->buffer, ioContext->buffer, ioContext->offset );
			sendIoContext->offset = ioContext->offset;

			retvalue = SendExec( NULL, sendIoContext );
		}
	}

	return retvalue;
}

// SendReliable Method
bool cRudpProcess::SendReliable(u_long addr, u_short port, u_long seq)
{
	cCSLock       lock( &m_cs );
	SOCKADDR_IN   saddr;

	ZeroMemory( (void*)&saddr, sizeof(SOCKADDR_IN) );
	saddr.sin_family = AF_INET;
	saddr.sin_port   = port;
	saddr.sin_addr.s_addr = addr;

	PerIoContext* perIoContext = m_ioContextPool->GetIoContext( m_send, IOCP_REQUEST_WRITE, saddr );

	if ( perIoContext != NULL )
	{
		RUDP* rudp = (RUDP*)perIoContext->buffer;

		rudp->ver  = RUDP_PHVer;
		rudp->hlen = RUDP_PHLen;
		rudp->tos  = TOS_RUDP;
		rudp->tlen = rudp->hlen;
		rudp->seq  = seq;
		rudp->addr = m_recvAddr.sin_addr.s_addr;
		rudp->port = m_recvAddr.sin_port;

		perIoContext->offset = rudp->tlen;
		return SendExec( NULL, perIoContext );
	}
	return false;
}

// GetPacket Method
char* cRudpProcess::GetPacket(void** handle, char tos, SOCKADDR_IN addr, u_short& seq)
{
	cCSLock       lock( &m_cs );
	PerIoContext* perIoContext = m_ioContextPool->GetIoContext( m_send, IOCP_REQUEST_WRITE, addr );
	char*         retvalue     = NULL;

	if ( perIoContext != NULL )
	{
		RUDP* rudp = (RUDP*)perIoContext->buffer;

		rudp->ver  = RUDP_PHVer;
		rudp->hlen = RUDP_PHLen;
		rudp->tos  = tos;
		rudp->tlen = rudp->hlen;
		rudp->seq  = (++seq);
		rudp->addr = m_recvAddr.sin_addr.s_addr;
		rudp->port = m_recvAddr.sin_port;

		(*handle) = perIoContext;
		retvalue  = perIoContext->buffer + rudp->hlen;
	}

	return retvalue;
}

// SendPacket Method
bool cRudpProcess::SendPacket(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	RUDP*         rudp         = (RUDP*)perIoContext->buffer;

	rudp->tlen = rudp->hlen + (u_short)length;

	perIoContext->offset = rudp->tlen;
	return SendExec( NULL, perIoContext );
}

// GetRegisterDate Method
void cRudpProcess::GetRegisterDate(TIMESTAMP_STRUCT* ts)
{
	SYSTEMTIME st;
	GetLocalTime( &st );
	ts->year     = st.wYear;
	ts->month    = st.wMonth;
	ts->day      = st.wDay;
	ts->hour     = st.wHour;
	ts->minute   = st.wMinute;
	ts->second   = st.wSecond;
	ts->fraction = st.wMilliseconds * 1000000;
}

// PostServerEvent Method
bool cRudpProcess::PostServerEvent(LPCTSTR format, ...)
{
	bool retvalue = false;
	if ( g_loginSrv != NULL )
	{
		LPVOID  msgBuf = NULL;
		DWORD   bufferLength;

		va_list args;

		va_start( args, format );

		bufferLength = _vscprintf( format, args ) + 1;
		msgBuf       = malloc( bufferLength );

		vsprintf( (char*)msgBuf, format, args );

		va_end( args );

		if ( msgBuf != NULL )
		{
			retvalue = PostServerEvent( (char*)msgBuf );
			free( msgBuf );
		}
	}
	return retvalue;
}

// PostServerEvent Method
bool cRudpProcess::PostServerEvent(char* message)
{
	HANDLE        handle      = NULL;
	SERVER_EVENT* serverEvent = (SERVER_EVENT*)GetPacket( &handle, TOS_LOGIN_LOG, mLog, mLogSeq );
	if ( serverEvent != NULL )
	{
		char*  computer = g_loginSrv->GetHostName( );
		u_long length   = sizeof(SERVER_EVENT) - sizeof(serverEvent->message);

		serverEvent->protocol = MB_SERVER_EVENT;
		GetRegisterDate( &serverEvent->registerDate );
		strcpy( serverEvent->source, g_serviceName );
		strcpy( serverEvent->computer, computer );
		strcpy( serverEvent->message, message );
		
		length += strlen(serverEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostMemberEvent Method
bool cRudpProcess::PostMemberEvent(char type, char category, long memberIdx, char* message)
{
	HANDLE handle = NULL;
	MEMBER_EVENT* memberEvent = (MEMBER_EVENT*)GetPacket( &handle, TOS_LOGIN_LOG, mLog, mLogSeq );
	if ( memberEvent != NULL )
	{
		char*  computer = g_loginSrv->GetHostName( );
		u_long length   = sizeof(MEMBER_EVENT) - sizeof(memberEvent->message);

		memberEvent->protocol = MB_MEMBER_EVENT;
		GetRegisterDate( &memberEvent->registerDate );
		memberEvent->type = type;
		strcpy( memberEvent->source, g_serviceName );
		memberEvent->category = category;
		memberEvent->memberIdx = memberIdx;
		strcpy( memberEvent->computer, computer );
		strcpy( memberEvent->message, message );

		length += strlen( memberEvent->message );
		return SendPacket( handle, length );
	}
	return false;
}

// PostConcurrentEvent Method
bool cRudpProcess::PostConcurrentEvent(long serverIdx, long minimum, long maximum, char* message)
{
	HANDLE handle = NULL;
	CONCURRENT_EVENT* concurrentEvent = (CONCURRENT_EVENT*)GetPacket( &handle, TOS_LOGIN_LOG, mLog, mLogSeq );
	if ( concurrentEvent != NULL )
	{
		char*  computer = g_loginSrv->GetHostName( );
		u_long length   = sizeof(CONCURRENT_EVENT) - sizeof(concurrentEvent->message);

		concurrentEvent->protocol = MB_CONCURRENT_EVENT;
		GetRegisterDate( &concurrentEvent->registerDate );
		concurrentEvent->serverIdx = serverIdx;
		concurrentEvent->minimum = minimum;
		concurrentEvent->maximum = maximum;
		strcpy( concurrentEvent->computer, computer );
		strcpy( concurrentEvent->message, message );

		length += strlen( concurrentEvent->message );
		return SendPacket( handle, length );
	}
	return false;
}

// PostChSync Method
bool cRudpProcess::PostChSync( )
{
	HANDLE          handle    = NULL;
	MB_SYN_CH_SYNC* synChSync = (MB_SYN_CH_SYNC*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );
	if ( synChSync != NULL )
	{
		synChSync->protocol = MB_CH_SYNC_SYN;
		return SendPacket( handle, sizeof(MB_SYN_CH_SYNC) );
	}
	return false;
}

// PostChList Method
bool cRudpProcess::PostChList(ServerTable* root)
{
	HANDLE          handle    = NULL;
	MB_SYN_CH_LIST* synChList = (MB_SYN_CH_LIST*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );
	if ( synChList != NULL )
	{
		synChList->protocol = MB_CH_LIST_SYN;

		PerChannel* ch = root->pool;
		for ( BYTE i = 0; i < root->channelCounter; i++ )
		{
			synChList->channels[ i ].cid    = ch->cid;
			synChList->channels[ i ].status = ch->status;
			ch = ch->next;

			synChList->rowCount++;
		}
		return SendPacket( handle, synChList->Length( ) );
	}
	return false;
}

// PostChStatus Method
bool cRudpProcess::PostChStatus(long cid, BYTE status)
{
	HANDLE            handle      = NULL;
	MB_SYN_CH_STATUS* synChStatus = (MB_SYN_CH_STATUS*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );
	if ( synChStatus != NULL )
	{
		synChStatus->protocol = MB_CH_STATUS_SYN;
		synChStatus->cid      = cid;
		synChStatus->status   = status;
		return SendPacket( handle, sizeof(MB_SYN_CH_STATUS) );
	}
	return false;
}

// BatchComplete Method
bool cRudpProcess::BatchComplete(MSGBUF* msgBuf, int msgLen)
{
	switch ( msgBuf->protocol )
	{
	case MB_CH_STATUS_SYN:
		{
			MB_SYN_CH_STATUS* recvMsg      = (MB_SYN_CH_STATUS*)msgBuf;
			cLoginProcess*    loginProcess = g_loginSrv->GetLoginProcess( );

			if ( loginProcess != NULL )
				loginProcess->ChannelStatus( recvMsg->cid, recvMsg->status );

			if ( g_udp == true )
				printf( "\tCID(=0x%08xh) STATUS(=%d)\n", recvMsg->cid, recvMsg->status );
		}
		break;
	default:
		return false;
	}
	return true;
}

// RecvComplete Method
//
// UDP:.......User Datagram Protocol.
// Datagram:..패킷 교환에서, 데이터 단말 장치와 망과의 사전 접속 절차에 의하지 않고,
//            하나하나의 패킷이 발신 데이터 단말 장치와 수신처 데이터 단말 장치 간의
//            경로 지정을 위한 충분한 정보를 가지고 있는 패킷.
bool cRudpProcess::RecvComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &m_cs );

	// 수신된 데이터 처리.
	perIoContext->offset = bytesTransferred;

	// 더블버퍼링 처리.
	PerIoContext** buffer = m_ioContextBackBuffer->buffer;
	long&          offset = m_ioContextBackBuffer->offset;

	if ( offset < MAX_IO_CONTEXT_BUFFER_LEN )
	{
		buffer[ offset ] = perIoContext;
		offset++;
	}
	else
	{
		m_ioContextPool->ReleaseIoContext( perIoContext );
		PostServerEvent( "Warning - cRudpProcess::RecvComplete Method - Overflow I/O Context Buffer." );
	}

	// 수신을 위해 I/O Context를 준비한다.
	perIoContext = m_ioContextPool->GetIoContext( m_recv, IOCP_REQUEST_READ );
	if ( RecvPost( completionKey, perIoContext ) == false )
	{
		PostServerEvent( "Error - cRudpProcess::RecvPost Method - Return value is false." );
		return false;
	}
	return true;
}

// SendComplete Method
bool cRudpProcess::SendComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &m_cs );
	RUDP*   rudp = (RUDP*)perIoContext->buffer;
	long    mask = (rudp->seq & SM_ClassD);

	if ( rudp->tos != TOS_RUDP )
	{
		if ( mGameBuffer != NULL && perIoContext->addr.sin_port == mGame.sin_port )
		{
			PerIoContext** temp     = &mGameBuffer->buffer[ mask ];
			bool           retvalue = true;

			if ( (*temp) != NULL )
			{
				retvalue = IocpRudp::SendComplete( completionKey, (*temp), (*temp)->offset );
			}
			(*temp) = perIoContext;

			mGameBuffer->offset = max( mGameBuffer->offset, mask );
			return retvalue;
		}
		else if ( mLogBuffer != NULL && perIoContext->addr.sin_port == mLog.sin_port )
		{
			PerIoContext** temp     = &mLogBuffer->buffer[ mask ];
			bool           retvalue = true;

			if ( (*temp) != NULL )
			{
				retvalue = IocpRudp::SendComplete( completionKey, (*temp), (*temp)->offset );
			}
			(*temp) = perIoContext;

			mLogBuffer->offset = max( mLogBuffer->offset, mask );
			return retvalue;
		}
	}

	return IocpRudp::SendComplete( completionKey, perIoContext, bytesTransferred );
}

// BackendThread Method
DWORD cRudpProcess::BackendThread( )
{
	DWORD currentTick;
	DWORD elapsedTick;

	DWORD relayTick = 0;

	try {
		while ( true )
		{
			currentTick = GetTickCount( );

			// 서버종료, Thread를 나간다.
			if ( m_endServer == true )
				break;

			// Double Buffering 처리.
			EnterCriticalSection( &m_cs );

				IoContextBuffer* temp  = m_ioContextBackBuffer;
				m_ioContextBackBuffer  = m_ioContextFrontBuffer;
				m_ioContextFrontBuffer = temp;

			LeaveCriticalSection( &m_cs );

			// 수신된 데이터 처리.
			PerIoContext** buffer = m_ioContextFrontBuffer->buffer;
			long&          offset = m_ioContextFrontBuffer->offset;

			if ( offset > 0 )
			{
				do {
					try {
						PerIoContext* ioContext = (*buffer);
						RUDP*         rudp      = (RUDP*)ioContext->buffer;

						if ( g_udpFrom == true )
						{
							printf( "#Recver::RecvComplete - Addr(%d.%d.%d.%d):Port(%d)\n"
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b1
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b2
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b3
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b4
									,ioContext->addr.sin_port );
						}
						if ( g_udpPacket == true )
						{
							// 패킷해더(RUDP Header) 정보 출력 - 버전/헤더길이/서비스 종류/전체길이.
							printf( "#Packets: VER:%02x/HLEN:%02x/TOS:%02x/TLEN:%04x/SEQ:%08x\n"
									,rudp->ver
									,rudp->hlen
									,rudp->tos
									,rudp->tlen
									,rudp->seq );
						}

						if ( rudp->tos == TOS_RUDP )
						{
							bool retvalue = ReSendExec( rudp->addr, rudp->port, rudp->seq );

							if ( g_udp == true )
								printf( "#Packets: ReSend->Seq(%08x) = %d\n", rudp->seq, retvalue );

							m_ioContextPool->ReleaseIoContext( ioContext );
						}
						else if ( rudp->ver == RUDP_PHVer && rudp->hlen == RUDP_PHLen )
						{
							u_long        pid          = (ioContext->addr.sin_addr.s_imp << 0x10) | ioContext->addr.sin_port;
							PacketBuffer* packetBuffer = mPacketPool->SearchPacketBuffer( pid );

							if ( packetBuffer == NULL )
							{
								packetBuffer = mPacketPool->GetPacketBuffer( pid );
							}
							if ( packetBuffer != NULL )
							{
								// Packet 흐름 검사를 위한 버퍼링.
								if ( packetBuffer->seq < rudp->seq )
								{
									if ( g_udp == true )
										printf( "PacketBuffer->Seq(%08x) < Packet->Seq(%08x)\n", packetBuffer->seq, rudp->seq );

									if ( packetBuffer->seq != 0 )
									{
										u_long lost = (rudp->seq - packetBuffer->ack);

										packetBuffer->seq = rudp->seq;
										if ( lost >= MAX_PACKET_BUFFER_LEN )
										{
											if ( g_udp == true )
												printf( "\tPackets: Buffer Overflow = %08x\n", lost );
										}
										else if ( lost > 0 )
										{
											for ( u_long ack = packetBuffer->ack; ack < packetBuffer->seq; ack++ )
											{
												u_long        mask     = (ack & SM_ClassD);
												PerIoContext* temp     = packetBuffer->buffer[ mask ];
												bool          reliable = true;

												if ( temp != NULL )
												{
													RUDP* rudp = (RUDP*)temp->buffer;
													reliable = (ack != rudp->seq);
												}
												if ( reliable )
												{
													SendReliable( rudp->addr, rudp->port, ack );
													if ( g_udp == true )
														printf( "\tPackets: Lost = %08x\n", ack );
												}
											}
										}
									}
									else
									{
										packetBuffer->seq = rudp->seq;
										packetBuffer->ack = rudp->seq;
									}
								}
								else if ( packetBuffer->seq > rudp->seq )
								{
									if ( packetBuffer->ack <= rudp->seq )
									{
										if ( g_udp == true )
											printf( "\tPackets: Recovery = %08x\n", rudp->seq );
									}
								}
								// Packet 흐름 정리를 위한 버퍼링.
								if ( packetBuffer->ack <= rudp->seq )
								{
									u_long         mask = (rudp->seq & SM_ClassD);
									PerIoContext** temp = &packetBuffer->buffer[ mask ];

									if ( (*temp) != NULL )
									{
										m_ioContextPool->ReleaseIoContext( (*temp) );
									}
									(*temp) = ioContext;

									packetBuffer->offset = max( packetBuffer->offset, mask );
									packetBuffer->update = currentTick + 180000;// 최대3분대기.
								}
								// Packet 흐름 처리를 위한 버퍼링.
								while ( packetBuffer->ack <= packetBuffer->seq )
								{
									u_long         mask = (packetBuffer->ack & SM_ClassD);
									PerIoContext** temp = &packetBuffer->buffer[ mask ];

									if ( (*temp) == NULL )
										break;

									RUDP* rudp = (RUDP*)(*temp)->buffer;

									if ( rudp->seq != packetBuffer->ack )
										break;

									// 서비스 타입이 [TOS_LOGIN] 또는 [TOS_GAME] 아닐경우 오류 처리.
									if ( rudp->tos == TOS_LOGIN || rudp->tos == TOS_GAME )
									{
										MSGBUF* msgBuf = (MSGBUF*)((char*)rudp + rudp->hlen);
										int     msgLen = rudp->tlen - rudp->hlen;

										BatchComplete( msgBuf, msgLen );
									}

									m_ioContextPool->ReleaseIoContext( (*temp) );
									(*temp) = NULL;

									packetBuffer->ack++;
								}
							}
						}
						else
						{
							m_ioContextPool->ReleaseIoContext( ioContext );
						}
					} catch ( char* str ) {
						PostServerEvent( "Caught 'BatchComplete' exception type: '%s'. Throwing 'cRudpProcess::BackendThread' exception", str );
						Sleep( 100 );
						throw;
					} catch ( ... ) {
						PostServerEvent( "In BatchComplete Method. Throwing 'cRudpProcess::BackendThread' exception." );
						Sleep( 100 );
						throw;
					}
					buffer++;
					offset--;
				} while ( offset > 0 );
			}
			// PacketPool 데이터 처리.
			if ( mPacketPool != NULL )
			{
				PacketBuffer* packetBuffer = mPacketPool->GetPagedPoolUsage( );
				PacketBuffer* next         = NULL;

				while ( packetBuffer )
				{
					next = packetBuffer->next;
						if ( packetBuffer->update < currentTick )
						{
							for ( u_long i = 0; i <= packetBuffer->offset; i++ )
							{
								PerIoContext* ioContext = packetBuffer->buffer[ i ];
								if ( ioContext != NULL )
								{
									m_ioContextPool->ReleaseIoContext( ioContext );
								}							
							}
							mPacketPool->ReleasePacketBuffer( packetBuffer );
						}
					packetBuffer = next;
				}
			}
			// Relay Packet 발송.
			if ( relayTick < currentTick )
			{
				HANDLE handle = NULL;

				if ( GetPacket( &handle, TOS_RELAY, mGame, mGameSeq ) != NULL )
					SendPacket( handle );

				if ( GetPacket( &handle, TOS_RELAY, mLog, mLogSeq ) != NULL )
					SendPacket( handle );

				relayTick = currentTick + 1000;
			}

			// 지연시간 약16(ms) - CPU 과부화 방지 & 내부 동기화.
			elapsedTick = GetTickCount( ) - currentTick;
			if ( elapsedTick < 16 )
			{
				Sleep( (16 - elapsedTick) );
			}
		}
	} catch ( char* str ) {
		PostServerEvent( "Caught 'while ( true ) { ... }' exception type: '%s'. Throwing 'cRudpProcess::BackendThread' exception", str );
		Sleep( 100 );
		throw;
	} catch ( ... ) {
		PostServerEvent( "In while ( true ) { ... }. Throwing 'cRudpProcess::BackendThread' exception." );
		Sleep( 100 );
		throw;
	}
	return 0;
}
