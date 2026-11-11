/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.08
* 내    용 : 조명 속성 다이얼로그
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Resource.h"
#include "UI/ColorBox.h"
#include "UI/NumericEdit.h"

class cLightSceneNode;

/// 조명 속성 다이얼로그
class cLightPropertyDialog : public CDialog
{
	DECLARE_DYNAMIC(cLightPropertyDialog)

public:
	cLightPropertyDialog();
	virtual ~cLightPropertyDialog();

	/// 조명 정보를 설정
	void UpdateLight( const NiColor& ambient,
		const NiColor& diffuse0, const NiColor& diffuse1, float diffuseAnimTime,
		float constant0, float constant1, float constantAttenAnimTime,
		float linear, float quadric );

	/// 활성화 여부를 설정
	void SetEnabled( bool enabled );

protected:
	virtual void DoDataExchange( CDataExchange* dx );
	BOOL OnInitDialog();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnClickedAmbientBox();
	afx_msg void OnClickedDiffuse0Box();
	afx_msg void OnClickedDiffuse1Box();
	afx_msg LRESULT OnChangedAttenuation( WPARAM id, LPARAM );

private:
	/// 색상
	cColorBox mAmbientBox;
	cColorBox mDiffuse0Box;
	cColorBox mDiffuse1Box;
	cNumericEdit mDiffuseAnimTimeEdit;

	/// 감쇠
	cNumericEdit mConstant0Edit;
	cNumericEdit mConstant1Edit;
	cNumericEdit mConstantAttenAnimTimeEdit;
	cNumericEdit mLinearEdit;
	cNumericEdit mQuadricEdit;
};

inline
void cLightPropertyDialog::SetEnabled( bool enabled )
{
	if( enabled )
	{
		/// 색상
		mAmbientBox.EnableWindow( TRUE );
		mDiffuse0Box.EnableWindow( TRUE );
		mDiffuse1Box.EnableWindow( TRUE );

		if( mDiffuse0Box.GetColor() != mDiffuse1Box.GetColor() )
			mDiffuseAnimTimeEdit.SetEnabled( true );
		else
			mDiffuseAnimTimeEdit.SetEnabled( false );

		/// 감쇠
		mConstant0Edit.SetEnabled( true );
		mConstant1Edit.SetEnabled( true );

		if( mConstant0Edit.GetValue() != mConstant1Edit.GetValue() )
			mConstantAttenAnimTimeEdit.SetEnabled( true );
		else
			mConstantAttenAnimTimeEdit.SetEnabled( false );

		mLinearEdit.SetEnabled( true );
		mQuadricEdit.SetEnabled( true );
	}
	else
	{
		mAmbientBox.SetColor( 0 );
		mAmbientBox.EnableWindow( FALSE );
		mDiffuse0Box.SetColor( 0 );
		mDiffuse0Box.EnableWindow( FALSE );
		mDiffuse1Box.SetColor( 0 );
		mDiffuse1Box.EnableWindow( FALSE );

		mDiffuseAnimTimeEdit.SetValue( 0.0f );
		mDiffuseAnimTimeEdit.SetEnabled( false );

		mConstant0Edit.SetValue( 0.0f );
		mConstant0Edit.SetEnabled( false );
		mConstant1Edit.SetValue( 0.0f );
		mConstant1Edit.SetEnabled( false );
		mConstantAttenAnimTimeEdit.SetValue( 0.0f );
		mConstantAttenAnimTimeEdit.SetEnabled( false );

		mLinearEdit.SetValue( 0.0f );
		mLinearEdit.SetEnabled( false );
		mQuadricEdit.SetValue( 0.0f );
		mQuadricEdit.SetEnabled( false );
	}
}
