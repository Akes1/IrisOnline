/* ==========================================================================
* 파    일 : BaseObject_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2006-11-27
* 주의사항 : 
*===========================================================================*/

#pragma once

#pragma pack(push,1)

//class cBaseObject;

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
	MAP_MIN = MAP_01,
	MAP_MAX = MAP_02,
	MAP_COUNT = MAP_MAX - MAP_MIN + 1
};

/// 타겟의 번호는 스크립트에 영향을 줄수 있으므로 항상 아래쪽에 추가 되어야 합니다.

enum eOBJECTSTATE
{
	eOBJECT_STATE_IDLE = 0,
	eOBJECT_STATE_MOVE,
	eOBJECT_STATE_ATTACK,
	eOBJECT_STATE_DIE,
	eOBJECT_STATE_STOP,

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

	eOBJECTTYPE_DISPLAY,
};


/// 객체에 대한 참조를 위해서 선언
struct sObject
{
	eOBJECTTYPE		type;
	unsigned long	index;
};


#pragma pack(pop)
