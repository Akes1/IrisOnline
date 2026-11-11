#include "StdAfx.h"
#include "OptionGauge.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "BarButton.h"

cOptionGauge::cOptionGauge( eUINodeType type )
: cUIWindow( type )
, mCurPercent(0)
, mpBarButton(0)
{
	
}

cOptionGauge::~cOptionGauge()
{
	mpBarButton = 0;
}

/// 주의사항 : 
bool cOptionGauge::OnCreate( cUINodeProperty* )
{
	/// 자식 바 버튼 생성 
	mpBarButton = new cBarButton;
	if( mpBarButton->CreateBySkinName( mBarButtonSkin, this, eOPTIONGAUGE_BARBUTTON ) == false )
	{
		return false;
	}
	return true;
}

/// 주의사항 : 
bool cOptionGauge::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_OPTIONGAUGE ) == false )
	{
		assert( 0 && "not optiongauge skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	cOptionGaugeSkin* p = (cOptionGaugeSkin*)pskin;
	/// 바 버튼 스킨 이름 저장
	mBarButtonSkin = p->mBarButtonSkin;

	cUISkin* pUISkin = UIMAN->GetSkin();
	if( !pskin )
	{
		assert(0);
		return false;
	}

	/// 스킨 네임 유효성 검사
	if( mBarButtonSkin && pUISkin->GetNodeSkin( mBarButtonSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}

	return true;
}

/// 주의사항 : 
bool cOptionGauge::IsValid() const
{
	if( !mpImage )
	{
		assert( 0 && "invalid image" );
		return false;
	}

	if( cUINode::IsValid() == false )
	{
		return false;
	}
	return true;
}

/// 주의사항 : 
bool cOptionGauge::HandleEvent( const cUIEvent& event )
{
	if( mEnabled == false || mVisible == false )
	{
		return true;
	}

	/// 바 버튼이 움직이는 이벤트를 받음.
	if( event.mType == eUIEVENT_SCROLLBAR_MOVE )
	{
		UpdateBarButtonPos( event.mPos );

		if( mpParent )
		{
			cUIEvent event;
			event.mType = eUIEVENT_OPTIONGAUGE_UPDATE;
			event.mpCaller = this;
			event.mID = mID;
			mpParent->HandleEvent( event );
		}
		return true;
	}
	else if( event.mType == eUIEVENT_LBUTTON_DOWN )
	{
		OnLButtonDown( event.mPos, event.mControl, event.mAlt, event.mShift );

		if( mpParent )
		{
			cUIEvent event;
			event.mType = eUIEVENT_OPTIONGAUGE_UPDATE;
			event.mpCaller = this;
			event.mID = mID;
			mpParent->HandleEvent( event );
		}
		return true;
	}

	return mpParent->HandleEvent( event );
}

void cOptionGauge::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, shift );

	cUIRect rect = mpBarButton->GetAbsoluteRect();
	if( rect.mLeft > pos.mX )
	{
		if( mCurPercent <= 10 )
			mCurPercent = 0;
		else
			mCurPercent -= 10;
	}
	else
	{
		if( mCurPercent >= 90 )
			mCurPercent = 100;
		else
			mCurPercent += 10;
	}
	SetPercent( mCurPercent);
}

/// 주의사항 : 바 버튼이 움직이도록 함  
void cOptionGauge::UpdateBarButtonPos( const cUIPos& pos )
{
	cUIRect child = mpBarButton->GetAbsoluteRect();
	cUIPos& oldPos = mpBarButton->GetOldMousePos();

	int leftpos = GetAbsoluteRect().mLeft;
	int rightpos = GetAbsoluteRect().mRight;
	int value = 0;

	/// 왼쪽 이동이면
	if( pos.mX - oldPos.mX < 0 )
	{
		if( child.mLeft > leftpos )
		{
			if( child.mLeft + pos.mX - oldPos.mX < leftpos )
			{
				/// 최소 위치까지 보정
				value = leftpos - child.mLeft;
			}
			else
			{
				/// 마우스 이동량 만큼 이동
				value = pos.mX - oldPos.mX;
			}
		}
		mpBarButton->MoveX( value , pos );
	}
	/// 오른쪽 이동이면
	else if( pos.mX - oldPos.mX > 0 )
	{
		if( child.mRight < rightpos )
		{
			/// 
			if( child.mRight + pos.mX - oldPos.mX >= rightpos )
			{
				/// 최대 위치까지 보정
				value = rightpos - child.mRight;
			}
			else
			{
				/// 마우스 이동량 만큼 이동
				value = pos.mX - oldPos.mX;
			}
		}
		mpBarButton->MoveX( value , pos );
	}

	UpdateCurGauge();
}

void cOptionGauge::SetPercent( float per )
{
	cUIRect gaugeAbr = GetAbsoluteRect();
	cUIRect barAbr = mpBarButton->GetAbsoluteRect();

	unsigned int width = gaugeAbr.GetWidth() - barAbr.GetWidth();
	int left = int(per * width * 0.01f);

	mpBarButton->SetRelativePos( cUIPos( left, mpBarButton->GetRelativeRect().mTop ) );
	UpdateCurGauge();
}

/// 주의사항 : 옵션게이지상의 바버튼이 어느위치에 와있는지 퍼센트로 계산 
void cOptionGauge::UpdateCurGauge()
{
	cUIRect gaugeAbr = GetAbsoluteRect();
	cUIRect barAbr = mpBarButton->GetAbsoluteRect();

	/// 100 퍼센트로 생각할 길이 
	unsigned int width = gaugeAbr.GetWidth() - barAbr.GetWidth();

	unsigned int curX = mpBarButton->GetRelativeRect().mLeft;
	mCurPercent = (float)curX * 100.0f / (float)width;
}

/////////////////////////////////////////////////////////////////////
cOptionGaugeSkin::cOptionGaugeSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cOptionGaugeSkin::~cOptionGaugeSkin()
{
}

/// 주의사항 : 버튼에서 필요한 내용 따로 파싱
bool cOptionGaugeSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			/// 블록 탈출 ^^
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_BARBUTTONSKIN:
			{
				mBarButtonSkin = parser.ParseString();
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