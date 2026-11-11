/* ==========================================================================
* 파    일 : UICursor.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-01-17
* 주의사항 :
*===========================================================================*/
#pragma once
//#include "UIRect.h"

enum eCursor_Type
{
	eCURSOR_DEFAULT = 0,		/// 기본 커서
	eCURSOR_ATTACK,			/// 공격 커서
	eCURSOR_SKILL,			/// 스킬 커서
	eCURSOR_TALK,			/// 대화 커서
	eCURSOR_MOVEMAP,		/// 맵 이동 커서
	eCURSOR_MOVESTOP,		/// 이동 불가 커서
	eCURSOR_DROPITEM,		/// 스킬 타겟 선택 커서
	eCURSOR_GATHERING,		/// 아이템 겟
	
	eCURSOR_CLICK,			/// 마우스 클릭시
	eCURSOR_USER,			/// 플레이어 커서
	eCURSOR_USER_SELL,		/// 노점 커서
	eCURSOR_TAROT,			/// 타로 점집 커서

	eCURSOR_MAX,

	eCURSOR_UNCLICK,		/// 마우스 클릭이 끝났을 시 기본 커서로 돌리기 위한 상태
};

enum eCusor_Mode
{
	eMODE_INANIMINATE,		/// 이미지 고정
	eMODE_ANIMATE,			/// 이미지 애니메이션
};

/* 
	스크립트는 사용하지 않는다!!! 
	 애니메이션 기능 나중 추가 
*/

class cUICursor
{
	static cUICursor* mpSingleton;

public:
	cUICursor();
	virtual ~cUICursor();

	/// 단일 객체를 리턴 
	static cUICursor* Get();

	/// 초기화
	bool Init();

	/// 업데이트
	void Process( float accumTime );

	/// 커서 타입을 바꾼다 ( 커서 형태를 체인지 )
	void SetCursor( eCursor_Type eType );

private:
	/// 윈도우 커서
	HCURSOR mCursor[ eCURSOR_MAX ];

	/// 현재 커서
	eCursor_Type mCurCursorType;

	/// alpha
	float mCusorAlpha;
};


inline cUICursor* cUICursor::Get()
{
	return mpSingleton;
}
#define CURSOR cUICursor::Get()