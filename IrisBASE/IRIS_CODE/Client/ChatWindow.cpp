#include "StdAfx.h"
#include "ChatWindow.h"
#include "ChatManager.h"
#include "TextBox.h"
#include "EditBox.h"
#include "Button.h"
#include "CheckBox.h"
#include "PlaneObject.h"
#include "ItemManager.h"
#include "GameResourceManager.h"
#include "PartyUnionManager.h"
#include "GameUIManager.h"
#include "ItemIcon.h"
#include "DragWindow.h"
#include "TipWindow.h"
#include "PushButton.h"
#include "FontAgent.h"

cChatWindow::cChatWindow()
: mpOptionImage(0)
, mpChatBoxImage(0)
, mpChatBox(0)
, mpChatEdit(0)
, mpOptionSkin(0)
, mDefalutRowNum(0)
, mCount(0)
, mpChatBoxImageUp(0)
, mpChatBoxImageDown(0)
, mpUpSkin(0)
, mpChatEditWhisper(0)
, mSelectType( eCHAT_MAX )
, mpSelectImage(0)
, mpSelectSkin(0)
, mpButtonWindow(0)
, mpLinkItemIcon(0)
, mpNormal(0)
, mpParty(0)
, mpGuild(0)
, mpShout(0)
, mpTrade(0)
, mpWhisper(0)
, mpSystem(0)
, mpMegaPhone(0)
, mLinkItemIndex(0)
, mpMoveLock(0)
{
	for( int i = 0; i < 8; ++i )
	{
		mpTexSkin[i] = 0;
	}
}

cChatWindow::~cChatWindow()
{
	SAFE_DELETE( mpSelectImage );
	SAFE_DELETE( mpChatBoxImageDown );
	SAFE_DELETE( mpChatBoxImageUp );
	SAFE_DELETE( mpOptionImage );
	SAFE_DELETE( mpChatBoxImage );
}

void cChatWindow::Open()
{
	cUIWindow::Open();

	OnShow();
}

void cChatWindow::Close()
{
	cUIWindow::Close();

	if( mpChatEdit )
	{
		mpChatEdit->Hide();
		mpChatEdit->Clear();
	}

	DeleteLinkItem();
}

void cChatWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mpChatEdit )
		mpChatEdit->Show();
}

void cChatWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mpChatEdit )
	{
		mpChatEdit->Hide();
		mpChatEdit->Clear();
	}
}

/// 
bool cChatWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpChatBox = (cTextBox*)GetChild( eUIID_CHATWINDOW_CHATBOX );
	mpChatEdit = (cEditBox*)GetChild( eUIID_CHATWINDOW_CHATBOX_EDITBOX );
	mpChatEditWhisper = (cEditBox*)GetChild( eUIID_CHATWINDOW_CHATBOX_EDIT_WHISPER );
	mpMoveLock = (cPushButton*)GetChild( eUIID_CHATWINDOW_MOVELOCK );

	mpNormal = GetChild( eUIID_CHATWINDOW_NORMAL );
	mpParty = GetChild( eUIID_CHATWINDOW_PARTY );
	mpGuild = GetChild( eUIID_CHATWINDOW_GUILD );
	mpShout = GetChild( eUIID_CHATWINDOW_SHOUT );
	mpTrade = GetChild( eUIID_CHATWINDOW_TRADE );
	mpWhisper = GetChild( eUIID_CHATWINDOW_WHISPER );
	mpSystem = GetChild( eUIID_CHATWINDOW_SYSTEM );
	mpMegaPhone = GetChild( eUIID_CHATWINDOW_MEGAPHONE );

	/// 채팅윈도우 등록
	CHATMANAGER->SetChatWindow( this );

	/// 에디트 등록
	CHATMANAGER->AddFocusNode( mpChatEdit );

	///
	cUISkin* skin = UIMAN->GetSkin();
	if( !skin )
	{
		assert(0);
		return false;
	}

	/// 채팅박스 배경이미지
	cUINodeSkin* pplaneSkin = skin->GetNodeSkin( "ChatBoxImage" );
	if( pplaneSkin && pplaneSkin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned int tx = pplaneSkin->mSkinInfo->mTexX;
		unsigned int ty = pplaneSkin->mSkinInfo->mTexY;
		unsigned int tw = pplaneSkin->mSkinInfo->mTexWidth;
		unsigned int th = pplaneSkin->mSkinInfo->mTexHeight;
		int x = mpChatBox->GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mTop;
		unsigned int w = mpChatBox->GetAbsoluteRect().GetWidth();
		unsigned int h = mpChatBox->GetAbsoluteRect().GetHeight();

		mpChatBoxImage = new cPlaneObject;
		if( mpChatBoxImage->Create( pplaneSkin->mpTexture, x, y, w, h,
									tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create chat box image");
			return false;
		}
	}
	else
	{
		assert(0);
		return false;
	}

	/// 채팅박스 이미지 틀 
	mpUpSkin = skin->GetNodeSkin( "ChatBoxImage_Up" );
	if( mpUpSkin && mpUpSkin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned int tx = mpUpSkin->mSkinInfo->mTexX;
		unsigned int ty = mpUpSkin->mSkinInfo->mTexY;
		unsigned int tw = mpUpSkin->mSkinInfo->mTexWidth;
		unsigned int th = mpUpSkin->mSkinInfo->mTexHeight;
		int x = mpChatBox->GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mTop - th;
		unsigned int w = mpChatBox->GetAbsoluteRect().GetWidth();

		mpChatBoxImageUp = new cPlaneObject;
		if( mpChatBoxImageUp->Create( mpUpSkin->mpTexture, x, y, w, th,
										tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create chat box up image");
			return false;
		}
	}

	cUINodeSkin* downSkin = skin->GetNodeSkin( "ChatBoxImage_Down" );
	if( downSkin && downSkin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned int tx = downSkin->mSkinInfo->mTexX;
		unsigned int ty = downSkin->mSkinInfo->mTexY;
		unsigned int tw = downSkin->mSkinInfo->mTexWidth;
		unsigned int th = downSkin->mSkinInfo->mTexHeight;
		int x = mpChatBox->GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mBottom;
		unsigned int w = mpChatBox->GetAbsoluteRect().GetWidth();

		mpChatBoxImageDown = new cPlaneObject;
		if( mpChatBoxImageDown->Create( downSkin->mpTexture, x, y, w, th,
			tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create chat box down image");
			return false;
		}
	}

	/// 채팅 옵션 스킨 로드
	mpOptionSkin = skin->GetNodeSkin( "OptionImage" );
	if( mpOptionSkin && mpOptionSkin->mpTexture )
	{
		mpOptionImage = new cUIImage( mpOptionSkin->mpTexture );
		unsigned int tx = mpOptionSkin->mSkinInfo->mTexX;
		unsigned int ty = mpOptionSkin->mSkinInfo->mTexY;
		unsigned int tw = mpOptionSkin->mSkinInfo->mTexWidth;
		unsigned int th = mpOptionSkin->mSkinInfo->mTexHeight;
		mpOptionImage->SetTextureRect( tx, ty, tx + tw, ty + th );

		unsigned int x = GetAbsoluteRect().mLeft;
		unsigned int y = mpChatBox->GetAbsoluteRect().mTop + mpChatBox->GetAbsoluteRect().GetHeight() + HEIGHT_GAP;
		
		cUIRect desc( x, y, x+tw, y+th );
		mpOptionImage->SetScreenRect( desc );
	}
	else
	{
		assert( 0 && "failed to find optionSkin skin" );
		return false;
	}

	/// 선택 버튼 이미지
	mpSelectSkin = skin->GetNodeSkin( "Chat_SelectImage" );
	if( mpSelectSkin && mpSelectSkin->mpTexture )
	{
		mpSelectImage = new cUIImage( mpSelectSkin->mpTexture );
		unsigned int tx = mpSelectSkin->mSkinInfo->mTexX;
		unsigned int ty = mpSelectSkin->mSkinInfo->mTexY;
		unsigned int tw = mpSelectSkin->mSkinInfo->mTexWidth;
		unsigned int th = mpSelectSkin->mSkinInfo->mTexHeight;
		unsigned int x = mpSelectSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpSelectSkin->mSkinInfo->mY + mpChatBox->GetAbsoluteRect().mBottom;
		mpSelectImage->SetTextureRect( tx, ty, tx + tw, ty + th );

		cUIRect desc( x, y, x+tw, y+th );
		mpSelectImage->SetScreenRect( desc );
	}

	/// 링크 아이템 아이콘 생성
	mpLinkItemIcon = new cItemIcon;
	if( mpLinkItemIcon->CreateBySkinName( "Inven_LinkIcon", this, 100 ) == false )
		return false;

	/// 윈도우 사이즈 갱신
	if( mpOptionSkin )
	{
		int w = mpOptionSkin->mSkinInfo->mTexWidth;
		int h = mpOptionSkin->mSkinInfo->mTexHeight + mpChatBox->GetAbsoluteRect().GetHeight() + HEIGHT_GAP;
		SetRelativeSize( cUISize(w, h) );
	}

	///
	if( mpChatBox )
	{
		mpChatBox->SetOutLine( true );
		mpChatBox->SetChatMode( true ); 

		mDefalutRowNum = mpChatBox->GetMaxRowInPage();
	}

	if( mpChatEditWhisper )
		mpChatEditWhisper->SetTextColor( COLOR_PINK );

	mpButtonWindow = GAMEUI->GetChatButtonWindow();
	if( !mpButtonWindow )
	{
		assert(0);
		return false;
	}
	
	/// 기본세팅
	if( mpNormal )
		mpNormal->SetPress( true );

	if( mpParty )
		mpParty->SetPress( true );

	if( mpGuild )
		mpGuild->SetPress( true );

	if( mpShout )
		mpShout->SetPress( true );

	if( mpTrade )
		mpTrade->SetPress( true );

	if( mpWhisper )
		mpWhisper->SetPress( true );

	if( mpMegaPhone )
		mpMegaPhone->SetPress( true );

	/// 선택된 타입
	UpdateChatType( eCHAT_NORMAL );

	InitBoxSize( DEFAULT_ROWNUM );
	return true;
}

///
bool cChatWindow::HandleEvent( const cUIEvent& event )
{
	if( event.mType == eUIEVENT_KEY_DOWN )
	if( event.mCode == eKEY_UP || event.mCode == eKEY_DOWN )
	{
		KeyEvent( event.mCode );
	}

	return cUIWindow::HandleEvent( event );
}

/// 주의사항 : 윈도우 Move 시에만 호출 
void cChatWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( !mpChatBox )
		return;

	/// 채팅박스 배경이미지
	if( mpChatBoxImage )
	{
		int x = mpChatBox->GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mTop;
		unsigned int w = mpChatBox->GetAbsoluteRect().GetWidth();
		unsigned int h = mpChatBox->GetAbsoluteRect().GetHeight();
		mpChatBoxImage->SetScreenRect( x, y, w, h );
	}

	/// 채팅 옵션 스킨 로드
	if( mpOptionImage )
	{
		int x = GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mTop + mpChatBox->GetAbsoluteRect().GetHeight() + HEIGHT_GAP;
		mpOptionImage->SetPos( x, y );
	}

	/// 위아래 이미지
	if( mpChatBoxImageUp && mpUpSkin )
	{
		int x = mpChatBox->GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mTop - mpUpSkin->mSkinInfo->mTexHeight;
		mpChatBoxImageUp->SetScreenXY( x, y );
	}

	if( mpChatBoxImageDown )
	{
		int x = mpChatBox->GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mBottom;
		mpChatBoxImageDown->SetScreenXY( x, y );
	}

	if( mpSelectImage && mpSelectSkin && mpChatBox )
	{
		int x = mpSelectSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		int y = mpSelectSkin->mSkinInfo->mY + mpChatBox->GetAbsoluteRect().mBottom;
		mpSelectImage->SetPos( x, y );
	}

	if( mpButtonWindow && mpSelectSkin && mpChatBox )
	{
		int x = mpSelectSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		int y = (mpSelectSkin->mSkinInfo->mY + mpChatBox->GetAbsoluteRect().mBottom) - mpButtonWindow->GetAbsoluteRect().GetHeight();
		mpButtonWindow->SetRelativePos( cUIPos(x, y) );
	}
}

/// 주의사항 : 해상도 변경시에 호출 
void cChatWindow::UpdateSkin()
{
	if( mpOptionSkin )
	{
		int h = mpOptionSkin->mSkinInfo->mTexHeight + mpChatBox->GetAbsoluteRect().GetHeight() + HEIGHT_GAP;
		int y = GetScreenHeight() - h;

		if( y > 0 )
			SetRelativePos( cUIPos( 0, y ) );
		else
			assert(0);
	}
}

///
void cChatWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	if( mpChatBoxImage )
		mpChatBoxImage->Draw();

	if( mpOptionImage )
		mpOptionImage->Draw();

	if( mpChatBoxImageUp )
		mpChatBoxImageUp->Draw();

	if( mpChatBoxImageDown )
		mpChatBoxImageDown->Draw();

	if( mpSelectImage )
		mpSelectImage->Draw();

	cUIWindow::OnRender( pKeeper );
}

void cChatWindow::OnMouseMove( const cUIPos& pos )
{
	// 이동기능 lock 아니면 
	if( IsLockedMove() == false )
		cUIWindow::OnMouseMove( pos );
}

/// 주의사항 : 
void cChatWindow::OnEditBoxEntered( cUINode*, unsigned int id )
{
	if( !mpChatEdit )
	{
		assert(0);
		return;
	}

	if( id == eUIID_CHATWINDOW_CHATBOX_EDITBOX )
	{
		CHATMANAGER->SendChatMsg( mSelectType, mpChatEdit->GetText() );

		mpChatEdit->Clear();
		mpChatEdit->ReleaseFocus();

		///
		DeleteLinkItem();
	}
	else if( id == eUIID_CHATWINDOW_CHATBOX_EDIT_WHISPER )
	{
		mpChatEdit->SetFocus();
	}
}

void cChatWindow::OnLButtonDoubleClick( const cUIPos& pos )
{
	OnLButtonDown( pos, false, false, false );
}

///
void cChatWindow::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, false );

	if( mpSelectSkin && mpButtonWindow && mpChatBox )
	{
		cUIRect rect;
		rect.mLeft = GetAbsoluteRect().mLeft + mpSelectSkin->mSkinInfo->mX;
		rect.mTop = mpChatBox->GetAbsoluteRect().mBottom + mpSelectSkin->mSkinInfo->mY;
		rect.mRight = rect.mLeft + mpSelectSkin->mSkinInfo->mWidth;
		rect.mBottom = rect.mTop + mpSelectSkin->mSkinInfo->mHeight;

		/// 이미지를 클릭하였으면
		if( rect.ContainPoint( pos ) == true )
		{
			if( mpButtonWindow->IsVisible() == true )
				mpButtonWindow->Hide();
			else
				mpButtonWindow->Show();
		}
	}
}

/// 주의사항 : 
void cChatWindow::OnButtonDowned(cUINode*, unsigned int id)
{
	switch( id )
	{
	case eUIID_CHATWINDOW_SIZE_PLUS_BUTTON:
		{
			/// 한페이지에 수용하는 최대 행수 
			unsigned int maxNum = mpChatBox->GetMaxRowInPage();
			if( maxNum < MAXROWNUM )
			{
				/// 채팅창 확대 
				int rowHeight = mpChatBox->GetRowHeight();
				UpdateSize( rowHeight );
			}	
		}	
		break;
	case eUIID_CHATWINDOW_SIZE_MINUS_BUTTON:
		{
			unsigned int maxNum = mpChatBox->GetMaxRowInPage();
			if( maxNum > mDefalutRowNum )
			{
				/// 채팅창 축소 
				int rowHeight = mpChatBox->GetRowHeight();
				UpdateSize( -rowHeight );
			}			
		}
		break;
	}
}

/// 주의사항 : 
void cChatWindow::OnCommand( cUINode*, unsigned int id )
{
	if( !mpChatBox )
	{
		assert(0);
		return;
	}

	switch( id )
	{
	case eUIID_CHATWINDOW_EXTEND_BUTTON:
		{
			cExtraList::cIterator b = mExtraList.Begin();
			cExtraList::cIterator end = mExtraList.End();
			for( ; b != end; ++b )
			{
				cUIWindow* w = (cUIWindow*)(*b);
				if( w && w->IsVisible() == false )
				{
					w->Show();
					break;
				}
			}
		}	
		break;
	case eUIID_CHATWINDOW_SIZE_PLUS_BUTTON:
		{
			/// 한페이지에 수용하는 최대 행수 
			unsigned int maxNum = mpChatBox->GetMaxRowInPage();
			if( maxNum < MAXROWNUM )
			{
				/// 채팅창 확대 
				int rowHeight = mpChatBox->GetRowHeight();
				UpdateSize( rowHeight );
			}	
		}	
		break;
	case eUIID_CHATWINDOW_SIZE_MINUS_BUTTON:
		{
			unsigned int maxNum = mpChatBox->GetMaxRowInPage();
			if( maxNum > mDefalutRowNum )
			{
				/// 채팅창 축소 
				int rowHeight = mpChatBox->GetRowHeight();
				UpdateSize( -rowHeight );
			}			
		}
		break;
	}
}

/// 주의사항 : 사이즈 확대 / 축소 
void cChatWindow::UpdateSize( int height )
{
	if( height == 0 )
	{
		assert(0);
		return;
	}

	if( !mpChatBox )
	{
		assert(0);
		return;
	}

	/// 채팅박스 갱신 
	cUISize boxsize;
	boxsize.mWidth = mpChatBox->GetAbsoluteRect().GetWidth();
	boxsize.mHeight = mpChatBox->GetAbsoluteRect().GetHeight() + height;
	mpChatBox->SetRelativeSize( boxsize );

	/// 채팅윈도우 갱신
	cUIRect winRc = GetAbsoluteRect();
	winRc.mTop -= height;
	SetRelativeRect( winRc );

	/// 캡션 윈도우 갱신
	cUIRect captionRc = GetCaptionRect();
	captionRc.mBottom = captionRc.mTop  + winRc.GetHeight();
	captionRc.mRight  = captionRc.mLeft + mpChatBox->GetAbsoluteRect().GetWidth();
	SetCaptionRect( captionRc );

	/// 채팅윈도우 자식 컨트롤 업데이트 
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();
	for( ; i != end; ++i )
	{
		cUINode* child = (cUINode*)(*i);
		if( child != mpChatBox )
		{
			cUIRect childRc = child->GetRelativeRect();
			childRc.mTop += height;
			childRc.mBottom += height;
			child->SetRelativeRect( childRc );
		}
	}

	if( mpChatBoxImage )
	{
		int x = mpChatBox->GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mTop;
		unsigned int w = mpChatBox->GetAbsoluteRect().GetWidth();
		unsigned int h = mpChatBox->GetAbsoluteRect().GetHeight();
		mpChatBoxImage->SetScreenRect( x, y, w, h );
	}

	/// 채팅 옵션 스킨 로드
	if( mpOptionImage )
	{
		int x = GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mTop + mpChatBox->GetAbsoluteRect().GetHeight() + HEIGHT_GAP;
		mpOptionImage->SetPos( x, y );
	}

	/// 사이즈 변경에 따른 채팅박스 데이터 갱신 
	bool sizeAdd = (height > 0 ) ? true : false;
	mpChatBox->UpdateTextBox( sizeAdd );
}

void cChatWindow::UpdateChatType( eChatMsgKind type )
{
	if( mSelectType == type )
		return;

	cUISkin* skin = UIMAN->GetSkin();
	if( !skin )
	{
		assert(0);
		return;
	}

	/// 이미지 세팅
	cUINodeSkin* nodeSkin = 0;
	switch( type )
	{
	case eCHAT_NORMAL:
		nodeSkin = skin->GetNodeSkin( "ChatButton_Normal" );
		break;
	case eCHAT_PARTY:
		nodeSkin = skin->GetNodeSkin( "ChatButton_Party" );
		break;
	case eCHAT_GUILD:
		nodeSkin = skin->GetNodeSkin( "ChatButton_Guild" );
		break;
	case eCHAT_SHOUT:
		nodeSkin = skin->GetNodeSkin( "ChatButton_Shout" );
		break;
	case eCHAT_TRADE:
		nodeSkin = skin->GetNodeSkin( "ChatButton_Trade" );
		break;
	case eCHAT_WHISPER:
		nodeSkin = skin->GetNodeSkin( "ChatButton_Whisper" );
		break;
	}
	
	/// 선택된 이미지 적용
	if( nodeSkin && mpSelectImage )
	{
		mSelectType = type;

		unsigned int tx = nodeSkin->mSkinInfo->mTexX;
		unsigned int ty = nodeSkin->mSkinInfo->mTexY;
		unsigned int tw = nodeSkin->mSkinInfo->mTexWidth;
		unsigned int th = nodeSkin->mSkinInfo->mTexHeight;
		mpSelectImage->SetTextureRect( tx, ty, tx + tw, ty + th );

		/// 그외 정보 갱신
		if( mpChatEditWhisper )
		{
			if( mSelectType == eCHAT_WHISPER )
				mpChatEditWhisper->Show();
			else
				mpChatEditWhisper->Hide();
		}
	}
}

/// 주의사항 : 
void cChatWindow::InsertText( eChatMsgKind msgkind, unsigned long textIndex, LPCTSTR msg, unsigned long color, bool checkPress, bool paste )
{
	if( !mpChatBox )
	{
		assert(0);
		return;
	}

	cUINode* checkNode = NULL;

	switch( msgkind )
	{
	case eCHAT_NORMAL:		checkNode = mpNormal;		break;
	case eCHAT_PARTY:		checkNode = mpParty;		break;
	case eCHAT_GUILD:		checkNode = mpGuild;		break;
	case eCHAT_SHOUT:		checkNode = mpShout;		break;
	case eCHAT_TRADE:		checkNode = mpTrade;		break;
	case eCHAT_WHISPER:		checkNode = mpWhisper;		break;
	case eCHAT_GM:			checkNode = mpNormal;		break;
	case eCHAT_MEGAPHONE:	checkNode = mpMegaPhone;	break;
	case eCHAT_GUILD_NOTICE:checkNode = mpGuild;		break;
	}

	// 버튼 눌림 체크
	if( checkPress == true )
	{
		if( !(checkNode && checkNode->IsPress() == true) )
			return;
	}

	// 앞단에 붙일 말이 있으면 처리
	cStringT str;
	if( textIndex > 0 )
	{
		str.Format( GAMERESOURCEMAN->GetGameText( textIndex ), msg );
	}
	else
	{
		str = msg;
	}
	
	if( paste == false )
	{
		mpChatBox->AddNewRow( (LPCTSTR)str.Cstr(), color );
	}
	else
	{
		mpChatBox->AddPasteRow( (LPCTSTR)str.Cstr(), color );
	}
}

/// 주의사항 : 
void cChatWindow::InsertTextSystem( eSystemMsgKind kind, LPCTSTR chatmsg )
{
	if( !mpChatBox )
	{
		assert(0);
		return;
	}

	unsigned long color = COLOR_WHITE;

	switch( kind )
	{
	case eSYSTEM_NORMAL:	color = 0xFFFFFFFF;		break;
	case eSYSTEM_GETMONEY:	color = 0xFFFFFF0A;		break;
	case eSYSTEM_GETITEM:	color = 0xFF02FFA3;		break;
	case eSYSTEM_QUEST:		color = 0xFF00FFFF;		break;
	case eSYSTEM_DOITEM:	color = 0xFFA6B27D;		break;
	case eSYSTEM_CASH:		color = 0xFF643264;		break;
	case eSYSTEM_NPCSTORE:  color = 0xFFE9A921;		break;
	case eSYSTEM_PVP:		color = COLOR_RED;		break;
	case eSYSTEM_GM:		color = COLOR_RED;		break;
	default:				assert(0);				break;
	}

	if( mpSystem && mpSystem->IsPress() == true )
	{
		TCHAR temp[256]={0,};
		::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 91 ), chatmsg );
		mpChatBox->AddNewRow( temp, color );
	}
}

void cChatWindow::InsertItemText( unsigned long itemIndex, unsigned short slotIndex, LPCTSTR msg1, LPCTSTR msg2, bool IsQuest )
{
	if( !mpChatBox )
	{
		assert(0);
		return;
	}

	/// 시스템 메세지 보이기 설정되어 있으면 출력
	if( mpSystem && mpSystem->IsPress() == true )
	{
		cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIndex );
		if( !itemDefine )
			return;
		
		/// 글씨 컬러 정하기
		unsigned long itemColor = COLOR_WHITE;
		unsigned long textColor = (IsQuest == false) ? COLOR_WHITE : 0xFF00FFFF;	/// 퀘스트 색
		ITEMMAN->GetTooltipColor( itemDefine->GetTipType(), itemColor );
		
		/// 새로운줄에 삽입할지 결정
		TCHAR itemName[256] = {0,};
		::_stprintf_s( itemName, GAMERESOURCEMAN->GetGameText( 241 ), itemDefine->GetName() );

		TCHAR temp[256]={0,};
		if( msg1 )
			::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 91 ), msg1 );
		else
			::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 93 ) );

		/// 인벤정보 구성
		sInventory inventory;
		::memset( &inventory, 0, sizeof(inventory) );

		inventory.ItemIndex = itemIndex;
		inventory.number = slotIndex;

		if( slotIndex >= MIN_INVENTORY && slotIndex <= MAX_INVENTORY )
		{
			cItem* item = ITEMMAN->GetItem( slotIndex );
			if( item && item->GetDefine() != 0 )
			{
				inventory.enhanced = item->GetEnhanceStep();
				inventory.cardSlot1 = item->GetCardIndex1();
				inventory.cardSlot2 = item->GetCardIndex2();
				inventory.cardSlot3 = item->GetCardIndex3();
				inventory.cardSlot4 = item->GetCardIndex4();
				inventory.cardSlot5 = item->GetCardIndex5();
			}
		}

		mpChatBox->AddNewRow( temp, textColor );
		mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
		mpChatBox->AddPasteRow( msg2, textColor );
	}
}

/// 아이템 거래 메세지 출력
void cChatWindow::InsertItemText( sInventory inventory, LPCTSTR name, LPCTSTR msg,  ePLAYER_JOB job, eChatMsgKind kind, void* exdata )
{
	if( !mpChatBox )
	{
		assert(0);
		return;
	}

	TCHAR temp[256] = {0,};

	unsigned long nameColor = COLOR_WHITE;
	
	if( job != ePLAYER_NONEJOB )
	{
		if(	ePLAYER_FIGHTER			<= job && job <= ePLAYER_SOULBLADER )	nameColor = COLOR_JOB_FIGHTER; 
		else if( ePLAYER_MAGE		<= job && job <= ePLAYER_HIGHPRIEST )	nameColor = COLOR_JOB_MAGE; 
		else if( ePLAYER_WANDERER	<= job && job <= ePLAYER_SNIPER )		nameColor = COLOR_JOB_WANDERER; 
		else if( ePLAYER_WARLOCK	<= job && job <= ePLAYER_MENTALIST )	nameColor = COLOR_JOB_WARLOCK; 
		else if( ePLAYER_ARMSMAN	<= job && job <= ePLAYER_HIGHLANDER )	nameColor = COLOR_JOB_ARMSMAN; 
		else if( ePLAYER_RANGER		<= job && job <= ePLAYER_SHARPSHOOTER )	nameColor = COLOR_JOB_RANGER;
	}
	
	cItemDefine* define = ITEMMAN->GetItemDefine( inventory.ItemIndex );
	if( !define )
	{
		/// 링크된 아이템이 없으면 그냥 출력
		switch( kind )
		{
		case eCHAT_WHISPER:
			{
				if( mpWhisper && mpWhisper->IsPress() == true )
				{
					unsigned long bRecv = (unsigned long)exdata;
					unsigned long textIdx = 0;
					unsigned long textIdx1 = 0;

					if( bRecv >= 1 )	
					{
						textIdx = 9999;
						textIdx1 = 10035;
					}
					else
					{
						textIdx = 9998;
						textIdx1 = 10034;
					}

					::_stprintf_s( temp, UIMAN->GetUIText( textIdx1 ), msg );

					mpChatBox->AddNewRow( UIMAN->GetUIText( textIdx ), COLOR_WHISPER );
					mpChatBox->AddPasteRow( name, nameColor );
					mpChatBox->AddPasteRow( temp, COLOR_WHISPER );
				}
			}
			break;
		case eCHAT_PARTYUNION:
			{
				if( mpParty && mpParty->IsPress() == true && exdata != 0)
				{
					unsigned long playerIdx = (unsigned long)exdata;
					int groupIdx = PARTYUNIONMAN->GetGroup( playerIdx );

					unsigned long textIndex = 0;
					switch( groupIdx )
					{
					case eUNION_GROUP1:	textIndex = 1410; break;
					case eUNION_GROUP2: textIndex = 1411; break;
					case eUNION_GROUP3: textIndex = 1412; break;
					default:			assert(0);		  return;
					}

					::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 12706 ), msg );

					mpChatBox->AddNewRow(	GAMERESOURCEMAN->GetGameText( 12701 ), COLOR_PARTY );
					mpChatBox->AddPasteRow( GAMERESOURCEMAN->GetGameText( textIndex ), COLOR_PARTY );
					mpChatBox->AddPasteRow( name, nameColor );
					mpChatBox->AddPasteRow( temp, COLOR_PARTY );
				}
			}
			break;
		case eCHAT_SYSTEM:		
		case eCHAT_NOTICE:		
		case eCHAT_GM:
			{
				::_stprintf_s( temp, UIMAN->GetUIText( 10000 ), name, msg );

				unsigned long textIdx	= CHATMANAGER->GetInfoTextIndex( kind );
				unsigned long color		= CHATMANAGER->GetTextColor( kind );
				InsertText( kind, textIdx, temp, color );				
			}	
			break;
		default:
			{
				cUINode* checkNode = NULL;

				switch( kind )
				{
				case eCHAT_NORMAL:		checkNode = mpNormal;		break;
				case eCHAT_PARTY:		checkNode = mpParty;		break;
				case eCHAT_GUILD:		checkNode = mpGuild;		break;
				case eCHAT_SHOUT:		checkNode = mpShout;		break;
				case eCHAT_TRADE:		checkNode = mpTrade;		break;
				case eCHAT_WHISPER:		checkNode = mpWhisper;		break;
				case eCHAT_GM:			checkNode = mpNormal;		break;
				case eCHAT_MEGAPHONE:	checkNode = mpMegaPhone;	break;
				}

				if( !(checkNode && checkNode->IsPress() == true) )
					return;

				::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 12706 ), msg );

				unsigned long color		= CHATMANAGER->GetTextColor( kind );
				unsigned long textIdx	= CHATMANAGER->GetInfoTextIndex( kind );

				mpChatBox->AddNewRow( GAMERESOURCEMAN->GetGameText( textIdx ), color );
				mpChatBox->AddPasteRow( name, nameColor );
				mpChatBox->AddPasteRow( temp, color );

			}
			break;
		}
	}
	else
	{
		/// 링크된 아이템이 있으면 연결정보 출력
		unsigned long itemColor = COLOR_WHITE;
		ITEMMAN->GetTooltipColor( define->GetTipType(), itemColor );

		TCHAR itemName[256] = {0,};

		/// 강화단계 + 이름 출력
		if( inventory.enhanced > 0 )
		{
			::_stprintf_s( itemName, GAMERESOURCEMAN->GetGameText(249), inventory.enhanced, define->GetName() );
		}
		else
		{
			::_stprintf_s( itemName, GAMERESOURCEMAN->GetGameText(241), define->GetName() );
		}

		switch( kind )
		{
		case eCHAT_NORMAL:
			{	
				if( mpNormal && mpNormal->IsPress() == true )
				{
					mpChatBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 12700 ), COLOR_WHITE );
					mpChatBox->AddPasteRow( name, nameColor );
					mpChatBox->AddPasteRow( GAMERESOURCEMAN->GetGameText( 12708 ), COLOR_WHITE );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_WHITE );
				}
			}
			break;
		case eCHAT_PARTY:
			{
				if( mpParty && mpParty->IsPress() == true )
				{
					mpChatBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 12701 ), COLOR_PARTY );
					mpChatBox->AddPasteRow( name, nameColor );
					mpChatBox->AddPasteRow( GAMERESOURCEMAN->GetGameText( 12708 ), COLOR_PARTY );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_PARTY );
				}
			}
			break;
		case eCHAT_PARTYUNION:
			{
				if( mpParty && mpParty->IsPress() == true && exdata != 0)
				{
					unsigned long playerIdx = (unsigned long)exdata;
					int groupIdx = PARTYUNIONMAN->GetGroup( playerIdx );

					unsigned long textIndex = 0;
					switch( groupIdx )
					{
					case eUNION_GROUP1:	textIndex = 1410; break;
					case eUNION_GROUP2: textIndex = 1411; break;
					case eUNION_GROUP3: textIndex = 1412; break;
					default:			assert(0);		  return;
					}
					
					mpChatBox->AddNewRow(	GAMERESOURCEMAN->GetGameText( 12701 ), COLOR_PARTY );
					mpChatBox->AddPasteRow( GAMERESOURCEMAN->GetGameText( textIndex ), COLOR_PARTY );
					mpChatBox->AddPasteRow( name, nameColor );
					mpChatBox->AddPasteRow( GAMERESOURCEMAN->GetGameText( 12708 ), COLOR_PARTY );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_PARTY );
				}
			}
			break;
		case eCHAT_GUILD:
			{
				if( mpGuild && mpGuild->IsPress() == true )
				{
					mpChatBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 12702 ), COLOR_GUILD );
					mpChatBox->AddPasteRow( name, nameColor );
					mpChatBox->AddPasteRow( GAMERESOURCEMAN->GetGameText( 12708 ), COLOR_GUILD );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_GUILD );
				}
			}
			break;
		case eCHAT_TRADE:
			{
				if( mpTrade && mpTrade->IsPress() == true )
				{
					mpChatBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 12703 ), COLOR_TRADE );
					mpChatBox->AddPasteRow( name, nameColor );
					mpChatBox->AddPasteRow( GAMERESOURCEMAN->GetGameText( 12708 ), COLOR_TRADE );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_TRADE );
				}
			}
			break;
		case eCHAT_WHISPER:
			{
				if( mpWhisper && mpWhisper->IsPress() == true )
				{
					unsigned long bRecv = (unsigned long)exdata;
					unsigned long textIdx = 0;
					unsigned long textIdx1 = 0;

					if( bRecv >= 1 )	
					{
						textIdx = 9999;
						textIdx1 = 10037;
					}
					else
					{
						textIdx = 9998;
						textIdx1 = 10036;
					}

					mpChatBox->AddNewRow( UIMAN->GetUIText( textIdx ), COLOR_WHISPER );
					mpChatBox->AddPasteRow( name, nameColor );
					mpChatBox->AddPasteRow( UIMAN->GetUIText( textIdx1 ), COLOR_WHISPER );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_WHISPER );
				}

				AddWhisperUser( name );
			}
			break;
		case eCHAT_GM:
			{	
				if( mpNormal && mpNormal->IsPress() == true )
				{
					::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 181 ), name );

					mpChatBox->AddNewRow( temp, COLOR_GM );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_GM );
				}
			}
			break;
		case eCHAT_NOTICE: ///현재는 강화 메시지만 처리하고 있음.
			{
				// 이름 존재
				if( name != 0 )
				{
					::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 268 ), name );

					unsigned long chatMsgcolor = 0xFFA6B27D;
					mpChatBox->AddNewRow( temp, chatMsgcolor );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, chatMsgcolor );

					cStringT str;
					str.Format( GAMERESOURCEMAN->GetGameText( 270 ), name );
					str += itemName;
					str += msg;
					UIMAN->AddEnhanceText( cFontAgent::eFont_System, str.Cstr(), COLOR_WHITE, 160 );
				}
				// 누군가의
				else
				{
					unsigned long chatMsgcolor = 0xFFA6B27D;
					mpChatBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 269 ), chatMsgcolor );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, chatMsgcolor );

					cStringT str;
					str += GAMERESOURCEMAN->GetGameText( 271 );
					str += itemName;
					str += msg;
					UIMAN->AddEnhanceText( cFontAgent::eFont_System, str.Cstr(), COLOR_WHITE, 160 );
				}
			}
			break;
		case eCHAT_MEGAPHONE:
			{
				if( mpMegaPhone && mpMegaPhone->IsPress() == true )
				{
					mpChatBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 12705 ), COLOR_MEGAPHONE );
					mpChatBox->AddPasteRow( name, nameColor );
					mpChatBox->AddPasteRow( GAMERESOURCEMAN->GetGameText( 12708 ), COLOR_MEGAPHONE );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_MEGAPHONE );
				}
			}
			break;
		default: 
			assert(0);
			break;
		}
	}
}

void cChatWindow::Clear()
{
	if( mpChatBox )
		mpChatBox->Clear();

	if( mpChatEdit )
		mpChatEdit->Clear();
}

/// 주의사항 : 
void cChatWindow::SetFocusToEdit()
{
	if( mpChatEdit )
		mpChatEdit->SetFocus();
}

/// 주의사항 :
void cChatWindow::SetChatText( LPCTSTR text )
{
	if( mpChatEdit )
		mpChatEdit->SetText( text );
}

void cChatWindow::AddExtraWindow( cUIWindow* w )
{
	if( w )
		mExtraList.PushBack( w );
}

///
void cChatWindow::AddWhisperUser( const cStringT& name )
{
	/// 기존의 등록된 유저가 있는지 찾기
	cWhisperList::cIterator i = mWhisperList.Begin();
	cWhisperList::cIterator end = mWhisperList.End();

	for( ; i != end; ++i )
	{
		/// 동일한 아디가 있으면, 갱신
		if( name == (*i) )
		{
			mWhisperList.Erase(i);
			break;
		}
	}

	/// 5명이 이미 등록되어있고 동일한 아이디없으면 새로 삽입
	if( mWhisperList.GetSize() >= 5 && i == end )
	{
		mWhisperList.PopFront();
	}

	mWhisperList.PushBack( name );
	mCount = -1;
}

void cChatWindow::ApplyLastWhisperUser()
{
	if( mpChatEdit == 0 )
		return;
	if( mWhisperList.GetSize() == 0 )
		return;

	cStringT name = mWhisperList.Back();

	cStringT str;
	str.Format( UIMAN->GetUIText(10005), (LPCTSTR)name.Cstr() );
	mpChatEdit->SetText( (LPCTSTR)str.Cstr() );

	SetFocusToEdit();
}

///
void cChatWindow::KeyEvent( eKeyCode code )
{
	int len = ::_tcslen( mpChatEdit->GetText() );
	if( len <= 0 )
		return;

	/// 귓속말 명령어가 입력되어있지 않으면 리턴 
	/// "/w", "/W", "/ㅈ"
	if( ::_tcsncmp( mpChatEdit->GetText(), UIMAN->GetUIText(10002), 2 ) != 0 &&
		::_tcsncmp( mpChatEdit->GetText(), UIMAN->GetUIText(10003), 2 ) != 0 && 
		::_tcsncmp( mpChatEdit->GetText(), UIMAN->GetUIText(10004), 2 ) != 0 )
		return; 

	if( code == eKEY_UP )
	{
		mCount++;

		if( mCount >= (int)mWhisperList.GetSize() )
		{
			mCount = 0;
		}
	}
	else if( code == eKEY_DOWN )
	{
		mCount--;

		if( mCount < 0 )
		{
			mCount = (int)mWhisperList.GetSize() - 1;
		}
	}
	else
	{
		assert(0);
		return;
	}

	///
	cWhisperList::cIterator b = (--mWhisperList.End());
	cWhisperList::cIterator end = (--mWhisperList.Begin());

	int index = 0;
	cStringT name;

	for( ; b != end; --b )
	{
		if( mCount == index )
		{
			name = (*b);
			break;
		}

		index++;
	}

	if( mpChatEdit )
	{
		/// "/w %s "
		cStringT str;
		str.Format( UIMAN->GetUIText(10005), (LPCTSTR)name.Cstr() );
		mpChatEdit->SetText( (LPCTSTR)str.Cstr() );
	}
	else
	{
		assert(0);
	}
}

void cChatWindow::OnIconDragged( cUINode* caller, unsigned int , const cUIPos& pos, bool shift )
{
	cIcon* icon = (cIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( this, DRAG_FROM_CHATLINK, icon, pos, shift );
}

/// 아이템 링크
bool cChatWindow::AddLinkItem( unsigned short slotIndex, unsigned long itemIndex )
{
	if( !mpLinkItemIcon )
	{
		assert(0);
		return false;
	}

	if( itemIndex > 0 )
	{
		if( mLinkItemIndex == itemIndex )
			return true;

		cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
		if( !define )
		{
			assert(0);
			return false;
		}

		/// 기존 락 해제
		unsigned short itemSlotIndex = mpLinkItemIcon->GetSlotIndex();
		if( itemSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
			if( oldItem )
			{
				oldItem->SetLock( false );
			}
		}

		// 아이템 인덱스로 세팅하는 경우,
		mLinkItemIndex = itemIndex;

		mpLinkItemIcon->SetSlotIndex( USHRT_MAX );
		mpLinkItemIcon->SetItemIndex( mLinkItemIndex );
		mpLinkItemIcon->ChangeImage( define->GetIconParam(), ICON_SIZE, ICON_SIZE );
	}
	else if( slotIndex != USHRT_MAX )
	{
		// 슬롯으로 세팅하는경우,
		if( slotIndex > MAX_INVENTORY )
			return false;

		cItem* item = ITEMMAN->GetItem( slotIndex );
		if( !item )
		{
			assert(0);
			return false;
		}

		cItemDefine* define = item->GetDefine();
		if( !define )
		{
			assert(0);
			return false;
		}

		/// 기존 등록 아이템 삭제
		unsigned short itemSlotIndex = mpLinkItemIcon->GetSlotIndex();
		if( itemSlotIndex == slotIndex )
			return true;

		if( itemSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
			if( oldItem )
			{
				oldItem->SetLock( false );
			}
		}

		/// 참조 슬롯 인덱스
		mpLinkItemIcon->SetSlotIndex( slotIndex );
		item->SetLock( true );

		/// 아이템 이미지 체인지
		mpLinkItemIcon->SetItemIndex( item->GetIndex() );
		mpLinkItemIcon->ChangeImage( define->GetIconParam(), ICON_SIZE, ICON_SIZE );
		cItem* newItem = ITEMMAN->GetItem( slotIndex );
		if( newItem )
			mpLinkItemIcon->SetSeal( newItem->IsSeal() );

		mLinkItemIndex = 0;
	}
	return true;
}

void cChatWindow::DeleteLinkItem()
{
	if( mpLinkItemIcon )
	{
		unsigned short itemSlotIndex = mpLinkItemIcon->GetSlotIndex();
		if( itemSlotIndex != USHRT_MAX )
		{
			/// 아이템 락 초기화
			if( itemSlotIndex <= MAX_INVENTORY )
			{
				cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
				if( oldItem )
				{
					oldItem->SetLock( false );
				}
			}
		}

		mLinkItemIndex = 0;

		mpLinkItemIcon->SetSlotIndex( USHRT_MAX );
		mpLinkItemIcon->ChangeImage( 0 );
		mpLinkItemIcon->SetItemIndex( 0 );
		mpLinkItemIcon->SetTipType( eTOOLTIP_NONE );
		mpLinkItemIcon->SetSeal( false );
	}
}

unsigned short cChatWindow::GetLinkSlotIndex()
{
	return ( mpLinkItemIcon ) ? mpLinkItemIcon->GetSlotIndex() : USHRT_MAX;
}

unsigned long cChatWindow::GetLinkItemIndex()
{
	return mLinkItemIndex;
}

bool cChatWindow::IsLinked()
{
	if( GetLinkItemIndex() > 0 || GetLinkSlotIndex() != USHRT_MAX )
		return true;
	return false;
}

cUIRect cChatWindow::GetSelectButtonRect()
{
	cUIRect rc(0, 0, 0, 0);
	if( mpSelectSkin && mpChatBox )
	{
		rc.mLeft = GetAbsoluteRect().mLeft + mpSelectSkin->mSkinInfo->mX;
		rc.mTop = mpChatBox->GetAbsoluteRect().mBottom + mpSelectSkin->mSkinInfo->mY;
		rc.mRight = rc.mLeft + mpSelectSkin->mSkinInfo->mWidth;
		rc.mBottom = rc.mTop + mpSelectSkin->mSkinInfo->mHeight;
	}
	return rc;
}

LPCTSTR cChatWindow::GetWhisperName()
{
	return ( mpChatEditWhisper ) ? mpChatEditWhisper->GetText() : 0;
}

unsigned int cChatWindow::GetMaxRowInPage()
{
	return ( mpChatBox ) ? mpChatBox->GetMaxRowInPage() : 0;
}

void cChatWindow::InitBoxSize( unsigned int size )
{
	if( size == 0 )
		return;

	if( size > mpChatBox->GetMaxRowInPage() )
	{
		int row = size - mpChatBox->GetMaxRowInPage();
		int rowHeight = mpChatBox->GetRowHeight() * row;
		UpdateSize( rowHeight );
	}
	else if( size < mpChatBox->GetMaxRowInPage() )
	{
		int row = mpChatBox->GetMaxRowInPage() - size;
		int rowHeight = mpChatBox->GetRowHeight() * row;
		UpdateSize( -rowHeight );
	}
}

void cChatWindow::SetPress( bool normal, bool party, bool guild, bool shout, bool trade, bool whisper, bool system, bool megaphone )
{
	if( mpNormal )
		mpNormal->SetPress( normal );

	if( mpParty )
		mpParty->SetPress( party );

	if( mpGuild )
		mpGuild->SetPress( guild );

	if( mpShout )
		mpShout->SetPress( shout );

	if( mpTrade )
		mpTrade->SetPress( shout );

	if( mpWhisper )
		mpWhisper->SetPress( whisper );
	
	if( mpSystem )
		mpSystem->SetPress( system );

	if( mpMegaPhone )
		mpMegaPhone->SetPress( megaphone );
}

void cChatWindow::SetMoveLock( bool lock )
{
	if( mpMoveLock )
		mpMoveLock->SetPress( lock );
}

bool cChatWindow::IsLockedMove()
{
	return ( mpMoveLock ) ? mpMoveLock->IsPress() : false;
}

bool cChatWindow::IsPressNormal()
{
	return ( mpNormal && mpNormal->IsPress() == true ) ? true : false;
}

bool cChatWindow::IsPressParty()
{
	return ( mpParty && mpParty->IsPress() == true ) ? true : false;
}

bool cChatWindow::IsPressGuild()
{
	return ( mpGuild && mpGuild->IsPress() == true ) ? true : false;
}

bool cChatWindow::IsPressShout()
{
	return ( mpShout && mpShout->IsPress() == true ) ? true : false;
}

bool cChatWindow::IsPressTrade()
{
	return ( mpTrade && mpTrade->IsPress() == true ) ? true : false;
}

bool cChatWindow::IsPressWhisper()
{
	return ( mpWhisper && mpWhisper->IsPress() == true ) ? true : false;
}

bool cChatWindow::IsPressSystem()
{
	return ( mpSystem && mpSystem->IsPress() == true ) ? true : false;
}

bool cChatWindow::IsPressMegaPhone()
{
	return ( mpMegaPhone && mpMegaPhone->IsPress() == true ) ? true : false;
}

//////////////////////////////////////////////////////////////////
//// 버튼 윈도우
cChatButtonWindow::cChatButtonWindow()
: mpNormal(0)
, mpParty(0)
, mpGuild(0)
, mpShout(0)
, mpTrade(0)
, mpWhisper(0)
{

}

cChatButtonWindow::~cChatButtonWindow()
{

}

bool cChatButtonWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	/// 기본세팅
	mpNormal = GetChild( eUIID_CHATBUTTON_NORMAL );
	mpParty = GetChild( eUIID_CHATBUTTON_PARTY );
	mpGuild = GetChild( eUIID_CHATBUTTON_GUILD );
	mpShout = GetChild( eUIID_CHATBUTTON_SHOUT );
	mpTrade = GetChild( eUIID_CHATBUTTON_TRADE );
	mpWhisper = GetChild( eUIID_CHATBUTTON_WHISPER );
	
	if( mpNormal )
		mpNormal->SetPress( true );

	if( mpParty )
		mpParty->SetPress( true );

	if( mpGuild )
		mpGuild->SetPress( true );

	if( mpShout )
		mpShout->SetPress( true );

	if( mpTrade )
		mpTrade->SetPress( true );

	if( mpWhisper )
		mpWhisper->SetPress( true );
	return true;
}

void cChatButtonWindow::OnCommand( cUINode* , unsigned int id )
{
	cChatWindow* win = GAMEUI->GetChatWindow();
	if( win )
	{
		switch( id )
		{	
		case eUIID_CHATBUTTON_NORMAL:
			win->UpdateChatType( eCHAT_NORMAL );
			break;
		case eUIID_CHATBUTTON_PARTY:
			win->UpdateChatType( eCHAT_PARTY );
			break;
		case eUIID_CHATBUTTON_GUILD:
			win->UpdateChatType( eCHAT_GUILD );
			break;
		case eUIID_CHATBUTTON_SHOUT:
			win->UpdateChatType( eCHAT_SHOUT );
			break;
		case eUIID_CHATBUTTON_TRADE:
			win->UpdateChatType( eCHAT_TRADE );
			break;
		case eUIID_CHATBUTTON_WHISPER:
			win->UpdateChatType( eCHAT_WHISPER );
			break;
		}

		Hide();
	}
}

void cChatButtonWindow::OnMouseOtherClick( const cUIPos& pos )
{
	cChatWindow* win = GAMEUI->GetChatWindow();
	if( win )
	{
		cUIRect rect = win->GetSelectButtonRect();
		if( mAbsoluteRect.ContainPoint( pos ) == false && 
			rect.ContainPoint( pos ) == false )
		{
			Hide();
		}
	}
}

void cChatButtonWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	cChatWindow* win = GAMEUI->GetChatWindow();
	if( win )
	{
		cUIRect rect = win->GetSelectButtonRect();
		int x = rect.mLeft + mAbsoluteRect.mLeft;
		int y = rect.mTop - mAbsoluteRect.GetHeight();
		SetRelativePos( cUIPos(x, y) );
	}
}
