#include "StdAfx.h"
#include "ItemTipWindow.h"
#include "FontAgent.h"

#include "ItemManager.h"
#include "PlaneObject.h"
#include "GameResourceManager.h"
#include "UIImage.h"
#include "InputSystem.h"
#include "ItemMixManager.h"
#include "TitleManager.h"
#include "SkillScript.h"

#include "GameUIManager.h"
#include "NpcStoreWindow.h"
#include "MakeSkillScript.h"

cItemTipWindow::cItemTipWindow( bool equipItem )
: mItemIndex(0)
, mCheckTime(false)
, mSlotCount(0)
, mIsEquipItem( equipItem )
, mImagePosY( 0 )
, mItemBill(0)
{
	Clear();
	::memset( mImage, 0, sizeof(mImage) );
	::memset( mSlotImage, 0, sizeof(mSlotImage) );

	SetExceptSnap( true );
}

cItemTipWindow::~cItemTipWindow()
{
	for( int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		SAFE_DELETE( mImage[i] );
		SAFE_DELETE( mSlotImage[i] );
	}
}

void cItemTipWindow::Clear()
{
	cTipWindow::Clear();
	mItemIndex = 0;
	mCheckTime = false;
	mTimerEnabled = false;
	mSlotCount = 0;
	mImagePosY = 0;
	mItemBill = 0;

	for( int i = 0; i < MAX_CARD_SLOT; ++i )
		mShowImage[i] = false;
}

void cItemTipWindow::SetTipPos( unsigned int x, unsigned int y )
{
	if( mMiddleBack )
		mMiddleBack->SetScreenXY( x, y );

	mX = x;
	mY = y;

	/// 카드 이미지 위치 세팅
	for( unsigned int i = 0; i < mSlotCount; ++i )
	{
		if( mShowImage && mImage[i] && mSlotImage[i] )
		{
			unsigned int ygap = mSlotImage[i]->GetHeight() - mImage[i]->GetHeight();

			/// 카드이미지위치에 따라 카드슬롯이미지 세팅
			unsigned int x = mX + ( i * (mSlotImage[i]->GetWidth()+GAP_SIZE) ) + GAP_SIZE;
			unsigned int y = mImagePosY + mY;
			mSlotImage[i]->SetPos( x, y );

			/// 슬롯이미지의 일정픽셀 더해서 찍는다 (중앙에출력하기위함)
			mImage[i]->SetPos( x + SLOT_GAP, y + ygap - SLOT_GAP );
		}
	}

	SetRelativePos( cUIPos(x, y) );

	/// 
	UpdateEdge();
}

bool cItemTipWindow::OnCreate( cUINodeProperty* prop )
{
	if( cTipWindow::OnCreate( prop ) == false )
		return false;

	/// 슬롯 타입 이미지 읽기
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cString str;
	NiTexture* texture = 0;
	for( unsigned int i = 0; i < MAX_CARD_SLOT-1; ++i )
	{
		str.Format( "PutCard_SlotType%d", i );
		cUINodeSkin* pSlotSkin = pSkin->GetNodeSkin( str );
		if( pSlotSkin )
		{
			mTexPos[i].mX = pSlotSkin->mSkinInfo->mTexX;
			mTexPos[i].mY = pSlotSkin->mSkinInfo->mTexY;

			texture = pSlotSkin->mpTexture;
		}
	}

	if( !texture )
	{
		assert(0);
		return false;
	}

	/// 이미지 생성
	cUINodeSkin* imgSkin = pSkin->GetNodeSkin( "PutCard_Slot0" );
	if( !imgSkin )
		return false;

	for( int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		mImage[i] = new cUIImage;
		unsigned int x = imgSkin->mSkinInfo->mX;
		unsigned int y = imgSkin->mSkinInfo->mY;
		unsigned int w = imgSkin->mSkinInfo->mWidth;
		unsigned int h = imgSkin->mSkinInfo->mHeight;
		mImage[i]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	/// 슬롯 이미지 생성
	for( int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		str.Format( "PutCard_SlotPos%d", i );
		imgSkin = pSkin->GetNodeSkin( str );
		if( imgSkin && texture )
		{
			mSlotImage[i] = new cUIImage( texture );
			unsigned int x = imgSkin->mSkinInfo->mX;
			unsigned int y = imgSkin->mSkinInfo->mY;
			mSlotSize.mWidth = imgSkin->mSkinInfo->mWidth;
			mSlotSize.mHeight = imgSkin->mSkinInfo->mHeight;
			mSlotImage[i]->SetScreenRect( cUIRect(x, y, x+mSlotSize.mWidth, y+mSlotSize.mHeight) );
		}
	}
	return true;
}

void cItemTipWindow::OnRender( cUIFontItemKeeper* fontKeeper )
{	
	if( mVisible == false )
		return;

	/// 배경
	if( mMiddleBack )
		mMiddleBack->Draw();

	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
	{
		if( mpBackImage[i] )
			mpBackImage[i]->Draw();
	}

	if( mSlotCount > 0 )
	{
		for( unsigned int i = 0; i < mSlotCount; ++i )
		{
			if( mSlotImage[i] )
				mSlotImage[i]->Draw();

			if( mImage[i] && mShowImage[i] == true )
				mImage[i]->Draw();
		}
	}

	int px = mX + GAP_SIZE;
	int py = mY;

	/// 데이터 출력 
	for( unsigned int row = 0; row < mIndex; ++row )
	{
		cStringT& str = mToolTip[row].mString;
		if( str.IsEmpty() )
			continue;

		py += mToolTip[row].mGapCount * GAP_SIZE;

		/// 다음줄 출력 
		unsigned int color = mToolTip[row].mColor;
		if( mIsEquipItem && row == 0 )
		{
			long width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), str.GetSize() );
			int x = (GetAbsoluteRect().GetWidth() - width) / 2;
			fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), mX + x, py, color );
		}
		else
		{
			fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), px, py, color );
		}

		py += mLineHeight;
	}

	///
	fontKeeper->DrawAll();
}

bool cItemTipWindow::SetItemInfo( eTip_Kind tipkind, const cUIPos& pos, sInventory& inven, sItemBill* bill, bool checkTime )
{
	cItemDefine* itemDefine = ITEMMAN->GetItemDefine( inven.ItemIndex );
	if( itemDefine == 0 )
		return false;

	Clear();

	mItemIndex = inven.ItemIndex;
	mCheckTime = checkTime;
	mItemBill = bill;

	//////////////////////////////////////////////////////////////////////////
	/// Tip Start
	if( mIsEquipItem )
	{
		AddGap( 1 );
		SetText( UIMAN->GetUIText( 117 ), 0xFF00FFFF );
	}

	// 이름 설정
	AddGap( 1 );
	SetName( itemDefine, inven.enhanced );

	// 레어도
	SetRare( itemDefine );
	AddGap( 1 );

	/// 아이템 분류 
	unsigned char type = itemDefine->GetType();
	unsigned char subType = itemDefine->GetSubType();
	short defenseType = 0;

	cItemAbility* itemAbil = itemDefine->GetAbililty();
	if( itemAbil )
		defenseType = itemAbil->GetDefenseType();

	SetType( type, subType, defenseType );
	AddGap( 1 );

	/// 장착성 아이템
	if( type == ITEM_WEAPON || type == ITEM_WEAR || type == ITEM_ACCESSORY )
	{
		/// 아이템 기본 수치 설정
		if( SetBasic( itemDefine, inven.enhanced ) == true )
			AddGap( 1 );

		/// 아이템 추가 수치 설정
		if( SetSpecial( itemDefine ) == true )
			AddGap( 1 );
	}
	else if( type == ITEM_CARD && subType == ITEM_CARD_EQUIP_C )
	{
		/// 아이템 기본 수치 설정
		if( SetBasic( itemDefine, inven.enhanced ) == true )
			AddGap( 1 );

		/// 아이템 추가 수치 설정
		if( SetSpecialCard( itemDefine ) == true )
			AddGap( 1 );
	}
	else
	{
		/// 비장착성 아이템 
		/// 사용효과 설정
		if( SetEffect( itemDefine ) == true )
			AddGap( 1 );

		/// 재사용시간 설정
		if( SetUseTime( itemDefine ) == true )
			AddGap( 1 );
	}

	// 발동효과 관련 세팅
	if( itemAbil && itemAbil->GetActiveInfluenceIndex() > 0 )
	{
		sActInfluenceScript* active = SKILLSCRIPT->GetActInfluenceInfo( itemAbil->GetActiveInfluenceIndex() );
		if( active )
		{
			unsigned long textIndex = 0;
			switch( active->mActivityState )
			{
			case eACTSTATUS_ACTIVESKILL:	textIndex = 2000951; break;
			case eACTSTATUS_HEALSKILL:		textIndex = 2000952; break;
			case eACTSTATUS_DAMAGE:			textIndex = 2000953; break;
			default: assert(0); break;
			}

			if( textIndex > 0 )
			{
				cStringT str = GAMERESOURCEMAN->GetItemToolTipText( 2000719 );
				str.Append( GAMERESOURCEMAN->GetItemToolTipText( textIndex ) );
				str.Append( GAMERESOURCEMAN->GetItemToolTipText( active->mTooltipIdx ) );

				SetText( (LPCTSTR)str.Cstr(), COLOR_GREEN );
			}
		}
		else
		{
			assert(0);
		}
	}

	// 카드정보세팅
	SetCard( inven );

	/// 아이템 세트 설정
	unsigned int setIndex = itemDefine->GetSetIndex();
	if( setIndex != 0 )
	{
		SetItemSetInfo( itemDefine, setIndex );
	}

	/// 아이템 설명 세팅
	LPCTSTR desc = itemDefine->GetDescription();
	if( desc )
	{
		CheckText( desc, COLOR_BLUE );
		AddGap( 1 );
	}
	
	// 무게
	SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000701 ), (unsigned long)itemDefine->GetWeight(), COLOR_WHITE );

	/// 보유 가능 갯수
	if( itemDefine->GetMaxHaveInven() > 0 )
	{
		sItemCount* p = ITEMMAN->GetItemCount( inven.ItemIndex );
		unsigned int cnt = ( p ) ? p->total : 0;

		TCHAR temp[128] = {0,};
		::_stprintf_s( temp, GAMERESOURCEMAN->GetItemToolTipText( 2000713 ), cnt, itemDefine->GetMaxHaveInven() );
		SetText( temp, COLOR_WHITE );
	}

	// 보조직업에 따른 재료
	typedef tHashSet<unsigned long> cMixedTypeSet;
	cMixedTypeSet* mixSet = MAKESKILLSCRIPT->GetMixedTypeSet( inven.ItemIndex );
	if( mixSet && mixSet->GetSize() > 0 )
	{
		int cnt = 0;
		cStringT str = GAMERESOURCEMAN->GetItemToolTipText( 2000058 );
		cMixedTypeSet::cIterator i = mixSet->Begin();
		cMixedTypeSet::cIterator end = mixSet->End();
		for( ; i != end; ++i )
		{
			// 보조직업 인덱스
			unsigned long mixedIndex = (unsigned long)(*i);
			unsigned long textIndex = ITEMMIXMAN->GetKeepSkillText( (unsigned char)mixedIndex );

			if( cnt > 0 )
				str.Append( _T(", ") );

			str.Append( UIMAN->GetUIText( textIndex ) );

			cnt++;
		}	

		SetText( (LPCTSTR)str.Cstr(), COLOR_WHITE );
		AddGap( 1 );
	}

	// 조합 레시피일떄
	if( type == ITEM_MIX_SKILL && (subType == ITEM_MIX_RECIPE || subType == ITEM_MIX_RECIPEGROUP) )
	{
		if( ITEMMIXMAN->IsKeepRecipe( inven.ItemIndex ) == true )
		{
			SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000715 ), COLOR_RED );
		}
	}

	// 직업 증서일때 이미 배웠는지 체크
	if( type == ITEM_MIX_SKILL && ( subType == ITEM_MIX_SKILL_L1 || subType == ITEM_MIX_SKILL_L2 ||  subType == ITEM_MIX_SKILL_L3 || subType == ITEM_MIX_SKILL_L4 ) )
	{
		if( ITEMMIXMAN->IsKeepMixSkill( itemAbil->GetInfluenceIndex(), subType ) == true )
		{
			SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000722 ), COLOR_RED );
		}
	}

	// 호칭 습득 여부
	unsigned long titleIndex = itemDefine->GetTitleIndex();
	if( titleIndex > 0 )
	{
		if( TITLEMAN->IsHaveTitle( titleIndex ) == true )
		{
			SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000720 ), COLOR_RED );
		}
	}

	// 제한
	SetLimit( itemDefine );

	// 분해 가능여부
	if( type == ITEM_WEAPON || type == ITEM_WEAR || type == ITEM_ACCESSORY )
	{
		unsigned long disjointIndex = ( itemDefine->GetDisjoint() ) ? 2000705 : 2000716;
		unsigned long color			= ( itemDefine->GetDisjoint() ) ? COLOR_BLUE : COLOR_RED;
		SetText( GAMERESOURCEMAN->GetItemToolTipText( disjointIndex ), color );
	}

	// 강화 가능 여부
	if( type == ITEM_WEAPON || type == ITEM_WEAR )
	{
		unsigned long enhancedIndex = 2000718;
		unsigned long color			= COLOR_RED;
		cEnhancedOptionArr* optionArr = itemDefine->GetEnhancedOptionArr();
		if( optionArr )
		{
			enhancedIndex = ( optionArr->IsEmpty() ) ? 2000718 : 2000717;
			color		  = ( optionArr->IsEmpty() ) ? COLOR_RED : COLOR_BLUE;
		}
		SetText( GAMERESOURCEMAN->GetItemToolTipText( enhancedIndex ), color );
	}

	/// 거래 가능 여부(0거래가능/1거래불가/2미인증거래가능/3미인증 거래불가)
	if( itemDefine->GetExchange() == eNORMAL_ITEM_EXCHAGNE_NO ) 
	{
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000711 ), COLOR_RED );
	}
	else if( itemDefine->GetExchange() == eSEALED_ITEM_EXCHAGNE_NO )
	{
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000711 ), COLOR_RED );
	}
	/// 미인증거래가능시 inven.seal 검사
	else if( itemDefine->GetExchange() == eSEALED_ITEM_EXCHAGNE_YES && inven.seal == 0 )
	{
		SetText( GAMERESOURCEMAN->GetItemToolTipText( 2000711 ), COLOR_RED );
	}

	/// 가격
	eItemIn itemIn = ITEMMAN->GetItemIn( inven.number );
	if( itemIn != ITEM_IN_NPCSTORE )
	{
		/// 판매가 세팅
		SetPrice( itemDefine->GetSellType(), itemDefine, 2000702, itemDefine->GetSellPrice() );
	}
	else
	{
		/// 현재 보고 있는 npc 상점의 비율에 따라 값이 다름.
		unsigned long sellRate = 100;
		unsigned long itemPrice;
		itemDefine->GetSellPrice();
		cNpcStoreWindow* win = GAMEUI->GetNpcStore();
		if( win )
		{
			sellRate = win->GetCurrentItemPriceRate();
			if( sellRate == ULONG_MAX )
			{	
				assert(0);
				sellRate = 100;
			}
		}
		else
			assert(0);

		itemPrice = unsigned long(itemDefine->GetBuyPrice() * sellRate * 0.01);

		/// 구매가 세팅
		SetPrice( itemDefine->GetBuyType(), itemDefine, 2000704, itemPrice );
	}

	if( tipkind == eTIP_ITEM )
	{
		/// 사용 기한 (0일반/1실제시간/2게임시간/3무제한)
		if( itemDefine->GetLicenseClass() != eLICENSE_NONE )
		{
			AddGap( 1 );

			TCHAR temp1[128] = {0,};
			TCHAR temp2[128] = {0,};

			if( mItemBill )
			{
				// 인증
				switch( itemDefine->GetLicenseClass() )
				{
				case eLICENSE_REALTIME:
					{
						// 기간제
						struct tm date;

						localtime_s( &date, &mItemBill->validDate );

						date.tm_year = date.tm_year+1900;
						date.tm_mon  = date.tm_mon+1;

						//2000725 %4d년 %d월 %d일 %d시 %d분"
						::_stprintf_s( temp1, GAMERESOURCEMAN->GetItemToolTipText( 2000725 ), date.tm_year, date.tm_mon, date.tm_mday, date.tm_hour, date.tm_min );
						::_stprintf_s( temp2, GAMERESOURCEMAN->GetItemToolTipText( 2000723 ), temp1 );
						SetText( temp2, COLOR_WHITE );
					}
					break;
				case eLICENSE_GAMETIME:
					{
						// 게임시간
						time_t ltime;
						long   result;

						time( &ltime );

						result = (long)difftime( ((sItemBillEx*)mItemBill)->begin, ltime );
						if( result < 0 )
							result = 0;

						// 시간제
						unsigned int d = (result / 60) / 60 / 24;
						unsigned int h = (result / 60) / 60 % 24;
						unsigned int m = (result / 60) % 60;
						unsigned int s = (result % 60) % 60 + 1;

						cStringT td,th,tm,ts;
						if( d != 0 )
							td.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000728 ), d );
						if( h != 0 )
							th.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000855 ), h );
						if( m != 0 )
							tm.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000727 ), m );
						if( s != 0 )
							ts.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000726 ), s );

						td += th;
						td += tm;
						td += ts;

						if( td.GetLength()  == 0 )
							td.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000808 ), 0 );

						::_stprintf_s( temp1, GAMERESOURCEMAN->GetItemToolTipText( 2000723 ), td.Cstr() );

						//
						if( mCheckTime )
						{
							mTimerEnabled = true;
							mUpdateSizebyTime = true;
						}

						// 정보구성
						mTimeLine[ eTIMELINE_LIMITDATE ].saveCoolTime = temp1;
						mTimeLine[ eTIMELINE_LIMITDATE ].timeTipIdx = mIndex;

						SetText( temp1, COLOR_WHITE );
					}
					break;
				case eLICENSE_NOLIMIT:
					{
						// 무제한
						::_stprintf_s( temp1, GAMERESOURCEMAN->GetItemToolTipText( 2000723 ), GAMERESOURCEMAN->GetItemToolTipText( 2000724 ) );
						SetText( temp1, COLOR_WHITE );
					}
					break;
				}
			}
			else
			{
				// 미인증
				switch( itemDefine->GetLicenseClass() )
				{
				case eLICENSE_REALTIME:
				case eLICENSE_GAMETIME:
					{
						// 미인증시 기본 시간 출력
						long validThru = itemDefine->GetLicenseValidThru();

						unsigned int d = (validThru / 60) / 60 / 24;
						unsigned int h = (validThru / 60) / 60 % 24;
						unsigned int m = (validThru / 60) % 60;
						unsigned int s = (validThru % 60) % 60;

						if( d != 0 )
							::_stprintf_s( temp1, GAMERESOURCEMAN->GetItemToolTipText( 2000728 ), d );
						else if( h != 0 )
							::_stprintf_s( temp1, GAMERESOURCEMAN->GetItemToolTipText( 2000855 ), h );
						else if( m != 0 )
							::_stprintf_s( temp1, GAMERESOURCEMAN->GetItemToolTipText( 2000727 ), m );
						else
							::_stprintf_s( temp1, GAMERESOURCEMAN->GetItemToolTipText( 2000726 ), s );

						::_stprintf_s( temp2, GAMERESOURCEMAN->GetItemToolTipText( 2000723 ), temp1 );
						SetText( temp2, COLOR_WHITE );
					}
					break;
				case eLICENSE_NOLIMIT:
					{
						// 무제한
						::_stprintf_s( temp1, GAMERESOURCEMAN->GetItemToolTipText( 2000723 ), GAMERESOURCEMAN->GetItemToolTipText( 2000724 ) );
						SetText( temp1, COLOR_WHITE );
					}
					break;
				}
			}
		}
	}

	/// Tip End
	//////////////////////////////////////////////////////////////////////////

	/// 배경 이미지 세팅
	SetBackImg( pos );
	return true;
}

/// 배경 이미지 세팅
void cItemTipWindow::SetBackImg( const cUIPos& pos )
{
	/// 최소 너비 제한 설정
	if( mMaxWidth < mLimitMinWidth )
		mMaxWidth = mLimitMinWidth;

	/// 최대 너비 제한 설정
	if( mMaxWidth > mLimitMaxWidth )
		mMaxWidth = mLimitMaxWidth;

	/// 세팅
	mX = pos.mX + 25;
	mY = pos.mY + 25;
	unsigned int w = mMaxWidth + GAP_SIZE * 2; 
	unsigned int h = mIndex * mLineHeight + (mGapCount*GAP_SIZE);

	/// 카드가 있으면 카드 넓이만큼 더 세팅
	if( mSlotCount > 0 )
	{
		unsigned int cardWidth = mSlotCount * ( mSlotSize.mWidth + SLOT_GAP + GAP_SIZE );

		if( cardWidth > w )
			w = cardWidth;
	}

	/// 화면 안벗어나게 조정
	if( mX + w > GetScreenWidth() )
	{	
		mX -= ((mX + w) - GetScreenWidth());
	}

	if( mY + h > GetScreenHeight() )
	{
		mY -= ((mY + h) - GetScreenHeight());
	}

	mMiddleBack->SetScreenRect( (short)mX, (short)mY, (unsigned short)w, (unsigned short)h );
	SetRelativeRect( cUIRect(mX, mY, mX+w, mY+h) );

	/// 카드 이미지 위치 세팅
	for( unsigned int i = 0; i < mSlotCount; ++i )
	{
		if( mShowImage && mImage[i] && mSlotImage[i] )
		{
			unsigned int ygap = mSlotImage[i]->GetHeight() - mImage[i]->GetHeight();

			/// 카드이미지위치에 따라 카드슬롯이미지 세팅
			unsigned int x = mX + ( i * (mSlotImage[i]->GetWidth()+GAP_SIZE) ) + GAP_SIZE;
			unsigned int y = mImagePosY + mY;
			mSlotImage[i]->SetPos( x, y );

			/// 슬롯이미지의 일정픽셀 더해서 찍는다 (중앙에출력하기위함)
			mImage[i]->SetPos( x + SLOT_GAP, y + ygap - SLOT_GAP );
		}
	}

	///
	UpdateEdge();
}

void cItemTipWindow::SetCard( sInventory& inven )
{
	cItemDefine* define = ITEMMAN->GetItemDefine( inven.ItemIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	/// 무기, 방어구 타입 아니면 리턴
	if( !(define->GetType() == ITEM_WEAPON || define->GetType() == ITEM_WEAR || define->GetType() == ITEM_ACCESSORY ) )
		return;

	/// 슬롯 존재여부 판단
	mSlotCount = define->GetPutSlotCount();
	if( mSlotCount == 0 )
		return;

	/// 이미지 출력 위치 저장
	mImagePosY = mIndex * mLineHeight + (mGapCount*GAP_SIZE);

	/// 슬롯 형태 세팅
	sItemPutCardSlot* data = define->GetPutCard();
	if( data )
	{
		cUIPos texpos;
		for( unsigned int i = 0; i < mSlotCount; ++i )
		{
			switch( data->mSlotType[i] )
			{
			case ITEM_CARD_SLOT_NONE:	continue;
			case ITEM_CARD_SLOT_A:	texpos = mTexPos[0]; break;
			case ITEM_CARD_SLOT_B:	texpos = mTexPos[1]; break;
			case ITEM_CARD_SLOT_AB:	texpos = mTexPos[2]; break;
			case ITEM_CARD_SLOT_O:	texpos = mTexPos[3]; break;
			}

			if( mSlotImage[i] )
				mSlotImage[i]->SetTextureRect( cUIRect(texpos.mX, texpos.mY, texpos.mX + mSlotSize.mWidth, texpos.mY + mSlotSize.mHeight ) );
		}
	}

	AddGap( 8 );

	/// 장착된 카드 이미지 세팅
	SetCardData( 0, inven.cardSlot1 );
	SetCardData( 1, inven.cardSlot2 );
	SetCardData( 2, inven.cardSlot3 );
	SetCardData( 3, inven.cardSlot4 );
	SetCardData( 4, inven.cardSlot5 );

	/// 하나라도 세팅되면 갭 넣기
	AddGap( 1 );
}

bool cItemTipWindow::SetCardData( int i, unsigned long cardIndex )
{
	if( cardIndex == 0 )
		return false;

	cItemDefine* cardDefine = ITEMMAN->GetItemDefine( cardIndex );
	if( !cardDefine )
		return false;

	/// 카드 이름 세팅
	/// 글씨 컬러 정하기
	unsigned long color = COLOR_WHITE;
	ITEMMAN->GetTooltipColor( cardDefine->GetTipType(), color );

	/// 카드이미지 세팅
	cIconParam* param = cardDefine->GetIconParam();
	if( param )
	{
		mImage[i]->SetTexture( param->mpTexture );
		mImage[i]->SetTextureRect( cUIRect( param->mTexPos.mX, param->mTexPos.mY, 
			param->mTexPos.mX + 28, param->mTexPos.mY + 28 ) );

		mShowImage[i] = true;
	}

	/// 카드 설명 세팅
	cItemAbility* pAbil = cardDefine->GetAbililty();
	if( pAbil )
	{
		TCHAR temp[256] = {0,};
		//LPCTSTR name = cardDefine->GetName();
		long index = pAbil->GetDescription();

		::_stprintf_s( temp, _T("%s: %s"), cardDefine->GetName(), GAMERESOURCEMAN->GetItemToolTipText( index ) );
		SetText( temp, color );
	}
	
	return true;
}


///////////
cChatTipWindow::cChatTipWindow()
: mpClose(0)
{

}

cChatTipWindow::~cChatTipWindow()
{

}

bool cChatTipWindow::OnCreate( cUINodeProperty* prop )
{
	if( cItemTipWindow::OnCreate( prop ) == false )
		return false;

	mpClose = GetChild( eUIID_CHATTIP_CLOSE );

	///
	SetExceptEvent( false );
	return true;
}

void cChatTipWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mMiddleBack )
	{
		mX = GetAbsoluteRect().mLeft;
		mY = GetAbsoluteRect().mTop;
		mMiddleBack->SetScreenXY( mX, mY );

		/// 카드 이미지 위치 세팅
		for( unsigned int i = 0; i < mSlotCount; ++i )
		{
			if( mShowImage && mImage[i] && mSlotImage[i] )
			{
				unsigned int ygap = mSlotImage[i]->GetHeight() - mImage[i]->GetHeight();

				/// 카드이미지위치에 따라 카드슬롯이미지 세팅
				unsigned int x = mX + ( i * (mSlotImage[i]->GetWidth()+GAP_SIZE) ) + GAP_SIZE;
				unsigned int y = mImagePosY + mY;
				mSlotImage[i]->SetPos( x, y );

				/// 슬롯이미지의 일정픽셀 더해서 찍는다 (중앙에출력하기위함)
				mImage[i]->SetPos( x + SLOT_GAP, y + ygap - SLOT_GAP );
			}
		}
	}

	UpdateEdge();
}

void cChatTipWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	SetBackImg( cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()) );
}

void cChatTipWindow::OnRender( cUIFontItemKeeper* fontKeeper )
{	
	if( mVisible == false )
		return;

	/// 배경
	mMiddleBack->Draw();

	for( int idx = 0; idx < eBACKIMAGE_MAX; ++idx )
	{
		if( mpBackImage[idx] )
			mpBackImage[idx]->Draw();
	}

	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
			((cUINode*)(*i))->OnRender(fontKeeper);
	}

	///
	if( mSlotCount > 0 )
	{
		for( unsigned int i = 0; i < mSlotCount; ++i )
		{
			if( mSlotImage[i] )
				mSlotImage[i]->Draw();

			if( mImage[i] && mShowImage[i] == true )
				mImage[i]->Draw();
		}
	}

	int px = mX + GAP_SIZE;
	int py = mY;

	/// 데이터 출력 
	for( unsigned int row = 0; row < mIndex; ++row )
	{
		cStringT& str = mToolTip[row].mString;
		if( str.IsEmpty() )
			continue;

		py += mToolTip[row].mGapCount * GAP_SIZE;

		/// 다음줄 출력 
		unsigned int color = mToolTip[row].mColor;
		fontKeeper->AddFontItem( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), px, py, color );

		py += mLineHeight;
	}

	///
	fontKeeper->DrawAll();
}

void cChatTipWindow::OnCommand(cUINode* , unsigned int id )
{
	if( id == eUIID_CHATTIP_CLOSE )
		Hide();
}

/// 배경 이미지 세팅
void cChatTipWindow::SetBackImg( const cUIPos& pos )
{
	if( !mpClose )
	{
		assert(0);
	}

	/// 최소 너비 제한 설정
	if( mMaxWidth < mLimitMinWidth )
		mMaxWidth = mLimitMinWidth;

	/// 최대 너비 제한 설정
	if( mMaxWidth > mLimitMaxWidth )
		mMaxWidth = mLimitMaxWidth;

	/// 세팅
	mX = GetRelativeRect().mLeft;
	mY = GetRelativeRect().mTop;
	unsigned int w = mMaxWidth + GAP_SIZE * 2;
	unsigned int h = mIndex * mLineHeight + (mGapCount*GAP_SIZE);

	if( mpClose )
		w += mpClose->GetRelativeRect().GetWidth();

	/// 카드가 있으면 카드 넓이만큼 더 세팅
	if( mSlotCount > 0 )
	{
		unsigned int cardWidth = mSlotCount * ( mSlotSize.mWidth + SLOT_GAP + GAP_SIZE );

		if( cardWidth > w )
			w = cardWidth;
	}

	SetRelativeSize( cUISize( w, h ) );
	mCaptionRect.Set( 0, 0, w, h );

	if( mMiddleBack )
		mMiddleBack->SetScreenRect( (short)mX, (short)mY, (unsigned short)w, (unsigned short)h );

	/// 카드 이미지 위치 세팅
	for( unsigned int i = 0; i < mSlotCount; ++i )
	{
		if( mShowImage && mImage[i] && mSlotImage[i] )
		{
			unsigned int ygap = mSlotImage[i]->GetHeight() - mImage[i]->GetHeight();

			/// 카드이미지위치에 따라 카드슬롯이미지 세팅
			unsigned int x = mX + ( i * (mSlotImage[i]->GetWidth()+GAP_SIZE) ) + GAP_SIZE;
			unsigned int y = mImagePosY + mY;
			mSlotImage[i]->SetPos( x, y );

			/// 슬롯이미지의 일정픽셀 더해서 찍는다 (중앙에출력하기위함)
			mImage[i]->SetPos( x + SLOT_GAP, y + ygap - SLOT_GAP );
		}
	}

	/// 추가 세팅
	if( mpClose )
	{
		int x = w - mpClose->GetRelativeRect().GetWidth() - 2;
		mpClose->SetRelativePos( cUIPos(x, 2) );
	}

	UpdateEdge();
}
