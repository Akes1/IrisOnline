#include "stdafx.h"
#include "Item.h"
#include "ItemIcon.h"
#include "CoolTimeManager.h"
#include "Application.h"

void cItem::Clear()
{
	if( mIcon )
		mIcon->Clear();
}

void cItem::BindIcon( cItemIcon* icon )
{
	if( !icon )
	{
		assert(0);
		return;
	}

	mIcon = icon;

	if( mCount )
	{
		mIcon->SetItemIndex( mIndex );
		mIcon->SetDBIndex( mDBIndex );
		mIcon->SetNumber( mCount );
		mIcon->SetUsedFlag( true );
		mIcon->SetSeal( mSeal );

		if( mDefine )
		{
			mIcon->ChangeImage( mDefine->GetIconParam() );
			mIcon->SetTipType( mDefine->GetTipType() );
			
			/// 겹치는 아이템만 갯수 출력 세팅
			if( mDefine->GetCapacity() > 1 )
			{
				mIcon->ShowNumber( true );
			}
			else
			{
				mIcon->ShowNumber( false );
			}
		}
	}
	else
	{
		mIcon->SetItemIndex( 0 );
		mIcon->SetDBIndex( 0 );
		mIcon->SetUsedFlag( false );
		mIcon->ShowNumber( false );
		mIcon->SetSeal( false );
	}

	mIcon->SetColor( mColor );
}

/// 다른 아이템과 화면상의 위치를 변경한다
bool cItem::SwicthPosition( cUIRect& rect )
{
	if( mIcon )
	{
		mIcon->SetRelativeRect( rect );
		return true;
	}
	return false;
}

void cItem::SetColor( NiColorA color )
{
	mColor = color;

	if( mIcon )
		mIcon->SetColor( mColor );
}

void cItem::SetTwoHanded( bool twoHanded, cIconParam* iconParam, cItemDefine* define )
{
	if( twoHanded )
	{
		mIconParam = iconParam;

		if( mIcon )
		{
			mIcon->ChangeImage( iconParam );
			if( define )
				mIcon->SetTipType( define->GetTipType() );
			mIcon->SetColor( 0.5f, 0.5f, 0.5f, 1.0f );
			mIcon->SetUsedFlag( false );
			mEnableUse = false;
		}
	}
	else
	{
		if( mIcon )
		{
			if( mCount == 0 )
			{
				mIcon->ChangeImage( 0 );
				mIcon->SetUsedFlag( false );
				mEnableUse = false;
			}
			else
			{
				mIcon->SetUsedFlag( true );
				mEnableUse = true;
			}	
		}
	}
}

void cItem::SetCount( unsigned short count )
{
	mCount = count;

	if( mCount == 0 )
	{
		/// lock 이 걸려있다면 해제
		if( mLock == true )
			SetLock( false );

		operator=( 0 );
	}
	else if( mIcon )
		mIcon->SetNumber( mCount );
}

void cItem::SetUsedFlag( bool enabled, bool colorchange )
{
	mEnableUse = enabled;

	if( mIcon )
		mIcon->SetUsedFlag( enabled, colorchange );
}

void cItem::SetSeal( bool seal )
{
	mSeal = seal;

	if( mIcon )
		mIcon->SetSeal( seal );
}

void cItem::SetCash( bool cash )
{
	mCash = cash;
}

void cItem::SetLock( bool lock )
{
	mLock = lock;

	if( mLock == true )
	{
		if( mIcon )
		{
			mIcon->SetUsedFlag( false );
		}
	}
	else
	{
		if( mIcon )
		{
			mIcon->SetUsedFlag( true );
			mIcon->SetColor( mColor );
		}
	}
}

bool cItem::StartCoolTime( unsigned long cool )
{
	if( mIcon == 0 )
		return false;

	/// 갯수가 0개 이상이어야한다.
	if( mCount <= 0 )
		return false;

	if( mIcon->IsActive() )
		return false;

	mIcon->StartDelay( cool );
	return true;
}

void cItem::EndCoolTime()
{
	if( mIcon == 0 )
		return;

	if( IsCoolTime() == true )
	{
		/// 쿨타임종료
		mIcon->StopDelay();
	}
}

bool cItem::IsCoolTime() const
{
	if( mIcon == 0 )
		return false;

	return mIcon->IsActive();
}

void cItem::CopyCoolTime( unsigned long startTime, unsigned long restTime, unsigned long totalTime )
{
	if( mIcon )
		mIcon->UpdateGauge( startTime, totalTime - restTime, totalTime, true );
}

/// 값을 복사 할 것과, 포인터를 참조할 것을 잘 구분해야한다!!
cItem& cItem::operator=( cItem& other )
{
	mIndex = other.mIndex;
	mDBIndex = other.mDBIndex;
	mCardIndex1 = other.mCardIndex1;
	mCardIndex2 = other.mCardIndex2;
	mCardIndex3 = other.mCardIndex3;
	mCardIndex4 = other.mCardIndex4;
	mCardIndex5 = other.mCardIndex5;
	mEnhanced = other.mEnhanced;
	mCount = other.mCount;
	mSeal = other.mSeal;
	mDefine = other.mDefine;
	mEquipType = other.mEquipType;
	mFlag = other.mFlag;
	mEnableUse = other.mEnableUse;
	mShowNumber = other.mShowNumber;
	mTipType = other.mTipType;
	mColor = other.mColor;
	mCash = other.mCash;
	mItemBill = other.mItemBill;

	/// 아이콘
	cItemIcon* otherIcon = other.mIcon;

	if( mIcon )
	{
		if( otherIcon )
		{
			/// 대입
			*mIcon = *otherIcon;
		}
		else
		{
			mIcon->mParam = other.mIconParam;
			mIcon->mImageFont = other.mImageFont;
			mIcon->mItemIndex = other.mIndex;
			mIcon->mDBIndex = other.mDBIndex;
			mIcon->mNumber = other.mNumber;
			mIcon->mNumText = other.mNumText;
			mIcon->mGauge = other.mGauge;
			mIcon->ShowNumber( other.mShowNumber );
			mIcon->SetUsedFlag( other.mEnableUse );
			mIcon->SetAniParam( other.mAniParam );
			mIcon->SetTipType( other.mTipType );
			mIcon->SetColor( other.mColor );
			mIcon->SetSeal( other.mSeal );
		}

		mIcon->ChangeImage( mIcon->mParam ); 
	}
	else
	{
		if( otherIcon )
		{
			mIconParam = otherIcon->mParam;
			mImageFont = otherIcon->mImageFont;
			SetAniParam( otherIcon->mAniParam );
			mNumber = otherIcon->mNumber;
			mNumText = otherIcon->mNumText;
			mGauge = otherIcon->mGauge;
			mShowNumber = otherIcon->mShowNumber;
			mTipType = otherIcon->mTipType;
			mColor = otherIcon->mColor;
			mSeal = otherIcon->mSeal;
		}
		else
		{
			mIconParam = other.mIconParam;
			mImageFont = other.mImageFont;
			SetAniParam( other.mAniParam );
			mNumber = other.mNumber;
			mNumText = other.mNumText;
			mShowNumber = other.mShowNumber;
			mTipType = other.mTipType;
			mColor = other.mColor;
			mSeal = other.mSeal;
			mCash = other.mCash;
		}
	}
	return *this;
}

cItem& cItem::operator=( unsigned int i )
{
	if( i )
	{
		SetCount( (unsigned short)i );
	}
	else
	{
		mItemBill = 0;
		mIndex = 0;
		mDBIndex = 0;
		mCardIndex1 = 0;
		mCardIndex2 = 0;
		mCardIndex3 = 0;
		mCardIndex4 = 0;
		mCardIndex5 = 0;
		mEnhanced = 0;
		mCount = 0;
		mSeal = false;
		mDefine = 0;
		mEquipType = ITEM_EQUIP_NORMAL;
		mFlag = ITEM_FLAG_NULL;
		mIconParam = 0;
		mImageFont = 0;
		mNumber = 0;
		mShowNumber = false;
		mColor = NiColorA::WHITE;
		mEnableUse = true;
		mLock = false;
		mCash = false;
		mItemBill = 0;
		::memset( mAniParam, 0, sizeof(mAniParam) );

		mTipType = eTOOLTIP_NONE;

		/// 아이콘
		if( mIcon )
		{
			mIcon->mParam = 0;
			mIcon->mItemIndex = 0;
			mIcon->mDBIndex = 0;
 			mIcon->mNumber = 0;
			mIcon->SetColor( 1.0f, 1.0f, 1.0f, 1.0f );
			mIcon->SetUsedFlag( false );
			mIcon->ShowNumber( false );
			mIcon->StopDelay();
			mIcon->SetAniParam( 0 );
			mIcon->SetSeal( false );
		}
	}
	return *this;
}

void cItem::SetAniParam( sAniParam* param )
{
	if( param == 0 )
	{
		for( unsigned int i = 0; i < eANI_MAX; ++i )
		{
			mAniParam[i].mAniOn = false;
		}
	}
	else
	{
		for( unsigned int i = 0; i < eANI_MAX; ++i, ++param )
		{
			mAniParam[i].mFrame = (*param).mFrame;
			mAniParam[i].mTexX = (*param).mTexX;
			mAniParam[i].mTexY = (*param).mTexY;
			mAniParam[i].mWidth = (*param).mWidth;
			mAniParam[i].mHeight = (*param).mHeight;
			mAniParam[i].mCurFrame = (*param).mCurFrame;
			mAniParam[i].mAniOn = (*param).mAniOn;
			mAniParam[i].mTimeOut = (*param).mTimeOut;
			mAniParam[i].mAccumTime = (*param).mAccumTime;
			mAniParam[i].mLastAccumTime = (*param).mLastAccumTime;
			mAniParam[i].mReturn = (*param).mReturn;
		}
	}
}
