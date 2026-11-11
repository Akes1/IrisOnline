#include "stdafx.h"
#include "MailSheet.h"

#include "UIImage.h"
#include "SpinBox.h"
#include "MailWindow.h"
#include "GameUIManager.h"
#include "NumberEditBox.h"
#include "MultiEditBox.h"
#include "Label.h"
#include "ItemIcon.h"
#include "ItemManager.h"
#include "DragWindow.h"
#include "MoneyWindow.h"
#include "ChatManager.h"
#include "NpcScript.h"
#include "Npc_Common.h"
#include "TipWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "TextBox.h"
#include "GameResourceManager.h"
#include "Button.h"
#include "Player_Common.h"
#include "ItemMixWindow.h"
#include "FontAgent.h"
#include "FilterManager.h"

cMailListSheet::cMailListSheet( eUINodeType type )
: cUIWindow( type )
, mpDelete(0)
, mpListBox(0)
, mpPageSpin(0)
, mImageX1(0)
, mImageHeight1(0)
, mImageX2(0)
, mImageHeight2(0)
, mpSend(0)
, mpAllSelect(0)
{
	::memset( mpImageData, 0, sizeof(mpImageData) );
}

cMailListSheet::~cMailListSheet()
{
	for( int i = 0; i < 10; ++i )
	{
		SAFE_DELETE( mpImageData[i].mpItemAdd );
		SAFE_DELETE( mpImageData[i].mpNewMail );
	}

	Clear();
}

void cMailListSheet::Clear()
{
	if( mpListBox )
		mpListBox->Clear();

	if( mpAllSelect )
		mpAllSelect->SetEnabled( false );

	if( mpDelete )
		mpDelete->SetEnabled( false );

	if( mpSend )
		mpSend->SetEnabled( false );

	SetTotalPage( 0 );
}

void cMailListSheet::OnShow()
{
	UpdatePage();
}

bool cMailListSheet::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpDelete = (cButton*)GetChild( eUIID_MAILLIST_DELETE );
	mpListBox = (cCheckBoxListBox*)GetChild( eUIID_MAILLIST_LISTBOX );
	mpPageSpin = (cSpinBox*)GetChild( eUIID_MAILLIST_PAGESPIN );
	mpSend = GetChild( eUIID_MAILLIST_SEND );
	mpAllSelect = (cButton*)GetChild( eUIID_MAILLIST_ALL );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* itemSkin = pSkin->GetNodeSkin( "MailList_ItemAdd" );
	if( !itemSkin || !itemSkin->mpTexture )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* mailSkin = pSkin->GetNodeSkin( "MailList_NewMail" );
	if( !mailSkin || !mailSkin->mpTexture )
	{
		assert(0);
		return false;
	}

	unsigned int tx = 0;
	unsigned int ty = 0;
	unsigned int w = 0;
	unsigned int h = 0;
	unsigned int x = 0;
	unsigned int y = 0;

	// 이미지생성
	for( int i = 0; i < 10; ++i )
	{
		//
		mpImageData[i].mpItemAdd = new cUIImage( itemSkin->mpTexture );

		tx = itemSkin->mSkinInfo->mTexX;
		ty = itemSkin->mSkinInfo->mTexY;
		w = itemSkin->mSkinInfo->mWidth;
		h = itemSkin->mSkinInfo->mHeight;
		x = itemSkin->mSkinInfo->mX;
		y = itemSkin->mSkinInfo->mY;

		mpImageData[i].mpItemAdd->SetTextureRect( tx, ty, tx + w, ty + h );
		mpImageData[i].mpItemAdd->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		mImageX1 = x;
		mImageHeight1 = h;

		//
		mpImageData[i].mpNewMail = new cUIImage( mailSkin->mpTexture );

		tx = mailSkin->mSkinInfo->mTexX;
		ty = mailSkin->mSkinInfo->mTexY;
		w = mailSkin->mSkinInfo->mWidth;
		h = mailSkin->mSkinInfo->mHeight;
		x = mailSkin->mSkinInfo->mX;
		y = mailSkin->mSkinInfo->mY;

		mpImageData[i].mpNewMail->SetTextureRect( tx, ty, tx + w, ty + h );
		mpImageData[i].mpNewMail->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		mImageX2 = x;
		mImageHeight2 = h;
	}

	UpdateImage();
	SetTotalPage( 0 );
	return true;
}

void cMailListSheet::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_MAILLIST_ALL:
		{
			// 전체선택
			int topIdx = mpListBox->GetCurrentTopIndex();
			int count = mpListBox->GetRowCount() - mpListBox->GetCurrentTopIndex();
			for( int i = topIdx; i < topIdx + count; ++i )
			{
				mpListBox->Check( true, i );
			}
		}
		break;
	case eUIID_MAILLIST_DELETE:
		{
			// 선택삭제
			int topIdx = mpListBox->GetCurrentTopIndex();
			int count = mpListBox->GetRowCount() - mpListBox->GetCurrentTopIndex();
			
			for( int i = topIdx, imgIdx = 0; i < topIdx + count; ++i, ++imgIdx )
			{
				bool check = mpListBox->GetCheck( i );
				if( check == true && mpImageData[imgIdx].mShowItem )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									 	 eMBEVENT_MAIL_SELECT_DELETE, eSKIN_YESNO,
										 GAMERESOURCEMAN->GetGameText( 790 ),
										 GAMERESOURCEMAN->GetGameText( 301 )
									   );
					return;
				}
			}
			
			SendSelectDelete();
		}
		break;
	case eUIID_MAILLIST_SEND:
		{
			unsigned int row = mpListBox->GetSelectIndex();
			long idx = (long)mpListBox->GetExtraData( row );
			POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( idx );
			if( data )
			{
				// 답장
				cMailWindow* mailWin = GAMEUI->GetMailWindow();
				if( mailWin )
				{
					// 이름 세팅
					cMailSendSheet* sheet = mailWin->GetSendSheet();
					if( sheet )
						sheet->SetName( data->fromName );

					mailWin->SetCurrent( 2 );
				}
			}
		}
		break;
	case eSPINBOX_LEFTBTN:
	case eSPINBOX_RIGNTBTN:
		{
			int page = mpPageSpin->GetSelectedIdx();

			if( page < mpPageSpin->GetSize() )
			{
				MSG_REQ_CHAT_POST_OPEN msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHAT;
				msg.Protocol = NM_CHAT_POST_OPEN_REQ;
				msg.startPage = page + 1;
				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				NetLock();
			}
		}
		break;
	}
}

void cMailListSheet::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	for( int i = 0; i < 10; ++i )
	{
		if( mpImageData[i].mpItemAdd && mpImageData[i].mShowItem )
			mpImageData[i].mpItemAdd->Draw();

		if( mpImageData[i].mpNewMail && mpImageData[i].mShowMail )
			mpImageData[i].mpNewMail->Draw();
	}
}

void cMailListSheet::OnListBoxChanged( cUINode* , unsigned int )
{
	unsigned int row = mpListBox->GetSelectIndex();
	long idx = (long)mpListBox->GetExtraData( row );
	POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( idx );
	if( data )
	{
		bool enable = ( data->type == POST_TYPE_NORMAL );
		if( mpSend )
			mpSend->SetEnabled( enable );
	}
}

void cMailListSheet::OnListBoxSelected( cUINode* , unsigned int )
{
	unsigned int row = mpListBox->GetSelectIndex();
	long idx = (long)mpListBox->GetExtraData( row );
	POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( idx );
	if( data )
	{
		// 서버에 읽음 알림
		MSG_REQ_CHAT_POST_READ msg;
		::memset( &msg, 0, sizeof(msg) );
		msg.Category = NM_CHAT;
		msg.Protocol = NM_CHAT_POST_READ_REQ;
		msg.idx = data->idx;
		NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

		// 읽음 갱신
		data->status = 1;

		// 편지 읽기 페이지로 변경
		cMailWindow* mailWin = GAMEUI->GetMailWindow();
		if( mailWin )
		{
			cMailReadSheet* sheet = mailWin->GetReadSheet();
			if( sheet )
			{
				sheet->SetData( data );
				mailWin->SetCurrent( 1 );
			}
		}

		CHATMANAGER->CheckNewMail();
	}
	else
		assert(0);
}

void cMailListSheet::UpdateRect()
{
	cUIWindow::UpdateRect();

	UpdateImage();
}

bool cMailListSheet::AddMail( POST_OPEN_RESULT* post )
{
	if( mpListBox )
	{
		cUISkin* pSkin = UIMAN->GetSkin();
		if( !pSkin )
		{
			assert(0);
			return false;
		}

		// 삭제, 유효만기이면 리턴
		if( !(post->status == 0 || post->status == 1) )
			return true;

		cUINodeSkin* pCheckSkin = pSkin->GetNodeSkin( "MailList_Check" );
		cUINodeSkin* pNoCheckSkin = pSkin->GetNodeSkin( "MailList_NoCheck" );

		cUIImage* image = 0;
		if( pCheckSkin && pNoCheckSkin )
		{
			image = new cUIImage( pNoCheckSkin->mpTexture );
			unsigned int tx = pNoCheckSkin->mSkinInfo->mTexX;
			unsigned int ty = pNoCheckSkin->mSkinInfo->mTexY;
			unsigned int x = pNoCheckSkin->mSkinInfo->mX;
			unsigned int y = pNoCheckSkin->mSkinInfo->mY;
			unsigned int w = pNoCheckSkin->mSkinInfo->mWidth;
			unsigned int h = pNoCheckSkin->mSkinInfo->mHeight;

			image->SetTextureRect( cUIRect(tx, ty, tx+w, ty+h ) );
			image->SetScreenRect( cUIRect(x, y, x+w, y+h ) );
		}

		// 유저로부터 온 메일
		int row = -1;

		switch ( post->type )
		{
			// 일반우편
		case POST_TYPE_NORMAL:
			{
				TCHAR name[256] = {0,};

				unsigned int colWidth = mpListBox->GetColumnMaxWidth( 0 );
				bool ret = UIMAN->ModifyText( post->content.title, name, 256, colWidth );
				if( ret )
				{
					row = mpListBox->AddRow( name, mDefaultColor, image, pCheckSkin, pNoCheckSkin, (void*)post->idx );
				}
				else
				{
					row = mpListBox->AddRow( post->content.title, mDefaultColor, image, pCheckSkin, pNoCheckSkin, (void*)post->idx );
				}

				if( row != -1 )
					mpListBox->AddColume( row, post->fromName, mDefaultColor );
			}
			break;
			// NPC로 부터 온 메일
		case POST_TYPE_AGENT_CANCEL:
			{
				sNPCList* pNpcList = NPCSCRIPT->GetNPCList( 41 );
				if( !pNpcList )
				{
					assert(0);
					return false;
				}

				TCHAR npcName[256] = {0,};
				_stprintf_s( npcName, _T("[NPC] %s"), pNpcList->mName );

				// 제목
				row = mpListBox->AddRow( GAMERESOURCEMAN->GetGameText( 788 ), mDefaultColor, image, pCheckSkin, pNoCheckSkin, (void*)post->idx );
				if( row != -1 )
					mpListBox->AddColume( row, npcName, mDefaultColor );
			}
			break;
		case POST_TYPE_AGENT_SALE:
			{
				sNPCList* pNpcList = NPCSCRIPT->GetNPCList( 41 );
				if( !pNpcList )
				{
					assert(0);
					return false;
				}

				TCHAR npcName[256] = {0,};
				_stprintf_s( npcName, _T("[NPC] %s"), pNpcList->mName );

				// 제목
				row = mpListBox->AddRow( GAMERESOURCEMAN->GetGameText( 775 ), mDefaultColor, image, pCheckSkin, pNoCheckSkin, (void*)post->idx );
				if( row != -1 )
					mpListBox->AddColume( row, npcName, mDefaultColor );
			}
			break;
		case POST_TYPE_AGENT_PURCHASE:
			{
				sNPCList* pNpcList = NPCSCRIPT->GetNPCList( 41 );
				if( !pNpcList )
				{
					assert(0);
					return false;
				}

				TCHAR npcName[256] = {0,};
				_stprintf_s( npcName, _T("[NPC] %s"), pNpcList->mName );

				// 제목
				row = mpListBox->AddRow( GAMERESOURCEMAN->GetGameText( 787 ), mDefaultColor, image, pCheckSkin, pNoCheckSkin, (void*)post->idx );
				if( row != -1 )
					mpListBox->AddColume( row, npcName, mDefaultColor );
			}
			break;
		}

		// 도착시간
		TCHAR dayStr[256] = {0,};
		_stprintf_s( dayStr, _T("%02d.%02d %02d:%02d"), post->registerDate.wMonth, 
			post->registerDate.wDay,
			post->registerDate.wHour,
			post->registerDate.wMinute );

		mpListBox->AddColume( row, dayStr, mDefaultColor );

		// 보관일
		cStringT timeStr;
		timeStr.Format( UIMAN->GetUIText( 111 ), (post->validThru < 0)? 0:post->validThru );
		mpListBox->AddColume( row, (LPCTSTR)timeStr.Cstr(), mDefaultColor );
	}
	return true;
}

// 위치 갱신
void cMailListSheet::UpdateImage()
{
	if( !mpListBox )
		return;

	cUIRect rc = mpListBox->GetAbsoluteRect();
	unsigned int rowHeight = mpListBox->GetRowHeight();
	unsigned int y1 = (unsigned int)((rowHeight - mImageHeight1) * 0.5f);
	unsigned int y2 = (unsigned int)((rowHeight - mImageHeight2) * 0.5f);

	for( int i = 0; i < 10; ++i )
	{
		unsigned int y = rowHeight * i + rc.mTop;

		if( mpImageData[i].mpItemAdd )
			mpImageData[i].mpItemAdd->SetPos( mImageX1 + rc.mLeft, y + y1 );

		if( mpImageData[i].mpNewMail )
			mpImageData[i].mpNewMail->SetPos( mImageX2 + rc.mLeft, y + y2 );
	}
}

void cMailListSheet::SetTotalPage( unsigned int total )
{
	if( mpPageSpin )
	{
		mpPageSpin->Clear();

		if( total > 0 )
		{
			cStringT str;
			for( unsigned int i = 0; i < total; ++i )
			{
				str.Format( _T("%d / %d"), i + 1, total );
				mpPageSpin->AddData( (LPCTSTR)str.Cstr() );
			}
		}

		if( total <= 1 )
			mpPageSpin->Hide();
		else
			mpPageSpin->Show();
	}

	UpdatePage();

	if( mpListBox )
	{
		bool enable = (mpListBox->GetRowCount() > 0);

		if( mpAllSelect )
			mpAllSelect->SetEnabled( enable );

		if( mpDelete )
			mpDelete->SetEnabled( enable );

		if( mpListBox->GetRowCount() > 0 )
		{
			// 기본선택 변경
			unsigned int row = mpListBox->GetSelectIndex();
			long idx = (long)mpListBox->GetExtraData( row );
			POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( idx );
			if( data )
			{
				bool enable = ( data->type == POST_TYPE_NORMAL );
				if( mpSend )
					mpSend->SetEnabled( enable );
			}
		}
	}
}

void cMailListSheet::ChangePage( unsigned int page )
{
	if( page > 0 )
	{
		if( mpPageSpin )
		{
			mpPageSpin->SetSelectIdx( page-1 );
			UpdatePage();
		}
	}
}

/// 페이지 버튼 등 재 세팅
void cMailListSheet::UpdatePage()
{
	if( mpListBox && mpPageSpin )
	{
		int page = mpPageSpin->GetSelectedIdx();

		/// 리스트박스 갱신
		mpListBox->ChangeTopIndex( page * 10 );

		/// 버튼 갱신
		int count = mpListBox->GetRowCount() - mpListBox->GetCurrentTopIndex();

		unsigned int topIdx = mpListBox->GetCurrentTopIndex();
		for( int i = 0; i < 10; ++i )
		{
			if( i < count )
			{
				long idx = (long)mpListBox->GetExtraData( topIdx + i );
				POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( idx );
				if( data )
				{
					// 읽음여부
					mpImageData[i].mShowMail = (data->status==0);

					// 아이템 첨부여부
					if( data->money > 0 || data->inventory1.idx > 0 || data->inventory2.idx > 0 || data->inventory3.idx > 0 )
					{
						mpImageData[i].mShowItem = true;
					}
					else
						mpImageData[i].mShowItem = false;
				}
				else
				{
					mpImageData[i].mShowItem = false;
					mpImageData[i].mShowMail = false;
				}
			}
			else
			{
				mpImageData[i].mShowItem = false;
				mpImageData[i].mShowMail = false;
			}
		}
	}
}

void cMailListSheet::SendSelectDelete()
{
	// 선택삭제
	int topIdx = mpListBox->GetCurrentTopIndex();
	int count = mpListBox->GetRowCount() - mpListBox->GetCurrentTopIndex();

	MSG_REQ_CHAT_POST_DELETE msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_POST_DELETE_REQ;
	msg.startPage = mpPageSpin->GetSelectedIdx();

	for( int i = topIdx; i < topIdx + count; ++i )
	{
		bool check = mpListBox->GetCheck( i );
		if( check == true )
		{
			long idx = (long)mpListBox->GetExtraData( i );

			msg.idx[msg.rowCount] = idx;
			msg.rowCount++;
		}
	}

	if( msg.rowCount > 0 )
		NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );

	NetLock();
}


//--------------------------------------------------------------------
cMailReadSheet::cMailReadSheet( eUINodeType type )
:cUIWindow( type )
, mpName(0)
, mpTime(0)
, mpTitle(0)
, mpText(0)
, mpMoney(0)
, mpGetMoney(0)
, mIndex(0)
, mpDelete(0)
, mpSend(0)
, mTempArrIndex(UINT_MAX)
{
	memset( mpItemSlot, 0, sizeof(mpItemSlot) );
}

cMailReadSheet::~cMailReadSheet()
{

}

void cMailReadSheet::Clear()
{
	SetMoney( 0 );

	mIndex = 0;
	mTempArrIndex = UINT_MAX;

	for( unsigned int i = 0; i < 3; ++i )
	{
		cItemIcon* icon = mpItemSlot[i];
		if( icon )
		{
			icon->SetSlotIndex( USHRT_MAX );
			icon->ChangeImage( 0 );
			icon->SetItemIndex( 0 );
			icon->SetNumber( 0 );
			icon->SetTipType( eTOOLTIP_NONE );
			icon->ShowNumber( false );
			icon->SetColor( NiColorA::WHITE );
			icon->SetSeal( false );
		}
	}

	if( mpName )
		mpName->Clear();

	if( mpTime )
		mpTime->Clear();

	if( mpTitle )
		mpTitle->Clear();

	if( mpText )
		mpText->Clear();
}

bool cMailReadSheet::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpName = (cLabel*)GetChild( eUIID_MAILREAD_NAME );
	mpTime = (cLabel*)GetChild( eUIID_MAILREAD_TIME );
	mpTitle = (cLabel*)GetChild( eUIID_MAILREAD_TITLE );
	mpText = (cTextBox*)GetChild( eUIID_MAILREAD_TEXT );
	mpMoney = (cLabel*)GetChild( eUIID_MAILREAD_MONEY );
	mpGetMoney = GetChild( eUIID_MAILREAD_GETMONEY );
	mpDelete = GetChild( eUIID_MAILREAD_DELETE );
	mpSend = GetChild( eUIID_MAILREAD_SEND );

	cString str;
	for( int i = 0; i < 3; ++i )
	{
		str.Format( "MailRead_Slot%02d", i );

		mpItemSlot[i] = new cItemIcon;
		if( mpItemSlot[i]->CreateBySkinName( str, this, i ) == false )
			return false;

		mpItemSlot[i]->SetLinked( true );
	}

	if( mpText ) 
	{
		mpText->SetDrawTextBottom( false );
	}

	Clear();
	return true;
}

void cMailReadSheet::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_MAILREAD_LIST:
		{
			// 목록보기
			cMailWindow* mailWin = GAMEUI->GetMailWindow();
			if( mailWin )
				mailWin->SetCurrent( 0 );
		}
		break;
	case eUIID_MAILREAD_DELETE:
		{
			// 삭제
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_MAIL_DELETE, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 778 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case eUIID_MAILREAD_SEND:
		{
			// 답장
			cMailWindow* mailWin = GAMEUI->GetMailWindow();
			if( mailWin )
			{
				LPCTSTR name = mpName->GetText();

				cMailSendSheet* sheet = mailWin->GetSendSheet();
				if( sheet )
					sheet->SetName( name );

				mailWin->SetCurrent( 0 );
				mailWin->SetCurrent( 2 );
			}
		}
		break;
	case eUIID_MAILREAD_GETMONEY:
		{
			POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( mIndex );
			if( data && data->money > 0 )
			{
				// 돈 찾기
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_MAIL_GETMONEY, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 777 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);

				return;
			}
		}
		break;
	case eUIID_MAILREAD_GETITEMALL:
		{
			POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( mIndex );
			if( !data )
				return;

			bool getItem = ( data->inventory1.idx > 0 || data->inventory2.idx > 0 || data->inventory3.idx > 0 );
			bool getMoney = ( data->money > 0 ) ? true : false;

			// 받을 아이템 없음
			if( !getItem && !getMoney )
				return;

			if( getMoney )
			{
				if( data->money + HERO->GetMoney() > MAX_HAVE_MONEY )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 11205 ),
						GAMERESOURCEMAN->GetGameText( 301 ),
						0, this );

					return;
				}
			}			

			unsigned int itemCount = 0;
			if( getItem )
			{
				unsigned long weight = 0;
				cItemDefine* define = 0;
				bool maxInven = false;

				define = ITEMMAN->GetItemDefine( data->inventory1.ItemIndex );
				if( define )
				{
					if( define->GetMaxHaveInven() > 0 )
					{
						sItemCount* p = ITEMMAN->GetItemCount( data->inventory1.ItemIndex );
						int haveCnt = ( p ) ? p->total : 0;
						if( haveCnt + data->inventory1.count > (int)define->GetMaxHaveInven() )
						{
							maxInven = true;
						}
					}
					
					weight += define->GetWeight() * data->inventory1.count;
					itemCount++;
				}

				define = ITEMMAN->GetItemDefine( data->inventory2.ItemIndex );
				if( define )
				{
					if( define->GetMaxHaveInven() > 0 )
					{
						sItemCount* p = ITEMMAN->GetItemCount( data->inventory2.ItemIndex );
						int haveCnt = ( p ) ? p->total : 0;
						if( haveCnt + data->inventory2.count > (int)define->GetMaxHaveInven() )
						{
							maxInven = true;
						}
					}

					weight += define->GetWeight() * data->inventory2.count;
					itemCount++;
				}

				define = ITEMMAN->GetItemDefine( data->inventory3.ItemIndex );
				if( define )
				{
					if( define->GetMaxHaveInven() > 0 )
					{
						sItemCount* p = ITEMMAN->GetItemCount( data->inventory3.ItemIndex );
						int haveCnt = ( p ) ? p->total : 0;
						if( haveCnt + data->inventory3.count > (int)define->GetMaxHaveInven() )
						{
							maxInven = true;
						}
					}

					weight += define->GetWeight() * data->inventory3.count;
					itemCount++;
				}

				// 아이템 최대 보유갯수 검사
				if( maxInven == true )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
										eMBEVENT_NONE, eSKIN_OK,
										GAMERESOURCEMAN->GetGameText( 220 ),
										GAMERESOURCEMAN->GetGameText( 301 ),
										0, this );
					return;
				}

				// 무게 체크
				if( ITEMMAN->GetWeight() + weight > HERO->GetMaxWeight() )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
										eMBEVENT_NONE, eSKIN_OK,
										GAMERESOURCEMAN->GetGameText( 208 ),
										GAMERESOURCEMAN->GetGameText( 301 ),
										0, GAMEUI->GetMailWindow() );
					return;
				}
			}

			// 빈 슬롯 찾기
			unsigned short slot[3] = {0,};
			if( getItem == true )
			{
				if( ITEMMAN->FindEmptySlotArr( slot, itemCount, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == false  )
				{
					// 빈슬롯 없음
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
						eMBEVENT_NONE, eSKIN_OK,
						GAMERESOURCEMAN->GetGameText( 782 ),
						GAMERESOURCEMAN->GetGameText( 301 ),
						0, GAMEUI->GetMailWindow() );
					return;
				}
			}

			//
			short i = 0;
			MSG_REQ_CHAT_POST_DOWNLOAD msg;
			::memset( &msg, 0, sizeof(msg));

			msg.Category = NM_CHAT;
			msg.Protocol = NM_CHAT_POST_DOWNLOAD_REQ;
			msg.idx		 = mIndex;

			if( data->inventory1.idx > 0 )
			{
				msg.inventoryNumber1 = slot[i];
				i++;
			}
			if( data->inventory2.idx > 0 )
			{
				msg.inventoryNumber2 = slot[i];
				i++;
			}
			if( data->inventory3.idx > 0 )
			{
				msg.inventoryNumber3 = slot[i];
				i++;
			}

			msg.money	 = getMoney;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			ITEMMAN->LockItemTry( ITEM_TRY_MAILDOWNLOAD );

			NetLock();
		}
		break;
	}
}

void cMailReadSheet::OnIconHovered( cUINode* caller, unsigned int, const cUIPos& pos )
{
	cItemIcon* itemIcon = (cItemIcon*)caller;
	if( itemIcon )
	{
		unsigned short slot = itemIcon->GetSlotIndex();
		POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( mIndex );
		if( data )
		{
			sInventory* inven = 0;
			switch( slot )
			{
			case 0: inven = &data->inventory1;	break;
			case 1:	inven = &data->inventory2;	break;
			case 2: inven = &data->inventory3;	break;
			default: return;
			}

			if( inven != 0 )
				UIMAN->ShowOtherItemTip( eTIP_ITEM, pos, *inven );
		}
	}
}

void cMailReadSheet::OnIconRightClicked( cUINode* caller, unsigned int id , bool )
{
	SendItemDownload( id );
}

void cMailReadSheet::OnIconDragged( cUINode* caller, unsigned int , const cUIPos& pos, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( (cUIWindow*)mpParent, DRAG_FROM_MAILREAD, icon, pos, shift );
}

void cMailReadSheet::SetData( POST_OPEN_RESULT* data )
{
	Clear();

	if( data )
	{
		mIndex = data->idx;

		switch ( data->type )
		{
		case POST_TYPE_NORMAL:
			{
				// 보낸이
				if( mpName )
					mpName->SetText( data->fromName );

				// 제목
				if( mpTitle )
					mpTitle->SetText( data->content.title );

				// 내용
				if( mpText )
					mpText->AddNewRow( data->content.message, mDefaultColor );
			}
			break;
		case POST_TYPE_AGENT_CANCEL:
			{
				sNPCList* pNpcList = NPCSCRIPT->GetNPCList( 41 );
				if( pNpcList != NULL )
				{
					TCHAR npcName[256] = {0,};
					_stprintf_s( npcName, _T("[NPC] %s"), pNpcList->mName );

					// 보낸이
					if( mpName )
						mpName->SetText( npcName );
				}
				// 제목
				if( mpTitle )
				{
					mpTitle->SetText( GAMERESOURCEMAN->GetGameText( 788 ) );
				}
				// 내용
				if( mpText )
				{
					if( data->inventory1.ItemIndex > 0 )
					{
						LPCTSTR itemName = ITEMMAN->GetItemName( data->inventory1.ItemIndex );

						TCHAR message[256] = {0,};
						_stprintf_s( message, GAMERESOURCEMAN->GetGameText( 750 ), itemName );
						mpText->AddNewRow( message, mDefaultColor );
					}
					else
					{
						mpText->AddNewRow( GAMERESOURCEMAN->GetGameText( 752 ), mDefaultColor );
					}
					
				}
			}
			break;
		case POST_TYPE_AGENT_SALE:
			{
				sNPCList* pNpcList = NPCSCRIPT->GetNPCList( 41 );
				if( pNpcList != NULL )
				{
					TCHAR npcName[256] = {0,};
					_stprintf_s( npcName, _T("[NPC] %s"), pNpcList->mName );

					// 보낸이
					if( mpName )
						mpName->SetText( npcName );
				}
				// 제목
				if( mpTitle )
				{
					mpTitle->SetText( GAMERESOURCEMAN->GetGameText( 775 ) );
				}
				// 내용
				if( mpText )
				{
					LPCTSTR itemName = ITEMMAN->GetItemName( data->receipt.inventory.ItemIndex );

					// 등록 판매 금액
					cStringT saleStr = cNumberEditBox::FormatNumber( data->receipt.price );

					// 갯수
					cStringT countStr = cNumberEditBox::FormatNumber( data->receipt.inventory.count );

					// 수수료
					cStringT chargeStr = cNumberEditBox::FormatNumber( data->receipt.salesCharges );

					// 판매 대금
					cStringT moneyStr = cNumberEditBox::FormatNumber( (data->receipt.price * data->receipt.inventory.count) - data->receipt.salesCharges );

					TCHAR message[512] = {0,};
					_stprintf_s( message, GAMERESOURCEMAN->GetGameText( 751 ), itemName, data->fromName, 
						(LPCTSTR)saleStr.Cstr(), (LPCTSTR)countStr.Cstr(), (LPCTSTR)chargeStr.Cstr(), (LPCTSTR)moneyStr.Cstr() );
					mpText->AddNewRow( message, mDefaultColor );
				}
			}
			break;
		case POST_TYPE_AGENT_PURCHASE:
			{
				sNPCList* pNpcList = NPCSCRIPT->GetNPCList( 41 );
				if( pNpcList != NULL )
				{
					TCHAR npcName[256] = {0,};
					_stprintf_s( npcName, _T("[NPC] %s"), pNpcList->mName );

					// 보낸이
					if( mpName )
						mpName->SetText( npcName );
				}
				// 제목
				if( mpTitle )
				{
					mpTitle->SetText( GAMERESOURCEMAN->GetGameText( 787 ) );
				}
				// 내용
				if( mpText )
				{
					if( data->receipt.inventory.ItemIndex > 0 )
					{
						LPCTSTR itemName = ITEMMAN->GetItemName( data->receipt.inventory.ItemIndex );

						TCHAR message[256] = {0,};
						_stprintf_s( message, GAMERESOURCEMAN->GetGameText( 789 ), itemName, data->receipt.inventory.count  );
						mpText->AddNewRow( message, mDefaultColor );
					}
				}
			}
			break;
		}

		// 도착시간
		TCHAR dayStr[256] = {0,};
		_stprintf_s( dayStr, _T("%02d.%02d %02d:%02d"), data->registerDate.wMonth, 
			data->registerDate.wDay,
			data->registerDate.wHour,
			data->registerDate.wMinute );

		if( mpTime )
			mpTime->SetText( dayStr );

		// 돈
		SetMoney( data->money );

		// 아이템
		SetItem( data->inventory1, 0 );
		SetItem( data->inventory2, 1 );
		SetItem( data->inventory3, 2 );

		// 답장버튼 enable
		bool enable = ( data->type == POST_TYPE_NORMAL );
		if( mpSend )
			mpSend->SetEnabled( enable );
	}
	else
		assert(0);
}

void cMailReadSheet::SetTime( LPCTSTR time )
{
	if( mpTime )
		mpTime->SetText( time );
}

void cMailReadSheet::SetMoney( unsigned long money )
{
	LPCTSTR str = cNumberEditBox::FormatNumber( money );
	if( mpMoney )
		mpMoney->SetText( str );

	bool enable = ( money > 0 );
	if( mpGetMoney )
		mpGetMoney->SetEnabled( enable );
}

void cMailReadSheet::SetItem( sInventory& inven, unsigned short slot )
{
	// 검사
	if( inven.idx == 0 )
		return;

	/// 아이템 등록
	cItemDefine* define = ITEMMAN->GetItemDefine( inven.ItemIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	if( slot >= 3 )
		return;

	if( !mpItemSlot[slot] )
	{
		assert(0);
		return;
	}

	mpItemSlot[slot]->SetSlotIndex( slot );
	mpItemSlot[slot]->SetItemIndex( inven.ItemIndex );
	mpItemSlot[slot]->ChangeImage( define->GetIconParam() );
	mpItemSlot[slot]->SetTipType( define->GetTipType() );
 	mpItemSlot[slot]->SetNumber( inven.count );
	mpItemSlot[slot]->SetSeal( (inven.seal==1) );
	
	// 제한 컬러
	int ret0 = ITEMMAN->CheckLimit( inven.ItemIndex );
	int ret1 = ITEMMAN->CheckLimitEtc( inven.ItemIndex );

	NiColorA color = NiColorA::WHITE;
	if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
	{
		color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
	}
	mpItemSlot[slot]->SetColor( color );

	bool show = ( define->GetCapacity() > 1 ) ? true : false;
	mpItemSlot[slot]->ShowNumber( show );
}

void cMailReadSheet::DownLoadData( MSG_RES_CHAT_POST_DOWNLOAD* msg )
{
	POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( mIndex );
	if( !data )
	{
		assert(0);
		return;
	}

	if( msg->rowCount == 1 )
	{
		// 한개 받기
		sInventory* temp = msg->inventory;

		int index = -1;
		if( data->inventory1.idx == temp->idx )
		{
			memset( &data->inventory1, 0, sizeof(data->inventory1) );
			index = 0;
		}
		else if( data->inventory2.idx == temp->idx )
		{
			memset( &data->inventory2, 0, sizeof(data->inventory3) );
			index = 1;
		}
		else if( data->inventory3.idx == temp->idx )
		{
			memset( &data->inventory3, 0, sizeof(data->inventory3) );
			index = 2;
		}

		if( index > -1 )
		{
			cItemIcon* icon = mpItemSlot[index];
			if( icon )
			{
				icon->SetSlotIndex( USHRT_MAX );
				icon->ChangeImage( 0 );
				icon->SetItemIndex( 0 );
				icon->SetNumber( 0 );
				icon->SetTipType( eTOOLTIP_NONE );
				icon->ShowNumber( false );
				icon->SetColor( NiColorA::WHITE );
				icon->SetSeal( false );
			}
		}
	}
	else if( msg->rowCount > 1 )
	{
		// 모두 받기
		memset( &data->inventory1, 0, sizeof(data->inventory1) );
		memset( &data->inventory2, 0, sizeof(data->inventory2) );
		memset( &data->inventory3, 0, sizeof(data->inventory3) );

		for( unsigned int i = 0; i < 3; ++i )
		{
			cItemIcon* icon = mpItemSlot[i];
			if( icon )
			{
				icon->SetSlotIndex( USHRT_MAX );
				icon->ChangeImage( 0 );
				icon->SetItemIndex( 0 );
				icon->SetNumber( 0 );
				icon->SetTipType( eTOOLTIP_NONE );
				icon->ShowNumber( false );
				icon->SetColor( NiColorA::WHITE );
				icon->SetSeal( false );
			}
		}
	}

	// 찾았으면 초기화
	if( msg->money == 1 )
	{
		data->money = 0;
		SetMoney( data->money );
	}
	
	// 아이템 세팅
	sInventory* inven = msg->inventory;
	for( int i = 0; i < msg->rowCount; ++i, ++inven )
	{
		ITEMMAN->NetSetItem( inven );

		///아이템 인벤토리에 셋팅 시 사운드 재생
		ITEMMAN->PlayIconSettingSound( inven->number );
	}

	if( msg->rowCount > 0 )
	{
		ITEMMAN->UpdateItem();
	}
}

void cMailReadSheet::SendItemDownload( unsigned int iconid )
{
	if( iconid >= 3 )
	{
		assert(0);
		return;
	}

	POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( mIndex );
	if( data )
	{
		sInventory* inven = 0;
		switch( iconid )
		{
		case 0:	inven = &data->inventory1; break;
		case 1: inven = &data->inventory2; break;
		case 2: inven = &data->inventory3; break;
		}

		if( inven == 0 || inven->idx == 0 )
			return;

		cItemDefine* define = ITEMMAN->GetItemDefine( inven->ItemIndex );
		if( !define )
		{
			assert(0);
			return;
		}

		// 무게 체크
		unsigned long weight = define->GetWeight() * inven->count;
		if( ITEMMAN->GetWeight() + weight > HERO->GetMaxWeight() )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 208 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, GAMEUI->GetMailWindow() );
			return;
		}

		if( define->GetMaxHaveInven() > 0 )
		{
			sItemCount* p = ITEMMAN->GetItemCount( inven->ItemIndex );
			int haveCnt = ( p ) ? p->total : 0;
			if( haveCnt + inven->count > (int)define->GetMaxHaveInven() )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 220 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, this );
				return;
			}
		}

		// 빈 슬롯 찾기
		unsigned short slotIndex = 0;
		if( ITEMMAN->FindEmptySlot( &slotIndex, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == true )
		{
			MSG_REQ_CHAT_POST_DOWNLOAD msg;
			::memset( &msg, 0, sizeof(msg));

			msg.Category = NM_CHAT;
			msg.Protocol = NM_CHAT_POST_DOWNLOAD_REQ;
			msg.idx = mIndex;
			msg.inventoryNumber1 = ( iconid == 0 )? slotIndex : 0;
			msg.inventoryNumber2 = ( iconid == 1 )? slotIndex : 0;
			msg.inventoryNumber3 = ( iconid == 2 )? slotIndex : 0;
			msg.money = 0;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

			ITEMMAN->LockItemTry( ITEM_TRY_MAILDOWNLOAD );

			NetLock();
		}
		else
		{
			// 빈슬롯 없음
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 782 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				0, GAMEUI->GetMailWindow() );
		}
	}
}

void cMailReadSheet::SendMoneyDownload()
{
	POST_OPEN_RESULT* data = CHATMANAGER->GetMailData( mIndex );
	if( data )
	{
		if( data->money > 0 )
		{
			// 우편으로 받은 돈과 인벤의 돈의 합이 한도 초과가 될때
			if( HERO->GetMoney() + data->money > MAX_HAVE_MONEY )
			{
				// 빈슬롯 없음
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 11205 ),
					GAMERESOURCEMAN->GetGameText( 301 ) );
				
				return;
			}

			MSG_REQ_CHAT_POST_DOWNLOAD msg;
			::memset( &msg, 0, sizeof(msg));

			msg.Category = NM_CHAT;
			msg.Protocol = NM_CHAT_POST_DOWNLOAD_REQ;
			msg.idx = mIndex;
			msg.money = 1;
			NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );	

			NetLock();
		}
	}
	else
		assert(0);
}

void cMailReadSheet::SendDelete()
{
	if( IsNetLock() )
		return;

	MSG_REQ_CHAT_POST_DELETE msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHAT_POST_DELETE_REQ;
	msg.startPage = 1;

	msg.rowCount = 1;
	msg.idx[0] = mIndex;

	NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );

	// lock
	NetLock();
}

//---------------------------------------------------------------------------------
cMailSendSheet::cMailSendSheet( eUINodeType type )
:cUIWindow( type )
, mpName(0)
, mpTitle(0)
, mpText(0)
, mpMoney(0)
, mpCharge(0)
, mLastSlot(0)
, mpSend(0)
, mMoney(0)
{
	::memset( mpItemSlot, 0, sizeof(mpItemSlot) );
}

cMailSendSheet::~cMailSendSheet()
{

}

void cMailSendSheet::Clear()
{
	SetMoney( 0 );
	SetCharge( MAIL_DEFAULT_CHARGE );

	for( unsigned int i = 0; i < 3; ++i )
	{
		cItemIcon* icon = mpItemSlot[i];
		if( icon )
		{
			/// 락 풀기
			unsigned short itemSlotIndex = icon->GetSlotIndex();
			if( itemSlotIndex <= MAX_INVENTORY )
			{
				cItem* invItem = ITEMMAN->GetItem( itemSlotIndex );
				if( invItem )
					invItem->SetLock( false );

				icon->SetSlotIndex( USHRT_MAX );
			}

			icon->ChangeImage( 0 );
			icon->SetItemIndex( 0 );
			icon->SetNumber( 0 );
			icon->SetTipType( eTOOLTIP_NONE );
			icon->ShowNumber( false );
			icon->SetSeal( false );
		}
	}

	if( mpName )
		mpName->Clear();

	if( mpTitle )
		mpTitle->Clear();

	if( mpText )
		mpText->Clear();

	if( mpSend )
		mpSend->SetEnabled( false );

	mMoney = 0;
	mLastSlot = 0;
}

bool cMailSendSheet::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpName = (cEditBox*)GetChild( eUIID_MAILSEND_NAME );
	mpTitle = (cEditBox*)GetChild( eUIID_MAILSEND_TITLE );
	mpText = (cMultiEditBox*)GetChild( eUIID_MAILSEND_TEXT );
	mpMoney = (cLabel*)GetChild( eUIID_MAILSEND_MONEY );
	mpCharge = (cLabel*)GetChild( eUIID_MAILSEND_CHARGE );
	mpSend = GetChild( eUIID_MAILSEND_SEND );

	cString str;
	for( int i = 0; i < 3; ++i )
	{
		str.Format( "MailSend_Slot%02d", i );

		mpItemSlot[i] = new cItemIcon;
		if( mpItemSlot[i]->CreateBySkinName( str, this, i ) == false )
			return false;
	}

	Clear();
	return true;
}

void cMailSendSheet::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mpTitle && mpName )
	{
		cUIWindow* mailWin = (cUIWindow*)GAMEUI->GetMailWindow();
		if( mailWin && mailWin->IsVisible() )
		{
			if( mpTitle->IsEdit() && mpName->IsEdit() )
			{
				if( mpSend )
					mpSend->SetEnabled( true );
			}
			else
			{
				if( mpSend )
					mpSend->SetEnabled( false );
			}
		}
	}
}

void cMailSendSheet::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_MAILSEND_SEND:
		{
			if( IsNetLock() )
				return;

			LPCTSTR name = mpName->GetText();
			int nameLen = mpName->GetLength();

			// 자기자신에게 발송 금지
			if( ::_tcscmp( name, HERO->GetName() ) == 0 )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 786 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, GAMEUI->GetMailWindow() );
				return;
			}

			unsigned long charge = MAIL_DEFAULT_CHARGE + (mLastSlot * MAIL_ITEM_CHARGE);
			if( HERO->GetMoney() >= (charge + mMoney) )
			{
				LPCTSTR title = mpTitle->GetText();
				int titleLen = mpTitle->GetLength();

				LPCTSTR text = mpText->GetText();
				int textLen = mpText->GetLength();

				if( FILTERMAN->CheckPartMatch( title ) == true )
				{
					/// 에러메시지 출력
					cStringT errorMsg;
					errorMsg.Format( GAMERESOURCEMAN->GetGameText( 155 ) );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
					return;
				}

				if( FILTERMAN->CheckPartMatch( text ) == true )
				{
					/// 에러메시지 출력
					cStringT errorMsg;
					errorMsg.Format( GAMERESOURCEMAN->GetGameText( 156 ) );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
					return;
				}

				if( nameLen > MAX_POST_NAME )
					nameLen = MAX_POST_NAME;

				if( titleLen > MAX_POST_TITLE )
					titleLen = MAX_POST_TITLE;

				if( textLen > MAX_POST_MESSAGE )
					textLen = MAX_POST_MESSAGE;

				// 보내기
				MSG_REQ_CHAT_POST_WRITE msg;
				::memset( &msg, 0, sizeof(msg) );
				msg.Category = NM_CHAT;
				msg.Protocol = NM_CHAT_POST_WRITE_REQ;

				::Sstrncpy( msg.name, LEN_POST_NAME, name, nameLen );
				::Sstrncpy( msg.title, LEN_POST_TITLE, title, titleLen );
				::Sstrncpy( msg.message, LEN_POST_MESSAGE, text, textLen );

				msg.rowCount = mLastSlot;
				msg.inventoryNumber1 = mpItemSlot[0]->GetSlotIndex();
				msg.inventoryNumber2 = mpItemSlot[1]->GetSlotIndex();
				msg.inventoryNumber3 = mpItemSlot[2]->GetSlotIndex();
				msg.money = mMoney;

				NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

				ITEMMAN->LockItemTry( ITEM_TRY_MAILSEND );

				NetLock();
			}
			else
			{
				// 수수료 부족
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 785 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, GAMEUI->GetMailWindow() );
			}
		}
		break;
	case eUIID_MAILSEND_ADDMONEY:
		GAMEUI->ShowMoneyWindow( eMONEY_IN, eMBEVENT_MAIL_MONEYIN );
		break;
	}
}

void cMailSendSheet::OnIconDragged( cUINode* caller, unsigned int , const cUIPos& pos, bool shift )
{
	cIcon* icon = (cIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( this, DRAG_FROM_MAILSEND, icon, pos, shift );
}

void cMailSendSheet::OnIconRightClicked( cUINode* caller, unsigned int , bool )
{
	cIcon* icon = (cIcon*)caller;
	if( icon )
	{
		DeleteItem( icon->GetID() );
	}
}

bool cMailSendSheet::AddItem( unsigned short slot )
{
	// 검사
	if( slot > MAX_INVENTORY )
		return false;

	/// 아이템 등록
	cItem* item = ITEMMAN->GetItem( slot );
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

	if( mLastSlot >= 3 )
		return false;

	if( !mpItemSlot[mLastSlot] )
	{
		assert(0);
		return false;
	}

	/// 거래불가 아이템 등록 안됨
	if( define->GetExchange() == eNORMAL_ITEM_EXCHAGNE_NO || define->GetExchange() == eSEALED_ITEM_EXCHAGNE_NO )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1516 ),
			GAMERESOURCEMAN->GetGameText( 301 ),
			0, GAMEUI->GetMailWindow() );
		return false;
	}

	/// 미인증 거래 가능 아이템이면서 실제로 미인증된 아이템이여야만 거래가능. 그 외엔 불가.
	if( define->GetExchange() == eSEALED_ITEM_EXCHAGNE_YES && item->IsSeal() == false )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1516 ),
			GAMERESOURCEMAN->GetGameText( 301 ),
			0, GAMEUI->GetMailWindow() );
		return false;
	}
	

	/// 락걸기 - 인벤토리 내, 참조된 아이템 
	item->SetLock( true );

	/// 참조한 슬롯인덱스 저장
	mpItemSlot[mLastSlot]->SetSlotIndex( slot );
	mpItemSlot[mLastSlot]->SetItemIndex( item->GetIndex() );
	mpItemSlot[mLastSlot]->ChangeImage( define->GetIconParam() );
	mpItemSlot[mLastSlot]->SetTipType( define->GetTipType() );
	mpItemSlot[mLastSlot]->SetNumber( item->GetCount() );
	mpItemSlot[mLastSlot]->SetSeal( item->IsSeal() );

	// 제한 컬러
	int ret0 = ITEMMAN->CheckLimit( item->GetIndex() );
	int ret1 = ITEMMAN->CheckLimitEtc( item->GetIndex() );

	NiColorA color = NiColorA::WHITE;
	if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
	{
		color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
	}
	mpItemSlot[mLastSlot]->SetColor( color );

	bool show = ( define->GetCapacity() > 1 ) ? true : false;
	mpItemSlot[mLastSlot]->ShowNumber( show );

	mLastSlot++;

	/// 수수료갱신
	SetCharge( MAIL_DEFAULT_CHARGE + (mLastSlot * MAIL_ITEM_CHARGE) );

	///아이템 슬롯에 셋팅 시 사운드 재생
	ITEMMAN->PlayIconSettingSound( slot );

	return true;
}

void cMailSendSheet::DeleteItem( unsigned int i )
{
	if( i >= 3 )
		return;

	/// 
	if( i >= mLastSlot )
	{
		assert(0);
		return;
	}

	if( !mpItemSlot[i] )
	{
		assert(0);
		return;
	}

	unsigned short itemSlotIndex = mpItemSlot[i]->GetSlotIndex();
	if( itemSlotIndex > MAX_INVENTORY )
		return;

	/// 락걸린 아이템 락풀기
	cItem* invItem = ITEMMAN->GetItem( itemSlotIndex );
	if( invItem )
	{
		invItem->SetLock( false );
	}

	/// 슬롯에서 삭제
	mpItemSlot[i]->SetSlotIndex( USHRT_MAX );
	mpItemSlot[i]->ChangeImage( 0 );
	mpItemSlot[i]->SetItemIndex( 0 );
	mpItemSlot[i]->SetTipType( eTOOLTIP_NONE );
	mpItemSlot[i]->SetNumber( 0 );
	mpItemSlot[i]->SetColor( NiColorA::WHITE );
	mpItemSlot[i]->SetSeal( false );

	UpdateItem( i );

	if( mLastSlot > 0 )
		mLastSlot--;

	SetCharge( MAIL_DEFAULT_CHARGE + (mLastSlot * MAIL_ITEM_CHARGE) );

	///아이템 슬롯에 셋팅 시 사운드 재생
	ITEMMAN->PlayIconSettingSound( itemSlotIndex );
}

void cMailSendSheet::UpdateItem( unsigned int index )
{
	int range = mLastSlot - 1;
	if( range < 0 )
	{
		assert(0);
		return;
	}

	for( int i = index; i < range; ++i )
	{
		cItemIcon* icon1 = mpItemSlot[i];
		cItemIcon* icon2 = mpItemSlot[i+1];

		if( icon1 && icon2 )
		{
			icon1->SetSlotIndex( icon2->GetSlotIndex() );
			icon1->ChangeImage( icon2->GetIconParam() );
			icon1->SetItemIndex( icon2->GetItemIndex() );
			icon1->SetTipType( icon2->GetTipType() );
			icon1->SetNumber( icon2->GetNumber() );
			icon1->ShowNumber( icon2->IsShowNumber() );
			icon1->SetColor( icon2->GetColor() );
			icon1->SetSeal( icon2->IsSeal() );
		}
		else
			assert(0);
	}

	/// 마지막 아이템 초기화
	cItemIcon* lastIcon = mpItemSlot[mLastSlot-1];
	if( lastIcon )
	{
		lastIcon->SetSlotIndex( USHRT_MAX );
		lastIcon->ChangeImage( 0 );
		lastIcon->SetTipType( eTOOLTIP_NONE );
		lastIcon->SetItemIndex( 0 );
		lastIcon->SetNumber( 0 );
		lastIcon->SetColor( NiColorA::WHITE );
		lastIcon->SetSeal( false );
	}
}

void cMailSendSheet::DeleteItemBySend()
{
	for( unsigned int i = 0; i < 3; ++i )
	{
		cItemIcon* icon = mpItemSlot[i];
		if( icon )
		{
			unsigned short itemSlotIndex = icon->GetSlotIndex();

			if( itemSlotIndex != USHRT_MAX )
				ITEMMAN->DeleteInventory( itemSlotIndex );

			icon->SetSlotIndex( USHRT_MAX );
			icon->ChangeImage( 0 );
			icon->SetItemIndex( 0 );
			icon->SetNumber( 0 );
			icon->SetTipType( eTOOLTIP_NONE );
			icon->ShowNumber( false );
			icon->SetColor( NiColorA::WHITE );
			icon->SetSeal( false );
		}
	}

	SetMoney( 0 );
	SetCharge( MAIL_DEFAULT_CHARGE );

	if( mpName )
		mpName->Clear();

	if( mpTitle )
		mpTitle->Clear();

	if( mpText )
		mpText->Clear();

	if( mpSend )
		mpSend->SetEnabled( false );

	mMoney = 0;
	mLastSlot = 0;
}

void cMailSendSheet::SetMoney( unsigned long money )
{
	if( mpMoney )
	{
		if( money > MAX_TRADE_MONEY )
			money = MAX_TRADE_MONEY;

		mMoney = money;

		LPCTSTR str = cNumberEditBox::FormatNumber( money );
		mpMoney->SetText( str );
	}
}

void cMailSendSheet::SetName( LPCTSTR name )
{
	if( mpName )
	{
		mpName->SetText( name );
		mpName->SetFocus();
	}
}

void cMailSendSheet::SetCharge( unsigned long charge )
{
	if( mpCharge )
	{
		LPCTSTR str = cNumberEditBox::FormatNumber( charge );
		mpCharge->SetText( str );
	}
}
