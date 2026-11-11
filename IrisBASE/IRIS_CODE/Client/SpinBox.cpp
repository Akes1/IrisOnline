#include "StdAfx.h"
#include "SpinBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIContainer.h"

#include "Button.h"
#include "Label.h"

cSpinBox::cSpinBox( eUINodeType type )
: cSpinCtrl( type )
, mpLabel(0)
, mpLeftButton(0)
, mpRightButton(0)
, mRotation(true)
{	
	mSelectedIdx = 0;
}

cSpinBox::~cSpinBox()
{	
	mpLabel = 0;
	mpLeftButton = 0;
	mpRightButton = 0;

	mSelectedIdx = 0;
}

/// 주의사항 : 
bool cSpinBox::OnCreate( cUINodeProperty* )
{
	mpLeftButton = new cButton;
	if( mpLeftButton->CreateBySkinName( mLeftButtonSkin, this, eSPINBOX_LEFTBTN ) == false )
	{
		return false;
	}

	mpRightButton = new cButton;
	if( mpRightButton->CreateBySkinName( mRightButtonSkin, this, eSPINBOX_RIGNTBTN ) == false )
	{
		return false;
	}

	mpLabel = new cLabel;
	if( mpLabel->CreateBySkinName( mLabelSkin, this, eSPINBOX_LABEL ) == false )
	{
		return false;
	}

	if( !mpLabel || !mpLeftButton || !mpRightButton )
	{
		assert( 0 && "failed to create spinbox child" );
		return false;
	}

	/// 라벨과 라벨 배경이미지를 중앙으로 정렬시킨다
	mpLabel->SetAlign( this, eALIGN_CENTER );

	/// left 버튼 위치 세팅
	cUIPos leftpos;
	leftpos.mX = 0;
	leftpos.mY = ( GetRelativeRect().GetHeight() - mpLeftButton->GetRelativeRect().GetHeight() ) / 2;
	mpLeftButton->SetRelativePos( leftpos );

	/// right 버튼 위치 세팅
	cUIPos rightpos;
	rightpos.mX = GetRelativeRect().GetWidth() - mpRightButton->GetRelativeRect().GetWidth();
	rightpos.mY = ( GetRelativeRect().GetHeight() - mpRightButton->GetRelativeRect().GetHeight() ) / 2;
	mpRightButton->SetRelativePos( rightpos );

	return true;
}


/// 주의사항 : 
bool cSpinBox::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_SPINBOX ) == false )
	{
		assert( 0 && "not spinbox skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	cSpinBoxSkin* p = (cSpinBoxSkin*)pskin;
	
	/// 자식 스킨 네임 적용 
	mLeftButtonSkin = p->mLeftButtonSkin;
	mRightButtonSkin = p->mRightButtonSkin;
	mLabelSkin = p->mLabelSkin;

	/// 스핀박스 타입 적용 
	mRotation = p->mRotation;

	cUISkin* pUISkin = UIMAN->GetSkin();
	if( !pskin )
	{
		assert(0);
		return false;
	}

	/// 스킨 네임 유효성 검사
	if( mLeftButtonSkin && pUISkin->GetNodeSkin( mLeftButtonSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}
	if( mRightButtonSkin && pUISkin->GetNodeSkin( mRightButtonSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}
	if( mLabelSkin && pUISkin->GetNodeSkin( mLabelSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}
	return true;
}

/// 주의사항 : 
bool cSpinBox::HandleEvent( const cUIEvent& event )
{
	if( mEnabled == false || mVisible == false )
	{
		return mpParent->HandleEvent( event );
	}

	if( event.mType == eUIEVENT_COMMAND )
	{
		CheckButtonEvent( event.mID );

		cUIEvent ev = event;
		ev.mpCaller = (cUINode*)this;
		mpParent->HandleEvent( ev );
	}
	return true;
}

void cSpinBox::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	if( mpLeftButton )
	{
		/// left 버튼 위치 세팅
		cUIPos leftpos;
		leftpos.mX = 0;
		leftpos.mY = ( GetRelativeRect().GetHeight() - mpLeftButton->GetRelativeRect().GetHeight() ) / 2;
		mpLeftButton->SetRelativePos( leftpos );
	}

	if( mpRightButton )
	{
		/// right 버튼 위치 세팅
		cUIPos rightpos;
		rightpos.mX = GetRelativeRect().GetWidth() - mpRightButton->GetRelativeRect().GetWidth();
		rightpos.mY = ( GetRelativeRect().GetHeight() - mpRightButton->GetRelativeRect().GetHeight() ) / 2;
		mpRightButton->SetRelativePos( rightpos );
	}

	if( mpLabel )
	{
		mpLabel->SetAlign( this, eALIGN_CENTER );
	}
}

/// 주의사항 : 
void cSpinBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, shift );
}

/// 주의사항 : 
void cSpinBox::OnLButtonUp( const cUIPos& )
{
}

/// 주의사항 : 어느 버튼이 눌렸는지 체크 
void cSpinBox::CheckButtonEvent( unsigned int id )
{
	cString str;

	switch( id )
	{
	case eSPINBOX_LEFTBTN:
		{
			mSelectedIdx--;

			/// 계속 도는 형태의 스핀박스
			if( mRotation == true )
			{
				if( mSelectedIdx < 0 )
				{
					mSelectedIdx = GetSize() - 1;
				}
			}
			/// 한뱡향의 끝까지밖에 못가는 스핀박스 
			else
			{
				if( mpRightButton->IsVisible() == false )
					mpRightButton->Show();

				/// 처음까지 가면 왼쪽 버튼을 보이지 않는다
				if( mSelectedIdx <= 0 )
				{
					mpLeftButton->Hide();
				}
			}
		}
		break;

	case eSPINBOX_RIGNTBTN:
		{
			mSelectedIdx++;

			if( mRotation == true )
			{
				if( mSelectedIdx > GetSize()-1 )
				{
					mSelectedIdx = 0;
				}
			}
			else
			{
				if( mpLeftButton->IsVisible() == false )
					mpLeftButton->Show();

				/// 끝까지가면 오른쪽 버튼을 보이지 않는다
				if( mSelectedIdx == GetSize() - 1 )
				{
					mpRightButton->Hide();
				}
			}
		}	
		break;
	}

	/// 최종 보정
	if( mSelectedIdx < 0 )
		mSelectedIdx = 0;

	/// 라벨값 갱신 
	UpdateLabel();
}

void cSpinBox::AddData( LPCTSTR str, void* pExData )
{
	cSpinCtrl::AddData( str, pExData );
	UpdateLabel();

	if( mRotation == false )
	{
		/// 현재 선택된 것이 맨마지막 이었으면,
		/// 새로추가할것을 고려해서 rightbutton 을 보여준다.
		if( GetSize() < 2 )
		{
			mpLeftButton->Hide();
			mpRightButton->Hide();
		}
		else if( mSelectedIdx <= 0 )
		{
			mpLeftButton->Hide();
			mpRightButton->Show();
		}	
		else 
		{
			mpLeftButton->Show();
			mpRightButton->Show();
		}	
	}
	else
	{
		mpLeftButton->Show();
		mpRightButton->Show();
	}
}

void cSpinBox::Clear()
{
	cSpinCtrl::Clear();
	mSelectedIdx = 0;

	mpLeftButton->Hide();
	mpRightButton->Hide();
}

void cSpinBox::UpdateLabel()
{
	if( mpLabel )
	{
		mpLabel->SetTextColor( mDefaultColor );
		mpLabel->SetAlign( this, eALIGN_CENTER );
		mpLabel->SetText( GetLabelData(mSelectedIdx) );
	}
}

void cSpinBox::SetSelectIdx( int idx )
{
	if( idx < 0 || idx > GetSize()-1 )
		return;

	mSelectedIdx = idx;
	UpdateLabel();
}

/////////////////////////////////////////////////////////////////////////////////
cSpinBoxSkin::cSpinBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mTextAlign( eALIGN_LEFT )
, mRotation( true )
{
}

cSpinBoxSkin::~cSpinBoxSkin()
{

}

/// 주의사항 : 스크립트 파일을 읽어들여서 값을 세팅한다
bool cSpinBoxSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	/// 그다음토큰을 계속 검사
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_ROTATION:
			{
				/// Rotation 다음에 오는 값을 읽어들이기 위함
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mRotation = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mRotation = false;
				}
				else
				{
					assert( 0 && "invalid token" );
					return false;
				}
			}
			break;
		case eTOKEN_LEFTBUTTON:
			{
				mLeftButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_RIGHTBUTTON:
			{
				mRightButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_LABELSKIN:
			{
				mLabelSkin = parser.ParseString();
			}
			break;
		default:
			if( cUINodeSkin::ParseLine( parser, token ) == false )
			{
				return false;
			}
			break;
		}
	}
	return true;
}
