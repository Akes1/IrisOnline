/* ==========================================================================
* 파    일 : ServerWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-04
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

const unsigned int MAXPAGE_NUM = 5;

class cListBox;
class cServerWindow : public cUIWindow
{
public:
	cServerWindow();
	virtual ~cServerWindow();

	void Open();
	
	/// 서버데이터 클리어
	void Clear();

	/// 서버 리스트 삽입
	void AddServer( long param, int status, LPCTSTR servername, BYTE age );
	void UpdateServer( long selServerNo );

	/// 접속버튼의 활성 비활성을 적용.
	void SetEnableConnectBtn( bool enable );

	/// 현재 선택된 서버 파라미터 리턴 
	long	GetSelectedServerParam();
	int		GetSelectedServerStatus();
	BYTE	GetSelectedServerAge();

	void UpdateButton();

	void SetEnabled( bool enabled );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnListBoxChanged( cUINode* pcaller, unsigned int id );
	void OnListBoxSelected( cUINode* pcaller, unsigned int id );
	void OnRadioPressed( cUINode* pcaller, unsigned int id );
	void UpdateSkin();
	void OnShow();

	void UpdateChannelInfo();
	void SendConnectServer();

private:
	/// 서버 정보와 서버 이름 매칭 정보 
	struct sServerData
	{
		long	mParam;		
		int		mStatus;
		cStringT mServerName;
		BYTE	mAge;

		sServerData() : mParam( 0 ), mStatus( 0 ), mAge( 0 ) {}
	};

//	typedef tArray<sServerData> cServerDataArray;
	typedef tArray<void*> cServerDataArray;
	cServerDataArray mServerData;

	/// 서버 리스트 박스 
	cListBox*	mpServerList;

	/// 페이지 버튼
	cUINode*		mpRadioNode[ MAXPAGE_NUM ];

	/// 버튼들의 정렬을 위한 영역
	cUIRect			mCheckRect;

	cUINode*		mpConnect;
	cUINode*		mpCancel;
	cUINode*		mpExit;
};
