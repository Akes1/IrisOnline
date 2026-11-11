/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.12
* 내    용 : 미니맵 창
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "SmallMiniMapWindow.h"

class cPlaneObject;
struct sChaseData
{
	cPlaneObject*	pImage;	// 추적 이미지
	cStringT		title;	// 퀘스트명
	int				x;
	int				y;
	int				textX;
	int				textY;
	float 			scale;

	sChaseData()
	{
		pImage = 0;
		x = 0;
		y = 0;
		textX = 0;
		textY = 0;
		scale = 1.0f;
	}
};

class cLabel;
class cPushButton;
class cCheckBox;
class cMiniMapWindow : public cSmallMiniMapWindow
{
public:
	cMiniMapWindow();
	virtual ~cMiniMapWindow();

	virtual void Open();
	virtual void Clear();

	void SetMapData();


	//
	virtual void AddNpc( sNpcData* data );
	virtual void AddPortal( unsigned long index );
	virtual void AddFindPosition( float xpos, float ypos );
	
	void SetPressExplain( bool press );
	bool IsPressExplain();

	void SetPressQuestView( bool press );
	bool IsPressQuestView();

	// 퀘스트 추적기능 이미지 갱신
	void UpdateQuestChase();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnPushButtonPressed(cUINode* , unsigned int id );
	void OnMouseMove( const cUIPos& pos );
	void OnMouseLeft( const cUIPos& pos );
	void OnLButtonDown(const cUIPos& pos, bool ctrl, bool alt, bool );
	void UpdateRect();
	void UpdateSkin();
	void OnShow();

	/// 이름정보 위치 업데이트
	void UpdateNamePos();

	//
	void ClearChaseImage();

private:
	cLabel*			mpExplain;
	cLabel*			mpQuestViewText;
	
	// 퀘스트 보기 기능
	cCheckBox*		mpQuestView;
	cPushButton*	mpPush;

	/// 마우스 좌표
	cStringT		mPosStr;

	// 퀘스트 추적 이미지
	cUINodeSkin*	mpChaseSkin;

	// 퀘스트 추적 이미지 관리자 (sChaseData)
	typedef tPointerArray<void*> cChaseImageArr;
	cChaseImageArr mChaseImageArr;
};
