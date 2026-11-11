/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.11
* 내    용 : 버튼
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 버튼
class cButton : public CButton
{
public:
	cButton();
	virtual ~cButton();

protected:
	virtual void DrawItem( LPDRAWITEMSTRUCT ds );

protected:
	void DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds );
	void DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds );
	void DrawCaption( CDC* dc, LPDRAWITEMSTRUCT ds );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd( CDC* dc );
};
