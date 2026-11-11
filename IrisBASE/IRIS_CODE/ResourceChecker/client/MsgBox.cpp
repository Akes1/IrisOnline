#include "StdAfx.h"
#include "MsgBox.h"
#include "Button.h"
#include "Label.h"
#include "EditBox.h"
#include "UISkinLexer.h"
#include "Parser.h"
#include "Application.h"

cMsgBox::cMsgBox( eUINodeType type )
: cUIWindow( type )
, mpYesButton(0)
, mpNoButton(0)
, mpEdit(0)
, mpLabel(0)
, mSkinType(eSKIN_NONE)
, mExtraData(0)
, mCaptionPos(0, 0)
, mStartTime(0)
, mTotalTime(0)
, mTimePrint(false)
, mTimeResult(false)
, mEnableLength(0)
{
	SetBackupVisible( false );

	mModalType = eMSGBOX_MODAL;
}

cMsgBox::~cMsgBox()
{
	if( mSkinType == eSKIN_EDIT )
		ReleaseFocus();

	mExtraData = 0;
	mpCaller = 0;
}

//////////////////////////////////////////////////////////////////////
cMessageBoxSkin::cMessageBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mMoveable( true )
{
	mCaptionRect.Set( 0, 0, 0, 0 );
}

cMessageBoxSkin::~cMessageBoxSkin()
{

}

/// 주의사항 : 스크립트 파일을 읽어들여서 값을 세팅한다
bool cMessageBoxSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	/// 그다음토큰을 계속 검사
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_CAPTIONXY:
			{
				mCaptionRect.mLeft = parser.ParseInt();
				mCaptionRect.mTop = parser.ParseInt();
			}
			break;
		case eTOKEN_CAPTIONWH:
			{
				mCaptionRect.mRight = mCaptionRect.mLeft + parser.ParseInt();
				mCaptionRect.mBottom = mCaptionRect.mTop + parser.ParseInt();
			}
			break;
		case eTOKEN_EDITSKIN:
			{
				mEditSkin = parser.ParseString();
			}
			break;
		case eTOKEN_OKBUTTONSKIN:
			{
				mOkButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_YESBUTTONSKIN:
			{
				mYesButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_NOBUTTONSKIN:
			{
				mNoButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_MOVABLE:
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mMoveable = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mMoveable = false;
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


