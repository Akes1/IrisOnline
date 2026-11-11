#include "StdAfx.h"
#include "UIIDList.h"

#include "Lexer.h"
#include "Parser.h"
#include "FileSystem.h"
#include "MemFile.h"

cUIIDList::cUIIDList()
{
}

cUIIDList::~cUIIDList()
{
}

bool cUIIDList::Load( const cString& pathName )
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// 스킨 파일의 경로 추출
	cString path;
	::GetFilePath( &path, pathName );

	/// 렉서에 밥주고 파싱
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	cString propName;
	unsigned int propID = 0;

	/// 그다음토큰을 계속 검사
	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );
	
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_STR:
			{
				/// 프로퍼티 이름과 아이디값 받아오기 
				propName = token;
				propID = parser.ParseInt();
			}
			break;
		default:
			assert( 0 && "invalid token" );
			cString msg;
			msg.Format("[%s] [Line:%d]", pathName.Cstr(), token.mLine );
			MessageBoxA( NULL, msg.Cstr(), "invalid token", MB_OK | MB_ICONERROR );
			return false;
		}

		/// 노드 스킨 맵에 추가
		if( mIDHashMap.Insert( propName, propID ) == false )
		{
			assert( 0 && "failed to insert prop name, maybe already exist" );
			cString msg;
			msg.Format("[%s] [Line:%d] [prop:%s]", pathName.Cstr(), token.mLine, propName );
			MessageBoxA( NULL, msg.Cstr(), "failed to insert prop name, maybe already exist", MB_OK | MB_ICONERROR );
			return false;
		}
	}
	return true;
}

/// 주의사항 : 아이디 리턴 
unsigned int cUIIDList::GetPropID( const cString& propname ) const
{
	cIDHashMap::cConstIterator i = mIDHashMap.Find( propname );

	if( i == mIDHashMap.End() )
	{
		return 0;
	}
	return i->mSecond;
}
