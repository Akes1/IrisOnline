// Include
#include "logdemon.h"
#include "packet.h"

// Local definitions

/* macro for SQL Class checking */
#define SQL_LOG_CHECK(sqlLog)               \
	sqlLog = g_logDemon->GetSQLLog( );      \
	if ( sqlLog == NULL )                   \
		return false;

// Global data

// cLogRecver Constructor
cLogRecver::cLogRecver(void) : mPacketPool(NULL)
{}

// ~cLogRecver Destructor
cLogRecver::~cLogRecver(void)
{}

// ErrorLog Method
void cLogRecver::ErrorLog( LPCTSTR format, ... )
{
	cCSLock lock( &m_cs );
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
		FILE*      stream = NULL;
		char       filename[ MAX_PATH ];
		SYSTEMTIME systemtime;
		char       buffer[ 1024 ];

		GetLocalTime( &systemtime );

		sprintf( filename,
				 "%s_%04d_%02d_%02d.log",
				 g_serviceName,
				 systemtime.wYear,
				 systemtime.wMonth,
				 systemtime.wDay );

		stream = fopen( filename, "at" );
		if ( stream != NULL )
		{
			sprintf( buffer,
					 "%04d %02d %02d::%02d %02d %02d : %s\n",
					 systemtime.wYear,
					 systemtime.wMonth,
					 systemtime.wDay,
					 systemtime.wHour,
					 systemtime.wMinute,
					 systemtime.wSecond,
					 (char*)msgBuf );

			fputs( buffer, stream );
			fclose( stream );
		}

		free( msgBuf );
	}
}

// Initialize Method
bool cLogRecver::Initialize(char* ipAddr, unsigned short port, unsigned short numWorkerThreads, unsigned int bufferLength)
{
	mPacketPool = new PacketPool( );
	return IocpUdpRecv::Initialize( ipAddr, port, numWorkerThreads, bufferLength );
}

// Shutdown Method
void cLogRecver::Shutdown(DWORD maxWait)
{
	Sleep( 50 );
	IocpUdpRecv::Shutdown( maxWait );
	mPacketPool->Shutdown( );

	delete mPacketPool;
	mPacketPool = NULL;
}

// SendSQL Method
bool cLogRecver::SendSQL(MSGBUF* msgBuf, int msgLen, SQL_REQUEST_LOG_TYPE logType)
{
	if ( (u_long)msgLen < m_ioContextPool->GetBufferLength( ) )
	{
		cSQLLog* sqlLog = NULL;
		SQL_LOG_CHECK( sqlLog );

		PerIoContext* cbIoContext = m_ioContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
		BOOL          retvalue;

		memcpy( cbIoContext->buffer, msgBuf, msgLen );

		cbIoContext->offset = msgLen;
		cbIoContext->iParam = logType;

		// sqlLog를 통해 SQLServer로 요청.
		retvalue = sqlLog->QueueRequest( logType, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
		return (retvalue == TRUE) ? true : false;
	}
	else
	{
		ErrorLog( "Error - cLogRecver::SendSQL:Logtype(0x%08x) - MsgLen(0x%08x)", logType, msgLen );
		return false;
	}
}

// BatchComplete Method
bool cLogRecver::BatchComplete(MSGBUF* msgBuf, int msgLen)
{
	switch ( msgBuf->protocol )
	{
	case MB_SERVER_EVENT:     return SendSQL( msgBuf, msgLen, SQL_LOG_SERVER_EVENT     );
	case MB_MEMBER_EVENT:     return SendSQL( msgBuf, msgLen, SQL_LOG_MEMBER_EVENT     );
	case MB_CHARACTER_EVENT:  return SendSQL( msgBuf, msgLen, SQL_LOG_CHARACTER_EVENT  );
	case MB_MONEY_EVENT:      return SendSQL( msgBuf, msgLen, SQL_LOG_MONEY_EVENT      );
	case MB_DEPOSIT_EVENT:    return SendSQL( msgBuf, msgLen, SQL_LOG_DEPOSIT_EVENT    );
	case MB_INVENTORY_EVENT:  return SendSQL( msgBuf, msgLen, SQL_LOG_INVENTORY_EVENT  );
	case MB_CONCURRENT_EVENT: return SendSQL( msgBuf, msgLen, SQL_LOG_CONCURRENT_EVENT );
	case MB_QUEST_EVENT:      return SendSQL( msgBuf, msgLen, SQL_LOG_QUEST_EVENT      );
	case MB_GUILD_EVENT:	  return SendSQL( msgBuf, msgLen, SQL_LOG_GUILD_EVENT      );
	case MB_DROPITEM_EVENT:	  return SendSQL( msgBuf, msgLen, SQL_LOG_DROPITEM_EVENT   );
	case MB_SKILL_EVENT:      return SendSQL( msgBuf, msgLen, SQL_LOG_SKILL_EVENT      );
	case MB_ITEM_BILL_EVENT:  return SendSQL( msgBuf, msgLen, SQL_LOG_ITEM_BILL_EVENT  );
	case MB_INFCASH_EVENT:    return SendSQL( msgBuf, msgLen, SQL_LOG_INFCASH_EVENT    );
	case MB_RESERVED_EVENT:   return SendSQL( msgBuf, msgLen, SQL_LOG_RESERVED_EVENT   );
	case MB_POST_EVENT:       return SendSQL( msgBuf, msgLen, SQL_LOG_POST_EVENT       );

	default:
		ErrorLog( "Warning - Unknown Protocol. Throwing 'cLogRecver::BatchComplete' exception." );
		return false;
	}
}

// RecvComplete Method
//
// UDP:.......User Datagram Protocol.
// Datagram:..패킷 교환에서, 데이터 단말 장치와 망과의 사전 접속 절차에 의하지 않고,
//            하나하나의 패킷이 발신 데이터 단말 장치와 수신처 데이터 단말 장치 간의
//            경로 지정을 위한 충분한 정보를 가지고 있는 패킷.
bool cLogRecver::RecvComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	// 수신된 데이터 처리.
	perIoContext->offset = bytesTransferred;

	// Double Buffering 처리.
	CSBlock( &m_cs )
	{
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
			ErrorLog( "Warning - cLogRecver::RecvComplete Method - Overflow I/O Context Buffer." );
		}
	}

	// 수신을 위해 I/O Context를 준비한다.
	perIoContext = m_ioContextPool->GetIoContext( m_socket, IOCP_REQUEST_READ );
	if ( RecvPost( completionKey, perIoContext ) == false )
	{
		ErrorLog( "Error - cLogRecver::RecvPost Method - Return value is false." );
		return false;
	}
	return true;
}

// CallbackComplete Method
bool cLogRecver::CallbackComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	CSBlock( &m_cs )
	{
		bool boolean = (perIoContext->iParam == completionKey && perIoContext->offset == bytesTransferred);

		// 사용이 완료된 I/O Context는 회수한다.
		if ( boolean == true )
		{
			m_ioContextPool->ReleaseIoContext( perIoContext );
		}
	}
	return true;
}

// BackendThread Method
DWORD cLogRecver::BackendThread( )
{
	DWORD      currentTick;
	DWORD      elapsedTick;

	try {
		while ( true )
		{
			currentTick = GetTickCount( );

			// 서버종료, Thread를 나간다.
			if ( m_endServer == true )
				break;

			// Double Buffering 처리.
			CSBlock( &m_cs )
			{
				IoContextBuffer* temp  = m_ioContextBackBuffer;
				m_ioContextBackBuffer  = m_ioContextFrontBuffer;
				m_ioContextFrontBuffer = temp;
			}

			// 수신된 데이터 처리.
			PerIoContext** buffer = m_ioContextFrontBuffer->buffer;
			long&          offset = m_ioContextFrontBuffer->offset;

			if ( offset > 0 )
			{
				do {
					try {
						PerIoContext* ioContext = (*buffer);
						RUDP*         rudp    = (RUDP*)ioContext->buffer;

						if ( g_udpFrom == true )
						{
							printf( "#Packets: Addr(%d.%d.%d.%d):Port(%d)\n"
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b1
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b2
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b3
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b4
									,ioContext->addr.sin_port );
						}
						if ( g_udpPacket == true )
						{
							// 패킷해더(Packet Header) 정보 출력 - 버전/헤더길이/서비스 종류/전체길이.
							printf( "#Packets: VER:%02x/HLEN:%02x/TOS:%02x/TLEN:%04x/SEQ:%08x\n"
									,rudp->ver
									,rudp->hlen
									,rudp->tos
									,rudp->tlen
									,rudp->seq );
						}

						if ( rudp->tos == TOS_RUDP )
						{
							cLogSender* sender   = g_logDemon->GetLogSender( );
							bool        retvalue = sender->ReSendPost( rudp->addr, rudp->port, rudp->seq );

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
										u_long lost;
										packetBuffer->seq = rudp->seq;

										lost = rudp->seq - packetBuffer->ack;
										if ( lost >= MAX_PACKET_BUFFER_LEN )
										{
											while ( packetBuffer->ack < packetBuffer->seq )
											{
												u_long        mask = (packetBuffer->ack & SM_ClassD);
												PerIoContext* temp = packetBuffer->buffer[ mask ];

												if ( temp != NULL )
												{
													RUDP* rudp = (RUDP*)temp->buffer;
													if ( rudp->seq == packetBuffer->ack )
														break;
												}

												if ( g_udp == true )
													printf( "\tPackets: Loss = %08x\n", packetBuffer->ack );
												packetBuffer->ack++;
											}
										}
										lost = rudp->seq - packetBuffer->ack;
										if ( lost > 0 )
										{
											u_long        mask     = (packetBuffer->ack & SM_ClassD);
											PerIoContext* temp     = packetBuffer->buffer[ mask ];
											bool          reliable = true;

											if ( temp != NULL )
											{
												RUDP* rudp = (RUDP*)temp->buffer;
												reliable = (packetBuffer->ack != rudp->seq);
											}
											if ( reliable )
											{
												cLogSender* sender = g_logDemon->GetLogSender( );
												sender->SendReliable( rudp->addr, rudp->port, packetBuffer->ack );
												if ( g_udp == true )
													printf( "\tPackets: Lost = %08x\n", packetBuffer->ack );
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
								else
								{
									m_ioContextPool->ReleaseIoContext( ioContext );
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

									// 서비스 타입이 [TOS_LOGIN_LOG] 또는 [TOS_GAME_LOG] 아닐경우 오류 처리.
									if ( rudp->tos == TOS_LOGIN_LOG || rudp->tos == TOS_GAME_LOG )
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
							else
							{
								m_ioContextPool->ReleaseIoContext( ioContext );
							}
						}
						else
						{
							m_ioContextPool->ReleaseIoContext( ioContext );
						}
					} catch ( char* str ) {
						ErrorLog( "Caught 'BatchComplete' exception type: '%s'. Throwing 'cLogRecver::BackendThread' exception", str );
						throw;
					} catch ( ... ) {
						ErrorLog( "In BatchComplete Method. Throwing 'cLogRecver::BackendThread' exception." );
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

			// 지연시간 약16(ms) - CPU 과부화 방지 & 내부 동기화.
			elapsedTick = GetTickCount( ) - currentTick;
			if ( elapsedTick < 10 )
			{
				Sleep( (10 - elapsedTick) );
			}
		}
	} catch ( char* str ) {
		ErrorLog( "Caught 'while ( true ) { ... }.' exception type: %s. Throwing 'cLogRecver::BackendThread' exception.", str );
	} catch ( ... ) {
		ErrorLog( "In while ( true ) { ... }. Throwing 'cLogRecver::BackendThread' exception." );
	}
	return 0;
}
