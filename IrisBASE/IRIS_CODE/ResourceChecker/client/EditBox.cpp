#include "StdAfx.h"
#include "EditBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

cEditBox::cEditBox( eUINodeType type )
: cUIIMENode( type )
, mSelectionImage( 0 )
, mEditX( 0 )
, mEditY( 0 )
, mLastTick( 0 )
, mEditTabIndex(0)
, mShowLen(0)
, mShowCaretFlag( false )
, mCaretTimeFlag( false )
, mCopyFlag( true )
, mEditingPos(0, 0)
, mSelectionStartPos( 0, 0 )
, mTextAlign( eALIGN_LEFT )
{
	mOriginalTexRect.Set( 0, 0, 0, 0 );
	
	::ZeroMemory( mShowText, sizeof(mShowText) );

	mTextColor = mDefaultColor;

	mProcessEnable = true;
}

cEditBox::~cEditBox()
{
//	SAFE_DELETE( mSelectionImage );
}

/////////////////////////////////////////////////////////////////////////////////////
cEditBoxSkin::cEditBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mTextAlign( eALIGN_LEFT )
, mMaxEditLength( 110 )
, mEditKind(0)
, mpSelectionTexture( 0 )
, mEditPos(0, 0)
, mSelectionSkin(0)
, mEditWidth(0)
{
	mSelectionSkin = new sSkinInfo;
}

cEditBoxSkin::~cEditBoxSkin()
{
	SAFE_DELETE( mSelectionSkin );
}

/// 주의사항 : 
bool cEditBoxSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

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
			/// 드래깅 영역 이미지 설정 
		case eTOKEN_CHILD_IMAGEINDEX:
			{
				int i = parser.ParseInt();
				mpSelectionTexture = UIMAN->GetSkin()->GetTexture( i );
			}
			break;
		case eTOKEN_CHILD_SIZE:
			{
				mSelectionSkin->mWidth = mSelectionSkin->mTexWidth = (unsigned short)parser.ParseInt();
				mSelectionSkin->mHeight = mSelectionSkin->mTexHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_CHILD_TEXPOS:
			{
				mSelectionSkin->mTexX = (unsigned short)parser.ParseInt();
				mSelectionSkin->mTexY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_EDITKIND:
			{
				mEditKind = parser.ParseInt();
			}
			break;
		case eTOKEN_TEXT_ALIGN:
			{
				///
				lexer->GetNextToken( &token );

				/// 정렬 
				mTextAlign = eALIGN_LEFT;

				if( token.mType == eTOKEN_CENTER )
				{
					mTextAlign = eALIGN_CENTER;
				}
				else if( token.mType == eTOKEN_RIGHT )
				{
					mTextAlign = eALIGN_RIGHT;
				}
			}
			break;
		case eTOKEN_TEXT:
			{
				/// 에디트 박스에 미리 텍스트를 지정해 놓을경우를 대비함.
				int i = parser.ParseInt();
				if( UIMAN->GetUIText( &mText, i ) == false )
				{
					cString msg;
					msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
					MessageBoxA( NULL, msg.Cstr(), "Bad Interface text index", MB_OK | MB_ICONERROR );
					return false;
				}
			}
			break;
		case eTOKEN_EDITPOS:
			{
				mEditPos.mX = parser.ParseInt();
				mEditPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_EDITWIDTH:
			{
				mEditWidth = parser.ParseInt();
			}
			break;
		case eTOKEN_EDITMAXLEN:
			{
				mMaxEditLength = parser.ParseInt();
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
