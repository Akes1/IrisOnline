#include "StdAfx.h"
#include "CallbackStreamPak.h"

#include "ModelInstance.h"
#include "ResourceManager.h"

NiCriticalSection cCallbackStreamPACK::mQueueCriticalSection;

cCallbackStreamPACK::cCallbackStreamPACK()
{
	// 테스트를 통해서 크기를 조절한다.
	mpFileQueue = NiNew NiTPrimitiveQueue<char*>(100);
}

cCallbackStreamPACK::~cCallbackStreamPACK()
{
	ClearQueue();

	SAFE_NIDELETE( mpFileQueue );
}

void cCallbackStreamPACK::AddLoadFile( const char* fileName )
{
	if( ::strlen( fileName ) == 0 ) 
	{
		return;
	}

	/// 큐를 채운다.
	mQueueCriticalSection.Lock();
	{
		char* name = new char[260];
		NiStrcpy( name, 260, fileName );
		mpFileQueue->Add( name );
	}
	mQueueCriticalSection.Unlock();
}

void cCallbackStreamPACK::ClearQueue()
{
	mQueueCriticalSection.Lock();
	{
		char* name = mpFileQueue->Remove();
		while( name )
		{
			delete[]name;
			name = mpFileQueue->Remove();
		}
	}
	mQueueCriticalSection.Unlock();
}

void cCallbackStreamPACK::BackgroundLoadBeginEx()
{
	mQueueCriticalSection.Lock();
	{
		/// 만약 외부에서 직접 접근하도록 수정한다면, 아래 주석 해제하시오..
		//	if( m_eBackgroundLoadStatus != IDLE ) return;
		if( mpFileQueue->IsEmpty() )
		{
			mQueueCriticalSection.Unlock();
			return;
		}

		/// Set m_uiLoad to UINT_MAX so we can determine in BackgroundLoadPoll
		/// whether the background thread has run far enough to set the variables
		/// we use to compute progress.
		m_uiLoad = UINT_MAX;

		/// 게임브리오 내부 처리이다.
		if( m_pkBGLoadProc == NULL )
			m_pkBGLoadProc = NiNew cBackgroundLoadProcedureEx( this );

		/// StreamPack한개에 ..로딩 쓰레드는 한개만으로 한다..
		assert( m_pkThread == NULL );
		if( m_pkThread ) 
		{
			mQueueCriticalSection.Unlock();
			return;
		}

		/// 로딩 쓰레드 생성..
		m_pkThread = NiThread::Create( m_pkBGLoadProc );
		assert(m_pkThread);
		if( !m_pkThread )
		{
			mQueueCriticalSection.Unlock();
			return;
		}

		/// 쓰레드 옵션 셋팅
		m_pkThread->SetPriority(NiThread::BELOW_NORMAL);
		m_pkThread->Resume();

		m_eBackgroundLoadStatus = LOADING;
	}
	mQueueCriticalSection.Unlock();
}

/// 주의!! 이부분은 메인과는 다른 쓰레드이므로 코딩에 매우 주의
void cCallbackStreamPACK::BackgroundLoadEx()
{
	bool isEmpty = false;
	char* fileName = NULL;
	NiNode* n = NULL;

	while( !isEmpty )
	{
		/// 큐를 하나 비운다.
		mQueueCriticalSection.Lock();
		{
			fileName = mpFileQueue->Remove(); 
			if( mpFileQueue->IsEmpty() ) 
			{
				isEmpty = true;
			}
		}
		mQueueCriticalSection.Unlock();

		if( !fileName || strlen(fileName) < 5) 
		{
			/// 로딩하려는 파일명이 NULL인 경우면..
			continue;
		}

		if( RESOURCEMAN->GetObjectByName(fileName) )
		{
			/// 이미 로딩되어 있다면..
			continue;
		}

#ifdef _PAK_LOAD


#else
		//if( NiStream::Load( cResourceManager::ConvertMediaFilename(fileName) ) )
		if( NiStream::Load( fileName ) )
		{
			n = NiDynamicCast( NiNode, NiStream::GetObjectAt(0) );

			/// TODO : 필수 수정 요소!!
			RESOURCEMAN->AddObject( n, fileName, false );

			BackgroundLoadOnExit();
		}
#endif
		/// 잡혀있는 메모리 비운다.
		SAFE_DELETE_ARRAY( fileName );
	}

	m_kSemaphore.Signal();
}

void cCallbackStreamPACK::BackgroundLoadOnExit()
{
	/// 읽어들인 데이터에 대한 메인쓰레드 처리를 위해
	/// 메인쓰레드에 CPU를 할당한다.
	NiSleep( 1 );

}

unsigned int cCallbackStreamPACK::cBackgroundLoadProcedureEx::ThreadProcedure( void* /*pArg*/ )
{
	( (cCallbackStreamPACK*)m_pkStream )->BackgroundLoadEx();

	return 0;
}

/// 주의!! 이부분은 Main Thread상에서의 처리부
/// 절대 로딩쓰레드의 처리부가 아니다.
void cCallbackStreamPACK::Process()
{
	if( m_eBackgroundLoadStatus == IDLE )
	{
		/// 로딩 쓰레드가 휴식상태면 다음 데이터에
		/// 대해 체크한다.
		BackgroundLoadBeginEx();
	}
	else
	{
		/// 로딩 진행형이면
		/// 로딩 쓰레드에 CPU할당..
		Sleep( 1 );

		/// 상태값 갱신
		LoadState loadState;
		BackgroundLoadPoll(&loadState);
	}
}