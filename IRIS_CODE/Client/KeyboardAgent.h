// * ====================================================================
// * 파    일 :	KeyboardAgent.h
// * 목    적 :	키보드 입력 관리
// * 작 성 자 :	이법석
// * 작 성 일 :	2006.01
// *
// * 추가내용 :
// *		추가날짜 	작업자		추가내용
// *		2006.01     이법석	    묵향 소스 갱신
// *		remake : int2000 - 좀 많이 바뀜
// * 주의사항 :	
// * ===================================================================

#pragma once

const int KEYBOARD_BUFFER = 32;

/// 키 상태값
#define _KEY_NONE		0
#define _KEY_UP			1
#define _KEY_DOWN		2
#define _KEY_PRESS		3

/// 이부분은 예외처리한다.
#define KEY_BACK		DIK_BACK			/// BACKSPACE key
#define KEY_TAB			DIK_TAB				/// TAB key
#define KEY_RETURN		DIK_RETURN			/// RETURN key
#define KEY_PADENTER	DIK_NUMPADENTER		/// NUMPAD ENTER key
#define KEY_SHIFT		DIK_LSHIFT			/// SHIFT key
#define KEY_RSHIFT		DIK_RSHIFT			/// SHIFT key
#define KEY_CONTROL		DIK_LCONTROL		/// CONTROL key
#define KEY_RCONTROL	DIK_RCONTROL		/// right CONTROL key
#define KEY_MENU		DIK_LMENU			/// MENU key
#define KEY_PAUSE		DIK_PAUSE			/// PAUSE key
#define KEY_CAPITAL		DIK_CAPITAL			/// CAPITAL key
#define KEY_ESCAPE		DIK_ESCAPE			/// ESCAPE key
#define KEY_SPACE		DIK_SPACE			/// SPACEBAR
#define KEY_PGUP		DIK_PGUP			/// PAGE UP key
#define KEY_PGDN		DIK_PGDN			/// PAGE DOWN key
#define KEY_END			DIK_END				/// END key
#define KEY_HOME		DIK_HOME			/// HOME key
#define KEY_LEFT		DIK_LEFT			/// LEFT ARROW key
#define KEY_UP			DIK_UP				/// UP ARROW key
#define KEY_RIGHT		DIK_RIGHT			/// RIGHT ARROW key
#define KEY_DOWN		DIK_DOWN			/// DOWN ARROW key
#define KEY_SNAPSHOT	DIK_SYSRQ			/// PRINTSCREEN key for Windows version 3.0 and later
#define KEY_INSERT		DIK_INSERT			/// INSERT key
#define KEY_DELETE		DIK_DELETE			/// DELETE key
#define KEY_ALT			DIK_LALT			/// ALT key
#define KEY_RALT		DIK_RALT			/// right ALT key
#define KEY_0			DIK_0				/// 0 key
#define KEY_1			DIK_1				/// 1 key
#define KEY_2			DIK_2				/// 2 key
#define KEY_3			DIK_3				/// 3 key
#define KEY_4			DIK_4				/// 4 key
#define KEY_5			DIK_5				/// 5 key
#define KEY_6			DIK_6				/// 6 key
#define KEY_7			DIK_7				/// 7 key
#define KEY_8			DIK_8				/// 8 key
#define KEY_9			DIK_9				/// 9 key
#define KEY_A			DIK_A				/// A key
#define KEY_B			DIK_B				/// B key
#define KEY_C			DIK_C				/// C key
#define KEY_D			DIK_D				/// D key
#define KEY_E			DIK_E				/// E key
#define KEY_F			DIK_F				/// F key
#define KEY_G			DIK_G				/// G key
#define KEY_H			DIK_H				/// H key
#define KEY_I			DIK_I				/// I key
#define KEY_J			DIK_J				/// J key
#define KEY_K			DIK_K				/// K key
#define KEY_L			DIK_L				/// L key
#define KEY_M			DIK_M				/// M key
#define KEY_N			DIK_N				/// N key
#define KEY_O			DIK_O				/// O key
#define KEY_P			DIK_P				/// P key
#define KEY_Q			DIK_Q				/// Q key
#define KEY_R			DIK_R				/// R key
#define KEY_S			DIK_S				/// S key
#define KEY_T			DIK_T				/// T key
#define KEY_U			DIK_U				/// U key
#define KEY_V			DIK_V				/// V key
#define KEY_W			DIK_W				/// W key
#define KEY_X			DIK_X				/// X key
#define KEY_Y			DIK_Y				/// Y key
#define KEY_Z			DIK_Z				/// Z key
#define KEY_PAD0		DIK_NUMPAD0			/// Numeric key pad 0 key
#define KEY_PAD1		DIK_NUMPAD1			/// Numeric key pad 1 key
#define KEY_PAD2		DIK_NUMPAD2			/// Numeric key pad 2 key
#define KEY_PAD3		DIK_NUMPAD3			/// Numeric key pad 3 key
#define KEY_PAD4		DIK_NUMPAD4			/// Numeric key pad 4 key
#define KEY_PAD5		DIK_NUMPAD5			/// Numeric key pad 5 key
#define KEY_PAD6		DIK_NUMPAD6			/// Numeric key pad 6 key
#define KEY_PAD7		DIK_NUMPAD7			/// Numeric key pad 7 key
#define KEY_PAD8		DIK_NUMPAD8			/// Numeric key pad 8 key
#define KEY_PAD9		DIK_NUMPAD9			/// Numeric key pad 9 key
#define KEY_MULTIPLY	DIK_MULTIPLY		/// NumericMultiply key
#define KEY_ADD			DIK_ADD				/// Numeric Add key
#define KEY_SUBTRACT	DIK_SUBTRACT		/// Numeric Subtract key
#define KEY_DECIMAL		DIK_DECIMAL			/// Numeric Decimal key
#define KEY_DIVIDE		DIK_DIVIDE			/// Numeric Divide key
#define KEY_F1			DIK_F1				/// F1 key
#define KEY_F2			DIK_F2				/// F2 key
#define KEY_F3			DIK_F3				/// F3 key
#define KEY_F4			DIK_F4				/// F4 key
#define KEY_F5			DIK_F5				/// F5 key
#define KEY_F6			DIK_F6				/// F6 key
#define KEY_F7			DIK_F7				/// F7 key
#define KEY_F8			DIK_F8				/// F8 key
#define KEY_F9			DIK_F9				/// F9 key
#define KEY_F10			DIK_F10				/// F10 key
#define KEY_F11			DIK_F11				/// F11 key
#define KEY_F12			DIK_F12				/// F12 key
#define KEY_F13			DIK_F13				/// F13 key
#define KEY_F14			DIK_F14				/// F14 key
#define KEY_F15			DIK_F15				/// F15 key
#define KEY_NUMLOCK		DIK_NUMLOCK			/// NUM LOCK key
#define KEY_SCROLL		DIK_SCROLL			/// SCROLL LOCK key
#define KEY_EQUALS		DIK_EQUALS			/// Plus (+) key
#define KEY_COMMA		DIK_COMMA			/// Comma (,) key
#define KEY_MINUS		DIK_MINUS			/// Minus (-) key
#define KEY_PERIOD		DIK_PERIOD			/// Period (.) key
#define KEY_SEMICOLON	DIK_SEMICOLON		/// (:)
#define KEY_SLASH		DIK_SLASH			/// (/)
#define KEY_OEM_3		DIK_GRAVE			/// ( ~ ) A°, Keyboard-specific punctuation key (may not appear on every keyboard)
#define KEY_LBRACKET	DIK_LBRACKET		/// Keyboard-specific punctuation key (may not appear on every keyboard)
#define KEY_RBRACKET	DIK_RBRACKET		/// Keyboard-specific punctuation key (may not appear on every keyboard)
#define KEY_BACKSLASH	DIK_BACKSLASH		/// ("\")
#define KEY_APOSTROPHE	DIK_APOSTROPHE		/// (')
#define KEY_OEM_102		DIK_OEM_102			/// <> or \| on enhanced, non-U.S. IBM-compatible 102-key keyboard

struct sKeyboardInfo;

class cKeyboardAgent
{
public:
	cKeyboardAgent();
	~cKeyboardAgent();

	///
	HRESULT Create( HWND hWnd, LPDIRECTINPUT8 pDi );

	///
	void Release();

	///
	void UpdateKeyboardState();

	///
	bool GetNextKeyState();

	///
	void ClearKeyState();

	/// Key입력 상태 값
	bool GetKeyDown( unsigned long key );
	bool GetKeyUp( unsigned long key );
	bool GetKeyPress( unsigned long key );
	bool GetKeyPressed( unsigned long key );
	int GetKeyState( unsigned long key );

	/// 두개의 키값이 모두 들어왔는지 체크.
	bool GetKeyDown( sKeyboardInfo* pKeyInfo );
	bool GetKeyPressed( sKeyboardInfo* pKeyInfo );
	bool GetKeyUp( sKeyboardInfo* pKeyInfo );

	/// 
	bool IsEmptyKeyboardEvent();

	//[int2000-add SetKeyState]
	void SetKeyState(unsigned long key, unsigned char state = 0x00) { mKeyState[key] = state; }


	///
	inline const LPDIRECTINPUTDEVICE8 GetDirectInput() { return mpDidKey; }

protected:
	///
	LPDIRECTINPUTDEVICE8    mpDidKey;

	///
	DIDEVICEOBJECTDATA		mDidodKey[KEYBOARD_BUFFER];

	unsigned char			mBufferKey[KEYBOARD_BUFFER];

	///
	unsigned long	mBufferedKeys, mPrevBuffer;

	///
	unsigned char			mKeyState[256];

	bool			mKeyEvent;
};



//#pragma once
//
//
//const int KEYBOARD_BUFFER = 32;
//
///// 이부분은 예외처리한다.
//#define KEY_BACK		DIK_BACK			/// BACKSPACE key
//#define KEY_TAB			DIK_TAB				/// TAB key
//#define KEY_RETURN		DIK_RETURN			/// RETURN key
//#define KEY_PADENTER	DIK_NUMPADENTER		/// NUMPAD ENTER key
//#define KEY_SHIFT		DIK_LSHIFT			/// SHIFT key
//#define KEY_RSHIFT		DIK_RSHIFT			/// SHIFT key
//#define KEY_CONTROL		DIK_LCONTROL		/// CONTROL key
//#define KEY_RCONTROL	DIK_RCONTROL		/// right CONTROL key
//#define KEY_MENU		DIK_LMENU			/// MENU key
//#define KEY_PAUSE		DIK_PAUSE			/// PAUSE key
//#define KEY_CAPITAL		DIK_CAPITAL			/// CAPITAL key
//#define KEY_ESCAPE		DIK_ESCAPE			/// ESCAPE key
//#define KEY_SPACE		DIK_SPACE			/// SPACEBAR
//#define KEY_PGUP		DIK_PGUP			/// PAGE UP key
//#define KEY_PGDN		DIK_PGDN			/// PAGE DOWN key
//#define KEY_END			DIK_END				/// END key
//#define KEY_HOME		DIK_HOME			/// HOME key
//#define KEY_LEFT		DIK_LEFT			/// LEFT ARROW key
//#define KEY_UP			DIK_UP				/// UP ARROW key
//#define KEY_RIGHT		DIK_RIGHT			/// RIGHT ARROW key
//#define KEY_DOWN		DIK_DOWN			/// DOWN ARROW key
//#define KEY_SNAPSHOT	DIK_SYSRQ			/// PRINTSCREEN key for Windows version 3.0 and later
//#define KEY_INSERT		DIK_INSERT			/// INSERT key
//#define KEY_DELETE		DIK_DELETE			/// DELETE key
//#define KEY_ALT			DIK_LALT			/// ALT key
//#define KEY_RALT		DIK_RALT			/// right ALT key
//#define KEY_0			DIK_0				/// 0 key
//#define KEY_1			DIK_1				/// 1 key
//#define KEY_2			DIK_2				/// 2 key
//#define KEY_3			DIK_3				/// 3 key
//#define KEY_4			DIK_4				/// 4 key
//#define KEY_5			DIK_5				/// 5 key
//#define KEY_6			DIK_6				/// 6 key
//#define KEY_7			DIK_7				/// 7 key
//#define KEY_8			DIK_8				/// 8 key
//#define KEY_9			DIK_9				/// 9 key
//#define KEY_A			DIK_A				/// A key
//#define KEY_B			DIK_B				/// B key
//#define KEY_C			DIK_C				/// C key
//#define KEY_D			DIK_D				/// D key
//#define KEY_E			DIK_E				/// E key
//#define KEY_F			DIK_F				/// F key
//#define KEY_G			DIK_G				/// G key
//#define KEY_H			DIK_H				/// H key
//#define KEY_I			DIK_I				/// I key
//#define KEY_J			DIK_J				/// J key
//#define KEY_K			DIK_K				/// K key
//#define KEY_L			DIK_L				/// L key
//#define KEY_M			DIK_M				/// M key
//#define KEY_N			DIK_N				/// N key
//#define KEY_O			DIK_O				/// O key
//#define KEY_P			DIK_P				/// P key
//#define KEY_Q			DIK_Q				/// Q key
//#define KEY_R			DIK_R				/// R key
//#define KEY_S			DIK_S				/// S key
//#define KEY_T			DIK_T				/// T key
//#define KEY_U			DIK_U				/// U key
//#define KEY_V			DIK_V				/// V key
//#define KEY_W			DIK_W				/// W key
//#define KEY_X			DIK_X				/// X key
//#define KEY_Y			DIK_Y				/// Y key
//#define KEY_Z			DIK_Z				/// Z key
//#define KEY_PAD0		DIK_NUMPAD0			/// Numeric key pad 0 key
//#define KEY_PAD1		DIK_NUMPAD1			/// Numeric key pad 1 key
//#define KEY_PAD2		DIK_NUMPAD2			/// Numeric key pad 2 key
//#define KEY_PAD3		DIK_NUMPAD3			/// Numeric key pad 3 key
//#define KEY_PAD4		DIK_NUMPAD4			/// Numeric key pad 4 key
//#define KEY_PAD5		DIK_NUMPAD5			/// Numeric key pad 5 key
//#define KEY_PAD6		DIK_NUMPAD6			/// Numeric key pad 6 key
//#define KEY_PAD7		DIK_NUMPAD7			/// Numeric key pad 7 key
//#define KEY_PAD8		DIK_NUMPAD8			/// Numeric key pad 8 key
//#define KEY_PAD9		DIK_NUMPAD9			/// Numeric key pad 9 key
//#define KEY_MULTIPLY	DIK_MULTIPLY		/// NumericMultiply key
//#define KEY_ADD			DIK_ADD				/// Numeric Add key
//#define KEY_SUBTRACT	DIK_SUBTRACT		/// Numeric Subtract key
//#define KEY_DECIMAL		DIK_DECIMAL			/// Numeric Decimal key
//#define KEY_DIVIDE		DIK_DIVIDE			/// Numeric Divide key
//#define KEY_F1			DIK_F1				/// F1 key
//#define KEY_F2			DIK_F2				/// F2 key
//#define KEY_F3			DIK_F3				/// F3 key
//#define KEY_F4			DIK_F4				/// F4 key
//#define KEY_F5			DIK_F5				/// F5 key
//#define KEY_F6			DIK_F6				/// F6 key
//#define KEY_F7			DIK_F7				/// F7 key
//#define KEY_F8			DIK_F8				/// F8 key
//#define KEY_F9			DIK_F9				/// F9 key
//#define KEY_F10			DIK_F10				/// F10 key
//#define KEY_F11			DIK_F11				/// F11 key
//#define KEY_F12			DIK_F12				/// F12 key
//#define KEY_F13			DIK_F13				/// F13 key
//#define KEY_F14			DIK_F14				/// F14 key
//#define KEY_F15			DIK_F15				/// F15 key
//#define KEY_NUMLOCK		DIK_NUMLOCK			/// NUM LOCK key
//#define KEY_SCROLL		DIK_SCROLL			/// SCROLL LOCK key
//#define KEY_EQUALS		DIK_EQUALS			/// Plus (+) key
//#define KEY_COMMA		DIK_COMMA			/// Comma (,) key
//#define KEY_MINUS		DIK_MINUS			/// Minus (-) key
//#define KEY_PERIOD		DIK_PERIOD			/// Period (.) key
//#define KEY_SEMICOLON	DIK_SEMICOLON		/// (:)
//#define KEY_SLASH		DIK_SLASH			/// (/)
//#define KEY_OEM_3		DIK_GRAVE			/// ( ~ ) A°, Keyboard-specific punctuation key (may not appear on every keyboard)
//#define KEY_LBRACKET	DIK_LBRACKET		/// Keyboard-specific punctuation key (may not appear on every keyboard)
//#define KEY_RBRACKET	DIK_RBRACKET		/// Keyboard-specific punctuation key (may not appear on every keyboard)
//#define KEY_BACKSLASH	DIK_BACKSLASH		/// ("\")
//#define KEY_APOSTROPHE	DIK_APOSTROPHE		/// (')
//#define KEY_OEM_102		DIK_OEM_102			/// <> or \| on enhanced, non-U.S. IBM-compatible 102-key keyboard
//
//class cKeyboardAgent
//{
//public:
//	cKeyboardAgent();
//	~cKeyboardAgent();
//
//	///
//	HRESULT Create( HWND hWnd, LPDIRECTINPUT8 pDi );
//
//	///
//	void Release();
//
//	///
//	void UpdateKeyboardState();
//
//	///
//	void StartGetKeyState();
//
//	///
//	bool GetNextKeyState();
//
//	///
//	void ClearKeyState();
//
//	/// Key입력 상태 값
//	bool GetKeyDown( unsigned long key );
//	bool GetKeyUp( unsigned long key );
//	bool GetKeyPressed( unsigned long key );
//	unsigned long GetDown() const;
//
//	/// 
//	bool IsEmptyKeyboardEvent();
//
//
//	///
//	inline const LPDIRECTINPUTDEVICE8 GetDirectInput() { return mpDidKey; }
//
//protected:
//	///
//	LPDIRECTINPUTDEVICE8    mpDidKey;
//
//	///
//	DIDEVICEOBJECTDATA		mDidodKey[KEYBOARD_BUFFER];
//
//	///
//	unsigned long	mBufferedKeys;
//
//	///
//	unsigned long	mBufferPos;
//
//	///
//	unsigned char	mKeyPress[256];
//
//	unsigned long mDown;
//};
//
//inline unsigned long cKeyboardAgent::GetDown() const
//{
//	return mDown;
//}
