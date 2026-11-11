#include "StdAfx.h"
#include ".\communityscript.h"

#include "./util/FileLoader.h"
#include "./util/Tokenizer.h"

cCommunityScript* cCommunityScript::mpCommunityScript = NULL;


cCommunityScript::cCommunityScript()
{
	mpCommunityScript = this;
}

cCommunityScript::~cCommunityScript() 
{
	Release();
}

void cCommunityScript::Release()
{
	cComHashMap::cIterator iter;

	for(iter = mComSkillMap.Begin() ; iter != mComSkillMap.End() ; ++iter)
	{
		delete (sCommunitySkillInfo*)(*iter).mSecond;
	}
	mComSkillMap.Clear();
}

bool cCommunityScript::Init()
{
	Release();

	/// script load
//#ifdef _CLIENT
	if( CommunitySkillScriptLoad_Client() == false )
	{
		assert(0);
		return false;
	}
//#else
// 	if( CommunitySkillScriptLoad_Server() == false )
// 	{
// 		assert(0);
// 		return false;
// 	}
//#endif

	return true;
}

#ifdef _CLIENT
bool cCommunityScript::CommunitySkillScriptLoad_Client()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/CommunitySkillList.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load CommunitySkillList.txt" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sCommunitySkillInfo* pInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		pInfo = new sCommunitySkillInfo;

		/// 분류 번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mIndex = str.ToInt();

		/// 타입 0:생활, 1:감정
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mType = static_cast<unsigned char>(str.ToInt());

		/// type에 따른 명령어 종류
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mRoleIndex = str.ToInt();

		/// 스킬창 표시 아이콘 번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mIconIndex = str.ToInt();

		/// 채팅 명령어 인덱스 ( interface msg )
		for( unsigned int i=0; i<5; i++ )
		{
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mCommandIndex[i] = str.ToInt();
		}

		/// 툴팁 인덱스 ( tooltip msg )
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mTooltipIndex = str.ToInt();

		/// 아이콘 표시 위치
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mPosX = str.ToInt();

		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mPosY = str.ToInt();

		for( unsigned int i=0;i<eGENDER_MAX * eRACE_MAX; i++ )
		{
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mAnimationTime[i] = str.ToFloat();
		}

		/// 보관소 저장
		if( mComSkillMap.Insert( pInfo->mIndex, pInfo ) == false )
		{
			assert(0);
			goto ERR;
		}
	}
	return true;

ERR:
	delete pInfo;
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parse", MB_OK | MB_ICONERROR );
	return false;
}
#endif

bool cCommunityScript::CommunitySkillScriptLoad_Server()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/CommunitySkillList.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load CommunitySkillList.txt" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sCommunitySkillInfo* pInfo = 0;
	while( tokenizer.IsEnd() == false )
	{
		pInfo = new sCommunitySkillInfo;

		/// 분류 번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mIndex = str.ToInt();

		/// 타입 0:생활, 1:감정
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		pInfo->mType = static_cast<unsigned char>(str.ToInt());

		// skip value
		//////////////////////////////////////////////////////////////////////////
		/// type에 따른 명령어 종류
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		/// 스킬창 표시 아이콘 번호
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		/// 채팅 명령어 인덱스 ( interface msg )
		for( unsigned int i=0; i<5; i++ )
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }

		/// 툴팁 인덱스 ( tooltip msg )
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		/// 아이콘 표시 위치
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
		//////////////////////////////////////////////////////////////////////////

		for( unsigned int i=0;i<eGENDER_MAX * eRACE_MAX; i++ )
		{
			if( tokenizer.GetNext( &str ) == false )	{ goto ERR; }
			pInfo->mAnimationTime[i] = str.ToFloat();
		}

		/// 보관소 저장
		if( mComSkillMap.Insert( pInfo->mIndex, pInfo ) == false )
		{
			assert(0);
			goto ERR;
		}
	}
	return true;

ERR:
	delete pInfo;
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parser", MB_OK | MB_ICONERROR );
	return false;
}

