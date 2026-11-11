#include "stdafx.h"
#include "PartyBuff.h"
#include "SkillScript.h"
#include "icon.h"

cPartyBuff::cPartyBuff()
: mpBuffSkin(0)
, mUpdatePos( false )
{
}

cPartyBuff::~cPartyBuff()
{
	Clear();
}

void cPartyBuff::Clear()
{
	mOverInfo.mOverIconIdx = (unsigned int)-1;
	mOverCheck = false;
	mChangeOverIcon = false;
	mRButtonClickIconIdx = (unsigned int)-1;

	mChangeOverIcon = false;

	/// 버프 목록
	{
		cBuffList::cIterator i = mBuffList.Begin();
		cBuffList::cIterator end = mBuffList.End();

		for( ; i != end; ++i )
		{
			sBuffBaseIcon* p = (sBuffBaseIcon*)(*i);
			SAFE_DELETE(p);
		}
		mBuffList.Clear();
	}

	/// 디버프 목록
	{
		cDeBuffList::cIterator i = mDeBuffList.Begin();
		cDeBuffList::cIterator end = mDeBuffList.End();

		for( ; i != end; ++i )
		{
			sBuffBaseIcon* p = (sBuffBaseIcon*)(*i);
			SAFE_DELETE(p);
		}
		mDeBuffList.Clear();
	}

	/// 기존에 떠있던 팁 닫기
	HideTip();
}

void cPartyBuff::OnShow()
{
	if( mBuffList.IsEmpty() && mDeBuffList.IsEmpty() )
	{
		Hide();
		return;
	}

	cUIWindow::OnShow();
}

void cPartyBuff::OnHide()
{
	cUIWindow::OnHide();

	mOverInfo.mOverIconIdx = (unsigned int)-1;
	mChangeOverIcon = false;
	mOverCheck = false;
}

bool cPartyBuff::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_WINDOW ) == false )
	{
		assert( 0 && "not window type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
		return false;

	cBuffWindowSkin* p = (cBuffWindowSkin*)pskin;
	if( !p )
		return false;

	SetAlpha( p->mAlpha );
	SetHoldView( p->mHoldView );

	mRow = p->mRow;
	mColumn = p->mColumn;
	mRowSpace = p->mRowSpace;
	mColumnSpace = p->mColumnSpace;
	mIconSize = p->mIconSize;

	mTimerEnabled = true;
	mTimeOut = 100;

	mpBuffSkin = p;
	return true;
}

void cPartyBuff::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	UpdateWindowSize();
}

void cPartyBuff::OnMouseMove(const cUIPos& pos )
{
	/// 1초 단위 체크
	if( mOverCheck == false )
		return;

	mOverCheck = false;

	UpdateMouseHover( pos );
}

void cPartyBuff::UpdateMouseHover( const cUIPos &pos )
{
	if( mVisible == false )
	{
		mChangeOverIcon = false;
		return;
	}

	unsigned int col;
	for( col=0; col<mColumn; col++ )
	{
		unsigned int s = GetAbsoluteRect().mLeft + col*(mIconSize + mColumnSpace);
		if( s <= (unsigned int)pos.mX && (unsigned int)pos.mX <= s + mIconSize )
			break;
	}
	if( col == mColumn )
		return;

	unsigned int row;
	for( row=0; row<mRow; row++ )
	{
		unsigned int s = GetAbsoluteRect().mTop + row*(mIconSize + mRowSpace);
		if( s <= (unsigned int)pos.mY && (unsigned int)pos.mY <= s + mIconSize )
			break;
	}
	if( row == mRow )
		return;

	if( mOverInfo.mOverIconIdx != col || mOverInfo.mRow != row )
	{
		mChangeOverIcon = true;
		mOverInfo.mOverIconIdx = col;
		mOverInfo.mRow = row;
		mOverInfo.mPos = pos;
	}
}

void cPartyBuff::AddBuff( unsigned long uniqueIdx, unsigned long influenceIdx )
{
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

	if( !(pInfo->mType == eINFLUENCETYPE_BUF || pInfo->mType == eINFLUENCETYPE_DEBUF) )
		return;

	sBuffBaseIcon* p = new sBuffBaseIcon;
	p->mUniqueIdx = uniqueIdx;
	p->mInfluenceIdx = influenceIdx;

	/// 이미지
	p->mImage.SetTexture( pParam->mpTexture );
	unsigned int tx = pParam->mTexPos.mX;
	unsigned int ty = pParam->mTexPos.mY;
	p->mImage.SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );

	// 버프, 디버프 분류
	if( pInfo->mType == eINFLUENCETYPE_BUF )
	{
		mBuffList.PushFront( p );
	}
	else if( pInfo->mType == eINFLUENCETYPE_DEBUF )
	{
		mDeBuffList.PushFront( p );
	}

	/// 기존에 떠있던 팁 닫기
	HideTip();

	UpdateWindowSize();
}

void cPartyBuff::DelBuff( unsigned long uniqueIdx )
{
	if( mBuffList.GetSize() <= 0 )
		return;
	
	/// 삭제
	cBuffList::cIterator i = mBuffList.Begin();
	cBuffList::cIterator iend = mBuffList.End();
	for( ; i != iend; ++i )
	{
		sBuffBaseIcon* p = (sBuffBaseIcon*)(*i);
		if( p->mUniqueIdx == uniqueIdx )
		{
			mBuffList.Erase( i );
			SAFE_DELETE( p );
			break;
		}
	}
	
	HideTip();
	if( mBuffList.IsEmpty() && mDeBuffList.IsEmpty() )
		Hide();

	UpdateWindowSize();
}

void cPartyBuff::DelDeBuff( unsigned long uniqueIdx )
{
	if( mDeBuffList.GetSize() <= 0 )
		return;

	/// 디버프 삭제
	cDeBuffList::cIterator i = mDeBuffList.Begin();
	cDeBuffList::cIterator end = mDeBuffList.End();

	for( ; i != end; ++i )
	{
		sBuffBaseIcon* p = (sBuffBaseIcon*)(*i);
		if( p->mUniqueIdx == uniqueIdx )
		{
			mDeBuffList.Erase( i );
			SAFE_DELETE( p );
			break;
		}
	}

	HideTip();
	if( mBuffList.IsEmpty() && mDeBuffList.IsEmpty() )
		Hide();

	UpdateWindowSize();
}

void cPartyBuff::UpdateRect()
{
	cUIWindow::UpdateRect();

	/// 
	mUpdatePos = true;

	//UpdateWindowSize();
}

void cPartyBuff::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible == true  )
	{
		cUIWindow::OnProcess( deltaTime, accumTime );

		if( mBuffList.GetSize() <= 0 && mDeBuffList.GetSize() <= 0)
			return;

		/// 버프아이콘
		{
			unsigned int count = 0;
			cBuffList::cIterator i = mBuffList.Begin();
			cBuffList::cIterator end = mBuffList.End();

			for( ; i != end; ++i )
			{
				sBuffBaseIcon* p = (sBuffBaseIcon*)(*i);
				if( !p ) 
					continue;

				if( count >= mColumn )
					break;

				/// 버프 이미지 뿌려질 위치 재설정
				if( mUpdatePos )
				{
					cUIRect rc = GetAbsoluteRect();

					unsigned int x = rc.mLeft + (count%mColumn) * (mIconSize+mColumnSpace);
					unsigned int y = rc.mTop;

					p->mImage.SetScreenRect( cUIRect(x, y, x+mIconSize, y+mIconSize) );
				}

				if( mChangeOverIcon && mOverInfo.mRow == 0 && mOverInfo.mOverIconIdx == count )
				{
					EventOverIcon( p, mOverInfo.mPos );
					mChangeOverIcon = false;
				}

				count++;
			}
		}

		/// 디버프아이콘
		{
			unsigned int count = 0;
			cDeBuffList::cIterator i = mDeBuffList.Begin();
			cDeBuffList::cIterator end = mDeBuffList.End();

			for( ; i != end; ++i )
			{
				sBuffBaseIcon* p = (sBuffBaseIcon*)(*i);
				if( !p ) 
					continue;

				if( count >= mColumn )
					break;

				/// 버프 이미지 뿌려질 위치 재설정
				if( mUpdatePos )
				{
					cUIRect rc = GetAbsoluteRect();
					unsigned int x = rc.mLeft + (count%mColumn) * (mIconSize+mColumnSpace);
					unsigned int y = rc.mTop + (mIconSize+mRowSpace);

					if( mBuffList.GetSize() == 0 )
						y = rc.mTop;

					p->mImage.SetScreenRect( cUIRect(x, y, x+mIconSize, y+mIconSize) );
				}

				unsigned int row = 1;
				if( mBuffList.GetSize() == 0  )
					row = 0;

				if( mChangeOverIcon && mOverInfo.mRow == row && mOverInfo.mOverIconIdx == count )
				{
					EventOverIcon( p, mOverInfo.mPos );
					mChangeOverIcon = false;
				}

				count++;
			}
		}

		mUpdatePos = false;
	}
}

void cPartyBuff::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mBuffList.GetSize() <= 0 && mDeBuffList.GetSize() <= 0)
		return;

	/// 버프아이콘 찍기 
	{
		unsigned int count = 0;
		cBuffList::cIterator i = mBuffList.Begin();
		cBuffList::cIterator end = mBuffList.End();

		for( ; i != end; ++i )
		{
			/// 일정갯수 이상은 찍지 않는다
			if( count >= mColumn )
				break;

			sBuffBaseIcon* p = (sBuffBaseIcon*)(*i);
			if( !p )
			{
				assert(0);
				continue;
			}
			
			p->mImage.Draw();
			count++;
		}
	}

	/// 디버프아이콘 찍기
	{
		unsigned int count = 0;
		cDeBuffList::cIterator i = mDeBuffList.Begin();
		cDeBuffList::cIterator end = mDeBuffList.End();

		for( ; i != end; ++i )
		{
			/// 일정갯수 이상은 찍지 않는다
			if( count == mColumn )
				break;

			sBuffBaseIcon* p = (sBuffBaseIcon*)(*i);
			if( !p )
			{
				assert(0);
				continue;
			}
			
			p->mImage.Draw();
			count++;
		}
	}
}

void cPartyBuff::UpdateWindowSize()
{
	/// window 재계산
	cUIRect rc;
	rc.mLeft = GetRelativeRect().mLeft;
	rc.mTop = GetRelativeRect().mTop;

	int buffCount = mBuffList.GetSize();
	int debuffCount = mDeBuffList.GetSize();

	if( buffCount > 0 )
		rc.mRight = rc.mLeft + (mIconSize+mColumnSpace) * buffCount - mColumnSpace;
	else
		rc.mRight = 0;

	if( debuffCount > 0 )
	{
		rc.mBottom = rc.mTop + (mIconSize+mRowSpace) * 2 - mRowSpace;

		/// 디버프가 더 만으면 디버프갯수로 재계싼
		if( buffCount < debuffCount )
			rc.mRight = rc.mLeft + (mIconSize+mColumnSpace) * debuffCount - mColumnSpace;
	}
	else
		rc.mBottom = rc.mTop + mIconSize;

	SetRelativeRect( rc );
}

bool cPartyBuff::IsHaveBuff()
{
	return ( mBuffList.GetSize() > 0 || mDeBuffList.GetSize() > 0 );
}
