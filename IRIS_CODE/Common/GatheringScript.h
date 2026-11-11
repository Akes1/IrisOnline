/* ==========================================================================
* 파    일 : GatheringScript.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-12-11
* 주의사항 :
*===========================================================================*/
#pragma once

struct sGatheringList;
struct sGatheringRegenHeader;

class cGatheringScript
{
public:

	static cGatheringScript*		mpGatheringScript;

public:

	/// 생성 소멸
	cGatheringScript(void);
	~cGatheringScript(void) {}

	bool						Init();
	bool						StageInit( unsigned short mapNumber, unsigned char levelMode );
	void						Release();

	bool						LoadGatheringList();
	sGatheringList*				GetGatheringInfo( unsigned long gatheringIdx );

	bool						LoadGatheringRegen( unsigned short mapnum, unsigned char levelMode,
		tPointerHashMap<unsigned long, void*>* pGatheringRegenMap );
	const sGatheringRegenHeader*		GetGatheringRegen( unsigned char levelMode, unsigned long regenIdx );

	/// 리젠 리스트 해쉬맵 포인터를 넘김
	tPointerHashMap<unsigned long, void*>* GetRegenGatheringMap( unsigned char levelMode ) { return (tPointerHashMap<unsigned long, void*>*)mGatheringRegenMap.GetAt( levelMode ); }

private:

	typedef tArray<unsigned long> cArray;
	typedef tArray<void*> cAryVoid;
	typedef tPointerHashSet<unsigned long> cHashSet;
	typedef tPointerHashMap<unsigned long, void*> cPointHashMap;

	/// 몬스터 리스트 해쉬맵
	//	typedef tPointerHashMap<unsigned long, sGatheringList*> cGatheringMap;
	cPointHashMap				mGatheringMap;

	/// 리젠 몬스터 해쉬맵
	//	typedef tPointerHashMap<unsigned long, sGatheringRegenHeader*> cGatheringRegenMap;
	cPointHashMap				mGatheringRegenMap;


};

#define GATHERINGSCRIPT cGatheringScript::mpGatheringScript
