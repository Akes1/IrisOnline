/* ==========================================================================
* 파    일 : TarotManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-07-30
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Tarot_Common.h"

enum eTarotType
{
	eTAROT_NONE = 0,

	eTAROT_USER,			// 일반유저
	eTAROT_SELF,			// 일반유저 - SELF
	eTAROT_TUTORIAL,		// 튜토리얼용
};

enum eTarotPage
{
	ePAGE_NONE = 0,
	ePAGE_OPEN,
	ePAGE_MANAGED,
	ePAGE_VIEW,
	ePAGE_SELECT,
	ePAGE_RESULT,

	ePAGE_NPCVIEW,		/// NPC 점집 열기
	ePAGE_NPCSELECT,	/// NPC 점집 카드 선택페이지
	ePAGE_NPCRESULT,	/// NPC 점집 결과ePAGE_SELFVIEW

	ePAGE_SELFVIEW,		// 혼자 보기
};

enum
{
	ERROR_NONE = 0, 

	ERROR_CARD,		/// 카드 부족
	ERROR_SPREAD,	/// 스프레드 부족
	ERROR_PRICE,	/// 가격 없음
	ERROR_TITLE,	/// 상점명없음
	ERROR_STUFF,
	ERROR_MAX_MONEY /// 
};

enum
{
	eSPREAD_ALL = 0,
	eSPREAD_STR,
	eSPREAD_INV,
};


///
struct sResultInfo
{
	BYTE							Dir;			// 타로카드 방향
	unsigned long					TarotCardIndex;	// 아이템 정의 인덱스
	unsigned long                   Value;			// 타로카드 값(스프레드 적용후)
	bool							Apply;			//
};

/// 페이지 정보
enum eNpcTarotPage
{
	eNPCPAGE_NONE = 0,
	eNPCPAGE_CHECK,
	eNPCPAGE_SELECT,
	eNPCPAGE_RESULT,
};

// 운세속성
enum eFortuneProp
{
	eFORTUNE_LOVE,	// 애정
	eFORTUNE_HEALTH,// 건강
	eFORTUNE_WORK,	// 직장
	eFORTUNE_MONEY,	// 재물
	eFORTUNE_LUCK,	// 행운

	eFORTUNE_NONE,
};

// 위치 속성
enum ePositionProp
{
	ePOS_FIRST,		// 첫번째
	ePOS_SECOND,	// 두번째
	ePOS_THIRD,		// 세번째

	ePOS_NONE,
};

struct sNpcSpreadParam
{
	int mX, mY;			/// 좌표
	short mPositionProp;	/// 위치 속성
	long mQuestionIdx;		/// 질문 인덱스
};

// npc 스프레드 속성
struct sNpcSpread
{
	sNpcSpreadParam		mParam[MAX_NPCSPREAD_COUNT];	
	unsigned int		mCount;					/// 들어있는 스프레드 정보 갯수
	short				mFortuneProp;			/// 운세 속성

	sNpcSpread()
	{
		memset( mParam, 0, sizeof(mParam) );
		mCount = 0;
	}
};

/// npc 타로카드
struct sNpcTarotCard
{
	unsigned int	mImageIndex;
	short			mNumber;			/// 고유번호( 0 ~ 21 )
	unsigned long	mStraightIdx;		/// 정방향 설명 인덱스 
	unsigned long	mInverseIdx;		/// 역방향 설명 인덱스	
	unsigned long	mCommnetIndex;		/// 해석 인덱스
};

// 타로카드 해석
struct sNpcTarotComment
{
	unsigned long mComment[5][3][2][3]; /// [운세속성][위치속성][방향][해석] = 해석번호

	sNpcTarotComment()
	{
		memset( mComment, 0, sizeof(mComment) );
	}
};

/// 타로카드로 받을수 있는 최대 운세+카드 수치
const int MAX_TAROTVALUE = 200;

class cUIImage;
class cCardParam;
class cTarotManager
{
public:
	static cTarotManager*	mSingleton;
    
public:
	cTarotManager();
	~cTarotManager();

	/// 초기화
	bool Init();
	void Clear();

	void ChangePage( unsigned char page );

	bool IsUsingTarot();
	unsigned int CheckOpenTarot();

	// 리더
	void TarotReadyLeader();	// 리더 오픈 준비

	// 이용자
	bool TarotTutorialOpen( unsigned long npcIndex );	// 튜토리얼 점집 이용	

	// self 리더
	bool TarotSelfOpen();

	/// 네트워크 메시지
	void NetworkMsgParser( char* msg );

	void ShowErrorMsg( int error );

	/// 이용자가 선택한 카드 인덱스 획득( 인자 : 배열인덱스, 결과 : 카드인덱스, 카드방향 )
	sResultInfo* GetSelectCardInfo( unsigned int arrayidx );

	/// 타롯이미지파람 (해당 아이템이 가진 이미지 획득)
	cCardParam* GetTarotParam( unsigned int itemIdx );

	/// 스크립트 인덱스로 타롯 이미지 파람 획득
	cCardParam* GetImageParam( unsigned int imgIndex );

	inline eTarotPage	GetCurrentPage() { return mCurrentPage; }
	inline eTarotType	GetTarotType()	{ return mTarotType; }

	sNpcSpread* GetNpcSpread( unsigned long spreadIdx );
	sNpcTarotCard* GetNpcTarot( unsigned long cardIdx );
	sNpcTarotComment* GetNpcTarotComment( unsigned long commentIdx );

protected:
	/// 스크립트 로드
    bool LoadTarotParams( const cString& pathName );
	bool LoadNpcSpread( const cString& path );
	bool LoadComment( const cString& path );
	bool LoadTarotDeck( const cString& path );
	void ClearCardinfo();

public:
	static cTarotManager* GetSingleton();

protected:
	/// 스프레드 정보 (sNpcSpread)
	typedef tPointerHashMap<unsigned long, void*> cNpcSpreadMap;
	cNpcSpreadMap	mNpcSpreadMap;

	/// 타로 카드 정보 (sNpcTarotCard)
	typedef tPointerHashMap<unsigned long, void*> cNpcTarotMap;
	cNpcTarotMap	mNpcTarotMap;

	/// 해석 정보 (sNpcTarotComment)
	typedef tPointerHashMap<unsigned long, void*> cNpcCommentMap;
	cNpcCommentMap		mNpcCommentMap;

protected:
	/// 타로카드 텍스쳐
	typedef tHashMap<unsigned int, void*> cTextureMap;
	cTextureMap mTarotTextureMap;

	/// 타로카드 이미지 파람
	typedef tHashMap<unsigned int, void*> cTarotParamMap;
	cTarotParamMap mTarotParamMap;
	
	/// 선택된 타로카드 인덱스 (이용자) (sTarotResult)
	typedef tArray<void*> cSelectInfoArr;
	cSelectInfoArr mSelectInfoArr;
	
	///
	eTarotPage			mCurrentPage;

	// 점집 타입 - 이용중인 타롯점집 타입
	eTarotType			mTarotType;
};

inline sResultInfo* cTarotManager::GetSelectCardInfo( unsigned int arrayidx )
{
	if( arrayidx >= mSelectInfoArr.GetSize() )
	{
		return 0;
	}

	return (sResultInfo*)mSelectInfoArr[arrayidx];
}

inline bool cTarotManager::IsUsingTarot()
{
	return (mCurrentPage != ePAGE_NONE);
}


inline cTarotManager* cTarotManager::GetSingleton()
{
	return mSingleton;
}

#define TAROTMAN cTarotManager::GetSingleton()
