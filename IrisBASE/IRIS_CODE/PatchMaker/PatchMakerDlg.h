// PatchMakerDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"

class Patchmaker;

// CPatchMakerDlg 대화 상자
class CPatchMakerDlg : public CDialog
{
// 생성입니다.
public:
	CPatchMakerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_PATCHMAKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT PatchMsg( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()

private:
	BOOL BrowseFolder(TCHAR* pTitle, TCHAR* pFolderPath );
	void LoadOptionFile();
	void SaveCurrentPathOption();
	void LoadPatchList();
	
	/// 패치 메이커 실행시 눌려서는 안되는 윈도우 Enable 시키기 
	void SetEnableState( BOOL isPatchMaking );
	
public:
	afx_msg void OnBnClickedButtonWorker();
	afx_msg void OnBnClickedButtonSource();
	afx_msg void OnBnClickedButtonTarget();
	afx_msg void OnBnClickedButtonBackup();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonSourceList();
	afx_msg void OnClickDelOption();


	CEdit mEditWorker;
	CEdit mEditSource;
	CEdit mEditTarget;
	CEdit mEditBackup;
	CEdit mEditSourceList;

private:
	Patchmaker *mPatchMaker;

};
