#include "Stdafx.h"
#include "NumberEditBox.h"
#include "ChatManager.h"
#include "StageManager.h"


static TCHAR buffer[ 80 ];

cNumberEditBox::cNumberEditBox( eUINodeType type )
: cEditBox( type )
, mMaxValue( ULONG_MAX )
, mValue( 0 )
, mUseZero( true )
{
}

cNumberEditBox::~cNumberEditBox()
{
}

void cNumberEditBox::Clear()
{
	mValue = 0;
	mMaxValue = INT_MAX;

	if( mUseZero )
		SetText( _T( "0" ), mTextAlign  );
	else
		SetText( _T( "" ), mTextAlign  );
}

bool cNumberEditBox::OnCreate( cUINodeProperty* property )
{
	if( cEditBox::OnCreate( property ) == false )
		return false;

	if( mUseZero )
		SetText( _T( "0" ), mTextAlign  );
	else
		SetText( _T( "" ), mTextAlign  );
	return true;
}

void cNumberEditBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnLButtonDown( pos, ctrl, alt, shift );
	
	/// 에디트가 가능한 에디트 박스만 실행 
	if( mEditKind != eEDIT_NOINPUT )
	{
		/// 포커스를 가진다 
		if( GetFocus() != this )
		{
			SetFocus();
			SetCapture();
		}

		/// 에디트가 눌림 
		SetAllSelect();
	}
}

/// 주의사항 : 
void cNumberEditBox::OnKeyDown( eKeyCode code )
{
	switch( code )
	{
	case eKEY_C:
		{
			/// 컨트롤키를 동시에 누르면,
			if( mControlKey )
			{
				KeyDownCopy();
			}
		}
		break;
	case eKEY_V:
		{
			if( mControlKey )
			{
				KeyDownPaste();
			}
		}
		break;
	case eKEY_TAB:
		{
			/// 게임스테이지가 아니면 리턴 
			if( STAGEMAN->GetCurrentStage() != eStage_Game )
				return;

			if( mShiftKey )
			{
				/// shfit + tab 키시 포커스 체인지 
				CHATMANAGER->ChangeFocus();
			}
		}
		break;
	case eKEY_HOME:
		{
			mDraggingFlag = false;
			CaretMoveHome();
		}
		break;
	case eKEY_END:
		{
			mDraggingFlag = false;
			CaretMoveEnd();
		}
		break;
	case eKEY_BACK:
		{
			/// 선택영역이 있으면,
			if( mDraggingFlag )
			{
				DeleteSell();
			}
			else
			{
				KeyDownBackSpace();
			}
		}
		break;
	case eKEY_DELETE:
		{
			/// 선택영역이 있으면,
			if( mDraggingFlag )
			{
				DeleteSell();
			}
			else
			{
				KeyDownDelete();
			}
		}
		break;
	default:
		break;
	}
}

void cNumberEditBox::OnChar( unsigned int wparam )
{
	cEditBox::OnChar( wparam );

	const int previousLenght = ( int )_tcslen( mText );

	SetText( FormatNumber( mText ), mTextAlign );

	if( previousLenght < (int)mTextLength )
	{
		CaretMoveRight();
	}

	/// 현재 캐럿 위치가 쉼표이면 한칸 이동
	if( _T(',') == mText[ mTextCaretX ] )
	{
		CaretMoveRight();
	}
}

void cNumberEditBox::OnFocusLost()
{
	mValue = GetValue();

	if( mValue == 0 )
	{
		if( mUseZero )
			SetText( _T( "0" ), mTextAlign  );
		else
			SetText( _T( "" ), mTextAlign  );
	}

	cEditBox::OnFocusLost();
}

LPCTSTR cNumberEditBox::UnformatNumber( LPCTSTR text ) const
{	
	LPTSTR b = buffer;
	for( LPCTSTR p = text; *p; ++p )
	{
		if( *p != ',' )
		{
			*b++ = *p;
		}
	}

	*b = _T('\0');

	return buffer;
}

LPCTSTR cNumberEditBox::FormatNumber( LPCTSTR source ) const
{
	LPTSTR dest = buffer;

	/// 쉼표를 제거한다.
	while( *source )
	{
		if( ',' != *source )
		{
			*dest++ = *source;
		}

		++source;
	}

	*dest = 0;

	return FormatNumber( ::_tstof( buffer ), mUseZero );
}


LPCTSTR cNumberEditBox::FormatNumber( double value, bool zero )
{
	if( !value )
		return ( zero == true ) ? _T( "0" ) : _T("");

	TCHAR buffer1[ 20 ] = {0,};
	static TCHAR buffer2[ 20 ] = {0,};

	::_stprintf_s( buffer1, _T( "/%0.0f" ), value );

	assert( ::_tcslen( buffer1 ) < 20 );
	LPCTSTR source = buffer1 + ::_tcslen( buffer1 ) - 1;
	LPTSTR dest	= buffer2 + sizeof( buffer2 )/sizeof(TCHAR) - 1;

	*dest-- = '\0';

	int gap = 3;	/// 자리수
	while( _T('/') != *source )
	{
		if( ! gap )
		{
			*( dest-- ) = _T(',');            
			gap = 3;
		}

		--gap;
		*( dest-- )= *( source-- );
	}

	return ++dest;
}

void cNumberEditBox::CaretMoveLeft()
{
	cEditBox::CaretMoveLeft();

	if( _T(',') == mText[ mTextCaretX ] )
	{
		cEditBox::CaretMoveLeft();
	}
}


void cNumberEditBox::CaretMoveRight()
{
	cEditBox::CaretMoveRight();

	if( _T(',') == mText[ mTextCaretX ] )
	{
		cEditBox::CaretMoveRight();
	}
}


void cNumberEditBox::KeyDownDelete()
{
	cEditBox::KeyDownDelete();

	::_tcscpy_s( mText, FormatNumber( mText ) );
	mTextLength	= ::_tcslen( mText );
	
	if( _T(',') == mText[ mTextCaretX ] )
	{
		cEditBox::CaretMoveRight();
	}

	UpdateText();
	UpdateShowText();
}


void cNumberEditBox::KeyDownBackSpace()
{
	if( 0 < mTextCaretX && _T(',') == mText[ mTextCaretX - 1 ] )
		cEditBox::KeyDownBackSpace();
	
	cEditBox::KeyDownBackSpace();
	SetText( FormatNumber( mText ), mTextAlign );
}

void cNumberEditBox::SetText( LPCTSTR text, int align )
{
	cEditBox::SetText( text, align );
}

void cNumberEditBox::SetValue( unsigned long value )
{
	mValue = value;

	if( mValue )
	{
		SetText( FormatNumber( ( double )( mMaxValue < value ? mMaxValue : value ), mUseZero ), mTextAlign );
	}
	else
	{
		if( mUseZero )
			SetText( _T( "0" ), mTextAlign  );
		else
			SetText( _T( "" ), mTextAlign  );
	}
}


void cNumberEditBox::SetMaxValue( unsigned long value )
{
	mMaxValue = value;
}


unsigned long cNumberEditBox::GetValue() const
{
	return _tstoi( UnformatNumber( mText ) );
}
unsigned long cNumberEditBox::GetMaxValue() const
{
	return mMaxValue;
}
