#include "StdAfx.h"
#include "Gauge.h"
#include "Token.h"
#include "UISkinLexer.h"
#include "Parser.h"

#include "UIImage.h"
#include "UIContainer.h"

cGauge::cGauge( eUINodeType type )
: cUINode( type )
//, mpGaugeImage(0)
, mValue(0)
, mDestValue(0)
, mMaxValue(0)
, mPercent(0)
, mSpeed(0)
, mState( eGaugeState_None )
, mStartTime(0)
, mShowPercent( false )
, mShowText( true )
, mTextX(0)
, mTextY(0)
, mpImageTexture(0)
, mImagePos(0, 0)
, mImageSize(0, 0)
, mImageTexpos(0, 0)
, mGaugeSize(0, 0)
, mImageStart( true )
, mRightDecrease( false )
{

	mProcessEnable = true;
}

cGauge::~cGauge()
{
//	SAFE_DELETE( mpGaugeImage );
}

/////////////////////////////////////////////////////////////////////////
/// 이중게이지
cDoubleGauge::cDoubleGauge( eUINodeType type )
: cGauge( type )
//, mpDestGaugeImage(0)
{

}

cDoubleGauge::~cDoubleGauge()
{
//	SAFE_DELETE( mpDestGaugeImage );
//	SAFE_DELETE( mpGaugeImage );
}


//////////////////////////////////////////////////////////////////////////
cGaugeTB::cGaugeTB( eUINodeType type )
: cGauge( type )
{

}

cGaugeTB::~cGaugeTB()
{

}


/////////////////////////////////////////////////////////////////////////////
cGaugeSkin::cGaugeSkin( eUINodeType type )
: cUINodeSkin( type )
, mGaugeSpeed( 0 )
, mShowPercent( false )
, mShowText( true )
, mpImageTexture(0)
, mImagePos(0, 0)
, mImageSize(0, 0)
, mImageTexpos(0, 0)
, mGaugeSize(0, 0)
, mImageStart( true )
, mpDestImageTexture(0)
, mDestImagePos(0, 0)
, mDestImageSize(0, 0)
, mDestImageTexpos(0, 0)
, mDestGaugeSize(0, 0)
, mDestImageStart( true )
{	
}

cGaugeSkin::~cGaugeSkin()
{

}

/// 주의사항 : 스크립트 파일을 읽어들여서 값을 세팅한다
bool cGaugeSkin::Load( cParser& parser )
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
		case eTOKEN_GAUGEIMAGEINDEX:
			{
				int i = parser.ParseInt();
				mpImageTexture = UIMAN->GetSkin()->GetTexture( i );
			}
			break;
		case eTOKEN_GAUGEPOS:
			{
				mImagePos.mX = (unsigned short)parser.ParseInt();
				mImagePos.mY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGESIZE:
			{
				mImageSize.mWidth = (unsigned short)parser.ParseInt();
				mImageSize.mHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGETEXPOS:
			{
				mImageTexpos.mX = (unsigned short)parser.ParseInt();
				mImageTexpos.mY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGEWIDTH:
			{
				mGaugeSize.mWidth = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGEHEIGTH:
			{
				mGaugeSize.mHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGESTART:
			{
				///
				lexer->GetNextToken( &token );

				if( token.mType == eALIGN_LEFT )
				{
					mImageStart = true;
				}
				else if( token.mType == eTOKEN_RIGHT )
				{
					mImageStart = false;
				}
			}
			break;
		case eTOKEN_GAUGEIMAGEINDEX_DEST:
			{
				int i = parser.ParseInt();
				mpDestImageTexture = UIMAN->GetSkin()->GetTexture( i );
			}
			break;
		case eTOKEN_GAUGEPOS_DEST:
			{
				mDestImagePos.mX = (unsigned short)parser.ParseInt();
				mDestImagePos.mY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGESIZE_DEST:
			{
				mDestImageSize.mWidth = (unsigned short)parser.ParseInt();
				mDestImageSize.mHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGETEXPOS_DEST:
			{
				mDestImageTexpos.mX = (unsigned short)parser.ParseInt();
				mDestImageTexpos.mY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGEWIDTH_DEST:
			{
				mDestGaugeSize.mWidth = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGEHEIGTH_DEST:
			{
				mDestGaugeSize.mHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGESTART_DEST:
			{
				///
				lexer->GetNextToken( &token );

				if( token.mType == eALIGN_LEFT )
				{
					mDestImageStart = true;
				}
				else if( token.mType == eTOKEN_RIGHT )
				{
					mDestImageStart = false;
				}
			}
			break;
		case eTOKEN_GAUGE_SPEED:
			{
				mGaugeSpeed = parser.ParseFloat();
			}
			break;
		case eTOKEN_SHOW_LABEL_PERCENT:
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mShowPercent = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mShowPercent = false;
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
		case eTOKEN_SHOW_TEXT:
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mShowText = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mShowText = false;
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

