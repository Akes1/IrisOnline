#include "StdAfx.h"
#include "ComboBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIImage.h"
#include "UIContainer.h"
//#include "FontAgent.h"

#include "ListBox.h"
#include "Button.h"
#include "EditBox.h"

cComboBox::cComboBox( eUINodeType type )
: cUIWindow( type )
, mpListBox(0)
, mpButton(0)
, mpEdit(0)
{
	mSize.mWidth = 0;
	mSize.mHeight = 0;
}

cComboBox::~cComboBox()
{
	mpEdit = 0;
	mpButton = 0;
	mpListBox = 0;
}

/////////////////////////////////////////////////////////////////////////////////
cComboBoxSkin::cComboBoxSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cComboBoxSkin::~cComboBoxSkin()
{

}

/// 주의사항 : 
bool cComboBoxSkin::Load( cParser& parser )
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
		case eTOKEN_EDITSKIN:
			{
				mEditSkin = parser.ParseString();
			}
			break;
		case eTOKEN_LISTSKIN:
			{
				mListboxSkin = parser.ParseString();
			}
			break;
		case eTOKEN_BUTTONSKIN:
			{
				mButtonSkin = parser.ParseString();
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

