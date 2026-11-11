/* ====================================================================
 * 파    일 :	
 * 목    적 :	
 * 작 성 자 :	정성대
 * 작 성 일 :	2006.08.29
 *
 * 추가내용 :
 *		추가날짜	작업자	추가내용
 *		2006.08.29	정성대	생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once
#include "BaseStage.h"
#include "FileLoader.h"
#include "Skill_Common.h"

struct MSG_MONSTER_INFO;
class cLoadUIManager;

class cLoadStage : public cBaseStage
{
public:
	cLoadStage();
	virtual ~cLoadStage();

	enum eLOADSTATE
	{
		eLOADSTATE_START = 0,

		eLOADSTATE_TERRAIN_HEADER,
		eLOADSTATE_TERRAIN_NODE,

		eLOADSTATE_NAVIMESH_HEADER = 10,
		eLOADSTATE_NAVIMESH_NODE,

		eLOADSTATE_NAVIFIELD = 20,

		eLOADSTATE_SCENE_HEADER,
		eLOADSTATE_SCENE_NODE,

		eLOADSTATE_SOUND_HEADER = 100,
		eLOADSTATE_SOUND_NODE,

		eLOADSTATE_MAP_PORTAL = 140,

		eLOADSTATE_MONSTERDATA = 151,

		eLOADSTATE_NPCDATA = 160,

		eLOADSTATE_HEROITEM = 170,

		eLOADSTATE_BASESKILL = 180,

		eLOADSTATE_HEROSKILL = 190,

		eLOADSTATE_GAMEINFO = 200,
		eLOADSTATE_END,

		eLOADSTATE_NONE = 240,

		eLOADTATE_HEROBASEINFO = 1000,
	};

	virtual void Init( cStageManager* pStageManager );
	virtual void InitStage();
	virtual void Exit();
	virtual void Open();
	virtual void Close();
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	virtual void NetworkMsgParser( unsigned long connectionindex, char* pMsg, unsigned long length );

	virtual void MsgBoxEventParser( unsigned int event, unsigned int result, void* exData );

	unsigned int GetCurrentState() { return (unsigned int)mLoadState; }

protected:

	void Msg_CharacterGameinAck( MSGROOT* pMsg );

	void ChangeLoadState( eLOADSTATE state );
	void ChangeGaugeState( unsigned int state );

	/// load process
	void LoadFaile( unsigned long error );
	void LoadTimeFaile( unsigned long error );
	bool BeginSceneLoad( unsigned int mapIdx, unsigned char mapMode );
	bool BeginSoundSceneLoad( unsigned int mapIdx, unsigned char mapMode );

protected:
	/// monster model list
	unsigned int	mMonsterClassCount;
	unsigned long	mMonsterClassIdx[50];

	/// npc model list
	unsigned int	mNpcClassCount;
	unsigned long	mNpcClassIdx[50];

	/// skill have list
	unsigned int	mSkillCount;
	unsigned int	mInfluenceCount;
	unsigned int	mGuardInfluenceCount;

	/// 스킬 / 효과
	sHaveSkill			mSkill[200];
	sInfluenceClient	mInfluence[50];
	sInfGuardList		mGuardInfluence[20];

protected:

	unsigned int	mCount;

	eLOADSTATE		mLoadState;
	sLoadFlag		mLoadFlag;

	unsigned int	mGageStateCount;

	/// 로드 스테이지 
	cLoadUIManager* mpLoadUIManager;

	/// file
	cFileLoader		mFileLoader;

	float			mGoPercent;

	unsigned long	mCheckTime;
};
