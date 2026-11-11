#include "StdAfx.h"
#include ".\skillicon.h"

#include "UIContainer.h"
#include "GameUIManager.h"
#include "SkillScript.h"
#include ".\communityscript.h"

cSkillIcon::cSkillIcon()
: mBaseSkill(false)
, mIsActiveSkill(false)
{
	SetIconType( eICON_SKILL );
	mEnableUse = false;
	mAuraUniqueIdx = 0;

	mJobIdx = 0;
	mSkillIdx = 0;
	mSkillStep = 0;
	mShotType = eSHOTTYPE_NONE;
}

cSkillIcon::~cSkillIcon()
{
}

void cSkillIcon::OnProcess( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	if( mAniEnable )
	{
		for( unsigned int i = 0; i <= eANI_FLASH; ++i )
		{
			/// 타이머
			mAniParam[i].mAccumTime = accumTime;

			unsigned long elasedTime = mAniParam[i].mAccumTime - mAniParam[i].mLastAccumTime;
			if( mAniParam[i].mAniOn && elasedTime > mAniParam[i].mTimeOut )
			{
				/// 타입에 따른 호출
				OnTimer(i);
				mAniParam[i].mLastAccumTime = mAniParam[i].mAccumTime;
			}
		}
	}

	if( mGauge.IsActive() )
	{
		unsigned int state = mGauge.Update( accumTime );
		if( state == cUIRoundGauge::eGaugeState_End )
		{
			/// 끝나는 순간에 아이콘 반짝이는 이미지 필요
			mAniParam[eANI_COUNT].mAniOn = false;
			SetAniOn( eANI_FLASH );
		}
		else if( mGauge.GetRestTime() > 0 && mGauge.GetRestTime() < 9000 )
		{
			/// 쿨타임 카운트 이미지
			short frame = (short)mGauge.GetRestTime()/1000;
			SetAniOn( eANI_COUNT, frame );
		}
	}
}

void cSkillIcon::SetSkillInfo( unsigned long idx, unsigned char step, unsigned long job, unsigned char tipKind )
{
	mJobIdx	= job;
	mSkillIdx = idx;
	mSkillStep = step;
	mTipKind = tipKind;

	sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( idx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}

	if( pInfo->mStepCount == 0 )
	{
		assert(0);
		return;
	}

	mBaseSkill = (pInfo->mpSetpInfoArray[0].mPlayerLevel == 0) && (pInfo->mStepCount == 1);
	mIsActiveSkill = (pInfo->mType != eSKILLTYPE_PASSIVE);

	if( mBaseSkill == false )
	{
		if( step != UCHAR_MAX )
			SetNumber( step + 1 );
		else
			ClearNumber();
	}

	/// 오라, 채널링스킬이면.
	mShotType = pInfo->mShotType;
	if( mShotType == eSHOTTYPE_AURA || mShotType == eSHOTTYPE_CHANNELING )
	{
		SetAuraOn( false );
	}
}

void cSkillIcon::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	if( mEnableUse == false || mIsActiveSkill == false )
		return;

	if( mEnableUse )
	{
		if( GetCapture() != this )
		{
			SetCapture();
		}

		mPressed = true;
		mShiftDown = shift;

		SendEvent( mpParent, eUIEVENT_ICON_CLICKED, this, mID, shift );
	}
}

void cSkillIcon::OnLButtonDoubleClick( const cUIPos& /*pos*/ )
{
	if( mEnableUse && mIsActiveSkill )
	{
		SendEvent( mpParent, eUIEVENT_ICON_DOUBLECLICKED, this, mID );
	}

	UIMAN->HideTip();
}

void cSkillIcon::OnMouseHovered( const cUIPos& pos )
{
	UIMAN->ShowSkillTip( (eTip_Kind)mTipKind, GAMEUI->GetSkillTip(), pos, mSkillIdx, mSkillStep, mJobIdx );
}


//////////////////////////////////////////////////////////////////////////
cCommunityIcon::cCommunityIcon()
{
	SetIconType( eICON_COMMUNITY );

	mEmoteIcon = false;
	mCommunityIndex = 0;
}

cCommunityIcon::~cCommunityIcon()
{

}

void cCommunityIcon::InitInfo( unsigned long index, bool emote )
{ 
	mCommunityIndex = index;
	mEmoteIcon = emote; 
}

void cCommunityIcon::OnMouseHovered( const cUIPos& pos )
{
	cSimpleTipWindow* p = GAMEUI->GetSimpleTip();
	if( p == 0 )
		return;

	sCommunitySkillInfo* info = COMMUNITYSCRIPT->GetCommunityInfo( mCommunityIndex );
	if( info == 0 )
		return;

	UIMAN->ShowCommunityTip( p, pos, UIMAN->GetUIText(info->mTooltipIndex ) );
}

void cCommunityIcon::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	if( mEnableUse )
	{
//		cUINode::OnLButtonDown( pos, ctrl, alt, shift );

		if( GetCapture() != this )
		{
			SetCapture();
		}

		mPressed = true;
		mShiftDown = shift;

		SendEvent( mpParent, eUIEVENT_ICON_CLICKED, this, mID, shift );
	}
}

void cCommunityIcon::OnLButtonDoubleClick( const cUIPos& /*pos*/ )
{
	SendEvent( mpParent, eUIEVENT_ICON_DOUBLECLICKED, this, mID );

	UIMAN->HideTip();
}
