#pragma once

#include "CardSceneNode.h"

const float CARDDATA_WIDTH = 2048.0f;
const float CARDDATA_HEIGHT = 1024.0f;


class cCardDramaObject
{
public:
	virtual ~cCardDramaObject();

	virtual bool Init( const char* pathname ) = 0;

	void Update( unsigned long time );

	void SetTargetAnimation( unsigned int aniIdx );

	void SetCardImage( unsigned int cardIdx, unsigned char race, unsigned char gender );

	inline cCardSceneNode* GetCardSceneNode() { return mSceneObject; }

	enum
	{
		eUV_BACK = 0,

		/// 최종 카드 이미지
		eUV_HUMAN_MALE,
		eUV_HUMAN_FEMALE,
		eUV_BEAST_MALE,
		eUV_BEAST_FEMALE,
		eUV_ELF_MALE,
		eUV_ELF_FEMALE,

		/// 종족 카드 이미지
		eUV_RACE_HUMAN,
		eUV_RACE_BEAST,
		eUV_RACE_ELF,

		/// 직업 카드 이미지
		eUV_JOB_FIGHT,
		eUV_JOB_MAGE,
		eUV_JOB_WARRIOR,
		eUV_JOB_THIEF,
		eUV_JOB_RANGER,
		eUV_JOB_SHAMAN,

		/// 종족 성별 카드 이미지
		eUV_GENDER_HUMAN_MALE,
		eUV_GENDER_HUMAN_FEMALE,
		eUV_GENDER_BEAST_MALE,
		eUV_GENDER_BEAST_FEMALE,
		eUV_GENDER_ELF_MALE,
		eUV_GENDER_ELF_FEMALE,

		eUV_MAX,
	};

protected:
	cCardDramaObject();

	virtual void Interpret( unsigned long time ) = 0;

protected:
	/// 장면 관리자의 노드
	cCardSceneNode*	mSceneObject;

	static NiRect<float>			mUVPoint[eUV_MAX];
};

class cSelectStage;

class cSelectCardDramaObject : public cCardDramaObject
{
public:
	cSelectCardDramaObject( cSelectStage* stage ) { mpStage = stage; }
	virtual ~cSelectCardDramaObject() {}

	virtual bool Init( const char* pathname );

protected:
	virtual void Interpret( unsigned long time );

protected:
	cSelectStage* mpStage;
};

class cMakeStage;

class cMakeCardDramaObject : public cCardDramaObject
{
public:
	cMakeCardDramaObject( cMakeStage* stage ) { mpStage = stage; }
	virtual ~cMakeCardDramaObject() {}

	virtual bool Init( const char* pathname );

	void SetMakeCardImage( unsigned char race );

protected:
	virtual void Interpret( unsigned long time );

protected:
	cMakeStage* mpStage;
};