#pragma once

#include "Stage_Common.h"
#include "Player_Common.h"
#include "Monster_Common.h"
#include "skill_Common.h"

#include "..\Client\ItemDefine.h"


struct sMapInfo
{
	unsigned long mapNum;
	unsigned long folderIdx;

	unsigned char mapMode;

	unsigned char minLevel;
	unsigned char maxLevel;

	unsigned char maxPlayer;

	unsigned long itemIdx;
};

enum
{
	/// 게임파일을 읽을때 사용할 토큰..
	eTOKEN_JOBINFO = 100,
	eTOKEN_JOBTREE,
	eTOKEN_HAIR,
	eTOKEN_HAIRCOLOR,
};

class cGameFileLexer : public cLexer
{
public:
	cGameFileLexer( const char* pbuffer, unsigned int size );
};

class cGameResourceManager
{
	static cGameResourceManager* mpSingleton;

public:
	cGameResourceManager();
	~cGameResourceManager();

	static cGameResourceManager* GetSingleton();


	bool Init();

public:

	unsigned int GetMapFolderIdx( unsigned long mapIdx, unsigned char mapMode );

	sStageChangePos*	GetPosScriptInfo( unsigned long idx );

	tPointerArray<void*>*	GetMapChangeStarterArr( unsigned short mapNum );
	tPointerArray<void*>*	GetMapChangeDestArr( unsigned short mapNum );

	unsigned short GetStageChangeMap( unsigned long posIdx );

	unsigned int GetHairCount() { return mHairCnt; }
	unsigned int GetHairColorCount() { return mHairColorCnt; }

	unsigned int GetFaceCount() { return mFaceCnt; }

	sMonsterScript*		GetMonsterListInfo( unsigned long monsterIdx );

	sPlayerSkillBaseInfo*	GetPlayerSkillInfo( unsigned long skillNumber );

protected:
	bool LoadMapInfo();

	bool StageChangePosScriptLoad();

	bool LoadCharacterBaseHairFileInfo();
	bool LoadHair( cParser& parser );
	bool LoadHairColor( cParser& parser );

	bool LoadFaceEmotionInfo();

	bool LoadMonsterList();

	bool PlayerSkillScriptLoad();
	bool PlayerSkillStepLoad();

protected:
	typedef tPointerHashMap<unsigned long, void*> cPointerHashMap;

	cPointerHashMap mMapInfoMap;

	cPointerHashMap mStageChangePosMap;
	tPointerArray<void*> mReturnArray;

	cPointerHashMap mMonsterMap;

	cPointerHashMap mPlayerSkillInfoMap;


	unsigned int mHairCnt;
	unsigned int mHairColorCnt;

	unsigned int mFaceCnt;
};

inline
cGameResourceManager* cGameResourceManager::GetSingleton()
{
	return mpSingleton;
}

#define GAMERESOURCEMAN	cGameResourceManager::GetSingleton()