#pragma once

class cRollupWindow;

class cMonModelDlg;
class cMonAniDlg;
class cMonDramaDlg;


// cMonsterForm 폼 뷰입니다.

class cMonsterForm : public CFormView
{
	static cMonsterForm* mSingleton;

	DECLARE_DYNCREATE(cMonsterForm)

public:
	cMonsterForm();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~cMonsterForm();

	void Init();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


	inline unsigned long GetBaseModelIndex() { return mBaseModelIndex; }

	unsigned int UpdateAnimationList( unsigned long modelIdx );
	void UpdateDrama();

	unsigned long GetCurrentMonsterIdx();

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()

public:
	/// 단일체를 리턴
	static cMonsterForm* GetSingleton();

protected:
	/// 두루마리 윈도우
	cRollupWindow* mRollupWnd;

	cMonModelDlg*	mpModelDlg;
	cMonAniDlg*		mpAniDlg;
	cMonDramaDlg*	mpDramaDlg;

	unsigned long	mBaseModelIndex;

public:
};

inline
cMonsterForm* cMonsterForm::GetSingleton()
{
	return mSingleton;
}

#define MONSTERFORM cMonsterForm::GetSingleton()

