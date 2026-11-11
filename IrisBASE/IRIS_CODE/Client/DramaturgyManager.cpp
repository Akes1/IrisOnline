#include "stdafx.h"
#include "DramaturgyManager.h"

#include "Player_Common.h"
#include "SkillScript.h"

#include "Token.h"
#include "Parser.h"
#include "Lexer.h"

cDramaturgyManager* cDramaturgyManager::mpDramaturgyManager = NULL;

cDramaturgyManager::cDramaturgyManager()
{
	mpDramaturgyManager = this;

}

cDramaturgyManager::~cDramaturgyManager()
{
	/// mDramatugyInfoMap Clear
	cSkillDramaMap::cIterator i = mPlayerDramaturgyInfoMap.Begin();
	cSkillDramaMap::cIterator iend = mPlayerDramaturgyInfoMap.End();
	for( ; i != iend; ++i )
	{
		cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)(i->mSecond);

		if( pInfoMap )
		{
			cDramaturgyInfoMap::cIterator k = pInfoMap->Begin();
			cDramaturgyInfoMap::cIterator kend = pInfoMap->End();
			for( ; k != kend; ++k )
			{
				delete (cDramaturgyInfo*)(k->mSecond);
			}
			pInfoMap->Clear();
			delete pInfoMap;
		}
	}
	mPlayerDramaturgyInfoMap.Clear();

	i = mMonsterDramaturgyInfoMap.Begin();
	iend = mMonsterDramaturgyInfoMap.End();
	for( ; i != iend; ++i )
	{
		cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)(i->mSecond);

		if( pInfoMap )
		{
			cDramaturgyInfoMap::cIterator k = pInfoMap->Begin();
			cDramaturgyInfoMap::cIterator kend = pInfoMap->End();
			for( ; k != kend; ++k )
			{
				delete (cDramaturgyInfo*)(k->mSecond);
			}
			pInfoMap->Clear();
			delete pInfoMap;
		}
	}
	mMonsterDramaturgyInfoMap.Clear();

	i = mInfluenceDramaInfoMap.Begin();
	iend = mInfluenceDramaInfoMap.End();
	for( ; i != iend; ++i )
	{
		delete (cDramaturgyInfo*)(i->mSecond);
	}
	mInfluenceDramaInfoMap.Clear();

	i = mActInfluenceDramaInfoMap.Begin();
	iend = mActInfluenceDramaInfoMap.End();
	for( ; i != iend; ++i )
	{
		delete (cDramaturgyInfo*)(i->mSecond);
	}
	mActInfluenceDramaInfoMap.Clear();


	i = mApplyDramaMap.Begin();
	iend = mApplyDramaMap.End();
	for( ; i != iend; ++i )
	{
		cDramaturgyInfo** ppInfo = (cDramaturgyInfo**)(i->mSecond);
		for(unsigned int k=0;k<eAPPLYDRAMA_MAX;k++)
			delete ppInfo[k];

		delete[] ppInfo;
	}
	mApplyDramaMap.Clear();
}

bool cDramaturgyManager::Init()
{
	if( LoadPlayerDramaturgyFileList() == false )
		return false;

	if( LoadMonsterDramaturgyFileList() == false )
		return false ;

	if( LoadApplyDramaturgyFileList() == false )
		return false;

	return true;
}

void cDramaturgyManager::ReLoadDramaturgyFileList()
{
	/// mDramatugyInfoMap Clear
	cSkillDramaMap::cIterator i = mPlayerDramaturgyInfoMap.Begin();
	cSkillDramaMap::cIterator iend = mPlayerDramaturgyInfoMap.End();
	for( ; i != iend; ++i )
	{
		cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)(i->mSecond);

		if( pInfoMap )
		{
			cDramaturgyInfoMap::cIterator k = pInfoMap->Begin();
			cDramaturgyInfoMap::cIterator kend = pInfoMap->End();
			for( ; k != kend; ++k )
			{
				delete (cDramaturgyInfo*)(k->mSecond);
			}

			pInfoMap->Clear();
			delete pInfoMap;
		}
	}
	mPlayerDramaturgyInfoMap.Clear();

	i = mMonsterDramaturgyInfoMap.Begin();
	iend = mMonsterDramaturgyInfoMap.End();
	for( ; i != iend; ++i )
	{
		cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)(i->mSecond);

		if( pInfoMap )
		{
			cDramaturgyInfoMap::cIterator k = pInfoMap->Begin();
			cDramaturgyInfoMap::cIterator kend = pInfoMap->End();
			for( ; k != kend; ++k )
			{
				delete (cDramaturgyInfo*)(k->mSecond);
			}
			pInfoMap->Clear();
			delete pInfoMap;
		}
	}
	mMonsterDramaturgyInfoMap.Clear();

	i = mInfluenceDramaInfoMap.Begin();
	iend = mInfluenceDramaInfoMap.End();
	for( ; i != iend; ++i )
	{
		delete (cDramaturgyInfo*)(i->mSecond);
	}
	mInfluenceDramaInfoMap.Clear();

	i = mActInfluenceDramaInfoMap.Begin();
	iend = mActInfluenceDramaInfoMap.End();
	for( ; i != iend; ++i )
	{
		delete (cDramaturgyInfo*)(i->mSecond);
	}
	mActInfluenceDramaInfoMap.Clear();

	i = mApplyDramaMap.Begin();
	iend = mApplyDramaMap.End();
	for( ; i != iend; ++i )
	{
		cDramaturgyInfo** ppInfo = (cDramaturgyInfo**)(i->mSecond);
		for(unsigned int k=0;k<eAPPLYDRAMA_MAX;k++)
			delete ppInfo[k];

		delete[] ppInfo;
	}
	mApplyDramaMap.Clear();

	if( LoadPlayerDramaturgyFileList() == false )
		return;

	if( LoadMonsterDramaturgyFileList() == false )
		return;

	if( LoadApplyDramaturgyFileList() == false )
		return;

	if( SKILLSCRIPT->InfluenceLoad() == false )
		return;

	if( SKILLSCRIPT->ActInfluenctLoad() == false )
		return;
}

/// 연출 파일 이름 리스트를 읽어 들인다.
bool cDramaturgyManager::LoadPlayerDramaturgyFileList()
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Script/Direct/PlayerDramaFileList.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Direct/PlayerDramaFileList.txt" );

	cDramaturgyInfoMap* pInfoMap = 0;
	unsigned long skillIdx = 0;
	unsigned int weaponState = 0;

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
				skillIdx = (unsigned long)token.ToInt();
				weaponState = (unsigned int)parser.ParseInt();
				assert( weaponState < eWEAPON_STATE_MAX );

				pInfoMap = (cDramaturgyInfoMap*)mPlayerDramaturgyInfoMap.GetAt( skillIdx );
				if( pInfoMap == 0 )
				{
					pInfoMap = new cDramaturgyInfoMap;
					bool check = mPlayerDramaturgyInfoMap.Insert( skillIdx, pInfoMap );
					assert(check);
				}

				for( unsigned int i=0; i<count; ++i )
				{
					cString str = parser.ParseString();

					if( str == "None" )
						continue;

					/// key 생성
					/// key = ((race*gender_max)+gender)*weaponstate_max + weaponstate
					unsigned short key = (unsigned short)(i * eWEAPON_STATE_MAX + weaponState);

					/// 연출 기본 정보는 모두 로딩한다.
					/// 객체 정보는 별도
					cDramaturgyInfo* pInfo = new cDramaturgyInfo( str.Cstr() );
					if( pInfo->LoadFile() == false )
					{
						assert(0);
						delete pInfo;
						continue;
					}

					/// 
					bool check = pInfoMap->Insert( key, pInfo );
					assert(check);
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

bool cDramaturgyManager::LoadMonsterDramaturgyFileList()
{
	cSkillHashMap* totalMap = SKILLSCRIPT->GetMonsterSkillTotalMap();

	cSkillHashMap::cIterator i = totalMap->Begin();
	cSkillHashMap::cIterator end = totalMap->End();

	cDramaturgyInfoMap* pInfoMap = 0;
	for( ;i != end; ++i )
	{
		unsigned long dramaKey = (unsigned long)i->mFirst;
		sMonsterSkillScript* info = (sMonsterSkillScript*)i->mSecond;
		if( info->mDramaFile == "None" )
			continue;

		pInfoMap = (cDramaturgyInfoMap*)mMonsterDramaturgyInfoMap.GetAt( info->mMonsterIdx );
		if( pInfoMap == 0 )
		{
			pInfoMap = new cDramaturgyInfoMap;
			if( mMonsterDramaturgyInfoMap.Insert( info->mMonsterIdx, pInfoMap ) == false )
			{
				assert(0);
				delete pInfoMap;
				return false;
			}
		}

		cDramaturgyInfo* pInfo = new cDramaturgyInfo( info->mDramaFile.Cstr() );
		if( pInfo->LoadFile() == false )
		{
			assert(0);
			delete pInfo;
			return false;
		}

		/// 
		if( pInfoMap->Insert( dramaKey, pInfo ) == false )
		{
			assert(0);
			delete pInfo;
			return false;
		}
	}

	return true;

/*
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Script/Direct/MonsterDramaList.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Direct/MonsterDramaList.txt" );

	unsigned long index;
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
				/// = monster index
				index = (unsigned long)token.ToInt();
				cDramaturgyInfo** pInfoArray = new cDramaturgyInfo*[eMONSTERATTACK_MAX];

				for( unsigned int i=0; i<eMONSTERATTACK_MAX; ++i )
				{
					pInfoArray[i] = 0;

					cString str = parser.ParseString();

					if( str == "None" )
						continue;

					/// 연출 기본 정보는 모두 로딩한다.
					/// 객체 정보는 별도
					cDramaturgyInfo* pInfo = new cDramaturgyInfo( str.Cstr() );
					if( pInfo->LoadFile() == false )
					{
						delete pInfo;
						continue;
					}

					pInfoArray[i] = pInfo;
				}

				if( mMonsterDramaturgyInfoMap.Insert( index, pInfoArray ) == false )
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
	*/
}

bool cDramaturgyManager::LoadApplyDramaturgyFileList()
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Script/Direct/ApplyDramaList.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Direct/ApplyDramaList.txt" );

	unsigned long index;
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
				index = (unsigned long)token.ToInt();
				cDramaturgyInfo** pInfoArray = new cDramaturgyInfo*[eAPPLYDRAMA_MAX];

				for( unsigned int i=0; i<eAPPLYDRAMA_MAX; ++i )
				{
					pInfoArray[i] = 0;

					cString str = parser.ParseString();

					if( str == "None" )
						continue;

					/// 연출 기본 정보는 모두 로딩한다.
					/// 객체 정보는 별도
					cDramaturgyInfo* pInfo = new cDramaturgyInfo( str.Cstr() );
					if( pInfo->LoadFile() == false )
					{
						delete pInfo;
						continue;
					}

					pInfoArray[i] = pInfo;
				}

				if( mApplyDramaMap.Insert( index, pInfoArray ) == false )
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

bool cDramaturgyManager::LoadInfluenceDrama( unsigned long influenceIdx, const cString& fileName, bool loadObject )
{
	if( fileName == "None" )
		return false;
    
	cDramaturgyInfo* pInfo = new cDramaturgyInfo( fileName.Cstr() );
	if( pInfo->LoadFile() == false )
	{
		assert(0);
		delete pInfo;
		return false;
	}

	if( mInfluenceDramaInfoMap.Insert( influenceIdx, pInfo ) == false )
	{
		assert(0);
		delete pInfo;
		return false;
	}

	/// 3D객체 파일을 미리 읽어두기
	if( loadObject == true )
		DirectLoadInfluenceDramaObj( influenceIdx );

	return true;
}

bool cDramaturgyManager::LoadActInfluenceDrama( unsigned long Idx, const cString& fileName, bool loadObject )
{
	if( fileName == "None" )
		return false;

	cDramaturgyInfo* pInfo = new cDramaturgyInfo( fileName.Cstr() );
	if( pInfo->LoadFile() == false )
	{
		assert(0);
		delete pInfo;
		return false;
	}

	if( mActInfluenceDramaInfoMap.Insert( Idx, pInfo ) == false )
	{
		assert(0);
		delete pInfo;
		return false;
	}

	/// 3D객체 파일을 미리 읽어두기
	if( loadObject == true )
		DirectLoadActInfluenceDramaObj( Idx );

	return true;
}

cDramaturgyInfo* cDramaturgyManager::GetPlayerDramaturgyInfo( unsigned long skillIdx, unsigned short weaponstate, unsigned char race, unsigned char gender )
{
	unsigned short weapon = weaponstate;
	if( weaponstate > eWEAPON_STATE_MAX )
		weapon = (unsigned short)(weaponstate - eWEAPON_STATE_SHEILD);

	cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)mPlayerDramaturgyInfoMap.GetAt( skillIdx );
	if( pInfoMap == 0 )
	{
		assert(0);
		return 0;
	}

	/// 정보 획득.
	unsigned short key = (unsigned short)(((race*eGENDER_MAX)+gender)*eWEAPON_STATE_MAX + weapon);
	cDramaturgyInfo* pInfo = (cDramaturgyInfo*)pInfoMap->GetAt( key );
	if( pInfo == 0 )
		return 0;

	return pInfo;
}

cDramaturgyInfo* cDramaturgyManager::GetPlayerDramaturgyInfo( unsigned long skillIdx, unsigned short dramaKey )
{
	cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)mPlayerDramaturgyInfoMap.GetAt( skillIdx );
	if( pInfoMap == 0 )
	{
		assert(0);
		return 0;
	}

	cDramaturgyInfo* pInfo = (cDramaturgyInfo*)pInfoMap->GetAt( dramaKey );
	if( pInfo == 0 )
		return 0;

	return pInfo;
}

cDramaturgyInfo* cDramaturgyManager::GetMonsterDramaturgyInfo( unsigned long monsterIdx, eMONSTERATTACK_TYPE type )
{
//	if( type >= eMONSTERATTACK_MAX )
//	{
//		assert(0);
//		return 0;
//	}

	cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)mMonsterDramaturgyInfoMap.GetAt( monsterIdx );
	if( pInfoMap == 0 )
	{
		assert(0);
		return 0;
	}

	unsigned long dramaKey = monsterIdx * 100 + type;
	cDramaturgyInfo* pInfo = (cDramaturgyInfo*)pInfoMap->GetAt( dramaKey );
	if( pInfo == 0 )
		return 0;

	return pInfo;
/*
	cDramaturgyInfo** pInfoArray = (cDramaturgyInfo**)mMonsterDramaturgyInfoMap.GetAt( monsterIdx );

	if( pInfoArray == 0 )
		return 0;

	return pInfoArray[type];
*/
}

cDramaturgyInfo* cDramaturgyManager::GetPlayerApplyDramaInfo( unsigned char race, unsigned char gender, eAPPLYDRAMA_TYPE type )
{
	if( type >= eAPPLYDRAMA_MAX )
	{
		assert(0);
		return 0;
	}

	unsigned long key = 100001 + (race*eGENDER_MAX)+gender;
	cDramaturgyInfo** pInfoArray = (cDramaturgyInfo**)mApplyDramaMap.GetAt( key );
	if( pInfoArray == 0 )
		return 0;

	return pInfoArray[type];
}

cDramaturgyInfo* cDramaturgyManager::GetMonsterApplyDramaInfo( unsigned long monsterIdx, eAPPLYDRAMA_TYPE type )
{
	if( type >= eAPPLYDRAMA_MAX )
	{
		assert(0);
		return 0;
	}

	cDramaturgyInfo** pInfoArray = (cDramaturgyInfo**)mApplyDramaMap.GetAt( monsterIdx );

	if( pInfoArray == 0 )
		return 0;

	return pInfoArray[type];
}

cDramaturgyInfo* cDramaturgyManager::GetInfluenceDramaInfo( unsigned long influenceIdx )
{
	return (cDramaturgyInfo*)mInfluenceDramaInfoMap.GetAt( influenceIdx );
}

cDramaturgyInfo* cDramaturgyManager::GetActInfluenceDramaInfo( unsigned long actIdx )
{
	return (cDramaturgyInfo*)mActInfluenceDramaInfoMap.GetAt( actIdx );

}

/// 연출정보에 포함된 객체를 읽어 들인다.
void cDramaturgyManager::DirectLoadPlayerDramaObj( unsigned long skillIdx )
{
	for( unsigned short weapon=0;weapon<eWEAPON_STATE_MAX;weapon++ )
	{
		for( unsigned char race=0;race<eRACE_MAX;race++ )
		{
			for( unsigned char gender=0;gender<eGENDER_MAX;gender++ )
			{
				cDramaturgyInfo* pInfo = GetPlayerDramaturgyInfo( skillIdx, weapon, race, gender );
				if( pInfo )
				{
					/// 연출에 필요한 객체 direct Load
					pInfo->LoadDramaObj();
				}
				else
				{
					continue;
				}
			}
		}
	}
}

void cDramaturgyManager::DirectLoadMonsterDramaObj( unsigned long monsterIdx )
{
	cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)mMonsterDramaturgyInfoMap.GetAt( monsterIdx );
	if( pInfoMap == 0 )
	{
		assert(0);
		return;
	}

	cDramaturgyInfoMap::cIterator k = pInfoMap->Begin();
	cDramaturgyInfoMap::cIterator kend = pInfoMap->End();
	for( ; k != kend; ++k )
	{
		cDramaturgyInfo* pInfo = (cDramaturgyInfo*)(k->mSecond);
		pInfo->LoadDramaObj();
	}
/*
	cDramaturgyInfo** pInfoArray = (cDramaturgyInfo**)mMonsterDramaturgyInfoMap.GetAt( monsterIdx );
	if( pInfoArray )
	{
		for( unsigned int i=0; i<eMONSTERATTACK_MAX; i++ )
		{
			if( pInfoArray[i] )
				pInfoArray[i]->LoadDramaObj();
		}
	}
*/
}

void cDramaturgyManager::DirectLoadPlayerApplyDramaObj( unsigned char race, unsigned char gender )
{
	unsigned long key = 100001 + (race*eGENDER_MAX)+gender;
	cDramaturgyInfo** pInfoArray = (cDramaturgyInfo**)mApplyDramaMap.GetAt( key );
	if( pInfoArray )
	{
		for( unsigned int i=0; i<eAPPLYDRAMA_MAX; i++ )
		{
			if( pInfoArray[i] )
				pInfoArray[i]->LoadDramaObj();
		}
	}
}

void cDramaturgyManager::DirectLoadMonsterApplyDramaObj( unsigned long monsterIdx )
{
	cDramaturgyInfo** pInfoArray = (cDramaturgyInfo**)mApplyDramaMap.GetAt( monsterIdx );
	if( pInfoArray )
	{
		for( unsigned int i=0; i<eAPPLYDRAMA_MAX; i++ )
		{
			if( pInfoArray[i] )
				pInfoArray[i]->LoadDramaObj();
		}
	}
}

void cDramaturgyManager::DirectLoadInfluenceDramaObj( unsigned long influenceIdx )
{
	cDramaturgyInfo* pInfo = (cDramaturgyInfo*)mInfluenceDramaInfoMap.GetAt( influenceIdx );
	if( pInfo )
		pInfo->LoadDramaObj();
}

void cDramaturgyManager::DirectLoadActInfluenceDramaObj( unsigned long Idx )
{
	cDramaturgyInfo* pInfo = (cDramaturgyInfo*)mActInfluenceDramaInfoMap.GetAt( Idx );
	if( pInfo )
		pInfo->LoadDramaObj();
}
