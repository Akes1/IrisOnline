/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.05
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "Lexer.h"

enum eNpcFunc;
enum eNpcUIOpenType;

/// 퀘스트 진행상황에 따른 연결 페이지
struct sQuestLinkPage
{
	unsigned long newIndex;
	unsigned long playIndex;
	unsigned long completeIndex;
	unsigned long rewardIndex;

	sQuestLinkPage::sQuestLinkPage()
	{
		newIndex = 0;
		playIndex = 0;
		rewardIndex = 0;
	}
};

/// Npc 대사 및 기능
struct sPageText;
class cPage;
class cParser;
class cUINpcTelling
{
public:
	cUINpcTelling();
	~cUINpcTelling();

	bool Load( cParser& parser );

	/// 기본 대사는 랜덤 획득
	sPageText* GetDefaultTextRandomly();
	unsigned long GetDefaultScene();	/// 기본장면 인덱스 

	///
	cPage* GetPage( unsigned long pageIdx );

	/// 퀘스트 관련 페이지 정보 가져오기
	sQuestLinkPage* GetQuestLinkPage( unsigned long questIdx );

	/// 기능
	int GetFuncCount() { return mFuncCount; }
	eNpcFunc GetFunc( int i );

	/// 
	unsigned long* GetTarotArr() { return mTarotIndex; }
	unsigned long GetTarotPrice() { return mTarotPrice; }
	unsigned long GetThemeIndex() { return mThemeIndex; }
	short GetSpreadCount() { return mSpreadCount; }
	unsigned long* GetSpreadArr() { return mSpreadIndex; }
	unsigned long GetJobData1() { return mJobData[0]; }
	unsigned long GetJobData2() { return mJobData[1]; }
	unsigned long GetEventIndex() { return mEventIndex; }

protected:
	bool LoadQuestLink( cParser& parser, sQuestLinkPage* linkPage );
	bool LoadPageList( cParser& parser, unsigned long& rewardIndex );
	bool LoadPage( cParser& parser, cPage* page );

private:
	/// 기본 대사 페이지 정보
	unsigned long mDefaultTextIndex;

	/// 퀘스트 페이지 정보 맵
	typedef tPointerHashMap<unsigned long, void*> cQuestLinkPage;
	cQuestLinkPage mQuestLinkPage;

	/// 기능 - 최대 5개
	eNpcFunc mFunc[5];
	int	mFuncCount;
	
	/// 가진 스프레드 정보
	short mSpreadCount;
	unsigned long mSpreadIndex[5];
	unsigned long mTarotIndex[22];
	unsigned long mTarotPrice;

	/// 테마 던젼 인덱스 정보
	unsigned long mThemeIndex;

	// npc 이벤트 인덱스
	unsigned long mEventIndex;

	// 직업 정보 인덱스
	unsigned long mJobData[2];
};

inline eNpcFunc cUINpcTelling::GetFunc( int i )
{
	return ( i < 5 ) ? mFunc[i] : (eNpcFunc)0;
}
