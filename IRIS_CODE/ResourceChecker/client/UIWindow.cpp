#include "StdAfx.h"
#include "UIWindow.h"
#include "UISkinLexer.h"
#include "Parser.h"

 #include "BuffWindow.h"

cUIWindow::cUIWindow( eUINodeType type )
: cUIContainer( type )
, mDragging( false )
, mTimerEnabled( false )
, mTimeOut( 0 )
, mAccumTime( 0 )
, mLastAccumTime( 0 )
, mDragStart(0, 0)
//, mMsgHandler( 0 )
, mAlphaBitArray( 0 )
, mModalType( eMSGBOX_NONE )
, mpCaller(0)
, mpRegistMsgBox(0)
, mOpenSoundIdx( ULONG_MAX )
, mCloseSoundIdx( ULONG_MAX )
, mEventType( eMBEVENT_NONE )
{
	mCaptionRect.Set( 0, 0, 0, 0);

	mProcessEnable = true;
}

cUIWindow::~cUIWindow()
{
	mpRegistMsgBox = 0;
}


//////////////////////////////////////////////////////////////////////////
cUIWindowSkin::cUIWindowSkin( eUINodeType type )
: cUIContainerSkin( type )
, mAlpha( 1.0f )
, mHoldView( false )
, mAlphaBitArray( 0 )
{
	mCaptionRect.Set( 0, 0, 0, 0 );

	mOpenSoundIdx = ULONG_MAX;
	mCloseSoundIdx = ULONG_MAX;
}

cUIWindowSkin::~cUIWindowSkin()
{
}

/// 주의사항 : 
bool cUIWindowSkin::Load( cParser& parser )
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
			break;

		switch( token.mType )
		{
		case eTOKEN_IMAGEINDEX:
			{
				int i = parser.ParseInt();
				mpTexture = UIMAN->GetSkin()->GetTexture( i );

				/// 해당 인덱스의 텍스쳐가 존재하지 않으면,
				if( !mpTexture )
				{
					assert( 0 );
					return false;
				}

				mAlphaBitArray = UIMAN->GetSkin()->GetAlphaBitArray( i );
			}
			break;
		case eTOKEN_OPENSOUND:
			{
				/// show 시 사운드
				mOpenSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_CLOSESOUND:
			{
				/// hide 시 사운드
				mCloseSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_HOLDVIEW:
			{
				/// mHoldView 다음에 오는 값을 읽어들이기 위함
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mHoldView = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mHoldView = false;
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
		case eTOKEN_ALPHA:
			{
				mAlpha = parser.ParseFloat();
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

cUINodeSkin* cUIWindowSkin::Create( const cString& name )
{
	if( name.Compare( "BuffWindow" ) == 0 || 
		name.Compare( "DeBuffWindow" ) == 0 || 
		name.Compare( "MallBuffWindow" ) == 0 || 
		name.Compare( "DuelBuffWindow1" ) == 0 || 
		name.Compare( "DuelDeBuffWindow1" ) == 0 || 
		name.Compare( "DuelBuffWindow2" ) == 0 || 
		name.Compare( "DuelDeBuffWindow2" ) == 0 || 
		name.Compare( "PartyBuff" ) == 0 || 
		name.Compare( "TargetBuff" ) == 0 )
	{
		return new cBuffWindowSkin;
	}

	return new cUIWindowSkin;
}
