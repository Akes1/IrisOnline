#include "StdAfx.h"
#include "SpinBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIImage.h"
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
					cString msg;
					msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
					MessageBoxA( NULL, msg.Cstr(), "invalid token", MB_OK | MB_ICONERROR );
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
