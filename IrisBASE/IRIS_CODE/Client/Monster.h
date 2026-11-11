/* ==========================================================================
* 파    일 : Monster.h
* 목    적 : 몬스터 관리
* 작 성 자 : 박경희
* 작 성 일 : 2006-10-12
* 주의사항 :
*===========================================================================*/
#pragma once

#include "BaseObject.h"
#include "Monster_Common.h"

struct sTextItem;
struct sModeImage;
class cPlaneObject;
class cModeAgent;

#define MODEIMAGE_VALID_DISTANCE	3000.0f
#define MODEIMAGE_TIME	5000		// 5초 지속

/// 오브젝트 상태 값
enum eMONSTER_ANIMATIONTYPE
{ 
	M_ANITYPE_IDLE1 = 1,	/// 대기
	M_ANITYPE_IDLE2,		/// 대기보조
	M_ANITYPE_WORK,			/// 걷기
	M_ANITYPE_RUN,			/// 달리기
	M_ANITYPE_ATTACK1,		/// 공격1
	M_ANITYPE_ATTACK2,		/// 공격2
	M_ANITYPE_ATTACK3,		/// 스킬1
	M_ANITYPE_CASTING,		/// 캐스팅
	M_ANITYPE_SKILL1,		/// 스킬2
	M_ANITYPE_SKILL2,		/// 스킬3
	M_ANITYPE_DAMAGE1,		/// 데미지1
	M_ANITYPE_DAMAGE2,		/// 데미지2
	M_ANITYPE_DIE1,			/// 죽음1
	M_ANITYPE_DIE2,			/// 죽음2
	M_ANITYPE_JUMP,			/// 점프

	M_ANITYPE_MODEANI = 16,

	M_ANITYPE_MODEANI_STURN = 16,

	M_ANITYPE_MODEANI_CASTING1 = 21,
	M_ANITYPE_MODEANI_CASTING2 = 22,

	M_ANITYPE_MODESKILL_START = 31,
	M_ANITYPE_MODESKILL_END = 37,
};

class cMonster : public cBaseObject
{
public:

	cMonster( unsigned char type = eOBJECTTYPE_MONSTER );
	virtual ~cMonster();

	inline virtual unsigned long	GetObjectID() { return mMonsterIdx; }
	inline unsigned long			GetMonsterClassIdx() { return mpMonsterInfo->mMonsterClassIdx; }
	inline sMonsterScript*			GetMonsterScript() { return mpMonsterInfo; }

	/// 객체 처리
	virtual bool		Create( sMonsterData* baseInfo, NiPoint3 pos, NiMatrix3 rotate = NiMatrix3::IDENTITY );
	virtual void		Update( unsigned long deltaTime, unsigned long accumTime );
	/// 객체 링크
	unsigned int		SetLinkdObject( unsigned int num, char* nif );
	void				UnLinkdObject( unsigned int num );
	/// 방향전환
	bool				SetDesiredDir(NiPoint3 CurrentPos, NiPoint3 TargetPos, unsigned long delay = 0 );


	/// 이동
	virtual bool		Move( float x, float y );


	/// 몬스터의 이름을 가져옴
	inline LPTSTR		GetName() { return mpMonsterInfo->mName;  }
	inline LPTSTR		GetJobName() { return mpMonsterInfo->mJobName; }

	/// 몬스터 레벨
	unsigned char		GetLevel(){ return mpMonsterInfo->mLevel; }
	/// 몬스터 분류
	unsigned char		GetKind() { return mpMonsterInfo->mKind;  }


	/// 스테이터스 처리
	unsigned long		GetHP()	{ return mHP; }
	unsigned long		GetMaxHP()	{ return mMaxHP; }
	/// 입력 받은 데미지 만큼 hp/mp를 감소
	virtual void		HPChange( unsigned long objectHP, unsigned long objectMaxHP );
	virtual void		HPChangeDie( unsigned long objectMaxHP, cBaseObject* pAttacker, unsigned char attackType );
	virtual void		MPChange( unsigned long objectMP, unsigned long objectMaxMP );
	/// 이동속도
	void				SetMoveSpeed( unsigned int moveSpeed, eMOVE_SPEED_TYPE moveSpeedType = eMOVESPEED_STOP );

	void				SetFixedObjectSizeScale( unsigned short size, bool init );

	/// 상태 처리
	virtual void				SetState( unsigned char state, bool bShowAni = true );
	virtual unsigned int		GetState() { return mState; }

	/// 길찾기 처리
	/// 이동시 최종 경로 셋팅
	void				SetLastGoto( float x, float y) { mLastGotoX = x; mLastGotoY = y; }
	/// 이동 경로
	void				SetPathArray( NiPoint2* moveArray, unsigned int count );
	/// 이동 정지
	void				MoveStop( float x, float y );
	/// 이동좌표 최종 목적지까기 가기전 정지의 범위
	void				SetMoveStopRange( float moveStopRange ) { mMoveStopRange = moveStopRange; }


	/// 공격 처리
	void				MonsterAttack( unsigned long uIdx, unsigned long skillIdx, cBaseObject* pTarget, float attackSpeedFactor );
	void				MonsterAttack( unsigned long uIdx, unsigned long skillIdx, NiPoint3 fieldPos, float attackSpeedFactor );

	virtual void		ApplyDamageDrama( int damage, unsigned int damageType, cBaseObject* pAttacker = 0 );

	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	float				GetFixedObjectSize()	{ return mFixedObjectSize; }

	// 선공몬스터인지 판단
	bool				IsPrecedeAttack();

	// 변신 인식 몬스터 판단
	bool				IsChangeMonCheck();

	/// 갱신
	void				UpdateNameCardTextValue();

	/// 대기동작2 실행
	void				PlayIdle2Ani();

	void ActiveChatBubble( sTextItem* text );

	virtual bool IsDie() { return (GetState()==eOBJECT_STATE_DIE); }

	void ChangeMonsterMode( long modeIdx );

	virtual cEffectSceneNode*	SetLinkdEffect( unsigned int num, const char* nif, NiTransform* trans, bool bLoop=false, bool bFollow=true );

	void UpdateTutorialBoard();
	void ClearTutorialBoard();

	virtual void SetSkillPushPull( float posX, float posY );

	void SetPvPTeamType( unsigned long teamType )	{ mPvPTeamType = teamType; }
	unsigned long GetPvPTeamType()	{ return mPvPTeamType; }

	// 오토더미용 몬스터
	bool				IsAutoDummy() { return (mAutoDummy==1); }

protected:

	/// 애니메이션 관계 설정
	virtual void		Interpret( unsigned long time );

protected:

	/// monster 고유번호
	unsigned long		mMonsterIdx;
	/// 몬스터 정보
	sMonsterScript*		mpMonsterInfo;
	
	cModeAgent*			mpModeAgent;

	/// 이동속도
	unsigned int		mMoveSpeed;
	/// HP/MP 관련
	unsigned long		mHP;
	unsigned long		mMP;
	unsigned long		mMaxHP;
	unsigned long		mMaxMP;

	/// 크기 효과
	unsigned short		mFixedObjectSizePer;

	/// 이동시 목적지 좌표
	float				mGotoX;
	float				mGotoY;
	/// 이동시 최종위치
	float				mLastGotoX;
	float				mLastGotoY;
	/// 이동시 방향 및 
	NiPoint3			mDesiredDir;
	/// 이동좌표 최종 목적지까기 가기전 정지의 범위
	float				mMoveStopRange;

	unsigned long		mMoveOverTime;		/// 경로 목적지까지 이동중 프로세스 간격에의해 넘어간 시간

	/// 몬스터의 상태
	eOBJECTSTATE		mState;
	///몬스터 객체 삭제 시간
	unsigned long		mMonsterDieTime;

	eMOVE_SPEED_TYPE	mMoveSpeedType;

	/// 길찾기
	static const unsigned int MAX_PATH_COUNT = 2048;
	NiPoint2			mPathArray[MAX_PATH_COUNT];
	unsigned int		mPathIndex;
	unsigned int		mPathCount;

	/// 
	unsigned long		mDelay;
	float				mAngle;
	float				mAccumAngle;
	NiPoint3			mAxis;

	/// 오브젝트의 거리 체크 수정 거리 - 적용 범위
	float				mFixedObjectSize;

	unsigned int		mDamageAfterAnim;

	typedef tList<unsigned long> cModeLoopList;
	cModeLoopList mModeLoopEffectList;
	cModeLoopList mModeLoopSoundList;

	unsigned long	mCurrentModeIndex;

	unsigned long	mPvPTeamType;		/// ePVPDM_TEAM_TYPE

	// 이미지 모드 정보
	cPlaneObject*	mpImagePlane;
	sModeImage*		mpImageMode;
	unsigned long	mImageStartAccumTime;
	unsigned long	mImageEndAccumTime;

	// 오토더미 여부 (0:아님,1:오토더미)
	char			mAutoDummy;	
};
