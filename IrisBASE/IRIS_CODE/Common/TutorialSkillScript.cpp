#include "gamesrv.h"
#include "StdAfx.h"
#include "TutorialSkillScript.h"
#include "FileLoader.h"
#include "Tokenizer.h"

cTutorialSkillScript* cTutorialSkillScript::mpSingleton = NULL;

cTutorialSkillScript::cTutorialSkillScript()
{
	mpSingleton = this;
}

cTutorialSkillScript::~cTutorialSkillScript()
{
	mpSingleton = 0;
}

void cTutorialSkillScript::Release()
{
	for( unsigned int i = 0; i < mTutorialSkillArr.GetSize(); ++i )
	{
		sTutorialSkill* p = (sTutorialSkill*)mTutorialSkillArr[i];
		SAFE_DELETE( p );
	}
	mTutorialSkillArr.Clear();
}

bool cTutorialSkillScript::Init()
{
	if( LoadScript( "./Script/Resource/TutorialSkillList.txt"  ) == false )
	{
		assert( 0 && "failed to read tutorial skill script" );
		return false;
	}
	return true;
}

bool cTutorialSkillScript::LoadScript( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load tutorial skill list" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		// 스킬 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned long skillIndex = (unsigned long)str.ToInt();

		// 스킬 단계
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned char skillStep = (unsigned char)str.ToInt();

		sTutorialSkill* p = new sTutorialSkill;
		p->skillIndex = skillIndex;
		p->skillStep = skillStep;

		mTutorialSkillArr.PushBack( p );
	}

	return true;
}