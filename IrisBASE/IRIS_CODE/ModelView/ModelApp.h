#pragma once

#ifndef __AFXWIN_H__
#error PCH에서 이 파일을 포함하기 전에 'stdafx.h'를 포함하십시오.
#endif

#include "Resource.h"

/// 어플리케이션
class cModelApp : public CWinApp
{
public:
	cModelApp();

public:
	virtual BOOL InitInstance();

	const cString& GetBaseDir() const;
	CSplitterWnd* GetMainSplitter();

private:
	/// 뮤텍스 검사
	bool CheckMutex();

	BOOL OnIdle( LONG count );

public:
	afx_msg void OnAppAbout();
	afx_msg void OnBackColor1();
	afx_msg void OnBackColor2();
	afx_msg void OnBackColor3();
	afx_msg void OnBackColor4();
	afx_msg void OnBackColor5();
	afx_msg void OnBackColor6();
	DECLARE_MESSAGE_MAP()

private:
	/// 기반 디렉토리
	cString mBaseDir;

	CSplitterWnd* mMainSplitter;
	unsigned int mColorChecked;
public:
};

inline
const cString& cModelApp::GetBaseDir() const
{
	return mBaseDir;
}
inline
CSplitterWnd* cModelApp::GetMainSplitter()
{
	return mMainSplitter;
}


extern cModelApp theApp;