#include "StdAfx.h"
#include "ChatExtraWindow.h"
#include "PlaneObject.h"
#include "TextBox.h"
#include "ChatManager.h"
#include "ItemManager.h"
#include "PartyUnionManager.h"
#include "Gameresourcemanager.h"
#include "GameUIManager.h"

cChatExtraWindow::cChatExtraWindow()
: mButtonH(0)
{
}

cChatExtraWindow::~cChatExtraWindow()
{
	SAFE_DELETE( mpChatBoxImageDown );
	SAFE_DELETE( mpChatBoxImageUp );
	SAFE_DELETE( mpChatBoxImage );
}

/// 주의사항 : 
bool cChatExtraWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpChatBox	= (cTextBox*)GetChild( eUIID_EXTRA_CHATEXBOX );
	mpNormal	= GetChild( eUIID_EXTRA_NOMAL );
	mpParty		= GetChild( eUIID_EXTRA_PARTY );
	mpGuild		= GetChild( eUIID_EXTRA_GUILD );
	mpShout		= GetChild( eUIID_EXTRA_SHOUT );
	mpTrade		= GetChild( eUIID_EXTRA_TRADE );
	mpWhisper	= GetChild( eUIID_EXTRA_WHISPER );
	mpSystem	= GetChild( eUIID_EXTRA_SYSTEM );
	mpMegaPhone = GetChild( eUIID_EXTRA_MEGAPHONE );
	
	/// 채팅윈도우 등록
	CHATMANAGER->SetChatWindow( this );
	
	if( mpChatBox )
	{
		mpChatBox->SetOutLine( true );
		mpChatBox->SetChatMode( true );
	}

	mDefalutRowNum = mpChatBox->GetMaxRowInPage();

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 채팅 박스의 스킨 로드
	cUINodeSkin* pplaneSkin = pSkin->GetNodeSkin( "ChatBoxImage" );
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
	mpUpSkin = pSkin->GetNodeSkin( "ChatBoxImage_Up" );
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

	cUINodeSkin* downSkin = pSkin->GetNodeSkin( "ChatBoxImage_Down" );
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

	/// 윈도우 사이즈 갱신
	if( mpChatBox )
	{
		cUISize winSize;
		winSize.mWidth = GetAbsoluteRect().GetWidth();
		winSize.mHeight = mpChatBox->GetRelativeRect().mTop + mpChatBox->GetRelativeRect().GetHeight();
		SetRelativeSize( winSize );
	}

	InitBoxSize( CHATEXTRA_DEFAULT_ROWNUM );

	return true;
}

/// 주의사항 : 
void cChatExtraWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	/// 채팅박스 배경이미지
	if( mpChatBoxImage )
	{
		int x = mpChatBox->GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mTop;
		unsigned int w = mpChatBox->GetAbsoluteRect().GetWidth();
		unsigned int h = mpChatBox->GetAbsoluteRect().GetHeight();
		mpChatBoxImage->SetScreenRect( x, y, w, h );
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
}

void cChatExtraWindow::UpdateSkin()
{
	UpdateRect();
}

void cChatExtraWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	if( mpChatBoxImage )
		mpChatBoxImage->Draw();

	if( mpChatBoxImageUp )
		mpChatBoxImageUp->Draw();

	if( mpChatBoxImageDown )
		mpChatBoxImageDown->Draw();

	cUIWindow::OnRender( pKeeper );
}

void cChatExtraWindow::OnMouseMove( const cUIPos& pos )
{
	cChatWindow* chat = GAMEUI->GetChatWindow();
	if( chat && chat->IsLockedMove() == false )
	{
		cUIWindow::OnMouseMove( pos );
	}
}

void cChatExtraWindow::OnButtonDowned(cUINode*, unsigned int id)
{
	switch( id )
	{
	case eUIID_EXTRA_SIZE_PLUS_BUTTON:
		{
			/// 증가
			unsigned int maxNum = mpChatBox->GetMaxRowInPage();

			if( maxNum < CHATEXTRA_MAXROWNUM )
			{
				int rowHeight = mpChatBox->GetRowHeight();
				UpdateSize( rowHeight );
			}
		}
		break;
	case eUIID_EXTRA_SIZE_MINUS_BUTTON:
		{
			/// 감소
			unsigned int maxNum = mpChatBox->GetMaxRowInPage();

			if( maxNum > mDefalutRowNum )
			{
				int rowHeight = mpChatBox->GetRowHeight();
				UpdateSize( -rowHeight );
			}
		}
		break;
	}
}

/// 주의사항 : 
void cChatExtraWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_EXTRA_CLOSE_BUTTON:
		{
			Hide();
		}
		break;
	case eUIID_EXTRA_SIZE_PLUS_BUTTON:
		{
			/// 증가
			unsigned int maxNum = mpChatBox->GetMaxRowInPage();

			if( maxNum < CHATEXTRA_MAXROWNUM )
			{
				int rowHeight = mpChatBox->GetRowHeight();
				UpdateSize( rowHeight );
			}
		}
		break;
	case eUIID_EXTRA_SIZE_MINUS_BUTTON:
		{
			/// 감소
			unsigned int maxNum = mpChatBox->GetMaxRowInPage();

			if( maxNum > mDefalutRowNum )
			{
				int rowHeight = mpChatBox->GetRowHeight();
				UpdateSize( -rowHeight );
			}
		}
		break;
	}
}

/// 주의사항 : 
void cChatExtraWindow::UpdateSize( int height )
{
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
	cUISize winSize;
	winSize.mWidth = GetAbsoluteRect().GetWidth();
	winSize.mHeight = mpChatBox->GetRelativeRect().mTop + mpChatBox->GetRelativeRect().GetHeight();
	SetRelativeSize( winSize );

	/// 캡션 윈도우 갱신
	cUIRect captionRc = GetCaptionRect();
	captionRc.mTop  = GetAbsoluteRect().GetHeight() - boxsize.mHeight;
	captionRc.mLeft = 0;
	captionRc.mRight  = captionRc.mLeft + mpChatBox->GetAbsoluteRect().GetWidth();
	captionRc.mBottom = captionRc.mTop  + mpChatBox->GetAbsoluteRect().GetHeight();
	SetCaptionRect( captionRc );


	/// 이미지갱신
	if( mpChatBoxImage )
	{
		int x = mpChatBox->GetAbsoluteRect().mLeft;
		int y = mpChatBox->GetAbsoluteRect().mTop;
		unsigned int w = mpChatBox->GetAbsoluteRect().GetWidth();
		unsigned int h = mpChatBox->GetAbsoluteRect().GetHeight();
		mpChatBoxImage->SetScreenRect( x, y, w, h );
	}

	/// 사이즈 변경에 따른 채팅박스 데이터 갱신 
	bool add = ( height >= 0 ) ? true : false;
	mpChatBox->UpdateTextBox( add );
}

/// 외부에서 채팅박스에 직접적으로 문자열을 넣을 경우 사용
void cChatExtraWindow::InsertText( eChatMsgKind msgkind, LPCTSTR text )
{
	if( !mpChatBox )
	{
		assert(0);
		return;
	}

	TCHAR temp[256]={0,};

	switch( msgkind )
	{
	case eCHAT_NORMAL:
		{
			if( mpNormal && mpNormal->IsPress() == true )
			{
				::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 86 ), text );
				mpChatBox->AddNewRow( temp, COLOR_WHITE );
			}
		}
		break;
	case eCHAT_PARTY:
		{
			if( mpParty && mpParty->IsPress() == true )
			{
				::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 87 ), text );
				mpChatBox->AddNewRow( temp, COLOR_PARTY );
			}
		}
		break;
	case eCHAT_GUILD:
		{
			if( mpGuild && mpGuild->IsPress() == true )
			{
				::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 88 ), text );
				mpChatBox->AddNewRow( temp, COLOR_GUILD );
			}
		}
		break;
	case eCHAT_SHOUT:
		{
			if( mpShout && mpShout->IsPress() == true )
			{
				::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 89 ), text );
				mpChatBox->AddNewRow( temp, COLOR_SHOUT );
			}
		}
		break;
	case eCHAT_TRADE:
		{
			if( mpTrade && mpTrade->IsPress() == true )
			{
				::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 90 ), text );
				mpChatBox->AddNewRow( temp, COLOR_TRADE );
			}
		}
		break;
	case eCHAT_WHISPER:
		{
			if( mpWhisper && mpWhisper->IsPress() == true )
			{
				mpChatBox->AddNewRow( text, COLOR_WHISPER );
			}
		}
		break;
	case eCHAT_GM:
		{
			if( mpNormal && mpNormal->IsPress() == true )
			{
				::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 180 ), text );
				mpChatBox->AddNewRow( temp, COLOR_GM );
			}
		}
		break;
	case eCHAT_MEGAPHONE:
		{
			if( mpMegaPhone && mpMegaPhone->IsPress() == true )
			{				
				mpChatBox->AddNewRow( text, COLOR_MEGAPHONE );
			}
		}
		break;
	}
}

/// 아이템 거래 메세지 출력
void cChatExtraWindow::InsertItemText( sInventory inventory, LPCTSTR name, LPCTSTR msg,  ePLAYER_JOB job, eChatMsgKind kind, void* exdata )
{
	if( !mpChatBox )
	{
		assert(0);
		return;
	}

	TCHAR temp[256] = {0,};

	unsigned long nameColor = 0;

	nameColor = COLOR_WHITE;

	if(	ePLAYER_FIGHTER			<= job && job <= ePLAYER_SOULBLADER )		nameColor = COLOR_JOB_FIGHTER; 
	else if( ePLAYER_MAGE		<= job && job <= ePLAYER_HIGHPRIEST )		nameColor = COLOR_JOB_MAGE; 
	else if( ePLAYER_WANDERER	<= job && job <= ePLAYER_SNIPER )			nameColor = COLOR_JOB_WANDERER; 
	else if( ePLAYER_WARLOCK	<= job && job <= ePLAYER_MENTALIST )		nameColor = COLOR_JOB_WARLOCK; 
	else if( ePLAYER_ARMSMAN	<= job && job <= ePLAYER_HIGHLANDER )		nameColor = COLOR_JOB_ARMSMAN; 
	else if( ePLAYER_RANGER		<= job && job <= ePLAYER_SHARPSHOOTER )		nameColor = COLOR_JOB_RANGER;

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
				InsertText( kind, temp );				
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
		case eCHAT_NOTICE:
			{
				if( name != 0 )
				{
					::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 268 ), name );

					mpChatBox->AddNewRow( temp, COLOR_RED );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_RED );
				}
				else
				{
					mpChatBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 269 ), COLOR_RED );
					mpChatBox->AddPasteRow( itemName, itemColor, &inventory );
					mpChatBox->AddPasteRow( msg, COLOR_RED );
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

void cChatExtraWindow::SetPos( int i )
{
	/// 생성위치 조정하기 ( 조금씩 밀려서 생성 
	cUISkin* pSkin = UIMAN->GetSkin();
	if( pSkin )
	{
		cUINodeSkin* pNodeSkin = pSkin->GetNodeSkin( "ChatExtraWindow" );
		if( pNodeSkin )
		{
			cUIPos pos;
			pos.mX = pNodeSkin->mSkinInfo->mX + (i*CHATEXTRA_GAP);
			pos.mY = pNodeSkin->mSkinInfo->mY + (i*CHATEXTRA_GAP);
			SetRelativePos( pos );
		}
	}
}

void cChatExtraWindow::SetPress( bool normal, bool party, bool guild, bool shout, bool trade, bool whisper, bool system,  bool megaphone )
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

bool cChatExtraWindow::IsPressNormal()
{
	return ( mpNormal && mpNormal->IsPress() == true ) ? true : false;
}

bool cChatExtraWindow::IsPressParty()
{
	return ( mpParty && mpParty->IsPress() == true ) ? true : false;
}

bool cChatExtraWindow::IsPressGuild()
{
	return ( mpGuild && mpGuild->IsPress() == true ) ? true : false;
}

bool cChatExtraWindow::IsPressShout()
{
	return ( mpShout && mpShout->IsPress() == true ) ? true : false;
}

bool cChatExtraWindow::IsPressTrade()
{
	return ( mpTrade && mpTrade->IsPress() == true ) ? true : false;
}

bool cChatExtraWindow::IsPressWhisper()
{
	return ( mpWhisper && mpWhisper->IsPress() == true ) ? true : false;
}

bool cChatExtraWindow::IsPressSystem()
{
	return ( mpSystem && mpSystem->IsPress() == true ) ? true : false;
}

bool cChatExtraWindow::IsPressMegaPhone()
{
	return ( mpMegaPhone && mpMegaPhone->IsPress() == true ) ? true : false;
}
