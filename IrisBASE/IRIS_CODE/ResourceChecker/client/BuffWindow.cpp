#include "Stdafx.h"
#include "BuffWindow.h"

#include "UISkinLexer.h"
#include "Parser.h"

#include "Skill_Common.h"
#include "SkillScript.h"
#include "icon.h"
#include "UIRootNode.h"


cBuffWindow::cBuffWindow()
{
	mUpdateList = false;

	mStartX = mStartY = 0;

	mOverInfo.mOverIconIdx = (unsigned int)-1;
	mOverCheck = false;
	mChangeOverIcon = false;

	mRButtonClickIconIdx = (unsigned int)-1;

	mIsHeroBuffPage = false;

	mRowSpace = 0;
	mColumnSpace = 0;

	mBuffTipShow = false;

	mRow = 0;
	mColumn = 0;
	mIconSize = 0;
	mScale = 1;

	/// 다른 윈도우들이 안붙게 한다
//	SetExceptSnap( true );
}

cBuffWindow::~cBuffWindow()
{
	cBuffIconList::cIterator i = mIconList.Begin();
	cBuffIconList::cIterator iend = mIconList.End();

	for( ; i != iend; ++i )
	{
		sBuffIcon* p = (sBuffIcon*)(*i);
		delete p;
	}
	mIconList.Clear();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
cBuffWindowSkin::cBuffWindowSkin( eUINodeType type )
: cUIWindowSkin(type)
{
	mRowSpace = 0;
	mColumnSpace = 0;

	mRow = 0;
	mColumn = 0;

	mIconSize = 0;
	mScale = 1.0f;
}

cBuffWindowSkin::~cBuffWindowSkin()
{
}

bool cBuffWindowSkin::Load( cParser& parser )
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
		case eTOKEN_ICON_SIZE:
			{
				mIconSize = parser.ParseInt();
			}
			break;
		case eTOKEN_BUFF_SPACE:
			{
				mColumnSpace = parser.ParseInt();
				mRowSpace = parser.ParseInt();
			}
			break;
		case eTOKEN_BUFF_ROW:
			{
				mRow = parser.ParseInt();
			}
			break;
		case eTOKEN_BUFF_COLUMN:
			{
				mColumn = parser.ParseInt();
			}
			break;
		case eTOKEN_BUFF_SCALE:
			{
				mScale = (float)parser.ParseInt()/100.0f;
			}
			break;
		case eTOKEN_HOLDVIEW:
			{
				if( parser.ParseString().Compare( "true" ) == 0 )
					mHoldView = true;
				else
					mHoldView = false;
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
