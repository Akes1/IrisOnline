#include "StdAfx.h"
#include "UIItemText.h"

#include "UINode.h"
#include "Lexer.h"
#include "Parser.h"

cUIItemText::cUIItemText()
: mNodeMap( 4096 )
{
}

cUIItemText::~cUIItemText()
{
	Clear();
}

void cUIItemText::Clear()
{
	cNodeMap::cIterator i = mNodeMap.Begin();
	cNodeMap::cIterator iend = mNodeMap.End();

	for( ; i != iend; ++i )
	{
		cUIItemTextNode* n = (cUIItemTextNode*)i->mSecond;
		delete n;
	}

	mNodeMap.Clear();
}

bool cUIItemText::Load( const cString& pathName )
{
	Clear();

	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cUIItemTextLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );
	cString text;
	unsigned int id = 0;

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;

		case eTOKEN_NULL:
			continue;

		case eTOKEN_INT:
			id = token.ToInt();
			break;

		case eTOKEN_STR:
			PushStringNode( id, token );
			break;

		case eTOKEN_TEXT_BLACK:
			PushColorNode( id, eCOLOR_BLACK );
			break;

		case eTOKEN_TEXT_WHITE:
			PushColorNode( id, eCOLOR_WHITE );
			break;

		case eTOKEN_TEXT_GRAY:
			PushColorNode( id, eCOLOR_GRAY );
			break;

		case eTOKEN_TEXT_RED:
			PushColorNode( id, eCOLOR_RED );
			break;

		case eTOKEN_TEXT_GREEN:
			PushColorNode( id, eCOLOR_GREEN );
			break;

		case eTOKEN_TEXT_BLUE:
			PushColorNode( id, eCOLOR_BLUE );
			break;

		case eTOKEN_TEXT_YELLOW:
			PushColorNode( id, eCOLOR_YELLOW );
			break;

		case eTOKEN_TEXT_VARNAME:
			PushVarNameNode( id );
			break;

		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

cUIItemTextNode* cUIItemText::NewNode( unsigned int id )
{
	cUIItemTextNode* n = 0;
	cNodeMap::cIterator i = mNodeMap.Find( id );

	if( i == mNodeMap.End() )
	{
		n = new cUIItemTextNode;
		mNodeMap.Insert( id, n );
	}
	else
	{
		n = ((cUIItemTextNode*)i->mSecond)->Back()->mpNext = new cUIItemTextNode;
	}
	return n;
}

void cUIItemText::PushStringNode( unsigned int id, const cString& str )
{
	cUIItemTextNode* n = NewNode( id );

	n->mType = eITEM_TEXTNODE_STRING;

#ifdef _UNICODE
	wchar_t buffer[256] = {0,};
	int len = ConvertToUnicode( str.Cstr(), buffer, 256 );
	assert( len > 0 );

	wchar_t* dest = n->mpString = new wchar_t[len+1];
	::_tcsncpy( dest, buffer, len );
	dest[len] = 0;
#else
	char* dest = n->mpString = new char[len+1];
	::strncpy( dest, buffer, len );
	dest[len] = 0;
#endif
}

void cUIItemText::PushColorNode( unsigned int id, unsigned long color )
{
	cUIItemTextNode* n = NewNode( id );

	n->mType = eITEM_TEXTNODE_COLOR;
	n->mColor = color;
}

void cUIItemText::PushVarNameNode( unsigned int id )
{
	cUIItemTextNode* n = NewNode( id );

	n->mType = eITEM_TEXTNODE_VARNAME;
	n->mColor = 0;
}

const cUIItemTextNode* cUIItemText::GetNode( unsigned int id ) const
{
	cNodeMap::cConstIterator i = mNodeMap.Find( id );

	if( i == mNodeMap.End() )
	{
		assert( 0 && "failed to find npc text node" );
		return 0;
	}
	else
		return (cUIItemTextNode*)i->mSecond;
}

cUIItemTextLexer::cUIItemTextLexer( const char* buffer, unsigned int size )
: cLexer( buffer, size )
{
	BindKeyword( "black", eTOKEN_TEXT_BLACK );
	BindKeyword( "white", eTOKEN_TEXT_WHITE );
	BindKeyword( "gray", eTOKEN_TEXT_GRAY );
	BindKeyword( "red", eTOKEN_TEXT_RED );
	BindKeyword( "green", eTOKEN_TEXT_GREEN );
	BindKeyword( "blue", eTOKEN_TEXT_BLUE );
	BindKeyword( "yellow", eTOKEN_TEXT_YELLOW );
	BindKeyword( "name", eTOKEN_TEXT_VARNAME );
}
