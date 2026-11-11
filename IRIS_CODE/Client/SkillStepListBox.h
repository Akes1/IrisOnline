#pragma once
#include "ListBox.h"


const int gMaxSkillInconCnt	= 10;
const int gSplitCount		= 2;
const int gSplitSize		= 200;

struct sPlayerSkillBaseInfo;

struct sSkillInfoEx
{
	sPlayerSkillBaseInfo*	mpSkillInfo;
	unsigned char			mSkillStep;

	sSkillInfoEx()
	{
		::ZeroMemory( this, sizeof(sSkillInfoEx) );
	}
};

// 배운 스킬에 대한 각 단계 모두를 출력하는 리스트 박스
class cSkillIcon;
class cSkillStepListBox : public cListBox
{

public:
	cSkillStepListBox( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cSkillStepListBox();

	/// 새로운 행을 해당 분할 영역에 추가, 생성한다.
	virtual int AddRowAt( unsigned char nSplitIndex, LPCTSTR str, unsigned long color, void* pExtraData = 0 );		

	// 해당 영역에 있는 원소 갯수
	inline virtual unsigned int	GetRowCount();

	// 전체 또는 설정 영역 클리어
	void			Clear();	
	void			ClearSplitRange( unsigned char nSplitIndex );
	
	// 스킬 아이콘 생성 및 분할 영역의 갯수와 사이즈 설정 생성		
	void			CreateSkillICon( unsigned char nCount = gMaxSkillInconCnt );
	void			CreateSplitSize( unsigned char nSplitCnt, unsigned int nSplitSize );

	// 리스트 박스 Row 변경시 스킬 아이콘 재 설정
	virtual void	UpdateIConListInfo();
	
	// 현재 영역 인덱스를 설정, 획득.
	void			SetCurrentSplitIndex( unsigned char nIndex ){ mCurrentSplitIndex = nIndex; }
	unsigned char	GetCurrentSplitIndex()	const	{ return mCurrentSplitIndex; }

	// 분할 영역 갯수, 사이즈 정보 획득
	unsigned int	GetSplitSize()			const	{ return mSplitSize; }


	void			UpdateIconInfo();

	virtual void	Sort();
	
protected:

	virtual void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	virtual void OnIconDoubleClicked( cUINode* caller, unsigned int id);
	virtual void OnIconClicked( cUINode* caller, unsigned int id);
	virtual void RenderData( cUIFontItemKeeper* pKeeper );
	virtual void UpdateSkin();
	virtual void OnMouseWheel( const cUIPos& pos, int wheel );
	virtual void OnScrollbarMoved( const cUIPos& pos );
	virtual void CheckButtonEvent( unsigned int id );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );

protected:

	typedef tArray<void*> cIconArr;
	cIconArr mSkillIconArr;				// 스킬 아이콘 컨테이너

//	unsigned char	mSplitCnt;			// 분할 갯수	
	unsigned int	mSplitSize;			// 분할 영역 크기
	unsigned char	mCurrentSplitIndex;	// 현재 분할 인덱스
	
	//	분할한 영역에서 마지막으로 삽입된 위치정보를 저장한다.
	typedef tArray<unsigned int> cArray;
	cArray mSplitInfo;					// 분할 영역 정보
};
