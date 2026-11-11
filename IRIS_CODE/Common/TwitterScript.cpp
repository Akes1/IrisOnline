#include "stdafx.h"
#include "TwitterScript.h"
#include "Tokenizer.h"


cTwitterScript* cTwitterScript::mpTwitterScript = NULL;


cTwitterScript::cTwitterScript()
{
	mpTwitterScript = this;
}

cTwitterScript::~cTwitterScript()
{
	mEventMonsterSet.Clear();
}

bool cTwitterScript::Init()
{
	if( LoadTwitterEventList() == false )
	{
		assert(0);
		return false;
	}

	return true;
}

bool cTwitterScript::LoadTwitterEventList()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/twitter_automsg.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load twitter_automsg.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 종류 Index( Quest, Monster )
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long monsterIdx = str.ToInt();

		/// 해쉬에 기록
		if( mEventMonsterSet.Insert( monsterIdx ) == false )
		{
			assert(0);
			return false;
		}	
	}

	return true;
}

bool cTwitterScript::IsMonsterEvnet( unsigned long monsterIdx )
{
	cMonsterSet::cIterator i = mEventMonsterSet.Find( monsterIdx );

	if( i != mEventMonsterSet.End() )
	{
		return true;
	}

	return false;

}
