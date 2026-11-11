/* ==========================================================================
* 파    일 : TutorialManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-10-12
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Lexer.h"

//--------
// 스크립트 관련 구조체

// 기본 이미지
struct sTutorialBaseImage
{
	// 텍스쳐
	NiTexture* mpTexture;

	// 출력 위치
	short mPosX;
	short mPosY;

	// 사이즈
	short mWidth;
	short mHeight;

	// 텍스쳐 위치
	short mTexPosX;
	short mTexPosY;

	// 프레임수
	short mFrameCount;

	// 반복 설정
	bool mRepeat;

	// 한 프레임 시간( ms )
	unsigned long mOneFrameTime;

	// 이미지 정렬( 0: 없음, 1 : width, 2 : hegith, 3 : width & height )
	unsigned char mAlign;

	sTutorialBaseImage()
	{
		mpTexture = 0;
		mPosX = 0;
		mPosY = 0;
		mWidth = 0;
		mHeight = 0;
		mTexPosX = 0;
		mTexPosY = 0;
		mFrameCount = 1;
		mRepeat = true;
		mOneFrameTime = 0;
		mAlign = 0;
	}
};

// 연출 정보
enum eTutorialDrama
{
	eDRAMA_NONE,
	eDRAMA_DOWN,	// 눌림효과
	eDRAMA_SHOW,	// 보기
	eDRAMA_HIDE,	// 안 보기
};

// 화면 이미지
struct sTutorialImage : sTutorialBaseImage
{
	// 텍스쳐 위치
	short mDownTexX;
	short mDownTexY;

	// 키 입력
	unsigned long mKey;

	// 기본 보기
	bool mDefaultShow;

	// 좌표 기준
	bool mLeftTop;

	// 연출
	eTutorialDrama	mDramaInfo;

	// 눌림 플래그 처리
	bool mIsDowned;

	sTutorialImage()
	{
		mDownTexX = 0;
		mDownTexY = 0;
		mKey = 0;
		mDefaultShow = true;
		mDramaInfo = eDRAMA_NONE;
		mIsDowned = false;
		mLeftTop = true;
	}
};

// 적용 오브젝트
enum eOBJECTTYPE;
struct sTutorialObject
{
	// 타입
	eOBJECTTYPE mObjectType;

	// 적용범위 ( 0 : 한개, 1 : 전체, 2 : 타겟 )
	char mSetAll;

	// 객체 연결 이미지 인덱스 (sTutorialBaseImage)
	unsigned long mImageIndex;

	// 객체 고유 인덱스 ( 한개 적용시만 처리 )
	unsigned long mClassIndex;

	sTutorialObject()
	{
		mObjectType = (eOBJECTTYPE)0;
		mSetAll = 1;
		mImageIndex = 0;
		mClassIndex = 0;
	}
};

// 모드 구조체
class cParser;
class cTutorialMode
{
public:
	cTutorialMode();
	~cTutorialMode();

public:
	// 모드 인덱스
	unsigned long	mModeIndex;	

	// 시작좌표
	unsigned long	mStartPosIndex;

	// 부활관련 안내문 인덱스
	unsigned long	mDieTextIndex;

public:
	// 연출 적용 오브젝트 정보 (sTutorialObject*)
	typedef tPointerArray<void*> cTutorialObjectArr;
	cTutorialObjectArr mObjectArr;

	// 화면 출력 이미지 관리 (sTutorialImage*)
	typedef tPointerArray<void*> cTutorialImageArr;
	cTutorialImageArr mImageArr;

	// 객체 출력 이미지 관리 (sTutorialBaseImage*)
	typedef tPointerHashMap<unsigned long, void*> cTutorialObjectImageMap;
	cTutorialObjectImageMap mObjectImageMap;
};


class cTutorialManager
{
public:
	static cTutorialManager* mpSingleton;

public:
	cTutorialManager();
	~cTutorialManager();

	void Clear();

	bool Init();
	void Open();
	void Close();
	void Process( unsigned long deltatime, unsigned long accumtime );

	// 타겟팅 갱신
	void UpdateTarget( char oldtype, unsigned long oldobjectIdx, char type, unsigned long objectIdx );

	//
	void NetworkMsgParser( char* msg );

	//
	sTutorialBaseImage* GetObjectImage( char type, unsigned long classIndex, unsigned long objectIndex );

	//
	cTutorialMode* GetTutorialMode( unsigned long modeIdx );
	cTutorialMode* GetCurrentMode();
	bool IsTutorialMode();

protected:
	bool LoadScript( const cString& pathName );
	bool LoadImages( const cString& path, cParser& parser );
	bool LoadMode( cParser& parser, cTutorialMode* mode );
	bool LoadObject( cParser& parser, cTutorialMode* mode );
	bool LoadImage( cParser& parser, cTutorialMode* mode );
	bool LoadObjectImage( cParser& parser, cTutorialMode* mode, unsigned long index );

	NiTexture* GetTexture( unsigned int index );

	//
	void ClearOldMode();

	// 모드 변경
	void ChangeMode( int modeIndex );

public:
	static cTutorialManager* GetSingleton();

private:
	/// 텍스쳐 맵
	typedef tPointerHashMap<unsigned int, void*> cTextureMap;
	cTextureMap mTextureMap;

	// 튜토리얼 모드 관리 (cTutorialMode*)
	typedef tPointerHashMap<unsigned long, void*> cModeMap;
	cModeMap	mModeMap;

	// 연출 이미지 ID 관리 맵
	typedef tPair<unsigned long, void*> cImageDataPair;
	typedef tArray<cImageDataPair> cImageIDArr;
	cImageIDArr		mImageIDArr;

	// 모드인덱스 (-1 : 포기or완료)
	int			mModeIndex;
};

inline cTutorialMode* cTutorialManager::GetTutorialMode( unsigned long modeIdx )
{
	return (cTutorialMode*)mModeMap.GetAt( modeIdx );
}

inline cTutorialMode* cTutorialManager::GetCurrentMode()
{
	if( mModeIndex == -1 )
		return 0;

	return GetTutorialMode( mModeIndex );
}

inline NiTexture* cTutorialManager::GetTexture( unsigned int index )
{
	return (NiTexture*)mTextureMap.GetAt( index );
}

inline cTutorialManager* cTutorialManager::GetSingleton()
{
	return mpSingleton;
}

#define TUTORIALMAN cTutorialManager::GetSingleton()
