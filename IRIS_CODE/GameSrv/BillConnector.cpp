// Include
#include "gamesrv.h"


// Local definitions
#pragma warning( disable: 4127 )

#define COMPANY_CODE		"EYAP"
#define LEN_USER_CC			4

#define pgGame_Start	0x00000001	// 게임 시작
#define pgGame_End		0x00000002	// 게임 종료
#define pgServer_Alive	0x00000003	// 서버 확인
#define pgServer_Reset	0x00000004	// 서버 초기화
#define pgUser_Alert	0x00000005	// 사용자 알림
#define pgUser_Sync		0x00000006	// 사용자 확인
#define pgBilling_Auth	0x00000008	// 과금인증
#define pgClient_Move	0x0000000A	// 사용자 게임서버 이동
#define pgServer_Conn	0x0000000C	// 서버연결

struct _PACKET_GAME
{
	long Packet_Type;				// 패킷 타입
	long Packet_Result;				// 처리결과값
	char Game_Server[ 32 ];			// 게임서버의 연결 ID
	char Session[ 32 ];				// GUID
	char User_CC[ 4 ];				// 구분코드(Company Code)
	char User_No[ 20 ];				// 사용자 번호나 주민번호
	char User_ID[ 40 ];				// 사용자 아이디
	char User_IP[ 24 ];				// 사용자 접속 아이피
	char User_Gender;				// 사용자 성별 - 'M'남성/'F','W'여성,'C'법인,'X'확인되지 않음
	char User_Status[ 3 ];			// 상용자 상태 - 'HO'개인/'PC'게임방/'SP'ISP/'PM'프리미엄
	char User_PayType[ 4 ];			// 결제 수단
	long User_Age;					// 사용자 나이
	long Game_No;					// 게임번호(과금DB에 등록된번호)
	char Bill_PayType[ 2 ];			// 지불방식 - 'AO'후불/'DO'직불/'PO'선불
	char Bill_Method[ 2 ];			// 과금방식 - 'FM,FW' 월정액, 주말 월정액/'FD,DB,HB' 일정액, 기간정액/'TX' 코인정액/'TB' 종량
	char Bill_Expire[ 12 ];			// 과금종료일 YYYMMDD
	long Bill_Remain;				// 남은 시간 및 포인트
};

/*-- _PACKET_GAME 메세지 정의
	sendMsg->Packet_Type   = htonl( 0 );
	sendMsg->Packet_Result = htonl( 0 );
	strcpy( sendMsg->Game_Server, "" );
	strcpy( sendMsg->Session, "" );
	strcpy( sendMsg->User_CC, "" );
	strcpy( sendMsg->User_No, "" );
	strcpy( sendMsg->User_ID, "" );
	strcpy( sendMsg->User_IP, "" );
	sendMsg->User_Gender;
	strcpy( sendMsg->User_Status, "" );
	strcpy( sendMsg->User_PayType, "" );
	sendMsg->User_Age      = htonl( 0 );
	sendMsg->Game_No       = htonl( 0 );
	strcpy( sendMsg->Bill_PayType, "" );
	strcpy( sendMsg->Bill_Method, "" );
	strcpy( sendMsg->Bill_Expire, "" );
	sendMsg->Bill_Remain   = htonl( 0 );
*/


// Global data
cBillConnector* cBillConnector::mBillConnector = NULL;

// cBillConnector Constructor
cBillConnector::cBillConnector(void)
{
	mBillConnector = this;
}

// ~cBillConnector Destructor
cBillConnector::~cBillConnector(void)
{
	mBillConnector = NULL;
}

// Initialize Method
bool cBillConnector::Initialize(char* ipAddr, unsigned short port, unsigned short numWorkerThreads, unsigned int bufferLength)
{
	return cIocpClient::Initialize( ipAddr, port, numWorkerThreads, bufferLength );
}

// Shutdown Method
void cBillConnector::Shutdown(DWORD maxWait)
{
	Sleep( 1000 );
	cIocpClient::Shutdown( maxWait );
}

// PostServerEvent Method
bool cBillConnector::PostServerEvent(LPCTSTR format, ...)
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

// PacketResultAuth Method
void cBillConnector::PacketResultAuth(long result)
{
	char buffer[260] = { 0, };

	switch ( result )
	{
	case 0:		// 인증성공
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Success", result );
		break;
	case 2:		// DB에 접근 할 수 없음.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 3:		// 네트워크에 오류 발생.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Network Error has occurred", result );
		break;
	case 10:	// 인증정보 없음. 결제 정보가 DB에 없음.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-You have insufficient points to play. Please top up.", result );
		break;
	case 11:	// 잔액 부족.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-You have insufficient points to play. Please top up.", result );
		break;
	case 12:	// 세션이 없슴.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 17:	// 중복게임 사용자.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-You have already connected another game.", result );
		break;
	case 18:	// 불법 사용자.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-You are illegal user.", result );
		break;
	case 20:	// 정의 되지 않은 패킷.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 25:	// COM 에러
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 26:	// 지정된 시간동안(30초) 응답이 없음.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 27:	// 빌링 DB서버로의 Connect 실패.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 30:	// 패킷에 있는 게임 번호와 빌링 서버에 등록된 게임 번호가 다름.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 31:	// 빌링 어카운트의 서비스 모듈 에러.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 33:	// 해당게임의 구매정보 없음.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-You have insufficient points to play. Please top up.", result );
		break;
	case 35:	// 과도한 인증 처리 요구에 의해 지금은 인증 처리 불가.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 100:	// 정의 되지 않은 오류.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 200:	// IP를 확인 하는 중 COM 에러가 발생.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 201:	// 해당 IP를 보유한 공급자의 정보가 부족.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 210:	// 인증 키를 확인하는 중 COM 에러가 발생.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 211:	// 인증 키를 찾지 못함.
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	case 301:	// 알려지지 않은 오류
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Billing Error", result );
		break;
	default:	// 알수없는 오류
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Unknown Error", result );
		break;
	}
	if ( !(result == 0 || result == 301) )
	{
		PostServerEvent( buffer );
	}
	if ( g_verbose )
	{
		printf( "%s\n", buffer );
	}
}

// PacketResultAlert Method
void cBillConnector::PacketResultAlert(long result)
{
	char buffer[260] = { 0, };

	switch ( result )
	{
	case 1:		// 사용자의 남은 시간(초 단위)
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)", result );
		break;
	case 0:		// 남은시간 없음
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)", result );
		break;
	case -1:	// 환불
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)", result );
		break;
	case -2:	// 중복 로그인
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-You are disconnected due to double login from other PC", result );
		break;
	case -3:	// 예약되어 있던 결제 정보 사용 시작
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)", result );
		break;
	case -4:	// 기타 강제 종료
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)", result );
		break;
	default:	// 알수없는 오류
		sprintf( buffer, "RT->ACCT::Packet_Result(%d)-Unknown Error", result );
		break;
	}
	if ( !(result == 1 || result == -3) )
	{
		PostServerEvent( buffer );
	}
	if ( g_verbose )
	{
		printf( "%s\n", buffer );
	}
}

// RecvComplete Method
bool cBillConnector::RecvComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock       lock( &mCs );
	cGameProcess* gameProcess = g_gameSrv->GetGameProcess( );

	// 오프셋의 크기를 누적한다.
	perIoContext->offset += bytesTransferred;

	DWORD packetSize = sizeof(_PACKET_GAME);
	DWORD length     = perIoContext->offset; // 수신된 누적버퍼의 크기
	DWORD offset     = 0;                    // 수신된 누적버퍼의 오프셋

	while ( length >= packetSize )
	{
		_PACKET_GAME* packet = (_PACKET_GAME*)(perIoContext->buffer + offset);

		packet->Packet_Type   = ntohl( packet->Packet_Type   );
		packet->Packet_Result = ntohl( packet->Packet_Result );
		packet->User_Age      = ntohl( packet->User_Age      );
		packet->Game_No       = ntohl( packet->Game_No       );
		packet->Bill_Remain   = ntohl( packet->Bill_Remain   );

		packet->User_CC;// 구분코드

		// Packet Type 검사.
		switch ( packet->Packet_Type )
		{
		case pgGame_Start:
			/*-- 설명
				 사용자가 게임 서버에 접속하였을 때 게임을 시작해도 되는지 빌링서버에 확인
				 인증 결과가 성공이면 빌링서버에서 사용량 계산을 시작 (종료 이벤트까지)
			*/
			/*-- 게임 서버 작업
				 결과값이 성공이면 게임을 시작, 실패인 경우 클라이언트에게 결과값 전송후 게임 종료
			*/
			PacketResultAuth( packet->Packet_Result );
			if ( packet->Packet_Result == 0 )
			{
				if ( gameProcess != NULL )
				{
					gameProcess->Pg_Game_Start( packet->Session, packet->User_ID, packet->User_Status, packet->Bill_Method, packet->Bill_Expire, packet->Bill_Remain );
				}
			}
			break;
//		case pgGame_End: break;		// 송신전용
		case pgServer_Alive:		// Echo
			ServerAlive( );
			if ( g_verbose == true )
			{
				printf( "RT->ACCT::Packet_Type=Server_Alive\n" );
			}
			break;
//		case pgServer_Reset: break; // 송신전용
		case pgUser_Alert:			// 수신전용
			/*-- 설명
				 Bill_Method = FM/FW/FD/TX 인 경우 게임종료 24시간 전부터 매 1시간마다 전송 종료 1시간 전부터 매 10분마다 전송, 종료 10분전부터 매 1분마다 전송
				 Bill_Method = DB/HB 인 경우 게임이 종료되기 5분전부터 1분에 한번씩 전송
				 예약 구매를 하여도 메세징은 계속되며, 예약구매 여부를 확인한 시점에서 새로 구매된 결제내역을 메세징
			*/
			/*-- 게임 서버 작업
				 빌링서버에서 받은 정보를 바탕으로 사용자에게 알림
				 게임서버에서 Packet_Result가 1과 -3이 아닌 경우 사용자를 강제종료(0x00000002 종료 패킷 전송 안함)
			*/
			PacketResultAlert( packet->Packet_Result );
			if ( gameProcess != NULL )
			{
				gameProcess->Pg_User_Alert( packet->Packet_Result, packet->Session, packet->User_ID, packet->Bill_Method, packet->Bill_Remain );
			}
			break;
		case pgUser_Sync:
			/*-- 설명
				 네트워크 장애와 같은 문제로 게임종료 이벤트를 빌링 시스템이 처리 하지 못한경우 사용자가 게임을 아직 진행중인지 확인
				 오래 접속되어 있는 사용자 여부 확인
				 빌링서버는 사용자가 "OFF" 이면 종료 시간을 기준으로 과금종료
				 종료시간을 알지 못하면 검사 주기만큼 시간을 빼고 계산
			*/
			/*-- 게임 서버 작업
				 같은 세션값을 가진 사용자가 있는지 확인해서, 결과 리턴
				 같은 세션값의 사용자가 있으면 Packet_Result에 1, 없으면 0 반환
			*/
			if ( packet->Packet_Result == 0 )
			{
				if ( gameProcess != NULL )
				{
					gameProcess->Pg_User_Sync( packet->Session, packet->User_ID, packet->User_Status );
				}
			}
			if ( g_verbose == true )
			{
				printf( "RT->ACCT::Packet_Type=User_Sync(%s, %s)\n", packet->User_ID, packet->User_Status );
			}
			break;
		case pgBilling_Auth:
			/*-- 설명
				 사용자가 사용 가/부 여부를 반환
			*/
			/*-- 게임 서버 작업
				 일정 시간 안에 인증 결과를 받지 못하면, 오류 처리(기본 10초)
				 (PC방 사용자의 경우, Bill_Method, Bill_Expire, Bill_Remain 에는 값이 리턴되지 않는다)
			*/
			PacketResultAuth( packet->Packet_Result );
			if ( packet->Packet_Result == 0 )
			{
				if ( gameProcess != NULL )
				{
					gameProcess->Pg_Billing_Auth( packet->Session
												 ,packet->User_ID
												 ,packet->User_IP
												 ,packet->User_Status
												 ,packet->Bill_Method
												 ,packet->Bill_Expire
												 ,packet->Bill_Remain );
				}
			}
			break;
		case pgClient_Move:
			/*-- 설명
				 클라이언트가 게임서버간의 이동을 하였을 경우 빌링서버에 통보
				 서버이동 통지 이벤트의 발생지점은 이동된 서버에서 발생
			*/
			/*-- 게임 서버 작업
				 이동된 서버ID를 패킷의 Game_Server값에 세팅해서 보내준다
			*/
			if ( packet->Packet_Result == 0 )
			{
				if ( gameProcess != NULL )
				{
					gameProcess->Pg_Client_Move( packet->Game_Server, packet->Session, packet->User_ID, packet->User_IP, packet->User_Status, packet->Game_No );
				}
			}
			if ( g_verbose == true )
			{
				printf( "RT->ACCT::Packet_Type=Client_Move(%s, %s)\n", packet->User_ID, packet->User_Status );
			}
			break;
//		case pgServer_Conn: break;	// 송신전용
		default:
			// Error.
			Close( completionKey, perIoContext );
			if ( g_verbose == true )
			{
				printf( "RT->ACCT::Packet_Type=Unknown(%d)\n", packet->Packet_Type );
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
void  cBillConnector::Close(ULONG_PTR completionKey, PerIoContext* perIoContext)
{
	if ( mEndClient == false && g_verbose == true )
	{
		VOID* msgBuf;
		if ( ErrorCode2String( &msgBuf ) )
		{
			printf( "\nBillConnector::Close(%d)-%s\n", GetLastError( ), (char*)msgBuf );
			LocalFree( msgBuf );
		}
	}
	cGameProcess* gameProcess = g_gameSrv->GetGameProcess( );
	if ( gameProcess != NULL )
	{
		gameProcess->Pg_Close( );
	}
	cIocpClient::Close( completionKey, perIoContext );
}

// ServerConn Method - 게임 서버 연결 구분 정보(서버 연결).
//                     항상 빌링 서버와 연결될 때 마다 가장 먼저 전송.
bool cBillConnector::ServerConn(char* Game_Server, long Game_No)
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	_PACKET_GAME* sendMsg   = (_PACKET_GAME*)ioContext->buffer;

	sendMsg->Packet_Type = htonl( pgServer_Conn );
	strcpy( sendMsg->Game_Server, Game_Server );
	sendMsg->Game_No = htonl( Game_No );

	ioContext->offset = sizeof(_PACKET_GAME);
	return SendPost( NULL, ioContext );
}

// ClientMove Method - 게임 서버 연결 구분 정보(사용자 서버간 이동).
//                     이동된 서버ID를 패킷의 Game_Server값에 셋팅해서 전송.
//                     사용자의 정보를 이동된 서버에 보낼 수 없을 경우 이동된 서버에서 게임시작 패킷을 다시 보내
//                     기존 연결을 중복로그인 처리하고 이동된 서버에서 게임을 진행
//                     (중복로그인 처리시 세션키값은 새롭게 생성해줘야 함)
bool cBillConnector::ClientMove(char* Game_Server, char* Session, char* User_ID, char* User_IP, char* User_Status, long Game_No)
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	_PACKET_GAME* sendMsg   = (_PACKET_GAME*)ioContext->buffer;

	sendMsg->Packet_Type = htonl( pgClient_Move );
	strcpy( sendMsg->Game_Server, Game_Server );
	strcpy( sendMsg->Session, Session );
	memcpy( sendMsg->User_CC, COMPANY_CODE, LEN_USER_CC );
	strcpy( sendMsg->User_ID, User_ID );
	strcpy( sendMsg->User_IP, User_IP );
	strcpy( sendMsg->User_Status, User_Status );
	sendMsg->Game_No = htonl( Game_No );

	ioContext->offset = sizeof(_PACKET_GAME);
	return SendPost( NULL, ioContext );
}

// ServerReset Method - 서버 초기화 시 전송 정보(서버 다운 시 전송 정보).
bool cBillConnector::ServerReset(char* Game_Server, long Game_No)
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	_PACKET_GAME* sendMsg   = (_PACKET_GAME*)ioContext->buffer;

	sendMsg->Packet_Type = htonl( pgServer_Reset );
	strcpy( sendMsg->Game_Server, Game_Server );
	sendMsg->Game_No = htonl( Game_No );

	ioContext->offset = sizeof(_PACKET_GAME);
	return SendPost( NULL, ioContext );
}

// BillingAuth Method - 처음 게임 사용자 접속 후 인증을 위한 정보.
bool cBillConnector::BillingAuth(char* Session, char* User_ID, char* User_IP, long Game_No)
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	_PACKET_GAME* sendMsg   = (_PACKET_GAME*)ioContext->buffer;

	sendMsg->Packet_Type = htonl( pgBilling_Auth );
	strcpy( sendMsg->Session, Session );
	memcpy( sendMsg->User_CC, COMPANY_CODE, LEN_USER_CC );
	strcpy( sendMsg->User_ID, User_ID );
	strcpy( sendMsg->User_IP, User_IP );
	sendMsg->Game_No = htonl( Game_No );

	ioContext->offset = sizeof(_PACKET_GAME);
	return SendPost( NULL, ioContext );
}

// GameStart Method - 게임 시작 즉 과금 시작 정보를 알림.
bool cBillConnector::GameStart(char* Game_Server, char* Session, char* User_ID, char* User_IP, long Game_No)
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	_PACKET_GAME* sendMsg   = (_PACKET_GAME*)ioContext->buffer;

	sendMsg->Packet_Type = htonl( pgGame_Start );
	strcpy( sendMsg->Game_Server, Game_Server );
	strcpy( sendMsg->Session, Session );
	memcpy( sendMsg->User_CC, COMPANY_CODE, LEN_USER_CC );
	strcpy( sendMsg->User_ID, User_ID );
	strcpy( sendMsg->User_IP, User_IP );
	sendMsg->Game_No = htonl( Game_No );

	ioContext->offset = sizeof(_PACKET_GAME);
	return SendPost( NULL, ioContext );
}

// UserSync Method - 게임 장기 사용자 확인.
bool cBillConnector::UserSync(long Packet_Result, char* Session, char* User_ID, char* User_IP, char* User_Status)
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	_PACKET_GAME* sendMsg   = (_PACKET_GAME*)ioContext->buffer;

	sendMsg->Packet_Type   = htonl( pgUser_Sync );
	sendMsg->Packet_Result = htonl( Packet_Result );
	strcpy( sendMsg->Session, Session );
	strcpy( sendMsg->User_ID, User_ID );
	strcpy( sendMsg->User_IP, User_IP );
	strcpy( sendMsg->User_Status, User_Status );

	ioContext->offset = sizeof(_PACKET_GAME);
	return SendPost( NULL, ioContext );
}

// GameEnd Method - 게임 종료 즉 과금 종료 요청 정보.
bool cBillConnector::GameEnd(char* Session, char* User_ID, char* User_IP, char* User_Status, long Game_No)
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	_PACKET_GAME* sendMsg   = (_PACKET_GAME*)ioContext->buffer;

	sendMsg->Packet_Type = htonl( pgGame_End );
	strcpy( sendMsg->Session, Session );
	memcpy( sendMsg->User_CC, COMPANY_CODE, LEN_USER_CC );
	strcpy( sendMsg->User_ID, User_ID );
	strcpy( sendMsg->User_IP, User_IP );
	strcpy( sendMsg->User_Status, User_Status );
	sendMsg->Game_No = htonl( Game_No );

	ioContext->offset = sizeof(_PACKET_GAME);
	return SendPost( NULL, ioContext );
}

// ServerAlive Method
bool cBillConnector::ServerAlive(long Game_No)
{
	if ( IsConnect( ) == false )
		return false;

	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );
	_PACKET_GAME* sendMsg   = (_PACKET_GAME*)ioContext->buffer;

	sendMsg->Packet_Type = htonl( pgServer_Alive );
	memcpy( sendMsg->User_CC, COMPANY_CODE, LEN_USER_CC );
	sendMsg->Game_No = htonl( Game_No );

	ioContext->offset = sizeof(_PACKET_GAME);
	return SendPost( NULL, ioContext );
}

// BackendThread Method
DWORD cBillConnector::BackendThread( )
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
				ServerReset( g_gameSrv->GetServerName( ) );
			}
			break;
		}
		if ( IsConnect( ) == false )
		{
			if ( Connect( mAddr ) == true )
			{
				ServerConn( g_gameSrv->GetServerName( ) );
				ServerReset( g_gameSrv->GetServerName( ) );
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
