// Include
#include "gamesrv.h"
#include "BillPayletter.h"
#include "BillPayLatter_Common.h"

// Local definitions
#pragma warning( disable: 4127 )


// Global data
cBillPayletter* cBillPayletter::mBillPayletter = NULL;

// cBillPayletter Constructor
cBillPayletter::cBillPayletter(void)
{
	mBillPayletter = this;
	mUniqueIdx = 0;
	mServerNum = 0;
	mChannelNum = 0;

	mServerAuth = false;
	mNextServerCheckTime = 0;
}

// ~cBillPayletter Destructor
cBillPayletter::~cBillPayletter(void)
{
	mBillPayletter = NULL;
}

// Initialize Method
bool cBillPayletter::BPInitialize(char* ipAddr, unsigned short port, unsigned short numWorkerThreads, unsigned int bufferLength,
								unsigned short serverNum, unsigned short channelNum )
{
	mUniqueIdx = 0;
	mServerAuth = false;

	mServerNum = serverNum;
	mChannelNum = channelNum;

	mNextServerCheckTime = GetTickCount( );

	return cIocpClient::Initialize( ipAddr, port, numWorkerThreads, bufferLength );
}

// Shutdown Method
void cBillPayletter::Shutdown(DWORD maxWait)
{
	Sleep( 1000 );
	cIocpClient::Shutdown( maxWait );
}

// PostServerEvent Method
bool cBillPayletter::PostServerEvent(LPCTSTR format, ...)
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


// RecvComplete Method
bool cBillPayletter::RecvComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock       lock( &mCs );
	cGameProcess* gameProcess = g_gameSrv->GetGameProcess( );

	// 오프셋의 크기를 누적한다.
	perIoContext->offset += bytesTransferred;

	DWORD packetSize = sizeof(_PAYLETTER_HEADER);
	DWORD length     = perIoContext->offset; // 수신된 누적버퍼의 크기
	DWORD offset     = 0;                    // 수신된 누적버퍼의 오프셋

	while ( length >= packetSize )
	{
		_PAYLETTER_HEADER* packet = (_PAYLETTER_HEADER*)(perIoContext->buffer + offset);

		DWORD msgSize = ntohs( packet->mPacketSize );

		if( length < msgSize )
			break;

		packetSize = msgSize;

		packet->mPacketType   = ntohs( packet->mPacketType );
		packet->mPacketSize   = ntohs( packet->mPacketSize );
		packet->mSeqNo        = ntohl( packet->mSeqNo      );

		// Packet Type 검사.
		switch ( packet->mPacketType )
		{
		case BP_ServerConnect_Res:
			{
				BP_RES_CONNECT* pConnect = (BP_RES_CONNECT*)packet;

				/// 접속 결과 기록
				PostServerEvent("cBillPayletter::RecvComplete BP_ServerConnect_Res [%d,%d]", pConnect->mSeqNo, pConnect->mResult );

				if( pConnect->mResult == 0 )
				{
					/// 접속허용
					mServerAuth = true;

					mNextServerCheckTime = GetTickCount() + SERVER_REFRESH;
				}
				else
				{
					mServerAuth = false;

					/// 접속 실패시 20초 간격으로 재시도
					mNextServerCheckTime = GetTickCount() + SERVER_REFRESH;
				}
			}
			break;
		case BP_UserAuthentication_Res:
			{
				BP_RES_AUTHENTICATION* pAuth = (BP_RES_AUTHENTICATION*)packet;
				pAuth->mUserNo = ntohl( pAuth->mUserNo );

				if( pAuth->mResult == 0 && pAuth->mUserStatus != 0 )
				{
					/// 인증 완료 버프 혜택 부여
					if ( gameProcess != NULL )
					{
						/// pc방 혜택 유저 기록
						PostServerEvent("cBillPayletter::RecvComplete BP_UserAuthentication_Res [%d,%d,%d,%d]", pAuth->mSeqNo, pAuth->mResult,
							pAuth->mUserStatus, pAuth->mUserNo );

						gameProcess->BP_User_Auth( pAuth->mUserNo );
					}
				}
			}
			break;
		case BP_ServerAlive_Res:
			{
				BP_RES_SERVERALIVE* pServerAlive = (BP_RES_SERVERALIVE*)packet;

				if( pServerAlive->mResult != 0 )
					PostServerEvent("cBillPayletter::RecvComplete BP_ServerAlive_Res [%d,%d]", pServerAlive->mSeqNo, pServerAlive->mResult );
			}
			break;
		case BP_UserAlive_Res:
			{
				BP_RES_USERALIVE* pUserAlive = (BP_RES_USERALIVE*)packet;
				pUserAlive->mUserNo = ntohl( pUserAlive->mUserNo );
				
				if( pUserAlive->mResult != 0 )
				{
					PostServerEvent("cBillPayletter::RecvComplete BP_UserAlive_Res [%d,%d,%d]", pUserAlive->mSeqNo, pUserAlive->mResult, pUserAlive->mUserNo );
					UserOut( pUserAlive->mUserNo );
				}
			}
			break;
		case BP_ItemGive_Syn:
			{
				BP_SYN_ITEMGIVE* pItem = (BP_SYN_ITEMGIVE*)packet;
				pItem->mUserNo = ntohl( pItem->mUserNo );

				if ( gameProcess != NULL )
					gameProcess->BP_Item_Give( pItem->mUserNo );
			}
			break;
		case BP_UserOut_Res:
			{
				BP_RES_USEROUT* pUserOut = (BP_RES_USEROUT*)packet;
				pUserOut->mUserNo = ntohl( pUserOut->mUserNo );

				if( pUserOut->mResult == 0 )
				{
					PostServerEvent("cBillPayletter::RecvComplete BP_UserOut_Res [%d,%d,%d]", pUserOut->mSeqNo, pUserOut->mResult, pUserOut->mUserNo );
					/// 버프 혜택 제거
					if ( gameProcess != NULL )
						gameProcess->BP_User_End( pUserOut->mUserNo );
				}
			}
			break;
		default:
			
			mNextServerCheckTime = GetTickCount( ) + SERVER_REFRESH;
			
			// Error.
			Close( completionKey, perIoContext );
			if ( g_verbose == true )
			{
				printf( "RT->ACCT::Packet_Type=Unknown(%d)\n", packet->mPacketType );
			}
			return false;
		}

		// 처리후 Offset 및 Length를 다시 계산한다.
		length -= packetSize;
		offset += packetSize;
	}

	// 이어 받기를 위해 데이터를 보관한한다.
	if ( offset > 0 )
	{
		PerIoContext* recvIoContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_READ );
		DWORD         error         = 0;

		if ( length > 0 )
		{
			memcpy( recvIoContext->buffer, (perIoContext->buffer + offset), length );
			recvIoContext->offset = length;
		}

		mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
		perIoContext = recvIoContext;
	}

	// 수신을 위해 I/O Context를 준비한다.
	return RecvPost( completionKey, perIoContext );
}

// Close Method
void  cBillPayletter::Close(ULONG_PTR completionKey, PerIoContext* perIoContext)
{
	if ( mEndClient == false && g_verbose == true )
	{
		VOID* msgBuf;
		if ( ErrorCode2String( &msgBuf ) )
		{
			Verbose->WriteLog("cBillPayletter::Close(%d)-%s", GetLastError( ), (char*)msgBuf );
			LocalFree( msgBuf );
		}
	}
	cGameProcess* gameProcess = g_gameSrv->GetGameProcess( );
	if ( gameProcess != NULL )
	{
		gameProcess->BP_Close( );
	}

	ServerDisConn();

	cIocpClient::Close( completionKey, perIoContext );
}

bool cBillPayletter::ServerConnect()
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	BP_REQ_CONNECT* sendMsg   = (BP_REQ_CONNECT*)ioContext->buffer;
	sendMsg->mPacketType = htons( BP_ServerConnect_Req );
	sendMsg->mPacketSize = htons( sizeof(BP_REQ_CONNECT) );
	sendMsg->mSeqNo = htonl( ++mUniqueIdx );
	sendMsg->mLoginServerNo = htons( mServerNum * SERVERNUM_UNIT + mChannelNum );
	ioContext->offset = sizeof(BP_REQ_CONNECT);

	/// 페이레터 접속 시도
	PostServerEvent("cBillPayletter::ServerConnect BP_ServerConnect_Req %d,%d", mUniqueIdx, mServerNum * SERVERNUM_UNIT + mChannelNum );

	return SendPost( NULL, ioContext );
}

bool cBillPayletter::UserAuthentication( unsigned long ipAddr, unsigned long memberIdx )
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	BP_REQ_AUTHENTICATION* sendMsg   = (BP_REQ_AUTHENTICATION*)ioContext->buffer;
	sendMsg->mPacketType = htons( BP_UserAuthentication_Req );
	sendMsg->mPacketSize = htons( sizeof(BP_REQ_AUTHENTICATION) );
	sendMsg->mSeqNo = htonl( ++mUniqueIdx );
	sendMsg->mDisconnect = 1;
	sendMsg->mIPAddr = htonl( ipAddr ); 
	sendMsg->mUserNo = htonl( memberIdx );
	ioContext->offset = sizeof(BP_REQ_AUTHENTICATION);
	return SendPost( NULL, ioContext );
}

bool cBillPayletter::ServerAlive()
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	BP_REQ_SERVERALIVE* sendMsg   = (BP_REQ_SERVERALIVE*)ioContext->buffer;
	sendMsg->mPacketType = htons( BP_ServerAlive_Req );
	sendMsg->mPacketSize = htons( sizeof(BP_REQ_SERVERALIVE) );
	sendMsg->mSeqNo = htonl( ++mUniqueIdx );
	sendMsg->mLoginServerNo = htons( mServerNum * SERVERNUM_UNIT + mChannelNum );
	ioContext->offset = sizeof(BP_REQ_SERVERALIVE);

	/// Verbose->WriteLog("cBillPayletter::ServerAlive BP_ServerAlive_Req %d,%d", mUniqueIdx, mServerNum * SERVERNUM_UNIT + mChannelNum );
	
	return SendPost( NULL, ioContext );
}

bool cBillPayletter::UserAlive( unsigned long memberIdx )
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	BP_REQ_USERALIVE * sendMsg   = (BP_REQ_USERALIVE *)ioContext->buffer;
	sendMsg->mPacketType = htons( BP_UserAlive_Req );
	sendMsg->mPacketSize = htons( sizeof(BP_REQ_USERALIVE ) );
	sendMsg->mSeqNo = htonl( ++mUniqueIdx );
	sendMsg->mUserNo = htonl( memberIdx );
	ioContext->offset = sizeof(BP_REQ_USERALIVE );

	return SendPost( NULL, ioContext );
}

bool cBillPayletter::UserOut( unsigned long memberIdx )
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	BP_REQ_USEROUT* sendMsg   = (BP_REQ_USEROUT*)ioContext->buffer;
	sendMsg->mPacketType = htons( BP_UserOut_Req );
	sendMsg->mPacketSize = htons( sizeof(BP_REQ_USEROUT) );
	sendMsg->mSeqNo = htonl( ++mUniqueIdx );
	sendMsg->mUserNo = htonl( memberIdx );
	ioContext->offset = sizeof(BP_REQ_USEROUT);
	return SendPost( NULL, ioContext );
}

bool cBillPayletter::ServerDisConn()
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	BP_RES_SERVERDISCONNECT* sendMsg   = (BP_RES_SERVERDISCONNECT *)ioContext->buffer;
	sendMsg->mPacketType = htons( BP_ServerDisConnect_Req );
	sendMsg->mPacketSize = htons( sizeof(BP_RES_SERVERDISCONNECT) );
	sendMsg->mSeqNo = htonl( ++mUniqueIdx );
	sendMsg->mLoginServerNo = mServerNum * SERVERNUM_UNIT + mChannelNum;
	ioContext->offset = sizeof(BP_RES_SERVERDISCONNECT);

	Verbose->WriteLog("cBillPayletter::ServerDisConn BP_ServerDisConnect_Req %d,%d", sendMsg->mSeqNo, sendMsg->mLoginServerNo );

	return SendPost( NULL, ioContext );
}

// BackendThread Method
DWORD cBillPayletter::BackendThread( )
{
	DWORD currentTick;
	DWORD elapsedTick;

	while ( true )
	{
		currentTick = GetTickCount( );

		// 서버종료, Thread를 나간다.
		if ( mEndClient == true )
		{
			if ( IsConnect( ) == true )
			{
				ServerDisConn();
			}
			break;
		}
		if ( IsConnect( ) == false )
		{
			if ( Connect( mAddr ) == true )
			{
				if( mNextServerCheckTime != 0 && mNextServerCheckTime < currentTick )
				{
					ServerConnect();

					mNextServerCheckTime = 0;
				}
			}
		}
		else
		{
			if( mNextServerCheckTime != 0 && mNextServerCheckTime < currentTick )
			{
				mNextServerCheckTime = currentTick + SERVER_REFRESH;

				if( mServerAuth == true )
					ServerAlive();
				else
					ServerConnect();
			}
		}

		
		// 지연시간 약16(ms) - CPU 과부화 방지 & 내부 동기화.
		elapsedTick = GetTickCount( ) - currentTick;
		if ( elapsedTick < 16 )
		{
			Sleep( (16 - elapsedTick) );
		}
	}

	return 0;
}
