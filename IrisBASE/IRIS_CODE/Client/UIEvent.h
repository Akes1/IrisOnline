/* ==========================================================================
* 파    일 : UIEvent.h
* 목    적 :
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.27
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIKeyCode.h"
#include "UIPos.h"

/// UI 이벤트 타입
enum eUIEventType
{
	eUIEVENT_NULL = 0,
	eUIEVENT_CAPTURE_LOST,		/// 다른 곳에서 캡쳐 가져감.
	eUIEVENT_FOCUS_LOST,		/// 다른 곳에서 포커스 생김
	eUIEVENT_COMMAND,
	eUIEVENT_CHECKBOX_CHANGED,
	eUIEVENT_EDITBOX_ENTER,
	eUIEVENT_EDITBOX_CHAR_UPDATED,
	eUIEVENT_LISTBOX_CHANGED,
	eUIEVENT_LISTBOX_SELECTED, 
	eUIEVENT_LISTBOX_SELECTEDRBUTTON, 
	eUIEVENT_SCROLLBAR_MOVE,
	eUIEVENT_BUTTON_DOWNED,
	eUIEVENT_BUTTON_CONTINUEDOWNED,		/// 매 프로세스마다 눌림( 플래그를 세팅하면 )
	eUIEVENT_RADIO_PRESSED,
	eUIEVENT_PUSHBUTTON_PRESSED,
	eUIEVENT_TABBUTTON_PRESSED,
	eUIEVENT_ICON_DRAGGED,
	eUIEVENT_ICON_DOUBLECLICKED,
	eUIEVENT_ICON_HOVERED,
	eUIEVENT_ICON_CLICKED,
	eUIEVENT_ICON_RIGHTCLICKED,
	eUIEVENT_IMAGE_HOVERED,
	eUIEVENT_IMAGE_CLICKED,
	eUIEVENT_LISTBOX_ROW_HOVERED,
	eUIEVENT_LISTBOX_ROW_LEFT,
	eUIEVENT_LISTBOX_IMAGE_HOVERED,
	eUIEVENT_LISTBOX_IMAGE_LEFT,
	eUIEVENT_GAUGE_END,
	eUIEVENT_COMBO_SELECTED,
	eUIEVENT_OPTIONGAUGE_UPDATE,
	
	/// 마우스 이벤트 (사용자에 의해 발생)
	eUIEVENT_MOUSE_MOVE,
	eUIEVENT_MOUSE_WHEEL,
	eUIEVENT_LBUTTON_DOWN,
	eUIEVENT_RBUTTON_DOWN,
	eUIEVENT_LBUTTON_UP,
	eUIEVENT_RBUTTON_UP,
	eUIEVENT_LBUTTON_DBLCLK,
	
	/// 마우스 이벤트 (내부적으로 발생)
	eUIEVENT_MOUSE_HOVERED,
	eUIEVENT_MOUSE_LEFT,
	eUIEVENT_MOUSE_OTHERCLICK,		/// 자기가 아닌 다른 클릭 이벤트 일어남

	/// 키 이벤트 (사용자에 의해 발생)
	eUIEVENT_KEY_DOWN,
	eUIEVENT_KEY_UP,

	/// IME 이벤트
	eUIEVENT_CHAR,
	eUIEVENT_IME_NOTIFY,
	eUIEVENT_IME_STARTCOMPOSITION,
	eUIEVENT_IME_ENDCOMPOSITION,
	eUIEVENT_IME_COMPOSITION,
};

class cUINode;

typedef tUIPos<int> cUIPos;

#pragma warning( push )
#pragma warning( disable: 4201 )

/// UI 이벤트
/// UI, 마우스, 키보드 등의 이벤트에 관한 정보를 담는다.
class cUIEvent
{
public:
	/// 메시지 타입
	eUIEventType mType;

	union
	{
		/// 내부 이벤트
		struct
		{
			/// 호출자 (이벤트를 발생한 UI 노드)
			cUINode* mpCaller;

			/// 호출자의 아이디
			unsigned int mID;
		};

		/// 마우스 & 키 이벤트
		struct
		{
			/// 위치
			cUIPos mPos;

			/// 휠 값
			int mWheel;

			/// 가상 키 코드
			eKeyCode mCode;
		};

		/// IME 이벤트
		struct
		{
			/// 메세지
			unsigned int mWparam;

			unsigned int mLparam;
		};
	};

	/// 시프트 키가 눌렸는지 여부
	bool mShift;

	/// 콘트롤 키가 눌렸는지 여부
	bool mControl;

	/// Alt 키가 눌렸는지 여부
	bool mAlt;

	/// 이벤트에 담을 확장 데이터
	void* mExtraData;

	cUIEvent() : mType(eUIEVENT_NULL), mpCaller(0), mID(UINT_MAX), mWheel(0), mCode(eKEY_COUNT),
				mWparam(0), mLparam(0), mShift(false), mControl(false), mAlt(false), mExtraData(0)
	{};

};

#pragma warning( pop )

/*
* Window Messages

#define WM_NULL                         0x0000
#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_MOVE                         0x0003
#define WM_SIZE                         0x0005

#define WM_ACTIVATE                     0x0006

#define WM_SETFOCUS                     0x0007
//#define WM_KILLFOCUS                    0x0008
#define WM_ENABLE                       0x000A
#define WM_SETREDRAW                    0x000B
#define WM_SETTEXT                      0x000C
#define WM_GETTEXT                      0x000D
#define WM_GETTEXTLENGTH                0x000E
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#ifndef _WIN32_WCE
#define WM_QUERYENDSESSION              0x0011
#define WM_QUERYOPEN                    0x0013
#define WM_ENDSESSION                   0x0016
#endif
#define WM_QUIT                         0x0012
#define WM_ERASEBKGND                   0x0014
#define WM_SYSCOLORCHANGE               0x0015
#define WM_SHOWWINDOW                   0x0018
#define WM_WININICHANGE                 0x001A

#define WM_DEVMODECHANGE                0x001B
#define WM_ACTIVATEAPP                  0x001C
#define WM_FONTCHANGE                   0x001D
#define WM_TIMECHANGE                   0x001E
#define WM_CANCELMODE                   0x001F
#define WM_SETCURSOR                    0x0020
#define WM_MOUSEACTIVATE                0x0021
#define WM_CHILDACTIVATE                0x0022
#define WM_QUEUESYNC                    0x0023

#define WM_GETMINMAXINFO                0x0024

#define WM_PAINTICON                    0x0026
#define WM_ICONERASEBKGND               0x0027
#define WM_NEXTDLGCTL                   0x0028
#define WM_SPOOLERSTATUS                0x002A
#define WM_DRAWITEM                     0x002B
#define WM_MEASUREITEM                  0x002C
#define WM_DELETEITEM                   0x002D
#define WM_VKEYTOITEM                   0x002E
#define WM_CHARTOITEM                   0x002F
#define WM_SETFONT                      0x0030
#define WM_GETFONT                      0x0031
#define WM_SETHOTKEY                    0x0032
#define WM_GETHOTKEY                    0x0033
#define WM_QUERYDRAGICON                0x0037
#define WM_COMPAREITEM                  0x0039

#define WM_NOTIFY                       0x004E
#define WM_INPUTLANGCHANGEREQUEST       0x0050
#define WM_INPUTLANGCHANGE              0x0051
#define WM_HELP                         0x0053

#define WM_INPUT                        0x00FF

#define WM_KEYFIRST                     0x0100
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_DEADCHAR                     0x0103
#define WM_SYSKEYDOWN                   0x0104
#define WM_SYSKEYUP                     0x0105
#define WM_SYSCHAR                      0x0106
#define WM_SYSDEADCHAR                  0x0107

#define WM_IME_STARTCOMPOSITION         0x010D
#define WM_IME_ENDCOMPOSITION           0x010E
#define WM_IME_COMPOSITION              0x010F
#define WM_IME_KEYLAST                  0x010F

#define WM_INITDIALOG                   0x0110
#define WM_COMMAND                      0x0111
#define WM_SYSCOMMAND                   0x0112
#define WM_TIMER                        0x0113
#define WM_HSCROLL                      0x0114
#define WM_VSCROLL                      0x0115
#define WM_INITMENU                     0x0116
#define WM_INITMENUPOPUP                0x0117
#define WM_MENUSELECT                   0x011F
#define WM_MENUCHAR                     0x0120
#define WM_ENTERIDLE                    0x0121

#define WM_MOUSEFIRST                   0x0200
#define WM_MOUSEMOVE                    0x0200
//#define WM_LBUTTONDOWN                  0x0201
//#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_MBUTTONDOWN                  0x0207
#define WM_MBUTTONUP                    0x0208
#define WM_MBUTTONDBLCLK                0x0209
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
#define WM_MOUSEWHEEL                   0x020A
#endif

#define WHEEL_DELTA                     120
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))

#define WHEEL_PAGESCROLL                (UINT_MAX)

#define WM_PARENTNOTIFY                 0x0210
#define WM_ENTERMENULOOP                0x0211
#define WM_EXITMENULOOP                 0x0212

#if(WINVER >= 0x0400)
#define WM_NEXTMENU                     0x0213
#define WM_SIZING                       0x0214
#define WM_CAPTURECHANGED               0x0215
#define WM_MOVING                       0x0216

#define WM_IME_SETCONTEXT               0x0281
#define WM_IME_NOTIFY                   0x0282
#define WM_IME_CONTROL                  0x0283
#define WM_IME_COMPOSITIONFULL          0x0284
#define WM_IME_SELECT                   0x0285
#define WM_IME_CHAR                     0x0286
#if(WINVER >= 0x0500)
#define WM_IME_REQUEST                  0x0288
#if(WINVER >= 0x0400)
#define WM_IME_KEYDOWN                  0x0290
#define WM_IME_KEYUP                    0x0291

#if((_WIN32_WINNT >= 0x0400) || (WINVER >= 0x0500))
#define WM_MOUSEHOVER                   0x02A1
#define WM_MOUSELEAVE                   0x02A3
#endif

#define WM_CUT                          0x0300
#define WM_COPY                         0x0301
#define WM_PASTE                        0x0302
#define WM_CLEAR                        0x0303
#define WM_UNDO                         0x0304
#define WM_RENDERFORMAT                 0x0305
#define WM_RENDERALLFORMATS             0x0306
#define WM_DESTROYCLIPBOARD             0x0307
#define WM_DRAWCLIPBOARD                0x0308
#define WM_PAINTCLIPBOARD               0x0309
#define WM_VSCROLLCLIPBOARD             0x030A
#define WM_SIZECLIPBOARD                0x030B
#define WM_ASKCBFORMATNAME              0x030C
#define WM_CHANGECBCHAIN                0x030D
#define WM_HSCROLLCLIPBOARD             0x030E
#define WM_QUERYNEWPALETTE              0x030F
#define WM_PALETTEISCHANGING            0x0310
#define WM_PALETTECHANGED               0x0311
#define WM_HOTKEY                       0x0312
*/
