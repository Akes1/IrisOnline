#pragma once

#include "item_common.h"
#include "Player_Common.h"
#include "..\Client\Pathfinder.h"


struct sItem
{
	sInventory inv;
};


class cAsyncSelect;
class cObjectManager;
class cItemManager;
class cSkillManager;

class cPlayer;

enum eSTAGE
{
	eStage_Disconnet = 0,
	eStage_ReLogin,
	eStage_Login,
	eStage_Load,
	eStage_MapChange,
	eStage_GameIn,
	eStage_GameProcess,
};

enum eSTATE
{
	eState_Idle = 0,
	eState_Move,

	eState_Max,

	eState_SitDown,
	eState_ItemPick,
	eState_Attack,
	eState_Gather,

	eState_Die,
	eState_Stop,
};

enum eACTION
{
	eAction_None = 0,
	eAction_MapChange,
	eAction_Attack,
	eAction_ItemPick,
};

struct sAction
{
	eACTION			type;
	NiPoint3		destPos;
	float			range;

	sObject			target;

	/// type에 따라 저장 정보를 달리한다..
	unsigned long	exData1;
	unsigned long	exData2;
};

struct sLoadFlag
{
	union
	{
		struct
		{
			bool heroBaseInfoOn		: 1;
			bool monsterModelOn		: 1;
			bool npcModelOn			: 1;

			bool skillInfoOn		: 1;
			bool InfluenceInfoOn	: 1;
			bool guardInfoOn		: 1;

			bool itemInfoOn			: 1;
			bool itemCoolTimeInfoOn	: 1;

			bool questInfoOn		: 1;
			bool optionInfoOn		: 1;

			bool shortcutInfoOn		: 1;
			bool titleInfoOn		: 1;
			bool makeSkillInfoOn	: 1;

		} flag;
		long loadInfo;
	};
};

class cDummyClient
{
public:
	cDummyClient();
	~cDummyClient();

	bool Init( HWND wnd, unsigned int msgIdx, char* id, char* pwd, char* spwd, long serverIdx, long channelIdx );

	/// 관리 데이터 처리부
	void ProcessManager( unsigned long deltaTime, unsigned long accumTime );

	/// 클라이언터 객체 처리부
	void Process( unsigned long deltaTime, unsigned long accumTime );

	void NetWorkProc( WPARAM wparam, LPARAM lparam );
	void NetWorkMsgParser( char* pMsg );

	void ChangeStage( eSTAGE stage );

	long GetCurServer() { return mCurServer; }
	long GetCurChannel() { return mCurChannel; }

	unsigned long GetObjectID() { return mPlayerInfo.CharacterIdx; }

	void SetCantMove( bool cantMove )	{ mCantMove = cantMove; }
	void SetCantSkill( bool cantSkill, unsigned char attributeType );
	bool IsCantSkill( unsigned long skillIdx );
	bool IsCantMove()	{ return mCantMove == true || mIsSleep == true || mIsStun == true; }

	bool IsDie() { return (mCurState == eState_Die); }

	NiPoint3 GetPos() { return mTrans.m_Translate; }

	float GetFixedObjectSize();
	void SetFixedObjectSizeScale( unsigned short scalePer, bool init );

protected:

	void SetMapIndex( unsigned short mapIdx, unsigned char mapMode );

	///
	void MsgParser_User( MSGROOT* pMsgRoot );
	void MsgParser_Player( MSGROOT* pMsgRoot );
	void MsgParser_Monster( MSGROOT* pMsgRoot );
	void MsgParser_Npc( MSGROOT* pMsgRoot );
	void MsgParser_Skill( MSGROOT* pMsgRoot );
	void MsgParser_Item( MSGROOT* pMsgRoot );
	void MsgParser_Quest( MSGROOT* pMsgRoot );
	void MsgParser_MakeSkill( MSGROOT* pMsgRoot );

	///
	void MakeCharacter();
	cStringT RandomString( int iLength );

	void ProcessMoveAction();

	bool ResetMovePath( NiPoint2 start, NiPoint2 goal );
	void MoveProcess( unsigned long deltaTime );
	bool Move( float x, float y );

	void ChangeState();
	void SetState( eSTATE state, unsigned long time = 0 );

	void UpdateWeaponState();

	void CheckEquip();

protected:

	/// 
	cAsyncSelect* mAsyncSelect;

	cString mId;
	cString mPwd;
	cString mSec;
	unsigned long mReLogintime;

	long mCurServer;
	long mCurChannel;

	eSTAGE mStage;

	sLoadFlag		mLoadFlag;

	//////////////////////////////////////////////////////////////////////////
	///
	sPlayerInfo		mPlayerInfo;
	sPlayerExrInfo	mPlayerExrInfo;
	sPlayerWearInfo mPlayerWearInfo;

	/// 무기 정보
	sPlayerWeaponInfo mPlayerWeaponInfo;

	sHeroInfo mHeroInfo;

	eWEAPON_STATE mPlayerWeaponState;

	float mFixedObjectSize;

	/// 돈 정보
	unsigned long	mMoney;

	/// 창고 보관액 정보
	unsigned long	mDeposit;

	///
	sItem mItemArray[LEN_INVENTORY];


	unsigned short mCurMapIdx;
	unsigned char mCurMapMode;

	unsigned short mMapFolderIdx;

	NiTransform		mTrans;
	NiPoint3		mDesiredDir;

	eSTATE	mCurState;
	unsigned long mStateTime;

	/// 공격 정보
	sObject mTarget;

	/// 이동 불가
	bool mCantMove;
	/// 스킬 사용 불가
	bool mCantSkillPhy;
	/// 스킬 사용 불가
	bool mCantSkillMag;
	/// 수면
	bool mIsSleep;
	/// 스턴
	bool mIsStun;

	/// 이동 정보
	float mGotoX;
	float mGotoY;
	NiPoint3	mTargetPos;

	static const unsigned int MAX_PATH_COUNT = 2048;
	NiPoint2 mPathArray[MAX_PATH_COUNT];
	unsigned int mPathIndex;
	unsigned int mPathCount;

	sAction mCurActionInfo;

	/// object 관리자..
	cObjectManager* mpObjectManager;
	cSkillManager* mpSkillManager;
};
