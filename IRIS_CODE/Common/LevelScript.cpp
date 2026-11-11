#include "gamesrv.h"

#include "StdAfx.h"
#include "levelscript.h"
#include "Tokenizer.h"


cLevelScript* cLevelScript::mpLevelScript = NULL;



cLevelScript::cLevelScript(void)
{
	/// 싱글톤
	if( mpLevelScript )
	{
#ifndef _CLIENT
		Verbose->WriteLog( "LevelScript new error");
#endif
	}
	else
	{
		mpLevelScript = this;
	}

	mMaxLevel = 0;
	mMaxSkill = 0;
}



cLevelScript::~cLevelScript(void)
{
}



bool cLevelScript::Init()
{

	mpExpTableMap = new cHashMap( 100 );
	mpSxpTableMap = new cHashMap( 100 );

	bool check = true;

	/// 경험치 테이블 로드
	if( !LoadExpTable() )
	{
#ifndef _CLIENT
		Verbose->WriteLog("ASSERT-070515-LoadExpTable ERROR");
#endif
		assert(NULL);
		MessageBox(NULL,_T("LoadExpTable ERROR"),_T("ERROR"),MB_OK);
		check = false;
	}

	/// 스킬경험치 테이블 로드
	if( !LoadSxpTable() )
	{
#ifndef _CLIENT
		Verbose->WriteLog("ASSERT-070515-LoadSxpTable ERROR");
#endif
		assert(NULL);
		MessageBox(NULL,_T("LoadSxpTable ERROR"),_T("ERROR"),MB_OK);
		check = false;
	}

	return check;

}



void cLevelScript::Release()
{

	if( mpExpTableMap )
	{
		sExpTable* pInfo = NULL;
		cHashMap::cIterator i = mpExpTableMap->Begin();
		cHashMap::cIterator end = mpExpTableMap->End();
		for( ; i != end; ++i )
		{
			pInfo = (sExpTable*)((*i).mSecond);
			if( pInfo )
			{
				delete pInfo;
			}
		}
		mpExpTableMap->Clear();
		SAFE_DELETE( mpExpTableMap );
	}

	if( mpSxpTableMap )
	{
		sSxpTable* pInfo = NULL;
		cHashMap::cIterator i = mpSxpTableMap->Begin();
		cHashMap::cIterator end = mpSxpTableMap->End();
		for( ; i != end; ++i )
		{
			pInfo = (sSxpTable*)((*i).mSecond);
			if( pInfo )
			{
				delete pInfo;
			}
		}
		mpSxpTableMap->Clear();
		SAFE_DELETE( mpSxpTableMap );
	}	
}



bool cLevelScript::LoadExpTable()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/ExpTable.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load ExpTable.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sExpTable* pList = NULL;
	while( tokenizer.IsEnd() == false )
	{
		pList = new sExpTable;

		/// 레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mLevel = (unsigned char)str.ToInt();

		/// 최대 레벨 기록
		if( mMaxLevel < pList->mLevel )
			mMaxLevel = pList->mLevel;

		/// 경험치
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mExp = str.ToInt();

		/// 보관소 저장
		if( mpExpTableMap->Insert( pList->mLevel, pList ) == false )
		{
			assert(0);
			goto ERR;
		}

	}
	return true;

ERR:
	delete pList;
	return false;
}



sExpTable* cLevelScript::GetExpTable( unsigned char level )
{
	sExpTable*	pExpTable;
	pExpTable = (sExpTable*)mpExpTableMap->GetAt( level );

	if( !pExpTable )
		return NULL;

	return pExpTable;
}



bool cLevelScript::LoadSxpTable()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/SxpTable.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load SxpTable.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sSxpTable* pList = NULL;
	while( tokenizer.IsEnd() == false )
	{
		pList = new sSxpTable;

		/// 레벨
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mLevel = (unsigned char)str.ToInt();

		/// 최대 레벨 기록
		if( mMaxSkill < pList->mLevel )
			mMaxSkill = pList->mLevel;

		/// 스킬
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mSxp = str.ToInt();

		/// 보관소 저장
		if( mpSxpTableMap->Insert( pList->mLevel, pList ) == false )
		{
			assert(0);
			goto ERR;
		}

	}
	return true;

ERR:
	delete pList;
	return false;
}



sSxpTable* cLevelScript::GetSxpTable( unsigned char level )
{
	sSxpTable*	pSxpTable;
	pSxpTable = (sSxpTable*)mpSxpTableMap->GetAt( level );

	if( !pSxpTable )
		return NULL;

	return pSxpTable;
}
