/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.12
* 내    용 : 숫자 에디트
* 주의사항 : 
*===========================================================================*/
#pragma once

class cNumericEdit;

/// 에디트
class cNumEdit : public CEdit
{
	DECLARE_DYNAMIC( cNumEdit )

public:
	cNumEdit();

	BOOL Create( DWORD style, const RECT& rect, CWnd* parent, UINT id );

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnKeyDown( UINT c, UINT repeats, UINT flags );
	afx_msg void OnChar( UINT c, UINT repeats, UINT flags );
//	afx_msg void OnEnChange();
//	afx_msg HBRUSH CtlColor( CDC* /*dc*/, UINT /*nCtlColor*/ );

private:
	/// 부모
	cNumericEdit* mParent;

	//BOOL   m_fOverFlow;
	//CBrush m_brError;
};

/// 스핀
class cNumSpin : public CSpinButtonCtrl
{
	DECLARE_DYNAMIC( cNumSpin )

public:
	cNumSpin();

	BOOL Create( DWORD style, const RECT& rect, CWnd* parent, UINT id );
	void SetStep( float step );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDeltapos( NMHDR* pnmhdr, LRESULT* presult );

private:
	/// 부모
	cNumericEdit* mParent;

	/// 증감값
	float mStep;
};

/// 버튼
class cNumEditButton : public CButton
{
	DECLARE_DYNAMIC( cNumEditButton )

public:
	cNumEditButton();
	BOOL Create( LPCTSTR caption, DWORD style, const RECT& rect, CWnd* parent, UINT id );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnBnSetStyle( WPARAM wparam, LPARAM lparam );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );

private:
	/// 부모
	cNumericEdit* mParent;
};

/// 팝업 윈도우
class cNumEditPopup : public CWnd
{
	DECLARE_DYNAMIC( cNumEditPopup )

public:
	cNumEditPopup();
	BOOL Create( cNumericEdit* parent, float step );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnMouseMove( UINT flags, CPoint point );
	afx_msg void OnLButtonUp( UINT flags, CPoint point );

private:
	/// 부모
	cNumericEdit* mParent;

	/// 드래그 좌표
	int mDragY;

	/// 증감값
	float mStep;
};

/// 전용 메시지
static const UINT NEN_CHANGED = ::RegisterWindowMessage(_T("NEN_CHANGED-{CD9B6FDC-F420-4956-91B0-AB882EC15A89}"));

/// 숫자 에디트
class cNumericEdit : public CWnd
{
	DECLARE_DYNAMIC( cNumericEdit )

public:
	cNumericEdit();

	BOOL Create( float min, float max, float value, float step, CWnd* parent, UINT id, DWORD style = WS_VISIBLE | WS_TABSTOP );
	void ShowPopup();
	void HidePopup();

	void SetDelta( float delta );
	void SetDeltaNotify( float delta );
	void SetValue( float value );
	void SetValueNotify( float value );
	void SetRange( float min, float max );
	void SetReadOnly( bool readOnly );
	void SetEnabled( bool enabled );

	float GetDelta() const;
	float GetValue() const;
	void GetRange( float* min, float* max ) const;
	bool IsReadOnly() const;

protected:
	void ResizeChildren( int cx, int cy );

public:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnSize( UINT type, int cx, int cy );
	//afx_msg void OnVScroll( UINT sbcode, UINT pos, CScrollBar* pscrollBar );
	afx_msg void OnSetFocus( CWnd* poldWnd );
	afx_msg void OnKeyDown( UINT c, UINT repeats, UINT flags );
	DECLARE_MESSAGE_MAP()

private:
	/// 에디트
	cNumEdit mEdit;

	/// 스핀
	cNumSpin mSpin;

	/// 버튼
	cNumEditButton mButton;

	/// 아이디
	UINT mID;

	/// 범위
	float mMin;
	float mMax;

	/// 값
	float mValue;

	/// 델타
	float mDelta;

	/// 증감값
	float mStep;

	/// 활성화 여부
	bool mEnabled;
};

inline float cNumericEdit::GetDelta() const
{
	return mDelta;
}

inline float cNumericEdit::GetValue() const
{
	return mValue;
}

inline void cNumericEdit::GetRange( float* min, float* max ) const
{
	*min = mMin;
	*max = mMax;
}

inline bool cNumericEdit::IsReadOnly() const
{
	return ( mEdit.GetStyle() & ES_READONLY ) > 0;
}
