/* ==========================================================================
* 파    일 : NoteWindow.h
* 목    적 : 쪽지 윈도우 ( 임시 )
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-15
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

const unsigned int MAXNUM = 5;
class cStyleListBox;
class cSendNoteWindow;
class cReceiveNoteWindow;
class cButton;
class cUIImage;
class cRadioButton;
struct sNoteList;
class cNoteWindow : public cUIWindow
{
public:
	cNoteWindow();
	virtual ~cNoteWindow();

	///
	void Clear();
	void AddNote( sNoteList* data );
	void UpdateButton();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnRadioPressed( cUINode* pcaller, unsigned int id );
	void OnListBoxSelected( cUINode* pcaller, unsigned int id );
	void UpdateSkin();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

protected:
	/// 쪽지 삭제 
	void DeleteNote();

private:
	/// sStyleCheckBox 저장정보
	typedef tList<void*> cStyleCheckList;
	cStyleCheckList		mStyleCheckList;

	///
	cStyleListBox*		mpNoteList;

	/// 페이지 버튼
	cRadioButton*		mpRadioNode[ MAXNUM ];
	cButton*			mpDelButton;

	/// 버튼들의 정렬을 위한 영역
	cUIRect			mCheckRect;
};