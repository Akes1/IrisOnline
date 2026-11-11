// Include
#include "agentdemon.h"
#include "serverprotocol.h"

#include "packet.h"

#include "protocol_category.h"
#include "protocol.h"

//#include "commonnetworkmsgstruct.h"

#define SafeCloseHandle(P)  if (P!=NULL) { CloseHandle(P); (P)=NULL; } // 핸들를 안전하게 해제
#define SafeDelete(P)       if (P!=NULL) { delete(P);      (P)=NULL; } // "


// Local definitions
enum REQUEST_CALLBACK_TYPE
{
	MIN_CALLBACK_TYPE = 0x1000,

		RCT_CHANNEL_START,
		RCT_CHANNEL_STOP,

//		RCT_LOGIN_START,
//		RCT_LOGIN_STOP,

	MAX_CALLBACK_TYPE,
};

// Global data

// AgentProcess Constructor.
AgentProcess::AgentProcess(void)
{

}

// ~AgentProcess Destructor.
AgentProcess::~AgentProcess(void)
{
    /// 서버 리스트 Delete
    for( unsigned int i = 0; i < mSrvList.GetSize(); i++ )
    {
        if( mSrvList[i] )
            SafeDelete( mSrvList[i] );
    }
    mSrvList.Clear();
}

// Initialize Method
bool AgentProcess::Initialize(char* addr, unsigned short port, unsigned short numWorkerThread)
{
    /// 서비스에 등록된 IRIS 서버를 Load한다.
    InitSrvList( );

	return cIocpServer::Initialize( addr, port, numWorkerThread );
}

// Shutdown Method
void AgentProcess::Shutdown(DWORD maxWait)
{
	cIocpServer::Shutdown( maxWait );
}

// ErrorLook Method
void AgentProcess::ErrorLook(char* msg, DWORD lastError)
{
	if ( g_verbose )
	{
		void* msgBuf;
		if ( ErrorCode2String( &msgBuf, lastError ) )
		{
			printf( "%s (%d): %s\n", msg, lastError, (char*)msgBuf );
			LocalFree( msgBuf );
		}
	}
}

// DoStartSvc Method
bool AgentProcess::DoStartSvc(char* serviceName)
{
	SC_HANDLE              hManager(NULL);
	SC_HANDLE              hService(NULL);
	bool                   retvalue=false;

	SERVICE_STATUS_PROCESS ssp; 
	DWORD                  dwOldCheckPoint; 
	DWORD                  dwStartTickCount;
	DWORD                  dwWaitTime;
	DWORD                  dwBytesNeeded;

	// Get a handle to the SCM database. 
	hManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( NULL == hManager )
	{
		ErrorLook( "OpenSCManager", GetLastError( ) );
		return retvalue;
	}

	// Get a handle to the service.
	hService = OpenService( hManager, serviceName, SERVICE_ALL_ACCESS );
	if ( hService == NULL )
	{
		ErrorLook( "OpenService", GetLastError( ) );
		CloseServiceHandle( hManager );
		return retvalue;
	}

	// Check the status in case the service is not stopped.
	if ( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
	{
		ErrorLook( "QueryServiceStatusEx", GetLastError( ) );
		goto return_cleanup;
	}

	// Check if the service is already running. It would be possible
	// to stop the service here, but for simplicity this example just returns.
	if ( ssp.dwCurrentState != SERVICE_STOPPED && ssp.dwCurrentState != SERVICE_STOP_PENDING )
	{
		if ( g_verbose )
			printf( "Cannot start the service because it is already running\n" );
		goto return_cleanup;
	}

	// Wait for the service to stop before attempting to start it.
	while ( ssp.dwCurrentState == SERVICE_STOP_PENDING )
	{
		// Save the tick count and initial checkpoint.
		dwStartTickCount = GetTickCount();
		dwOldCheckPoint  = ssp.dwCheckPoint;

		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssp.dwWaitHint / 10;

		if ( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;

		Sleep( dwWaitTime );

		// Check the status until the service is no longer stop pending.
		if ( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
		{
			ErrorLook( "QueryServiceStatusEx", GetLastError( ) );
			goto return_cleanup;
		}
		if ( ssp.dwCheckPoint > dwOldCheckPoint )
		{
			// Continue to wait and check.
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint  = ssp.dwCheckPoint;
		}
		else if ( (GetTickCount( )-dwStartTickCount) > ssp.dwWaitHint )
		{
			if ( g_verbose )
				printf( "Timeout waiting for service to stop\n" );
			goto return_cleanup;
		}
	}

	// Attempt to start the service.
	if ( !StartService( hService, 0, NULL) )
	{
		ErrorLook( "StartService", GetLastError( ) );
		goto return_cleanup;
	}
	else if ( g_verbose )
		printf( "Service start pending...\n" );

	// Check the status until the service is no longer start pending.
	if ( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
	{
		ErrorLook( "QueryServiceStatusEx", GetLastError( ) );
		goto return_cleanup;
	}

	// Save the tick count and initial checkpoint.
	dwStartTickCount = GetTickCount();
	dwOldCheckPoint  = ssp.dwCheckPoint;

	while ( ssp.dwCurrentState == SERVICE_START_PENDING )
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssp.dwWaitHint / 10;

		if ( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;

		Sleep( dwWaitTime );

		// Check the status again.
		if ( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
		{
			ErrorLook( "QueryServiceStatusEx", GetLastError( ) );
			goto return_cleanup;
		}
		if ( ssp.dwCheckPoint > dwOldCheckPoint )
		{
			// Continue to wait and check.
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssp.dwCheckPoint;
		}
		else if ( (GetTickCount( )-dwStartTickCount) > ssp.dwWaitHint )
		{
			if ( g_verbose )
				printf( "No progress made within the wait hint.\n" );
			break;
		}
	}

	// Determine whether the service is running.
	if ( ssp.dwCurrentState == SERVICE_RUNNING )
	{
		retvalue = true;
		if ( g_verbose )
			printf( "Service started successfully.\n" );
	}
	else
	{ 
		retvalue = false;
		if ( g_verbose )
		{
			printf( "Service not started.\n" );
			printf( "  Current State: %d\n", ssp.dwCurrentState  );
			printf( "  Exit Code: %d\n",     ssp.dwWin32ExitCode );
			printf( "  Check Point: %d\n",   ssp.dwCheckPoint    );
			printf( "  Wait Hint: %d\n",     ssp.dwWaitHint      );
		}
	}

return_cleanup:
	CloseServiceHandle( hService );
	CloseServiceHandle( hManager );
	return retvalue;
}

// DoStopSvc Method
bool AgentProcess::DoStopSvc(char* serviceName)
{
	SC_HANDLE              hManager(NULL);
	SC_HANDLE              hService(NULL);
	bool                   retvalue=false;

	SERVICE_STATUS_PROCESS ssp;
	DWORD                  dwStartTime = GetTickCount();
	DWORD                  dwBytesNeeded;
	DWORD                  dwTimeout = 30000; // 30-second time-out

	// Get a handle to the SCM database.
	hManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if ( NULL == hManager )
	{
		ErrorLook( "OpenSCManager", GetLastError( ) );
		return false;
	}

	// Get a handle to the service.
	hService = OpenService( hManager, serviceName, SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS );
	if ( hService == NULL )
	{
		ErrorLook( "OpenService", GetLastError( ) );
		CloseServiceHandle( hManager );
		return false;
	}

	// Make sure the service is not already stopped.
	if ( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
	{
		ErrorLook( "QueryServiceStatusEx", GetLastError( ) );
		goto return_cleanup;
	}
	if ( ssp.dwCurrentState == SERVICE_STOPPED )
	{
		if ( g_verbose )
			printf("Service is already stopped.\n");
		goto return_cleanup;
	}

	// If a stop is pending, wait for it.
	while ( ssp.dwCurrentState == SERVICE_STOP_PENDING ) 
	{
		if ( g_verbose )
			printf("Service stop pending...\n");

		Sleep( ssp.dwWaitHint );

		if ( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
		{
			ErrorLook( "QueryServiceStatusEx", GetLastError( ) );
			goto return_cleanup;
		}
		if ( ssp.dwCurrentState == SERVICE_STOPPED )
		{
			retvalue = true;
			if ( g_verbose )
				printf("Service stopped successfully.\n");
			goto return_cleanup;
		}
		if ( (GetTickCount( ) - dwStartTime) > dwTimeout )
		{
			if ( g_verbose )
				printf("Service stop timed out.\n");
			goto return_cleanup;
		}
	}

	// If the service is running, dependencies must be stopped first.
	StopDependentServices( hManager, hService );

	// Send a stop code to the service.
	if ( !ControlService( hService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS) &ssp ) )
	{
		ErrorLook( "ControlService", GetLastError( ) );
		goto return_cleanup;
	}

	// Wait for the service to stop.
	while ( ssp.dwCurrentState != SERVICE_STOPPED ) 
	{
		Sleep( ssp.dwWaitHint );

		if ( !QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
		{
			ErrorLook( "QueryServiceStatusEx", GetLastError( ) );
			goto return_cleanup;
		}
		if ( ssp.dwCurrentState == SERVICE_STOPPED )
			break;
		if ( (GetTickCount() - dwStartTime) > dwTimeout )
		{
			if ( g_verbose )
				printf( "Wait timed out\n" );
			goto return_cleanup;
		}
	}

	if ( ssp.dwCurrentState == SERVICE_STOPPED )
	{
		retvalue = true;
		if ( g_verbose )
			printf("Service stopped successfully.\n");
		goto return_cleanup;
	}

return_cleanup:
	CloseServiceHandle(hService);
	CloseServiceHandle(hManager);
	return retvalue;
}

// StopDependentServices Method
bool AgentProcess::StopDependentServices(SC_HANDLE hManager, SC_HANDLE hService)
{
	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess;
	SC_HANDLE               hDepService;
	SERVICE_STATUS_PROCESS  ssp;

	DWORD dwStartTime = GetTickCount();
	DWORD dwTimeout   = 30000; // 30-second time-out

	// Pass a zero-length buffer to get the required buffer size.
	if ( EnumDependentServices( hService, SERVICE_ACTIVE, lpDependencies, 0, &dwBytesNeeded, &dwCount ) )
	{
		// If the Enum call succeeds, then there are no dependent services, so do nothing.
		return true;
	} 
	else 
	{
		if ( GetLastError( ) != ERROR_MORE_DATA )
			return false; // Unexpected error

		// Allocate a buffer for the dependencies.
		lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc( GetProcessHeap( ), HEAP_ZERO_MEMORY, dwBytesNeeded );

		if ( !lpDependencies )
			return false;

		__try {
			// Enumerate the dependencies.
			if ( !EnumDependentServices( hService, SERVICE_ACTIVE, lpDependencies, dwBytesNeeded, &dwBytesNeeded, &dwCount ) )
				return false;

			for ( i = 0; i < dwCount; i++ )
			{
				ess = *(lpDependencies + i);

				// Open the service.
				hDepService = OpenService( hManager, ess.lpServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS );
				if ( !hDepService )
					return false;

				__try {
					// Send a stop code.
					if ( !ControlService( hDepService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS) &ssp ) )
						return false;

					// Wait for the service to stop.
					while ( ssp.dwCurrentState != SERVICE_STOPPED )
					{
						Sleep( ssp.dwWaitHint );

						if ( !QueryServiceStatusEx( hDepService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded ) )
							return false;

						if ( ssp.dwCurrentState == SERVICE_STOPPED )
							break;

						if ( GetTickCount() - dwStartTime > dwTimeout )
							return false;
					}
				}
				__finally
				{
					// Always release the service handle.
					CloseServiceHandle( hDepService );
				}
			}
		}
		__finally
		{
			// Always free the enumeration buffer.
			HeapFree( GetProcessHeap(), 0, lpDependencies );
		}
	}
	return true;
}

// PostServerEvent Method
bool AgentProcess::PostServerEvent(LPCTSTR format, ...)
{
	if ( g_agentDemon != NULL )
	{
		AgentUdpSend* agentUdpSend = g_agentDemon->GetAgentUdpSend( );
		if ( agentUdpSend != NULL )
		{
			LPVOID msgBuf = NULL;
			DWORD  bufLen;

			va_list args;

			va_start( args, format );

				bufLen = _vscprintf( format, args ) + 1;
				msgBuf = malloc( bufLen );
				vsprintf( (char*)msgBuf, format, args );

			va_end( args );

			if ( msgBuf != NULL )
			{
				agentUdpSend->PostServerEvent( (char*)msgBuf );
				free( msgBuf );
			}
		}
	}
	return false;
}

// BatchComplete Method
bool AgentProcess::BatchComplete(PerSocketContext* socketContext, void* ptr)
{
	Packet*  packet  = (Packet*)ptr;
	MSGROOT* msgRoot = (MSGROOT*)((char*)ptr + packet->hlen);

	if ( g_packet == true )
	{
		printf( "AgentProcess::BatchComplete:MsgRoot::Category:%d/Protocol:%d\n", msgRoot->Category, msgRoot->Protocol );
	}

	switch( msgRoot->Category )
	{
	case NM_SERVERTOOL:
		switch( msgRoot->Protocol )
        {
        case NM_SERVERTOOL_SERVER_LIST_REQ:
            {
                HANDLE                   handle  = NULL;
                MSG_RES_SRVTOOL_SRVLIST* sendMsg = (MSG_RES_SRVTOOL_SRVLIST*)GetMsgRoot( &handle, 
                                                                                         socketContext, 
                                                                                         NM_SERVERTOOL, 
                                                                                         NM_SERVERTOOL_SERVER_LIST_RES );
                if ( sendMsg )
                {
                    unsigned int i = 0;
                    for( i = 0; i < mSrvList.GetSize(); i++ )
                    {
                        sServiceCtr* pInfo = (sServiceCtr*)mSrvList[i];
                        if( pInfo )
                        {
                            strcpy_s( sendMsg->Servers[i].serviceName,  pInfo->serviceName  );
                            strcpy_s( sendMsg->Servers[i].ip,           pInfo->ip           );
                            sendMsg->Servers[i].state = (BYTE)pInfo->status;
                            
                        }
                        ++(sendMsg->rowCount);
                    }

                    SendMsgRoot( handle, sendMsg->GetMsgLength() );
                }
            }
            break;
		case NM_SERVERTOOL_AGENT_ON_REQ:
			{
                MSG_REQ_SRVTOOL_AGENT_ONOFF* recvMsg = (MSG_REQ_SRVTOOL_AGENT_ONOFF*)msgRoot;

				PerIoContext* ioContext = mIoContextPool->GetIoContext( socketContext->socket, IOCP_REQUEST_CALLBACK );
					sServiceCtr* serviceCtr = (sServiceCtr*)ioContext->buffer;

                    strcpy( serviceCtr->serviceName, recvMsg->serviceName );

					ioContext->offset = sizeof(sServiceCtr);
					ioContext->iParam = RCT_CHANNEL_START;
				QueueRequest( (ULONG_PTR)socketContext, (LPOVERLAPPED)ioContext, ioContext->offset );
			}
			break;
		case NM_SERVERTOOL_AGENT_OFF_REQ:
			{
                MSG_REQ_SRVTOOL_AGENT_ONOFF* recvMsg = (MSG_REQ_SRVTOOL_AGENT_ONOFF*)msgRoot;

				PerIoContext* ioContext = mIoContextPool->GetIoContext( socketContext->socket, IOCP_REQUEST_CALLBACK );
					sServiceCtr* serviceCtr = (sServiceCtr*)ioContext->buffer;

                    strcpy( serviceCtr->serviceName, recvMsg->serviceName );

					ioContext->offset = sizeof(sServiceCtr);
					ioContext->iParam = RCT_CHANNEL_STOP;
				QueueRequest( (ULONG_PTR)socketContext, (LPOVERLAPPED)ioContext, ioContext->offset );
			}
			break;
        case NM_SERVERTOOL_SERVER_STATUS_REQ:
            {
                MSG_REQ_SRVTOOL_STATUS* recvMsg = (MSG_REQ_SRVTOOL_STATUS*)msgRoot;

                for( unsigned int i = 0; i < mSrvList.GetSize(); i++ )
                {
                    sServiceCtr* pServer = (sServiceCtr*)mSrvList[i];
                    if( pServer && strcmp( pServer->serviceName, recvMsg->serviceName ) == 0 )
                    {
                        UpdateServiceCtrl( pServer );

                        HANDLE                   handle  = NULL;
                        MSG_RES_SRVTOOL_STATUS* sendMsg = (MSG_RES_SRVTOOL_STATUS*)GetMsgRoot( &handle, socketContext, NM_SERVERTOOL, NM_SERVERTOOL_SERVER_STATUS_RES );

                        strcpy_s( sendMsg->serviceName, pServer->serviceName );
                        sendMsg->status = pServer->status;

                        SendMsgRoot( handle, sizeof(MSG_RES_SRVTOOL_STATUS) );
                    }
                }
            }
            break;
		}
		return true;

	default:
		return false;
	}
	return true;
}

// RecvComplete Method
bool AgentProcess::RecvComplete(PerSocketContext* socketContext, PerIoContext* ioContext, DWORD bytesTransferred)
{
	cCSLock lock( &mCs );
	char*   recvBuf    = (socketContext->buffer + (socketContext->offset + socketContext->InternalHigh));
	DWORD   recvBufLen = (socketContext->length - (socketContext->offset + socketContext->InternalHigh));

	// 오프셋의 크기를 누적한다.
	ioContext->offset += bytesTransferred;

	DWORD length = ioContext->offset; // 수신된 누적버퍼의 크기
	DWORD offset = 0;                 // 수신된 누적버퍼의 오프셋

	while ( length >= PHLen )
	{
		Packet* packet    = (Packet*)(ioContext->buffer + offset);
		DWORD   packetLen = packet->tlen;

		// 헤더길이가 정의된 길이와 다를 경우 오류 처리한다.
		if ( packet->hlen != PHLen )
		{
			Close( socketContext, ioContext );
			return false;
		}

		// 데이터의 길이가 최소 단위보다 작을 경우 오류 처리해야 한다.
		if ( packetLen < PHLen )
		{
			Close( socketContext, ioContext );
			return false;
		}

		// 데이터의 길이가 완전할때까지 이어 받기를 해야 한다.
		if ( packetLen > length )
			break;

		// Sequence Number & Acknowledgment Number 검사.
		if ( packet->seq != (++socketContext->ack) )
		{
			Close( socketContext, ioContext );
			return false;
		}

		if ( g_packet == true )
		{
			// 패킷헤더(Packet Header) 정보출력 - 버전/헤더길이/서비스 종류/전체길이.
			printf( "AgentProcess::RecvComplete::VER:%02x/HLEN:%02x/TOS:%02x/TLEN:%04x/SEQ:%08x\n",
					packet->ver,
					packet->hlen,
					packet->tos,
					packet->tlen,
					packet->seq );
		}

		// Type of service를 검사한다.
		switch ( packet->tos )
		{
		case TOS_SERVERTOOL:
			// Receive Data 복사.
			if ( packetLen <= recvBufLen )
			{
				memcpy( recvBuf, packet, packetLen );

				recvBuf                     += packetLen;
				recvBufLen                  -= packetLen;
				socketContext->InternalHigh += packetLen;
			}
			else
			{
				// Buffer Overflow (버퍼 오버플로).
				Close( socketContext, ioContext );
				return false;
			}
			break;
		case TOS_TTL:
			socketContext->timeToLive = (GetTickCount( ) + MAX_TTL);     // 최대 03분
			break;
		default:
			// Error.
			Close( socketContext, ioContext );
			return false;
		}

		// 처리후 Offset 및 Length를 다시 계산한다.
		length -= packetLen;
		offset += packetLen;
	}

	// 이어 받기를 위해 데이터를 보관한한다.
	if ( offset > 0 )
	{
		PerIoContext* recvIoContext = mIoContextPool->GetIoContext( socketContext->socket, IOCP_REQUEST_READ );

		if ( length > 0 )
		{
			memcpy( recvIoContext->buffer, (ioContext->buffer + offset), length );
			recvIoContext->offset = length;
		}

		mIoContextPool->ReleaseIoContext( ioContext );
		ioContext = recvIoContext;
	}

	// 수신을 위해 I/O Context를 준비한다.
	if ( RecvPost( ioContext ) == false )
	{
		Close( socketContext );
		return false;
	}
	return true;
}

// CallbackComplete Method
bool AgentProcess::CallbackComplete(PerSocketContext* socketContext, PerIoContext* ioContext, DWORD bytesTransferred)
{
	switch ( ioContext->iParam )
	{
	case RCT_CHANNEL_START:
		if ( socketContext->socket == ioContext->socket )
		{
			sServiceCtr* serviceCtr = (sServiceCtr*)ioContext->buffer;

			PostServerEvent( "%s - Attempt to start the service.", serviceCtr->serviceName );

			if ( DoStartSvc( serviceCtr->serviceName ) )
			{
				PostServerEvent( "%s - The start of the server has been completed.", serviceCtr->serviceName );
			}
			else
			{
				PostServerEvent( "%s - The start of the server has failed.", serviceCtr->serviceName );
			}
		}
		break;
	case RCT_CHANNEL_STOP:
		if ( socketContext->socket == ioContext->socket )
		{
			sServiceCtr* serviceCtr = (sServiceCtr*)ioContext->buffer;

			PostServerEvent( "%s - Send a stop code to the service.", serviceCtr->serviceName );

			if ( DoStopSvc( serviceCtr->serviceName ) )
			{
				PostServerEvent( "%s - The stop of the server has been completed.", serviceCtr->serviceName );
			}
			else
			{
				PostServerEvent( "%s - The stop of the server has failed.", serviceCtr->serviceName );
			}
		}
		break;
	}
	mIoContextPool->ReleaseIoContext( ioContext );
	return true;
}

// BackendThread Method
DWORD AgentProcess::BackendThread( )
{
	DWORD currTickCount = 0;
	while ( true )
	{
		currTickCount = GetTickCount( );

		// 클라이언트 종료, Thread를 나간다.
		if ( mEndServer == true )
			break; // while ( true )

		// 클라이언트의 소켓 관리.
		if ( mSocketContextPool != NULL )
		{
			cCSLock           lock( &mCs );
			PerSocketContext* socketContext = mSocketContextPool->GetPagedPoolUsage( );
			PerSocketContext* next          = NULL;
			char*             recvBuf       = NULL;

			while ( socketContext != NULL )
			{
				next    = socketContext->next;
				recvBuf = (socketContext->buffer + socketContext->offset);

				// Receive 데이터 처리를 위한 코드.
				if ( socketContext->InternalHigh )
				{
					u_long& offset = socketContext->Internal;
					u_long& length = socketContext->InternalHigh;

					if ( socketContext->status.connectionDead == false )
					{
						do {
							Packet* packet = (Packet*)(recvBuf + socketContext->Internal);
								if ( BatchComplete( socketContext, packet ) == false )
								{
									Close( socketContext );
									break;
								}
							offset += packet->tlen;
						} while ( offset < length );
					}

					memset( recvBuf, 0, socketContext->InternalHigh );
					socketContext->Internal     = 0;
					socketContext->InternalHigh = 0;
				}

				// Close Socket On.
				if ( socketContext->status.connectionDead == false )
				{
					// Time To Live.
					if ( socketContext->timeToLive < currTickCount )
						Close( socketContext );
				}
				else
					socketContext->status.closeSocket = 1;

				// SocketContext 회수.
				if ( socketContext->status.closeSocket )
				{
					if ( g_verbose == true )
					{
						char* inetnum = inet_ntoa( socketContext->addr.sin_addr );

						printf( "AgentProcess::BackendThread\n" );
						printf( "\t[SOCKET:%d] %s (In) the connection has been terminated.\n", socketContext->socket, inetnum );
					}

					mSocketContextPool->ReleasePerSocketContext( socketContext );
				}

				socketContext = next;
			} // while ( socketContext != NULL )
		}

		// I/O Context 처리.
		IoContextPresent( );

		// FPS 처리.
		BackendThreadFps( currTickCount );
	} // while ( true )
	return 0L;
}

// BackendThreadFps Method
void AgentProcess::BackendThreadFps(DWORD currentTick)
{
	// 지연시간 약16(ms) - CPU 과부화 방지 & 내부 동기화.
	DWORD elapsedTick = GetTickCount( ) - currentTick;

	if ( elapsedTick < 16 )
	{
		Sleep( (16 - elapsedTick) );
	}
}

MSGROOT* AgentProcess::GetMsgRoot( void** handle, PerSocketContext* perSocketContext, char category, char protocol )
{
	PerIoContext*     perIoContext     = NULL;
	Packet*           packet           = NULL;
	MSGROOT*          msgRoot          = NULL;

	perIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_WRITE );
	if ( perIoContext != NULL )
	{
		packet       = (Packet*)perIoContext->buffer;

		packet->ver  = PHVer;
		packet->hlen = PHLen;
		packet->tos  = TOS_GAME;
		packet->tlen = packet->hlen;
		packet->seq  = (++perSocketContext->seq);

		(*handle) = perIoContext;
		msgRoot   = (MSGROOT*)(perIoContext->buffer + packet->hlen);
			msgRoot->Category = category;
			msgRoot->Protocol = protocol;
		return msgRoot;
	}
	else
	{
		return NULL;
	}
}

// SendMsgRoot Method
bool AgentProcess::SendMsgRoot(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	Packet*       packet       = (Packet*)perIoContext->buffer;

	packet->tlen = packet->hlen + (u_short)length;

	perIoContext->offset = packet->tlen;
	return SendPost( perIoContext );
}

void AgentProcess::AddServieCtrl( const char* name, const char* ip )
{
    sServiceCtr* p     = new sServiceCtr;
  //  p->IsService    = false;
    strcpy_s( p->serviceName, name );
    strcpy_s( p->ip,          ip   );

    mSrvList.PushBack( p );
}

void AgentProcess::InitSrvList( )
{
    SC_HANDLE hManager          = NULL;
    PUCHAR    pBuf              = NULL;
    ULONG     dwBufSize         = 0x00;
    ULONG     dwBufNeed         = 0x00;
    ULONG     dwNumberOfService = 0x00;
    LPENUM_SERVICE_STATUS_PROCESS ssp = NULL;

    hManager = OpenSCManager( NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT );
    if (hManager != NULL)
    {
        EnumServicesStatusEx( hManager
            ,SC_ENUM_PROCESS_INFO
            ,SERVICE_WIN32
            ,SERVICE_STATE_ALL
            ,NULL
            ,dwBufSize
            ,&dwBufNeed
            ,&dwNumberOfService
            ,NULL
            ,NULL );

        //if ( dwBufNeed > 0 )
        if( GetLastError() == ERROR_MORE_DATA )
        {
            dwBufSize = dwBufNeed +1;
            pBuf      = (PUCHAR)malloc( dwBufSize );

            EnumServicesStatusEx( hManager
                ,SC_ENUM_PROCESS_INFO
                ,SERVICE_WIN32			// SERVICE_DRIVER
                ,SERVICE_STATE_ALL
                ,pBuf
                ,dwBufSize
                ,&dwBufNeed
                ,&dwNumberOfService
                ,NULL
                ,NULL );

            ssp = (LPENUM_SERVICE_STATUS_PROCESS)pBuf;
            for ( ULONG i=0; i < dwNumberOfService ; i++ )
            {
                if ( strnicmp( ssp[i].lpDisplayName, "IRIS ON LINE", strlen("IRIS ON LINE") ) == 0 )
                {
                    if( strcmp( ssp[i].lpServiceName, g_serviceName ) == 0 )
                        continue;
                    
                    for( unsigned int count = 0; count < mSrvList.GetSize(); count++ )
                    {
                        sServiceCtr* p = (sServiceCtr*)mSrvList[count];

                        if( p && strcmp( p->serviceName, ssp[i].lpServiceName ) == 0 )
                            p->status = ssp[i].ServiceStatusProcess.dwCurrentState;
                    }

                    // 참조: Service State -- for CurrentState
                    if ( g_verbose )
                    {
                        printf( "Display Name : %s \n",   ssp[i].lpDisplayName );
                        printf( "Service Name : %s \n",   ssp[i].lpServiceName );
                        printf( "Process Id   : %04x %04x %04x \n"
                            ,ssp[i].ServiceStatusProcess.dwServiceType
                            ,ssp[i].ServiceStatusProcess.dwProcessId
                            ,ssp[i].ServiceStatusProcess.dwCurrentState );
                    }
                }
            }

            if( pBuf )
                free( pBuf );
        }
        if( hManager )
            CloseServiceHandle( hManager );
    }
}

void AgentProcess::UpdateServiceCtrl( sServiceCtr* pSrv )
{
    SC_HANDLE       hManager    = NULL;
    SC_HANDLE       hService    = NULL;
    SERVICE_STATUS  scStatus;
    DWORD           lastError   = 0;

    hManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS  | SC_MANAGER_CONNECT );
    if ( hManager != NULL )
    {
        hService = OpenService( hManager, pSrv->serviceName, SERVICE_QUERY_STATUS );
        if( hService )
        {
            if( QueryServiceStatus( hService, &scStatus ) )
            {
                if( pSrv->status != scStatus.dwCurrentState )
                {
                    pSrv->status = scStatus.dwCurrentState;

                    if ( g_verbose )
                    {
                        printf( "\n\n[Change State]\n" );
                        printf( "Service Name : %s \n", pSrv->serviceName   );
                        printf( "CurrentState : %04x\n",pSrv->status        );
                    }
                }
            }
            else
                lastError = GetLastError( );

            CloseServiceHandle( hService );
        } // hService
    }
    else
        lastError = GetLastError( );

    if( hManager )
        CloseServiceHandle( hManager );

    if( lastError != 0 )
    {
        VOID* msgBuf;
        if ( ErrorCode2String( &msgBuf ) )
        {
            printf( "AgentProcess::UpdateServiceCtrl (%d): %s\n", lastError, (char*)msgBuf );
            LocalFree( msgBuf );
        }
    }
}
