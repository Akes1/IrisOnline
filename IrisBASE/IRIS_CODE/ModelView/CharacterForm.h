#pragma once


class cRollupWindow;


class cCharModelDlg;
class cCharAniDlg;
class cCharDramaDlg;

// cCharacterForm 폼 뷰입니다.

class cCharacterForm : public CFormView
{
	static cCharacterForm* mSingleton;

	DECLARE_DYNCREATE(cCharacterForm)

public:
	cCharacterForm();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~cCharacterForm();

	void Init();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void UpdateAnimationList( unsigned char race, unsigned char gender);
	void UpdateDrama();

	unsigned char GetCurrentRace();
	unsigned char GetCurrentGender();
	unsigned char GetCurrentWeaponState();

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()

public:
	/// 단일체를 리턴
	static cCharacterForm* GetSingleton();

protected:
	/// 두루마리 윈도우
	cRollupWindow*	mRollupWnd;

	cCharModelDlg*	mpModelDlg;
	cCharAniDlg*	mpAniDlg;
	cCharDramaDlg*	mpDramaDlg;


public:
};

inline
cCharacterForm* cCharacterForm::GetSingleton()
{
	return mSingleton;
}

#define CHARACTERFORM cCharacterForm::GetSingleton()

