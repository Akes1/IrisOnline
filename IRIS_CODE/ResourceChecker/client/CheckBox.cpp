#include "StdAfx.h"
#include "CheckBox.h"
#include "Token.h"
#include "UISkinLexer.h"
#include "Parser.h"

#include "UIEvent.h"
#include "UIImage.h"
#include "UIContainer.h"
//#include "SoundSystem.h"

cCheckBox::cCheckBox( eUINodeType type )
: cUINode( type )
, mCheckPos(0, 0)
, mNoCheckPos(0, 0)
, mOriginalTexWidth(0)
, mOriginalTexHeight(0)
{
	mCheckSoundIdx = (unsigned long)-1;
	mNoCheckSoundIdx = (unsigned long)-1;
}

cCheckBox::~cCheckBox()
{
}

//////////////////////////////////////////////////////////////////////////
cCheckBoxSkin::cCheckBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mPressed( false )
, mCheckPos(0, 0)
, mNoCheckPos(0, 0)
{
	mCheckSoundIdx = (unsigned long)-1;
	mNoCheckSoundIdx = (unsigned long)-1;
}

cCheckBoxSkin::~cCheckBoxSkin()
{
}

/// 주의사항 : 
bool cCheckBoxSkin::Load( cParser& parser )
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
		case eTOKEN_ONSOUND:
			{
				/// 체크시 사운드
				mCheckSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_OFFSOUND:
			{
				/// 체크 해제시 사운드
				mNoCheckSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_CHECKPOS:
			{
				mCheckPos.mX = parser.ParseInt();
				mCheckPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_NOCHECKPOS:
			{
				mNoCheckPos.mX = parser.ParseInt();
				mNoCheckPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_CHECKED:
			{
				/// checked 다음에 오는 값을 읽어들이기 위함
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mPressed = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mPressed = false;
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
