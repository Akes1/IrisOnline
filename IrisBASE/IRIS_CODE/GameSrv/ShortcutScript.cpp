#include "StdAfx.h"
#include "ShortcutScript.h"
#include "Tokenizer.h"

cShortCutScript* cShortCutScript::mpSingleton = 0;

cShortCutScript::cShortCutScript()
{
	mpSingleton = this;

	memset( mShortCut, 0, sizeof(mShortCut) );
}

cShortCutScript::~cShortCutScript()
{
}

bool cShortCutScript::Init()
{
	if( LoadShortCutScript( "./Script/Resource/Default_QuickSlot.txt" ) == false )
	{
		assert( 0 && "failed to load default shortcut script" );
		return false;
	}
	return true;
}

bool cShortCutScript::LoadShortCutScript( const cString& pathName )
{
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load LoadShortCutScript" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		// 클래스 구분 	0 : 공통
		if( tokenizer.GetNext( &str ) == false )
			return false;

		char job = (char)str.ToInt();

		// 인덱스 
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned long idx = (unsigned long)str.ToInt();

		/// 단축키명
		if( tokenizer.GetNext( &str ) == false )
			return false;

		/// 정보 타입
		if( tokenizer.GetNext( &str ) == false )
			return false;

		char type = (char)str.ToInt();

		/// 스킬인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned long skillIdx = (unsigned long)str.ToInt();
	
		char shortcutType = eSHORTCUT_NONE;
		switch( type )
		{
		case 1: shortcutType = eSHORTCUT_COMMU;	break; /// 커뮤니티스킬
		case 2: shortcutType = eSHORTCUT_SKILL;	break; /// 일반 스킬
		default: assert(0);						break;
		}

		if( job > MAX_JOBKIND )
		{
			assert(0);
			return false;
		}

		if( job == 0 )
		{
			// 공통은 전부 삽입
			for( int i = 0; i < MAX_JOBKIND; ++i )
			{
				mShortCut[i][idx].mType				= shortcutType;
				mShortCut[i][idx].mIndex			= skillIdx;
				mShortCut[i][idx].mExData			= 0;
			}
		}
		else if( job > 0 )
		{
			// 직업 특성 정보 삽입
			mShortCut[job-1][idx].mType				= shortcutType;
			mShortCut[job-1][idx].mIndex			= skillIdx;
			mShortCut[job-1][idx].mExData			= 0;
		}
	}
	return true;
}

sShortcut* cShortCutScript::GetShortCut( unsigned long firstjob )
{
	switch( firstjob )
	{		
	case ePLAYER_FIGHTER:
		{
			return mShortCut[0];
		}
		break;
	case ePLAYER_MAGE:
		{
			return mShortCut[1];
		}
		break;
	case ePLAYER_ARMSMAN:
		{
			return mShortCut[2];
		}
		break;
	case ePLAYER_RANGER:
		{
			return mShortCut[3];
		}
		break;
	case ePLAYER_WANDERER:
		{
			return mShortCut[4];
		}
		break;
	case ePLAYER_WARLOCK:
		{
			return mShortCut[5];
		}
		break;
	}

	assert(0);
	return NULL;
}
