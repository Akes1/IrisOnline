/* ====================================================================
 * 파    일 :	ObjectManager.h
 * 목    적 :	게임 객체를 관리한다.
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
#include "CommonNetworkMsgStruct.h"
#include "BaseObject_Common.h"
#include "Monster_Common.h"
#include "stage_common.h"
#include "PVP_Common.h"

enum eOBJECTTYPE;

class cHero;
class cPlayer;
//class cDynamicEntity;
class cBaseObject;
class cMonster;
//class cTotem;
class cSceneNode;
class cManagedStaticSceneNode;
class cMapPortalSceneNode;
class cNpc;
//class cLexer;
//class cToken;

class cInputSystem;

struct sMonsterData;
struct sNpcData;

class cObjectManager
{
	friend class cInputSystem;

public:
	static cObjectManager*	mpObjectManager;

public:
	/// No Server용 Client에서 사용
	void AddHeroInfo( unsigned long objectIdx, LPTSTR heroname, char race, char gender );
	void AddHero();
	void AddPlayer( unsigned long objectIdx, char race, char gender, int x, int y );
	//

public:
	cObjectManager();
	virtual ~cObjectManager();

	/// 초기부(시작시 한번)
	bool Init();

	/// 종료부(종료시 한번)
	void Exit();

	/// 초기부(맵이동등 갱신시)
	void Open();

	/// 종료부(맵이동등 갱신시)
	void Close();

	/// select창으로 넘어가는 경우 사용
	void ClearHeroData();

	/// 처리부
	void Process( unsigned long deltaTime, unsigned long accumTime );

	/// 입력에 대한 처리
	bool ProcessLButtonClick( int mouseX, int mouseY );
	bool ProcessLButtonDClick( int mouseX, int mouseY );
	bool ProcessMouseOver( int mouseX, int mouseY );

	bool ProcessRButtonDown( int mouseX, int mouseY );
	bool ProcessRButtonUp( int mouseX, int mouseY );

	/// 주캐릭터의 이동
	void HeroGoto( int mouseX, int mouseY );

	/// Hero 공격 자동.. 070420 wonju
	bool HeroAutoAttack(unsigned long id);

	/// hero 생성
	void AddHeroFromServerMsg( MSGROOT* pMsg );
	/// Player 생성
	void AddPlayerFromServerMsg( MSGROOT* pMsg, bool showEffect = false );

	/// player 제거
	void RemovePlayer( unsigned long objectIdx );

	///
	void MovePlayer( unsigned long objectIdx, float x, float y, long moveSpeed );

	void ActionMovePlayer( MSGROOT* pMsg );

	void AddTotem( unsigned long uIdx, unsigned long totemInfoIdx, NiPoint3 totemPos, float dir );
	void RemoveTotem( unsigned long uIdx );

	void AddNpc( sNpcData* pNpcInfo );
//	void AddNpc(  unsigned long objIdx, sNPCRegen* pNpcInfo );
	void RemoveNpc( unsigned long objectIdx );
	void FadeOutNpcAll();

	/// 061012 PKH 몬스터 추가
	bool		AddMonster( sMonsterData* pMonstInfo, bool isRegen = false );
	/// 061012 PKH 몬스터 객체 제거
	void		RemoveMonster( unsigned long objectIdx );

	///
	void AddGathering( unsigned long uIdx, unsigned long gatheringIdx, NiPoint3 genPos, float dir, unsigned char teamType = ePVPDM_TEAMTYPE_MAX );
	///
	void RemoveGathering( unsigned long idx );

	void AddUserPortal( unsigned long uIdx, unsigned long ownerIdx, unsigned long posScriptIdx, NiPoint3 genPos );
	void RemoveUserPortal( unsigned long uIdx );

	/// 061012 PKH 몬스터 이동 관련
	void		MonsterMoveStop( unsigned long monsterIdx, float posX, float posY );
	void		MonsterGoto( int idx, NiPoint2* moveArray, unsigned long count,
		unsigned int moveSpeed, eMOVE_SPEED_TYPE moveSpeedType = eMOVESPEED_RUN, float range = 0, 
		sObject* pTarget = NULL );

	cBaseObject* GetObject( sObject* pInfo );
	cBaseObject* GetObject( unsigned char type, unsigned long idx );

	/// npc 맵 가져오기 
	const tPointerHashMap<unsigned long, void*>* GetNpcMap();

	///
	inline cHero* GetHero()	{ return mpHero; }

	/// 061123 PKH 플레이어 정보 가져감
	cPlayer*	GetPlayer( unsigned long idx );
	cPlayer*	GetPlayerByName( LPCTSTR name );
	/// 061127 PKH 몬스터 정보를 가져감
	cMonster*	GetMonster( unsigned long idx );


	NiPoint3	GetGatherPos( unsigned long idx );
	cSceneNode*	GetGatherSceneNode( unsigned long idx );

	/// NPC 정보를 획득
	cNpc*		GetNpc( unsigned long idx );

	/// 070108 PKH player 치트
	//bool		PlayerCheat( cLexer* pLexer );

	//float		GetMapSize() { return mMapSize; }
	//void		SetMapSize( float mapSize ) { mMapSize = mapSize; }

	/// 070503 PKH 죽은 몬스터 기록
	void		AddMonsterDie( unsigned long monsterUniqueIdx ) { mMonsterDieArray.PushBack( monsterUniqueIdx ); }
	void		AddRemoveNpc( unsigned long npcIdx ) { mNpcRemoveArray.PushBack(npcIdx); }

	/// 객체에 대한 Pick처리
	unsigned int ObjectPicking( int mouseX, int mouseY, cBaseObject** out );
	void ClearPickObject();

	/// 
	cManagedStaticSceneNode* ManagedStaticPicking( int mouseX, int mouseY );
	cMapPortalSceneNode* MapPotalPicking();

	inline cBaseObject* GetOverObject() { return GetObject( &mOverObject); }
	inline void ResetOverObject() { mOverObject.index = 0; mOverObject.type = eOBJECTTYPE_NONE; }

	/// 080307 PKH 오브젝트 보간거리를 포함한 거리
	float		ObjectSizeRange( cBaseObject* object1, cBaseObject* object2, float range );

	/// 길드 마크 갱신
	void		ChangeGuildMarkRegistDate( unsigned long guildIndex, tm markRegistDate );

	/// 주변 몬스터 선공/비선공 이름 컬러 갱신
	void		UpdateMonsterNameColor();

	void		UpdateHelmetState();

	//
	void		UpdateTutorialImage( eOBJECTTYPE type );
	void		ClearTutorialImage();

private:
	cHero*		mpHero;

	/// 커서상의 객체 정보..
	sObject			mPickObject;
	sObject			mOverObject;

	cBaseObject*	mRbuttonObject;

	struct sOverNode
	{
		unsigned int	type;
		unsigned long	indexByManager;
	};
	sOverNode	mOverNode;

	//float		mMapSize;

	/// player map
//	typedef tPointerHashMap<unsigned long, cPlayer*> cPlayerHashMap;
	typedef tPointerHashMap<unsigned long, void*> cPlayerHashMap;
	cPlayerHashMap*	mpPlayerMap;

	/// monster map
//	typedef tPointerHashMap<unsigned long, cMonster*> cMonsterHashMap;
	typedef tPointerHashMap<unsigned long, void*> cMonsterHashMap;
	cMonsterHashMap*	mpMonsterMap;

	/// npc map
//	typedef tPointerHashMap<unsigned long, cNpc*> cNpcHashMap;
	typedef tPointerHashMap<unsigned long, void*> cNpcHashMap;
	cNpcHashMap*	mpNpcMap;

	typedef tPointerHashMap<unsigned long, void*> cTotemHashMap;
	cTotemHashMap* mpTotemMap;

	typedef tPointerHashMap<unsigned long, unsigned long> cGatheringMap;
	cGatheringMap* mpGatheringMap;

	typedef tPointerHashMap<unsigned long, unsigned long> cFPortalHashMap;
	cFPortalHashMap* mpFPortalMap;

	/// pick시 이벤트 처리용 객체들
	/// 1. 고정적인 이동 포탈


	/// 차폐물
//	tArray<cSceneNode*> mCollidableArray;
	tArray<void*> mCollidableArray;

	/// 061212 PKH 죽은 몬스터를 기록해두는 배열
	typedef tArray<unsigned long> cMonsterDieArray;
	cMonsterDieArray mMonsterDieArray;

	/// 임의 제거되는 npc용
	typedef tArray<unsigned long> cNpcRemoveArray;
	cNpcRemoveArray mNpcRemoveArray;

	typedef tArray<unsigned long> cPortalRemoveArray;
	cPortalRemoveArray mPortalRemoveArray;

};

inline const tPointerHashMap<unsigned long, void*>* cObjectManager::GetNpcMap()
{
	return (tPointerHashMap<unsigned long, void*>*)mpNpcMap;
}

#define OBJECTMANAGER	cObjectManager::mpObjectManager
#define OBJECTMAN cObjectManager::mpObjectManager
#define HERO			cObjectManager::mpObjectManager->GetHero()
