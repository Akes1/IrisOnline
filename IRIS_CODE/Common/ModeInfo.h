#pragma once

//////////////////////////////////////////////////////////////////////////
// Mode used resource object
enum eMODEOBJ_KIND
{
	eMODEOBJ_EFFECT = 0,	/// 이펙트 객체(nif)
	eMODEOBJ_SOUND,		/// 사운드 객체
};

#ifdef _CLIENT
struct sModeObj
{
	eMODEOBJ_KIND	kind;
	cString			file;

	/// effect인 경우 사용
	NiMatrix3		rot;
	float			scale;

	/// sound인 경우 사용
	unsigned long	soundListIndex;

	sModeObj()
	{
		rot = NiMatrix3::IDENTITY;
		scale = 1.0f;
	}
};
#endif


//////////////////////////////////////////////////////////////////////////
// Mode Info
enum eMODEINFO_TYPE
{
	eMODEINFO_APPLYBUFF = 0,
	eMODEINFO_GENMONSTER,
	eMODEINFO_GENTOTEM,
	eMODEINFO_GENGATHERING,
	eMODEINFO_SPEECH,
	eMODEINFO_MODESTATE,
	eMODEINFO_AGGRO,
	eMODEINFO_MODEINITSKILL,
	eMODEINFO_APPLYRESOURCE,
	eMODEINFO_MODECHANGE,
	eMODEINFO_USESKILL,
	eMODEINFO_IMMUNE,
	eMODEINFO_APPLYAURA,
	eMODEINFO_IMAGE,

	eMODEINFO_MAX,
};

///

struct sCoditionInfo
{
	unsigned char conType;
	unsigned long conValue;
};

struct sModeInfoBase
{
	sModeInfoBase() {} 
	virtual ~sModeInfoBase() {}


	eMODEINFO_TYPE	type;
};

struct sModeBuff : public sModeInfoBase
{
	unsigned long	mApplyBuffIdx;
	bool			mModeDel;
};

struct sModeAura : public sModeInfoBase
{
	unsigned long	mApplyBuffIdx;
	bool			mEnemyApply;
	unsigned short	mRange;
	//	unsigned char	mAttributeType;
	bool			mModeDel;
};

struct sModeGenMonster : public sModeInfoBase
{
	unsigned long	mGenMonsterIdx;

	unsigned long	mOrderNum;

	unsigned char	mCount;

	unsigned long	mGenRangeMin;
	unsigned long	mGenRangeMax;

	bool			mLoop;
	unsigned long	mDelayTime;

	unsigned long	mGenPatternIdx;
};

struct sModeGenTotem : public sModeInfoBase
{
	unsigned long	mGenTotemIdx;

	unsigned long	mOrderNum;

	unsigned char	mCount;

	unsigned long	mGenRangeMin;
	unsigned long	mGenRangeMax;

	bool			mLoop;
	unsigned long	mDelayTime;

	unsigned long	mGenPatternIdx;
	unsigned long	mApplyType;
};

struct sModeGenGathering : public sModeInfoBase
{
	unsigned long	mGenGatheringIdx;

	unsigned long	mOrderNum;

	unsigned char	mCount;

	unsigned long	mGenRangeMin;
	unsigned long	mGenRangeMax;

	bool			mLoop;
	unsigned long	mDelayTime;

	unsigned long	mGenPatternIdx;
};

struct sModeSpeech : public sModeInfoBase
{
	unsigned char	mSpeechType;	// 표시 설정 타입 (0 : 말풍선, 1: 말풍선+일반채팅, 2:말풍선+외치기채팅)
	unsigned long	mSpeechIdx;
};

struct sModeImage : public sModeInfoBase
{
	cString			mFileName;		// 이미지 파일명
	int				mTexX;
	int				mTexY;
	unsigned long	mTime;			// 모드 변환후 출력 시간
};

struct sModeState : public sModeInfoBase
{
	unsigned long	mAnimationIdx;

	unsigned long	mMaintainTime;
};

struct sModeResource : public sModeInfoBase
{
	unsigned short	mModeObjIdx;
	bool			mLoop;
	bool			mFollow;
	unsigned char	mLinkPos;

	NiPoint3		mVar;
};

struct sModeAggro : public sModeInfoBase
{
};

struct sModeChange : public sModeInfoBase
{
	unsigned long	mModeIdx;

	sCoditionInfo	mCondition[2];
};

struct sModeSkill : public sModeInfoBase
{
	unsigned long	mSkillIdx;

	unsigned long	mModeDelayTime;

	unsigned short	mActionPer;

	sCoditionInfo	mCondition[2];
};

struct sModeImmune : public sModeInfoBase
{
	unsigned long	mSkillApplyType;
};

struct sModeInitSkill : public sModeInfoBase
{
};

struct sModeDeathMonster
{
	unsigned long	mGenMonsterIdx;

	unsigned long	mOrderNum;

	unsigned char	mCount;

	unsigned long	mGenRangeMin;
	unsigned long	mGenRangeMax;
};

