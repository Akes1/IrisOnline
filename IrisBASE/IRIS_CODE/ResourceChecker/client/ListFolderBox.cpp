#include "StdAfx.h"
#include "ListFolderBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"
#include "UIEvent.h"
#include "ListBox.h"
#include "ScrollBar.h"
#include "UIImage.h"
#include "GameResourceManager.h"

cListFolderBox::cListFolderBox( eUINodeType type )
:cListCtrl( type )
, mpScrollBar(0)
, mpSelectImage(0)
, mTopColumnIdx(0)
, mSelectedColumnIdx(0)
, mRowHeight(0)
, mMaxRowInPage(0)
, mTotalMaxNum(0)
, mHighCheckPos(0, 0)
, mHighNoCheckPos(0, 0)
, mLowCheckPos(0, 0)
, mLowNoCheckPos(0, 0)
, mSelectedRow(0)
, mSelectedRowColumn(0)
, mColumnCount(0)
, mRenderRow(0)
, mRenderStartColumn(0)
, mLowGap(0)
, mShowNumber(true)
, mShowSelect(true)
{
}

cListFolderBox::~cListFolderBox()
{
	mpScrollBar = 0;
//	SAFE_DELETE( mpSelectImage );
}

// void cListFolderBox::Clear()
// {
// 	mSelectedColumnIdx = 0;
// 	mSelectedRow = 0;
// 	mSelectedRowColumn = 0;
// 
// 	mTopColumnIdx = 0;
// 	mColumnCount = 0;
// 
// 	UpdateScrollBar();
// 	UpdateSelectImage();
// 
// 	cListCtrl::Clear();
// }

//////////////////////////////////////////////////////////
cListFolderBoxSkin::cListFolderBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mSelectBarSkin(0)
, mpSelectBarTexture(0)
, mSelectBarWidth(0)
{
	mSelectBarSkin = new sSkinInfo;
//	mRowHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_UI);
}

cListFolderBoxSkin::~cListFolderBoxSkin()
{
	SAFE_DELETE(mSelectBarSkin);
}

/// 주의사항 : 
bool cListFolderBoxSkin::Load( cParser& parser )
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
			/// ui image 관련 
		case eTOKEN_CHILD_IMAGEINDEX:
			{
				int i = parser.ParseInt();
				mpSelectBarTexture = UIMAN->GetSkin()->GetTexture( i );
				if( mpSelectBarTexture == 0 )
				{
					assert( 0 && "failed to load texture" );
					cString msg;
					msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
					MessageBoxA( NULL, msg.Cstr(), "invalid image index", MB_OK | MB_ICONERROR );
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
