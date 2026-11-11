#pragma once

#include "Lexer.h"

class cKeyboardAgent;

///※주의 : 인덱스는 path_shortcut.txt 스크립트 파일 인덱스와 동일해야 함
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
	 
	 eKEY_EXTRA1_QUICK1 = 55,
	 eKEY_EXTRA1_QUICK2 = 56,
	 eKEY_EXTRA1_QUICK3 = 57,
	 eKEY_EXTRA1_QUICK4 = 58,
	 eKEY_EXTRA1_QUICK5 = 59,
	 eKEY_EXTRA1_QUICK6 = 60,
	 eKEY_EXTRA1_QUICK7 = 61,
	 eKEY_EXTRA1_QUICK8 = 62,
	 eKEY_EXTRA1_QUICK9 = 63,
	 eKEY_EXTRA1_QUICK0 = 64,

	 eKEY_EXTRA2_QUICK1 = 65,
	 eKEY_EXTRA2_QUICK2 = 66,
	 eKEY_EXTRA2_QUICK3 = 67,
	 eKEY_EXTRA2_QUICK4 = 68,
	 eKEY_EXTRA2_QUICK5 = 69,
	 eKEY_EXTRA2_QUICK6 = 70,
	 eKEY_EXTRA2_QUICK7 = 71,
	 eKEY_EXTRA2_QUICK8 = 72,
	 eKEY_EXTRA2_QUICK9 = 73,
	 eKEY_EXTRA2_QUICK0 = 74,
};

#define KEY_CHAR_INFO    SHORTCUTMAN->GetShortcutKeyInfo( eKEY_CHAR_INFO )    
#define KEY_SKILL        SHORTCUTMAN->GetShortcutKeyInfo( eKEY_SKILL )        
#define KEY_INVEN        SHORTCUTMAN->GetShortcutKeyInfo( eKEY_INVEN )        
#define KEY_QUEST        SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUEST )        
#define KEY_FRIEND       SHORTCUTMAN->GetShortcutKeyInfo( eKEY_FRIEND )       
#define KEY_GUILD        SHORTCUTMAN->GetShortcutKeyInfo( eKEY_GUILD )        
#define KEY_DUNGEON      SHORTCUTMAN->GetShortcutKeyInfo( eKEY_DUNGEON )      
#define KEY_PVP          SHORTCUTMAN->GetShortcutKeyInfo( eKEY_PVP )          
#define KEY_BOOK         SHORTCUTMAN->GetShortcutKeyInfo( eKEY_BOOK )         
#define KEY_WIDE_MAP     SHORTCUTMAN->GetShortcutKeyInfo( eKEY_WIDE_MAP )     
#define KEY_ITEM_MALL    SHORTCUTMAN->GetShortcutKeyInfo( eKEY_ITEM_MALL )    

#define KEY_MOVE_UP      SHORTCUTMAN->GetShortcutKeyInfo( eKEY_MOVE_UP )      
#define KEY_MOVE_DOWN    SHORTCUTMAN->GetShortcutKeyInfo( eKEY_MOVE_DOWN )    
#define KEY_MOVE_LEFT    SHORTCUTMAN->GetShortcutKeyInfo( eKEY_MOVE_LEFT )    
#define KEY_MOVE_RIGHT   SHORTCUTMAN->GetShortcutKeyInfo( eKEY_MOVE_RIGHT )   
#define KEY_AUTO_RUN     SHORTCUTMAN->GetShortcutKeyInfo( eKEY_AUTO_RUN )     
#define KEY_JUMP         SHORTCUTMAN->GetShortcutKeyInfo( eKEY_JUMP )         
#define KEY_TARGET       SHORTCUTMAN->GetShortcutKeyInfo( eKEY_TARGET )       
#define KEY_ATTACK       SHORTCUTMAN->GetShortcutKeyInfo( eKEY_ATTACK )       
#define KEY_ITEM_PICK    SHORTCUTMAN->GetShortcutKeyInfo( eKEY_ITEM_PICK )    
#define KEY_WEAPON_SWAP  SHORTCUTMAN->GetShortcutKeyInfo( eKEY_WEAPON_SWAP )  
#define KEY_SITDOWN      SHORTCUTMAN->GetShortcutKeyInfo( eKEY_SITDOWN )      
#define KEY_IDLE_STATE   SHORTCUTMAN->GetShortcutKeyInfo( eKEY_IDLE_STATE )   
#define KEY_SCORE_BOARD  SHORTCUTMAN->GetShortcutKeyInfo( eKEY_SCORE_BOARD )

#define KEY_WHISPER      SHORTCUTMAN->GetShortcutKeyInfo( eKEY_WHISPER )      
#define KEY_ANSWHISPER   SHORTCUTMAN->GetShortcutKeyInfo( eKEY_ANSWHISPER )   
#define KEY_CHAT_GUILD   SHORTCUTMAN->GetShortcutKeyInfo( eKEY_CHAT_GUILD )   
#define KEY_CHAT_PARTY   SHORTCUTMAN->GetShortcutKeyInfo( eKEY_CHAT_PARTY )   
#define KEY_CHAT_YELL    SHORTCUTMAN->GetShortcutKeyInfo( eKEY_CHAT_YELL )    

#define KEY_OPTION       SHORTCUTMAN->GetShortcutKeyInfo( eKEY_OPTION )       
#define KEY_HIDE_UI      SHORTCUTMAN->GetShortcutKeyInfo( eKEY_HIDE_UI )      
#define KEY_SCREENSHOT   SHORTCUTMAN->GetShortcutKeyInfo( eKEY_SCREENSHOT )   
#define KEY_VIDEO	     SHORTCUTMAN->GetShortcutKeyInfo( eKEY_VIDEO )   

#define KEY_COMMON_QUICK1	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK1 )
#define KEY_COMMON_QUICK2	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK2 )
#define KEY_COMMON_QUICK3	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK3 )
#define KEY_COMMON_QUICK4	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK4 )
#define KEY_COMMON_QUICK5	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK5 )
#define KEY_COMMON_QUICK6	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK6 )
#define KEY_COMMON_QUICK7	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK7 )
#define KEY_COMMON_QUICK8	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK8 )
#define KEY_COMMON_QUICK9	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK9 )
#define KEY_COMMON_QUICK0	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_COMMON_QUICK0 )

#define KEY_QUICK1      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK1 )       
#define KEY_QUICK2      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK2 )       
#define KEY_QUICK3      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK3 )       
#define KEY_QUICK4      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK4 )       
#define KEY_QUICK5      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK5 )       
#define KEY_QUICK6      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK6 )       
#define KEY_QUICK7      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK7 )       
#define KEY_QUICK8      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK8 )       
#define KEY_QUICK9      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK9 )       
#define KEY_QUICK0      	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_QUICK0 )       

#define KEY_EXTRA1_QUICK1	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK1 )
#define KEY_EXTRA1_QUICK2	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK2 )
#define KEY_EXTRA1_QUICK3	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK3 )
#define KEY_EXTRA1_QUICK4	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK4 )
#define KEY_EXTRA1_QUICK5	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK5 )
#define KEY_EXTRA1_QUICK6	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK6 )
#define KEY_EXTRA1_QUICK7	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK7 )
#define KEY_EXTRA1_QUICK8	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK8 )
#define KEY_EXTRA1_QUICK9	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK9 )
#define KEY_EXTRA1_QUICK0	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA1_QUICK0 )

#define KEY_EXTRA2_QUICK1	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK1 )
#define KEY_EXTRA2_QUICK2	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK2 )
#define KEY_EXTRA2_QUICK3	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK3 )
#define KEY_EXTRA2_QUICK4	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK4 )
#define KEY_EXTRA2_QUICK5	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK5 )
#define KEY_EXTRA2_QUICK6	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK6 )
#define KEY_EXTRA2_QUICK7	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK7 )
#define KEY_EXTRA2_QUICK8	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK8 )
#define KEY_EXTRA2_QUICK9	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK9 )
#define KEY_EXTRA2_QUICK0	 SHORTCUTMAN->GetShortcutKeyInfo( eKEY_EXTRA2_QUICK0 )


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
	BYTE	mMixKey;		/// 조합 DIK_KEY 값 0 이면 없음
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
	unsigned char	mMixKey;		/// 추가 조합 DIK_KEY 값 0 이면 없음

	sKeyboardScript::sKeyboardScript()
	{
		mIconPosX = 0;
		mIconPosY = 0;
 		mKey	  = 0;
		mMixKey   = 0;
	}
};

class cShortcutManager
{
public:
	static cShortcutManager* mpShortcutManager;

	cShortcutManager();
	~cShortcutManager();

	/// Key Pressed 처리
	bool ProcessKeyPressed( cKeyboardAgent* keyInfo );

	/// Key Up & Down 처리
	bool ProcessKeyUpDown( cKeyboardAgent* keyInfo );

	/// 모든 단축키 아이콘을 단축키를 기준으로 키보드에 업데이트
	void UpdateAllIconByShortcut();
	void UpdateAllIconByKeyboard();

	/// 키보드 아이콘만 mKeyboardKey.mShortcutIndex 기준으로 다시 그리기.
	void UpdateKeyboardIcon();

	/// 유저 단축키 초기 기본 셋팅
	bool InitShortcut();
	bool InitKeyboard();
	void ResetKeyboard();
	void ResetShortcut();

	/// 키보드 shortcutIndex 만 초기화
	void ClearKeyboard();

	/// 유저 단축키 모두 지우기
	void EraseShortcut( int keyboardIndex );

	///  단축키 박스에서 키보드박스로 아이콘을 드래그 하여 단축키 셋팅
	void SetShortcut( int fromShortcutIndex, int toKeyboardIndex );

	///  키보드 박스에서 키보드 박스로 단축키 이동
	void MoveShortcut( int fromKeyboardIndex, int toKeyboardIndex );

	/// 단축키 설정이 하나라도 바뀌었는지 검사.
	bool IsSettingChanged(); 

	unsigned char GetShortcutKey( int index );
	sKeyboardInfo* GetShortcutKeyInfo( int index );

	void SetShortCutValue( int index, int value );
	int  GetShortCutValue( int index );
	
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
