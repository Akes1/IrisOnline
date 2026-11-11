///* ==========================================================================
//* 파    일 : TutorialObject.h
//* 목    적 : 
//* 작 성 자 : 
//* 작 성 일 : 2009-05-25
//* 주의사항 :
//*===========================================================================*/
#pragma once
#include "ThemeObject.h"

// 튜토리얼 객체
class cTutorialObject : public cThemeObject
{
public:
	cTutorialObject();
	virtual ~cTutorialObject();

	void* operator new( size_t n ); 
	void operator delete( void* ptr, size_t n );

	virtual bool InitTheme( unsigned short themeIdx, unsigned char mode );
};
