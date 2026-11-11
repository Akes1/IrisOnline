#pragma once

class cRollupWindow;
class cNpcModelDlg;
class cNpcAniDlg;

// cNpcForm 폼 뷰입니다.

class cNpcForm : public CFormView
{
	static cNpcForm* mSingleton;

	DECLARE_DYNCREATE(cNpcForm)

public:
	cNpcForm();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~cNpcForm();

	void Init();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	inline unsigned long GetBaseModelIndex() { return mBaseModelIndex; }

	unsigned int UpdateAnimationList( unsigned long modelIdx );

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnSize( UINT type, int cx, int cy );
	DECLARE_MESSAGE_MAP()

public:
	/// 단일체를 리턴
	static cNpcForm* GetSingleton();

protected:
	/// 두루마리 윈도우
	cRollupWindow* mRollupWnd;

	cNpcModelDlg*	mpModelDlg;
	cNpcAniDlg*		mpAniDlg;

	unsigned long	mBaseModelIndex;
public:
};


inline
cNpcForm* cNpcForm::GetSingleton()
{
	return mSingleton;
}

#define NPCFORM cNpcForm::GetSingleton()
