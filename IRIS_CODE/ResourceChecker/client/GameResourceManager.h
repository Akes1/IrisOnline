/* ====================================================================
 * 파    일 :	GameResourceManager.h
 * 목    적 :	게임 리소스(스크립트 파일등)파일 관리
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.06
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		2006.06     이법석	    생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once

#include "Player_Common.h"

class cUIText;
class cUIFuncText;
class cParser;
class cStatusCalc_Client;
class cDamageCalc;
class cStatusScript;
class cMonsterScript;
class cNPCScript;
class cLevelScript;
class cSkillScript;
class cCommunityScript;
class cStageScript;
class cTotemScript;
class cPvPScript;
class cGatheringScript;
class cVehicleScript;
class cMakeSkillScript;
class cPostScipt;

struct sMonsterScript;
struct sRegenMonsterScript;

class cBaseObject;
class cMonster;

struct sTextItem;

struct sMapInfo
{
	unsigned long mapNum;
	unsigned char mapMode;
	unsigned long folderIdx;

	bool useTile;

	cString	textureName;

	cString bgmName;
	unsigned long bgmWaitTime;
};

struct sFaceEmoteInfo
{
	unsigned long	index;				/// list index
	unsigned char	type;				/// 0:base, 1:wink, 2:emote

	cString			textureName[eRACE_MAX*eGENDER_MAX];
};

struct sJobInfo
{
	unsigned long jobNameIndex;
	unsigned long jobTargetIconIndex;
};

enum
{
	eData_Start = 0,
	eData_End,
	eData_Error,
};

class cGameResourceManager
{
public:
	static cGameResourceManager* mpGameResourceManager;

	cGameResourceManager();
	~cGameResourceManager();

	void Exit();

	/// 초기화
	bool Init();

	bool InitLogin();
	bool InitServer();

	DWORD	IntroLoadThread();
	DWORD	LoginLoadThread();
	DWORD	ServerLoadThread();

	LPCTSTR GetMonsterName( unsigned int nameIdx );
	LPCTSTR GetGatheringName( unsigned int nameIdx );

	unsigned long	GetMapFolderIdx( unsigned long mapIdx, unsigned char mapMode );
	tPointerHashMap<unsigned long, void*>* GetMapInfo(){ return &mMapInfoMap; }
	bool		IsMapUseTile( unsigned int mapIdx, unsigned char mapMode );
	const char* GetModelFileName( unsigned long modelIndex );
	void		GetMapMode( unsigned long mapIdx, tArray<unsigned char>* modeArr );
	LPCTSTR		GetMapName( unsigned int nameIdx );
	LPCTSTR		GetMapLoadTexture( unsigned int mapIdx, unsigned char mapMode );

protected:
	/// Game에 사용되는 고정 텍스트를 로딩한다.
	bool LoadGameText();

	bool LoadPlayerJob();
	bool LoadPlayerJobInfo( cParser& parser );
	bool LoadPlayerJobTree( cParser& parser, unsigned int depth );

	/// Skill 관련..
	/// 연출 파일도 같이 로드된다.
	//bool LoadSkillInfo();

	/// 모델정보 및 캐릭터 기본 파일 정보..
	/// player, npc, monster의 모델파일정보 kfm파일
	bool LoadModelFileInfo();

	/// 캐릭터의 기본 외형정보 nif파일..
	bool LoadCharacterBaseFileInfo();

	/// 캐릭터의 헤어 정보 (모델파일 / 텍스쳐파일)
	bool LoadCharacterBaseHairFileInfo();
	bool LoadHair( cParser& parser );
	bool LoadHairColor( cParser& parser );

	bool LoadMapInfo();
	bool LoadFaceEmotionInfo();

	void AddRaceTree( unsigned char race, unsigned long jobIdx );

	bool LoadTarotDramaPattern();

	/// 참조 데이터를 재로딩 할경우에 사용하려고 만들어 본다.
	void RemovePlayerJobInfo();
	void RemoveAllModelFileName();
	void RemoveAllCharacterBaseFileInfo();


protected:

	HANDLE			mDataLoadThread;
	unsigned int	mDataLoadFlag;

	//////////////////////////////////////////////////////////////////////////
	// text 문서
	cUIText*	mpGameText;
	cUIText*	mpCategoryName;

	cUIText*	mpItemToolTipText;

	cUIText*	mpTarotCardText;
	cUIText*	mpCardBookText;

	cUIText*	mpSkillNameText;
	cUIText*	mpSkillToolTipText;

	cUIText*	mpNpcNameText;
	cUIText*	mpMonsterNameText;
	cUIText*	mpGatheringNameText;

	cUIText*	mpQuestTitleText;

	cUIText*	mpMapNameText;

	cUIText*	mpEmotionFilterText;

	/// 호칭관련 텍스트
	cUIText*	mpTitleText;

	/// 팁 관련 텍스트
	cUIText*	mpTipText;

	/// npc 타롯 관련 텍스트
	cUIText*	mpNpcTarotText;


	/// 
	cUIFuncText*	mpMonsterNote;
	sTextItem*		mTextItem;

	//////////////////////////////////////////////////////////////////////////
	// 각종 게임 정보를 위한 자료구조들..
	typedef tPointerHashMap<unsigned long, void*>	cArrayHashMap;
	typedef tArray<void*>							cFileNameArray;
	typedef tArray<unsigned long>					cJobIndexArray;

	/// 직업 정보
	cArrayHashMap			mJobInfoMap;
	/// 직업을 key로 하는 전직 직업들의 리스트를 준다.
	cArrayHashMap			mJobTree;	
	/// 종족을 key로 하는 모든 직업들의 리스트를 준다.
	cArrayHashMap			mRaceTree;

	/// Item에 연결된 3D데이터파일명..
	//cArrayHashMap			mItemFileMap;

	/// kfm파일 ( NiString 사용 )
	cArrayHashMap			mModelFileNameMap;

	/// 머리 종류들
	cArrayHashMap			mDefaultHairFileNameMap;

	/// 머리 색깔( 텍스쳐 파일 )
	cArrayHashMap			mHairColorTextureMap;
	unsigned int			mHairColorMax;

	/// 얼굴 종류들
	cArrayHashMap			mDefaultFaceFileNameMap;

	/// 인덱스 -> 상의 하의 손 발 각각 1개의 정보를 갖는다.
	cArrayHashMap			mDefaultWearFileNameMap;

	cArrayHashMap			mMapInfoMap;

	cArrayHashMap			mTarotDramaPattern;

	cArrayHashMap			mFaceEmotionMap;
	unsigned int			mBaseFaceCount;

	///
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	/// Loading 프로세스를 위한 자료구조들..(로딩해야할것들..)

	/// 맵용 텍스쳐 파일들
	NiTPrimitiveQueue<char*>*		mpMapTextureFileQueue;

	/// 객체 파일들
	NiTPrimitiveQueue<char*>*		mpObjectFileQueue;

	/// 모델 파일들
	NiTPrimitiveQueue<char*>*		mpModelFileQueue;

	/// 사운드 파일들
	NiTPrimitiveQueue<char*>*		mpSoundFileQueue;

	///
	//////////////////////////////////////////////////////////////////////////


	/// 070115 PKH 데미지 계산기
	//cStatusCalc_Client*	mpStatusCalc;
	cStatusScript*		mpStatusScript;

	/// 070130 PKH 몬스터
	cMonsterScript*		mpMonsterScript;

	/// NPC
	cNPCScript*			mpNpcScript;

	/// 070515 PKH 레벨 테이블
	cLevelScript*		mpLevelScript;

	/// 070615 PKH 
	cSkillScript*		mpSkillScript;

	cMakeSkillScript*	mpMakeSkillScript;

	///
	cCommunityScript*	mpCommunityScript;

	/// 
	cStageScript*		mpStageScript;

	///
	cTotemScript*		mpTotemScript;

	///
	cPvPScript*			mpPVPScript;

	/// 채집 스크립트
	cGatheringScript*	mpGatheringScript;

	/// 
	cVehicleScript*		mpVehicleScript;
};

#define GAMERESOURCEMAN	cGameResourceManager::mpGameResourceManager
