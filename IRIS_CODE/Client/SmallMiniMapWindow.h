/* ==========================================================================
* 파    일 : SmallMiniMapWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-05-21
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "Npc_Common.h"

class cUIImage;
struct sNpcData;
struct sPartyUserData;
struct sUnionUserData;
struct sAniParam;
class cLabel;

struct sBaseImageData
{
	cUIImage*		mpImage;
	cUIPos			mPos;		/// 데이터좌표
	cUIPos			mScreenPos;	/// 스크린좌표
	cUISize			mSize;
	bool			mShow;
	bool			mEnableShow;
	cStringT		mName;
	
	sBaseImageData()
	{
		mpImage = 0;
		mPos.mX = 0;
		mPos.mY = 0;
		mScreenPos.mX = 0;
		mScreenPos.mY = 0;
		mSize.mWidth = 0;
		mSize.mHeight = 0;
		mEnableShow = true;
		mShow = true;
	}
	virtual ~sBaseImageData(){}
};

struct sAniImageData : sBaseImageData
{
	sAniParam*		mAniParam;

	sAniImageData()
	{
		mAniParam = 0;
	}
	~sAniImageData(){}
};

/// npc 이미지 데이터
struct sNpcImageData : sBaseImageData
{
	unsigned long	mClassIndex;
	eNpcQuestEffect	mQuestEffect;	/// 퀘스트 관련 상태
	sAniParam*		mAniParam;
	cUIImage*		mpAniImage;
	cStringT		mJobName;
	
	sNpcImageData()
	{
		mClassIndex = 0;
		mQuestEffect = eNPCQUEST_EFFECT_NONE;
		mAniParam = 0;
		mpAniImage = 0;
	}
	~sNpcImageData(){}
};

/// 포탈 이미지 데이터
struct sPortalImageData : sBaseImageData
{
	int				mTargetMapNum;

	sPortalImageData()
	{
		mTargetMapNum = 0;
	}
	~sPortalImageData(){}
};

/// 유저 데이터
struct sUserImageData : sBaseImageData
{
	int				mOldX;
	int				mOldY;
	unsigned short	mMapNum;
	unsigned short  mRoomNum;
	DWORD			mChannelNum;
	unsigned long	mUserIndex;
	
	sUserImageData()
	{
		mOldX = 0;
		mOldY = 0;
		mMapNum = 0;
		mRoomNum = 0;
		mChannelNum = 0;
	}
	~sUserImageData(){}
};

// 미니윈도우에서 맵 이미지가 시작되야하는 좌표 & 맵 이미지 적용될 사이즈 
const int MAP_X = 10;
const int MAP_Y = 34;
const int MAP_W = 150;
const int MAP_H = 150;

// 맵 텍스쳐 사이즈
const unsigned short MAPTEX_W = 512;
const unsigned short MAPTEX_H = 512;

/// 추가 감소되는 값
const float MAX_PERCENT = 50.f;
const float MIN_PERCENT = 20.f;		
const float ADD_PERCENT = 20.f;

const int NPCICON_SIZE = 16;		/// 사이즈
const int POSTICON_SIZE = 16;		/// 사이즈

const int PIXNPCANI_X = 2;			// NPC 애니메이션 이미지 좌표 보정
const int PIXNPCANI_Y = 8;

// 미니맵
class cSmallMiniMapWindow : public cUIWindow
{
public:
	cSmallMiniMapWindow();
	virtual ~cSmallMiniMapWindow();

	virtual void Open();
	virtual void Close();
	virtual void Clear();

	// 맵 포탈 정보 세팅
	virtual void SetMapData();

	virtual void ClearNpc();	// 전체 삭제
	virtual void AddNpc( sNpcData* data );
	virtual void DeleteNpc( unsigned long npcIdx );
	virtual void UpdateNpcEffect( unsigned long npcIdx, eNpcQuestEffect status );

	virtual void ClearParty(); // 전체 삭제
	virtual void AddParty( sPartyUserData* data );
	virtual void DeleteParty( unsigned long userIdx );
	virtual void UpdatePartyPos( unsigned long userIdx, float posX, float posY );
	virtual void UpdatePartyMap( unsigned long userIdx, unsigned short mapnum, unsigned short roomnum, DWORD channelnum );
	
	virtual void AddUnion( sUnionUserData* data );
	virtual void DeleteUnion( unsigned long userIdx );
	virtual void UpdateUnion( unsigned long userIdx, float posX, float posY );

	virtual void AddPortal( unsigned long index );

	// 찾아가기 기능
	virtual void AddFindPosition( float xpos, float ypos );
	virtual void DeleteFindPsotion();

	// 깃발 기능
	virtual void ClearFlag();
	virtual void AddFlag( unsigned long objectIdx, unsigned long userIdx, float xpos, float ypos );
	virtual void DeleteFlag( unsigned long objectIdx );

	// 게양 포인트
	virtual void ClearFlagPoint();
	virtual void AddFlagPoint( unsigned long objectIdx, float xpos, float ypos );
	virtual void DeleteFlagPoint( unsigned long objectIdx );

	//
	void SetImagePercent( float per );
	float GetImagePercent() { return mPercent; }

	///팀원 수장 표시
	void SetPvpLeader( unsigned long userIdx );

	///석상 표시
	void ClearStone();
	void AddStone( float posX, float posY, unsigned char teamType );
	void DeleteStone( unsigned char teamType );

	
protected:
	virtual bool OnCreate( cUINodeProperty* pproperty );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );	
	virtual void OnCommand( cUINode*, unsigned int id );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnMouseLeft( const cUIPos& pos );
	virtual void UpdateRect();
	virtual void UpdateSkin();
	virtual void OnHide();
	virtual bool ContainPoint( const cUIPos& pos ) const;
	
	/// 이름정보 위치 업데이트
	virtual void UpdateNamePos();

	// 애니메이션 업데이트
	void UpdateAni( unsigned long accumTime, sAniParam* p, cUIImage* image );

	//
	void UpdateNpc( short tl, short tt, unsigned short tr, unsigned short tb );
	void UpdateParty( short tl, short tt, unsigned short tr, unsigned short tb, double xr, double yr, short heroScreenX, short heroScreenY );
	void UpdatePortal( short tl, short tt, unsigned short tr, unsigned short tb );
	void UpdatePartyUnion( short tl, short tt, unsigned short tr, unsigned short tb );
	void UpdateFindPos( short tl, short tt, unsigned short tr, unsigned short tb );
	void UpdateFlag( short tl, short tt, unsigned short tr, unsigned short tb );
	void UpdateFlagPoint( short tl, short tt, unsigned short tr, unsigned short tb );
	void UpdateStone( short tl, short tt, unsigned short tr, unsigned short tb );

protected:
	// 관리맵
	typedef tPointerHashMap<unsigned long, void*> cDataHashMap;

	// npc
	cDataHashMap	mNpcHashMap;

	// portal
	cDataHashMap	mPortalHashMap;

	// party
	cDataHashMap	mPartyHashMap;

	// partyUnion
	cDataHashMap	mPartyUnionHashMap;

	// flag
	cDataHashMap	mFlagMap;

	// flag 게양 포인트
	cDataHashMap	mFlagPointMap;

	// 석상 이미지
	cDataHashMap	mPvPStoneMap;

	// 깃발
	//sUserImageData*	mpFlagData;

	// 깃발 게양 포인트
	//sAniImageData* mpFlagPointData;

	// 찾아가기 기능
	sAniImageData*	mpPositionData;
	
protected:
	/// 업데이트 여부 
	bool mNeedUpdate;

	/// 큰 미니맵 여부
	bool mIsWidMap;

	//
	cUIImage*	mpMapImage;
	cUIImage*	mpPosBackImage;	// 좌표부분 배경이미지
	cUIPos		mPos;			// 좌표부분 스크린좌표
	
	/// 맵 뷰 퍼센트에 따른 텍스쳐 사이즈 조정
	double mPerTexW, mPerTexH;

	/// 축소 퍼센트 ( 100 % 축소 : 전체 지도 )
	float mPercent;

	/// 화면 출력 hero 위치 
	float mOldX, mOldY;

	/// hero 데이터
	cUIImage*		mpHeroImage;
	NiPoint3		mHeroDir;
	cUISize			mHeroSize;

	/// 카메라 데이터
	cUIImage*		mpCameraImage;
	NiPoint3		mCameraDir;
	cUISize			mCameraSize;

	// 마우스 오버
	sBaseImageData*	 mpHoverData;
	
	/// 퀘스트 이미지 스킨
	cUINodeSkin*	mpQuestNew;
	cUINodeSkin*	mpQuestPlay;
	cUINodeSkin*	mpQuestComplete;
	
	cUINodeSkin*	mpNexLevelQuestNew;
	cUINodeSkin*	mpThemeQuestNew;
	cUINodeSkin*	mpThemeQuestComplete;
	cUINodeSkin*	mpRepeatQuestNew;
	cUINodeSkin*	mpRepeatQuestComplete;

	cUINodeSkin*	mPartyInSkin;
	cUINodeSkin*	mPartyOutSkin;
	cUINodeSkin*	mPvpPartyLeader;	   /// 데스매치 전장 수장표시
	cUINodeSkin*	mPvpTeamAStone;		   /// 데스매치 전장 TeamA 석상표시
	cUINodeSkin*	mPvpTeamBStone;		   /// 데스매치 전장 TeamB 석상표시

	cLabel*		mpMapName;
	cLabel*		mpChannelNum;

	//이름 출력 
	cStringT mName;
	cStringT mJobName;
	cUIPos mNamePos;
	cUIPos mJobPos;

	// 이전 수장 저장.
	unsigned long mOldPvpLeaderIndex;
};

inline void cSmallMiniMapWindow::SetImagePercent( float per )
{
	mPercent = per;

	if( mPercent > MAX_PERCENT )
		mPercent = MAX_PERCENT;
	if( mPercent < MIN_PERCENT )
		mPercent = MIN_PERCENT;
}
