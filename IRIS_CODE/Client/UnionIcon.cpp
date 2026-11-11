#include "StdAfx.h"
#include "UnionIcon.h"
#include "FontAgent.h"
#include "GameResourceManager.h"
#include "GameUIManager.h"
#include "SimpleTipWindow.h"

cUnionIcon::cUnionIcon( eUINodeType type )
: cBaseIcon( type )
, mpGauge(0)
, mMaxValueHP(0)
, mValueHP(0)
, mNamePos(0, 0)
, mIsSetInfo( false )
, mGaugePos(0, 0)
, mHeight(0)
, mJob(0)
, mLevel(0)
{
	mIconType = eICON_UNION;
	mJobAndLv.Reserve( 25 );
}

cUnionIcon::~cUnionIcon()
{
	SAFE_DELETE( mpGauge );
}

bool cUnionIcon::OnCreate( cUINodeProperty* )
{
	cUISkin* skin = UIMAN->GetSkin();
	if( !skin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* gaugeSkin = skin->GetNodeSkin( "PartyUnion_Gauge" );
	if( gaugeSkin && gaugeSkin->mpTexture )
	{
		/// 선택 이미지 생성
		mpGauge = new cUIImage( gaugeSkin->mpTexture );
		unsigned int tx = gaugeSkin->mSkinInfo->mTexX;
		unsigned int ty = gaugeSkin->mSkinInfo->mTexY;
		unsigned int x = gaugeSkin->mSkinInfo->mX;
		unsigned int y = gaugeSkin->mSkinInfo->mY;
		unsigned int w = gaugeSkin->mSkinInfo->mWidth;
		unsigned int h = gaugeSkin->mSkinInfo->mHeight;
		mpGauge->SetTextureRect( tx, ty, tx + w, ty + h );
		mpGauge->SetScreenRect( cUIRect( x+GetAbsoluteRect().mLeft,  y+GetAbsoluteRect().mTop, 
										 x+GetAbsoluteRect().mLeft+w,y+GetAbsoluteRect().mTop + h) );

		mGaugePos.mX = x;
		mGaugePos.mY = y;

		mHeight = h;
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}

bool cUnionIcon::HandleEvent( const cUIEvent& event )
{
	if( event.mType == eUIEVENT_RBUTTON_DOWN )
		return cUINode::HandleEvent( event );
	
	if( mIsSetInfo == true )
		return cUINode::HandleEvent( event );
	return true;
}

void cUnionIcon::UpdateRect()
{
	cUINode::UpdateRect();

	/// 이미지 영역을 갱신
	if( mpImage )
		mpImage->SetScreenRect( mAbsoluteRect );

	if( !mName.IsEmpty() )
	{
		cUIRect rc = GetAbsoluteRect();
		mNamePos.mX = rc.mLeft + (int)((rc.GetWidth() - FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, mName.Cstr(), mName.GetLength())) * 0.5f );
		mNamePos.mY = rc.mTop + 7;
	}

	//
	if( mpGauge )
	{
		mpGauge->SetPos( mGaugePos.mX + GetAbsoluteRect().mLeft, mGaugePos.mY + GetAbsoluteRect().mTop );
	}
}

void cUnionIcon::OnMouseHovered( const cUIPos& pos )
{
	cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
	if( !pTip ) 
	{
		assert(0);
		return;
	}
	
	if( mJobAndLv.GetLength() > 0 )
	{
		UIMAN->ShowCommunityTip( pTip, pos, mJobAndLv.Cstr() );
	}	
}

void cUnionIcon::OnMouseLeft( const cUIPos& pos )
{
	cSimpleTipWindow* pTip = GAMEUI->GetSimpleTip();
	if( pTip )
	{
		pTip->Hide();
	}
}

void cUnionIcon::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 정보가 세팅되지 않은 객체는 보이지 않는다.
	if( mIsSetInfo == false )
		return;

	if( mpImage )
		mpImage->Draw();

	if( mpGauge )
		mpGauge->Draw();

	if( !mName.IsEmpty() )
		pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPTSTR)mName.Cstr(), mNamePos.mX, mNamePos.mY, COLOR_DICENUM );

	pKeeper->DrawAll();
}

void cUnionIcon::SetInfo( LPCTSTR name, unsigned int maxhp, unsigned int hp, unsigned short job, unsigned char level )
{
	mIsSetInfo = true;

	/// 이름
	mName	= name;
	mJob	= job;
	mLevel	= level;

	mJobAndLv.Format( GAMERESOURCEMAN->GetGameText( 881 ), mLevel, GAMERESOURCEMAN->GetJobName( mJob ) );

	// 출력 좌표 갱신
	cUIRect rc = GetAbsoluteRect();
	mNamePos.mX = rc.mLeft + (int)((rc.GetWidth() - FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, mName.Cstr(), mName.GetLength())) * 0.5f );
	mNamePos.mY = rc.mTop + 7;

	/// 게이지
	UpdateGaugeInfo( maxhp, hp );
}

void cUnionIcon::ClearInfo()
{
	mIsSetInfo = false;

	/// Clear
	mName.Clear();
	mJobAndLv.Clear();

	mNamePos.mX = 0;
	mNamePos.mY = 0;

	UpdateGaugeInfo( 0, 0 );
}

void cUnionIcon::UpdateGaugeInfo( unsigned int maxhp, unsigned int hp )
{
	if( mIsSetInfo == false )
		return;

	/// 게이지 변경
	mMaxValueHP = maxhp;
	mValueHP = hp;

	if( mMaxValueHP > 0 )
	{
		unsigned int percent = (mValueHP * 100) / mMaxValueHP;

		/// 사이즈 적용
		unsigned int w = UNIONGAUGE_WIDTH * percent / 100;
		if( mpGauge )
		{
			mpGauge->SetScreenWH( w, mHeight );
		}
	}
	else
		assert(0);
}
