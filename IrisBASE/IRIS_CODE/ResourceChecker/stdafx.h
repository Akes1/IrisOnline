// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

// 아래 지정된 플랫폼에 우선하는 플랫폼을 대상으로 하는 경우 다음 정의를 수정하십시오.
// 다른 플랫폼에 사용되는 해당 값의 최신 정보는 MSDN을 참조하십시오.
#ifndef WINVER				// Windows XP 이상에서만 기능을 사용할 수 있습니다.
#define WINVER 0x0501		// 다른 버전의 Windows에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#ifndef _WIN32_WINNT		// Windows XP 이상에서만 기능을 사용할 수 있습니다.                   
#define _WIN32_WINNT 0x0501	// 다른 버전의 Windows에 맞도록 적합한 값으로 변경해 주십시오.
#endif						

#ifndef _WIN32_WINDOWS		// Windows 98 이상에서만 기능을 사용할 수 있습니다.
#define _WIN32_WINDOWS 0x0410 // Windows Me 이상에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#ifndef _WIN32_IE			// IE 6.0 이상에서만 기능을 사용할 수 있습니다.
#define _WIN32_IE 0x0600	// 다른 버전의 IE에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 일부 CString 생성자는 명시적으로 선언됩니다.

// MFC의 공통 부분과 무시 가능한 경고 메시지에 대한 숨기기를 해제합니다.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.
#include <afxdisp.h>        // MFC 자동화 클래스
#include <io.h>


#define DIRECTINPUT_VERSION         0x0800

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT

#pragma warning( push, 3 )
#include <NiBuildConfiguration.h>

#include <NiMain.h>

#include <NiSystem.h>

#include <NiMeshLib.h>

/// Animation
#include <NiParticle.h>
#include <NiAnimation.h>

#include <NiUniversalTypes.h>
#include <NiVersion.h>

#include <NiDX9SystemDesc.h>
#include <NiD3DDefines.h>

#include <NiDirectXVersion.h>

#include <NiCollision.h>

#include <NiFilename.h>

/// Shader
#include <NiShaderFactory.h>
#include <NiShaderLibrary.h>
#include <NiShaderLibraryDesc.h>

#include <NiD3DShaderProgramFactory.h>

#include <dinput.h>

#include <MMSystem.h>

#include <NiMilesAudio.h>
#pragma warning (pop)

NiSmartPointer(NiFont);
NiSmartPointer(NiOldString2D);

#include <afxtempl.h>
#include <afxmt.h>      // MFC Multi-threading support
#include <afxpriv.h>    // Undocumented MFC

#include <assert.h>
#include "BasicString.h"
#include "Algorithm.h"
#include "Hasher.h"
#include "Functor.h"
#include "PriorityQueue.h"
#include "Pool.h"
#include "Array.h"
#include "PointerList.h"
#include "List.h"
#include "Set.h"
#include "Map.h"
#include "HashSet.h"
#include "HashMap.h"
#include "PointerHashMap.h"
#include "Lexer.h"
#include "Parser.h"

#include "Allocator.h"
#include "DefragmentArray.h"
#include "PointerArray.h"
#include "PointerHashSet.h"

#include "Console.h"
#include "File.h"
#include "./util/FileLoader.h"
#include "FileSaver.h"

#include "./util/Tokenizer.h"
#include "TargaImage.h"

#define STRSAFE_NO_DEPRECATE
#define STRSAFE_LIB
#include <STRSAFE.H> 

/// defines
#define SAFE_DELETE(x) if(x) { delete x; x = NULL; }
#define SAFE_DELETE_ARRAY(x) if(x) { delete[] x; x = NULL; }
#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; }
#define SAFE_NIDELETE(x) if(x) { NiDelete x; x = NULL; }
#define SAFE_EXIT_DELETE(x) if(x) { x->Exit(); delete x; x = NULL; }

inline long FloatToInt( float value ) { int i = (int)value; return ( value - i >= 0.999f ) ? ++i : i; }

const int SECOND = 1000;
const int MINUTE = 60 * SECOND;
const int HOUR = 60 * MINUTE;
const int DAY = 24 * HOUR;

#define S_PI    (3.14159265358979323846f)
#define S_ToRadian( degree ) ((degree) * (S_PI / 180.0f))

int ConvertToAscii( const wchar_t* uniStr, char* str, unsigned int bufferSize );
int ConvertToUnicode( const char* str, wchar_t* uniStr, unsigned int buffersize );

#define MAXSTRINGLEN(s) ((sizeof(s)/sizeof(s[0]))-1)
void Sstrncpy( LPTSTR pDest, unsigned int destMaxLen, LPCTSTR pSrc, unsigned int count );
void Sstrncat( LPTSTR pDest, unsigned int destMaxLen, LPCTSTR pSrc, unsigned int count );
// 
#ifdef _DEVSYS
#undef assert
#define assert(exp) {if(!(exp)) __asm{int 0x03}}
#endif

/// 비표준 확장을 사용했습니다 : 구조체/공용 구조체의 이름이 없습니다.
#pragma warning( disable: 4201 )

// 복사 생성자를 생성하지 못했습니다.
#pragma warning( disable: 4511 )

// 컴파일러에서 주어진 클래스에 대한 대입 연산자를 생성할 수 없습니다.
#pragma warning( disable: 4512 )

// 조건식이 상수입니다.
#pragma warning( disable: 4127 )

// 참조되지 않은 형식 매개 변수입니다.
#pragma warning( disable: 4100 )

void GameErrorLog( char* format, ... );

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


