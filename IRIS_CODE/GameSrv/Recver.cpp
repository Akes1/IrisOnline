// Include
#include "gamesrv.h"
#include "packet.h"

// Local definitions
#pragma warning( disable: 4127 )

// Global data

// cRecver Constructor
cRecver::cRecver(void) : mPacketPool(NULL)
{}

// ~cRecver Destructor
cRecver::~cRecver(void)
{}

// Initialize Method
bool cRecver::Initialize(char* ipAddr, unsigned short port, unsigned short numWorkerThreads, unsigned int bufferLength)
{
	mPacketPool = new PacketPool( );
	return IocpUdpRecv::Initialize( ipAddr, port, numWorkerThreads, bufferLength );
}

// Shutdown Method
void cRecver::Shutdown(DWORD maxWait)
{
	Sleep( 50 );
	IocpUdpRecv::Shutdown( maxWait );
	mPacketPool->Shutdown( );

	delete mPacketPool;
	mPacketPool = NULL;
}

// PostServerEvent Method
bool cRecver::PostServerEvent(LPCTSTR format, ...)
{
	cSender* sender   = g_gameSrv->GetSender( );
	bool     retvalue = false;

	if ( sender != NULL )
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
			retvalue = sender->PostServerEvent( (char*)msgBuf );
			free( msgBuf );
		}
	}
	return retvalue;
}

// BatchComplete Method
bool cRecver::BatchComplete(MSGBUF* msgBuf, int msgLen)
{
	cGameProcess* gameProcess = g_gameSrv->GetGameProcess( );

	if ( gameProcess == NULL )
		return false;
	else if ( gameProcess->IsChannelCheck( ) == false )
		return false;

	switch ( msgBuf->protocol )
	{
	case MB_CHAT_MEGAPHONE_SYN:
		{
			MB_SYN_CHAT_MEGAPHONE* recvMsg = (MB_SYN_CHAT_MEGAPHONE*)msgBuf;
			int				       len	   = msgLen - (sizeof(MB_SYN_CHAT_MEGAPHONE) - sizeof(recvMsg->msg) );
			gameProcess->ChatMegaphone( recvMsg->cid, recvMsg->msg, len );
		}
		break;
	case MB_CHAT_SYN:
		{
			MB_SYN_CHAT_SYNC* recvMsg = (MB_SYN_CHAT_SYNC*)msgBuf;
			int				  len	  = msgLen - (sizeof(MB_SYN_CHAT_SYNC) - sizeof(recvMsg->msg) );
			gameProcess->ChatSync( recvMsg->cid, recvMsg->msg, len );
		}
		break;
	case MB_SERVER_NOTICE_SYN:
		{
			MB_SYN_SERVER_NOTICE* recvMsg = (MB_SYN_SERVER_NOTICE*)msgBuf;
			gameProcess->ServerNotice( recvMsg->cid, recvMsg->message );
		}
		break;
	case MB_SERVER_DOWN_COUNT_NOTICE_SYN:
		{
			MB_SYN_SERVER_DOWN_COUNT_NOTICE* recvMsg = (MB_SYN_SERVER_DOWN_COUNT_NOTICE*)msgBuf;
			gameProcess->ServerDownCountNotice( recvMsg->cid, recvMsg->count );
		}
		break;
	case MB_FRIEND_SYN:
		{
			MB_SYN_FRIEND_SYN* recvMsg = (MB_SYN_FRIEND_SYN*)msgBuf;
			int				   len	   = msgLen - (sizeof(MB_SYN_FRIEND_SYN) - sizeof(recvMsg->msg));
			gameProcess->FriendSync( recvMsg->cid, recvMsg->msg, len );
		}
		break;
	case MB_GUILD_SYNC_SYN:
		{
			MB_SYN_GUILD_SYNC* recvMsg = (MB_SYN_GUILD_SYNC*)msgBuf;
			int                len     = msgLen - (sizeof(MB_SYN_GUILD_SYNC) - sizeof(recvMsg->msg));
			gameProcess->GuildSync( recvMsg->cid, recvMsg->msg, len );
		}
		break;
	case MB_PVP_STATUS_SYN:
		{
			MB_SYN_PVP_STATUS* recvMsg = (MB_SYN_PVP_STATUS*)msgBuf;
			gameProcess->PVPStatus( recvMsg->cid, recvMsg->status, recvMsg->pvpType, recvMsg->aryPos );
		}
		break;
	case MB_PVP_NOTICE_SYN:
		{
			MB_SYN_PVP_NOTICE* recvMsg = (MB_SYN_PVP_NOTICE*)msgBuf;
			gameProcess->PVPNotice( recvMsg->cid, recvMsg->leftMinute, recvMsg->pvpType, recvMsg->aryPos );
		}
		break;
	case MB_PVP_PLAYERLIST_SYN:
		{
			MB_SYN_PVP_PLAYERLIST* recvMsg = (MB_SYN_PVP_PLAYERLIST*)msgBuf;
			gameProcess->PVPPlayerList( recvMsg );
		}
		break;
	case MB_PVP_GMCHANNELMOVE_SYN:
		{
			MB_SYN_PVPGM_CHANNELMOVE* recvMsg = (MB_SYN_PVPGM_CHANNELMOVE*)msgBuf;
			gameProcess->PvPGMChannelRes( recvMsg );
		}
		break;
	case MB_THEME_STATUS_SYN:
		{
			MB_SYN_THEME_STATUS* recvMsg = (MB_SYN_THEME_STATUS*)msgBuf;
			gameProcess->ThemeStatus( recvMsg->cid, recvMsg->status, recvMsg->mapType );
		}
		break;
	case MB_CH_STATUS_SYN:
		{
			MB_SYN_CH_STATUS* recvMsg = (MB_SYN_CH_STATUS*)msgBuf;
			gameProcess->ChannelStatus( recvMsg->cid, recvMsg->status );
		}
		break;
	case MB_CH_LIST_SYN:
		{
			MB_SYN_CH_LIST* recvMsg = (MB_SYN_CH_LIST*)msgBuf;
			for ( BYTE i = 0; i < recvMsg->rowCount; i++ )
				gameProcess->ChannelStatus( recvMsg->channels[ i ].cid, recvMsg->channels[ i ].status );
		}
		break;
	case MB_MONSTER_SYN_SYN:
		{
			MB_SYN_MONSTER_SYNC* pRecMsg = (MB_SYN_MONSTER_SYNC*)msgBuf;
			int len = msgLen - (sizeof(MB_SYN_MONSTER_SYNC) - sizeof(pRecMsg->msg));
			gameProcess->MonsterSync( pRecMsg->cid, pRecMsg->msg, len );
		}
		break;
	case MB_CH_SYNC_SYN:
		gameProcess->ChannelSync( );
		break;
	case MB_POST_RECEIVED_SYN:
		gameProcess->PostReceived( (MB_SYN_POST_RECEIVED*)msgBuf );
		break;
	case MB_PARTY_SYN:
		gameProcess->PartySync( (MB_SYN_PARTY*)msgBuf );
		break;
    case MB_ENHANCED_RESULT_SYN:
        gameProcess->EnhancedSync( (MB_SYN_ENHANCED_RESULT*)msgBuf );
        break;
	case MB_NO_CHAT_SYN:
		gameProcess->NoChatSync( (MB_SYN_NO_CHAT*)msgBuf );
		break;
	case MB_UNIQUEITEM_GET_SYN:
		gameProcess->UniqueItemSync( (MB_SYN_UNIQUEITEM_GET*)msgBuf );
		break;
	case MB_ADD_GM_EVENT_SYN:
		gameProcess->AddGMEventSync( (MB_SYN_ADD_GM_EVENT*)msgBuf );
		break;
	case MB_DEL_GM_EVENT_SYN:
		gameProcess->DelGMEventSync( (MB_SYN_DEL_GM_EVENT*)msgBuf );
		break;
	case MB_GM_KICK_EVENT_SYN:
		gameProcess->GMKickEventSync( (MB_SYN_GM_KICK_EVENT*)msgBuf );
		break;
	case MB_ITEM_DROPLIMIT_UPDATE:
		gameProcess->ItemDropLimitUpdate( (MB_SYN_DROPITEM_UPDATE*)msgBuf );
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
bool cRecver::RecvComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
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
			PostServerEvent( "Warning - cRecver::RecvComplete Method - Overflow I/O Context Buffer." );
		}
	}

	// 수신을 위해 I/O Context를 준비한다.
	perIoContext = m_ioContextPool->GetIoContext( m_socket, IOCP_REQUEST_READ );
	if ( RecvPost( completionKey, perIoContext ) == false )
	{
		PostServerEvent( "Error - cRecver::RecvPost Method - Return value is false." );
		return false;
	}
	return true;
}

// BackendThread Method
DWORD cRecver::BackendThread( )
{
	DWORD currentTick;
	DWORD elapsedTick;

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
						RUDP*         rudp      = (RUDP*)ioContext->buffer;

						if ( g_udpFrom == true )
						{
							SOCKADDR_IN& addr = ioContext->addr;
							printf( "#Packets: Addr(%d.%d.%d.%d):Port(%d)\n"
									,addr.sin_addr.S_un.S_un_b.s_b1
									,addr.sin_addr.S_un.S_un_b.s_b2
									,addr.sin_addr.S_un.S_un_b.s_b3
									,addr.sin_addr.S_un.S_un_b.s_b4
									,addr.sin_port );
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
							cSender* sender   = g_gameSrv->GetSender( );
							bool     retvalue = sender->ReSendPost( rudp->addr, rudp->port, rudp->seq );

							if ( g_udp == true )
								printf( "#Packets: ReSendPost->Seq(%08x) = %d\n", rudp->seq, retvalue );

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

										lost = packetBuffer->seq - packetBuffer->ack;
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

												PostServerEvent( "Packets: Loss = %08x", packetBuffer->ack );
												packetBuffer->ack++;
											}
										}
										lost = packetBuffer->seq - packetBuffer->ack;
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
												cSender* sender = g_gameSrv->GetSender( );
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
						PostServerEvent( "Caught 'BatchComplete' exception type: '%s'. Throwing 'cRecver::BackendThread' exception", str );
						Sleep( 100 );
						throw;
					} catch ( ... ) {
						PostServerEvent( "In BatchComplete Method. Throwing 'cRecver::BackendThread' exception." );
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

			// 지연시간 약16(ms) - CPU 과부화 방지 & 내부 동기화.
			elapsedTick = GetTickCount( ) - currentTick;
			if ( elapsedTick < 16 )
			{
				Sleep( (16 - elapsedTick) );
			}
		}
	} catch ( char* str ) {
		PostServerEvent( "Caught 'while ( true ) { ... }' exception type: '%s'. Throwing 'cRecver::BackendThread' exception", str );
		Sleep( 100 );
		throw;
	} catch ( ... ) {
		PostServerEvent( "In while ( true ) { ... }. Throwing 'cRecver::BackendThread' exception." );
		Sleep( 100 );
		throw;
	}
	return 0;
}
