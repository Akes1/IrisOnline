/* ==========================================================================
* 파    일 : ChannelWindow.h
* 목    적 : 캐릭터 선택화면의 채널윈도우  
* 작 성 자 : 김가람
* 작 성 일 : 2007-04-10
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

const unsigned int MAXPAGE_NUM = 5;

class cListBox;
class cButton;
class cChannelWindow : public cUIWindow
{
public:
	cChannelWindow();
	virtual ~cChannelWindow();

	void Open();

	/// 채널데이터 클리어
	void Clear();

	/// 서버 리스트 삽입
	void AddChannel( long param, int status, LPCTSTR channelname, LPCTSTR condition, unsigned long color, bool select = false );

	/// 채널 정보
	long GetSelectedChannelParam();

	/// 채널상태
	int GetSelectedChannelStatus();

	void UpdateButton();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnListBoxSelected( cUINode* pcaller, unsigned int id );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void UpdateSkin();
	void OnShow();

	//
	void CheckConnect();

private:
	/// 채널 정보와 채널 이름 매칭 정보 
	struct sChannelData
	{
		long		 mParam;		
		unsigned int mStatus;

		sChannelData() : mParam( 0 ), mStatus( 0 ){}
	};

	typedef tArray<void*> cChannelData;
	cChannelData mChannelData;

	/// 채널 리스트 박스 
	cListBox*	mpChannelList;
	cButton*	mpConnectBtn;

	/// 페이지 버튼
	cUINode*		mpRadioNode[ MAXPAGE_NUM ];

	/// 버튼들의 정렬을 위한 영역
	cUIRect			mCheckRect;
};

