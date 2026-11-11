#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Windows 헤더에서 거의 사용되지 않는 내용을 제외시킵니다.
#endif

// 아래 지정된 플랫폼보다 우선하는 플랫폼을 대상으로 하는 경우 다음 정의를 수정하십시오.
// 다른 플랫폼에 사용되는 해당 값의 최신 정보는 MSDN을 참조하십시오.
#ifndef WINVER				// Windows 95 및 Windows NT 4 이후 버전에서만 기능을 사용할 수 있습니다.
#define WINVER 0x0400		// Windows 98과 Windows 2000 이후 버전에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#ifndef _WIN32_WINNT		// Windows NT 4 이후 버전에서만 기능을 사용할 수 있습니다.
#define _WIN32_WINNT 0x0400		// Windows 98과 Windows 2000 이후 버전에 맞도록 적합한 값으로 변경해 주십시오.
#endif						

#ifndef _WIN32_WINDOWS		// Windows 98 이후 버전에서만 기능을 사용할 수 있습니다.
#define _WIN32_WINDOWS 0x0410 // Windows Me 이후 버전에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#ifndef _WIN32_IE			// IE 4.0 이후 버전에서만 기능을 사용할 수 있습니다.
#define _WIN32_IE 0x0501	// IE 5.0 이후 버전에 맞도록 적합한 값으로 변경해 주십시오.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 일부 CString 생성자는 명시적으로 선언됩니다.

// MFC의 공통 부분과 무시 가능한 경고 메시지에 대한 숨기기를 해제합니다.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소
#include <afxext.h>         // MFC 익스텐션
#include <afxdisp.h>        // MFC 자동화 클래스

#include <afxdtctl.h>		// Internet Explorer 4 공용 컨트롤에 대한 MFC 지원
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// Windows 공용 컨트롤에 대한 MFC 지원
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>

#include <afxmt.h>      // MFC Multi-threading support
//#include <afxole.h>     // MFC OLE support
#include <shlobj.h>     // IShellFolder support
#include <lm.h>         // WNet support
#include <afxpriv.h>    // Undocumented MFC

#define DIRECTINPUT_VERSION         0x0800

//#ifndef _DX9
//#define _DX9
//#endif

#pragma warning (push, 3)

#include <NiBuildConfiguration.h>
#include <NiSystem.h>
#include <NiMain.h>

#include <NiAnimation.h>
#include <NiCollision.h>
#include <NiParticle.h>

#include <NiMeshLib.h>

#include <NiUniversalTypes.h>
#include <NiVersion.h>

#include <NiDX9SystemDesc.h>
#include <NiD3DDefines.h>

#include <NiDirectXVersion.h>


#include <NiFilename.h>
#include <NiFont.h>
#include <NiFontString.h>
#include <Ni2DString.h>

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

#pragma warning( disable: 4702 )

#include <assert.h>
#include "BasicString.h"
#include "Algorithm.h"
#include "Hasher.h"
#include "Functor.h"
#include "PriorityQueue.h"
#include "Pool.h"
#include "Array.h"
#include "List.h"
#include "PointerList.h"
#include "PointerHashMap.h"
#include "Set.h"
#include "Map.h"
#include "HashSet.h"
#include "HashMap.h"

#include "Console.h"
#include "File.h"
#include "FileLoader.h"
#include "FileSaver.h"

#include "Tokenizer.h"
#include "TargaImage.h"

#define STRSAFE_NO_DEPRECATE
#define STRSAFE_LIB
#include <STRSAFE.H> 


#define MAXSTRINGLEN(s) ((sizeof(s)/sizeof(s[0]))-1)
void Sstrncpy( LPTSTR pDest, unsigned int destMaxLen, LPCTSTR pSrc, unsigned int count );
void Sstrncat( LPTSTR pDest, unsigned int destMaxLen, LPCTSTR pSrc, unsigned int count );

#define SAFE_DELETE(x) if(x) { delete x; x = NULL; }
#define SAFE_DELETE_ARRAY(x) if(x) { delete[] x; x = NULL; }
#define SAFE_NIDELETE(x) if(x) { NiDelete x; x = NULL; }


#pragma warning( disable: 4201 )
#pragma warning( disable: 4258 )


#ifdef _FASTDEBUG
#undef assert
#define assert(exp) { if(!(exp)) __asm{int 0x03}}
#endif
