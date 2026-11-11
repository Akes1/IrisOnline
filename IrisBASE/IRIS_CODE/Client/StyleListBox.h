#pragma once
#include "ListBox.h"

struct sListImageData : public sData
{
	cUIImage*		mpImage;

	sListImageData()
	{
		mpImage = 0;
	}
};

// 이미지 리스트박스
class cImageListBox : public cListBox
{
public:
	cImageListBox( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cImageListBox();

	virtual void Clear();

	virtual int AddRow( LPCTSTR str, unsigned long color, cUIImage* image, void* pExtraData = 0 );
	virtual bool AddColume( unsigned int rowIdx, LPCTSTR str, unsigned long color );

protected:
	virtual void OnMouseMove( const cUIPos& pos );
	virtual	void OnMouseLeft( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void RenderData( cUIFontItemKeeper* pKeeper );
};


//----------------------------------------------------------
// 체크박스용 리스트 박스
// eSTYLE_CHECKBOX
struct sListCheckData : public sListImageData
{
	cUINodeSkin*	mpCheckSkin;
	cUINodeSkin*	mpNoCheckSkin;
	bool			mCheck;

	sListCheckData()
	{
		mpCheckSkin = 0;
		mpNoCheckSkin = 0;
		mCheck = false;
	}
};

//
class cCheckBoxListBox : public cImageListBox
{
public:
	cCheckBoxListBox( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cCheckBoxListBox();

	virtual void Clear();

	virtual int AddRow( LPCTSTR str, unsigned long color, cUIImage* image, cUINodeSkin* checkSkin, cUINodeSkin* nocheckSkin, void* pExtraData = 0 );
	virtual bool AddColume( unsigned int rowIdx, LPCTSTR str, unsigned long color );

	void Check( bool check, unsigned int row );
	bool GetCheck( unsigned int row );

protected:
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void RenderData( cUIFontItemKeeper* pKeeper );
};

//----------------------------------------------------------
// 이미지폰트 및 이미지 리스트박스
// eSTYLE_IMAGEANDFONT
struct sListImageAndFontData : public sListImageData
{
	cUIImage*		mpEdgeImage;		/// 테두리이미지
	cUIImage*		mpSealImage;		/// 봉인 이미지
	cUIImageFont*	mpFont;
	bool			mShowFont;
	unsigned long	mCount;

	sListImageAndFontData()
	{
		mpFont = 0;
		mShowFont = true;
		mCount = 0;
		mpEdgeImage = 0;
		mpSealImage = 0;
	}
};

//
class cImageAndFontDataListBox : public cImageListBox
{
public:
	cImageAndFontDataListBox( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cImageAndFontDataListBox();

	virtual void Clear();

	virtual int AddRow( LPCTSTR str, unsigned long color, cUIImage* image, cUIImage* edgeImage, cUIImage* sealImage, cUIImageFont* font, 
						bool showfont, unsigned long count, void* pExtraData = 0 );

	virtual bool AddColume( unsigned int rowIdx, LPCTSTR str, unsigned long color );

protected:
	virtual void RenderData( cUIFontItemKeeper* pKeeper );
};
