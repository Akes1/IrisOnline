#include "StdAfx.h"
#include "UIFuncText.h"

#include "UINode.h"
#include "Parser.h"

cUIFuncText::cUIFuncText()
{
}

cUIFuncText::~cUIFuncText()
{
	Clear();
}

void cUIFuncText::Clear()
{
	cNodeMap::cIterator i = mNodeMap.Begin();
	cNodeMap::cIterator iend = mNodeMap.End();

	for( ; i != iend; ++i )
	{
		cUIFuncTextNode* n = (cUIFuncTextNode*)i->mSecond;
		delete n;
	}

	mNodeMap.Clear();
}

bool cUIFuncText::Load( const cStringW& pathName )
{
	Clear();

	/// 파일 열기
	cFileLoaderW loader;
	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// 렉서에 밥주고 파싱
	cTokenW token;
	cUIFuncTextLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParserW parser( &lexer, pathName );
	cStringW text;
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
			PushColorNode( id, COLOR_BLACK );
			break;

		case eTOKEN_TEXT_WHITE:
			PushColorNode( id, COLOR_DEFAULT );
			break;

		case eTOKEN_TEXT_GRAY:
			PushColorNode( id, COLOR_GRAY );
			break;

		case eTOKEN_TEXT_RED:
			PushColorNode( id, COLOR_RED );
			break;

		case eTOKEN_TEXT_GREEN:
			PushColorNode( id, COLOR_GREEN );
			break;

		case eTOKEN_TEXT_BLUE:
			PushColorNode( id, COLOR_SKYBLUE );
			break;

		case eTOKEN_TEXT_YELLOW:
			PushColorNode( id, COLOR_YELLOW );
			break;

		case eTOKEN_TEXT_BROWN:
			PushColorNode( id, COLOR_BROWN );
			break;

		case eTOKEN_TEXT_VARNAME:
			PushVarNameNode( id );
			break;

		case eTOKEN_TEXT_NEWLINE:
			PushNewLineNode( id );
			break;

		default:
			{
				char buf[256] = {0,};
				::sprintf_s( buf, "error %s file", pathName.Cstr() );
				NiMessageBox( buf, "error");
			}
			return false;
		}
	}
	return true;
}

cUIFuncTextNode* cUIFuncText::NewNode( unsigned int id )
{
	cUIFuncTextNode* n = 0;
	cNodeMap::cIterator i = mNodeMap.Find( id );

	if( i == mNodeMap.End() )
	{
		n = new cUIFuncTextNode;
		mNodeMap.Insert( id, n );
	}
	else
	{
		n = ((cUIFuncTextNode*)i->mSecond)->Back()->mpNext = new cUIFuncTextNode;
	}
	return n;
}

void cUIFuncText::PushStringNode( unsigned int id, const cStringW& str )
{
	if( str.IsEmpty() )
		return;

	cUIFuncTextNode* n = NewNode( id );

	n->mType = eTEXTNODE_STRING;

	/*wchar_t buffer[256] = {0,};
	int len = ConvertToUnicode( str.Cstr(), buffer, 256 );
	assert( len > 0 );*/

	int len = str.GetLength();
	wchar_t* dest = n->mpString = new wchar_t[len+1];
	::_tcsncpy_s( dest, len+1, str.Cstr(), len );
	dest[len] = 0;
}

void cUIFuncText::PushColorNode( unsigned int id, unsigned long color )
{
	cUIFuncTextNode* n = NewNode( id );

	n->mType = eTEXTNODE_COLOR;
	n->mColor = color;
}

void cUIFuncText::PushVarNameNode( unsigned int id )
{
	cUIFuncTextNode* n = NewNode( id );

	n->mType = eTEXTNODE_VARNAME;
	n->mColor = 0;
}

void cUIFuncText::PushNewLineNode( unsigned int id )
{
	cUIFuncTextNode* n = NewNode( id );
	n->mType = eTEXTNODE_NEWLINE;
	n->mColor = 0;
}

cUIFuncTextNode* cUIFuncText::GetNode( unsigned int id )
{
	cNodeMap::cConstIterator i = mNodeMap.Find( id );

	if( i == mNodeMap.End() )
	{
		assert( 0 && "failed to find npc text node" );
		return 0;
	}
	else
		return (cUIFuncTextNode*)i->mSecond;
}

cUIFuncTextLexer::cUIFuncTextLexer( const wchar_t* buffer, unsigned int size )
: cLexerW( buffer, size )
{
	BindKeyword( L"#black", eTOKEN_TEXT_BLACK );
	BindKeyword( L"#white", eTOKEN_TEXT_WHITE );
	BindKeyword( L"#gray", eTOKEN_TEXT_GRAY );
	BindKeyword( L"#red", eTOKEN_TEXT_RED );
	BindKeyword( L"#green", eTOKEN_TEXT_GREEN );
	BindKeyword( L"#blue", eTOKEN_TEXT_BLUE );
	BindKeyword( L"#yellow", eTOKEN_TEXT_YELLOW );
	BindKeyword( L"#brown", eTOKEN_TEXT_BROWN );
	BindKeyword( L"#name", eTOKEN_TEXT_VARNAME );
	BindKeyword( L"#enter", eTOKEN_TEXT_NEWLINE );
}
