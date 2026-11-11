#include "StdAfx.h"
#include "TabWindow.h"
#include "Token.h"
#include "UISkinLexer.h"
#include "Parser.h"

#include "UIImage.h"
#include "UIContainer.h"
#include "UINodeProperty.h"

cTabWindow::cTabWindow( eUINodeType type )
: cUIWindow( type )
, mCurrentButton( 0 )
, mCurrentSheet( 0 )
{
}

cTabWindow::~cTabWindow()
{
}


///////////////////////////////////////////////////////////////////////////
cTabWindowSkin::cTabWindowSkin( eUINodeType type )
: cUIWindowSkin( type )
, mPressed( false )
{

}

cTabWindowSkin::~cTabWindowSkin()
{

}
