/* ==========================================================================
* 파    일 : BaseObject_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2006-11-27
* 주의사항 : 
*===========================================================================*/

#pragma once

#pragma pack(push,1)

class cBaseObject;




const float SYNC_MOVE_RANGE = 300.0f;
const unsigned long SYNC_MOVE_TIME = 300;
const unsigned long SYNC_NORMALATTACK = 100;
const unsigned short FIXED_OBJECT_SIZE = 20;

enum eNAME_OF_MAP
{
	MAP_NONE = 0,                       // 예약.
	MAP_01,                             // 테르미누스
	MAP_02,                             // 벌목지
	MAP_03,                             // 쉐버즈 설월
	MAP_04,                             // 장막의 계곡
	MAP_05,                             // 미궁 1층
	MAP_06,                             // 미궁 2층
	MAP_07,                             // 아네스의 눈
	MAP_08,                             // 잿더미 요새
	MAP_09,                             // 알투스 협곡
	MAP_10,                             // 아네스의 눈
	MAP_11,                             // 칼바람 분지
	MAP_12,                             
	MAP_13,                             // 숨겨진 동굴
	MAP_14,								// 용사의 안식처

	MAP_MIN = MAP_01,
	MAP_MAX = MAP_14,
	MAP_COUNT = MAP_MAX - MAP_MIN + 1,

	MAP_DEATHMATCH = 101,				// 전장인던 - 데스매치
	MAP_FLAG,							// 전장인던 - 깃발뺏기
	MAP_PVP_MIN = MAP_DEATHMATCH,
	MAP_PVP_MAX = MAP_FLAG,
	MAP_PVP_COUNT = MAP_PVP_MAX - MAP_PVP_MIN + 1,

	MAP_CIRCUS = 200,			// 테마인던 - 파니예서커스
	MAP_RIOHA,					// 테마인던 - 라이오하
	MAP_DECOMUSS,
	MAP_THEME_MIN = MAP_CIRCUS,
	MAP_THEME_MAX = MAP_DECOMUSS,
	MAP_THEME_CNT = MAP_THEME_MAX - MAP_THEME_MIN + 1,

	MAP_TUTORIAL = 501,
	MAP_ETC_MIN = MAP_TUTORIAL,
	MAP_ETC_MAX = MAP_TUTORIAL,
	MAP_ETC_CNT = MAP_ETC_MAX - MAP_ETC_MIN + 1,
};

#define MAX_THEMEDATA_MAX		MAP_THEME_CNT + MAP_ETC_CNT

enum SERVER_TYPE
{
	_E_ST_NORMAL_MAP_ = 0,			// 일반 맵 서버
	_E_ST_ID_PVP_,					// 인스턴트 던전 서버 - PVP
	_E_ST_ID_THEME_,				// 인스턴트 던전 서버 - THEME
	_E_ST_ID_TUTORIAL_,				// 인스턴트 던전 서버 - TUTORIAL
};


//const int PVP_MAP_NO = 101;
//const int PVP2_MAP_NO = 102;
//const int THEME_CIRCUS_MAP_NO = 200;
//const int THEME_RIOHA_MAP_NO = 201;

/// 타겟의 번호는 스크립트에 영향을 줄수 있으므로 항상 아래쪽에 추가 되어야 합니다.

enum eOBJECTSTATE
{
	eOBJECT_STATE_IDLE = 0,
	eOBJECT_STATE_MOVE,
	eOBJECT_STATE_ATTACK,
	eOBJECT_STATE_DIE,
	eOBJECT_STATE_STOP,
	eOBJECT_STATE_PUSHPULL,

	eOBJECT_STATE_MAX,
};

enum eOBJECTTYPE
{
	eOBJECTTYPE_NONE = 0,
	eOBJECTTYPE_HERO,
	eOBJECTTYPE_PLAYER,
	eOBJECTTYPE_MONSTER,
	eOBJECTTYPE_NPC,
	eOBJECTTYPE_ITEM,
	eOBJECTTYPE_TOTEM,
	eOBJECTTYPE_GATHERING,
	eOBJECTTYPE_PORTAL,
	eOBJECTTYPE_USERPORTAL,

	eOBJECTTYPE_DISPLAY,
};

enum eODDITYTYPE
{
	eODDITYTYPE_CANTMOVE,				/// 이동 불가
	eODDITYTYPE_CANTSKILL_PHY,			/// 스킬 사용 불가(물리)
	eODDITYTYPE_CANTSKILL_MAG,			/// 스킬 사용 불가(마법)
	eODDITYTYPE_SLEEP,					/// 수면
	eODDITYTYPE_STUN,					/// 스턴
	eODDITYTYPE_DAMAGE_REFLECTION_PHY,	/// 데미지 반사(물리)
	eODDITYTYPE_DAMAGE_REFLECTION_MAG,	/// 데미지 반사(마법)
	eODDITYTYPE_DAMAGE_ABSORB_PHY,		/// 데미지 흡수(물리:보호막)
	eODDITYTYPE_DAMAGE_ABSORB_MAG,		/// 데미지 흡수(마법:보호막)
	eODDITYTYPE_INVINCIBILITY_PHY,		/// 물리 무적
	eODDITYTYPE_INVINCIBILITY_MAG,		/// 마법 무적
	eODDITYTYPE_HP_ABSORB,				/// 공격자가 타겟 데미지를 hp로흡수
	eODDITYTYPE_MP_ABSORB,				/// 공격자가 타겟 데미지를 mp로흡수
	eODDITYTYPE_STOPITEM_KIND,			/// 아이템 사용불가 (종류별)
	eODDITYTYPE_STOPITEM_IDX,			/// 아이템 사용불가 (인덱스별)
	eODDITYTYPE_DAMAGE_ABSORB_PHY_RANGE,/// 물리 원거리 데미지 흡수(%)
	eODDITYTYPE_IGNORE_DEFENCE_PHY,		/// 방어력 무시(물리)
	eODDITYTYPE_IGNORE_DEFENCE_MAG,		/// 방어력 무시(마법)
	eODDITYTYPE_AVOID_MON_LOOK,			/// 몬스터 선공 인식 회피
	eODDITYTYPE_ADD_APPLY_CNT,			/// 범위 스킬 적용 인원 증가

	eODDITYTYPE_MAX,
};

#ifndef _OBJ_LINK_INFO
#define _OBJ_LINK_INFO
enum eLINK
{
	eLINK_HEAD = 0,
	eLINK_BODY,
	eLINK_FOOT,
	eLINK_RHAND,
	eLINK_LHAND,
	eLINK_LARM,

	eLINK_HEAD2,

	eLINK_RWEAPON = 10,
	eLINK_LWEAPON,

	eLINK_NULL = 100,
};

enum ePART
{
	ePART_HAIR = 0,
	ePART_FACE,
	ePART_BODY1,
	ePART_BODY2,
	ePART_HAND,
	ePART_FOOT,

	ePART_MAX,

	/// 귀 분리객체
	ePART_EAR = 100,
};
#endif

/// 객체에 대한 참조를 위해서 선언
struct sObject
{
	unsigned char	type;			/// eOBJECTTYPE
	unsigned long	index;
};


//struct sMultiTarget
//{
//	sObject		mTarget;
//	bool		mIsApply;
//
//	unsigned long	mDetailCnt;
//	long			mDetailDamageHP[10];
//	long			mDetailDamageMP[10];
//	unsigned char	mDetailDamageKind[10];
//	long			mDetailReflection[10];
//	long			mDetailAttkAbsorbHP[10];
//	long			mDetailAttkAbsorbMP[10];
//
//	sMultiTarget()
//	{
//		memset( this, 0, sizeof(sMultiTarget) );
//		mIsApply = true;
//	}
//};

#pragma pack(pop)
