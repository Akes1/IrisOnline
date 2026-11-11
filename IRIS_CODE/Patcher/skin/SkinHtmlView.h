/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.20
* 
* 내    용 : 게시물 뷰
* 추가내용 :
*			작업일		작업자	작업내용
*
* 주의사항 : 
*===========================================================================*/
#pragma once

//#ifndef __AFXEXT_H__
#include <afxext.h>
//#endif

#include <afxhtml.h>

class cSkinComponentResource;
class cSkinHtmlViewResource;

/// 게시물 뷰
class cSkinHtmlView : public CHtmlView
{
public:
	cSkinHtmlView();
	DECLARE_DYNCREATE(cSkinHtmlView)
	~cSkinHtmlView();

	/// 생성
	BOOL Create( cSkinComponentResource* resource, CWnd* parent, UINT id );

	/// 이동
	void Go();

protected:
	/// 리소스
	cSkinHtmlViewResource* mResource;

	/// Url
	cString mUrl;

	/// 접속 여부
	bool mConnected;

public:
	virtual HRESULT OnTranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID);
	virtual void OnDocumentComplete(LPCTSTR lpszURL);
	virtual void OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);
	virtual void OnDownloadBegin();
	virtual void OnDownloadComplete();
	virtual void OnNavigateComplete2(LPCTSTR strURL);
	virtual HRESULT OnGetHostInfo(DOCHOSTUIINFO * pInfo);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
