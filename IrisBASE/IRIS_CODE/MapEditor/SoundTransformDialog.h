/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.07
* 내    용 : 사운드 배치 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UI/Dialog.h"
#include "UI/Button.h"
#include "UI/CheckButton.h"
#include "UI/CheckButtonGroup.h"
#include "UI/NumericEdit.h"

class cGroupSound;

/// 사운드 배치 다이얼로그
class cSoundTransformDialog : public cDialog
{
	DECLARE_DYNAMIC(cSoundTransformDialog)

public:
	cSoundTransformDialog();
	virtual ~cSoundTransformDialog();

	void SetCheckedButton( int id );
	unsigned int GetCheckedButton() const;

	/// 사운드 정보를 설정
	void UpdateSound( const NiPoint3& translate, float radius, float attenDist );

	/// 활성화 여부를 설정
	void SetEnabled( bool enabled );

	/// 위치
	NiPoint3 GetTranslate() const;

	/// 반지름
	float GetRadius() const;

	/// 감쇠 거리
	float GetAttenDistance() const;

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
	cNumericEdit mRadiusEdit;
	cNumericEdit mAttenDistEdit;
};

inline
void cSoundTransformDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		mTxEdit.SetEnabled( true );
		mTyEdit.SetEnabled( true );
		mTzEdit.SetEnabled( true );
		mRadiusEdit.SetEnabled( true );
		mAttenDistEdit.SetEnabled( true );
	}
	else
	{
		mTxEdit.SetValue( 0.0f );
		mTyEdit.SetValue( 0.0f );
		mTzEdit.SetValue( 0.0f );
		mRadiusEdit.SetValue( 0.0f );
		mAttenDistEdit.SetValue( 0.0f );

		mTxEdit.SetEnabled( false );
		mTyEdit.SetEnabled( false );
		mTzEdit.SetEnabled( false );
		mRadiusEdit.SetEnabled( false );
		mAttenDistEdit.SetEnabled( false );
	}
}

inline
NiPoint3 cSoundTransformDialog::GetTranslate() const
{
	float x = mTxEdit.GetValue() * 100.0f;
	float y = mTyEdit.GetValue() * 100.0f;
	float z = mTzEdit.GetValue() * 100.0f;
	return NiPoint3( x, y, z );
}

inline
float cSoundTransformDialog::GetRadius() const
{
	return mRadiusEdit.GetValue() * 100.0f;
}

inline
float cSoundTransformDialog::GetAttenDistance() const
{
	return mAttenDistEdit.GetValue() * 100.0f;
}
