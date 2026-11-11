#include "stdafx.h"
#include "gameResourceManager.h"
#include "ResourceManager.h"
#include "Player_Common.h"
#include "SkillScript.h"

#include "UIText.h"

#include "GameFileLexer.h"
#include "Parser.h"
#include "FileSystem.h"

#include "Application.h"

#define HAIRCOLOR_KEY(hair,color)	(unsigned int)(((unsigned char)(hair)|((unsigned short)((unsigned char)(color))<<8)))

cGameResourceManager* cGameResourceManager::mpGameResourceManager = NULL;

cGameResourceManager::cGameResourceManager()
{
	mpGameResourceManager = this;

	mpSkillScript = 0;

	mpGameText = 0;
	mpSkillNameText = 0;
	mpItemNameText = 0;
	mpNpcNameText = 0;
	mpMonsterNameText = 0;

	mHairColorMax = 0;

	mBaseFaceCount = 0;

}

cGameResourceManager::~cGameResourceManager()
{
	SAFE_DELETE(mpMonsterNameText);
	SAFE_DELETE(mpNpcNameText);
	SAFE_DELETE(mpItemNameText);
	SAFE_DELETE(mpSkillNameText);
	SAFE_DELETE(mpGameText);
}

/* ------------------------------------------------------------------
 * 함수이름 :	Init()
 * 목    적 :	Application의 Init()에서 실행된다.
 *				프로그램 실행시 한번만 로딩된다.
 * 주의사항 :	꼭 로딩해야 하는것들만 넣을것.. (Select창과 Make창에서 사용할 것들)
 * ------------------------------------------------------------------ */
bool cGameResourceManager::Init()
{
	//////////////////////////////////////////////////////////////////////////
	/// 스크립트 파일을 로딩한다.

	/// Game용 텍스트 파일
	if( !LoadGameText() )
	{
		NiMessageBox( "Lang폴더 업그레이드 체크!!", "Error" );
		return false;
	}

	/// Model정보
	if( !LoadModelFileInfo() )
	{
		NiMessageBox( "ModelFileList.txt 체크하세요!!", "Error" );
		return false;
	}

	if( !LoadCharacterBaseHairFileInfo() )
	{
		NiMessageBox( "CharacterHairFile.txt 체크하세요!!", "Error" );
		return false;
	}

	if( !LoadCharacterBaseFileInfo() )
	{
		NiMessageBox( "CharacterBaseFile.txt 체크하세요!!", "Error" );
		return false;
	}

	if( !LoadFaceEmotionInfo() )
	{
		NiMessageBox( "EmotionList.txt 체크하세요!!", "Error" );
		return false;
	}

	///
	if( !LoadItemInfo() )
	{
		NiMessageBox( "Item_Define.txt 체크하세요!!", "Error" );
		return false;
	}

	if( !LoadItemOnePieceInfo() )
	{
		NiMessageBox( "Item_OnePiece.txt 체크하세요!!", "Error" );
		return false;
	}

	if( !LoadItemFileInfo() )
	{
		NiMessageBox( "Item_FileName.txt 체크하세요!!", "Error" );
		return false;
	}

	if( !LoadMonsterInfo() )
	{
		NiMessageBox( "MonsterList.txt 체크하세요!!", "Error" );
		return false;
	}

	if( !LoadNpcInfo() )
	{
		NiMessageBox( "NPCList.txt 체크하세요!!", "Error" );
		return false;
	}

	mpSkillScript = new cSkillScript;
	if( !mpSkillScript->Init() )
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	/// 사용할 그래픽 데이터들을 로딩한다.

	/// 1. 기본 인터페이스용 이미지 로딩..

	/// 2. 기본 사운드 파일 로딩..

	/// 3. kfm파일 로딩..
	cArrayHashMap::cIterator i = mModelFileNameMap.Begin();
	cArrayHashMap::cIterator end = mModelFileNameMap.End();
	cString* pModelFile;

	for( ; i != end; ++i )
	{
		unsigned long idx = (unsigned long)(*i).mFirst;
		pModelFile = (cString*)(*i).mSecond;

		if( pModelFile )
		{
			cString pathName;
			if( idx <= 100 )
				pathName.Format( "Data/Character/%s", pModelFile->Cstr() );
			else if( idx <= 1000 )
				pathName.Format( "Data/Npc/%s", pModelFile->Cstr() );
			else
				pathName.Format( "Data/Monster/%s", pModelFile->Cstr() );

			RESOURCEMAN->LoadKFM( pathName );
		}
	}

	unsigned int characterCount = eGENDER_MAX * eRACE_MAX;
	cFileNameArray* pArray = 0;
	cArrayHashMap::cIterator iter;
	cArrayHashMap::cIterator iterEnd;
	cString* name = NULL;

	/// default hair loading
	iter = mDefaultHairFileNameMap.Begin();
	iterEnd = mDefaultHairFileNameMap.End();
	for( ; iter != iterEnd; ++iter )
	{
		pArray = (cFileNameArray*)(*iter).mSecond;
		if( pArray )
		{
			for( unsigned int cnt=0; cnt<characterCount; ++cnt )
			{
				name = (cString*) (*pArray)[cnt];
				if( name && name->GetLength() )
				{
					cString pathName;
					pathName.Format( "Data/Character/%s", name->Cstr() );

					RESOURCEMAN->LoadNIF( pathName );
				}
			}
		}
	}


	/// default face loading
	iter = mDefaultFaceFileNameMap.Begin();
	iterEnd = mDefaultFaceFileNameMap.End();
	for( ; iter != iterEnd; ++iter )
	{
		pArray = (cFileNameArray*)(*iter).mSecond;
		if( pArray )
		{
			for( unsigned int cnt=0; cnt<characterCount; ++cnt )
			{
				name = (cString*) (*pArray)[cnt];
				if( name && name->GetLength() )
				{
					cString pathName;
					pathName.Format( "Data/Character/%s", name->Cstr() );

					RESOURCEMAN->LoadNIF( pathName );
				}
			}
		}
	}

	/// default body, hand, foot loading
	iter = mDefaultWearFileNameMap.Begin();
	iterEnd = mDefaultWearFileNameMap.End();
	for( ; iter != iterEnd; ++iter )
	{
		pArray = (cFileNameArray*)(*iter).mSecond;
		if( pArray )
		{
			for( unsigned int cnt=0; cnt<characterCount; ++cnt )
			{
				name = (cString*) (*pArray)[cnt];
				if( name && name->GetLength() )
				{
					cString pathName;
					pathName.Format( "Data/Character/%s", name->Cstr() );

					RESOURCEMAN->LoadNIF( pathName );
				}
			}
		}
	}
	//
	//////////////////////////////////////////////////////////////////////////

	return true;
}

/* ------------------------------------------------------------------
 * 함수이름 :	Exit()
 * 목    적 :	종료부
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cGameResourceManager::Exit()
{
	if( mpSkillScript )
	{
		mpSkillScript->Release();

		delete mpSkillScript;
		mpSkillScript = 0;
	}

	RemoveAllCharacterBaseFileInfo();
	RemoveAllModelFileName();

	/// 정의
	{
		cArrayHashMap::cIterator i = mItemInfo.Begin();
		cArrayHashMap::cIterator iend = mItemInfo.End();

		for( ; i != iend; ++i )
		{
			delete (sItemInfo*)(i->mSecond);
		}
		mItemInfo.Clear();
	}

	/// 파일명
	{
		cArrayHashMap::cIterator i = mItemFileInfo.Begin();
		cArrayHashMap::cIterator iend = mItemFileInfo.End();

		for( ; i != iend; ++i )
		{
			delete [] ((cString*)(i->mSecond));
		}
		mItemFileInfo.Clear();
	}

	{
		cArrayHashMap::cIterator i = mItemOnePieceInfo.Begin();
		cArrayHashMap::cIterator iend = mItemOnePieceInfo.End();

		for( ; i != iend; ++i )
		{
			delete (sItemOnepieceInfo*)(i->mSecond);
		}
		mItemOnePieceInfo.Clear();
	}


	{
		cArrayHashMap::cIterator i = mFaceEmotionMap.Begin();
		cArrayHashMap::cIterator end = mFaceEmotionMap.End();
		for( ; i != end; ++i )
		{
			sFaceEmoteInfo* p = (sFaceEmoteInfo*)(*i).mSecond;
			SAFE_DELETE(p);
		}
		mFaceEmotionMap.Clear();
	}

	{
		cArrayHashMap::cIterator i = mMonsterInfo.Begin();
		cArrayHashMap::cIterator end = mMonsterInfo.End();
		for( ; i != end; ++i )
		{
			sMonsterInfo* pInfo = (sMonsterInfo*)((*i).mSecond);
			SAFE_DELETE(pInfo);
		}
		mMonsterInfo.Clear();
	}

	{
		cArrayHashMap::cIterator i = mNpceInfo.Begin();
		cArrayHashMap::cIterator end = mNpceInfo.End();
		for( ; i != end; ++i )
		{
			sNpcInfo* pInfo = (sNpcInfo*)((*i).mSecond);
			SAFE_DELETE(pInfo);

		}
		mNpceInfo.Clear();
	}
}


/* ------------------------------------------------------------------
 * 함수이름 :	GetModelFileName( unsigned long modelIndex )
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
const char* cGameResourceManager::GetModelFileName( unsigned long modelIndex )
{
	cString* str = (cString*)mModelFileNameMap.GetAt( modelIndex );
	if( !str ) return 0;

	return (*str).Cstr();
}

/* ------------------------------------------------------------------
 * 함수이름 :	LoadModleFileInfo()
 * 목    적 :	모델 정보파일들을 로드한다.
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cGameResourceManager::LoadModelFileInfo()
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Script/Resource/ModelFileList.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Resource/ModelFileList.txt" );

	unsigned long fileIdx;
	cString* name = NULL;

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
			{
				name = new cString;

				fileIdx = token.ToInt();
				*name = parser.ParseString();

				mModelFileNameMap.Insert( fileIdx, name );
				name = NULL;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}

/* ------------------------------------------------------------------
 * 함수이름 :	GetDefaultHairFileName( unsigned int hairIdx, char race, char gender )
 * 목    적 :	기본 머리 파일명 제공
 * 주의사항 :	
 * ------------------------------------------------------------------ */
const char* cGameResourceManager::GetDefaultHairFileName( unsigned int hairIdx, char race, char gender )
{
	assert(race < eRACE_MAX);
	assert(gender < eGENDER_MAX);
	if( race < eRACE_MAX || gender < eGENDER_MAX )
	{
		cFileNameArray* pArray = (cFileNameArray*)mDefaultHairFileNameMap.GetAt( hairIdx );
		if( pArray ) 
		{
			cString* name = (cString*)(*pArray)[(race*eGENDER_MAX) + gender];
			if( name )
				return name->Cstr();
		}
	}
	return NULL;
}

const char* cGameResourceManager::GetHairColorTextureFileName( unsigned int hairIdx, unsigned int colorIdx, char race, char gender )
{
	if( colorIdx >= mHairColorMax )
		return NULL;

	assert(race < eRACE_MAX);
	assert(gender < eGENDER_MAX);
	if( race < eRACE_MAX || gender < eGENDER_MAX )
	{
		unsigned int key = HAIRCOLOR_KEY(hairIdx,colorIdx);

		cFileNameArray* pArray = (cFileNameArray*)mHairColorTextureMap.GetAt( key );
		if( pArray ) 
		{
			cString* name = (cString*)(*pArray)[(race*eGENDER_MAX) + gender];
			if( name )
				return name->Cstr();
		}
	}

	return NULL;
}

/* ------------------------------------------------------------------
 * 함수이름 :	GetDefaultFaceFileName( unsigned int faceIdx, char race, char gender )
 * 목    적 :	기본 얼굴 파일명 제공
 * 주의사항 :	
 * ------------------------------------------------------------------ */
const char* cGameResourceManager::GetDefaultFaceFileName( unsigned int faceIdx, char race, char gender )
{
	assert(race < eRACE_MAX);
	assert(gender < eGENDER_MAX);
	if( race < eRACE_MAX || gender < eGENDER_MAX )
	{
		cFileNameArray* pArray = (cFileNameArray*)mDefaultFaceFileNameMap.GetAt( faceIdx );
		if( pArray ) 
		{
			cString* name = (cString*)(*pArray)[(race*eGENDER_MAX) + gender];
			if( name )
				return name->Cstr();
		}
	}
	return NULL;
}

const char* cGameResourceManager::GetFaceTextureFileName( unsigned long idx, char race, char gender )
{
	assert(race < eRACE_MAX);
	assert(gender < eGENDER_MAX);
	if( race < eRACE_MAX || gender < eGENDER_MAX )
	{
		sFaceEmoteInfo* info = (sFaceEmoteInfo*)mFaceEmotionMap.GetAt(idx);
		if( info )
		{
			if( info->textureName[race*eGENDER_MAX+gender].Compare("None") == 0 )
				return NULL;

			return info->textureName[race*eGENDER_MAX+gender].Cstr();
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------
* 함수이름 :	GetDefaultWearFileName( unsigned int part, char race, char gender )
* 목    적 :	각 복장의 기본 그래픽 파일 이름을보를 제공한다.
* 주의사항 :	
* ------------------------------------------------------------------ */
const char*	cGameResourceManager::GetDefaultWearFileName( unsigned int part, char race, char gender )
{
	assert(race < eRACE_MAX);
	assert(gender < eGENDER_MAX);
	if( race < eRACE_MAX || gender < eGENDER_MAX )
	{
		cFileNameArray* pArray = (cFileNameArray*)mDefaultWearFileNameMap.GetAt( part );
		if( pArray ) 
		{
			cString* name = (cString*)(*pArray)[(race*eGENDER_MAX) + gender];
			if( name )
				return name->Cstr();
		}
	}
	return NULL;
}

/* ------------------------------------------------------------------
 * 함수이름 :	LoadCharacterBaseFileInfo()
 * 목    적 :	캐릭터 기본 객체파일 로드한다.
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cGameResourceManager::LoadCharacterBaseFileInfo()
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Script/Resource/CharacterBaseFile.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Resource/CharacterBaseFile.txt" );

	cFileNameArray* pArray = 0;
	unsigned int index = 0;
	char type = 0;
	cString* fileName = 0;

	unsigned int count = eRACE_MAX * eGENDER_MAX;
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
			{
				pArray = new cFileNameArray;
				pArray->Reserve( count );

				// 인덱스 	부위 파일명(count 개)
				index = token.ToInt();
				type = (char)parser.ParseInt();
				for( unsigned int i=0; i<count; ++i)
				{
					fileName = new cString;
					*fileName = parser.ParseString();
					pArray->PushBack( fileName );
				}

				switch( type )
				{
				case ePART_FACE:
					{
						mDefaultFaceFileNameMap.Insert( index, pArray );
					}
					break;
				case ePART_BODY1:
				case ePART_BODY2:
				case ePART_HAND:
				case ePART_FOOT:
					mDefaultWearFileNameMap.Insert( type, pArray );
				}
				pArray = NULL;
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}

bool cGameResourceManager::LoadFaceEmotionInfo()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/EmotionList.txt";
	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MapData.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	unsigned int count = eRACE_MAX * eGENDER_MAX;
	sFaceEmoteInfo* pInfo = 0;
	unsigned long filterKey = 0;
	while( tokenizer.IsEnd() == false )
	{
		pInfo = new sFaceEmoteInfo;

		/// 
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->index = str.ToInt();

		/// type
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->type = (unsigned char)str.ToInt();

		/// texture file name
		for( unsigned int i=0; i<count; i++ )
		{
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->textureName[i] = str;
		}

		/// filter text index
		for( unsigned int i=0; i<10; i++ )
		{
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			filterKey = str.ToInt();
		}

		/// 해쉬에 몬스터 별로 기록
		if( mFaceEmotionMap.Insert( pInfo->index, pInfo ) == false )
		{
			assert(0);
		}

		if( pInfo->type == 0 )
			mBaseFaceCount++;
	}

	return true;

ERR:
	delete pInfo;
	return false;
}

bool cGameResourceManager::LoadCharacterBaseHairFileInfo()
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Script/Resource/CharacterHairFile.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cGameFileLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Resource/CharacterHairFile.txt" );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_HAIR:
			{
				if( LoadHair(parser) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_HAIRCOLOR:
			{
				mHairColorMax = parser.ParseInt();
				if( LoadHairColor(parser) == false )
				{
					assert(0);
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

bool cGameResourceManager::LoadHair( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	cFileNameArray* pArray = 0;
	unsigned int index = 0;
	cString* fileName = 0;

	unsigned int count = eRACE_MAX * eGENDER_MAX;
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				pArray = new cFileNameArray;
				pArray->Reserve( count );

				// 인덱스 	부위 파일명(count 개)
				index = token.ToInt();
				for( unsigned int i=0; i<count; ++i)
				{
					fileName = new cString;
					*fileName = parser.ParseString();
					pArray->PushBack( fileName );
				}

				mDefaultHairFileNameMap.Insert( index, pArray );
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}

bool cGameResourceManager::LoadHairColor( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	cFileNameArray* pArray = 0;
	unsigned int hairIdx = 0;
	unsigned int colorIdx = 0;
	cString* fileName = 0;

	unsigned int count = eRACE_MAX * eGENDER_MAX;
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}
		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				pArray = new cFileNameArray;
				pArray->Reserve( count );

				// 키 생성
				hairIdx = token.ToInt();
				colorIdx = parser.ParseInt();
				unsigned int key = HAIRCOLOR_KEY(hairIdx,colorIdx);

				for( unsigned int i=0; i<count; ++i)
				{
					fileName = new cString;
					*fileName = parser.ParseString();
					pArray->PushBack( fileName );
				}

				mHairColorTextureMap.Insert( key, pArray );
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}

	return true;
}

/* ------------------------------------------------------------------
 * 함수이름 :	RemoveAllModleFileName()
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cGameResourceManager::RemoveAllModelFileName()
{
	cArrayHashMap::cIterator i = mModelFileNameMap.Begin();
	cArrayHashMap::cIterator end = mModelFileNameMap.End();
	cString* pModelFile;

	for( ; i != end; ++i )
	{
		pModelFile = (cString*)(*i).mSecond;
		delete pModelFile;
	}
	mModelFileNameMap.Clear();
}

/* ------------------------------------------------------------------
 * 함수이름 :	RemoveAllCharacterBaseFileInfo()
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cGameResourceManager::RemoveAllCharacterBaseFileInfo()
{
	cFileNameArray* pArray = 0;
	cString* name = 0;

	/// 기본 복장 정보 제거
	cArrayHashMap::cIterator i = mDefaultWearFileNameMap.Begin();
	cArrayHashMap::cIterator end = mDefaultWearFileNameMap.End();
	for( ; i != end; ++i )
	{
		pArray = (cFileNameArray*)(*i).mSecond;
		if( pArray )
		{
			for( unsigned int cnt=0; cnt< pArray->GetSize(); ++cnt )
			{
				name = (cString*)(*pArray)[cnt];
				SAFE_DELETE(name);
			}
			SAFE_DELETE(pArray);
		}
	}
	mDefaultWearFileNameMap.Clear();

	/// 기본 얼굴 정보 제거
	i = mDefaultFaceFileNameMap.Begin();
	end = mDefaultFaceFileNameMap.End();
	for( ; i != end; ++i )
	{
		pArray = (cFileNameArray*)(*i).mSecond;
		if( pArray )
		{
			for( unsigned int cnt=0; cnt< pArray->GetSize(); ++cnt )
			{
				name = (cString*)(*pArray)[cnt];
				SAFE_DELETE(name);
			}
			SAFE_DELETE(pArray);
		}
	}
	mDefaultFaceFileNameMap.Clear();

	/// 기본 헤어 텍스쳐 정보 제거
	i = mHairColorTextureMap.Begin();
	end = mHairColorTextureMap.End();
	for( ; i != end; ++i )
	{
		pArray = (cFileNameArray*)(*i).mSecond;
		if( pArray )
		{
			for( unsigned int cnt=0; cnt< pArray->GetSize(); ++cnt )
			{
				name = (cString*)(*pArray)[cnt];
				SAFE_DELETE(name);
			}
			SAFE_DELETE(pArray);
		}
	}
	mHairColorTextureMap.Clear();

	/// 기본 헤어 정보 제거
	i = mDefaultHairFileNameMap.Begin();
	end = mDefaultHairFileNameMap.End();
	for( ; i != end; ++i )
	{
		pArray = (cFileNameArray*)(*i).mSecond;
		if( pArray )
		{
			for( unsigned int cnt=0; cnt< pArray->GetSize(); ++cnt )
			{
				name = (cString*)(*pArray)[cnt];
				SAFE_DELETE(name);
			}
			SAFE_DELETE(pArray);
		}
	}
	mDefaultHairFileNameMap.Clear();
}

bool cGameResourceManager::LoadItemInfo()
{
	cFileLoader loader;

	if( loader.Open( "./Script/Resource/Item_Define.txt", true ) == false )
	{
		assert( 0 && "failed to load item define" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", "./Script/Resource/Item_Define.txt" );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long index = (long)str.ToInt();
		assert( index > 0 );

		/// 이름
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 이름 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int nameIndex = (unsigned int)str.ToInt();

		/// 툴팁 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		unsigned int tipIndex = (unsigned int)str.ToInt();

		/// 종류
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned char type = (unsigned char)str.ToInt();

		/// 세부 종류
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned char subType = (unsigned char)str.ToInt();

		/// 검색 대분류
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 검색 중분류
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 검색 소분류
		if( tokenizer.GetNext( &str ) == false )
			return false;

//skip

		/// 무게
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		short weight = (short)str.ToInt();

		/// 겹침갯수
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		short capacity = (short)str.ToInt();

		/// 판매방법
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		unsigned long sellType = (unsigned long)str.ToInt();

		/// 판매가격
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		unsigned long sellPrice = (unsigned long)str.ToInt();

		/// 구입방법
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		unsigned long buyType = (unsigned long)str.ToInt();

		/// 구입가격
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		unsigned long buyPrice = (unsigned long)str.ToInt();

		/// 최대 출현개수
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		unsigned short presentNum = (unsigned short)str.ToInt();

		/// 보유 가능갯수
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		unsigned long maxHaveInven = (unsigned long)str.ToInt();

		// 기간제 설정
		if( tokenizer.GetNext( &str ) == false )
			return false;

		// 사용기한
		if( tokenizer.GetNext( &str ) == false )
			return false;

		// 인증여부
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 사용자거래 여부(0거래가능/1거래불가)
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 능력 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long abilityIndex = (long)str.ToInt();

		/// 제한 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long limitIndex = (long)str.ToInt();

		/// 카드 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		//long cardIndex = (long)str.ToInt();

		/// 타로 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long tarotIndex = (long)str.ToInt();

		/// 스프레드 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long spreadIndex = (long)str.ToInt();

		/// 분해 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long disjointIndex = (long)str.ToInt();

		/// 카드슬롯 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long cardSlotIndex = (long)str.ToInt();

		/// 강화확률 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long enhanceRateIndex = (long)str.ToInt();

		/// 강화 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long enhanceIndex = (long)str.ToInt();

		/// 체인지 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 호칭 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		long titleIndex = (long)str.ToInt();

		/// 세트 인덱스
		if( tokenizer.GetNext( &str) == false )
			return false;

//		long setIndex = (long)str.ToInt();

		/// 아이콘 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		unsigned int iconIndex = (unsigned int)str.ToInt();

		/// 모델 인덱스0
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int fileIndex0 = (unsigned int)str.ToInt();

		/// 모델 인덱스1
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int fileIndex1 = (unsigned int)str.ToInt();

		/// 투구 착용시 귀 표시여부 (0:귀표시, 1:귀표시 안함)
		if( tokenizer.GetNext( &str ) == false )
			return false;

		char hideEar = (char)str.ToInt();

		/// 아이템 툴팁컬러 ( 레어도관련 )
		if( tokenizer.GetNext( &str ) == false )
			return false;

//		unsigned short tooltipColor = (unsigned short)str.ToInt();

		// 홈페이지 공개여부
		if( tokenizer.GetNext( &str ) == false )
			return false;

		sItemInfo* p = new sItemInfo;
		p->mIndex = index;
		p->mNameIndex = nameIndex;
		p->mType = type;
		p->mSubType = subType;
		p->mModelIndex1 = fileIndex0;
		p->mModelIndex2 = fileIndex1;
		p->mHideEar = hideEar;

		if( mItemInfo.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert item define, maybe already exist" );
			return false;
		}
	}
	return true;
}

bool cGameResourceManager::LoadItemOnePieceInfo()
{
	cFileLoader loader;

	if( loader.Open( "./Script/Resource/Item_OnePiece.txt", true ) == false )
	{
		assert( 0 && "failed to load item onepiece" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), "\t\r\n", "./Script/Resource/Item_OnePiece.txt" );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long index = (long)str.ToInt();
		assert( index > 0 );

		if( tokenizer.GetNext( &str ) == false )
			return false;

		long upperIndex = (long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		long lowerIndex = (long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		long handsIndex = (long)str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		long footsIndex = (long)str.ToInt();

		sItemOnepieceInfo* p = new sItemOnepieceInfo;
		p->mIndex = index;
		p->mUpperIndex = upperIndex;
		p->mLowerIndex = lowerIndex;
		p->mHandsIndex = handsIndex;
		p->mFootsIndex = footsIndex;

		if( mItemOnePieceInfo.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert item onpiece info, maybe already exist" );
			return false;
		}
	}
	return true;
}

bool cGameResourceManager::LoadItemFileInfo()
{
	cFileLoader loader;

	if( loader.Open( "./Script/Resource/Item_FileName.txt", true ) == false )
	{
		assert( 0 && "failed to load item file name" );
		return false;
	}

	unsigned int iend = eRACE_MAX * eGENDER_MAX;

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", "./Script/Resource/Item_FileName.txt" );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		long index = str.ToInt();
		assert( index > 0 );

		iend = eRACE_MAX * eGENDER_MAX;

		/// 아이템 파일 이름 배열을 생성
		cString* p = new cString[iend];

		for( unsigned int i = 0; i < iend; ++i )
		{
			if( tokenizer.GetNext( &(p[i]) ) == false )
			{
				delete [] p;
				return false;
			}
		}

		if( mItemFileInfo.Insert( index, p ) == false )
		{
			assert( 0 && "failed to insert item file names, maybe already exist" );
			return false;
		}
	}
	return true;
}

bool cGameResourceManager::LoadMonsterInfo()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/MonsterList.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MonsterList.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sMonsterInfo* info = NULL;
	while( tokenizer.IsEnd() == false )
	{
		info = new sMonsterInfo;

		//몬스터 종류 index
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		info->mIndex = str.ToInt();

		/// 몬스터 이름Idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		info->mNameIndex = str.ToInt();

		char name[32];
		/// 몬스터 이름
		::sprintf_s( name, "%s", GAMERESOURCEMAN->GetMonsterName( info->mNameIndex ) ); 
		info->mName = name;

		/// 몬스터 직업이름
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// kfm파일idx
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		info->mModelIndex = str.ToInt();

		/// 이름 높이값
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 분류
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 속성
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// HP
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// MP
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// exp획득량
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// sxp획득량
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 물리방어
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 마법방어
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 명중수치
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 회피율
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 시야범위
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 비중
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 크기
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		info->mScale = str.ToFloat();

		/// 연출이펙트 크기
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		info->mEffectScale = str.ToFloat();

		/// 피해범위 - 보간 사이즈
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 행동범위
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 행동(정지)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 행동(걷기)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 행동(뛰기)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 정지시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 이동속도(걷기)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 이동속도(뛰기,도망)
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 선공여부
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 변신인식
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 추적시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 추적거리
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 도망조건
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 도망확률
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 회복여부
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 회복시작시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 해쉬에 몬스터 별로 기록
		if( mMonsterInfo.Insert( info->mIndex, info ) == false )
		{
			assert(0);
			return false;
		}
	}

	return true;

ERR:
	delete info;
	return false;
}

bool cGameResourceManager::LoadNpcInfo()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/NPCList.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load NPCList.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// NPC 번호
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long npcClassIdx = str.ToInt(); 

		/// NPC 이름
		if( tokenizer.GetNext( &str ) == false )	{ return false; }

		unsigned long nameIdx = str.ToInt();

		char name[32];
		::sprintf_s( name, "%s", GAMERESOURCEMAN->GetNpcName( nameIdx ) ); 

		// type
		if( tokenizer.GetNext( &str ) == false )	{ return false; }

		/// NPC 직업 인덱스
		if( tokenizer.GetNext( &str ) == false )	{ return false; }

//		char jobName[32];
//		_stprintf( jobName, _T( "%s" ), GAMERESOURCEMAN->GetNpcName( str.ToInt() ) ); 

		/// NPC 애니메이션 파일 번호(KFM)]
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned int modelIndex = str.ToInt(); 

		if( tokenizer.GetNext( &str ) == false )	{ return false; }

		if( tokenizer.GetNext( &str ) == false )	{ return false; }

		if( tokenizer.GetNext( &str ) == false )	{ return false; }

		sNpcInfo* info = new sNpcInfo;

		info->mIndex = npcClassIdx;
		char str[32];
		::sprintf_s( str, "%s", name );
		info->mName = str;
		info->mNameIndex = nameIdx;
		info->mModelIndex = modelIndex;

		/// 해쉬에 NPC 별로 기록
		if( mNpceInfo.Insert( info->mIndex, info ) == false )
		{
			assert(0);
			delete info;
			return false;
		}
	}

	return true;
}

///
bool cGameResourceManager::LoadGameText()
{
	///
	mpGameText = new cUIText;
	if( !mpGameText )
	{
		assert( 0 && "mpGameText not create!" );
		return false;
	}

	cString path;
	path.Format("./language/%s/SystemMsg_c.txt", cApplication::mLangaugeFolder.Cstr() );
	if( mpGameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load GameText" );
		return false;
	}

	/// 스킬명
	mpSkillNameText = new cUIText;
	if( !mpSkillNameText )
	{
		assert( 0 && "mpSkillNameText not create!" );
		return false;
	}

	path.Format("./language/%s/Skill_Name_c.txt", cApplication::mLangaugeFolder.Cstr() );
	if( mpSkillNameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load GameText" );
		return false;
	}

	/// item이름
	mpItemNameText = new cUIText;
	if( !mpItemNameText )
	{
		assert( 0 && "mpItemNameText not create!" );
		return false;
	}

	path.Format("./language/%s/Item_Name_c.txt", cApplication::mLangaugeFolder.Cstr() );
	if( mpItemNameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load GameText" );
		return false;
	}

	/// NPC 이름
	mpNpcNameText = new cUIText;
	if( !mpNpcNameText )
	{
		assert( 0 && "mpNpcNameText not create!" );
		return false;
	}

	path.Format("./language/%s/NPC_Name_c.txt", cApplication::mLangaugeFolder.Cstr() );
	if( mpNpcNameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load NpcName" );
		return false;
	}

	/// 몬스터 이름
	mpMonsterNameText = new cUIText;
	if( !mpMonsterNameText )
	{
		assert( 0 && "mpMonsterNameText not create!" );
		return false;
	}

	path.Format("./language/%s/Mon_Name_c.txt", cApplication::mLangaugeFolder.Cstr() );
	if( mpMonsterNameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load MonsterName" );
		return false;
	}

	return true;
}

LPCTSTR cGameResourceManager::GetGameText( unsigned int textIdx )
{
	assert(mpGameText);
	return mpGameText->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetSkillNameText( unsigned int textIdx )
{
	assert(mpSkillNameText);
	return mpSkillNameText->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetNpcName( unsigned int nameIdx )
{
	assert( mpNpcNameText );
	return mpNpcNameText->GetText( nameIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetMonsterName( unsigned int nameIdx )
{
	assert( mpMonsterNameText );
	return mpMonsterNameText->GetText( nameIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetItemNameText( unsigned int nameIdx )
{
	assert(mpItemNameText);
	return mpItemNameText->GetText( nameIdx ).Cstr();
}

const char* cGameResourceManager::GetItemFileName( unsigned long itemFileIdx, char race, char gender, bool weapon )
{
	if( itemFileIdx == 0 )
		return 0;

	cArrayHashMap::cConstIterator i = mItemFileInfo.Find( itemFileIdx );

	if( i == mItemFileInfo.End() )
		return 0;

	cString* p = (cString*)i->mSecond;
	if( p[race * eGENDER_MAX + gender] == "None" )
	{
		if( weapon )
			return p[0].Cstr();

		return 0;
	}

	return p[race * eGENDER_MAX + gender].Cstr();
}

sItemOnepieceInfo* cGameResourceManager::GetItemOnepieceInfo( unsigned long itemIndex )
{
	if( itemIndex == 0 )
		return 0;

	return (sItemOnepieceInfo*)mItemOnePieceInfo.GetAt( itemIndex );
}


sMonsterInfo* cGameResourceManager::GetMonsterInfo( unsigned long monIdx )
{
	return (sMonsterInfo*)mMonsterInfo.GetAt( monIdx );
}
