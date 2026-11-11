#pragma once

#include "ModeInfo.h"

//typedef tPointerList<void*> cModeInfoList;
typedef tPointerArray<void*> cModeInfoArr;

///
class cMode
{
public:
	cMode();
	virtual ~cMode();

	///
	inline unsigned long GetModeIndex() { return mModeIndex; }
	void SetModeIndex( unsigned long modeIdx )	{ mModeIndex = modeIdx; }

	///
	inline cModeInfoArr* GetInitModeInfoArr()			{ return &mInitModeInfoArr; }
	inline cModeInfoArr* GetProcessModeInfoArr()		{ return &mProcessModeInfoArr; }
	inline cModeInfoArr* GetConditionModeInfoArr()	{ return &mConditionModeInfoArr; }
	inline cModeInfoArr* GetCheckSkillModeInfoArr()	{ return &mCheckSkillModeInfoArr; }
	inline cModeInfoArr* GetImmuneModeInfoArr()	{ return &mCheckImmuneModeInfoArr; }

	///
	void PushInitData( sModeInfoBase* pBase, bool bProcess = false );
	void PushConditionData( sModeInfoBase* pBase );
	void PushCheckSkill( sModeInfoBase* pBase );
	void PushImmune( sModeInfoBase* pBase );

protected:
	unsigned long mModeIndex;

	/// init info
	cModeInfoArr	mInitModeInfoArr;

	/// server
	/// process list
	cModeInfoArr	mProcessModeInfoArr;

	/// mode change check list
	cModeInfoArr	mConditionModeInfoArr;

	/// use skill check list
	cModeInfoArr	mCheckSkillModeInfoArr;

	/// immunity check list
	cModeInfoArr	mCheckImmuneModeInfoArr;

};


//////////////////////////////////////////////////////////////////////////
///
class cParser;

class cModeAgent
{
public:
	cModeAgent();
	virtual ~cModeAgent();

	bool LoadFile( unsigned long monsterIdx );

	inline cMode* GetMode( unsigned long modeIdx ) { return (cMode*)mModeMap.GetAt( modeIdx); }

#ifdef _CLIENT
	inline sModeObj* GetModeObject( unsigned short modeObjIdx ) { return (sModeObj*)mModeObjMap.GetAt( modeObjIdx); }
#else
	inline tArray<unsigned long>* GetDeathDoor()	{ return (tArray<unsigned long>*)&mDeathDoorAry; }
	inline tPointerArray<void*>* GetDeathMonster()	{ return &mDeathMonsterAry; }
	inline tArray<unsigned long>* GetTeleportPortal()	{ return (tArray<unsigned long>*)&mTeleportPortalAry; }
#endif

protected:
	bool LoadHeader( cParser& parser );
	bool LoadMode( cMode* mode, cParser& parser, unsigned long *orderNumMon, unsigned long *orderNumTotem );

	unsigned int GetLinkIndex( cString str );

protected:

	typedef tArray<unsigned long> cAry;
	typedef tPointerArray<void*> cPAry;

#ifdef _CLIENT
	/// 사용 객체 정보
	typedef tPointerHashMap<unsigned short,void*> cModeObjMap;
	cModeObjMap	mModeObjMap;
#else
	/// <unsigned long deathDoorIdx>
	cAry		mDeathDoorAry;

	/// <sModeDeathMonster* pDeathMonster>
	cPAry		mDeathMonsterAry;

	/// <unsigned long mapChangePosIdx>
	cAry		mTeleportPortalAry;
#endif
	

	/// 
	typedef tPointerHashMap<unsigned long, void*> cModeMap;
	cModeMap	mModeMap;
};
