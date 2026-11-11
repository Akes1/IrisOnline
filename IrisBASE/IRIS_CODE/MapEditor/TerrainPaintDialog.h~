/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.17
* 내    용 : 지형 색칠 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/Dialog.h"
#include "UI/Button.h"
#include "UI/CheckButton.h"
#include "UI/CheckButtonGroup.h"
#include "UI/ColorBox.h"
#include "UI/ColorBoxGroup.h"
#include "UI/ImageButton.h"
#include "UI/ImageButtonGroup.h"

#include "Resource.h"

class cBox;
class cTileGroup;
class cTerrainTexture;

/// 텍스처 조합
class cTexBlend
{
public:
	cTexBlend( int texIndex0, int texIndex1, int texIndex2 );

public:
	int mIndex0;
	int mIndex1;
	int mIndex2;
};

inline
cTexBlend::cTexBlend( int i0, int i1, int i2 )
: mIndex0( i0 )
, mIndex1( i1 )
, mIndex2( i2 )
{
}

/// 지형 색칠 다이얼로그
class cTerrainPaintDialog : public cDialog
{
	DECLARE_DYNAMIC(cTerrainPaintDialog)

public:
	cTerrainPaintDialog();
	virtual ~cTerrainPaintDialog();

	/// 갱신
	void UpdateInnerRadius( float radius );
	void UpdateOuterRadius( float radius );
	void UpdateTileGroup( cTileGroup* terrain );
	void UpdateColor( unsigned char red, unsigned char green, unsigned char blue );
	void UpdateTextures();

	/// 팔레트
	bool LoadPalette( const cString& pathName );
	bool SavePalette( const cString& pathName );

	/// 버튼
	void SetCheckedButton( int id );
	unsigned int GetCheckedButton() const;

	/// 불투명도 (단위: percent)
	unsigned int GetOpacity() const;

	/// 반지름 (단위: meter)
	float GetInnerRadius() const;
	float GetOuterRadius() const;

	/// 알파 (단위: percent)
	void IncreaseAlpha0();
	void IncreaseAlpha1();
	void IncreaseAlpha2();
	float GetAlpha0() const;
	float GetAlpha1() const;
	float GetAlpha2() const;

	/// 블러 비율 (단위: percent)
	unsigned int GetBlurRatio() const;
	COLORREF GetColor() const;

	///
	bool GetApplyDetailTex() const;
	unsigned int GetDetailTexIndex0() const;
	unsigned int GetDetailTexIndex1() const;
	unsigned int GetDetailTexIndex2() const;

protected:
	void SelectTextureIcon( int texIndex0, int texIndex1, int texIndex2 );
	void SelectTextureImage( int texIndex0, int texIndex1, int texIndex2 );
	void SelectTextureName( int texIndex0, int texIndex1, int texIndex2 );
	void SelectTextureBlend( int texIndex0, int texIndex1, int texIndex2 );

protected:
	void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCheckedVisibility();
	afx_msg void OnClickedButton();
	afx_msg void OnClickedTexIcon();
	afx_msg void OnClickedLoadPalette();
	afx_msg void OnClickedSavePalette();
	afx_msg void OnClickedRefreshTexBlendList();
	afx_msg void OnClickedSetDetailTextures();
	afx_msg void OnClickedSetDetailTexturesToAll();
	afx_msg void OnClickedUnLoadTexture0();
	afx_msg void OnClickedUnLoadTexture1();
	afx_msg void OnClickedUnLoadTexture2();
	afx_msg void OnSelchangeTextureBlend();
	afx_msg void OnHScroll( UINT code, UINT pos, CScrollBar* bar );

private:
	/// 버튼
	cCheckButton mDetailButton;
	cCheckButton mBlurAlphaButton;
	cCheckButton mShapenAlphaButton;
	cCheckButton mColorButton;
	cCheckButton mBlurColorButton;
	cCheckButton mLighterButton;
	cCheckButton mDarkerButton;
	cCheckButton mBrushButton;
	cCheckButton mPickButton;
	cCheckButtonGroup mGroup;

	cButton mSavePaletteButton;
	cButton mLoadPaletteButton;

	cButton mRefreshTexBlendList;

	cButton mSetDetailTexturesToTilesButton;
	cButton mSetDetailTexturesToAllButton;
	cButton mDeleteTextureButton[3];

	/// 색 상자
	cColorBox mColorBox[24];
	cColorBoxGroup mColorBoxGroup;

	/// 텍스처 아이콘
	cImageButton mTexIcon[3][20];
	cImageButtonGroup mTexIconGroup[3];

	cImageButton mDetailIcon[3];

	/// 알파
	float mAlpha0;
	float mAlpha1;
	float mAlpha2;

	/// 디테일 텍스처 리스트
	typedef tMap<unsigned int, cTexBlend> cTexBlendMap;
	cTexBlendMap mTexBlendMap;

	typedef tArray<cTexBlend> cTexBlendArray;
	cTexBlendArray mTexBlendArray;
};

inline
unsigned int cTerrainPaintDialog::GetCheckedButton() const
{
	cCheckButton* btn = mGroup.GetCheckedButton();

	if( btn == 0 )
		return 0;
	else
		return btn->GetDlgCtrlID();
}

inline
unsigned int cTerrainPaintDialog::GetOpacity() const
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_OPACITY );
	return slider->GetPos();
}

inline
float cTerrainPaintDialog::GetInnerRadius() const
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_INNER_RADIUS );
	return slider->GetPos() / 10.0f;
}

inline
float cTerrainPaintDialog::GetOuterRadius() const
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_OUTER_RADIUS );
	return slider->GetPos() / 10.0f;
}

inline
float cTerrainPaintDialog::GetAlpha0() const
{
	return mAlpha0;
}

inline
float cTerrainPaintDialog::GetAlpha1() const
{
	return mAlpha1;
}

inline
float cTerrainPaintDialog::GetAlpha2() const
{
	return mAlpha2;
}

inline
unsigned int cTerrainPaintDialog::GetBlurRatio() const
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_TPAINT_BLUR_RATIO );
	return slider->GetPos();
}

inline
COLORREF cTerrainPaintDialog::GetColor() const
{
	int i = mColorBoxGroup.GetCheckedBoxIndex();
	assert( i >= 0 );

	return mColorBox[i].GetColor();
}

inline
bool cTerrainPaintDialog::GetApplyDetailTex() const
{
	CButton* check = (CButton*)GetDlgItem( IDC_CHECK_TPAINT_APPLY_DETAIL_TEX );
	return check->GetCheck() != 0;
}

inline
unsigned int cTerrainPaintDialog::GetDetailTexIndex0() const
{
	int i = mTexIconGroup[0].GetCheckedButtonIndex() + 1;

	if( i < 0 )
		i = 0;
	return i;
}

inline
unsigned int cTerrainPaintDialog::GetDetailTexIndex1() const
{
	int i = mTexIconGroup[1].GetCheckedButtonIndex() + 1;

	if( i < 0 )
		i = 0;
	return i;
}

inline
unsigned int cTerrainPaintDialog::GetDetailTexIndex2() const
{
	int i = mTexIconGroup[2].GetCheckedButtonIndex() + 1;

	if( i < 0 )
		i = 0;
	return i;
}
