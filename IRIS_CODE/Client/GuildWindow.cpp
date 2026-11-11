#include "StdAfx.h"
#include "GuildWindow.h"
#include "ChatManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
//
#include "ListBox.h"
#include "Label.h"
#include "EditBox.h"
#include "Button.h"
#include "GuildManager.h"
#include "GameUIManager.h"
#include "UIImage.h"
#include "FilterManager.h"
#include "HeroInfoWindow.h"

cGuildWindow::cGuildWindow()
: mpName(0)
, mpMasterName(0)
, mpPosition(0)
, mpTotal(0)
, mpConnectCount(0)
, mpList(0)
, mpNoticeEdit(0)
, mpGuildAdd(0)
, mpGuildMemDrop(0)
, mpGuildOut(0)
, mpGuildBreak(0)
, mpGuildGivePosition(0)
, mpNotice(0)
, mpMarkChange(0)
, mpMarkImage(0)
, mpMarkSkin(0)
, mShowMark(false)
{

}

cGuildWindow::~cGuildWindow()
{
	SAFE_DELETE( mpMarkImage );
}

void cGuildWindow::Toggle()
{
	if( IsVisible() == false )
	{
		if( HERO->GetGuildIndex() == 0 )
		{
			CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1205 ) );
			return;
		}
	}

	cUIWindow::Toggle();
}

void cGuildWindow::Clear()
{
	if( mpName )
		mpName->Clear();

	if( mpMasterName )
		mpMasterName->Clear();
	
	if( mpTotal )
		mpTotal->Clear();

	if( mpConnectCount )
		mpConnectCount->Clear();

	if( mpPosition )
		mpPosition->Clear();

	if( mpList )
		mpList->Clear();

	if( mpNoticeEdit )
		mpNoticeEdit->Clear();

	/// 버튼 초기화
	if( mpGuildAdd )
		mpGuildAdd->SetEnabled( false );

	if( mpGuildGivePosition )
		mpGuildGivePosition->SetEnabled( false );

	if( mpGuildOut )
		mpGuildOut->SetEnabled( false );

	if( mpGuildMemDrop )
		mpGuildMemDrop->SetEnabled( false );

	if( mpMarkChange )
		mpMarkChange->SetEnabled( false );

	if( mpGuildBreak )
		mpGuildBreak->SetEnabled( false );
}

void cGuildWindow::HideMark()
{
	mShowMark = false;
}

void cGuildWindow::ClearList()
{
	if( mpList )
		mpList->Clear();
}

void cGuildWindow::Open()
{
}

void cGuildWindow::Close()
{
	cUIWindow::Close();

	UIMAN->HideTip();
}

void cGuildWindow::OnShow()
{
	cUIWindow::OnShow();

	if( HERO->GetGuildIndex() > 0 )
	{
		if( mpNoticeEdit )
		{
			mpNoticeEdit->Clear();

			UpdateGuildNotice( GUILDMAN->GetGuildNotice() );
		}
	}
}

void cGuildWindow::OnHide()
{
	cUIWindow::OnHide();

	UIMAN->HideTip();

	cGuildButtonWindow* win = GAMEUI->GetGuildButtonWindow();
	if( win )
		win->Hide();
}

bool cGuildWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpName = (cLabel*)GetChild( eUIID_GUILD_NAME );
	mpMasterName = (cLabel*)GetChild( eUIID_GUILD_MASTERNAME );
	mpPosition = (cLabel*)GetChild( eUIID_GUILD_POSITION );
	mpTotal = (cLabel*)GetChild( eUIID_GUILD_TOTAL );
	mpConnectCount = (cLabel*)GetChild( eUIID_GUILD_CONNECTCOUNT );
	mpList = (cListBox*)GetChild( eUIID_GUILD_MEMBERLIST );
	mpNoticeEdit = (cEditBox*)GetChild( eUIID_GUILD_NOTICEEDIT );
	
	mpGuildAdd =		  (cButton*)GetChild( eUIID_GUILD_ADD );
	mpGuildGivePosition = (cButton*)GetChild( eUIID_GUILD_GIVEPOSITION );
	mpGuildMemDrop =	  (cButton*)GetChild( eUIID_GUILD_MEMBERDROP );
	mpGuildOut =		  (cButton*)GetChild( eUIID_GUILD_OUT );
	mpMarkChange =		  (cButton*)GetChild( eUIID_GUILD_MARKCHANGE );
	mpGuildBreak =		  (cButton*)GetChild( eUIID_GUILD_BREAK );
	
	mpNotice = GetChild( eUIID_GUILD_NOTICE );

	/// 
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return 0;
	}

	mpMarkSkin = pSkin->GetNodeSkin( "Guild_Mark" );
	if( mpMarkSkin )
	{
		mpMarkImage = new cUIImage;
		if( mpMarkImage )
		{
			unsigned int x = mpMarkSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mpMarkSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
			mpMarkImage->SetScreenRect( cUIRect(x, y, x+MARK_WIDTH, y+MARK_HEIGHT) );
		}
	}

	///
	Clear();
	return true;
}

void cGuildWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	cUIWindow* win = (cUIWindow*)GAMEUI->GetGuildButtonWindow();
	if( mpGuildGivePosition && win )
	{
		int x = mpGuildGivePosition->GetAbsoluteRect().mLeft;
		int y = mpGuildGivePosition->GetAbsoluteRect().mBottom;
		win->SetRelativePos( cUIPos(x, y) );
	}

	if( mpMarkSkin && mpMarkImage )
	{
		int x = mpMarkSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		int y = mpMarkSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpMarkImage->SetScreenRect( cUIRect(x, y, x+MARK_WIDTH, y+MARK_HEIGHT) );
	}
}

void cGuildWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mpMarkImage && mShowMark )
		mpMarkImage->Draw();
}

void cGuildWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_GUILD_CLOSE:
		{
			Hide();
		}
		break;
	case eUIID_GUILD_NOTICE:
		{
			if( mpNoticeEdit )
			{
				/// 길드 공지 수정
				LPCTSTR notice = mpNoticeEdit->GetText();
				int len = ::_tcslen( notice );
				if( len > 0 )
				{
					if( FILTERMAN->CheckPartMatch( notice ) == false )
					{
						GUILDMAN->GuildNotice( notice );
						mpNoticeEdit->Clear();
					}
					else
					{
						/// 필터링 됨
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 336 ) );
					}
				}
				else
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1249 ) );
				}
			}
			else
			{
				assert(0);
			}
		}
		break;
	case eUIID_GUILD_ADD:
		{
			if( GUILDMAN->GetTotalCount() >= MAX_GUILD )
			{
				/// 길드인원 초과
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1210 ) );
			}
			else
			{
				/// 길드원 초대
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_GUILD_ADDUSERNAME, eSKIN_EDIT,
					GAMERESOURCEMAN->GetGameText( 1258 ), 
					GAMERESOURCEMAN->GetGameText( 1243 ),
					0, 0, 0, false, false, 2, MAX_NAME_SIZE );
			}
		}
		break;
	case eUIID_GUILD_MARKCHANGE:
		{
			if( HERO->GetGuildIndex() == 0 )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1205 ) );
				return;
			}

			/// 길드장인지 검사
			if( GUILDMAN->GetMasterIndex() != HERO->GetObjectID() )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1253 ) );
				return;
			}

			/// 길드 마크 변경
			NiTexture* texture = GUILDMAN->LoadPreviewGuildMark( REGIST_GUILDMARK );
			if( texture )
			{
				/// 길드마크 미리보기
				HERO->PreviewGuildMark( texture );

				cGuildMarkWindow* win = GAMEUI->GetGuildMarkWindow();
				if( win )
				{
					win->ChangeMarkRegist( texture );
					win->Show();
				}

				GUILDMAN->ClearPreviewGuildMark();
			}
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1255 ) );
			}
		}
		break;
	case eUIID_GUILD_GIVEPOSITION:
		{
			/// 길드 직위 부여
			cUIWindow* win = (cUIWindow*)GAMEUI->GetGuildButtonWindow();
			if( win )
			{
				if( win->IsVisible() )
					win->Hide();
				else
					win->Show();
			}
		}
		break;
	case eUIID_GUILD_MEMBERDROP:
		{
			if( HERO->GetGuildIndex() == 0 )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1205 ) );
				return;
			}

			/// 길드장인지 검사
			if( GUILDMAN->GetMasterIndex() != HERO->GetObjectID() )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1224 ) );
				return;
			}

			// 멤버 제명
			sGuildUserInfo* data = GetSelectUserInfo();
			if( data )
			{
				if( HERO->GetObjectID() == data->characterIdx )
				{
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1223 ) );
					return;
				}

				TCHAR str[128]={0,};
				::_stprintf_s( str, GAMERESOURCEMAN->GetGameText( 1239 ), data->name );

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_GUILD_MEMBERDROP, eSKIN_YESNO,
									str, 
									GAMERESOURCEMAN->GetGameText( 301 ),
									(void*)data->characterIdx
									);
			}
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1242 ) );
			}
		}
		break;
	case eUIID_GUILD_OUT:
		{
			if( HERO->GetGuildIndex() == 0 )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1205 ) );
				return;
			}

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_GUILD_OUT, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 1240 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case eUIID_GUILD_BREAK:
		{
			if( HERO->GetGuildIndex() == 0 )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1205 ) );
				return;
			}

			// 길드장검사
			if( GUILDMAN->GetMasterIndex() != HERO->GetObjectID() )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1227 ) );
				return;
			}

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_GUILD_BREAK, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 1241 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	}
}

///
void cGuildWindow::OnListBoxRowHovered( cUINode* , unsigned int , const cUIPos& pos, void* extraData )
{
	sGuildUserInfo* data = (sGuildUserInfo*)extraData;
	if( data )
	{
		UIMAN->HideTip();

		if( data->channelNum > 0 )
		{
			// 동일 채널시.
			if( HERO->GetChannelNum() == data->channelNum )
			{
				UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, GAMERESOURCEMAN->GetMapName( data->mapnum ) );
			}
			else
			{
				// 타채널의 일반 맵인경우
				if( data->mapnum >= MAP_MIN && data->mapnum <= MAP_MAX )
				{
					TCHAR temp[128]={0,};
					::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 1260 ), data->channelNum );
					UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, temp );
				}
				else
				{
					// 타채널의 던전 맵인경우
					UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, GAMERESOURCEMAN->GetMapName( data->channelNum ) );
				}
			}
		}
		else
		{
			UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, GAMERESOURCEMAN->GetGameText( 1236 ) );
		}
	}
}

void cGuildWindow::OnListBoxRowLeft(cUINode* , unsigned int )
{
	UIMAN->HideTip();
}

/// 길드 유저 추가
void cGuildWindow::AddGuildPlayer( sGuildUserInfo* data )
{
	if( !mpList )
	{
		assert(0);
		return;
	}

	LPCTSTR posText = GUILDMAN->GetTextIndex( data->position );

	unsigned long color = mDefaultColor;

	if( data->channelNum > 0 )
	{
		/// 다른채널에 접속중이 유저
		if( HERO->GetChannelNum() != data->channelNum )
			color = COLOR_OLIVE;
	}
	else
	{
		color = COLOR_DARKGRAY;
	}

	/// 직위
	int row = mpList->AddRow( posText, color, (void*)data );
	if( row > -1 )
	{
		/// 이름
		mpList->AddColume( row, data->name, color );

		/// 레벨
		cStringT str;
		str.Format( _T("%dLv"), data->level );
		mpList->AddColume( row, (LPCTSTR)str.Cstr(), color );

		/// 직업
		LPCTSTR jobName = GAMERESOURCEMAN->GetJobName( data->job );
		mpList->AddColume( row, jobName, color );
	}
	else
	{
//		assert(0);
	}
}

void cGuildWindow::SetGuildName( LPCTSTR name )
{
	if( mpName )
		mpName->SetText( name );
}

void cGuildWindow::SetMasterName( LPCTSTR name )
{
	if( mpMasterName )
		mpMasterName->SetText( name );
}

void cGuildWindow::SetTotalCount( unsigned int count )
{
	if( mpTotal )
		mpTotal->SetValue( count );
}

void cGuildWindow::SetConnectCount( unsigned int count )
{
	if( mpConnectCount )
		mpConnectCount->SetValue( count );
}

/// 직위변경에 따른 기능 세팅
void cGuildWindow::SetPosition( unsigned char position )
{
	if( mpPosition )
	{
		LPCTSTR str = GUILDMAN->GetTextIndex( position );
		mpPosition->SetText( str );

		cHeroInfoWindow* pHeroInfoWin = GAMEUI->GetHeroInfoWindow();
		if( pHeroInfoWin )
		{
			pHeroInfoWin->SetPosition( str );
		}

		cGuildButtonWindow* win = GAMEUI->GetGuildButtonWindow();
		if( win )
			win->SetPosition( position );

		/// 1. 길드초대버튼 - 장로 이상.
		bool set = ( position == eGUILD_MASTER || position == eGUILD_SUBMASTER  || position == eGUILD_ELDER );
		if( mpGuildAdd )
			mpGuildAdd->SetEnabled( set );

		/// 2. 직위부여버튼	- 길드장, 부길드장
		set = ( position == eGUILD_MASTER || position == eGUILD_SUBMASTER );
		if( mpGuildGivePosition )
			mpGuildGivePosition->SetEnabled( set );
		
		/// 3. 길드탈퇴버튼	- 길드장 제외
		set = ( position != eGUILD_MASTER );
		if( mpGuildOut )
			mpGuildOut->SetEnabled( set );

		/// 4. 길드원추방버튼 - 길드장만 가능
		set = ( position == eGUILD_MASTER );
		if( mpGuildMemDrop )
			mpGuildMemDrop->SetEnabled( set );

		/// 5. 마크변경버튼	 - 길드장만 가능
		set = ( position == eGUILD_MASTER );
		if( mpMarkChange )
			mpMarkChange->SetEnabled( set );

		/// 6. 길드해체버튼	 - 길드장만 가능
		set = ( position == eGUILD_MASTER );
		if( mpGuildBreak )
			mpGuildBreak->SetEnabled( set );

		// 길드장일경우
		set = ( position == eGUILD_MASTER );
		if( mpNoticeEdit )
			mpNoticeEdit->SetEnabled( set );
		if( mpNotice )
			mpNotice->SetEnabled( set );
	}
}

bool cGuildWindow::ChangeMark( NiTexture* tex )
{
	if( tex && mpMarkImage )
	{
		mpMarkImage->SetTexture( tex );
		mpMarkImage->SetTextureRect( 0, 0, MARK_WIDTH, MARK_HEIGHT );
		mShowMark = true;
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}

cUIRect cGuildWindow::GetGivePositionBtnRect()
{
	cUIRect rc(0, 0, 0, 0);
	if( mpGuildGivePosition )
		rc = mpGuildGivePosition->GetAbsoluteRect();
	return rc;
}

/// 선택된 유저정보 
sGuildUserInfo* cGuildWindow::GetSelectUserInfo()
{
	if( !mpList )
	{
		assert(0);
		return 0;
	}

	int row = mpList->GetSelectIndex();
	sGuildUserInfo* data = (sGuildUserInfo*)mpList->GetExtraData( row );
	return data;
}

unsigned int cGuildWindow::GetSelectRow()
{
	if( mpList )
		return mpList->GetSelectIndex();
	return 0;
}

void cGuildWindow::SetSelectRow( unsigned int row )
{
	if( mpList )
		mpList->ChangeSelectRow( row );
}

void cGuildWindow::UpdateGuildNotice( LPCTSTR notice )
{
	if( mpNoticeEdit )
		mpNoticeEdit->SetText( notice );
}

void cGuildWindow::SetEnableMarkChange( bool enable )
{
	if( mpMarkChange )
		mpMarkChange->SetEnabled( enable );
}

void cGuildWindow::OnNetLock( int lockTry )
{
	if( mpGuildAdd )
		mpGuildAdd->NetLock( lockTry );

	if( mpGuildMemDrop )
		mpGuildMemDrop->NetLock( lockTry );

	if( mpGuildOut )
		mpGuildOut->NetLock( lockTry );

	if( mpGuildBreak )
		mpGuildBreak->NetLock( lockTry );
}

void cGuildWindow::OnNetUnLock( int lockTry )
{
	if( mpGuildAdd )
		mpGuildAdd->NetUnLock( lockTry );

	if( mpGuildMemDrop )
		mpGuildMemDrop->NetUnLock( lockTry );

	if( mpGuildOut )
		mpGuildOut->NetUnLock( lockTry );

	if( mpGuildBreak )
		mpGuildBreak->NetUnLock( lockTry );
}


//////////////////////////////////////////////////////////////////
//// 버튼 윈도우
cGuildButtonWindow::cGuildButtonWindow()
: mpMaster(0)
, mpSubMaster(0)
, mpElder(0)
, mpUpperHouse(0)
, mpLowerHouse(0)
, mpGeneral(0)
{

}

cGuildButtonWindow::~cGuildButtonWindow()
{
}

bool cGuildButtonWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	UIMAN->AddEventOtherClick( this );

	mpMaster = GetChild( eUIID_GUILDPOSITION_MASTER );
	mpSubMaster = GetChild( eUIID_GUILDPOSITION_SUBMASTER );
	mpElder = GetChild( eUIID_GUILDPOSITION_ELDER );
	mpUpperHouse = GetChild( eUIID_GUILDPOSITION_UPPERHOUSE );
	mpLowerHouse = GetChild( eUIID_GUILDPOSITION_LOWERHOUSE );
	mpGeneral = GetChild( eUIID_GUILDPOSITION_GENERAL );

	return true;
}

void cGuildButtonWindow::OnCommand( cUINode* , unsigned int id )
{
	cGuildWindow* win = GAMEUI->GetGuildWindow();	
	if( !win )
	{
		assert(0);
		return;
	}

	//char heroPos = HERO->GetGuildPosition();

	TCHAR str[128]={0,};
	char position = eGUILD_NONE;

	switch( id )
	{
	case eUIID_GUILDPOSITION_MASTER:	/// 길드장
		if( HERO->GetGuildPosition() == eGUILD_MASTER )
			position = eGUILD_MASTER;
		break;		
	case eUIID_GUILDPOSITION_SUBMASTER:	/// 부길드장
		if( HERO->GetGuildPosition() == eGUILD_MASTER )
			position = eGUILD_SUBMASTER;
		break;
	case eUIID_GUILDPOSITION_ELDER:		/// 장로
		if( HERO->GetGuildPosition() == eGUILD_MASTER ||  HERO->GetGuildPosition() == eGUILD_SUBMASTER )
			position = eGUILD_ELDER;
		break;
	case eUIID_GUILDPOSITION_UPPERHOUSE:	/// 상원 의원
		if( HERO->GetGuildPosition() == eGUILD_MASTER ||  HERO->GetGuildPosition() == eGUILD_SUBMASTER )
			position = eGUILD_UPPERHOUSE;
		break;
	case eUIID_GUILDPOSITION_LOWERHOUSE:	/// 하원 의원
		if( HERO->GetGuildPosition() == eGUILD_MASTER ||  HERO->GetGuildPosition() == eGUILD_SUBMASTER )
			position = eGUILD_LOWERHOUSE;
		break;
	case eUIID_GUILDPOSITION_GENERAL:		/// 일반
		if( HERO->GetGuildPosition() == eGUILD_MASTER ||  HERO->GetGuildPosition() == eGUILD_SUBMASTER )
			position = eGUILD_GENERAL;
		break;
	default:
		Hide();
		return;
	}

	/// 알림창 띄우기
	if( position != eGUILD_NONE )
	{
		sGuildUserInfo* info = win->GetSelectUserInfo();
		if( info )
		{
			/// 자기자신 불가
			if( HERO->GetObjectID() != info->characterIdx )
			{
				LPCTSTR positionStr = GUILDMAN->GetTextIndex( position );
				::_stprintf_s( str, GAMERESOURCEMAN->GetGameText( 1247 ), info->name, positionStr );

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_GUILD_GIVEPOSITION, eSKIN_YESNO,
					str,
					GAMERESOURCEMAN->GetGameText( 301 ),
					(void*)position );
			}
			else
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1248 ) );
			}
		}
	}
	else
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 1245 ) );
	}

	Hide();
}

void cGuildButtonWindow::OnMouseOtherClick( const cUIPos& pos )
{
	cGuildWindow* win = GAMEUI->GetGuildWindow();
	if( win )
	{
		cUIRect rect = win->GetGivePositionBtnRect();
		if( mAbsoluteRect.ContainPoint( pos ) == false && 
			rect.ContainPoint( pos ) == false )
		{
			Hide();
		}
	}
}

void cGuildButtonWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	cGuildWindow* win = GAMEUI->GetGuildWindow();
	if( win )
	{
		cUIRect rect = win->GetGivePositionBtnRect();
		int x = rect.mLeft;
		int y = rect.mBottom;
		SetRelativePos( cUIPos(x, y) );
	}
}

void cGuildButtonWindow::SetPosition( unsigned char position )
{
	bool otherSet = false;

	if( position == eGUILD_MASTER )
	{
		if( mpMaster )
			mpMaster->SetEnabled( true );
		if( mpSubMaster )
			mpSubMaster->SetEnabled( true );

		otherSet = true;
	}
	else if( position == eGUILD_SUBMASTER )
	{
		if( mpMaster )
			mpMaster->SetEnabled( false );
		if( mpSubMaster )
			mpSubMaster->SetEnabled( false );

		otherSet = true;
	}
	else
	{
		Hide();
	}
	
	if( mpElder )
		mpElder->SetEnabled( otherSet );
	if( mpUpperHouse )
		mpUpperHouse->SetEnabled( otherSet );
	if( mpLowerHouse )
		mpLowerHouse->SetEnabled( otherSet );
	if( mpGeneral )
		mpGeneral->SetEnabled( otherSet );
}

///////////////////////////////////////////////////////////////////////
// 길드마크 등록 UI
cGuildMarkWindow::cGuildMarkWindow()
: mpMarkImage(0)
, mpMarkSkin(0)
{
}

cGuildMarkWindow::~cGuildMarkWindow()
{
	SAFE_DELETE( mpMarkImage );
}

void cGuildMarkWindow::OnShow()
{
	cGuildWindow* win = GAMEUI->GetGuildWindow();
	if( win )
	{
		win->SetEnableMarkChange( false );
	}
}

void cGuildMarkWindow::OnHide()
{
	SAFE_DELETE( mpMarkImage );

	cGuildWindow* win = GAMEUI->GetGuildWindow();
	if( win )
	{
		win->SetEnableMarkChange( true );
	}
}

void cGuildMarkWindow::HideESC( bool onsound )
{
	/// 미리보기 취소
	HERO->UpdateGuildMark();

	cUIWindow::HideESC( onsound );
}

bool cGuildMarkWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return 0;
	}

	mpMarkSkin = pSkin->GetNodeSkin( "GuildMark_Mark" );
	if( !mpMarkSkin )
	{
		assert(0);
		return 0;
	}
	return true;
}

void cGuildMarkWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mpMarkImage )
		mpMarkImage->Draw();
}

void cGuildMarkWindow::OnCommand( cUINode* , unsigned int id )
{
	if( id == eUIID_GUILDMARK_YES )
	{
		GUILDMAN->GuildMarkUpdate();
	}

	/// 미리보기 취소
	HERO->UpdateGuildMark();
	Hide();
}

void cGuildMarkWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpMarkSkin && mpMarkImage )
	{
		unsigned int x = mpMarkSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpMarkSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpMarkImage->SetScreenRect( cUIRect(x, y, x+MARK_WIDTH, y+MARK_HEIGHT) );
	}
}

bool cGuildMarkWindow::ChangeMarkRegist( NiTexture* tex )
{
	if( mpMarkSkin && tex )
	{
		/// 텍스처 설정
		mpMarkImage = new cUIImage( tex );
		unsigned int x = mpMarkSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpMarkSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;

		mpMarkImage->SetTextureRect( 0, 0, MARK_WIDTH, MARK_HEIGHT );
		mpMarkImage->SetScreenRect( cUIRect(x, y, x+MARK_WIDTH, y+MARK_HEIGHT) );
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}
