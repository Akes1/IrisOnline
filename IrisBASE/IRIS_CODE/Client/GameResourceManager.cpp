#include "stdafx.h"
#include "gameResourceManager.h"
#include "ResourceManager.h"
#include "Monster_Common.h"

#include "UIText.h"
#include "UIFuncText.h"

#include "Application.h"

#include "StatusCalc_Client.h"
#include "StatusScript.h"
#include "MonsterScript.h"
#include "NpcScript.h"
#include "LevelScript.h"
#include "SkillScript.h"
#include "CommunityScript.h"
#include "StageScript.h"
#include "TotemScript.h"
#include "PvPScript.h"
#include "GatheringScript.h"
#include "VehicleScript.h"
#include "MakeSkillScript.h"
#include "TutorialScript.h"
#include "TwitterScript.h"

#include "GameFileLexer.h"
#include "Parser.h"
#include "FilterManager.h"

#include "ChatBubble.h"
#include "BaseObject.h"

#include "ItemManager.h"
#include "DramaturgyManager.h"
#include "TitleManager.h"
#include "QuestManager.h"
#include "TriggerManager.h"
#include "ShortcutManager.h"

#define HAIRCOLOR_KEY(hair,color)	(unsigned int)(((unsigned char)(hair)|((unsigned short)((unsigned char)(color))<<8)))

cGameResourceManager* cGameResourceManager::mpGameResourceManager = NULL;

cGameResourceManager::cGameResourceManager()
: mpStatusCalc(0)
, mpStatusScript(0)
, mpMonsterScript(0)
, mpLevelScript(0)
, mpNpcScript(0)
, mpCommunityScript(0)
, mpSkillScript(0)
, mpStageScript(0)
, mpTotemScript(0)
, mpPVPScript(0)
, mpGatheringScript(0)
, mpVehicleScript(0)
, mpMakeSkillScript(0)
, mpTwitterScript(0)
{
	mpGameResourceManager = this;

	mpGameText = 0;
	mpCategoryName = 0;

	mpItemToolTipText = 0;

	mpTarotCardText = 0;
	mpCardBookText = 0;

	mpSkillNameText = 0;
	mpSkillToolTipText = 0;
	
	mpNpcNameText = 0;
	mpMonsterNameText = 0;
	mpGatheringNameText = 0;
	mpQuestTitleText = 0;

	mpTitleText = 0;
	mpTipText = 0;
	mpNpcTarotText = 0;

	mpMapNameText = 0;
	mpEmotionFilterText = 0;
	mBaseFaceCount = 0;

	mHairColorMax = 0;	

	mpMonsterNote = 0;
	mTextItem = 0;

	mDataLoadThread = NULL;
	mDataLoadFlag = eData_Start;
}

cGameResourceManager::~cGameResourceManager()
{
	LoadEnd();

	if( mTextItem )
	{
		sTextItem* ptxt = mTextItem;
		sTextItem* pNext;
		while( ptxt )
		{
			pNext = ptxt->mpNext;
			delete ptxt;

			ptxt = pNext;
		}
		mTextItem = 0;
	}

	SAFE_DELETE( mpNpcTarotText );
	SAFE_DELETE( mpMonsterNote );
	SAFE_DELETE( mpEmotionFilterText );
	SAFE_DELETE( mpMapNameText );
	SAFE_DELETE( mpTipText );
	SAFE_DELETE( mpTitleText );
	SAFE_DELETE( mpQuestTitleText );
	SAFE_DELETE( mpGatheringNameText );
	SAFE_DELETE( mpMonsterNameText );
	SAFE_DELETE( mpNpcNameText );
	SAFE_DELETE( mpSkillToolTipText );
	SAFE_DELETE( mpSkillNameText );

	SAFE_DELETE( mpCardBookText );
	SAFE_DELETE( mpTarotCardText );

	SAFE_DELETE( mpItemToolTipText );

	SAFE_DELETE( mpCategoryName );
	SAFE_DELETE( mpGameText );

	mpGameResourceManager = 0;
}

/* ------------------------------------------------------------------
 * 함수이름 :	Init()
 * 목    적 :	Application의 Init()에서 실행된다.
 *				프로그램 실행시 한번만 로딩된다.
 * 주의사항 :	꼭 로딩해야 하는것들만 넣을것.. (Select창과 Make창에서 사용할 것들)
 * ------------------------------------------------------------------ */
bool cGameResourceManager::Init()
{
	mDataLoadFlag = eData_Start;

	/// Game용 텍스트 파일
	if( !LoadGameText() )
		return false;

	return true;
}

bool cGameResourceManager::LoadInitData1()
{
	if( mDataLoadFlag != eData_Start )
		return true;

	unsigned int  threadId;
	mDataLoadThread = (HANDLE)_beginthreadex( NULL, 0, LoadIntroStageInit, (LPVOID)this, 0, &threadId );
//	mDataLoadThread = CreateThread( NULL, 0, LoadIntroStageInit, (LPVOID)this, 0, &threadId );

	return (mDataLoadThread != NULL) ? true : false;
/*
	switch( mDataLoadFlag1 )
	{
	case eData_PlayerJob:
		{
			/// 종족을 key로 하는 직업 리스트 자료구조를 생성한다.
			for( unsigned int race=0; race<eRACE_MAX; ++race )
			{
				cJobIndexArray* pArray = new cJobIndexArray;
				mRaceTree.Insert( race, pArray );
			}

			/// 직업에 관련한 연결 정보를 생성
			if( !LoadPlayerJob() )
			{
				return eData_Error;
			}
			mDataLoadFlag1 = eData_SkillInfo;
		}
		break;
	case eData_SkillInfo:
		{
			if( !LoadSkillInfo() )
			{
				return eData_Error;
			}

			/// 070615 PKH 스킬 스크립트 로드
			mpSkillScript = new cSkillScript;
			if( !mpSkillScript->Init() )
			{
				return false;
			}

			mpCommunityScript = new cCommunityScript;
			if( !mpCommunityScript->Init() )
			{
				return false;
			}

			mDataLoadFlag1 = eData_StatusScript;
		}
		break;
	case eData_StatusScript:
		{
			/// 070115 PKH 스텟 계산기
			mpStatusScript = new cStatusScript;
			if( !mpStatusScript->Init() )
			{
				return eData_Error;
			}
			mpStatusCalc = new cStatusCalc_Client;
			mDataLoadFlag1 = eData_NPCScript;
		}
		break;
	case eData_NPCScript:
		{
			/// NPC
			mpNpcScript = new cNPCScript;
			if( !mpNpcScript->Init( ) )
			{
				return eData_Error;
			}

			mDataLoadFlag1 = eData_MonsterScript;
		}
		break;
	case eData_MonsterScript:
		{
			/// 070130 PKH 몬스터 스크립트
			mpMonsterScript = new cMonsterScript;
			if( !mpMonsterScript->Init( ) )
			{
				return eData_Error;
			}


			mDataLoadFlag1 = eData_ItemInfo;
		}
		break;
	case eData_ItemInfo:
		{
			if( ITEMMAN->Init() == false )
				return eData_Error;

			mDataLoadFlag1 = eData_ModelInfo;
		}
		break;
	case eData_ModelInfo:
		{
			/// Model정보
			if( !LoadModelFileInfo() )
			{
				return eData_Error;
			}
			mDataLoadFlag1 = eData_EmotionInfo;
		}
		break;
	case eData_EmotionInfo:
		{
			if( !LoadFaceEmotionInfo() )
			{
				return eData_Error;
			}

			unsigned int characterCount = eGENDER_MAX * eRACE_MAX;
			cFileNameArray* pArray = 0;
			cString* name = NULL;

			cArrayHashMap::cIterator iter;
			cArrayHashMap::cIterator iterEnd;

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
							pathName.Format( "./Data/Character/%s", name->Cstr() );
							RESOURCEMAN->LoadNIF( pathName );
						}
					}
				}
			}

			mDataLoadFlag1 = eData_End;
		}
		break;
	default:
		return eData_Error;
	}

	return mDataLoadFlag1;
*/
}

bool cGameResourceManager::LoadInitData2()
{
	if( mDataLoadFlag != eData_Start )
		return true;

	unsigned int  threadId;
	mDataLoadThread = (HANDLE)_beginthreadex( NULL, 0, LoadLoginStageInit, (LPVOID)this, 0, &threadId );
//	mDataLoadThread = CreateThread( NULL, 0, LoadLoginStageInit, (LPVOID)this, 0, &threadId );

	return (mDataLoadThread != NULL) ? true : false;
/*
	switch( mDataLoadFlag2 )
	{
	case eData_BaseHair:
		{
			if( !LoadCharacterBaseHairFileInfo() )
			{
				return eData_Error;
			}

			mLoadCount = 0;
			mDataLoadFlag2 = eData_BaseHairEx;
		}
		break;
	case eData_BaseHairEx:
		{
			unsigned int characterCount = eGENDER_MAX * eRACE_MAX;
			cFileNameArray* pArray = 0;
			cString* name = NULL;

			/// default hair loading
			cArrayHashMap::cIterator iter = mDefaultHairFileNameMap.Begin();
			for(unsigned long c=0;c<mLoadCount;c++)
				iter++;

			if( iter != mDefaultHairFileNameMap.End() )
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
							pathName.Format( "./Data/Character/%s", name->Cstr() );
							RESOURCEMAN->LoadNIF( pathName );
						}
					}
				}
				mLoadCount++;
			}
			else
			{
				mLoadCount = 0;
				mDataLoadFlag2 = eData_ModelData;
			}
		}
		break;
	case eData_ModelData:
		{
			/// kfm파일 로딩..
			cString* pModelFile;
			cArrayHashMap::cIterator iter = mModelFileNameMap.Begin();
			for(unsigned long c=0;c<mLoadCount;c++)
				iter++;

			if( iter != mModelFileNameMap.End() )
			{
				unsigned long idx = (unsigned long)(*iter).mFirst;
				pModelFile = (cString*)(*iter).mSecond;

				if( pModelFile )
				{
					cString pathName;
					if( idx <= 100 )
					{
						pathName.Format( "./Data/Character/%s", pModelFile->Cstr() );
						RESOURCEMAN->LoadKFM( pathName );
					}
				}
				mLoadCount++;
			}
			else
			{
				mLoadCount = 0;
				mDataLoadFlag2 = eData_End;
			}
		}
		break;
	case eData_End:
		break;
	default:
		return eData_Error;
	}

	return mDataLoadFlag2;
*/
}

bool cGameResourceManager::LoadInitData3()
{
	if( mDataLoadFlag != eData_Start )
		return true;

	unsigned int  threadId;
	mDataLoadThread = (HANDLE)_beginthreadex( NULL, 0, LoadServerStageInit, (LPVOID)this, 0, &threadId );
	//mDataLoadThread = CreateThread( NULL, 0, LoadServerStageInit, (LPVOID)this, 0, &threadId );

	return (mDataLoadThread != NULL) ? true : false;
/*
	switch( mDataLoadFlag3 )
	{
	case eData_BaseFile:
		{
			if( !LoadCharacterBaseFileInfo() )
			{
				return eData_Error;
			}

			mLoadCount = 0;
			mDataLoadFlag3 = eData_DefaultData;
		}
		break;
	case eData_DefaultData:
		{
			unsigned int characterCount = eGENDER_MAX * eRACE_MAX;
			cFileNameArray* pArray = 0;
			cString* name = NULL;

			/// default body, hand, foot loading
			cArrayHashMap::cIterator iter = mDefaultWearFileNameMap.Begin();
			for(unsigned long c=0;c<mLoadCount;c++)
				iter++;

			if( iter != mDefaultWearFileNameMap.End() )
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
							pathName.Format( "./Data/Character/%s", name->Cstr() );
							RESOURCEMAN->LoadNIF( pathName );
						}
					}
				}

				mLoadCount++;
			}
			else
			{
				RESOURCEMAN->LoadNIF( "./Data/Effect/DropBox.nif" );
				RESOURCEMAN->LoadNIF( "./Data/Effect/DropBox02.nif" );

				mLoadCount = 0;
				mDataLoadFlag3 = eData_DramaInfo;
			}
		}
		break;
	case eData_DramaInfo:
		{
			if( DRAMATURGYMAN->Init() == false )
				return eData_Error;

			mDataLoadFlag3 = eData_TitleInfo;
		}
		break;
	case eData_TitleInfo:
		{
			if( TITLEMAN->Init() == false )
				return eData_Error;

			mDataLoadFlag3 = eData_End;
		}
		break;
	case eData_End:
		break;
	default:
		return eData_Error;
	}

	return mDataLoadFlag3;
*/
}


bool cGameResourceManager::InitLogin()
{
	mDataLoadFlag = eData_Start;

	if( !LoadMapInfo() )
	{
		return false;
	}

	if( !LoadTarotDramaPattern() )
	{
		return false;
	}

	/// 삭제코드
	mpStageScript = new cStageScript;
	if( !mpStageScript->Init() )
	{
		return false;
	}

	/// 070515 PKH 레벨 테이블 로드
	mpLevelScript = new cLevelScript;
	if( !mpLevelScript->Init() )
	{
		return false;
	}

	return true;
}

bool cGameResourceManager::InitServer()
{
	mDataLoadFlag = eData_Start;

	mpTotemScript = new cTotemScript;
	if( !mpTotemScript->Init() )
	{
		return false;
	}

	mpPVPScript = new cPvPScript;
	if( mpPVPScript->Init() == false )
	{
		return false;
	}

	mpGatheringScript = new cGatheringScript;
	if( mpGatheringScript->Init() == false )
	{
		return false;
	}

	mpVehicleScript = new cVehicleScript;
	if( mpVehicleScript->Init() == false )
	{
		return false;
	}

	mpTwitterScript = new cTwitterScript;
	if( mpTwitterScript->Init() == false )
	{
		return false;
	}

	return true;
}


/* ------------------------------------------------------------------
 * 함수이름 :	Exit()
 * 목    적 :	종료부
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cGameResourceManager::Exit()
{

	SAFE_DELETE( mpTwitterScript );
	SAFE_DELETE( mpVehicleScript );

	/// 081211 PKH 채집
	if( mpGatheringScript )
	{
		mpGatheringScript->Release();
		delete mpGatheringScript;
		mpGatheringScript = 0;
	}

	///
	SAFE_DELETE( mpPVPScript );

	/// 070615 PKH 스킬 스크립트 제거
	if( mpTotemScript )
	{
		mpTotemScript->Release();
		delete mpTotemScript;
		mpTotemScript = 0;
	}

	///
	SAFE_DELETE(mpCommunityScript);

	/// 070615 PKH 스킬 스크립트 제거
	if( mpSkillScript )
	{
		mpSkillScript->Release();
		delete mpSkillScript;
		mpSkillScript = 0;
	}

	if( mpMakeSkillScript )
	{
		delete mpMakeSkillScript;
		mpMakeSkillScript = 0;
	}

	/// 070515 PKH 레벨 테이블 제거
	if( mpLevelScript )
	{
		mpLevelScript->Release();
		delete mpLevelScript;
		mpLevelScript = 0;
	}

	/// 070130 PKH 몬스터
	if( mpMonsterScript )
	{
		mpMonsterScript->Release();
		delete mpMonsterScript;
		mpMonsterScript = 0;
	}

	if( mpStageScript )
	{
		mpStageScript->Release();
		delete mpStageScript;
		mpStageScript = 0;
	}

	if( mpNpcScript )
	{
		mpNpcScript->Release();
		delete mpNpcScript;
		mpNpcScript = 0;
	}

	/// 070117 PKH 스텟 계산기
	SAFE_DELETE( mpStatusCalc );

	if( mpStatusScript )
	{
		mpStatusScript->Release();
		delete mpStatusScript;
		mpStatusScript = 0;
	}
	cArrayHashMap::cIterator i = mTarotDramaPattern.Begin();
	cArrayHashMap::cIterator end = mTarotDramaPattern.End();
	for( ; i != end; ++i )
	{
		unsigned long* p = (unsigned long*)(*i).mSecond;
		SAFE_DELETE_ARRAY(p);
	}
	mTarotDramaPattern.Clear();

	i = mMapInfoMap.Begin();
	end = mMapInfoMap.End();
	for( ; i != end; ++i )
	{
		cArrayHashMap* pModeMap = (cArrayHashMap*)(*i).mSecond;
		if( pModeMap != NULL )
		{
			cArrayHashMap::cIterator i2 = pModeMap->Begin();
			cArrayHashMap::cIterator e2 = pModeMap->End();
			for( ; i2 != e2 ; ++i2 )
			{
				sMapInfo* p = (sMapInfo*)(*i2).mSecond;
				SAFE_DELETE(p);
			}
			pModeMap->Clear();
			SAFE_DELETE( pModeMap );
		}
	}
	mMapInfoMap.Clear();

	i = mFaceEmotionMap.Begin();
	end = mFaceEmotionMap.End();
	for( ; i != end; ++i )
	{
		sFaceEmoteInfo* p = (sFaceEmoteInfo*)(*i).mSecond;
		SAFE_DELETE(p);
	}
	mFaceEmotionMap.Clear();

	RemoveAllCharacterBaseFileInfo();
	RemoveAllModelFileName();
	RemovePlayerJobInfo();
}

/* ------------------------------------------------------------------
 * 함수이름 :	DataLoading()
 * 목    적 :	읽어야 할 데이터들의 목록을 참조해서 데이터 로딩
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cGameResourceManager::DataLoading()
{
	/// Queue들에 저장되어 있는 파일을 로드한다.

	/*
	NiTQueue<char*>*		mpMapTextureFileQueue;
	NiTQueue<char*>*		mpObjectFileQueue;
	NiTQueue<char*>*		mpModelFileQueue;
	NiTQueue<char*>*		mpSoundFileQueue;
	*/

}

/* ------------------------------------------------------------------
 * 함수이름 :	GetItemFileName( unsigned long itemFileIdx, char race, char gender )
 * 목    적 :	아이템 파일명을 획득
 * 주의사항 :	이곳의 첫번째 인자는 아이템 번호가 아니라 아이템 파일 번호다
 * ------------------------------------------------------------------ */
//const char* cGameResourceManager::GetItemFileName( unsigned long itemFileIdx, char race, char gender )
//{
//	assert(race < eRACE_MAX);
//	assert(gender < eGENDER_MAX);
//	if( race < eRACE_MAX || gender < eGENDER_MAX )
//	{
//		cFileNameArray* pArray = (cFileNameArray*)mItemFileMap.GetAt( itemFileIdx );
//		if( pArray )
//		{
//			cString* name = (cString*)(*pArray)[(race*eGENDER_MAX) + gender];
//			if( name )
//				return name->Cstr();
//		}
//	}
//	return NULL;
//}

/* ------------------------------------------------------------------
 * 함수이름 :	LoadItemFileInfo()
 * 목    적 :	아이템용 그래픽 파일 정보
 * 주의사항 :	
 * ------------------------------------------------------------------ */
/*
bool cGameResourceManager::LoadItemFileInfo()
{

	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Resource/ItemFileList.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Resource/ItemFileList.txt" );

	cFileNameArray* pArray = 0;
	unsigned long itemFileIdx = 0;
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

				itemFileIdx = (unsigned long)token.ToInt();
				for( unsigned int i=0; i<count; ++i)
				{
					fileName = new cString;
					*fileName = parser.ParseString();
					pArray->PushBack( fileName );

					/// 임시 테스트로 데이터를 모두 읽는다.
					RESOURCEMAN->LoadNIFFile( fileName, cResourceManager::eCHARACTEREFFECT );
				}

				mItemFileMap.Insert( itemFileIdx, pArray );
			}
			break;
		default:
			assert( 0 && "invalid token" );
			return false;
		}
	}
	return true;
}
*/

/* ------------------------------------------------------------------
 * 함수이름 :	RemoveAllItemInfo()
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
/*
void cGameResourceManager::RemoveAllItemInfo()
{
	unsigned long idx;
	sItemTotalInfo* pInfo = NULL;
	NiTMapIterator iter = mItemInfoMap.GetFirstPos();
	while( iter )
	{
		mItemInfoMap.GetNext( iter, idx, pInfo );
		SAFE_DELETE( pInfo );
	}
	mItemInfoMap.RemoveAll();
}
*/

/* ------------------------------------------------------------------
 * 함수이름 :	GetModelFileName( unsigned long modelIndex )
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
const char* cGameResourceManager::GetModelFileName( unsigned long modelIndex )
{
	cString* str = (cString*)mModelFileNameMap.GetAt( modelIndex );
	if( !str ) return 0;

	return str->Cstr();
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
const char* cGameResourceManager::GetDefaultFaceFileName( char race, char gender )
{
	assert(race < eRACE_MAX);
	assert(gender < eGENDER_MAX);
	if( race < eRACE_MAX || gender < eGENDER_MAX )
	{
		cFileNameArray* pArray = (cFileNameArray*)mDefaultFaceFileNameMap.GetAt( 0 );
		assert(pArray);
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

bool cGameResourceManager::LoadMapInfo()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/MapData.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MapData.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sMapInfo* pInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		pInfo = new sMapInfo;

		// Map number
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mapNum = str.ToInt();

		if( pInfo->mapNum > 999 )	{ goto ERR; }

		cArrayHashMap* pModeMap = (cArrayHashMap*)mMapInfoMap.GetAt( pInfo->mapNum );
		if( pModeMap == NULL )
		{
			pModeMap = new cArrayHashMap;
			mMapInfoMap.Insert( pInfo->mapNum, pModeMap );
		}

		/// Map 폴더번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->folderIdx = str.ToInt();

		/// 한맵에 여러개의 폴더 인덱스를 가질수 없다.
		if( pModeMap->GetSize() != 0 )
		{
			cArrayHashMap::cIterator b = pModeMap->Begin();
			sMapInfo* pMapInfo = (sMapInfo*)(*b).mSecond;
			if( pMapInfo == NULL )
				goto ERR;

			if( pMapInfo->folderIdx != pInfo->folderIdx )
				goto ERR;			
		}

		/// Map Loading Image
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->textureName = str;

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->useTile = (str.ToInt() == 0)? false:true;

		/// Map BGM file
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->bgmName = str;

		/// Map BGM WaitTime
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->bgmWaitTime = str.ToInt();

		/// 난이도
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mapMode = (unsigned char)str.ToInt();

		/// 일반맵에 다른 모드를 사용할 수 없다.
		if( MAP_MIN <= pInfo->mapNum && pInfo->mapNum <= MAP_MAX && pInfo->mapMode != DEFAULT_MAPMODE )
			goto ERR;

		/// 최소 요구 레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->minLevel = (unsigned char)str.ToInt();

		/// 최대 제한 레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->maxLevel = (unsigned char)str.ToInt();

		/// 최대 인원 
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->maxPlayerCnt = (unsigned char)str.ToInt();

		/// 요구 아이템
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->requireItem = str.ToInt();

		if( pModeMap->Insert( pInfo->mapMode, pInfo ) == false )
		{
			assert(0);
			goto ERR;
		}

//		/// 해쉬에 몬스터 별로 기록
//		if( mMapInfoMap.Insert( pInfo->mapNum, pInfo ) == false )
//		{
//			continue;
////			assert(0);
////			goto ERR;
//		}
	}

	return true;

ERR:
	delete pInfo;
	return false;
}

bool cGameResourceManager::LoadFaceEmotionInfo()
{
	cUIText	emotionFilterText;

	cStringW path;
	path.Format( L"./language/%s/EmotionFilterName.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( emotionFilterText.Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load emotionFilterName" );
		return false;
	}

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
			if( filterKey == 0 )
				continue;

			const cStringT text = emotionFilterText.GetText( filterKey );
			if( text.GetLength() == 0 )
				continue;

			FILTERMAN->AddEmotionFilter( text.Cstr(), pInfo->index );
		}

		/// 해쉬에 몬스터 별로 기록
		if( mFaceEmotionMap.Insert( pInfo->index, pInfo ) == false )
		{
			assert(0);
			goto ERR;
		}

		if( pInfo->type == 0 )
			mBaseFaceCount++;
	}

	return true;

ERR:
	delete pInfo;
	return false;
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
//				pArray->Reserve( count );

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
//				pArray->Reserve( count );

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
//				pArray->Reserve( count );

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
		SAFE_DELETE( pModelFile );
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

/// 주의 사항 : Player의 직업 연결을 생성한다.
bool cGameResourceManager::LoadPlayerJob()
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Script/Resource/PlayerJobTree.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cGameFileLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Resource/PlayerJobTree.txt" );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_JOBINFO:
			{
				if( LoadPlayerJobInfo(parser) == false )
				{
					assert(0);
					return false;
				}
			}
			break;
		case eTOKEN_JOBTREE:
			{
				unsigned int depth = parser.ParseInt();

				if( LoadPlayerJobTree(parser, depth) == false )
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

///
bool cGameResourceManager::LoadPlayerJobInfo( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	unsigned long jobIdx = 0;
	unsigned char race = 0;
	unsigned int jobNameIdx = 0;
	unsigned long iconIdx = 0;

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
				jobIdx = (unsigned long)token.ToInt();
				race   = (unsigned char)parser.ParseInt();
				jobNameIdx = parser.ParseInt();
				iconIdx = (unsigned long)parser.ParseInt();

				AddRaceTree( race, jobIdx );

				sJobInfo* info = new sJobInfo;
				info->jobNameIndex = jobNameIdx;
				info->jobTargetIconIndex = iconIdx;

				if( mJobInfoMap.Insert( jobIdx, info ) == false )
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

///
bool cGameResourceManager::LoadPlayerJobTree( cParser& parser, unsigned int depth )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		assert( 0 && "wrong script" );
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

		switch( token.mType )
		{
		case eTOKEN_ERROR:
			return false;
		case eTOKEN_NULL:
			continue;
		case eTOKEN_INT:
			{
				cJobIndexArray	tempArray;

				/// 기본 직업군을 읽어들인다.
				unsigned long jobIdx = token.ToInt();
				tempArray.PushBack(jobIdx);

				/// 최종 차수까지의 직업군을 모두 담는다.
				for( unsigned int i=0; i<depth-1; ++i )
				{
					unsigned long jobIdx = (unsigned long)parser.ParseInt();
					tempArray.PushBack(jobIdx);
				}

				if( tempArray.GetSize() )
				{
					/// 정보를 담는다.
					for( int i=tempArray.GetSize()-1; i>0; --i )
					{
						/// 최종 전직차수의 직업 인덱스를 key로 사용
						unsigned long jobIdx = tempArray[i];

						/// 새로운 직업연결을 생성
						cJobIndexArray* newJobTree = new cJobIndexArray;

						/// 차수까지의 부모 직업들을 담는다.
						for( int k=0 ; k<i ; ++k )
						{
							unsigned long parentJob = tempArray[k];
							newJobTree->PushBack( parentJob );
						}

						/// 참조 맵을 구성한다.
						if( mJobTree.Insert( jobIdx, newJobTree) == false )
						{
							assert(0);
							SAFE_DELETE(newJobTree);
						}
					}
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
void cGameResourceManager::RemovePlayerJobInfo()
{
	cArrayHashMap::cIterator i = mJobTree.Begin();
	cArrayHashMap::cIterator end = mJobTree.End();

	for( ; i != end; ++i )
	{
		cJobIndexArray* pArray = (cJobIndexArray*)(*i).mSecond;
		SAFE_DELETE(pArray);
	}
	mJobTree.Clear();

	cArrayHashMap::cIterator b = mJobInfoMap.Begin();
	cArrayHashMap::cIterator bend = mJobInfoMap.End();

	for( ; b != bend; ++b )
	{
		sJobInfo* info = (sJobInfo*)(*b).mSecond;
		SAFE_DELETE(info);
	}
	mJobInfoMap.Clear();

	/// 종족 - 직업군... 제거
	i = mRaceTree.Begin();
	end = mRaceTree.End();

	for( ; i != end; ++i )
	{
		cJobIndexArray* pArray = (cJobIndexArray*)(*i).mSecond;
		SAFE_DELETE(pArray);
	}
	mRaceTree.Clear();
}

/* ------------------------------------------------------------------
* 함수이름 :	LoadSkillInfo
* 목    적 :	스킬에 대한 정보 로딩
* 주의사항 :	연출 파일도 같이 로딩한다.
* ------------------------------------------------------------------ */
bool cGameResourceManager::LoadSkillInfo()
{
/*
	/// 스킬 리스트 로딩.
	if(!SKILLMAN->SkillScriptLoad())
	{
		return false;
	}

	/// 연출 리스트 로딩.
	if( !SKILLMAN->PlayerDramaScriptLoad() ) return false;
	if( !SKILLMAN->MonsterDramaScriptLoad() ) return false;
*/
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

	cStringW path;
	path.Format( L"./language/%s/SystemMsg.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpGameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load GameText" );
		return false;
	}

	mpCategoryName = new cUIText;
	if( !mpCategoryName )
	{
		assert( 0 && "mpCategoryName not create!" );
		return false;
	}

	path.Format( L"./language/%s/Item_CategoryName.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpCategoryName->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load CategoryName" );
		return false;
	}

	/// 아이템 툴팁
	mpItemToolTipText = new cUIText;
	if( !mpItemToolTipText )
	{
		assert( 0 && "mpItemToolTipText not create!" );
		return false;
	}

	path.Format( L"./language/%s/Item_Tooltip.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpItemToolTipText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load GameText" );
		NiMessageBox( "failed to load GameText", "error" );
		return false;
	}

	/// 타롯 설명
	mpTarotCardText = new cUIText;
	if( !mpTarotCardText )
	{
		assert( 0 && "mpTarotCardText not create!" );
		return false;
	}

	path.Format( L"./language/%s/Card_Tarot.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpTarotCardText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load TarotText" );
		return false;
	}

	/// 카드북 설명
	mpCardBookText = new cUIText;
	if( !mpCardBookText )
	{
		assert( 0 && "mpCardBookText not create!" );
		return false;
	}

	path.Format( L"./language/%s/CardBook_Tooltip.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpCardBookText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load CardBookText" );
		return false;
	}

	/// 스킬명
	mpSkillNameText = new cUIText;
	if( !mpSkillNameText )
	{
		assert( 0 && "mpSkillNameText not create!" );
		return false;
	}

	path.Format( L"./language/%s/Skill_Name.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpSkillNameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load GameText" );
		return false;
	}

	/// 스킬 툴팁
	mpSkillToolTipText = new cUIText;
	if( !mpSkillToolTipText )
	{
		assert( 0 && "mpSkillToolTipText not create!" );
		return false;
	}

	path.Format( L"./language/%s/Skill_Tooltip.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpSkillToolTipText->Load( path.Cstr() ) == false )
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

	path.Format( L"./language/%s/NPC_Name.txt", cApplication::mLangaugeFolderW.Cstr() );
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

	path.Format( L"./language/%s/Mon_Name.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpMonsterNameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load MonsterName" );
		return false;
	}

	/// 
	mpGatheringNameText = new cUIText;
	if( !mpGatheringNameText )
	{
		assert( 0 && "mpGatheringNameText not create!" );
		return false;
	}

	path.Format( L"./language/%s/gatheringname.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpGatheringNameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load GatheringName" );
		return false;
	}

	/// 퀘스트 타이틀 명
	mpQuestTitleText = new cUIText;
	if( !mpQuestTitleText )
	{
		assert( 0 && "mpQuestTitleText not create!" );
		return false;
	}

	path.Format( L"./language/%s/Quest_Name.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpQuestTitleText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load QuestName" );
		return false;
	}

	/// 호칭 관리
	mpTitleText = new cUIText;
	if( !mpTitleText )
	{
		assert( 0 && "mpTitleText not create!" );
		return false;
	}

	path.Format( L"./language/%s/TitleMsg.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpTitleText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load Title" );
		return false;
	}

	/// 팁 텍스트 관리
	mpTipText = new cUIText;
	if( !mpTipText )
	{
		assert( 0 && "mpTipText not create!" );
		return false;
	}

	path.Format( L"./language/%s/Tip_Text.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpTipText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load Tip Text" );
		return false;
	}

	mpMapNameText = new cUIText;
	if( !mpMapNameText )
	{
		assert( 0 && "mpMapNameText not create!" );
		return false;
	}

	path.Format( L"./language/%s/Map_Name.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpMapNameText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load MapName" );
		return false;
	}

	mpEmotionFilterText = new cUIText;
	if( !mpEmotionFilterText )
	{
		assert( 0 && "mpEmotionFilterText not create!" );
		return false;
	}

	path.Format( L"./language/%s/EmotionFilterName.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpEmotionFilterText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load EmotionFilterName" );
		return false;
	}

	mpMonsterNote = new cUIFuncText;
	if( !mpMonsterNote )
	{
		assert( 0 && "mpMonsterNote not create!" );
		return false;
	}

	cStringT pathname;
	pathname.Format( L"./language/%s/Mon_Note.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpMonsterNote->Load( pathname.Cstr() ) == false )
	{
		assert( 0 && "failed to load Monster Note" );
		return false;
	}

	mpNpcTarotText = new cUIText;
	if( !mpNpcTarotText )
	{
		assert( 0 && "mpNpcTarotText not create!" );
		return false;
	}

	path.Format( L"./language/%s/Card_NpcTarot.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpNpcTarotText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load mpNpcTarotText" );
		return false;
	}

	return true;
}

bool cGameResourceManager::LoadTarotDramaPattern()
{

	cFileLoader loader;
	cString pathName = "./Script/Resource/TarotDramaPattern.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load TarotDramaPattern.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	unsigned long* arr = 0;
	while( tokenizer.IsEnd() == false )
	{
		arr = new unsigned long[22];

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		unsigned int index = str.ToInt();

		for( unsigned int i=0; i<22; i++ )
		{
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			arr[i] = str.ToInt();
		}

		/// 해쉬에 몬스터 별로 기록
		if( mTarotDramaPattern.Insert( index, arr ) == false )
		{
			assert(0);
			goto ERR;
		}
	}

	return true;

ERR:
	delete arr;
	return false;
}

unsigned long* cGameResourceManager::GetTarotPattern()
{
	unsigned int size = mTarotDramaPattern.GetSize();
	if( size == 0 )
	{
		assert(0);
		return 0;
	}

	unsigned int idx = ::rand() % size;

	return (unsigned long*)mTarotDramaPattern.GetAt( idx );

}



LPCTSTR cGameResourceManager::GetGameText( unsigned int textIdx )
{
	assert(mpGameText);
	return mpGameText->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetCategoryName( unsigned int textIdx )
{
	assert( mpCategoryName );
	return mpCategoryName->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetItemToolTipText( unsigned int textIdx )
{
	assert(mpItemToolTipText);
	return mpItemToolTipText->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetTarotText( unsigned int textIdx )
{
	assert(mpTarotCardText);
	return mpTarotCardText->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetCardBookText( unsigned int textIdx )
{
	assert( mpCardBookText );
	return mpCardBookText->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetSkillNameText( unsigned int textIdx )
{
	assert(mpSkillNameText);
	return mpSkillNameText->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetSkillToolTipText( unsigned int textIdx )
{
	assert(mpSkillToolTipText);
	return mpSkillToolTipText->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetNpcName( unsigned int nameIdx )
{
	assert( mpNpcNameText );
	return mpNpcNameText->GetText( nameIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetHeroTitle( unsigned int titleIdx )
{
	assert( mpTitleText );
	return mpTitleText->GetText( titleIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetTipText( unsigned int textIdx )
{
	assert( mpTipText );
	return mpTipText->GetText( textIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetMonsterName( unsigned int nameIdx )
{
	assert( mpMonsterNameText );
	return mpMonsterNameText->GetText( nameIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetGatheringName( unsigned int nameIdx )
{
	assert( mpGatheringNameText );
	return mpGatheringNameText->GetText( nameIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetQuestTitle( unsigned int nameIdx )
{
	assert( mpQuestTitleText );
	return mpQuestTitleText->GetText( nameIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetMapName( unsigned int nameIdx )
{
	assert( mpMapNameText );
	return mpMapNameText->GetText( nameIdx ).Cstr();
}

LPCTSTR cGameResourceManager::GetNpcTarotText( unsigned int textIdx )
{
	assert( mpNpcTarotText );
	return mpNpcTarotText->GetText( textIdx ).Cstr();
}

LPCTSTR	cGameResourceManager::GetMapLoadTexture( unsigned int mapIdx, unsigned char mapMode )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
		return 0;
	}

	sMapInfo* info = (sMapInfo*)pMap->GetAt( mapMode );
	if( info == 0 )
	{
		assert(0);
		return 0;
	}

	return (LPCTSTR)info->textureName.Cstr();
}

cString	cGameResourceManager::GetMapSoundName( unsigned int mapIdx, unsigned char mapMode )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
		return 0;
	}

	sMapInfo* info = (sMapInfo*)pMap->GetAt( mapMode );
	if( info == 0 )
	{
		assert(0);
		return 0;
	}

	return info->bgmName;
}

unsigned long cGameResourceManager::GetMapSoundWaitTime( unsigned int mapIdx, unsigned char mapMode )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
		return 0;
	}

	sMapInfo* info = (sMapInfo*)pMap->GetAt( mapMode );
	if( info == 0 )
	{
		assert(0);
		return 0;
	}

	return info->bgmWaitTime;
}

unsigned char	cGameResourceManager::GetMapMinLevel( unsigned int mapIdx, unsigned char mapMode )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
		return 0;
	}

	sMapInfo* info = (sMapInfo*)pMap->GetAt( mapMode );
	if( info == 0 )
	{
		assert(0);
		return 0;
	}

	return info->minLevel;

}

unsigned char	cGameResourceManager::GetMapMaxLevel( unsigned int mapIdx, unsigned char mapMode )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
		return 0;
	}

	sMapInfo* info = (sMapInfo*)pMap->GetAt( mapMode );
	if( info == 0 )
	{
		assert(0);
		return 0;
	}

	return info->maxLevel;

}

unsigned char	cGameResourceManager::GetMapMaxPlayerCount( unsigned int mapIdx, unsigned char mapMode )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
		return 0;
	}

	sMapInfo* info = (sMapInfo*)pMap->GetAt( mapMode );
	if( info == 0 )
	{
		assert(0);
		return 0;
	}

	return info->maxPlayerCnt;

}

unsigned long	cGameResourceManager::GetMapRequireItem( unsigned int mapIdx, unsigned char mapMode )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
		return 0;
	}

	sMapInfo* info = (sMapInfo*)pMap->GetAt( mapMode );
	if( info == 0 )
	{
		assert(0);
		return 0;
	}

	return info->requireItem;

}


bool cGameResourceManager::IsMapUseTile( unsigned int mapIdx, unsigned char mapMode )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
		return 0;
	}

	sMapInfo* info = (sMapInfo*)pMap->GetAt( mapMode );
	if( info == 0 )
	{
		assert(0);
		return 0;
	}
	return info->useTile;
}

unsigned long cGameResourceManager::GetMapFolderIdx( unsigned long mapIdx, unsigned char mapMode )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
		return 0;
	}

	sMapInfo* info = (sMapInfo*)pMap->GetAt( mapMode );
	if( info == 0 )
	{
		assert(0);
		return 0;
	}
	return info->folderIdx;
}

void cGameResourceManager::GetMapMode( unsigned long mapIdx, tArray<unsigned char>* modeArr )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		assert(0);
	}

	cArrayHashMap::cIterator pos = pMap->Begin();
	cArrayHashMap::cIterator end = pMap->End();
	for( ; pos != end; ++pos )
	{
		sMapInfo* pInfo = (sMapInfo*)(*pos).mSecond;
		if( pInfo )
		{
			modeArr->PushBack( pInfo->mapMode );
		}
		else
		{
			assert(0);
		}
	}
}

////
bool cGameResourceManager::IsCorrelateJob( unsigned long currentJob, unsigned long checkJob )
{
	if( currentJob == checkJob )
		return true;

	cJobIndexArray* pArray = (cJobIndexArray*)mJobTree.GetAt( currentJob );
	if( pArray )
	{
		for( unsigned int i=0; i<pArray->GetSize(); ++i )
		{
			if( (*pArray)[i] == checkJob )
			{
				return true;
			}
		}
	}

	return false;
}

bool cGameResourceManager::IsCorrelateMageJob( unsigned long currentJob )
{
	if( currentJob == ePLAYER_MAGE || currentJob == ePLAYER_WARLOCK )
		return true;

	cJobIndexArray* pArray = (cJobIndexArray*)mJobTree.GetAt( currentJob );
	if( pArray )
	{
		for( unsigned int i=0; i<pArray->GetSize(); ++i )
		{
			if( (*pArray)[i] == ePLAYER_MAGE || (*pArray)[i] == ePLAYER_WARLOCK )
			{
				return true;
			}
		}
	}

	return false;
}

LPCTSTR cGameResourceManager::GetJobName( unsigned long jobIdx )
{
	//unsigned int nameIdx = 0;

	sJobInfo* info = (sJobInfo*)mJobInfoMap.GetAt( jobIdx );
	if( info )
	{
		return GetGameText( info->jobNameIndex );
	}
	return 0;
}

sJobInfo* cGameResourceManager::GetJobInfo( unsigned long jobIdx )
{
	return (sJobInfo*)mJobInfoMap.GetAt( jobIdx );
}

LPCTSTR cGameResourceManager::GetRaceName( unsigned char race )
{
	switch( race )
	{
	case eRACE_HUMAN:
		{
			return GetGameText( 1000 );
		}
		break;
	case eRACE_BEAST:
		{
			return GetGameText( 1001 );
		}
		break;
	case eRACE_ELF:
		{
			return GetGameText( 1002 );
		}
		break;
	}
	return 0;
}

void cGameResourceManager::AddRaceTree( unsigned char race, unsigned long jobIdx )
{
	cJobIndexArray* pArray = (cJobIndexArray*)mRaceTree.GetAt( race );
	if( pArray )
	{
		pArray->PushBack( jobIdx );
	}
}

void cGameResourceManager::GetDefaultJob( unsigned char race, tArray<unsigned long>* out )
{
	if( out == 0) return;

	cJobIndexArray* pArray = (cJobIndexArray*)mRaceTree.GetAt( race );
	if( pArray )
	{
		for( unsigned int i=0; i< pArray->GetSize(); ++i )
		{
			if( (*pArray)[i] % 1000 == 0 )
			{
				out->PushBack( (*pArray)[i] );
			}
		}
	}
}

tArray<unsigned long>* cGameResourceManager::GetParentJob( unsigned long currentJob )
{
	cJobIndexArray* pArray = (cJobIndexArray*)mJobTree.GetAt( currentJob );
	if( pArray )
	{
		return pArray;
	}
	return 0;
}

unsigned int cGameResourceManager::GetJobStep( unsigned long jobIdx )
{
	cJobIndexArray* pArray = GetParentJob( jobIdx );
	if( pArray == 0 )
		return 0;

	return pArray->GetSize();
}

sTextItem* cGameResourceManager::GetMonsterTalk( unsigned long talkIdx, cBaseObject* mon, cBaseObject* target )
{
	/// delete default text item..
	if( mTextItem )
	{
		sTextItem* ptxt = mTextItem;
		sTextItem* pNext;
		while( ptxt )
		{
			pNext = ptxt->mpNext;
			delete ptxt;

			ptxt = pNext;
		}

		mTextItem = 0;
	}

	/// new text item..
	cUIFuncTextNode* node = mpMonsterNote->GetNode( talkIdx );
	if( node == 0 )
		return 0;

	unsigned long color = (unsigned long)0xFFFCFF9E;

	sTextItem* ptxt;
	sTextItem* pCurrent = 0;
	for( ; node; node = node->GetNext() )
	{
		ptxt = 0;
		switch( node->mType )
		{
		case eTEXTNODE_STRING:
			{
				ptxt = new sTextItem;
				::_tcscpy_s( ptxt->mText, node->mpString );
				ptxt->mColor = color;
			}
			break;
		case eTEXTNODE_VARNAME: 
			{
				if( target )
				{
					ptxt = new sTextItem;
					::_tcscpy_s( ptxt->mText, target->GetName() );
					ptxt->mColor = color;
				}
				else
				{
					ptxt = new sTextItem;
					::_tcscpy_s( ptxt->mText, _T("Wook!!") );
					ptxt->mColor = color;
				}
			}
			break;
		case eTEXTNODE_COLOR:
			color = node->mColor;
			break;

		default:
			assert( 0 && "invalid npc text node type" );
			return 0;
		}

		if( ptxt )
		{
			if( mTextItem == 0 )
			{
				mTextItem = pCurrent = ptxt;
			}
			else
			{
				pCurrent->mpNext = ptxt;
				pCurrent = ptxt;
			}
		}
	}

	return mTextItem;
}

unsigned int cGameResourceManager::LoadIntroStageInit( void* ptr )
{
	cGameResourceManager* manager = (cGameResourceManager*)ptr;
	return manager->IntroLoadThread( );
}

unsigned int cGameResourceManager::LoadLoginStageInit( void* ptr )
{
	cGameResourceManager* manager = (cGameResourceManager*)ptr;
	return manager->LoginLoadThread( );
}

unsigned int cGameResourceManager::LoadServerStageInit( void* ptr )
{
	cGameResourceManager* manager = (cGameResourceManager*)ptr;
	return manager->ServerLoadThread( );
}

void cGameResourceManager::LoadEnd()
{
	if( mDataLoadThread != NULL ) 
	{ 
		WaitForSingleObjectEx( mDataLoadThread, INFINITE, FALSE );

		CloseHandle(mDataLoadThread); 
		mDataLoadThread = 0; 
	}
}

DWORD cGameResourceManager::IntroLoadThread()
{
	/// 종족을 key로 하는 직업 리스트 자료구조를 생성한다.
	for( unsigned int race=0; race<eRACE_MAX; ++race )
	{
		cJobIndexArray* pArray = new cJobIndexArray;
		mRaceTree.Insert( race, pArray );
	}

	/// 직업에 관련한 연결 정보를 생성
	if( !LoadPlayerJob() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	if( !LoadSkillInfo() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	/// 070615 PKH 스킬 스크립트 로드
	mpSkillScript = new cSkillScript;
	if( !mpSkillScript->Init() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	mpMakeSkillScript = new cMakeSkillScript;
	if( !mpMakeSkillScript->Init() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	mpCommunityScript = new cCommunityScript;
	if( !mpCommunityScript->Init() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	/// 070115 PKH 스텟 계산기
	mpStatusScript = new cStatusScript;
	if( !mpStatusScript->Init() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}
	mpStatusCalc = new cStatusCalc_Client;
	if( mpStatusCalc == 0 )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	/// NPC
	mpNpcScript = new cNPCScript;
	if( !mpNpcScript->Init() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	/// 070130 PKH 몬스터 스크립트
	mpMonsterScript = new cMonsterScript;
	if( !mpMonsterScript->Init() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	if( ITEMMAN->Init() == false )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	/// Model정보
	if( !LoadModelFileInfo() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	if( !LoadFaceEmotionInfo() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	if( TRIGGERMAN->Init() == false )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	if( QUESTMAN->Init() == false )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	if( SHORTCUTMAN->Init() == false )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	unsigned int characterCount = eGENDER_MAX * eRACE_MAX;
	cFileNameArray* pArray = 0;
	cString* name = NULL;

	cArrayHashMap::cIterator iter;
	cArrayHashMap::cIterator iterEnd;

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
					pathName.Format( "./Data/Character/%s", name->Cstr() );
					RESOURCEMAN->LoadNIF( pathName );
				}
			}
		}
	}

	mDataLoadFlag = eData_End;
	return 0;
}

DWORD cGameResourceManager::LoginLoadThread()
{
	if( !LoadCharacterBaseHairFileInfo() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	{
		unsigned int characterCount = eGENDER_MAX * eRACE_MAX;
		cFileNameArray* pArray = 0;
		cString* name = NULL;

		/// default hair loading
		cArrayHashMap::cIterator iter = mDefaultHairFileNameMap.Begin();
		cArrayHashMap::cIterator iterEnd = mDefaultHairFileNameMap.End();

		for( ;iter != iterEnd; iter++ )
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
						pathName.Format( "./Data/Character/%s", name->Cstr() );
						RESOURCEMAN->LoadNIF( pathName );
					}
				}
			}
		}
	}

	{
		/// kfm파일 로딩..
		cString* pModelFile;
		cArrayHashMap::cIterator iter = mModelFileNameMap.Begin();
		cArrayHashMap::cIterator iterEnd = mModelFileNameMap.End();

		for( ;iter != iterEnd; iter++ )
		{
			unsigned long idx = (unsigned long)(*iter).mFirst;
			pModelFile = (cString*)(*iter).mSecond;

			if( pModelFile )
			{
				cString pathName;
				if( idx <= 100 )
				{
					pathName.Format( "./Data/Character/%s", pModelFile->Cstr() );
					RESOURCEMAN->LoadKFM( pathName );
				}
			}
		}
	}

	mDataLoadFlag = eData_End;
	return 0;
}

DWORD cGameResourceManager::ServerLoadThread()
{
	if( !LoadCharacterBaseFileInfo() )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	{
		unsigned int characterCount = eGENDER_MAX * eRACE_MAX;
		cFileNameArray* pArray = 0;
		cString* name = NULL;

		/// default body, hand, foot loading
		cArrayHashMap::cIterator iter = mDefaultWearFileNameMap.Begin();
		cArrayHashMap::cIterator iterEnd = mDefaultWearFileNameMap.End();

		for( ;iter != iterEnd; iter++ )			
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
						pathName.Format( "./Data/Character/%s", name->Cstr() );
						RESOURCEMAN->LoadNIF( pathName );
					}
				}
			}
		}

		RESOURCEMAN->LoadNIF( "./Data/Effect/DropBox.nif" );
		RESOURCEMAN->LoadNIF( "./Data/Effect/eff_pc_itemdrop_01.nif" );
		RESOURCEMAN->LoadNIF( "./Data/Effect/eff_pc_itemdrop_02.nif" );
	}

	if( DRAMATURGYMAN->Init() == false )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	if( TITLEMAN->Init() == false )
	{
		mDataLoadFlag = eData_Error;
		return 0;
	}

	mDataLoadFlag = eData_End;
	return 0;
}

bool cGameResourceManager::IsValidMap( unsigned long mapIdx )
{
	cArrayHashMap* pMap = (cArrayHashMap*)mMapInfoMap.GetAt( mapIdx );
	if( pMap == NULL )
	{
		return false;
	}

	if( pMap->IsEmpty() == true )
	{
		return false;
	}

	return true;
}
