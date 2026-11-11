#pragma once

#ifndef __AFXWIN_H__
	#error PCH에서 이 파일을 포함하기 전에 'stdafx.h'를 포함하십시오.
#endif

#include "Resource.h"

/// 어플리케이션
class cMapEditorApp : public CWinApp
{
public:
	cMapEditorApp();

public:
	virtual BOOL InitInstance();

	const cString& GetBaseDir() const;

private:
	/// 뮤텍스 검사
	bool CheckMutex();

	BOOL OnIdle( LONG count );

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

private:
	/// 기반 디렉토리
	cString mBaseDir;
};

inline
const cString& cMapEditorApp::GetBaseDir() const
{
	return mBaseDir;
}

extern cMapEditorApp theApp;
