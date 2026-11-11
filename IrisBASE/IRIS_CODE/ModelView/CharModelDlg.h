#pragma once

#include ".\Engine\Player_Common.h"

// cCharModelDlg 대화 상자입니다.

class cCharModelDlg : public CDialog
{
	DECLARE_DYNAMIC(cCharModelDlg)

public:
	cCharModelDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~cCharModelDlg();

	enum
	{
		COMBO_HAIR = 0,
		COMBO_HAIRCOLOR,
		COMBO_FACE,
		COMBO_JACKET,
		COMBO_PANTS,
		COMBO_GLOVES,
		COMBO_BOOTS,
		COMBO_WEAPON_TYPE,
		COMBO_WEAPON_KIND,
		COMBO_SHIELD,

		COMBO_HELMET,

		///
		COMBO_COS_JACKET,
		COMBO_COS_PANTS,
		COMBO_COS_GLOVES,
		COMBO_COS_BOOTS,
		COMBO_COS_HELMET,
		COMBO_COS_ACCHEAD,
		COMBO_COS_ACCBACK,

		COMBO_COS_ONEPIECE,

		COMBO_MAX,
	};

	/// 
	void OpenData();

	inline unsigned char GetRace() { return mCurRace; }
	inline unsigned char GetGender() { return mCurGender; }
	inline unsigned char GetWeaponState() { return mCurWeaponState; }

protected:
//	void UpdateComboData();

	void UpdateCharacterData( unsigned long modelIndex );
	void UpdateWeaponData();
	void UpdateCostumeData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
	afx_msg void OnBnClickedRadio5();

	/// base
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnCbnSelchangeCombo4();
	afx_msg void OnCbnSelchangeCombo5();
	afx_msg void OnCbnSelchangeCombo6();
	afx_msg void OnCbnSelchangeCombo7();
	afx_msg void OnCbnSelchangeCombo8();
	afx_msg void OnCbnSelchangeCombo9();
	afx_msg void OnCbnSelchangeCombo10();
	afx_msg void OnCbnSelchangeCombo11();

	/// costume
	afx_msg void OnCbnSelchangeCombo12();
	afx_msg void OnCbnSelchangeCombo13();
	afx_msg void OnCbnSelchangeCombo14();
	afx_msg void OnCbnSelchangeCombo15();
	afx_msg void OnCbnSelchangeCombo16();
	afx_msg void OnCbnSelchangeCombo17();
	afx_msg void OnCbnSelchangeCombo18();
	afx_msg void OnCbnSelchangeCombo19();

	DECLARE_MESSAGE_MAP()

protected:
	CComboBox	mComboArray[COMBO_MAX];

	CButton		mRaceRadio[eRACE_MAX];
	CButton		mGenderRadio[eGENDER_MAX];
	
	unsigned char		mCurRace;
	unsigned char		mCurGender;

	unsigned int mLinkWeapon[2];
	unsigned char mCurWeaponState;

	unsigned int mLinkCostume[2];
};
