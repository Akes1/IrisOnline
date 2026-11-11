#include "StdAfx.h"
#include "FriendWindow.h"
#include "UIImage.h"
#include "ChatWindow.h"
#include "ChatManager.h"
#include "Button.h"
#include "RadioButton.h"
#include "GameUIManager.h"
#include "Player_Common.h"
#include "PartyManager.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"


cFriendWindow::cFriendWindow()
: mpFriendList(0)
, mParty(0)
, mAddFriend(0)
, mDelFriend(0)
, mWhisper(0)
{
	for( int i = 0; i < MAXPAGE_NUM; ++i )
		mpRadioNode[i] = 0;

	memset( mpNodeSkin, 0, sizeof(mpNodeSkin) );

}

cFriendWindow::~cFriendWindow()
{
	Clear();
}

///
void cFriendWindow::Clear()
{
	if( mpFriendList )
		mpFriendList->Clear();

	/// 버튼 설정 초기화
	if( mParty )
		mParty->SetEnabled( false );
	if( mDelFriend )
		mDelFriend->SetEnabled( false );
	if( mWhisper )
		mWhisper->SetEnabled( false );

	for( int i = 0 ; i < MAXPAGE_NUM; ++i )
	{
		if( mpRadioNode[i] )
			mpRadioNode[i]->Hide();
	}
}

/// 주의사항 : 
bool cFriendWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpFriendList = (cFriendListbox*)GetChild( eUIID_GAME_FRIEND_LISTBOX );
	mParty = (cButton*)GetChild( eUIID_GAME_FRIEND_PARTYBUTTON );
	mAddFriend = (cButton*)GetChild( eUIID_GAME_FRIEND_ADDFRIENDBUTTON );
	mDelFriend = (cButton*)GetChild( eUIID_GAME_FRIEND_DELETEFREINDBUTTON );
	mWhisper = (cButton*)GetChild( eUIID_GAME_FRIEND_WHISPERBUTTON );

	/// 버튼 초기화 설정
	if( mParty )
		mParty->SetEnabled( false );
	if( mDelFriend )
		mDelFriend->SetEnabled( false );
	if( mWhisper )
		mWhisper->SetEnabled( false );

	/// 사용이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cString str[FRIEND_IMGNUM];		
	str[0] = "Human_M";		
	str[1] = "Beast_M"; 
	str[2] = "Elf_M";   
	str[3] = "Human_W";
	str[4] = "Beast_W";    
	str[5] = "Elf_W";   

	for( int i = 0; i < FRIEND_IMGNUM; ++i )
	{
		cUINodeSkin* pNodeSkin = pSkin->GetNodeSkin( str[i] );
		if( pNodeSkin )
			mpNodeSkin[i] = pNodeSkin;
		else
		{
			assert(0);
			return false;
		}
	}

	for( int i = 0; i < MAXPAGE_NUM; ++i )
	{
		mpRadioNode[i] = GetChild( eUIID_GAME_FRIEND_PAGERADIO1 + i );
		if( mpRadioNode[i] )
			mpRadioNode[i]->Hide();
	}
	return true;
}


void cFriendWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();
	UpdateButton();
}

///
void cFriendWindow::OnListBoxChanged( cUINode*, unsigned int )
{
	if( !mpFriendList )
	{
		assert(0);
		return;
	}

	unsigned int index = mpFriendList->GetSelectIndex();
	unsigned long characterIdx = (unsigned long)mpFriendList->GetExtraData( index );

	sFriendData* data = CHATMANAGER->GetFriendData( characterIdx );
	if( !data )
	{
		assert(0);
		return;
	}

	bool connect = (data->ChannelNum > 0);

	if( mParty )
		mParty->SetEnabled( connect );
	if( mWhisper )
		mWhisper->SetEnabled( connect );
}

///
void cFriendWindow::OnCommand( cUINode*, unsigned int id )
{
	if( !mpFriendList )
		return;

	switch( id )
	{
	case eUIID_GAME_FRIEND_CLOSE:
		Hide();
		break;
	case eUIID_GAME_FRIEND_PARTYBUTTON:
		{
			/// 파티 요청 
			unsigned int index = mpFriendList->GetSelectIndex();
			unsigned long characterIdx = (unsigned long)mpFriendList->GetExtraData( index );

			sFriendData* data = CHATMANAGER->GetFriendData( characterIdx );
			if( !data )
			{
				assert(0);
				return;
			}

			PARTYMAN->RequestPartyAdd( data->CharacterName );
		}
		break;
	case eUIID_GAME_FRIEND_WHISPERBUTTON:
		{
			/// 귓속말 자동입력
			cChatWindow* pWindow = (cChatWindow*)UIMAN->FindNodeByID( eUIID_GAME_CHATWINDOW );
			if( pWindow )
			{
				/// 선택된 친구의 아이디를 가져옴
				unsigned int index = mpFriendList->GetSelectIndex();
				unsigned long characterIdx = (unsigned long)mpFriendList->GetExtraData( index );

				sFriendData* data = CHATMANAGER->GetFriendData( characterIdx );
				if( !data )
				{
					assert(0);
					return;
				}

				/// "/w %s "
				cStringT str;
				str.Format( UIMAN->GetUIText(10005), data->CharacterName );

				/// 채팅윈도우로 포커스 옮기기
				pWindow->SetFocusToEdit();
				pWindow->SetChatText( (LPCTSTR)str.Cstr() );
			}
		}
		break;
	case eUIID_GAME_FRIEND_ADDFRIENDBUTTON:
		{
			/// 친구 추가 창 띄우기
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_FRIEND_ADD, eSKIN_EDIT,
				GAMERESOURCEMAN->GetGameText( 21 ), // 친구 추가 메시지
				GAMERESOURCEMAN->GetGameText( 18 ), // 친구추가
				0, 0, 0, false, false, 2, MAX_NAME_SIZE );
		}
		break;
	case eUIID_GAME_FRIEND_DELETEFREINDBUTTON:
		if( mpFriendList->GetRowCount() > 0 )
		{
			unsigned int index = mpFriendList->GetSelectIndex();
			unsigned long characterIdx = (unsigned long)mpFriendList->GetExtraData( index );
			sFriendData* data = CHATMANAGER->GetFriendData( characterIdx );
			if( !data )
			{
				assert(0);
				return;
			}

			/// 친구 삭제 창 띄우기
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_FRIEND_DELETE, eSKIN_YESNO,
								GAMERESOURCEMAN->GetGameText( 22 ), // 친구 삭제 메시지
								GAMERESOURCEMAN->GetGameText( 19 ), // 친구삭제
								(void*)data->Idx );
		}
		break;
	default:
		assert(0);
		break;
	}
}

///
void cFriendWindow::OnRadioPressed( cUINode*, unsigned int id )
{
	int row = 0;
	switch( id )
	{
	case eUIID_GAME_FRIEND_PAGERADIO1:	row = 0; break;
	case eUIID_GAME_FRIEND_PAGERADIO2:	row = 10; break;
	case eUIID_GAME_FRIEND_PAGERADIO3:	row = 20; break;
	case eUIID_GAME_FRIEND_PAGERADIO4:	row = 30; break;
	case eUIID_GAME_FRIEND_PAGERADIO5:	row = 40; break;
	default: assert(0); return;
	}	

	if( mpFriendList )
		mpFriendList->ChangeTopIndex( row );
}

///
bool cFriendWindow::AddFriend( sFriendData* data )
{
	if( !data )
	{
		assert(0);
		return false;
	}

	if( !mpFriendList )
	{
		assert(0);
		return false;
	}

	// 갯수 제한 검사
	if( mpFriendList->GetRowCount() >= MAX_FRIEND )
	{
		assert(0);
		return false;
	}

	// 데이터 삽입
	char i = GetFriendImage( data->Race, data->Gender );
	if( i >= FRIEND_IMGNUM )
	{
		assert(0);
		return false;
	}

	if( mpNodeSkin[i] )
	{
		cUIImage* image = 0;
		image = new cUIImage( mpNodeSkin[i]->mpTexture );
		if( image )
		{
			unsigned int tx = mpNodeSkin[i]->mSkinInfo->mTexX;
			unsigned int ty = mpNodeSkin[i]->mSkinInfo->mTexY;
			unsigned int tw = mpNodeSkin[i]->mSkinInfo->mTexWidth;
			unsigned int th = mpNodeSkin[i]->mSkinInfo->mTexHeight;
			int x = mpNodeSkin[i]->mSkinInfo->mX;
			int y = mpNodeSkin[i]->mSkinInfo->mY;
			image->SetTextureRect( tx, ty, tx + tw, ty + th );
			image->SetScreenRect( cUIRect( x, y, x+tw, y+th ) );
		}
		else
			assert(0);

		bool connect = ( data->ChannelNum > 0 );
		unsigned long color = ( connect == true && data->Block == 0 ) ? mDefaultColor : COLOR_DARKGRAY;

		// 삽입
		int row = mpFriendList->AddRow( data->CharacterName, color, image, (void*)data->CharacterIdx );
		if( row != -1 )
		{
			// 접속중
			if( connect == true && data->Block == 0 )
			{
				if( HERO->GetChannelNum() == data->ChannelNum )
				{
					// 레벨 lv 직업 맵이름
					cStringT str;
					str.Format( _T("%d Lv %s %s"), data->Level, GAMERESOURCEMAN->GetJobName( data->Job ), GAMERESOURCEMAN->GetMapName( data->MapNum ) );
					mpFriendList->AddColume( row, (LPCTSTR)str.Cstr(), color );
				}
				else
				{
					// 타채널의 일반 맵인경우
					if( data->MapNum >= MAP_MIN && data->MapNum <= MAP_MAX )
					{
						cStringT channel;
						channel.Format( GAMERESOURCEMAN->GetGameText( 1260 ), data->ChannelNum );

						cStringT str;
						str.Format( _T("%d Lv %s %s"), data->Level, GAMERESOURCEMAN->GetJobName( data->Job ), (LPTSTR)channel.Cstr() );
						mpFriendList->AddColume( row, (LPCTSTR)str.Cstr(), color );
					}
					else
					{
						// 타채널의 던전 맵인경우
						cStringT str;
						str.Format( _T("%d Lv %s %s"), data->Level, GAMERESOURCEMAN->GetJobName( data->Job ), GAMERESOURCEMAN->GetMapName( data->ChannelNum ) );
						mpFriendList->AddColume( row, (LPCTSTR)str.Cstr(), color );
					}
				}
			}
			else
			{
				// 미접속
				mpFriendList->AddColume( row, GAMERESOURCEMAN->GetGameText( 38 ), color );
			}
		}
		else
			assert(0);
	}

	// 등록 신청 출력
	if( data->Status == 1 )
	{
		cStringT str;
		str.Format( GAMERESOURCEMAN->GetGameText( 40 ), data->CharacterName );

		/// 친구등록 신청
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
							eMBEVENT_FRIEND_ADD_REQ, eSKIN_YESNO,
							(LPCTSTR)str.Cstr(),				// 친구 요청 메시지
							GAMERESOURCEMAN->GetGameText( 20 ), // 친구 요청
							(void*)data->Idx );
	}

	// 버튼 처리
	if( mpFriendList->GetRowCount() == MAX_FRIEND )
	{
		if( mAddFriend )
			mAddFriend->SetEnabled( false );
	}

	// 갱신		
	Update();
	return true;
}

void cFriendWindow::Update()
{
	/// 선택된 리스트에 따른 버튼 업데이트
	if( !mpFriendList || !mDelFriend )
		return;

	bool connect = false;
	if( mpFriendList->GetRowCount() == 0 )
	{
		mDelFriend->SetEnabled( false );
	}
	else
	{
		mDelFriend->SetEnabled( true );

		unsigned int index = mpFriendList->GetSelectIndex();
		unsigned long characterIdx = (unsigned long)mpFriendList->GetExtraData( index );

		sFriendData* data = CHATMANAGER->GetFriendData( characterIdx );
		if( data )
		{
			connect = (data->ChannelNum > 0);
		}
		else
			assert(0);
	}

	if( mParty )
		mParty->SetEnabled( connect );
	if( mWhisper )
		mWhisper->SetEnabled( connect );

	///
	UpdateButton();
}

void cFriendWindow::UpdateButton()
{
	if( !mpFriendList )
	{
		assert(0);
		return;
	}

	/// 버튼 위치 업데이트
	int total = mpFriendList->GetRowCount() - 1;
	int page = ( total / 10 );
	
	/// 위치 조정
	if( mpRadioNode[0] )
	{
		int w = mpRadioNode[0]->GetAbsoluteRect().GetWidth();	
		int x = mpFriendList->GetRelativeRect().mLeft + (int)((mpFriendList->GetRelativeRect().GetWidth() - (w*(page+1))) * 0.5f);
		for( int i = 0; i <= page; ++i )
		{
			cUIPos pos;
			pos.mX = x + (w*i);
			pos.mY = mpRadioNode[i]->GetRelativeRect().mTop;
			mpRadioNode[i]->SetRelativePos( pos );
		}

		((cRadioButton*)mpRadioNode[0])->SetPress( true );
	}

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
}

char cFriendWindow::GetFriendImage( char race, char gender )
{
	/// 남자면 그대로 리턴, 성별이 여자면 검사
	char result = race;
	if( gender == eGENDER_FEMALE )
	{
		if( race == eRACE_HUMAN ) result = 3;
		if( race == eRACE_BEAST ) result = 4;
		if( race == eRACE_ELF ) result = 5;
	}
	return result;
}

void cFriendWindow::OnNetLock( int lockTry )
{
	if( mParty )
		mParty->NetLock( lockTry );
	if( mAddFriend )
		mAddFriend->NetLock( lockTry );
	if( mDelFriend )
		mDelFriend->NetLock( lockTry );
}

void cFriendWindow::OnNetUnLock( int lockTry )
{
	if( mParty )
		mParty->NetUnLock( lockTry );
	if( mAddFriend )
		mAddFriend->NetUnLock( lockTry );
	if( mDelFriend )
		mDelFriend->NetUnLock( lockTry );

	///
	Update();
}


//////////////////////////////////////////////////////////////////////////////////////////
#include "ListBox.h"
#include "FontAgent.h"

cFriendListbox::cFriendListbox( eUINodeType type )
: cImageListBox( type )
{
}

cFriendListbox::~cFriendListbox()
{
}

/// 주의사항 : 리스트 아이템 그려주기
void cFriendListbox::RenderData( cUIFontItemKeeper* pKeeper )
{
	/// 전체 리스트를 얻어옴 
	if( GetRowCount() <= 0 )
		return;

	/// 스킨 받아오기 
	cListBoxSkin* pListSkin = (cListBoxSkin*)mpSkin;
	if( !pListSkin )
		return;

	/// 리스트 박스의 좌표 받아오기
	int textposX = 0;
	int textPosY = GetAbsoluteRect().mTop;

	unsigned int rowCount = ( mMaxRowInPage > GetRowCount() )? GetRowCount() : mMaxRowInPage;
	for( unsigned int row = mTopRowIdx; row < mTopRowIdx + rowCount; ++row )
	{
		sRowData* rData = GetRowData( row );
		if( !rData )
			continue;

		/// 리스트 데이터 출력
		unsigned int colCount = rData->mColumes.GetSize();
		unsigned int fixposY = textPosY;
		for( unsigned int col = 0; col< colCount; ++col )
		{
			/// 각 컬럼 데이터 가져오기
			sListImageData* data = (sListImageData*)rData->mColumes[col];

			if( data->mpImage && col == 0 )
			{
				cUIImage* image = data->mpImage;

				sListCtrlParam* pParam = pListSkin->mpImgParam;
				unsigned int imageX = pParam->mColumnX;
				unsigned int imageY = textPosY + (int)((mRowHeight-image->GetHeight())*0.5f);

				image->SetPos( GetAbsoluteRect().mLeft + imageX, imageY );
				image->Draw();
			}

			sListCtrlParam* pParam = pListSkin->GetCtrlParam( col );

			unsigned int tX = 0;
			unsigned int tW = 0;
			eText_Align align = eALIGN_LEFT;

			if( pParam )
			{
				tX = pParam->mColumnX;
				tW = pParam->mColumnW;
				align = pParam->mAlign;
			}

			/// 최종 출력 좌표 계산 
			unsigned int strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
			textposX = UpdateTextPos( align, strWidth, tX, tW );

			/// Hover 된 아이템은 다른 색으로 표시
			unsigned color = data->mTextColor;
			if( mHoveredRowIdx != -1 && row - mTopRowIdx == (unsigned int)mHoveredRowIdx )
				color = COLOR_RED;

			pKeeper->AddFontItem( cFontAgent::eFont_System, (LPTSTR)data->mText.Cstr(), textposX, fixposY + 5, color );

			/// 두번째 컬럼부터는 바로 한줄 밑에 찍는다
			fixposY += FONTAGENT->GetTextHeight(cFontAgent::eFont_System) + 2;
		}

		textPosY += mRowHeight;
	}
}
