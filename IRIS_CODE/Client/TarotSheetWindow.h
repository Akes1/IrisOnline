/* ==========================================================================
* 파    일 : TarotSheetWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-06-10
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ItemIcon.h"

const int TAROT_COUNT = 22;
const int SPREAD_COUNT = 5;
const int ICON_MAX = TAROT_COUNT + SPREAD_COUNT;
const int ROW_WIDTH = 143;
const int ROW_HEIGHT = 32;

class cUIImage;
class cPlaneObject;
class cTextBox;
class cLabel;
class cTarotSheetWindow : public cUIWindow
{
public:
	cTarotSheetWindow( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cTarotSheetWindow();

	/// 클리어
	void Clear();
	void ClearIcon( int i );
	
	void Open();

	/// 정보 업데이트
	void UpdateData( int i );

	/// 등록여부 검색
	unsigned long IsRegistItem( int i );
	unsigned short GetSlotIndex( int i );
	
protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender( cUIFontItemKeeper* pKeeper );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool );
	void OnLButtonDoubleClick( const cUIPos& pos );
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconRightClicked( cUINode* caller, unsigned int id, bool shift );
	void UpdateRect();
	
	/// 선택이미지 갱신
	void UpdateText();
	void UpdateSelectImage();

public:
	struct sCardBookData
	{
		cItemIcon*		mIcon;
		cIconParam		mIconParam;
		cStringT		mName;

		sCardBookData()
		{
			mIcon = 0;
		}
	};

private:
	/// 커다란 카드이미지
	cUINodeSkin*	mpCardSkin;
	cUINodeSkin*	mpLineSkin;
	cUINodeSkin*	mpBackSkin;

	cUIImage*		mpCardImage;
	cUIImage*		mpLineImage;
	cUIImage*		mpBackImage;
	
	/// 카드 데이터 
	sCardBookData	mData[ICON_MAX];

	/// 선택용 이미지
	cPlaneObject*	mpSelectImg;
	int				mSelectIndex;

	/// 설명 박스
	cTextBox*		mpDescBox;
	cLabel*			mpName;
	cLabel*			mpValue;
};
