/* ==========================================================================
* 파    일 : Party_Common.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-10-12
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Lexer.h"

////////////////////////////////////////////////////////////////////
// 튜토리얼 스크립트 토큰
enum 
{
	eTOKEN_IMAGES = 100,

	eTOKEN_MODE,

	eTOKEN_OBJECT,
	eTOKEN_IMAGE,
	eTOKEN_OBJECTIMAGE,

	eTOKEN_STARTPOSIDX,
	eTOKEN_DIETEXTINDEX,

	eTOKEN_OBJMONSTER,
	eTOKEN_OBJNPC,
	eTOKEN_OBJITEM,

	eTOKEN_II,
	eTOKEN_POS,
	eTOKEN_SIZE,
	eTOKEN_TEXPOS,
	eTOKEN_DOWNPOS,
	eTOKEN_ANIFRAME,
	eTOKEN_ANIREPEAT,
	eTOKEN_ANITIME,
	eTOKEN_KEYINPUT,
	eTOKEN_DEFAULTSHOW,
	eTOKEN_DRAMA,
	eTOKEN_LEFTTOP,
	eTOKEN_IMGALIGN,
};

class cTutorialLexer : public cLexer
{
public:
	cTutorialLexer( const char* buffer, unsigned int size );
};

#pragma pack(push,1)
#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"

//--------
// 튜토리얼 종료 처리
#define ERROR_TUTORIAL_GIVEUP_SUCCESS	0	// 포기 성공
#define ERROR_TUTORIAL_GIVEUP_MAP		1	// 맵이동 실패
#define ERROR_TUTORIAL_GIVEUP_TUTORIAL	2	// 튜토리얼 상태 아님

//
struct MSG_SYN_TUTORIAL_CHANGEMODE : public MSGROOT
{
	int		mModeIndex;		// 모드인덱스
};

//--------
// 부활
#define ERROR_TUTORIAL_RESURRECTION_SUCCESS	0	// 성공
#define ERROR_TUTORIAL_RESURRECTION_FAIL	1	// 실패

struct MSG_RES_TUTORIAL_RESURRECTION : MSG_ERROR
{
	unsigned long	mHP;
	unsigned long	mMaxHP;
	unsigned long	mMP;
	unsigned long	mMaxMP;
	float			mPosX;
	float			mPosY;
};

struct MSG_SYN_TUTORIAL_RESURRECTION : MSG_ERROR
{
	unsigned long	mCharacterIdx;
	unsigned long	mHP;
	unsigned long	mMaxHP;
	unsigned long	mMP;
	unsigned long	mMaxMP;
	float			mPosX;
	float			mPosY;
};

#pragma pack(pop)
