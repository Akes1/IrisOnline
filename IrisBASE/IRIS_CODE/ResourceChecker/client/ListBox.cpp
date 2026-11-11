#include "StdAfx.h"
#include "ListBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIEvent.h"
#include "UIImage.h"
#include "UIContainer.h"

#include "ScrollBar.h"

cListBox::cListBox( eUINodeType type )
: cListCtrl( type )
, mpSelectImage(0)
, mpScrollBar(0)
, mpSkin(0)
, mSelectBarWidth(0)
, mContinueEvent( false )
, mShowImage( true )
, mPrintPos(0, 0)
, mPrintSize(0, 0)
{
	mMaxRowInPage = 0;
	mTotalMaxNum = 0;

	mSelectedRowIdx = 0;
	mHoveredRowIdx = -1;
	mImageHoveredIdx = -1;

	mTopRowIdx = 0;
	mRowHeight = 0;

	mHoverTextColor = COLOR_WHITE;
}

cListBox::~cListBox()
{
	mpScrollBar = 0;

//	SAFE_DELETE( mpSelectImage );
}

///////////////////////////////////////////////////////////////////////////////
cListBoxSkin::cListBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mTotalMaxNum( 0 )
, mRowHeight( 0 )
, mSelectBarSkin(0)
, mpImgParam(0)
, mpSelectBarTexture(0)
, mSelectBarWidth(0)
, mPrintPos(0, 0)
, mPrintSize(0, 0)
{
	mSelectBarSkin = new sSkinInfo;
	mpImgParam = new sListCtrlParam;

//	mRowHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_UI);
}

cListBoxSkin::~cListBoxSkin()
{
	///
	for( unsigned int i = 0; i < mCtrlParamArr.GetSize(); ++i)
	{
		delete (sListCtrlParam*)mCtrlParamArr[i];
	}
	mCtrlParamArr.Clear();

	SAFE_DELETE( mpImgParam );
	SAFE_DELETE( mSelectBarSkin );
}

/// ÁÖÀÇ»çÇ× : 
bool cListBoxSkin::Load( cParser& parser )
{
	for( unsigned int i = 0; i < mCtrlParamArr.GetSize(); ++i)
	{
		delete (sListCtrlParam*)mCtrlParamArr[i];
	}
	mCtrlParamArr.Clear();

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
			/// ºí·Ï Å»Ãâ ^^
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_PRINTPOS:
			{
				mPrintPos.mX = parser.ParseInt();
				mPrintPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_PRINTSIZE:
			{
				mPrintSize.mWidth = parser.ParseInt();
				mPrintSize.mHeight = parser.ParseInt();
			}
			break;
		case eTOKEN_IMGCOLX:
			{
				mpImgParam->mColumnX = parser.ParseInt();
				mpImgParam->mColumnW = parser.ParseInt();
				if( mpImgParam->mColumnW <= 0 )
				{
					assert(0);
					return 0;
				}
			}
			break;
		case eTOKEN_COLUMNX:
			{
				/// ÁÂÇ¥ 
				unsigned columnX = parser.ParseInt();

				/// ³ÐÀÌ 
				unsigned columnW = parser.ParseInt();
				if( columnW <= 0 )
				{
					assert(0);
					return 0;
				}

				///
				lexer->GetNextToken( &token );

				/// Á¤·Ä 
				eText_Align align = eALIGN_LEFT;

				if( token.mType == eTOKEN_CENTER )
				{
					align = eALIGN_CENTER;
				}
				else if( token.mType == eTOKEN_RIGHT )
				{
					align = eALIGN_RIGHT;
				}

				/// µ¥ÀÌÅÍ »ðÀÔ
				sListCtrlParam* pCtrlParam = new sListCtrlParam;
				pCtrlParam->mColumnX = columnX;
				pCtrlParam->mColumnW = columnW;
				pCtrlParam->mAlign = align;
				mCtrlParamArr.PushBack( pCtrlParam );
			}
			break;
		case eTOKEN_ROWHEIGHT:
			{
				mRowHeight = parser.ParseInt();
			}
			break;
		case eTOKEN_MAXITEMNUM:
			{
				mTotalMaxNum = parser.ParseInt();
			}
			break;
			/// ui image °ü·Ã 
		case eTOKEN_CHILD_IMAGEINDEX:
			{
				int i = parser.ParseInt();
				mpSelectBarTexture = UIMAN->GetSkin()->GetTexture( i );
				if( mpSelectBarTexture == 0 )
				{
					assert( 0 && "failed to load texture" );
					return false;
				}
			}
			break;
		case eTOKEN_CHILD_SIZE:
			{
				mSelectBarSkin->mWidth = mSelectBarSkin->mTexWidth = (unsigned short)parser.ParseInt();
				mSelectBarSkin->mHeight = mSelectBarSkin->mTexHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_CHILD_TEXPOS:
			{
				mSelectBarSkin->mTexX = (unsigned short)parser.ParseInt();
				mSelectBarSkin->mTexY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_SELECTWIDTH:
			{
				mSelectBarWidth = parser.ParseInt();
			}
			break;
		case eTOKEN_SCROLLSKIN:
			{
				mScrollSkin = parser.ParseString();
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

///
sListCtrlParam*	cListBoxSkin::GetCtrlParam( unsigned int index )
{
	if( mCtrlParamArr.GetSize() == 0 )
		return 0;

	if( index > mCtrlParamArr.GetSize() - 1 )
	{
		assert(0);
		return 0;
	}

	sListCtrlParam* pParam = (sListCtrlParam*)mCtrlParamArr[index];
	assert( pParam );

	return pParam;
}
