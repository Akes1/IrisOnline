/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 조명 배치 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/Dialog.h"
#include "UI/Button.h"
#include "UI/CheckButton.h"
#include "UI/CheckButtonGroup.h"
#include "UI/NumericEdit.h"

class cGroupLight;

/// 조명 배치 다이얼로그
class cLightTransformDialog : public cDialog
{
	DECLARE_DYNAMIC(cLightTransformDialog)

public:
	cLightTransformDialog();
	virtual ~cLightTransformDialog();

	void SetCheckedButton( int id );
	unsigned int GetCheckedButton() const;

	/// 조명 정보를 설정
	void UpdateLight( const NiPoint3& translate, float radius );

	/// 활성화 여부를 설정
	void SetEnabled( bool enabled );

	/// 위치
	NiPoint3 GetTranslate() const;

	/// 반지름
	float GetRadius() const;

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedButton();
	afx_msg void OnClickedCreate();
	afx_msg LRESULT OnChangedTransform( WPARAM id, LPARAM );

private:
	/// 버튼
	cButton mCreateButton;
	cCheckButton mPickButton;
	cCheckButton mAttachToTerrainButton;
	cCheckButton mAttachToObjectButton;
	cCheckButtonGroup mButtonGroup;

	/// 숫자 에디트
	cNumericEdit mTxEdit;
	cNumericEdit mTyEdit;
	cNumericEdit mTzEdit;
	cNumericEdit mRadiusEdit;
};

inline
void cLightTransformDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		mTxEdit.SetEnabled( true );
		mTyEdit.SetEnabled( true );
		mTzEdit.SetEnabled( true );
		mRadiusEdit.SetEnabled( true );
	}
	else
	{
		mTxEdit.SetValue( 0.0f );
		mTyEdit.SetValue( 0.0f );
		mTzEdit.SetValue( 0.0f );
		mRadiusEdit.SetValue( 3.0f );

		mTxEdit.SetEnabled( false );
		mTyEdit.SetEnabled( false );
		mTzEdit.SetEnabled( false );
		mRadiusEdit.SetEnabled( false );
	}
}

inline
NiPoint3 cLightTransformDialog::GetTranslate() const
{
	float x = mTxEdit.GetValue() * 100.0f;
	float y = mTyEdit.GetValue() * 100.0f;
	float z = mTzEdit.GetValue() * 100.0f;
	return NiPoint3( x, y, z );
}

inline
float cLightTransformDialog::GetRadius() const
{
	return mRadiusEdit.GetValue() * 100.0f;
}
