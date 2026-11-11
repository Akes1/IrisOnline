#include "StdAfx.h"
#include ".\duelbuff.h"

#include "GameUIManager.h"
#include "GameResourceManager.h"

#include "Skill_Common.h"
#include "SkillScript.h"
#include "icon.h"

#include "ObjectManager.h"
#include "Hero.h"

cDuelBuff::cDuelBuff()
{
	mLeftAlign = true;
}

cDuelBuff::~cDuelBuff()
{
}

void cDuelBuff::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mVisible == false )
		return;

	unsigned int count = 0;
	cBuffIconList::cIterator i = (--mIconList.End());
	cBuffIconList::cIterator iend = (--mIconList.Begin());
	for( ; i != iend; --i )
	{
		sBuffIcon* p = (sBuffIcon*)(*i);
		if( p == 0 )
			continue;

		if( count == mRow*mColumn )
			continue;

		/// 영역 재설정
		if( mUpdateList )
		{
			unsigned int sx = mStartX;
			unsigned int sy = mStartY;
			if( mpParent )
			{
				cUIRect parentAbsolute = mpParent->GetAbsoluteRect();
				sx += parentAbsolute.mLeft;
				sy += parentAbsolute.mTop;
			}

			unsigned int x = 0;
			unsigned int y = 0;
			if( mLeftAlign )
				x = sx + ((count%mColumn) * (mIconSize+mColumnSpace));
			else
				x = sx + ((mColumn-(count%mColumn)-1) * (mIconSize+mColumnSpace));

			y = sy + ((count/mColumn) * (mIconSize+mRowSpace));

			p->mImage.SetScreenRect( cUIRect(x, y, x+mIconSize, y+mIconSize) );
			p->mTextX = x;
			p->mTextY = y+mIconSize + 1;
		}

		if( mChangeOverIcon && mOverInfo.mOverIconIdx == count )
		{
			EventOverIcon( p, mOverInfo.mPos );
			mChangeOverIcon = false;
		}

		if( mRButtonClickIconIdx == count )
		{
			mRButtonClickIconIdx = (unsigned int)-1;

			sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( p->mInfluenceIdx );
			if( pScript )
			{
				if( pScript->mTypeDetail == eINFLUENCETYPEDETAIL_NAME )
					return;
			
				if( p->mAuraType == false )
				{
					if( pScript->mType == eINFLUENCETYPE_BUF )
					{
						MSG_REQ_INFLUENCE_DELETE Msg;
						Msg.Category = NM_SKILL;
						Msg.Protocol = NM_SKILL_INFLUENCE_DELETE_REQ;
						Msg.mUniqueIdx = p->mUniqueIdx;
						NETWORK->SendNetworkMsg( (char*)&Msg, sizeof( Msg ) );

						/// 락
						if( mpParent )
							mpParent->NetLock();
					}
					else if( pScript->mType == eINFLUENCETYPE_CASH )
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_DELETE_CASHBUFF, eSKIN_YESNO,
							GAMERESOURCEMAN->GetGameText( 6501 ),
							GAMERESOURCEMAN->GetGameText( 6500 ),
							(void*)p->mUniqueIdx );
					}
				}
			}
			else
				assert(0);
		}

		count++;
	}
}

void cDuelBuff::OnRender( cUIFontItemKeeper* /*pKeeper*/ )
{
	if( mUpdateList )
	{
		/// window 재계산
		cUIRect r;
		r.mRight = mStartX + mColumn*(mIconSize+mColumnSpace) - mColumnSpace;
		r.mTop = mStartY;

		if( mLeftAlign )
		{
			r.mLeft = mStartX;
		}
		else
		{
			if( mIconList.GetSize() < mColumn )
				r.mLeft = mStartX + (mColumn - mIconList.GetSize())*(mIconSize+mColumnSpace);
			else
				r.mLeft = mStartX;
		}

		unsigned int row = min((mIconList.GetSize()-1)/mColumn + 1, mRow);
		r.mBottom = mStartY + row * (mIconSize+mRowSpace) - mRowSpace;

		SetRelativeRect( r );
	}

	unsigned int count = 0;
	cBuffIconList::cIterator i = (--mIconList.End());
	cBuffIconList::cIterator iend = (--mIconList.Begin());
	for( ; i != iend; --i )
	{
		if( count == mRow*mColumn )
			break;

		sBuffIcon* p = (sBuffIcon*)(*i);

		if( p->mShow )
			p->mImage.Draw();

		count++;
	}
}


void cDuelBuff::OnMouseMove( const cUIPos& pos )
{
	/// 1초 단위 체크
	if( mOverCheck == false )
		return;
	mOverCheck = false;

	unsigned int x = mStartX;
	unsigned int y = mStartY;
	if( mpParent )
	{
		x += mpParent->GetAbsoluteRect().mLeft;
		y += mpParent->GetAbsoluteRect().mTop;
	}

	unsigned int col;
	for( col=0; col<mColumn; col++ )
	{
		unsigned int s = x + col*(mIconSize + mColumnSpace);
		if( s <= (unsigned int)pos.mX && (unsigned int)pos.mX <= s + mIconSize )
			break;
	}
	if( col == mColumn )
		return;

	if( mLeftAlign == false )
		col = mColumn-1 - col;

	unsigned int row;
	for( row=0; row<mRow; row++ )
	{
		unsigned int s = y + row*(mIconSize + mRowSpace);
		if( s <= (unsigned int)pos.mY && (unsigned int)pos.mY <= s + mIconSize )
			break;
	}
	if( row == mRow )
		return;

	unsigned int over = row*mColumn + col;
	if( mOverInfo.mOverIconIdx != over )
	{
		mChangeOverIcon = true;
		mOverInfo.mOverIconIdx = over;
		mOverInfo.mPos = pos;
	}
}

/// 일반버프윈도우와 방향이 다르다
void cDuelBuff::OnRButtonDown( const cUIPos& pos, bool /*ctrl*/, bool /*alt*/, bool /*shift*/ )
{
	if( HERO->IsJumping() == true )
		return;

	if( IsNetLock() == true )
		return;

	unsigned int x = mStartX;
	unsigned int y = mStartY;

	if( mpParent )
	{
		x += mpParent->GetAbsoluteRect().mLeft;
		y += mpParent->GetAbsoluteRect().mTop;
	}

	unsigned int col;
	for( col=0; col<mColumn; col++ )
	{
		unsigned int s = x + col*(mIconSize + mColumnSpace);
		if( s <= (unsigned int)pos.mX && (unsigned int)pos.mX <= s + mIconSize )
			break;
	}
	if( col == mColumn )
		return;

	unsigned int row;
	for( row=0; row<mRow; row++ )
	{
		unsigned int s = y + row*(mIconSize + mRowSpace);
		if( s <= (unsigned int)pos.mY && (unsigned int)pos.mY <= s + mIconSize )
			break;
	}
	if( row == mRow )
		return;

	unsigned int over = row*mColumn + col;

	mRButtonClickIconIdx = over;
}


void cDuelBuff::EventOverIcon( sBuffBaseIcon* p, const cUIPos& pos )
{
	assert( p );
	cBuffToolTipWindow* pTip = GAMEUI->GetBuffTip();

	if( pTip )
	{
		mBuffTipShow = true;
		UIMAN->ShowBuffTip( pTip, pos, p->mUniqueIdx, p->mInfluenceIdx, false );
	}
}

void cDuelBuff::AddIcon( unsigned long uniqueIdx, unsigned long influenceIdx )
{

	cBuffIconList::cIterator i	= mIconList.Begin();
	cBuffIconList::cIterator end = mIconList.End();
	sBuffIcon* tmpBuf = NULL;
	for( ; i != end ; ++i )
	{
		tmpBuf = (sBuffIcon*)(*i);

		if( tmpBuf == NULL )
		{
			assert(0);
			return;
		}

		if( tmpBuf->mInfluenceIdx == influenceIdx &&
			tmpBuf->mUniqueIdx	== uniqueIdx		)
		{
			return;
		}

	}

	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( influenceIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}
	cIconParam* pParam = UIMAN->GetIconParam( pInfo->mIconIdx );
	if( pParam == 0 )
	{
		assert(0);
		return;
	}

	sBuffIcon* p = new sBuffIcon;
	p->mTotalTime = pInfo->mContinuanceTime;

	p->mAuraType = (pInfo->mTypeDetail == eINFLUENCETYPEDETAIL_AURA);

	p->mUniqueIdx = uniqueIdx;
	p->mInfluenceIdx = influenceIdx;

	/// 이미지 셋팅
	p->mImage.SetTexture( pParam->mpTexture );

	/// 텍스처 영역 설정
	unsigned int tx = pParam->mTexPos.mX;
	unsigned int ty = pParam->mTexPos.mY;
	p->mImage.SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );

	mIconList.PushBack( p );

	HideTip();

	mUpdateList = true;

	/// npc 윈도우가 보이면 우선 예약
	Show();

	mRButtonClickIconIdx = (unsigned int)-1;
}