#include "StdAfx.h"
#include "ServerWindow.h"
#include "StageManager.h"

#include "ListBox.h"
#include "GameResourceManager.h"
#include "Application.h"

cServerWindow::cServerWindow()
: mpServerList(0)
, mpConnect(0)
, mpCancel(0)
, mpExit(0)
{
	for( int i = 0; i < MAXPAGE_NUM; ++i )
		mpRadioNode[i] = 0;

	mCheckRect.Set( 0, 0, 0, 0 );
}

cServerWindow::~cServerWindow()
{
	Clear();
}

void cServerWindow::Open()
{
	Hide();
}

void cServerWindow::Clear()
{
	if( mpServerList )
		mpServerList->Clear();

	for( unsigned int i = 0; i < mServerData.GetSize(); ++i )
	{
		delete (sServerData*)mServerData[i];
	}
	mServerData.Clear();

	for( int i = 0 ; i < MAXPAGE_NUM; ++i )
	{
		if( mpRadioNode[i] )
			mpRadioNode[i]->Hide();
	}
}

void cServerWindow::OnShow()
{
	cUIWindow::OnShow();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = GetRelativeRect().mTop;
	SetRelativePos( cUIPos( x, y) );

	if( mpCancel && mpExit )
	{
		// 웹런쳐를 통한 로그인이면
		if( THEAPP->GetAuthCode() != -1 )
		{
			mpCancel->Hide();
			mpExit->Show();
		}
		else
		{
			mpCancel->Show();
			mpExit->Hide();
		}
	}
}

/// 주의사항 : 
bool cServerWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;
	
	/// 서버리스트 박스 저장
	mpServerList = (cListBox*)GetChild( eUIID_SERVER_SERVERLIST );
	mpConnect = GetChild( eUIID_SERVER_CONNECT_BUTTON );
	mpCancel = GetChild( eUIID_SERVER_CANCEL_BUTTON );
	mpExit = GetChild( eUIID_SERVER_EXIT );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 정렬영역
	cUINodeSkin* pRectSkin = pSkin->GetNodeSkin( "Server_ButtonRect" );
	if( pRectSkin )
	{
		mCheckRect.mLeft = pRectSkin->mSkinInfo->mX;
		mCheckRect.mTop = pRectSkin->mSkinInfo->mY;
		mCheckRect.mRight = mCheckRect.mLeft + pRectSkin->mSkinInfo->mWidth;
		mCheckRect.mBottom = mCheckRect.mTop + pRectSkin->mSkinInfo->mHeight;
	}

	for( int i = 0; i < MAXPAGE_NUM; ++i )
	{
		mpRadioNode[i] = GetChild( eUIID_SERVER_SERVERPAGE1 + i );
		if( mpRadioNode[i] )
			mpRadioNode[i]->Hide();
	}

	return true;
}

void cServerWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = GetRelativeRect().mTop;
	SetRelativePos( cUIPos( x, y) );

	UpdateButton();
}

///
void cServerWindow::OnRadioPressed( cUINode*, unsigned int id )
{
	int row = 0;
	switch( id )
	{
	case eUIID_SERVER_SERVERPAGE1:	row = 0; break;
	case eUIID_SERVER_SERVERPAGE2:	row = 10; break;
	case eUIID_SERVER_SERVERPAGE3:	row = 20; break;
	case eUIID_SERVER_SERVERPAGE4:	row = 30; break;
	case eUIID_SERVER_SERVERPAGE5:	row = 40; break;
	default: assert(0); return;
	}	

	if( mpServerList )
		mpServerList->ChangeTopIndex( row );
}

/// 주의사항 : 
void cServerWindow::OnCommand( cUINode*, unsigned int id )
{
	switch(id)
	{
	case eUIID_SERVER_CONNECT_BUTTON:
		{
			SendConnectServer();
		}
		break;
	case eUIID_SERVER_CANCEL_BUTTON:
		{	
			STAGEMAN->ChangeStage( eStage_Login );
		}
		break;
	case eUIID_SERVER_EXIT:
		{
			/// 프로그램 종료
			::SendMessage( THEAPP->GetHWND(), WM_CLOSE, 0, 0 );
			::PostQuitMessage(0);
		}
		break;
	}
}

void cServerWindow::SetEnabled( bool enabled )
{
	cUIWindow::SetEnabled( enabled );
}

/// 주의사항 : 
void cServerWindow::OnListBoxChanged( cUINode*, unsigned int id )
{
	if( id == eUIID_SERVER_SERVERLIST )
	{
		bool enable = ( GetSelectedServerStatus() == 0 ) ? false : true;

		if( mpConnect )
			mpConnect->SetEnabled( enable );
	}
}

void cServerWindow::OnListBoxSelected( cUINode*, unsigned int id )
{
	if( id == eUIID_SERVER_SERVERLIST )
	{
		SendConnectServer();
	}
}

// 서버 접속 요청
void cServerWindow::SendConnectServer()
{
	if(  NETWORK->IsConnected( ) == false ) 
		return;

	if( GetSelectedServerStatus() <= 0 )
		return;

	BYTE age = GetSelectedServerAge();

	/// 접속
	MSG_REQ_COMEIN_GAME msg;
	memset( &msg, 0, sizeof(msg) );

	msg.Category     = NM_USER;
	msg.Protocol     = NM_USER_COME_IN_GAME_REQ;
	msg.NumOfServer	 = GetSelectedServerParam();
	msg.NumOfChannel = 0;
	msg.Tutorial	 = 0;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	bool adult = ( age > 0 ) ? true : false;
	STAGEMAN->SetAdultServer( adult );

	STAGEMAN->SetServerIdx( GetSelectedServerParam() );

	/// 연결 대기창 등록
	UIMAN->CreateMsgBox( eStage_Server, eMSGBOX_MODAL,
		eMBEVENT_CONNECT_GAMESRV, eSKIN_NONE,
		GAMERESOURCEMAN->GetGameText( 0 ), // 캐릭터 목록 전송
		GAMERESOURCEMAN->GetGameText( 2 ) // 알림
		);

	SetEnabled( false );
}


/// 주의사항 : 서버리스트를 새로 삽입한다 
void cServerWindow::AddServer( long param, int status, LPCTSTR servername, BYTE age )
{
	/// 서버 정보 세팅 
	sServerData* data = new sServerData;
	data->mParam = param;
	data->mStatus = status;
	data->mServerName = servername;
	data->mAge = age;
	mServerData.PushBack( data );
}

// update
void cServerWindow::UpdateServer( long selServerNo )
{
	unsigned int size = mServerData.GetSize();

	// 랜덤
	if( size > 1 )
	{
		int s = 0;
		sServerData temp;

		for( unsigned int i = 0; i < size; ++i )
		{
			s = (int)(rand( ) % size);

			sServerData* pi = (sServerData*)mServerData[ i ];
			sServerData* ps = (sServerData*)mServerData[ s ];

			temp	= *pi;
			*pi		= *ps;
			*ps		= temp;
		}
	}

	// UI 셋팅
	if( mpServerList )
	{
		unsigned int selRow = 0;
		for( unsigned int i = 0; i < size; ++i )
		{
			sServerData* p = (sServerData*)mServerData[ i ];
			if( p )
			{
				unsigned int textId = 0;
				unsigned long color = mDefaultColor;
				switch ( p->mStatus )
				{
				case 4: textId = 700; color = COLOR_RED;	break;	/// 혼잡
				case 3: textId = 701; color = COLOR_ORANGE;	break;	/// 양호
				case 2: textId = 702; color = COLOR_GREEN;	break;	/// 원활
				case 1:	textId = 703; color = COLOR_GRAY;	break;	/// 준비중
				default: textId = 703; color = COLOR_GRAY;	break;	/// 준비중
				}

				int row = mpServerList->AddRow( (LPCTSTR)p->mServerName.Cstr(), mDefaultColor, p );
				if( row > -1 )
					mpServerList->AddColume( row, UIMAN->GetUIText( textId ), color );
				else
					assert(0);

				if( selServerNo == p->mParam )
					selRow = i;
			}
		}

		//
		mpServerList->ChangeSelectRow( selRow );
	}
	else
		assert(0);
}

void cServerWindow::UpdateButton()
{
	if( !mpServerList )
	{
		assert(0);
		return;
	}

	/// 버튼 보이고 안보이고 하기
	int total = mpServerList->GetRowCount() - 1;
	int page = ( total / 10 );

	for( int i = 0; i < MAXPAGE_NUM; ++i )
	{
		if( mpRadioNode[i] )
		{
			if( i <= page )
				mpRadioNode[i]->Show();
			else
				mpRadioNode[i]->Hide();
		}
	}

	/// 위치 조정
	if( mpRadioNode[0] )
	{
		int w = mpRadioNode[0]->GetAbsoluteRect().GetWidth();	
		int x = mCheckRect.mLeft + (int)((mCheckRect.GetWidth() - (w*(page+1))) * 0.5f);
		for( int i = 0; i <= page; ++i )
		{
			cUIPos pos;
			pos.mX = x + (w*i);
			pos.mY = mpRadioNode[i]->GetRelativeRect().mTop;
			mpRadioNode[i]->SetRelativePos( pos );
		}

		mpRadioNode[0]->SetPress( true );
	}
}

long cServerWindow::GetSelectedServerParam()
{
	sServerData* pData = (sServerData*)mpServerList->GetExtraData( mpServerList->GetSelectIndex() );
	if( pData )
		return pData->mParam;
	return -1;
}

///
int cServerWindow::GetSelectedServerStatus()
{
	sServerData* pData = (sServerData*)mpServerList->GetExtraData( mpServerList->GetSelectIndex() );
	if( pData )
		return pData->mStatus;
	return -1;
}

BYTE cServerWindow::GetSelectedServerAge()
{
	sServerData* pData = (sServerData*)mpServerList->GetExtraData( mpServerList->GetSelectIndex() );
	if( pData )
		return pData->mAge;
	return 0;
}

///
void cServerWindow::SetEnableConnectBtn( bool enable )
{
	if( mpConnect )
		mpConnect->SetEnabled( enable );
}
