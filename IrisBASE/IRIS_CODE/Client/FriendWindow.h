/* ==========================================================================
* 파    일 : FriendWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-07-06
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "Chat_Common.h"

const unsigned int MAXPAGE_NUM = 5;
const unsigned int FRIEND_IMGNUM = 6;

/// 친구 정보
struct sFriendData
{
	long				Idx;									// 고유번호
	unsigned long		CharacterIdx;							// 캐릭터 고유번호
	wchar_t				CharacterName[ MAX_NAME_BUFFER_SIZE ];	// 캐릭터 이름(친구)
	char				Race;									// 종족
	char				Gender;									// 성별
	char				Level;									// 레벨
	unsigned long		Job;									// 직업(ePLAYER_JOB)
	short				MapNum;									// 맵(위치)
	DWORD				ChannelNum;
	BYTE				Status;									// 친구상태(0등록완료/1등록신청)
	BYTE				Block;									// 차단상태(0차단안함/1차단함)
};

class cFriendListbox;
class cUIImage;
class cButton;
class cFriendWindow : public cUIWindow
{
public:
	cFriendWindow();
	virtual ~cFriendWindow();

	virtual void Open() {}

	void Clear();
	bool AddFriend( sFriendData* data );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnRadioPressed( cUINode* pcaller, unsigned int id );
	void OnListBoxChanged( cUINode* caller, unsigned int id );
	void UpdateSkin();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

	char GetFriendImage( char race, char gender );

	void Update();
	void UpdateButton();

private:
	cUINodeSkin*	mpNodeSkin[FRIEND_IMGNUM];

	/// 친구기능 버튼들
	cFriendListbox*		mpFriendList;
	cButton* mParty;
	cButton* mAddFriend;
	cButton* mDelFriend;
	cButton* mWhisper;

	/// 페이지 버튼
	cUINode*		mpRadioNode[ MAXPAGE_NUM ];

};

//////////////////////////////////////////////////////////////////////////////////////////
#include "StyleListBox.h"

/// 일반 리스트박스와 다르게 컬럼을 아랫줄에 출력하기 위해
/// 출력부가 변경된 리스트박스
class cFriendListbox : public cImageListBox
{
public:
	cFriendListbox( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cFriendListbox();

protected:
	/// 리스트 박스의 아이템을 그려줌.
	virtual void RenderData( cUIFontItemKeeper* pKeeper );
};
