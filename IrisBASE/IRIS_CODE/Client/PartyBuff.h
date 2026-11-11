/* ==========================================================================
* 파    일 : PartyBuff.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-11-27
* 주의사항 :
*===========================================================================*/
#pragma once
#include "BuffWindow.h"

/// 파티원 버프 윈도우
class cPartyBuff : public cBuffWindow
{
public:
	cPartyBuff();
	virtual ~cPartyBuff();

	///
	void Clear();
	void UpdatePos();

	/// 파티원 버프 아이콘 세팅
	void AddBuff( unsigned long uniqueIdx, unsigned long influenceIdx );
	void DelBuff( unsigned long uniqueIdx );
	void DelDeBuff( unsigned long uniqueIdx );
	bool IsHaveBuff();

	/// 마우스 hover 상태에서 버프 아이콘이 추가/삭제로 변경되었을때 툴팁을 업데이트 시키기 위해 hover상태 다시 체크
	void UpdateMouseHover( const cUIPos &pos );

protected:
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnMouseMove( const cUIPos& pos );
	bool SetSkin( const cUINodeSkin* pskin );
	void UpdateSkin();
	void UpdateRect();
	void OnShow();
	void OnHide();

	virtual void UpdateWindowSize();

protected:
	/// 버프 정보리스트
	typedef tPointerList<void*> cBuffList;
	cBuffList	mBuffList;

	typedef tPointerList<void*> cDeBuffList;
	cDeBuffList		mDeBuffList;

	///
	cUINodeSkin*	mpBuffSkin;
	/// 아이콘 출력좌표 업데이트
	bool			mUpdatePos;
};

inline void cPartyBuff::UpdatePos()
{
	mUpdatePos = true;
}
