// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일입니다.
#include <winsock2.h>
#include <windows.h>

// C의 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <assert.h>

#pragma warning (push, 3)
#include <mmsystem.h>
#pragma warning (pop)

#ifdef _MSC_VER
#pragma warning(disable:4511)	// 복사 생성자를 생성하지 못했습니다.
#pragma warning(disable:4512)	// 컴파일러에서 주어진 클래스에 대한 대입 연산자를 생성할 수 없습니다.
#endif

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include "../Common/GameFile.h"
#include "NiPoint2.h"
#include "NiPoint3.h"

/// STL
#include "BasicString.h"
#include "Algorithm.h"
#include "PriorityQueue.h"
#include "Allocator.h"
#include "Pool.h"
#include "Array.h"
#include "DefragmentArray.h"
#include "List.h"
#include "Queue.h"
#include "HashMap.h"
#include "HashSet.h"
#include "PointerArray.h"
#include "PointerList.h"
#include "PointerHashMap.h"
#include "PointerHashSet.h"

#include "File.h"
#include "FileLoader.h"

// 변수
extern bool	gExit;

/// defines
#define SAFE_DELETE(x) if(x) { delete x; x = NULL; }
#define SAFE_DELETE_ARRAY(x) if(x) { delete[] x; x = NULL; }
#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; }
#define SAFE_RELEASE_DELETE(x) if(x) { x->Release(); delete x; x = NULL; }

inline long FloatToInt( float value ) { int i = (int)value; return ( value - i >= 0.999f ) ? ++i : i; }

const int SECOND = 1000;
const int MINUTE = 60 * SECOND;
const int HOUR = 60 * MINUTE;
const int DAY = 24 * HOUR;

#define S_PI    (3.14159265358979323846f)
#define S_ToRadian( degree ) ((degree) * (S_PI / 180.0f))
