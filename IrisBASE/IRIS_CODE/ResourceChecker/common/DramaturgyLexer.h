/* =======================================================================================================
* 파    일 :	DramaturgyLexer.h , DramaturgyLexer.cpp
* 목    적 :	연출파일 토큰 저장소
* 작 성 자 :	정원주
* 작 성 일 :    2006년 9월 28일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 :	기본적으로 UI 스크립트와 구조 동일 - 이순규씨 Lexer상속 -> Parser와 연동
* ======================================================================================================== */
#pragma once

#include "Lexer.h"

/// 연출 토큰 타입
enum
{
	/// 대분류
	eTOKEN_HEADER = 100,
	eTOKEN_CASTING,
	eTOKEN_ACTIVITY,
	eTOKEN_APPLY,

	/// header부
	eTOKEN_TYPE,		// 연출 type (스킬, 효과)
	eTOKEN_EFFECT,
	eTOKEN_SOUND,
	eTOKEN_TRAILTEX,

	/// 세부 내용(casting, activity, apply)
	eTOKEN_ANI,			// 적용 애니 번호
	eTOKEN_ACTION,		// 적용 effect 상세
	eTOKEN_ACTIONEX,	// 적용 effect 상세 확장
	eTOKEN_SOUNDPLAY,	// 적용 sound 상세

	eTOKEN_ACTIVE,		// Activity & Apply 동시 처리 (서버인 경우 데미지 적용)
	eTOKEN_BULLET,		// ACTIVE의 역활을 수행하고, 발사체를 생성 충돌 검출 한다.
	eTOKEN_CRASH,
	eTOKEN_DAMAGE,		// 데미지 출력부
	eTOKEN_TRAIL,		// 
	eTOKEN_SHAKE,
	eTOKEN_FADEOUT,
};

class cDramaturgyLexer : public cLexer
{
public:
	cDramaturgyLexer( const char* pbuffer, unsigned int size );
};
