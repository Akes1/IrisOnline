/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 오브젝트 배치 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/Dialog.h"
#include "UI/CheckButton.h"
#include "UI/CheckButtonGroup.h"
#include "UI/NumericEdit.h"

class cObjectGroup;

/// 오브젝트 배치 다이얼로그
class cObjectTransformDialog : public cDialog
{
	DECLARE_DYNAMIC(cObjectTransformDialog)

public:
	cObjectTransformDialog();
	~cObjectTransformDialog();

	void SetCheckedButton( int id );
	unsigned int GetCheckedButton() const;

	/// 갱신
	void UpdateObject( cObjectGroup* obj );

	/// 활성화 여부
	void SetEnabled( bool enabled );

	/// 위치
	NiPoint3 GetTranslate() const;

	/// 회전
	void GetRotate( float* xangle, float* yangle, float* zangle ) const;

	/// 배율
	float GetScale() const;

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

	virtual void OnOK() {}
	virtual void OnCancel() {}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedButton();
	afx_msg LRESULT OnChangedTransform( WPARAM id, LPARAM );

private:
	/// 버튼
	cCheckButton mPickButton;
	cCheckButton mAttachToTerrainButton;
	cCheckButton mAttachToObjectButton;
	cCheckButtonGroup mButtonGroup;

	/// 숫자 에디트
	cNumericEdit mTxEdit;
	cNumericEdit mTyEdit;
	cNumericEdit mTzEdit;
	cNumericEdit mRxEdit;
	cNumericEdit mRyEdit;
	cNumericEdit mRzEdit;
	cNumericEdit mSxEdit;
};

inline
void cObjectTransformDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		mTxEdit.SetEnabled( true );
		mTyEdit.SetEnabled( true );
		mTzEdit.SetEnabled( true );
		mRxEdit.SetEnabled( true );
		mRyEdit.SetEnabled( true );
		mRzEdit.SetEnabled( true );
		mSxEdit.SetEnabled( true );
	}
	else
	{
		mTxEdit.SetValue( 0.0f );
		mTyEdit.SetValue( 0.0f );
		mTzEdit.SetValue( 0.0f );
		mRxEdit.SetValue( 0.0f );
		mRyEdit.SetValue( 0.0f );
		mRzEdit.SetValue( 0.0f );
		mSxEdit.SetValue( 0.0f );

		mTxEdit.SetEnabled( false );
		mTyEdit.SetEnabled( false );
		mTzEdit.SetEnabled( false );
		mRxEdit.SetEnabled( false );
		mRyEdit.SetEnabled( false );
		mRzEdit.SetEnabled( false );
		mSxEdit.SetEnabled( false );
	}
}

inline
NiPoint3 cObjectTransformDialog::GetTranslate() const
{
	float x = mTxEdit.GetValue() * 100;
	float y = mTyEdit.GetValue() * 100;
	float z = mTzEdit.GetValue() * 100;
	return NiPoint3( x, y, z );
}

inline
void cObjectTransformDialog::GetRotate( float* xangle, float* yangle, float* zangle ) const
{
	*xangle = D3DXToRadian( mRxEdit.GetValue() );
	*yangle = D3DXToRadian( mRyEdit.GetValue() );
	*zangle = D3DXToRadian( mRzEdit.GetValue() );
}

inline
float cObjectTransformDialog::GetScale() const
{
	return mSxEdit.GetValue();
}
