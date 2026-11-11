/* ==========================================================================
* 파    일 : ItemScript.h
* 목    적 : 
* 작 성 자 : 임 소 연
* 작 성 일 : 2009-09-28
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Lexer.h"
#include "Parser.h"

class cItemNpcStoreLexer : public cLexer
{
public:
	cItemNpcStoreLexer( const char* buffer, unsigned long size );
};


/// 아이템 npc상점 탭 갯수
const int NPCSTORE_TAB_COUNT = 5;

enum
{
	eTOKEN_NPCSTORE = 100,	/// Npc 상점
	eTOKEN_TAB				/// 기능
};

struct sNpcItemData
{
	unsigned long			mTextIdx;
	unsigned long			mRate;
	unsigned char			mForce;
    tArray<unsigned long>   mItem;

	sNpcItemData()
	{
		mTextIdx    = 0;
        mRate       = 0;
        mForce      = 0;
	}
};

/// 감소 세력 포인트
enum eNpcItemSellType
{
	eNPC_ITEM_SELL_TYPE_MONEY_OR_COIN,
	eNPC_ITEM_SELL_TYPE_FIRE,
	eNPC_ITEM_SELL_TYPE_WATER,
	eNPC_ITEM_SELL_TYPE_WIND,
	eNPC_ITEM_SELL_TYPE_EARTH,
};

/// 아이템 Npc 상점
class cItemNpcStore
{
public:
    cItemNpcStore(void);
    ~cItemNpcStore(void);

	void Clear();
	bool Load( cParser& parser );
	bool LoadTab( sNpcItemData* pTab, cParser& parser );

    bool IsItemIdx( unsigned long itemIdx );
    bool IsItemIdx( unsigned long tabIndex, unsigned long itemIdx );

    unsigned long  GetTabCount( )   {   return mTabAry.GetSize();   }

    unsigned long   GetItemCount( unsigned long tabIndex );
	unsigned long   GetItemIndex( unsigned long tabIndex, unsigned int cellIndex );
	unsigned long   GetTextIndex( unsigned long tabIndex );
	unsigned long	GetItemSellRate(  unsigned long tabIndex );
	unsigned char   GetItemSellType( unsigned long tabIndex );

    sNpcItemData*   GetNpcItemData ( unsigned long tabIndex );


private:

    typedef tArray<void*>  cPAry;
    cPAry   mTabAry;
};


class cItemScript
{
public:

	/// 생성 소멸
	cItemScript(void);
	~cItemScript(void);

    bool                    Init();
    void                    Release();

	bool                    LoadNpcStore            ( const cString& pathName );
    cItemNpcStore*          GetItemNpcStore         ( unsigned long npcIndex ) const;

private:
	typedef tPointerHashMap<unsigned long, void*> cNpcStoreMap;
	cNpcStoreMap    mNpcStoreMap;

public:
	static cItemScript*	 mpItemScript;

};


#define ITEMSCRIPT cItemScript::mpItemScript
