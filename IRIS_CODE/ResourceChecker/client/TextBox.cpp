#include "StdAfx.h"
#include "TextBox.h"
#include "Token.h"
#include "UISkinLexer.h"
#include "Parser.h"

#include "UIContainer.h"
#include "UIImage.h"
#include "UIText.h"
#include "UIFuncText.h"
#include "Token.h"
#include "ScrollBar.h"
#include "Item_Common.h"

cTextBox::cTextBox( eUINodeType type )
: cTextCtrl( type )
, mTextPosX(0)
, mTextPosY(0)
, mNextPosY(0)
, mTopRowIdx(0)
, mMaxRowInPage(0)
, mRowHeight(0)
, mpScrollBar(0)
, mFixScrollH(0)
, mDrawTextBottom( true )
, mScrollBottom( false )
{
	mScrollbarPosType = eRIGHT_SCROLLBAR;
	mTextColor = mDefaultColor;
}

cTextBox::~cTextBox()
{
	/// 스크롤바 소멸( 부모단에서 해줌 )
	mpScrollBar = 0;
}

///
 void cTextBox::Clear()
 {
 	cTextCtrl::Clear();
 
 	/// 모든 값 초기화 
 	mTopRowIdx = 0;
 
//  	/// 스크롤바 초기화 
//  	UpdateScrollBarButton();
//  
//  	/// 스크롤바 위치 초기화 
//  	UpdateScrollBarPos();
 }

//////////////////////////////////////////////////////////////////////////////
cTextBoxSkin::cTextBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mRowHeight( 0 )
, mTotalMaxNum( 0 )
, mScrollBarRightPos(true)
{
}

cTextBoxSkin::~cTextBoxSkin()
{

}

/// 주의사항 : 스크립트 파일을 읽어들여서 값을 세팅한다
bool cTextBoxSkin::Load( cParser& parser )
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
		case eTOKEN_ROWHEIGHT:
			{
				mRowHeight = parser.ParseInt();
			}
			break;
		case eTOKEN_SCROLLBAR_RIGHTPOS:
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mScrollBarRightPos = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mScrollBarRightPos = false;
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
		case eTOKEN_SCROLLSKIN:
			{
				mScrollSkin = parser.ParseString();
			}
			break;
		case eTOKEN_MAXITEMNUM:
			{
				mTotalMaxNum = parser.ParseInt();
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
