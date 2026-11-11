#include "StdAfx.h"
#include "TipWindow.h"
#include "PlaneObject.h"
#include "FontAgent.h"

cTipWindow::cTipWindow()
: mMiddleBack( 0 )
, mIndex(0)
, mMaxWidth(0)
, mGapCount(0)
, mLineHeight(0)
, mLimitMinWidth(30)
, mLimitMaxWidth(250)
{
	mX = 0;
	mY = 0;
	mWidth = 0;
	mHeight = 0;

	mLineHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_System) + LINEGAP_SIZE;

	SetBackupVisible( false );

	memset( mpBackImage, 0, sizeof(mpBackImage) );

	SetExceptSnap( true );
}

cTipWindow::~cTipWindow()
{
	SAFE_DELETE( mMiddleBack );

	for( int i = 0; i < eBACKIMAGE_MAX; ++i )
		SAFE_DELETE( mpBackImage[i] );
}

void cTipWindow::Clear()
{
	for( unsigned int i=0; i<MAX_TIP; i++ )
	{
		mToolTip[i].mString.Clear();
		mToolTip[i].mGapCount = 0;
	}

	//
	mUpdateSizebyTime = true;

	mMaxWidth = 0;
	mIndex = 0;
	mGapCount = 0;
}

void cTipWindow::OnHide()
{
	cUIWindow::OnHide();

	UIMAN->HideTip();
}

bool cTipWindow::OnCreate( cUINodeProperty* prop )
{
	if( cUIWindow::OnCreate( prop ) == false )
		return false;

	SetExceptEvent( true );

	///
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 배경 이미지 생성
	cUINodeSkin* pEdgeSkin = pSkin->GetNodeSkin( "Tip_Edge" );
	if( pEdgeSkin && pEdgeSkin->mpTexture )
	{
		for( int i = 0; i < eBACKIMAGE_MAX; ++i )
		{
			/// 텍스처 영역 설정
			unsigned short tx = (unsigned short)pEdgeSkin->mSkinInfo->mTexX;
			unsigned short ty = (unsigned short)pEdgeSkin->mSkinInfo->mTexY;
			unsigned short tw = (unsigned short)pEdgeSkin->mSkinInfo->mTexWidth;
			unsigned short th = (unsigned short)pEdgeSkin->mSkinInfo->mTexHeight;
			unsigned int x = GetAbsoluteRect().mLeft;
			unsigned int y = GetAbsoluteRect().mTop;

			mpBackImage[i] = new cPlaneObject;
			if( mpBackImage[i]->Create( pEdgeSkin->mpTexture, x, y, tw, th, tx, ty, tx + tw, ty + th ) == false )
			{
				assert( 0 && "failed to create back image");
				return false;
			}
		}
	}

	/*NiAlphaProperty* alphaProp = mMiddleBack->GetAlphaProp();

	alphaProp->SetAlphaBlending( true );
	alphaProp->SetSrcBlendMode( NiAlphaProperty::ALPHA_ZERO );
	alphaProp->SetDestBlendMode( NiAlphaProperty::ALPHA_SRCCOLOR );*/
	return true;
}

bool cTipWindow::SetSkin( const cUINodeSkin* skin )
{
	assert( skin );

	if( skin->IsKindof( eUINODE_WINDOW ) == false )
	{
		assert( 0 && "not window type" );
		return false;
	}

	unsigned short x = (unsigned short)skin->mSkinInfo->mX;
	unsigned short y = (unsigned short)skin->mSkinInfo->mY;
	unsigned short w = (unsigned short)skin->mSkinInfo->mWidth;
	unsigned short h = (unsigned short)skin->mSkinInfo->mHeight;

	/// 이미지 설정
	if( skin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned short tx = (unsigned short)skin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)skin->mSkinInfo->mTexY;
		unsigned short tw = (unsigned short)skin->mSkinInfo->mTexWidth;
		unsigned short th = (unsigned short)skin->mSkinInfo->mTexHeight;

		if( mMiddleBack == 0 )
		{
			mMiddleBack = new cPlaneObject;

			if( mMiddleBack->Create( skin->mpTexture,
				x, y, w, h,
				tx, ty, tx + tw, ty + th ) == false )
			{
				assert( 0 && "failed to create back image");
				return false;
			}
		}
		else
		{
			mMiddleBack->SetTextureRect( tx, ty, tx + tw, ty + th );
		}
	}

	SetRelativeRect( cUIRect(x, y, x + w, y + h) );
	return true;
}

void cTipWindow::UpdateEdge()
{
	cUIRect rect = GetAbsoluteRect();

	int x = rect.mLeft;
	int y = rect.mTop;
	int w = rect.GetWidth();
	int h = rect.GetHeight();

	if( mpBackImage[eBACKIMAGE_TOP] )
		mpBackImage[eBACKIMAGE_TOP]->SetScreenRect( x, y, w, 1 );

	if( mpBackImage[eBACKIMAGE_BOTTOM] )
		mpBackImage[eBACKIMAGE_BOTTOM]->SetScreenRect( x, y + h - 1, w, 1 );

	if( mpBackImage[eBACKIMAGE_LEFT] )
		mpBackImage[eBACKIMAGE_LEFT]->SetScreenRect( x, y, 1, h );

	if( mpBackImage[eBACKIMAGE_RIGHT] )
		mpBackImage[eBACKIMAGE_RIGHT]->SetScreenRect( x + w - 1, y, 1, h );
}

///
void cTipWindow::CalcMaxWidth( const cStringT& str )
{
	unsigned int width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), str.GetSize() );
	if( width > mMaxWidth )
		mMaxWidth = width;

	if( mMaxWidth > mLimitMaxWidth )
		mMaxWidth = mLimitMaxWidth;
}

///
void cTipWindow::SetText( const LPCTSTR str, unsigned long value, unsigned long color )
{
	mToolTip[mIndex].mString.Format( str, value );
	mToolTip[mIndex].mColor = color;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;

	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

///
void cTipWindow::SetText( const LPCTSTR str, float value, unsigned long color )
{
	mToolTip[mIndex].mString.Format( str, value );
	mToolTip[mIndex].mColor = color;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;

	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

///
void cTipWindow::SetText( const LPCTSTR str, short value, unsigned long color )
{
	mToolTip[mIndex].mString.Format( str, value );
	mToolTip[mIndex].mColor = color;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;

	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

///
void cTipWindow::SetText( LPCTSTR str, unsigned long color )
{
	/// 삽입시에 문자열 길이 검사 
	if( CheckText( str, color ) == false )
	{
		assert(0);
		return;
	}
}

///
void cTipWindow::SetText( LPCTSTR str1, LPCTSTR str2, unsigned long color )
{
	mToolTip[mIndex].mString.Format( str1, str2 );
	mToolTip[mIndex].mColor = color;
	CalcMaxWidth( mToolTip[mIndex].mString );
	mIndex++;

	if( mIndex > MAX_TIP )
	{
		assert( 0 && "error tip index" );
	}
}

///
void cTipWindow::AppendText( LPCTSTR str1, LPCTSTR str2, unsigned long color )
{
	cStringT temp1 = str1;
	cStringT temp2 = str2;
	temp1.Append( temp2 );

	/// 삽입시에 문자열 길이 검사 
	if( CheckText( (LPCTSTR)temp1.Cstr(), color ) == false )
	{
		assert(0);
		return;
	}
}

///
bool cTipWindow::CheckText( LPCTSTR str, unsigned long color )
{
	typedef tArray<cStringT> cStringArr;
	cStringArr stringArr;

	unsigned int len = ::_tcslen( str );
	if( len == 0 )
		return false;

	/// \\n 부호로 문자열 자르기
	LPCTSTR s = str;
	LPCTSTR p = str;
	TCHAR temp[256] = {0,};
	for( unsigned int i = 0; i < len - 1; ++i, ++p )
	{
		if( ::_tcsncmp( p, _T("\\n"), 2 ) == 0 )
		{
			Sstrncpy( temp, 256, s, p - s );
			stringArr.PushBack( temp );

			s = p + 2;
		}
	}

	unsigned int checkLen = p - s;
	if( checkLen < len )
	{
		Sstrncpy( temp, 256, s, len );
		stringArr.PushBack( temp );
	}

	unsigned int maxWidth = mLimitMaxWidth - GAP_SIZE;

	/// 문자열 삽입
	TCHAR tempStr[256] = {0,};
	for( unsigned int i = 0; i < stringArr.GetSize(); ++i )
	{
		cStringT data = stringArr[i];

		unsigned int width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, data.Cstr(), data.GetLength() );
		unsigned int count = width / maxWidth;

		/// 한줄이면
		if( count <= 0 )
		{
			Sstrncpy( tempStr, 256, data.Cstr(), data.GetLength() );
			mToolTip[mIndex].mString = tempStr;
			mToolTip[mIndex].mColor = color;
			CalcMaxWidth( mToolTip[mIndex].mString );
			mIndex++;
			if( mIndex > MAX_TIP )
			{
				assert( 0 && "error tip index" );
			}
		}
		/// 여러줄 이면
		else
		{
			/// 너무 길면 잘라내기.
			unsigned int cutLen = 0;
			unsigned int cutWidth = 0;

			LPCTSTR orginStr = data.Cstr();
			LPCTSTR checkStr = data.Cstr();

			bool flag = true;
			while( flag )
			{
				int length = checkStr - orginStr;
				if( cutLen + length >= data.GetLength() )
					break;

				cutLen++;
				cutWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, checkStr, cutLen );

				if( cutWidth > maxWidth )
				{
					/// 넘어간 글자 하나 땡기기
					if( cutLen > 1)
						cutLen--;

					Sstrncpy( tempStr, 256, checkStr, cutLen );
					mToolTip[mIndex].mString = tempStr;
					mToolTip[mIndex].mColor = color;
					CalcMaxWidth( mToolTip[mIndex].mString );
					mIndex++;
					if( mIndex > MAX_TIP )
					{
						assert( 0 && "error tip index" );
					}	

					::ZeroMemory( tempStr, 256 );
					checkStr += cutLen;

					cutLen = 0;
				}
			}

			if( cutLen != 0 )
			{
				Sstrncpy( tempStr, 256, checkStr, cutLen );

				mToolTip[mIndex].mString = tempStr;
				mToolTip[mIndex].mColor = color;
				CalcMaxWidth( mToolTip[mIndex].mString );
				mIndex++;
				if( mIndex > MAX_TIP )
				{
					assert( 0 && "error tip index" );
				}
			}
		}
	}

	return true;
}

void cTipWindow::AddGap( unsigned int count )
{
	mToolTip[mIndex].mGapCount += count;
	mGapCount += count;
}
