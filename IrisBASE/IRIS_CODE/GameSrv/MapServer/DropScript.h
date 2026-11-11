/* ==========================================================================
* 파    일 : DropScript.h
* 목    적 : 드롭에 관련된 스크립트 파일을 읽어 온다.
* 작 성 자 : 박경희
* 작 성 일 : 2007-01-12
* 주의사항 :
*===========================================================================*/
#pragma once

struct sDropMoneyScript;
struct sDropClassScript;
struct sDropLevelTypeScript;
struct sDropItemListScript;
struct sPartyExp;

const int DROP_MONSTER_TYPE_UNIT = 100000;

class cDropScript
{

public:
	cDropScript(void);
	~cDropScript(void){};

	bool				Init( bool dbInsertDataOnly = false );
	void				Release();

	/// 몬스터
	sDropMoneyScript*	GetDropMonMoney( unsigned long monsterIdx )	{ return (sDropMoneyScript*)mDropMonMoney.GetAt( monsterIdx ); }
	tArray<void*>*		GetDropMonClass( unsigned long monsterIdx )	{ return (tArray<void*>*)mDropMonClass.GetAt( monsterIdx ); }
	tArray<void*>*		GetDropMonLevelType( unsigned char monsterLevel, unsigned char type );
	tArray<void*>*		GetDropMonQuest( unsigned long monsterClassIdx, unsigned long questIdx );
	sDropItemListScript*	GetDropMonItem( unsigned long dropItemGroup )	{ return (sDropItemListScript*)mDropMonItem.GetAt( dropItemGroup ); }
	unsigned long		GetDropMonCantTime( unsigned long itemIdx );
	void*				GetItemDropLimit()	{ return &mDropMonCantTime; }

	/// 채집
	sDropMoneyScript*	GetDropGatherMoney( unsigned long gatherIdx )	{ return (sDropMoneyScript*)mDropGatherMoney.GetAt( gatherIdx ); }
	tArray<void*>*		GetDropGatherClass( unsigned long gatherIdx )	{ return (tArray<void*>*)mDropGatherClass.GetAt( gatherIdx ); }
	tArray<void*>*		GetDropGatherQuest( unsigned long classIdx, unsigned long questIdx );
	sDropItemListScript*	GetDropGatherItem( unsigned long dropItemGroup )	{ return (sDropItemListScript*)mDropGatherItem.GetAt( dropItemGroup ); }
	unsigned long		GetDropGatherCantTime( unsigned long itemIdx );


	/// 레벨차 패널티
	float				GetDropExpIncrease( bool isTheme, char levelDifference );
	float				GetDropIncrease( bool isTheme, char levelDifference );

	/// 파티 경험치
	float				GetPartyCalcInfo( unsigned long idx );


private:

	/// 몬스터
	bool				LoadDropMonMoney();			/// money 정보 로드
	bool				LoadDropMonClass();			/// class별 정보 로드	
	bool				LoadDropMonLevelType();		/// level/type별 정보 로드
	bool				LoadDropMonQuest();			/// 퀘스트별 정보 로드
	bool				LoadDropMonItemGroup();		/// 드랍 아이템 그룹 정보 로드
	bool				LoadDropMonItemCantTime();	/// 드랍 아이템 시간제약

	/// 채집
	bool				LoadDropGatherMoney();			/// money 정보 로드
	bool				LoadDropGatherClass();			/// class별 정보 로드	
	bool				LoadDropGatherQuest();			/// 퀘스트별 정보 로드
	bool				LoadDropGatherItemGroup();		/// 드랍 아이템 그룹 정보 로드
	bool				LoadDropGatherItemCantTime();	/// 드랍 아이템 시간제약


	/// 경험치 패널티
	bool				LoadDropExpIncrease();

	bool				LoadDropIncreadse();


	/// 파티 경험치
	bool				LoadPartyCalcInfo();

public:

	static cDropScript* mpDropScript;

private:

	typedef tPointerHashMap<unsigned long, void*> cPointrHashMap;
	typedef tHashMap<unsigned long, float> cHashMap;
	typedef tHashMap<unsigned long, unsigned long> cLongHashMap;
	typedef tArray<void*> cAry;

	/// 몬스터
	/// <unsigned long monsterIdx, sDropMoneyScript* pMoney>
	cPointrHashMap		mDropMonMoney;			/// money 드랍 정보
	/// <unsigned long monsterIdx, cAry<sDropClassScript*> pDropMonsterAry>
	cPointrHashMap		mDropMonClass;			/// 몬스터 종류별 드랍 정보(일반드랍)
	/// <unsigned char type * DROP_MONSTER_TYPE_UNIT + unsigned char monsterLevel, cAry<sDropLevelTypeScript*> pDropLevelType>
	cPointrHashMap		mDropMonLevelType;		/// 몬스터 레벨/분류별 드랍 정보(월드드랍)
	/// <sDropLevelTypeScript*>
	cAry				mShutDownDeleteAry;		/// 시스템 shutdown 시 sDropLevelTypeScript*를 중복 delete 를 하지않기위해 만듬
	/// <unsigned long monItemGroupIdx, sDropItemListScript* pItemGroup>
	cPointrHashMap		mDropMonItem;			/// 드랍 아이템 목록
	/// <unsigned long monItemIdx, unsigned long cantDropTime>
	cLongHashMap		mDropMonCantTime;			/// 드랍 시간 제한

	/// 채집
	/// <unsigned long gatheringIdx, sDropMoneyScript* pMoney>
	cPointrHashMap		mDropGatherMoney;		/// money 드랍 정보
	/// <unsigned long gatheringIdx, cAry<sDropClassScript*> pDropGathering>
	cPointrHashMap		mDropGatherClass;		/// 채집 종류별 드랍 정보
	/// <unsigned long gatheringItemGroupIdx, sDropItemListScript* pItemGroup>
	cPointrHashMap		mDropGatherItem;		/// 드랍 아이템 목록
	/// <unsigned long gatheringItemitemIdx, unsigned long cantDropTime>
	cLongHashMap		mDropGatherCantTime;	/// 드랍 시간 제한

	/// 일반 필드 경험치 패널티
	typedef tPointerHashMap<char, float> cDropExpIncrease;
	cDropExpIncrease	mNormalDropExpIncreaseMap;

	char				mNormalScriptMinLevelDifferent;	/// 스크립트에 정의된 최소 레벨차
	char				mNormalScriptMaxLevelDifferent;	/// 스크립트에 정의된 최대 레벨차

	/// 테마 경험치 패널티
	cDropExpIncrease	mThemeDropExpIncreaseMap;

	char				mThemeScriptMinLevelDifferent;	/// 스크립트에 정의된 최소 레벨차
	char				mThemeScriptMaxLevelDifferent;	/// 스크립트에 정의된 최대 레벨차


	/// 일반/테마 아이템 드랍 패널티
	typedef tPointerHashMap<char, float> cDropIncrease;
	cDropIncrease		mNormalDropIncreaseMap;
	char				mNormalDropMinLevelDifferent;

	cDropIncrease		mThemeDropIncreaseMap;
	char				mThemeDropMinLevelDifferent;

	/// 파티 경험치
	cHashMap			mPartyCalcMap;
};

#define DROPSCRIPT cDropScript::mpDropScript
