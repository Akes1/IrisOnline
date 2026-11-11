/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.06.23
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

class cBuffDC
{
public:
	cBuffDC( CWnd* wnd, int width, int height, COLORREF backColor, bool transparent = true );
	virtual ~cBuffDC();

	void Draw( CDC* dc, int destX, int destY, int destWidth, int destHeight );
	void Rotate( float degree );
	void Rotate( float degree, const CPoint& origin );

	CDC* GetDC();

protected:
	CWnd* mWnd;
	int mWidth;
	int mHeight;
	COLORREF mBackColor;
	bool mTransparent;

	CBitmap mMemBitmap;
	CBitmap* mOldBitmap;
	CDC mMemDC;
};

inline
CDC* cBuffDC::GetDC()
{
	return &mMemDC;
}
