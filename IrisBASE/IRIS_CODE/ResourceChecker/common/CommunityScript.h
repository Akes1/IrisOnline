#pragma once

#include "Player_Common.h"

enum eCOMMU_LIFE
{
	eCOMMU_LIFE_REST = 1,
	eCOMMU_LIFE_PARTY,
	eCOMMU_LIFE_TRADE,
	eCOMMU_LIFE_STREETSTALL,
	eCOMMU_LIFE_DUEL,
	eCOMMU_LIFE_ENHANCED,
	eCOMMU_LIFE_PUTCARD,
	eCOMMU_LIFE_TAROT,
	eCOMMU_LIFE_FOLLOW,
	eCOMMU_LIFE_TITLE,
	eCOMMU_LIFE_ITEMMIX,
	eCOMMU_LIFE_TARGET_ME,
	eCOMMU_LIFE_TARGET_PARTY_1,
	eCOMMU_LIFE_TARGET_PARTY_2,
	eCOMMU_LIFE_TARGET_PARTY_3,
	eCOMMU_LIFE_TARGET_PARTY_4,
	eCOMMU_LIFE_TARGET_PARTY_5,
};

#ifdef _CLIENT
	struct sCommunitySkillInfo
	{
		unsigned long	mIndex;			/// 분류 번호
		unsigned char	mType;			/// 타입 0:생활, 1:감정
		unsigned long	mRoleIndex;		/// type에 따른 명령어 종류
		unsigned long	mIconIndex;		/// 스킬창 표시 아이콘 번호
		unsigned long	mCommandIndex[5];	/// 채팅 명령어 인덱스 ( interface msg )
		unsigned long	mTooltipIndex;	/// 툴팁 인덱스 ( tooltip msg )

		/// 아이콘 표시 위치
		unsigned int	mPosX;
		unsigned int	mPosY;

		float			mAnimationTime[eGENDER_MAX * eRACE_MAX];	/// 
	};
#else
	struct sCommunitySkillInfo
	{
		unsigned long	mIndex;			/// 분류 번호
		unsigned char	mType;			/// 타입 0:생활, 1:감정
//		unsigned long	mRoleIndex;		/// type에 따른 명령어 종류
//		unsigned long	mIconIndex;		/// 스킬창 표시 아이콘 번호
//		unsigned long	mCommandIndex[5];	/// 채팅 명령어 인덱스 ( interface msg )
//		unsigned long	mTooltipIndex;	/// 툴팁 인덱스 ( tooltip msg )

		/// 아이콘 표시 위치
//		unsigned int	mPosX;
//		unsigned int	mPosY;

		float			mAnimationTime[eGENDER_MAX * eRACE_MAX];	/// 
	};
#endif


class cCommunityScript
{
public:
	static cCommunityScript*	 mpCommunityScript;

public:
	cCommunityScript();
	~cCommunityScript();

	///	초기화
	bool					Init();
	/// 전체 사용 메모리 제거
	void					Release();

	enum
	{
		COMMUNITY_LIFE = 0,
		COMMUNITY_EMOTE,
	};

	typedef tPointerHashMap<unsigned long, void*> cComHashMap;
	inline cComHashMap* GetComSkillMap() { return &mComSkillMap; }

	sCommunitySkillInfo* GetCommunityInfo( unsigned long idx );

	float GetEmotingMaintainTime( unsigned long commIndex, unsigned char race, unsigned char gender );

protected:

	/// data loading
#ifdef _CLIENT
	bool CommunitySkillScriptLoad_Client();
#endif
	bool CommunitySkillScriptLoad_Server();

protected:
	cComHashMap		mComSkillMap;
};

inline 
sCommunitySkillInfo* cCommunityScript::GetCommunityInfo( unsigned long idx )
{
	return (sCommunitySkillInfo*)mComSkillMap.GetAt( idx );
}

inline 
float cCommunityScript::GetEmotingMaintainTime( unsigned long commIndex, unsigned char race, unsigned char gender )
{
	sCommunitySkillInfo* pInfo = GetCommunityInfo( commIndex );
	if( pInfo == 0 )
		return 0.0f;
	if( pInfo->mType == 0 )
		return 0.0f;

	unsigned int key = race * eGENDER_MAX + gender;
	if( key >= eGENDER_MAX * eRACE_MAX )
	{
		assert(0);
		return 0.0f;
	}

	return pInfo->mAnimationTime[race * eGENDER_MAX + gender];
}

#define COMMUNITYSCRIPT cCommunityScript::mpCommunityScript