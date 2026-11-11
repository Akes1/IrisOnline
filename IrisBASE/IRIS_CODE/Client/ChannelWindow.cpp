#include "StdAfx.h"
#include "ChannelWindow.h"
#include "StageManager.h"
#include "SelectStage.h"

#include "Button.h"
#include "ListBox.h"
#include "DisplayObject.h"
#include "GameResourceManager.h"

cChannelWindow::cChannelWindow()
: mpChannelList(0)
, mpConnectBtn(0)
{
	for( int i = 0; i < MAXPAGE_NUM; ++i )
		mpRadioNode[i] = 0;

	mCheckRect.Set( 0, 0, 0, 0 );
}

cChannelWindow::~cChannelWindow()
{
	Clear();
}

void cChannelWindow::Open()
{
	Hide();
}

void cChannelWindow::Clear()
{
	if( mpChannelList )
		mpChannelList->Clear();

	for( unsigned int i = 0; i < mChannelData.GetSize(); ++i )
	{
		delete (sChannelData*)mChannelData[i];
	}
	mChannelData.Clear();

	for( int i = 0 ; i < MAXPAGE_NUM; ++i )
	{
		if( mpRadioNode[i] )
			mpRadioNode[i]->Hide();
	}
}

void cChannelWindow::OnShow()
{
	if( mpChannelList && mpChannelList->GetRowCount() > 0 )
	{
		if( mpConnectBtn )
			mpConnectBtn->SetEnabled( true );
	}
	else
	{
		if( mpConnectBtn )
			mpConnectBtn->SetEnabled( false );
	}

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = GetRelativeRect().mTop;
	SetRelativePos( cUIPos( x, y) );
}

bool cChannelWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpChannelList = (cListBox*)GetChild( eUIID_CHANNEL_CHANNELLISTBOX );
	mpConnectBtn = (cButton*)GetChild( eUIID_CHANNEL_CONNECT_BUTTON );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 정렬영역
	cUINodeSkin* pRectSkin = pSkin->GetNodeSkin( "Channel_ButtonRect" );
	if( pRectSkin )
	{
		mCheckRect.mLeft = pRectSkin->mSkinInfo->mX;
		mCheckRect.mTop = pRectSkin->mSkinInfo->mY;
		mCheckRect.mRight = mCheckRect.mLeft + pRectSkin->mSkinInfo->mWidth;
		mCheckRect.mBottom = mCheckRect.mTop + pRectSkin->mSkinInfo->mHeight;
	}

	for( int i = 0; i < MAXPAGE_NUM; ++i )
	{
		mpRadioNode[i] = GetChild( eUIID_CHANNEL_RADIOPAGE1 + i );
		if( mpRadioNode[i] )
			mpRadioNode[i]->Hide();
	}
	return true;
}

void cChannelWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = GetRelativeRect().mTop;
	SetRelativePos( cUIPos( x, y) );

	UpdateButton();
}

void cChannelWindow::OnListBoxSelected( cUINode* , unsigned int id )
{
	if( id == eUIID_CHANNEL_CHANNELLISTBOX )
	{
		CheckConnect();
	}
}

void cChannelWindow::AddChannel( long param, int status, LPCTSTR channelname, LPCTSTR condition, unsigned long color, bool select )
{
	/// 채널 정보 세팅 
	sChannelData* data = new sChannelData;
	data->mParam = param;
	data->mStatus = status;
	mChannelData.PushBack( data );

	/// 리스트박스에 세팅 
	int row = mpChannelList->AddRow( channelname, mDefaultColor, data );
	if( row <= -1 )
	{
		assert(0);
	}

	if( mpChannelList )
	{
		mpChannelList->AddColume( row, condition, color );

		if( select == true )
			mpChannelList->ChangeSelectRow( row );
	}
}

void cChannelWindow::UpdateButton()
{
	if( !mpChannelList )
	{
		assert(0);
		return;
	}

	/// 버튼 보이고 안보이고 하기
	int total = mpChannelList->GetRowCount() - 1;
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

/// 채널 정보
long cChannelWindow::GetSelectedChannelParam()
{
	sChannelData* pData = (sChannelData*)mpChannelList->GetExtraData( mpChannelList->GetSelectIndex() );
	if( pData )
		return pData->mParam;
	return -1;
}

/// 채널상태
int cChannelWindow::GetSelectedChannelStatus()
{
	sChannelData* pData = (sChannelData*)mpChannelList->GetExtraData( mpChannelList->GetSelectIndex() );
	if( pData )
		return pData->mStatus;
	return -1;
}

void cChannelWindow::OnCommand( cUINode* pcaller, unsigned int id )
{
	switch( id )
	{
	case eUIID_CHANNEL_CONNECT_BUTTON:
		{
			CheckConnect();
		}
		break;
	case eUIID_CHANNEL_CANCEL_BUTTON:
		{
			cSelectStage* p = (cSelectStage*)STAGEMAN->GetStage( eStage_Select );
			if( p )
				p->CancelChannel();
		}
		break;
	default:
		assert(0);
		break;
	}
}

void cChannelWindow::CheckConnect()
{
	long channelNum = GetSelectedChannelParam();
	if( channelNum <= -1 )
		return;

	int status = GetSelectedChannelStatus();
	if( status == 5 )
	{
		UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 37 ), // 
							GAMERESOURCEMAN->GetGameText( 2 ) );
		return;
	}

	cSelectStage* p = (cSelectStage*)STAGEMAN->GetStage( eStage_Select );
	if( p )
	{
		cDisplayObject* obj = p->GetCurrentDisplayObject();
		if( obj )
		{
			if( obj->GetMapNum() == 0 )
			{
				// 처음 생성하여 맵번호가 0이면 튜토리얼 진입여부
				UIMAN->CreateMsgBox( eStage_Select, eMSGBOX_MODAL,
					eMBEVENT_TUTORIAL_SELECT, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 29 ), // 
					GAMERESOURCEMAN->GetGameText( 2 ),  // 알림
					(void*)channelNum );
			}
			else
			{
				// 접속할 맵이 0이 아니면 게임 접속
				p->SendConnect( channelNum, (obj->GetMapNum() == MAP_TUTORIAL) );
			}
		}
	}
}
