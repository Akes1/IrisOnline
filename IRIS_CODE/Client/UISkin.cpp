#include "StdAfx.h"
#include "UISkin.h"
#include "UISkinLexer.h"
#include "Parser.h"
///
#include "Button.h"
#include "CheckBox.h"
#include "ComboBox.h"
#include "EditBox.h"
#include "Gauge.h"
#include "Icon.h"
#include "Label.h"
#include "ListBox.h"
#include "MsgBox.h"
#include "OptionGauge.h"
#include "RadioButton.h"
#include "ScrollBar.h"
#include "SpinBox.h"
#include "UIWindow.h"
#include "TextBox.h"
#include "TabWindow.h"
#include "ListFolderBox.h"
#include "ResourceManager.h"
#include "TargaImage.h"
#include "BitArray.h"
#include "Application.h"

cUISkin::cUISkin()
{
}

cUISkin::~cUISkin()
{
	Clear();
}

/// 주의사항 : 
void cUISkin::Clear()
{
	/// 텍스처 맵을 지움
	mTextureMap.Clear();

	/// 비트 배열 맵을 지움
	{
		cBitArrayMap::cIterator i = mAlphaBitArrayMap.Begin();
		cBitArrayMap::cIterator iend = mAlphaBitArrayMap.End();

		for( ; i != iend; ++i )
		{
			delete (cBitArray*)i->mSecond;
		}

		mAlphaBitArrayMap.Clear();
	}

	/// 노드 스킨 맵을 지움
	{
		cNodeSkinMap::cIterator i = mNodeSkinMap.Begin();
		cNodeSkinMap::cIterator iend = mNodeSkinMap.End();

		for( ; i != iend; ++i )
		{
			delete (cUINodeSkin*)i->mSecond;
		}

		mNodeSkinMap.Clear();
	}
}

/// 주의사항 : 
bool cUISkin::Load( const cString& pathName )
{
	/// 지우기
	Clear();

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
	cUISkinLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_IMAGES:
			{
				/// 이미지 파일들을 로딩
				if( LoadImages( path, parser ) == false )
				{
					assert( 0 && "fail to load imagefile" );
					return false;
				}
			}
			break;
		case eTOKEN_SKINFILES:
			{
				/// 스크립트 파일들을 로딩
				if( LoadScripts( path, parser ) == false )
				{
					assert( 0 && "fail to load script" );
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

/// 주의사항 : 
bool cUISkin::LoadImages( const cString& , cParser& parser )
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

		/// 텍스처 로딩
		unsigned int id = token.ToInt();
		parser.ParseString( &token );

		cString pathName;
		pathName.Format( "./language/%s/%s", cApplication::mLangaugeFolder.Cstr(), token.Cstr() );
		NiTexture* p = RESOURCEMAN->LoadTexture( pathName, false );

		if( p == 0 )
		{
			assert( 0 && "failed to load texture" );
			return false;
		}

		/// 텍스처 맵에 추가
		mTextureMap.Insert( id, p );

		/// 텍스처에 알파가 있는 경우 알파 비트 배열을 생성 및 로딩
		cTargaImage tgaImage;
		if( tgaImage.Load( pathName ) == true && tgaImage.GetBytesPerPixel() == 4 )
		{
			cBitArray* bitArray = new cBitArray( tgaImage.GetWidth() * tgaImage.GetHeight() );
			bitArray->mWidth = tgaImage.GetWidth();
			bitArray->mHeight = tgaImage.GetHeight();
			unsigned char alpha = 0;

			for( int y = tgaImage.GetHeight()-1, i = 0; y >= 0; --y )
			{
				for( int x = 0, xend = tgaImage.GetWidth(); x < xend; ++x, ++i )
				{
					tgaImage.GetPixel( 0, 0, 0, &alpha, (unsigned int)x, (unsigned int)y );
					bitArray->Set( i, alpha > 0 );
				}
			}

			mAlphaBitArrayMap.Insert( id, bitArray );
		}
	}
	return true;
}

/// 주의사항 : 
bool cUISkin::LoadScripts( const cString& path, cParser& parser )
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
		case eTOKEN_STR:
			{
				cString pathName( path );
				pathName += token;

				if( LoadScript( pathName ) == false )
				{
					assert( 0 && "failed to load skin script" );
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	
	/// 각 노드 스킨에 대해 적합성 검사
	cNodeSkinMap::cIterator i = mNodeSkinMap.Begin();
	cNodeSkinMap::cIterator end = mNodeSkinMap.End();

	for( ; i != end; ++i )
	{
		if( ((cUINodeSkin*)(*i).mSecond)->IsValid() == false )
		{
			assert( 0 && "found invalid node skin" );
			return false;
		}
	}
	return true;
}


/// 주의사항 : 
/// 참고!!!!!! 여기서 메모리 해제시 릭이 나는경우는 똑같은 이름의
///				스킨이 생성되었기 때문이다. 
///				GetNodeSkin 을 해서 이미 존재하는 이름이 있으면 
///				두개이상 존재한다는 뜻임!!
bool cUISkin::LoadScript( const cString& pathName )
{
	/// 파일 로딩
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cUISkinLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		cString name;
		if( parser.ParseString( &name ) == false )
			return false;

		cUINodeSkin* p = 0;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_TABSHEET:
		case eTOKEN_WINDOW:
			{
				/// 윈도우 스킨 생성
				p = cUIWindowSkin::Create( name.Cstr() );
			}
			break;
		case eTOKEN_BARBUTTON:
			{
				p = new cButtonSkin( eUINODE_BARBUTTON );
			}
			break;
		case eTOKEN_PUSHBUTTON:
			{
				p = new cButtonSkin( eUINODE_PUSHBUTTON );
			}
			break;
		case eTOKEN_BUTTON:
			{
				p = new cButtonSkin( eUINODE_BUTTON );
			}
			break;
		case eTOKEN_TABBUTTON:
			{
				/// 버튼 스킨 생성
				p = new cButtonSkin( eUINODE_TABBUTTON );
			}
			break;
		case eTOKEN_CHECKBOX:
			{
				/// 체크 박스 스킨 생성
				p = new cCheckBoxSkin;
			}
			break;
		case eTOKEN_LABEL:
			{
				/// 라벨 스킨 생성
				p = new cLabelSkin;
			}
			break;
		case eTOKEN_MULTIEDITBOX:
			{
				p = new cEditBoxSkin( eUINODE_MULTIEDITBOX );
			}
			break;
		case eTOKEN_EDITBOX:
			{
				/// 에디트 스킨 생성
				p = new cEditBoxSkin;
			}
			break;
		case eTOKEN_LISTBOX:
			{
				/// 리스트 스킨 생성
				p = new cListBoxSkin;
			}
			break;
		case eTOKEN_LISTFOLDERBOX:
			{
				/// 리스트 스킨 생성
				p = new cListFolderBoxSkin;
			}
			break;
		case eTOKEN_SCROLLBAR:
			{
				/// 스크롤바 스킨 생성
				p = new cScrollBarSkin;
			}
			break;
		case eTOKEN_COMBOBOX:
			{
				/// 콤보박스 스킨 생성
				p = new cComboBoxSkin;
			}
			break;
		case eTOKEN_GAUGE:
			{
				p = new cGaugeSkin;
			}
			break;
		case eTOKEN_GAUGETB:
			{
				p = new cGaugeSkin( eUINODE_GAUGETB );
			}
			break;
		case eTOKEN_DOUBLEGAUGE:
			{
				/// 게이지바 스킨 생성
				p = new cGaugeSkin( eUINODE_DOUBLEGAUGE );
			}
			break;
		case eTOKEN_TEXTBOX:
			{
				/// 텍스트박스 스킨 생성
				p = new cTextBoxSkin;
			}
			break;
		case eTOKEN_TABWINDOW:
			{
				/// 탭윈도우 스킨 생성
				p = new cTabWindowSkin;
			}
			break;
		case eTOKEN_SPINBOX:
			{
				/// 스핀컨트롤 스킨 생성
				p = new cSpinBoxSkin;
			}
			break;
		case eTOKEN_MESSAGEBOX:
			{
				/// 메세지 박스 스킨 생성
				p = new cMessageBoxSkin;
			}
			break;
		case eTOKEN_ICON:
			{
				/// 아이콘 스킨 생성
				p = new cIconSkin;
			}
			break;
		case eTOKEN_RADIOBUTTON:
			{
				/// 라디오 버튼 스킨 생성 
				p = new cRadioButtonSkin;
			}	
			break;
		case eTOKEN_OPTIONGAUGE:
			{	
				/// 옵션 게이지 스킨 생성 
				p = new cOptionGaugeSkin;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}

		/// 로딩
		if( p->Load( parser ) == false )
		{
			delete p;
			return false;
		}

		/// 노드 스킨 맵에 추가
		if( mNodeSkinMap.Insert( name, p ) == false )
		{
			assert( 0 && "failed to insert ui node skin, maybe already exist" );
			return false;
		}
	}
	return true;
}

///
bool cUISkin::Reload( const cString& pathName )
{
	/// 텍스처 맵을 지움
	mTextureMap.Clear();

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
	cUISkinLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_IMAGES:
			{
				/// 이미지 파일들을 로딩
				if( LoadImages( path, parser ) == false )
				{
					assert( 0 && "fail to load imagefile" );
					return false;
				}
			}
			break;
		case eTOKEN_SKINFILES:
			{
				/// 스크립트 파일들을 로딩
				if( ReloadScripts( path, parser ) == false )
				{
					assert( 0 && "fail to load script" );
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

///
bool cUISkin::ReloadScripts( const cString& path, cParser& parser )
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
			break;

		switch( token.mType )
		{
		case eTOKEN_STR:
			{
				cString pathName( path );
				pathName += token;

				if( ReloadScript( pathName ) == false )
				{
					assert( 0 && "failed to Reload skin script" );
					return false;
				}
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	/// 각 노드 스킨에 대해 적합성 검사
	cNodeSkinMap::cIterator i = mNodeSkinMap.Begin();
	cNodeSkinMap::cIterator end = mNodeSkinMap.End();

	for( ; i != end; ++i )
	{
		if( ((cUINodeSkin*)(*i).mSecond)->IsValid() == false )
		{
			assert( 0 && "found invalid node skin" );
			return false;
		}
	}
	return true;
}

/// 스크립트 리로드 기능 ( 스킨의 데이터만 바뀐다 )
bool cUISkin::ReloadScript( const cString& pathName )
{
	/// 파일 로딩
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cUISkinLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		cString name;
		if( parser.ParseString( &name ) == false )
			return false;

		if( token.mType == eTOKEN_ERROR )
			return false;

		if( token.mType == eTOKEN_NULL )
			continue;

		cUINodeSkin* p = GetNodeSkin( name );
		if( p == 0 )
		{
			assert( 0 );
			return false;
		}

		/// 로딩
		if( p->Load( parser ) == false )
		{
			assert( 0 && "failed to update skin script" );
			return false;
		}
	}
	return true;
}

