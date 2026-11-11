/* ==========================================================================
* 파    일 : Page.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-01-24
* 주의사항 :
*===========================================================================*/
#pragma once

/// 대사로 인한 결과 이벤트
enum eAnswerEvent
{
	eEVENT_NONE = 0,
	eEVENT_YES,
	eEVENT_REWARD,
	eEVENT_COMPLETE,
};

/// 기본 대사용 구조체
struct sPageText
{
	/// 텍스트 인덱스, 텍스트 줄수
	unsigned int textIndex;
	unsigned int numLine;

	sPageText::sPageText()
	{
		textIndex = 0;
		numLine = 0;
	}
};

/// 대답용 구조채
struct sPageAnswer : sPageText
{
	/// 다음페이지 인덱스
	unsigned int nextPageIndex;

	sPageAnswer::sPageAnswer()
	{
		nextPageIndex = 0;
	}
};

/// 장면 이미지 구조체
struct sSceneData
{
public:
	cString	 mTexName;

	/// 텍스처
//	NiTexture* mpTexture;
	
	/// 텍스처 위치 좌표
	unsigned int mTexPosX;
	unsigned int mTexPosY;
	
	/// 텍스처 사이즈
	unsigned int mTexWidth;
	unsigned int mTexHeight;
};

/// 페이지 정보
class cPage
{
	friend class cUINpcTelling;

public:
	cPage();
	~cPage();

	unsigned long GetNextPageIdx() { return mNextPageIndex; }
	unsigned long GetSceneIdx() { return mSceneIndex; }
	eAnswerEvent GetAnswerEvent() { return mAnswerType; }

	sPageText* GetPageText() { return (sPageText*)mDefaultTextArray[0]; }
	tArray<void*>& GetAnswerArr() { return mAnswerArr; }

private:
	/// 기본대사배열 sPageText
	typedef tArray<void*> cTextArray;
	cTextArray mDefaultTextArray;

	/// 대답 배열 sPageAnswer
	typedef tArray<void*> cAnswerArr;
	cAnswerArr mAnswerArr;

	/// 다음 페이지 ( 분기 페이지가 아니면 사용가능 )
	/// 다음이 있으면 값 세팅
	unsigned long mNextPageIndex;

	/// 대화 결과
	eAnswerEvent mAnswerType;

	/// 장면인덱스
	unsigned long mSceneIndex;

	/// 사운드 인덱스
	/// unsigned long mSoundIndex;
};

inline
cPage::cPage()
: mNextPageIndex(0)
, mAnswerType( eEVENT_NONE )
, mSceneIndex(0)
{
	mDefaultTextArray.Reserve( 10 );
	mAnswerArr.Reserve( 10 );
}

inline
cPage::~cPage()
{
	/// 기본대사배열 삭제
	for( unsigned int i = 0; i < mDefaultTextArray.GetSize(); ++i )
	{
		sPageText* pageText = (sPageText*)mDefaultTextArray[i];
		SAFE_DELETE( pageText );
	}
	mDefaultTextArray.Clear();

	/// 분기배열 삭제
	for( unsigned int i = 0; i < mAnswerArr.GetSize(); ++i )
	{
		sPageAnswer* pageAns = (sPageAnswer*)mAnswerArr[i];
		SAFE_DELETE( pageAns );
	}
	mAnswerArr.Clear();
}