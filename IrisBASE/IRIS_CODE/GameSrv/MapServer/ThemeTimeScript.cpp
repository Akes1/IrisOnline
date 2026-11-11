#include "gamesrv.h"

#include "StdAfx.h"
#include "ThemeTimeScript.h"
#include "FileLoader.h"
#include "Tokenizer.h"


cThemeTimeScript* cThemeTimeScript::mpThemeTimeScript = NULL;


cThemeTimeScript::cThemeTimeScript(void)
{
	/// 싱글톤
	if( mpThemeTimeScript )
	{
		Verbose->WriteLog( "cThemeTimeScript new error");
	}
	else
	{
		mpThemeTimeScript = this;
	}

}


bool cThemeTimeScript::Init()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/themetime.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load themetime.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 맵번호
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long mapNumber = str.ToInt();	

		/// 초기화 시각
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char mResetClockHour = (unsigned char)str.ToInt();					

		/// 초기화 시간 간격
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char mResetIntervalHour = (unsigned char)str.ToInt();						

		sThemeTime* pList = new sThemeTime;

		pList->mResetClockHour = mResetClockHour;
		pList->mResetIntervalHour = mResetIntervalHour;

		/// 해쉬에 몬스터 별로 기록
		if( mThemeTimeMap.Insert( mapNumber, pList ) == false )
			return false;
	}

	return true;
}


void cThemeTimeScript::Release()
{
	cPHashMap::cIterator b;
	for( b = mThemeTimeMap.Begin() ; b != mThemeTimeMap.End() ; ++b )
	{
		sThemeTime* pInfo = (sThemeTime*)((*b).mSecond);
		SAFE_DELETE( pInfo );
	}
	mThemeTimeMap.Clear();
}
