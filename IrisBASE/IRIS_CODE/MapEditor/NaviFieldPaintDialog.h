/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.17
* 내    용 : 네비필드 색칠 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/Dialog.h"
#include "UI/CheckButton.h"
#include "UI/CheckButtonGroup.h"
#include "UI/ColorBox.h"
#include "UI/NumericEdit.h"

#include "Resource.h"

class cMapEditorView;

/// 네비필드 색칠 다이얼로그
class cNaviFieldPaintDialog : public cDialog
{
	DECLARE_DYNAMIC(cNaviFieldPaintDialog)

public:
	cNaviFieldPaintDialog();
	virtual ~cNaviFieldPaintDialog();

	/// 갱신
	void UpdateRadius( float radius );

	/// 버튼
	void SetCheckedButton( int id );
	unsigned int GetCheckedButton() const;

	/// 반지름
	float GetRadius() const;

	/// 색
	COLORREF GetColor() const;

	/// 필드값
	void SetValue( unsigned char value );
	unsigned char GetValue() const;

protected:
	void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedButton();
	afx_msg void OnHScroll( UINT code, UINT pos, CScrollBar* bar );
	afx_msg LRESULT OnChangedValue( WPARAM id, LPARAM );

private:
	/// 버튼
	cCheckButton mColorButton;
	cCheckButton mPickButton;
	cCheckButtonGroup mGroup;

	/// 색 상자
	cColorBox mColorBox;
	COLORREF mColor[16];

	/// 숫자 에디트
	cNumericEdit mValueEdit;
};

inline
unsigned int cNaviFieldPaintDialog::GetCheckedButton() const
{
	cCheckButton* btn = mGroup.GetCheckedButton();

	if( btn == 0 )
		return 0;
	else
		return btn->GetDlgCtrlID();
}

inline
float cNaviFieldPaintDialog::GetRadius() const
{
	CSliderCtrl* slider = (CSliderCtrl*)GetDlgItem( IDC_SLIDER_NPAINT_RADIUS );
	return slider->GetPos() / 10.0f;
}

inline
COLORREF cNaviFieldPaintDialog::GetColor() const
{
	return mColorBox.GetColor();
}

inline
void cNaviFieldPaintDialog::SetValue( unsigned char i )
{
	assert( i < 16 );

	mColorBox.SetColor( mColor[i] );
	mValueEdit.SetValue( i );
}

inline
unsigned char cNaviFieldPaintDialog::GetValue() const
{
	return (unsigned char)mValueEdit.GetValue();
}
