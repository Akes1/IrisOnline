#pragma once

class cUIFontItemKeeper
{
public:
	cUIFontItemKeeper();
	~cUIFontItemKeeper();

	void*			operator new( size_t n ); 
	void			operator delete( void* ptr );

	void AddFontItem( unsigned short fontIdx, const wchar_t* str, int posX, int posY, unsigned long color, bool outerLine = false );

	void DrawAll();

	inline bool IsEmpty() { return mList.IsEmpty(); }

	struct sUIFontItem
	{
		unsigned short	mFontIdx;
		TCHAR			mStr[256]; 
		int				mStrLen;
		POINT			mPos; 
		unsigned long	mColor; 
		bool			mOuterLine;

		sUIFontItem() : mFontIdx(0), mStrLen(0), mColor(0)
		{
			mPos.x = 0; mPos.y = 0;
			::memset( mStr, 0, sizeof(mStr) );
			mOuterLine = false;
		}
	};

private:
	typedef tPool< sUIFontItem >	cUIFontItemPool;
	static cUIFontItemPool mPool;

//	typedef tList<sUIFontItem*> cUIFontItemList;
	typedef tList<void*> cUIFontItemList;
	cUIFontItemList mList;
};
