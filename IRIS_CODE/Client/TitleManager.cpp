#include "stdafx.h"
#include "TitleManager.h"
#include "GameResourceManager.h"
#include "GameUIManager.h"
#include "ChatManager.h"
#include "TitleWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "ItemManager.h"

cTitleManager* cTitleManager::mpSingleton = 0;

cTitleManager::cTitleManager()
: mRequestTitleIndex(0)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	ClearQuickList();
}

cTitleManager::~cTitleManager()
{
	mpSingleton = 0;

	cTitleDefineMap::cIterator i = mTitleDefineMap.Begin();
	cTitleDefineMap::cIterator end = mTitleDefineMap.End();
	
	for( ; i != end; ++i )
	{
		delete (cTitleDefine*)(i->mSecond);
	}
	mTitleDefineMap.Clear();
}

bool cTitleManager::Init()
{
	if( Load( "./Script/Resource/Title_Define.txt" ) == false )
	{
		assert( 0 && "failed to load title define" );
		return false;
	}
	return true;
}

/// 스크립트 로드
bool cTitleManager::Load( const cString& pathname )
{
	cFileLoader loader;

	if( loader.Open( pathname, true ) == false )
	{
		assert( 0 && "failed to load title define" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathname.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long index = str.ToInt();

		/// 이름인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long nameIdx = str.ToInt();

		/// 내용인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long textIdx = str.ToInt();

		/// 효과인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long influenceIdx = str.ToInt();

		/// 색상 R
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned char colorR = (unsigned char)str.ToInt();

		/// 색상 G
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned char colorG = (unsigned char)str.ToInt();

		/// 색상 B
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned char colorB = (unsigned char)str.ToInt();

		/// 타이틀 정의 생성
		cTitleDefine* define = new cTitleDefine;
		define->mName = GAMERESOURCEMAN->GetHeroTitle( nameIdx );
		define->mDescription = GAMERESOURCEMAN->GetHeroTitle( textIdx );
		define->mInfluenceIndex = influenceIdx;

		/// 변환 ( R,G,B 순서로 )
		unsigned long color = (unsigned long)( (DWORD)((BYTE)colorR << 16) | (WORD)((BYTE)colorG << 8) | (BYTE)colorB );
		define->mColor = color | 0xff000000;

		if( mTitleDefineMap.Insert( index, define ) == false )
		{
			assert( 0 && "failed to add title define" );
			return false;
		}
	}
	return true;
}

/// 게임스테이지 open
void cTitleManager::Open()
{
	/// 로딩된 정보를 바탕으로 UI 세팅
	SortTitleList();
}

void cTitleManager::Close()
{
	mHaveTitleSet.Clear();

	cTitleWindow* win = (cTitleWindow*)GAMEUI->GetTitleWindow();
	if( win )
		win->Clear();
}

/// 정렬후 UI에 삽입
void cTitleManager::SortTitleList()
{
	cTitleWindow* win = (cTitleWindow*)GAMEUI->GetTitleWindow();
	if( win )
	{
		win->Clear();

		typedef tList<unsigned long> cSortList;
		cSortList list;

		/// 기본으로 없음 출력
		win->AddTitle( 0 );

		cHaveTitleSet::cIterator i = mHaveTitleSet.Begin();
		cHaveTitleSet::cIterator end = mHaveTitleSet.End();
		for( ; i != end; ++i )
		{
			unsigned long titleIdx = (unsigned long)(*i);
			list.PushBack( titleIdx );
		}

		if( list.GetSize() > 1 )
			list.Sort();

		///
		cSortList::cIterator li = list.Begin();
		cSortList::cIterator lend = list.End();
		for( ; li != lend; ++li )
		{
			unsigned long titleIdx = (unsigned long)(*li);
			win->AddTitle( titleIdx );
		}

		for( int i = 0; i < TITLE_QUICK_LIST_COUNT; i++ )
		{
			if( mHaveQuickTitleArr[i] != -1 )
			{
				win->AddTitle( mHaveQuickTitleArr[i], true );
			}
		}
	}
	else
	{
		assert(0);
		return;
	}
}

void cTitleManager::TitleSet( unsigned long titleIdx )
{
	cUIWindow* titleWin = GAMEUI->GetTitleWindow();
	if( titleWin )
	{
		/// 락 검사
		if( titleWin->IsNetLock() == true )
			return;

		/// 이미 같은 호칭 세팅중인지 검사
		if( HERO->GetTitleIndex() == titleIdx )
			return;

		mRequestTitleIndex = titleIdx;

		MSG_REQ_PLAYER_TITLE_SET msg;
		::memset( &msg, 0, sizeof(msg) );

		msg.Category = NM_PLAYER;
		msg.Protocol = NM_PLAYER_TITLE_SET_REQ;
		msg.mTitleIndex = titleIdx;

		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		/// 락 걸기
		titleWin->NetLock();
	}
}

void cTitleManager::ClearList()
{
	mHaveTitleSet.Clear();
}

void cTitleManager::ClearQuickList()
{
	for( int i = 0; i < TITLE_QUICK_LIST_COUNT; i++ )
		mHaveQuickTitleArr[i] = -1;
}

void cTitleManager::AddHaveTitle( unsigned long titleIdx )
{
	if( mHaveTitleSet.Insert( titleIdx ) == false )
	{
		assert(0);
	}
}

void cTitleManager::AddHaveQuickTitle( unsigned char idx, long nTitleIdx )
{
	if( idx >= TITLE_QUICK_LIST_COUNT )
	{
		assert(0);
		return;
	}

	for( int i = 0; i < TITLE_QUICK_LIST_COUNT; i++ )
	{
		if(  mHaveQuickTitleArr[ i ] == nTitleIdx )
			return;
	}

	mHaveQuickTitleArr[ idx ] = nTitleIdx;
}

bool cTitleManager::IsHaveTitle( unsigned long titleIdx )
{
	cHaveTitleSet::cIterator i = mHaveTitleSet.Find( titleIdx );
	return ( i != mHaveTitleSet.End() );
}

void cTitleManager::SetHaveQuickTitle( long *pTitleIdxArr )
{
	for( unsigned int i = 0; i < TITLE_QUICK_LIST_COUNT; i++, pTitleIdxArr++ )
	{
		mHaveQuickTitleArr[ i ] = *pTitleIdxArr;
	}
}

void cTitleManager::GetQuickTitle( long* pListArr )
{
	for( unsigned int i = 0; i < TITLE_QUICK_LIST_COUNT; i++, pListArr++ )
	{
		*pListArr = (long)mHaveQuickTitleArr[ i ];
	}
}

void cTitleManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	switch( tempMsg->Protocol )
	{
	case NM_PLAYER_TITLE_LIST_RES:
		{
			MSG_RES_PLAYER_TITLELIST* pmsg = (MSG_RES_PLAYER_TITLELIST*)msg;

			/// 초기화
			ClearList();

			/// 저장
			for( int i = 0; i < pmsg->rowCount; ++i )
				AddHaveTitle( pmsg->mTitleIndex[i] );

			/// UI 갱신
			if( pmsg->rowCount > 0 )
			{
				SortTitleList();	
			}
		}
		break;
	case NM_PLAYER_TITLE_ADD_RES:
		{
			///
			MSG_RES_PLAYER_TITLEADD* pmsg = (MSG_RES_PLAYER_TITLEADD*)msg;

			/// 소유 호칭목록에 추가
			AddHaveTitle( pmsg->mTitleIndex );

			/// 소팅 후 UI 세팅
			SortTitleList();

			/// 호칭 획득 알림
			cTitleDefine* define = GetTitleDefine( pmsg->mTitleIndex );
			if( define )
			{
				cStringT str;
				str.Format(  GAMERESOURCEMAN->GetGameText( 82 ), define->mName );
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)str.Cstr() );
			}

			cTitleWindow* win = (cTitleWindow*)GAMEUI->GetTitleWindow();
			if( win )
				win->Show();

			ITEMMAN->CheckItemLimitAll();
		}
		break;
	case NM_PLAYER_TITLE_SET_RES:
		{
			/// 호칭변경 결과	
			MSG_RES_PLAYER_TITLE_SET* pmsg = (MSG_RES_PLAYER_TITLE_SET*)msg;

			/// 호칭변경
			if( pmsg->ErrorCode == ERROR_PLAYER_TITLE_SET_SUCCESS )
			{
				/// hero 세팅
				HERO->SetTitle( mRequestTitleIndex );

				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 83 ) );
			}
			else
			{
				/// 실패
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 84 ) );
			}

			cUIWindow* titleWin = GAMEUI->GetTitleWindow();
			if( titleWin )
				titleWin->NetUnLock();
		}
		break;
	case NM_PLAYER_TITLE_SET_SYN:
		{
			/// 다른유저 호칭변경
			MSG_SYN_PLAYER_TITLE_SET* pmsg = (MSG_SYN_PLAYER_TITLE_SET*)msg;

			cPlayer* player = OBJECTMAN->GetPlayer( pmsg->mCharacterIndex );
			if( player )
			{
				player->SetTitle( pmsg->mTitleIndex );
			}
			else
			{
				assert(0);
			}
		}
		break;
	}
}

cTitleDefine* cTitleManager::GetTitleDefine( unsigned long titleIdx )
{
	return (cTitleDefine*)mTitleDefineMap.GetAt( titleIdx );
}
