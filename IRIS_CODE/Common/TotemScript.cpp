#include "StdAfx.h"
#include "TotemScript.h"

#include "Totem_Common.h"


#include "Token.h"
#include "Parser.h"
#include "Lexer.h"
#include "FileLoader.h"
#include "Tokenizer.h"


cTotemScript* cTotemScript::mpTotemScript = NULL;



cTotemScript::cTotemScript(void)
{

	/// 싱글톤
	if( mpTotemScript )
	{
		assert(NULL);
	}
	else
	{
		mpTotemScript = this;
	}

	mpTotemMap = new cPointHashMap( 100 );
}



bool cTotemScript::Init( )
{

	/// 토템 정보 로드
	if( !LoadTotemList() )
	{
		assert(NULL);
		return false;
	}

	return true;

}

void cTotemScript::Release()
{
	if( mpTotemMap )
	{
		cPointHashMap::cIterator k = mpTotemMap->Begin();
		cPointHashMap::cIterator kend = mpTotemMap->End();
		for( ; k != kend; ++k )
		{
			delete (sTotemScript*)(k->mSecond);
		}
		mpTotemMap->Clear();

		delete mpTotemMap;
		mpTotemMap = 0;
	}
}

bool cTotemScript::LoadTotemList()
{

	cFileLoader loader;
	cString pathName = "./Script/Resource/SkillList_Trap.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load TotemList.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sTotemScript* pList = NULL;
	while( tokenizer.IsEnd() == false )
	{
		pList = new sTotemScript;

		/// 토템 고유번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mTotemIdx = str.ToInt();

		/// 적용 효과 번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mInfluenceIdx = str.ToInt();

		/// 연출파일명
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mFileName = str;

		/// 종속/비종속 여부
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mDependent = str.ToInt() == 0 ? true : false;

		/// 유지시간
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mContinuanceTime = str.ToInt();		

		/// 적용주기
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mApplyTime = str.ToInt();		

		/// 유지거리
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mAttackerRange = (unsigned short)str.ToInt();

		/// 적용범위1
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mApplyRange1 = (unsigned short)str.ToInt();

		/// 적용범위2
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pList->mApplyRange2 = (unsigned short)str.ToInt();

		/// 해쉬에 몬스터 별로 기록
		if( mpTotemMap->Insert( pList->mTotemIdx, pList ) == false )
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



sTotemScript* cTotemScript::GetTotemInfo( unsigned long totemIdx )
{
	return (sTotemScript*)mpTotemMap->GetAt( totemIdx );
}
