/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.25
* 내    용 : 패처 어플리케이션
* 주의사항 : 
*===========================================================================*/
#pragma once

#ifndef __AFXWIN_H__
#Error "PCH에 대해 이 파일을 포함하기 전에 'StdAfx.h'를 포함합니다"
#endif


#pragma data_seg("shared")
long mUsageCnt = 0;
#pragma data_seg()
#pragma comment(linker, " /section:shared,rws")

#include "resource.h"
#include "skin/SkinResource.h"

/// 패처 어플리케이션
class cPatcherApp : public CWinApp
{
public:
	cPatcherApp();
	~cPatcherApp();

	/// 스킨 정보 로딩
	bool LoadSkin();

	/// 스킨 리소스를 리턴
	cSkinManager* GetSkinResource() const;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

private:
	/// 뮤텍스 검사
	bool CheckMutex();

public:
	/// 뮤텍스
	HANDLE mMutex;

private:
	/// 스킨
	cSkinManager* mSkinManager;

	/// 경로
	cString mPath;

	/// 명령인수
//	cString mCmdLine;
//	cString mIP;
};

inline
cSkinManager* cPatcherApp::GetSkinResource() const
{
	return mSkinManager;
}
