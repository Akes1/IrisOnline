#include "StdAfx.h"
#include "UIManager.h"

#include "UINode.h"
#include "UIFuncText.h"
#include "UIText.h"
#include "UIIDList.h"
#include "UIImageFont.h"
#include "BaseIcon.h"
#include "UINpcTelling.h"
#include "Page.h"
#include "Application.h"
#include "./client/ResourceManager.h"

cUIManager* cUIManager::mpSingleton = 0;

cUIManager::cUIManager()
: mMousePos(0, 0)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

// 	for( unsigned int i = 0; i < eStage_Max; ++i )
// 	{
// 		mpStageRootNode[i] = new cUIRootNode;
// 	}

	mpRootNode = 0;
	mpSkinText = 0;
	mpIDList = 0;
	mpSkin = 0;
	mpProperty = 0;
	mpCaptureNode = 0;
	mpHoveredNode = 0;
	mpFocusedNode = 0;
	mpTopMsgBox = 0;
	mTipWindow = 0;
	mpDragWindow = 0;
	mpUnionDragWindow = 0;
	mChatItemTipWindow = 0;
// 	mNoticePrintText = 0;
// 	mEnhancePrintText = 0;
//	mpModeImage = 0;

	/// Npc
	mpNpcText = 0;
	
//  	mNoticePrintText = new cCenterNoticeText;
//  	mEnhancePrintText = new cCenterNoticeText;
}

cUIManager::~cUIManager()
{
//	mpModeImage = 0;

// 	SAFE_DELETE( mEnhancePrintText );
// 	SAFE_DELETE( mNoticePrintText );
	SAFE_DELETE( mpNpcText );
	SAFE_DELETE( mpSkin );
	SAFE_DELETE( mpSkinText );
	SAFE_DELETE( mpProperty );
	SAFE_DELETE( mpIDList );

	// 
	{
		cImageHashMap::cIterator i = mImageHashMap.Begin();
		cImageHashMap::cIterator end = mImageHashMap.End();
		for( ; i != end; ++i )
		{
			cImagePair& data = (i->mSecond);

// 			cPlaneObject* p	= (cPlaneObject*)data.mFirst;
// 			SAFE_DELETE( p );
		}
		mImageHashMap.Clear();
	}

//	mImageIndexGenerator.RestoreAll();

	/// 이미지 폰트
	{
		cImageFontMap::cIterator i = mImageFontMap.Begin();
		cImageFontMap::cIterator iend = mImageFontMap.End();

		for( ; i != iend; ++i )
		{
			delete (cUIImageFont*)(i->mSecond);
		}
	}

	/// 아이콘
	{
		cIconParamMap::cIterator i = mIconParamMap.Begin();
		cIconParamMap::cIterator iend = mIconParamMap.End();

		for( ; i != iend; ++i )
		{
			delete (cIconParam*)(i->mSecond);
		}
	}

	/// NPC
	{
		cNpcTellingMap::cIterator i = mNpcTellingMap.Begin();
		cNpcTellingMap::cIterator iend = mNpcTellingMap.End();

		for( ; i != iend; ++i )
		{
			delete (cUINpcTelling*)(i->mSecond);
		}
	}

	/// page
	{
		cPageMap::cIterator i = mPageMap.Begin();
		cPageMap::cIterator end = mPageMap.End();

		for( ; i != end; ++i )
		{
			delete (cPage*)(i->mSecond);
		}
		mPageMap.Clear();
	}

	/// scene
	{
		cSceneMap::cIterator i = mSceneMap.Begin();
		cSceneMap::cIterator end = mSceneMap.End();

		for( ; i != end; ++i )
		{
			delete (sSceneData*)(i->mSecond);
		}
		mSceneMap.Clear();
	}

// 	for( unsigned int i = 0; i < eStage_Max; ++i )
// 	{
// 		SAFE_DELETE( mpStageRootNode[i] );
// 	}

	/// 메세지 박스
	mModalList.Clear();
	mModallessList.Clear();

	///
	mOtherClickList.Clear();

	mpTopMsgBox = 0;
	mpSingleton = 0;

	mManagedSortUIList.Clear();
}

bool cUIManager::Init()
{
	mpIDList = new cUIIDList;
	if( !mpIDList )
	{
		assert(0 && "mpIDList not create!" );
		return false;
	}

	mpProperty = new cUIProperty;
	if( !mpProperty )
	{
		assert(0 && "mpProperty not create!" );
		return false;
	}

	mpSkinText = new cUIText;
	if( !mpSkinText )
	{
		assert(0 && "mpSkinText not create!" );
		return false;
	}

	mpSkin = new cUISkin;
	if( !mpSkin )
	{
		assert(0 && "mpskin not create!" );
		return false;
	}

	if( LoadIDList( "./Script/Interface/List_ID.txt" ) == false )
	{
		assert(0 && "LoadIDList fail" );
		return false;
	}

	if( LoadProperty( "./Script/Interface/List_Property.txt" ) == false )
	{
		assert(0 && "LoadProperty fail" );
		return false;
	}

	cString path;
	path.Format("./language/%s/InterfaceMsg.txt", cApplication::mLangaugeFolder.Cstr() );
	if( LoadSkinText( path.Cstr() ) == false )
	{
		assert(0 && "LoadSkinText fail" );
		return false;
	}

	if( LoadSkin( "./Script/Interface/List_SkinImage.txt" ) == false )
	{
		assert(0 && "LoadSkin fail" );
		return false;
	}

	if( LoadIconParams( "./Script/Interface/Path_Icon.txt" ) == false )
	{
		assert(0 && "LoadIconParams fail" );
		return false;
	}

	if( LoadImageFonts( "./Script/Interface/Path_Font.txt" ) == false )
	{
		assert( 0 && "failed to load ui image fonts" );
		return false;
	}

	//
	mpNpcText = new cUIFuncText;

	path.Format("./language/%s/NPC_Note.txt", cApplication::mLangaugeFolder.Cstr() );
	if( LoadNpcText( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load LoadNpcText" );
		return false;
	}
	if( LoadNpcScript( "./Script/Resource/NPC_Func.txt" ) == false )
	{
		assert( 0 && "failed to load LoadNpcScript" );
		return false;
	}
	if( LoadNpcScene( "./Script/Resource/NPC_Scene.txt" ) == false )
	{
		assert( 0 && "failed to load LoadNpcScene" );
		return false;
	}
	return true;
}

bool cUIManager::LoadIDList( const cString& pathName )
{
	/// 스킨 로딩 
	if( mpIDList->Load( pathName ) == false )
	{
		assert( 0 && "failed to load idlist" );
		return false;
	}
	return true;
}

/// 주의사항 : 
bool cUIManager::LoadSkin( const cString& pathName )
{
	/// 스킨 로딩 
	if( mpSkin->Load( pathName ) == false )
	{
		assert( 0 && "failed to load ui skin" );
		return false;
	}
	return true;
}

bool cUIManager::LoadProperty( const cString& pathName )
{
	/// 속성 로딩
	if( mpProperty->Load( pathName ) == false )
	{
		assert( 0 && "failed to load ui property" );
		return false;
	}
	return true;
}

/// 주의사항 : 
bool cUIManager::LoadSkinText( const cString& pathName )
{
	if( mpSkinText->Load( pathName ) == false )
	{
		assert( 0 && "failed to load skin text" );
		return false;
	}
	return true;
}

bool cUIManager::LoadImageFonts( const cString& pathName )
{
	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load image fonts" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int index = str.ToInt();

		/// 이미지 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int imageIndex = str.ToInt();

		/// 텍스처 시작 위치
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int texX = str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int texY = str.ToInt();

		/// 텍스처 폰트 문자 크기
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int charWidth = str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int charHeight = str.ToInt();

		/// 이미지 폰트를 생성
 		NiTexture* tex = UIMAN->GetSkin()->GetTexture( imageIndex );
 		if( tex == 0 )
 			continue;

		cUIImageFont* imageFont = new cUIImageFont( tex, texX, texY, charWidth, charHeight );

		if( mImageFontMap.Insert( index, imageFont ) == false )
		{
			assert( 0 && "failed to insert image font, maybe already exist" );
			cString msg;
			msg.Format("[%s] [Index:%u]", pathName.Cstr(), index );
			MessageBoxA( NULL, msg.Cstr(), "failed to insert image font, maybe already exist", MB_OK | MB_ICONERROR );
			return false;
		}
	}
	return true;
}

bool cUIManager::LoadIconParams( const cString& pathName )
{
	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load icon params" );
		MessageBoxA( NULL, pathName.Cstr(), "failed to load icon params", MB_OK | MB_ICONERROR );
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );

	/// 아이콘 이미지를 로딩
	if( parser.ExpectTokenString( "images" ) == false )
		return false;
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		if( token.mType == eTOKEN_RCURLY )
			break;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				/// 텍스처 로딩
				unsigned int id = token.ToInt();
				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_STR );


				cString pathName;
				if( id >= LANGUAGE_ICON_INDEX )
				{
					pathName.Format( "./Language/%s/%s", cApplication::mLangaugeFolder.Cstr(), token.Cstr() );
				}
				else
					pathName.Format( "./Data/2DData/%s", token.Cstr() );

 				NiTexture* p = RESOURCEMAN->LoadTexture( pathName.Cstr(), false );

				if( p == 0 )
				{
					assert( 0 && "failed to load texture" );
					return false;
				}

				/// 텍스처 맵에 추가
				mIconTextureMap.Insert( id, p );

				unsigned int width, height;
				width = p->GetWidth();
				height = p->GetHeight();
				width = 0;
				height = 0;
			}
			break;
		default:
			cString msg;
			msg.Format("[%s] [Line:%d]", pathName.Cstr(), token.mLine );
			MessageBoxA( NULL, msg.Cstr(), "invalid token", MB_OK | MB_ICONERROR );
			assert( 0 && "invalid token" );
			return false;
		}
	}

	/// 아이콘 인자 리스트를 파싱
	if( parser.ExpectTokenString( "iconParams" ) == false )
		return false;
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		if( token.mType == eTOKEN_RCURLY )
			break;

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				unsigned int iconIndex = token.ToInt();

				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_INT );
				unsigned int imageIndex = token.ToInt();

				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_INT );
				unsigned int texx = token.ToInt();

				lexer.GetNextToken( &token );
				assert( token.mType == eTOKEN_INT );
				unsigned int texy = token.ToInt();

				/// 아이콘 인자 맵에 추가
				cTextureMap::cIterator i = mIconTextureMap.Find( imageIndex );
				if( i == mIconTextureMap.End() )
				{
					assert( 0 && "failed to find icon texture by id" );
					cString msg;
					msg.Format("[%s] [Line:%d] [imageIndex:%u]", pathName.Cstr(), token.mLine, imageIndex );
					MessageBoxA( NULL, msg.Cstr(), "failed to find icon texture by id", MB_OK | MB_ICONERROR );
					return false;
				}

				cIconParam* param = new cIconParam;
				param->mpTexture = (NiTexture*)i->mSecond;
				//test
				assert( param->mpTexture );
				//
				param->mTexPos.mX = texx;
				param->mTexPos.mY = texy;

				if( mIconParamMap.Insert( iconIndex, param ) == false )
				{
					assert( 0 && "failed to insert icon param, icon id maybe already exist" );
					cString msg;
					msg.Format("[%s] [Line:%d] [iconIndex:%u]", pathName.Cstr(), token.mLine, iconIndex );
					MessageBoxA( NULL, msg.Cstr(), "failed to insert icon param, icon id maybe already exist", MB_OK | MB_ICONERROR );
					return false;
				}
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

LPCTSTR cUIManager::GetUIText( unsigned int textId )
{
	assert( mpSkinText );
	return mpSkinText->GetText( textId ).Cstr();
}

bool cUIManager::GetUIText( cStringT* str, unsigned int idx )
{
	assert( mpSkinText );
	return mpSkinText->GetText( str, idx );
}

bool cUIManager::AddPage( unsigned long pageIdx, cPage* page )
{
	if( !page )
		return false;

	if( mPageMap.Insert( pageIdx, page ) == false )
	{
		assert( 0 && "failed to insert page" );
		return false;
	}

	return true;
}

cPage* cUIManager::GetPage( unsigned long pageIdx )
{
	return (cPage*)mPageMap.GetAt( pageIdx );
}

cIconParam* cUIManager::GetIconParam( unsigned int iconId )
{
	cIconParamMap::cIterator i = mIconParamMap.Find( iconId );

	if( i == mIconParamMap.End() )
		return 0;
	else
		return (cIconParam*)(i->mSecond);
}
