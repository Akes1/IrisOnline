#include "StdAfx.h"
#include "UIProperty.h"
#include "UISkinLexer.h"
#include "Parser.h"
#include "UINodeProperty.h"

#include "FileSystem.h"
#include "MemFile.h"

//#include "ServerWindow.h"

cUIProperty::cUIProperty()
{
}

cUIProperty::~cUIProperty()
{
	Clear();
}

void cUIProperty::Clear()
{
	cNodePropertyMap::cIterator i = mNodePropertyMap.Begin();
	cNodePropertyMap::cIterator end = mNodePropertyMap.End();

	for( ; i != end; ++i )
	{
		delete (cUINodeProperty*)(*i).mSecond;
	}

	mNodePropertyMap.Clear();
}

bool cUIProperty::Load( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	cString path;
	::GetFilePath( &path, pathName );

	cToken token;
	cUISkinLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		/// 목록 파일을 읽는다. "ex) uiPropertyList.txt"
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_PROPERTYFILES:
			if( LoadScripts( path, parser ) == false )
			{
				return false;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			cString msg;
			msg.Format("[%s] [Line:%d]", pathName.Cstr(), token.mLine );
			MessageBoxA( NULL, msg.Cstr(), "invalid token", MB_OK | MB_ICONERROR );
			return false;
		}
	}
	return true;
}

/// 주의사항 : 
bool cUIProperty::LoadScripts( const cString& path, cParser& parser )
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
			break;
		}

		if( token.mType == eTOKEN_STR )
		{
			/// path 를 붙여서 밑에서 새로운 파일 로딩하도록 함.
			cString pathName( path );
			pathName += token;

			///// 각속성을 각 노드들이 파싱하게함 
			if( LoadScript( pathName ) == false )
			{
				assert( 0 && "failed to load script file" );
				cString msg;
				msg.Format("[%s] [Line:%d]", pathName.Cstr(), token.mLine );
				MessageBoxA( NULL, msg.Cstr(), "failed to load script file", MB_OK | MB_ICONERROR );
				return false;
			}
		}
		else
		{
			assert( 0 && "invalid keyword" );
			cString msg;
			msg.Format("[%s] [Line:%d]", path.Cstr(), token.mLine );
			MessageBoxA( NULL, msg.Cstr(), "invalid keyword", MB_OK | MB_ICONERROR );
			return false;
		}
	}

	/// 각 노드 속성들이 유효한지 검사
	cNodePropertyMap::cIterator i = mNodePropertyMap.Begin();
	cNodePropertyMap::cIterator end = mNodePropertyMap.End();

	for( ; i != end; ++i )
	{
		if( ((cUINodeProperty*)(*i).mSecond)->IsValid() == false )
		{
			assert( 0 && "found invalid node property" );
			return false;
		}
	}

	return true;
}

/// 주의사항 : 각 속성들을 각 노드들이 파싱하도록 함 
/// 참고!!!!!! 여기서 메모리 해제시 릭이 나는경우는 똑같은 이름의
///				프로퍼티가 생성되었기 때문이다. 
///				GetNodeProperty 을 해서 이미 존재하는 이름이 있으면 
///				두개이상 존재한다는 뜻임!!
bool cUIProperty::LoadScript( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	cToken token;
	cUISkinLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		cString name;
		if( parser.ParseString( &name ) == false )
		{
			return false;
		}

		eUINodeType type = eUINODE_NULL;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
			/// window 와 동일한 tabsheet 키워드를 따로 둔 이유는
			/// 사용자가 스크립트에서 tabsheet 를 구별하기 쉽게하기위해서임.
		case eTOKEN_PROPERTY_TABSHEET:
			{
				type = eUINODE_TABSHEET;
			}
			break;
		case eTOKEN_PROPERTY_WINDOW:
			{
				/// ui 윈도우 속성
				type = eUINODE_WINDOW;
			}
			break;
		case eTOKEN_PROPERTY_CHECKBOX:
			{
				/// 체크박스 속성
				type = eUINODE_CHECKBOX;
			}
			break;
		case eTOKEN_PROPERTY_LABEL:
			{
				/// 라벨 속성
				type = eUINODE_LABEL;
			}
			break;
		case eTOKEN_PROPERTY_COMBOBOX:
			{
				/// 콤보박스 속성
				type = eUINODE_COMBOBOX;
			}
			break;
		case eTOKEN_PROPERTY_PUSHBUTTON:
			{
				/// 푸쉬 버튼 속성
				type = eUINODE_PUSHBUTTON;
			}
			break;
		case eTOKEN_PROPERTY_BUTTON:
			{
				/// 버튼 속성
				type = eUINODE_BUTTON;
			}
			break;
		case eTOKEN_PROPERTY_TABBUTTON:
			{
				/// 탭버튼 속성
				type = eUINODE_TABBUTTON;
			}
			break;
		case eTOKEN_PROPERTY_MULTIEDITBOX:
			{
				/// 멀티 에디트박스 속성
				type = eUINODE_MULTIEDITBOX;
			}
			break;
		case eTOKEN_PROPERTY_EDITBOX:
			{
				/// 에디트박스 속성
				type = eUINODE_EDITBOX;
			}
			break;
		case eTOKEN_PROPERTY_LISTBOX:
			{
				/// 리스트박스 속성
				type = eUINODE_LISTBOX;
			}
			break;
		case eTOKEN_PROPERTY_SCROLLBAR:
			{
				/// 스크롤바 속성
				type = eUINODE_SCROLLBAR;
			}
			break;
		case eTOKEN_PROPERTY_BARBUTTON:
			{
				/// 바 속성
				type = eUINODE_BARBUTTON;
			}
			break;
		case eTOKEN_PROPERTY_SPINBOX:
			{
				/// 스핀박스 속성
				type = eUINODE_SPINBOX;
			}
			break;
		case eTOKEN_PROPERTY_GAUGE:
			{
				/// 게이지바 속성
				type = eUINODE_GAUGE;
			}
			break;
		case eTOKEN_PROPERTY_GAUGETB:
			{
				type = eUINODE_GAUGETB;
			}
			break;
		case eTOKEN_PROPERTY_DOUBLEGAUGE:
			{
				type = eUINODE_DOUBLEGAUGE;
			}
			break;
		case eTOKEN_PROPERTY_TEXTBOX:
			{
				/// 텍스트박스 속성
				type = eUINODE_TEXTBOX;
			}
			break;
		case eTOKEN_PROPERTY_TABWINDOW:
			{
				/// 탭윈도우 속성
				type = eUINODE_TABWINDOW;
			}
			break;
		case eTOKEN_PROPERTY_NUMBEREDIT:
			{
				/// 에디트박스 속성
				type = eUINODE_NUMBEREDIT;
			}
			break;
		case eTOKEN_PROPERTY_RADIOBUTTON:
			{
				/// 라디오버튼 속성 
				type = eUINODE_RADIOBUTTON;
			}
			break;
		case eTOKEN_PROPERTY_OPTIONGAUGE:
			{
				/// 옵션게이지 속성 
				type = eUINODE_OPTIONGAUGE;
			}
			break;
		case eTOKEN_PROPERTY_LISTFOLDERBOX:
			{
				/// 폴더 리스트박스 속성
				type = eUINODE_LISTFOLDERBOX;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			cString msg;
			msg.Format("[%s] [Line:%d]", pathName.Cstr(), token.mLine );
			MessageBoxA( NULL, msg.Cstr(), "invalid token", MB_OK | MB_ICONERROR );
			return false;
		}

		/// 생성
		cUINodeProperty* p = new cUINodeProperty( name, type );

		/// 로딩
		if( p->Load( parser ) == false )
		{
			delete p;
			return false;
		}

		/// 노드 속성 맵에 추가
		if( mNodePropertyMap.Insert( name, p ) == false )
		{
			assert( 0 && "failed to insert ui node property, maybe already exist" );
			cString msg;
			msg.Format("[%s] [Line:%d] [prop:%s]", pathName.Cstr(), token.mLine, name.Cstr() );
			MessageBoxA( NULL, msg.Cstr(), "failed to insert ui node property, maybe already exist", MB_OK | MB_ICONERROR );
			return false;
		}
	}
	return true;
}

/// 주의사항 : 
cUINodeProperty* cUIProperty::GetNodeProperty( const cString& name ) const
{
	cNodePropertyMap::cConstIterator i = mNodePropertyMap.Find( name );
	
	if( i == mNodePropertyMap.End() )	
	{
		//assert( 0 && "failed to find ui node property" );
		return 0;
	}
	return (cUINodeProperty*)(i->mSecond);
}
