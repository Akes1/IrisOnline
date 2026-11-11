#pragma once

#include "Lexer.h"

class cKeyboardAgent;

///※주의 : 키보드 인덱스는 path_shortcut.txt 스크립트 파일 인덱스와 동일해야 함
// enum eSHORTCUT_KEY
// {
// 	eKEY_COMMON_QUICK1  = 32,
// 	eKEY_COMMON_QUICK10 = 41,
// 	eKEY_QUICK1 = 42,
// 	eKEY_QUICK10 = 51,
// };


 enum eSHORTCUT_KEY
 {
	 eKEY_CHAR_INFO     = 1, 
	 eKEY_SKILL         = 2,	
	 eKEY_INVEN         = 3,	
	 eKEY_QUEST         = 4,	
	 eKEY_FRIEND        = 5,	
	 eKEY_GUILD         = 6,	
	 eKEY_DUNGEON       = 7,	
	 eKEY_PVP           = 8,	
	 eKEY_BOOK          = 9,	
	 eKEY_WIDE_MAP      = 10,
	 eKEY_ITEM_MALL     = 11,

	 eKEY_MOVE_UP       = 12,
	 eKEY_MOVE_DOWN     = 13,
	 eKEY_MOVE_LEFT     = 14,
	 eKEY_MOVE_RIGHT    = 15,
	 eKEY_AUTO_RUN      = 16,
	 eKEY_JUMP          = 17,
	 eKEY_TARGET        = 18,
	 eKEY_ATTACK        = 19,
	 eKEY_ITEM_PICK     = 20,
	 eKEY_WEAPON_SWAP   = 21,
	 eKEY_SITDOWN       = 22,
	 eKEY_IDLE_STATE    = 23,
	 eKEY_SCORE_BOARD   = 54,

	 eKEY_WHISPER       = 24,
	 eKEY_ANSWHISPER    = 25,
	 eKEY_CHAT_GUILD    = 26,
	 eKEY_CHAT_PARTY    = 27,
	 eKEY_CHAT_YELL     = 28,

	 eKEY_OPTION        = 29,
	 eKEY_HIDE_UI       = 30,
	 eKEY_SCREENSHOT    = 31,
	 eKEY_VIDEO         = 32,

	 eKEY_COMMON_QUICK1 = 33,
	 eKEY_COMMON_QUICK2 = 34,
	 eKEY_COMMON_QUICK3 = 35,
	 eKEY_COMMON_QUICK4 = 36,
	 eKEY_COMMON_QUICK5 = 37,
	 eKEY_COMMON_QUICK6 = 38,
	 eKEY_COMMON_QUICK7 = 39,
	 eKEY_COMMON_QUICK8 = 40,
	 eKEY_COMMON_QUICK9 = 41,
	 eKEY_COMMON_QUICK0 = 42,

	 eKEY_QUICK1        = 43,
	 eKEY_QUICK2        = 44,
	 eKEY_QUICK3        = 45,
	 eKEY_QUICK4        = 46,
	 eKEY_QUICK5        = 47,
	 eKEY_QUICK6        = 48,
	 eKEY_QUICK7        = 49,
	 eKEY_QUICK8        = 50,
	 eKEY_QUICK9        = 51,
	 eKEY_QUICK0        = 52,
	 eKEY_QUICK_SWITCH  = 53,
};

class cShortcutLexer : public cLexer
{
public:
	cShortcutLexer( const char* buffer, unsigned int size );

	enum
	{
		/// 글자색
		eTOKEN_SHORTCUT = 100,
		eTOKEN_KEYBORAD,
	};
};

struct sKeyboardInfo
{
	BYTE	mKey;			/// DIK_KEY 값
	int		mShortcutIndex; /// 단축키 인덱스(eSHORTCUT_KEY) 값
};

struct sShortcutScript
{
	int		mPathIconIndex;
	int		mKeyboardIndex;
	int		mIconPosX;
	int		mIconPosY;

	sShortcutScript::sShortcutScript()
	{
		mPathIconIndex = -1;
		mKeyboardIndex = -1;
		mIconPosX = 0;
		mIconPosY = 0;
	}
};

struct sKeyboardScript
{
	int		mIconPosX;
	int		mIconPosY;
	unsigned char	mKey;			/// DIK_KEY 값

	sKeyboardScript::sKeyboardScript()
	{
		mIconPosX = 0;
		mIconPosY = 0;
 		mKey	  = 0;
	}
};

class cShortcutManager
{
public:
	static cShortcutManager* mpShortcutManager;

	cShortcutManager();
	~cShortcutManager();

	bool Init();
	tPointerHashMap<int,void*>* GetShortcutScriptMap() { return &mShortcutScriptMap; }
	tPointerHashMap<int,void*>* GetKeyboardScriptMap() { return &mKeyboardScriptMap; }
	tHashMap<int,int>*			GetShortcutMap()	   { return &mShortCutKey; }

protected:
	bool LoadShortcutPath( const cString& pathName );

private:
	typedef tHashMap< int, int >		  sShortCutMap;
	typedef tPointerHashMap< int, void* > sKeyboardMap;
	typedef tPointerHashMap< int, void* > sKeyMap;	

	sShortCutMap	mShortCutKey;
	sKeyboardMap	mKeyboardKey;
	
	sKeyMap	mShortcutScriptMap;
	sKeyMap	mKeyboardScriptMap;
};

#define SHORTCUTMAN		cShortcutManager::mpShortcutManager
