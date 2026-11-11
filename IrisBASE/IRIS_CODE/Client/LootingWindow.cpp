#include "StdAfx.h"
#include "LootingWindow.h"
#include "ItemManager.h"
#include "UIImage.h"
#include "ItemIcon.h"
#include "TipWindow.h"
#include "GameUIManager.h"
#include "Button.h"
#include "GameResourceManager.h"
#include "ChatWindow.h"
#include "MegaphoneWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "PartyManager.h"
#include "SceneManager.h"
#include "DropItemSceneNode.h"

cLootingWindow::cLootingWindow()
: mpItemList(0)
, mpGetButton(0)
, mpCloseButton(0)
, mpLinkButton(0)
, mLinkDelayTime(0)
, mDropIndex(0)
{
	SetBackupVisible( false );
}

cLootingWindow::~cLootingWindow()
{
	/// 아이템 루팅 맵 삭제
	cItemLootList::cIterator i = mItemLootList.Begin();
	cItemLootList::cIterator end = mItemLootList.End();
	for( ; i != end; ++i )
	{
		sLootData* data = (sLootData*)(*i);
		SAFE_DELETE( data );
	}
	mItemLootList.Clear();
}

void cLootingWindow::Clear()
{
	UIMAN->HideTip();

	/// 아이템 루팅 맵 삭제
	cItemLootList::cIterator i = mItemLootList.Begin();
	cItemLootList::cIterator end = mItemLootList.End();
	for( ; i != end; ++i )
	{
		sLootData* data = (sLootData*)(*i);
		SAFE_DELETE( data );
	}
	mItemLootList.Clear();

	if( mpItemList )
		mpItemList->Clear();

	mDropIndex = 0;
}

void cLootingWindow::Open()
{
}

void cLootingWindow::Close()
{
	cUIWindow::Close();

	UIMAN->HideTip();

	cUIWindow* win = (cUIWindow*)GAMEUI->GetItemDivideWindow();
	if( win )
		win->Hide();
}

void cLootingWindow::OnShow()
{
	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);

	SetRelativePos( cUIPos( x, y) );

	mLinkDelayTime = 0;

	if( mpLinkButton )
		mpLinkButton->SetEnabled( true );

	if( mpGetButton )
		mpGetButton->Hide();

	// 파티 상태일 때, 
	if( PARTYMAN->IsHeroLeader() && PARTYMAN->GetPartyDivide() == PARTY_DIVIDE_LEADER )
	{
		// 파티장 분배시
		cItemDivideWindow* win = GAMEUI->GetItemDivideWindow();
		if( win )
			win->Show( mDropIndex );
	}
	else
	{
		if( mpGetButton )
			mpGetButton->Show();
	}

	cUIWindow::OnShow();
}

void cLootingWindow::OnHide()
{
	UIMAN->HideTip();

	/// 닫기 요청
	MSG_REQ_ITEM_GET_CLOSE msg;
	::memset( &msg, 0, sizeof(msg) );
	msg.Category = NM_ITEM;
	msg.Protocol = NM_ITEM_GET_CLOSE_REQ;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 락 걸기
	NetLock( eLOCK_CLOSE );

	cUIWindow::OnHide();
}

bool cLootingWindow::AddItem( sItemData* data )
{
	if( !data )
	{
		assert(0);
		return false;
	}

	cItemDefine* itemDefine = ITEMMAN->GetItemDefine( data->itemDefineIndex );
	if( itemDefine == 0 )
	{
		assert(0);
		return false;
	}

	/// 정보 관리
	sLootData* lootData = new sLootData;
	lootData->itemDefineIndex = data->itemDefineIndex;
	lootData->count			  = data->count;
	mItemLootList.PushBack( lootData );

	/// 테두리이미지
	cUIImage* edgeImage = 0;
	cIconParam* pTipParam = UIMAN->GetIconParam( 10 );
	if( pTipParam && itemDefine->GetTipType() != eTOOLTIP_NONE )
	{
		edgeImage = new cUIImage( pTipParam->mpTexture );

		unsigned int tx = pTipParam->mTexPos.mX + ( itemDefine->GetTipType() * ICON_SIZE );
		unsigned int ty = pTipParam->mTexPos.mY;
		edgeImage->SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );
		edgeImage->SetScreenRect( cUIRect( 0, 0, ICON_SIZE, ICON_SIZE ) );
	}

	// 봉인이미지
	cUIImage* sealImage = 0;
	cIconParam* pSealParam = UIMAN->GetIconParam( SEAL_INDEX );
	if( pSealParam && itemDefine->GetLicenseType() == 1 )
	{
		sealImage = new cUIImage( pSealParam->mpTexture );

		unsigned int tx = pSealParam->mTexPos.mX;
		unsigned int ty = pSealParam->mTexPos.mY;
		sealImage->SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );
		sealImage->SetScreenRect( cUIRect( 0, 0, ICON_SIZE, ICON_SIZE ) );
	}

	/// 아이템 이미지 생성
	cIconParam* iconParam = itemDefine->GetIconParam();
	if( iconParam )
	{
		cUIImage* pImage = new cUIImage( iconParam->mpTexture );
		if( pImage )
		{
			unsigned int tx = iconParam->mTexPos.mX;
			unsigned int ty = iconParam->mTexPos.mY;
			pImage->SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );
			pImage->SetScreenRect( cUIRect( 0, 0, ICON_SIZE, ICON_SIZE ) );

			//
			int ret0 = ITEMMAN->CheckLimit( data->itemDefineIndex );
			int ret1 = ITEMMAN->CheckLimitEtc( data->itemDefineIndex );

			NiColorA color = NiColorA::WHITE;
			if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
			{
				color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
			}
			pImage->SetColor( color );

			/// 이름 길이 조절
			TCHAR name[ITENNAME_LEN] = {0,};

			unsigned int colWidth = mpItemList->GetColumnMaxWidth( 0 );
			bool ret = UIMAN->ModifyText( itemDefine->GetName(), name, ITENNAME_LEN, colWidth );
			LPCTSTR itemname = ( ret == true ) ? name : itemDefine->GetName();

			cUIImageFont* font	= UIMAN->GetImageFont( eIAMGEFONT_ITEMCOUNT );
			bool showfont		= ( itemDefine->GetCapacity() > 1 );

			mpItemList->AddRow( itemname, mDefaultColor, pImage, edgeImage, sealImage, font, showfont, data->count, lootData );
		}
	}
	return true;
}


void cLootingWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);

	SetRelativePos( cUIPos( x, y) );
}

void cLootingWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	// 위치
	cUIWindow* win = (cUIWindow*)GAMEUI->GetItemDivideWindow();
	if( win )
	{
		cUIRect rc = GetAbsoluteRect();

		int x = mAbsoluteRect.mRight;
		int y = mAbsoluteRect.mBottom - win->GetAbsoluteRect().GetHeight();

		win->SetRelativePos( cUIPos( x, y ) );
	}
}

bool cLootingWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpItemList = (cImageAndFontDataListBox*)GetChild( eUIID_GAME_LOOTING_LIST );
	mpGetButton = (cButton*)GetChild( eUIID_GAME_LOOTING_ALL );
	mpCloseButton = (cButton*)GetChild( eUIID_GAME_LOOTING_CLOSE );
	mpLinkButton = (cButton*)GetChild( eUIID_GAME_LOOTING_LINK );
	
	return true;
}

void cLootingWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible == false )
		return;

	cUIWindow::OnProcess(deltaTime, accumTime);

	if( mLinkDelayTime != 0 )
	{
		if( mLinkDelayTime <= deltaTime )
		{
			mLinkDelayTime = 0;
			mpLinkButton->SetEnabled( true );
		}
		else
			mLinkDelayTime -= deltaTime;
	}
}

/// 주의사항 : 
void cLootingWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_LOOTING_CLOSE:
		Hide();
		break;
	case eUIID_GAME_LOOTING_ALL:
		{
			cUIWindow* win = (cUIWindow*)GAMEUI->GetItemDivideWindow();
			if( win && win->IsVisible() )
				return;
		
			/// 전체 아이템 습득처리
			/// 거래불가 아이템 검사
			cItemLootList::cIterator i = mItemLootList.Begin();
			cItemLootList::cIterator end = mItemLootList.End();
			for( ; i != end; ++i )
			{
				sLootData* data = (sLootData*)(*i);
				cItemDefine* define = ITEMMAN->GetItemDefine( data->itemDefineIndex );
				if( define ->GetExchange() == eNORMAL_ITEM_EXCHAGNE_NO || define->GetExchange() == eSEALED_ITEM_EXCHAGNE_NO )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_ITEM_NOTEXCHANGE_GET, eSKIN_YESNO,
						GAMERESOURCEMAN->GetGameText( 246 ),
						GAMERESOURCEMAN->GetGameText( 301 ),
						(void*)ITEM_GET_ALL );
					return;
				}
			}

			/// 전체 획득 요청
			ITEMMAN->TryToGetItem( (unsigned int)ITEM_GET_ALL, 0 );
		}
		break;
	case eUIID_GAME_LOOTING_LINK:
		{
			/// 아이템 전체 채팅창 링크.
			cChatWindow* chatWin = GAMEUI->GetChatWindow();
			if( chatWin && chatWin->IsVisible() )
			{
				cItemLootList::cIterator i = mItemLootList.Begin();
				cItemLootList::cIterator end = mItemLootList.End();
				for( ; i != end; ++i )
				{
					sLootData* data = (sLootData*)(*i);
					cItemDefine* define = ITEMMAN->GetItemDefine( data->itemDefineIndex );

					if( !define )
					{
						assert(0);
						return;
					}

					if( HERO->GetPartyIndex() > 0 || HERO->GetPartyUnionIndex() > 0 )
					{

						MSG_REQ_CHAT_PARTY msg;
						::memset( &msg, 0, sizeof( msg ) );

						msg.Category = NM_CHAT;
						msg.Protocol = NM_CHAT_PARTY_REQ;
						msg.InventoryNumber = SHRT_MAX;
						msg.itemIndex		= data->itemDefineIndex;

						NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );
					}
					else
					{
						MSG_REQ_CHAT msg;
						::memset( &msg, 0, sizeof( msg ) );

						msg.Category = NM_CHAT;
						msg.Protocol = NM_CHAT_NORMAL_REQ;

						msg.emotionIdx = 0;
						msg.InventoryNumber = SHRT_MAX;
						msg.itemIndex		= data->itemDefineIndex;
				
						NETWORK->SendNetworkMsg( (char*)&msg, (u_short)msg.GetMsgLength() );
					}
				}

				// 1초 지연 걸기
				mLinkDelayTime = LINK_DELAYTIME;
				mpLinkButton->SetEnabled( false );
			}

		}
		break;
	default:
		assert(0);
		return;
	}
}

sLootData* cLootingWindow::GetSelectData()
{
	if( mpItemList )
	{
		int selectRow = mpItemList->GetSelectIndex();
		return (sLootData*)mpItemList->GetExtraData( selectRow );
	}
	return 0;
}

///
void cLootingWindow::OnListBoxSelected( cUINode*, unsigned int )
{
	/// 자동습득중이면 이벤트 리턴
	if( !mpItemList )
		return;

	// 아이템 분배버튼 상태이면
	cUIWindow* win = (cUIWindow*)GAMEUI->GetItemDivideWindow();
	if( win && win->IsVisible() )
		return;
	
	/// 리스트로부터 지우기
	sLootData* data = GetSelectData();
	cItemDefine* define = ITEMMAN->GetItemDefine( data->itemDefineIndex );
	if( data && define )
	{
		/// 거래가능
		if( define->GetExchange() == eNORMAL_ITEM_EXCHAGNE_YES || define->GetExchange() == eSEALED_ITEM_EXCHAGNE_YES )
		{
			ITEMMAN->TryToGetItem( data->itemDefineIndex, data->count );
		}
		else
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_ITEM_NOTEXCHANGE_GET, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 246 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
	}
}

///
void cLootingWindow::OnListBoxImageHorvered(cUINode*, unsigned int, const cUIPos& pos, void* extraData )
{
	sLootData* data = (sLootData*)extraData;
	if( data )
		UIMAN->ShowItemTip( eTIP_ITEM, pos, data->itemDefineIndex, USHRT_MAX, true );
}

///
void cLootingWindow::OnListBoxImageLeft(cUINode* , unsigned int )
{
	UIMAN->HideTip();
}

void cLootingWindow::OnListBoxSelectedRButton( cUINode* , unsigned int , bool shift )
{
	// shift + 우클릭
	if( shift == true )
	{
		sLootData* data = GetSelectData();
		if( data )
		{
			cMegaphoneWindow* megaWin = GAMEUI->GetMegaphoneWindow();
			if( megaWin && megaWin->IsVisible() )
			{
				megaWin->AddLinkItem( USHRT_MAX, data->itemDefineIndex );
				return;
			}

			cChatWindow* chatWin = GAMEUI->GetChatWindow();
			if( chatWin && chatWin->IsVisible() )
			{
				chatWin->AddLinkItem( USHRT_MAX, data->itemDefineIndex );
				return;
			}
		}
	}
}

float cLootingWindow::GetWeight()
{
	float weight = 0;

	cItemLootList::cIterator i = mItemLootList.Begin();
	cItemLootList::cIterator end = mItemLootList.End();
	for( ; i != end; ++i )
	{
		sLootData* data = (sLootData*)(*i);

		cItemDefine* define = ITEMMAN->GetItemDefine( data->itemDefineIndex );
		if( define )
		{
			weight += ( data->count * define->GetWeight() );	
		}
	}

	return weight;
}

void cLootingWindow::OnNetLock( int lockTry )
{
	if( lockTry == eLOCK_CLOSE )
	{
		if( mpCloseButton )
			mpCloseButton->NetLock( lockTry );
	}
	else if( lockTry == eLOCK_GETITEM )
	{
		if( mpGetButton )
			mpGetButton->NetLock( lockTry );
	}
}

void cLootingWindow::OnNetUnLock( int lockTry )
{
	if( lockTry == eLOCK_CLOSE )
	{
		if( mpCloseButton )
			mpCloseButton->NetUnLock( lockTry );
	}
	else if( lockTry == eLOCK_GETITEM )
	{
		if( mpGetButton )
			mpGetButton->NetUnLock( lockTry );
	}
}


//------------------------------------------------------
cItemDivideWindow::cItemDivideWindow()
: mpListBox(0)
, mpGive(0)
, mTempPlayerIndex(0)
, mTempItemIndex(0)
{
	SetBackupVisible( false );
}

cItemDivideWindow::~cItemDivideWindow()
{

}

void cItemDivideWindow::Clear()
{
	mTempPlayerIndex = 0;
	mTempItemIndex = 0;
}

void cItemDivideWindow::OnHide()
{
	if( mpListBox )
		mpListBox->Clear();
}

void cItemDivideWindow::Show( unsigned long dropIndex )
{
	if( dropIndex == 0 )
		return;

	if( !mpListBox )
	{
		assert(0);
		return;
	}

	unsigned long nodeIndex = ITEMMAN->GetDropNodeIndex( dropIndex );
	cManagedStaticSceneNode* node = (cManagedStaticSceneNode*)SCENEMAN->GetManagedStaticNode( cSceneNode::eDROPITEM, nodeIndex );
	if( !node )
	{
		assert(0);
		return;
	}

	cDropItemSceneNode* dropSceneNode = (cDropItemSceneNode*)node;
	unsigned long* partyArr = dropSceneNode->GetParty();

	for( int i = 0; i < MAX_ITEM_PARTY; ++i, ++partyArr )
	{
		unsigned long userIndex = (*partyArr);
		if( HERO->GetObjectID() == userIndex )
		{
			// 자기정보 삽입
			mpListBox->AddRow( HERO->GetName(), COLOR_WHITE, (void*)userIndex );
		}
		else
		{
			// 파티정보로 삽입
			sPartyUserData* data = PARTYMAN->GetPartyInfoByUserIndex( userIndex );
			if( data )
			{
				mpListBox->AddRow( data->name, COLOR_WHITE, (void*)userIndex );
			}
		}
	}

	cUIWindow::Show();
}

bool cItemDivideWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpListBox = (cListBox*)GetChild( eUIID_GAME_ITEMDIVIDE_LIST );
	mpGive = (cButton*)GetChild( eUIID_GAME_ITEMDIVIDE_GIVE );

	return true;
}

void cItemDivideWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	// 위치
	cUIWindow* win = (cUIWindow*)GAMEUI->GetLootingWindow();
	if( win )
	{
		cUIRect rc = win->GetAbsoluteRect();

		int x = rc.mRight;
		int y = rc.mBottom - GetAbsoluteRect().GetHeight();

		SetRelativePos( cUIPos( x, y ) );
	}
}

void cItemDivideWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_ITEMDIVIDE_GIVE:
		{
			if( !mpListBox )
			{
				assert(0);
				return;
			}

			cLootingWindow* looting = GAMEUI->GetLootingWindow();
			if( !looting )
			{
				assert(0);
				return;
			}

			int row = mpListBox->GetSelectIndex();
			unsigned long playerIndex = (unsigned long)mpListBox->GetExtraData( row );

			LPCTSTR name = 0;
			if( HERO->GetObjectID() == playerIndex )
			{
				// 본인
				name = HERO->GetName();
			}
			else
			{
				// 파티원
				sPartyUserData* partyUser = PARTYMAN->GetPartyInfoByUserIndex( playerIndex );
				if( partyUser )
				{
					name = partyUser->name;
				}
				else
				{
					// 파티에 존재하지 않음
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS, 
										eMBEVENT_NONE, eSKIN_OK,
										GAMERESOURCEMAN->GetGameText( 171 ),
										GAMERESOURCEMAN->GetGameText( 301 )
										);
					return;
				}
			}

			sLootData* itemData = looting->GetSelectData();
			if( !itemData )
			{
				assert(0);
				return;
			}

			cItemDefine* define = ITEMMAN->GetItemDefine( itemData->itemDefineIndex );
			if( !define )
			{
				assert(0);
				return;
			}

			// 
			mTempPlayerIndex = playerIndex;
			mTempItemIndex = itemData->itemDefineIndex;

			cStringT str;
			str.Format( GAMERESOURCEMAN->GetGameText( 165 ), name, define->GetName() );

			// 지급
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_ITEM_PARTYGIVE, eSKIN_YESNO,
								(LPCTSTR)str.Cstr(),
								GAMERESOURCEMAN->GetGameText( 301 ) );
		}
		break;
	}
}

void cItemDivideWindow::OnListBoxRowLeft( cUINode* , unsigned int )
{
	UIMAN->HideTip();
}

void cItemDivideWindow::OnListBoxRowHovered(cUINode* , unsigned int , const cUIPos& pos, void* extraData )
{
	UIMAN->HideTip();

	unsigned long userIndex = (unsigned long)extraData;
	if( userIndex > 0 )
	{
		if( HERO->GetObjectID() == userIndex )
		{
			cStringT str;
			str.Format( _T("%dLv %s"), HERO->GetLevel(), GAMERESOURCEMAN->GetJobName( HERO->GetJob() ) );
			UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, (LPCTSTR)str.Cstr() );
		}
		else
		{
			sPartyUserData* data = PARTYMAN->GetPartyInfoByUserIndex( userIndex );
			if( data )
			{
				cStringT str;
				str.Format( _T("%dLv %s"), data->level, GAMERESOURCEMAN->GetJobName( data->job ) );
				UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, (LPCTSTR)str.Cstr() );
			}
		}
	}
	else
		assert(0);
}

void cItemDivideWindow::OnNetLock( int lockTry )
{
	if( mpListBox )
		mpListBox->NetLock( lockTry );
}

void cItemDivideWindow::OnNetUnLock( int lockTry )
{
	if( mpListBox )
		mpListBox->NetUnLock( lockTry );
}
