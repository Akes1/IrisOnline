#include "stdafx.h"
#include "DramaturgyManager.h"

#include "Player_Common.h"
#include "SkillScript.h"

#include "Token.h"
#include "Parser.h"
#include "Lexer.h"

#include "FileLoader.h"

cDramaturgyManager* cDramaturgyManager::mpDramaturgyManager = NULL;

cDramaturgyManager::cDramaturgyManager()
{
	mpDramaturgyManager = this;

}

cDramaturgyManager::~cDramaturgyManager()
{
	/// mPlayerDramatugyInfoMap Clear
	cSkillDramaMap::cIterator i = mPlayerDramaturgyInfoMap.Begin();
	cSkillDramaMap::cIterator iend = mPlayerDramaturgyInfoMap.End();
	for( ; i != iend; ++i )
	{
		cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)(i->mSecond);
		if( pInfoMap )
		{
			cDramaturgyInfoMap::cIterator j = pInfoMap->Begin();
			cDramaturgyInfoMap::cIterator jend = pInfoMap->End();
			for( ; j != jend; ++j )
			{
				delete (cDramaturgyInfo*)(j->mSecond);
			}
			pInfoMap->Clear();
		}
		delete pInfoMap;
	}
	mPlayerDramaturgyInfoMap.Clear();

	/// mMonsterDramaturgyInfoMap Clear
	i = mMonsterDramaturgyInfoMap.Begin();
	iend = mMonsterDramaturgyInfoMap.End();
	for( ; i != iend; ++i )
	{
		cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)(i->mSecond);
		if( pInfoMap )
		{
			cDramaturgyInfoMap::cIterator j = pInfoMap->Begin();
			cDramaturgyInfoMap::cIterator jend = pInfoMap->End();
			for( ; j != jend; ++j )
			{
				delete (cDramaturgyInfo*)(j->mSecond);
			}
			pInfoMap->Clear();
		}
		delete pInfoMap;
	}
	mMonsterDramaturgyInfoMap.Clear();
}

bool cDramaturgyManager::Init()
{
	if( LoadPlayerDramatugyFileList() == false )
		return false;

	if( LoadMonsterDramaturgyFileList() == false )
		return false;

	return true;
}

/// 연출 파일 이름 리스트를 읽어 들인다.
bool cDramaturgyManager::LoadPlayerDramatugyFileList()
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( "./Script/Resource/PlayerDramaFileList.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Resource/PlayerDramaFileList.txt" );

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


					cDramaturgyInfo* pInfo = new cDramaturgyInfo( str.Cstr() );
					if( pInfo->LoadFile() == false )
					{
//						assert(0);
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
	if( loader.Open( "./Script/Resource/MonsterDramaList.txt", true ) == false )
	{
		return false;
	}

	///
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, "./Script/Resource/MonsterDramaList.txt" );

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

					cDramaturgyInfo* pInfo = new cDramaturgyInfo( str.Cstr() );
					if( pInfo->LoadFile() == false )
					{
						delete pInfo;
						continue;
					}

					pInfoArray[i] = pInfo;
				}

				if( mMonsterDramaturgyInfoMap.Insert( index, pInfoArray) == false )
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

cDramaturgyInfo*	cDramaturgyManager::GetPlayerDramaturgyInfo( unsigned long skillIdx, unsigned short weaponstate, unsigned char race, unsigned char gender, unsigned long* dramakey )
{
	unsigned short weapon = weaponstate;
	if( weaponstate > eWEAPON_STATE_MAX )
		weapon = (unsigned short)(weaponstate - eWEAPON_STATE_SHEILD);

	cDramaturgyInfoMap* pInfoMap = (cDramaturgyInfoMap*)mPlayerDramaturgyInfoMap.GetAt( skillIdx );
	if( pInfoMap == 0 )
		return 0;

	/// 정보 획득
	unsigned short key = (unsigned short)(((race*eGENDER_MAX)+gender)*eWEAPON_STATE_MAX + weapon);
	cDramaturgyInfo* pInfo = (cDramaturgyInfo*)pInfoMap->GetAt( key );

	*dramakey = key;

	return pInfo;
}

cDramaturgyInfo* cDramaturgyManager::GetMonsterDramaturgyInfo( unsigned long monsterIdx, eMONSTERATTACK_TYPE type )
{
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
	if( type >= eMONSTERATTACK_MAX )
	{
		assert(0);
		return 0;
	}

	cDramaturgyInfo** pInfoArray = (cDramaturgyInfo**)mMonsterDramaturgyInfoMap.GetAt( monsterIdx );

	if( pInfoArray == 0 )
		return 0;

	return pInfoArray[type];
*/
}