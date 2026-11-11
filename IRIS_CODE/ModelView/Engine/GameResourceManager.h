/* ====================================================================
 * 파    일 :	GameResourceManager.h
 * 목    적 :	게임 리소스(스크립트 파일등)파일 관리
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.06
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		2006.06     이법석	    생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once
#include "Player_Common.h"

class cSkillScript;

struct sNpcInfo
{
	unsigned long mIndex;

	cString mName;
	unsigned long mNameIndex;

	unsigned long mModelIndex;
};

struct sMonsterInfo
{
	unsigned long mIndex;

	cString		mName;
	unsigned long mNameIndex;

	unsigned long mModelIndex;

	float mScale;
	float mEffectScale;
};

struct sItemInfo
{
	unsigned long mIndex;

	unsigned char mType;
	unsigned char mSubType;

	unsigned long mNameIndex;

	unsigned long mModelIndex1;
	unsigned long mModelIndex2;

	char	mHideEar;
};

struct sItemOnepieceInfo
{
	unsigned long mIndex;

	unsigned long mUpperIndex;
	unsigned long mLowerIndex;
	unsigned long mHandsIndex;
	unsigned long mFootsIndex;
};

struct sFaceEmoteInfo
{
	unsigned long	index;				/// list index
	unsigned char	type;				/// 0:base, 1:wink, 2:emote

	cString			textureName[eRACE_MAX*eGENDER_MAX];
};

class cUIText;
class cParser;

// 각종 게임 정보를 위한 자료구조들..
typedef tPointerHashMap<unsigned long, void*>	cArrayHashMap;
typedef tArray<void*>							cFileNameArray;

typedef tArray<void*>	cArray;

class cGameResourceManager
{
public:
	static cGameResourceManager* mpGameResourceManager;

	cGameResourceManager();
	~cGameResourceManager();

	/// 초기화
	bool Init();

	/// 종료부
	void Exit();

	const char*	GetModelFileName( unsigned long modelIndex );

	/// 복장의 기본 파일명을 제공한다.
	const char*	GetDefaultHairFileName( unsigned int hairIdx, char race, char gender );
	const char*	GetHairColorTextureFileName( unsigned int hairIdx, unsigned int colorIdx, char race, char gender );
	const char*	GetDefaultFaceFileName( unsigned int faceIdx, char race, char gender );
	const char*	GetDefaultWearFileName( unsigned int part, char race, char gender );

	const char*	GetFaceTextureFileName( unsigned long idx, char race, char gender );

	inline unsigned int	GetDefaultFaceCount() { return mBaseFaceCount; }
	inline unsigned int	GetDefaultHairCount() { return mDefaultHairFileNameMap.GetSize(); }
	inline unsigned int	GetHairColorCount() { return mHairColorMax; }

	LPCTSTR		GetGameText( unsigned int textIdx );

	LPCTSTR		GetSkillNameText( unsigned int textIdx );
	LPCTSTR		GetItemNameText( unsigned int nameIdx );

	LPCTSTR		GetNpcName( unsigned int nameIdx );
	LPCTSTR		GetMonsterName( unsigned int nameIdx );


	/// Item 정보 참조
	cArrayHashMap*	GetItemList() { return &mItemInfo; }
	const char*		GetItemFileName( unsigned long itemFileIdx, char race, char gender, bool weapon );
	sItemOnepieceInfo* GetItemOnepieceInfo( unsigned long itemIndex );

	/// monster 
	cArrayHashMap*	GetMonsterList() { return &mMonsterInfo; }
	sMonsterInfo* GetMonsterInfo( unsigned long monIdx );

	/// npc
	cArrayHashMap*	GetNpcList() { return &mNpceInfo; }

protected:

	/// Game에 사용되는 고정 텍스트를 로딩한다.
	bool LoadGameText();

	/// 모델정보 및 캐릭터 기본 파일 정보..
	/// player, npc, monster의 모델파일정보 kfm파일
	bool LoadModelFileInfo();

	/// 캐릭터의 기본 외형정보 nif파일..
	bool LoadCharacterBaseFileInfo();

	/// 캐릭터의 헤어 정보 (모델파일 / 텍스쳐파일)
	bool LoadCharacterBaseHairFileInfo();
	bool LoadHair( cParser& parser );
	bool LoadHairColor( cParser& parser );

	bool LoadFaceEmotionInfo();

	bool LoadItemInfo();
	bool LoadItemFileInfo();
	bool LoadItemOnePieceInfo();

	bool LoadMonsterInfo();
	bool LoadNpcInfo();

	/// 참조 데이터를 재로딩 할경우에 사용하려고 만들어 본다.
	void RemoveAllModelFileName();
	void RemoveAllCharacterBaseFileInfo();

protected:
	// text 문서
	cUIText*	mpGameText;

	cUIText*	mpItemNameText;

	cUIText*	mpSkillNameText;

	cUIText*	mpNpcNameText;

	cUIText*	mpMonsterNameText;

	/// kfm파일 ( NiString 사용 )
	cArrayHashMap mModelFileNameMap;

	/// 머리 종류들
	cArrayHashMap mDefaultHairFileNameMap;

	/// 머리 색깔( 텍스쳐 파일 )
	cArrayHashMap mHairColorTextureMap;
	unsigned int mHairColorMax;

	/// 얼굴 종류들
	cArrayHashMap mDefaultFaceFileNameMap;

	/// 인덱스 -> 상의 하의 손 발 각각 1개의 정보를 갖는다.
	cArrayHashMap mDefaultWearFileNameMap;

	cArrayHashMap			mFaceEmotionMap;
	unsigned int			mBaseFaceCount;

	/// 아이템 정보
	cArrayHashMap mItemInfo;
	cArrayHashMap mItemFileInfo;
	cArrayHashMap mItemOnePieceInfo;

	/// 몬스터 리스트
	cArrayHashMap mMonsterInfo;
	cArrayHashMap mNpceInfo;

	cSkillScript*		mpSkillScript;

};

#define GAMERESOURCEMAN	cGameResourceManager::mpGameResourceManager
