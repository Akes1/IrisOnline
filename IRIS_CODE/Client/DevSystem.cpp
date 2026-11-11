#include "StdAfx.h"
#include "devsystem.h"
#include "rendersystem.h"

#include "Application.h"
#include "FontAgent.h"

#include "Hero.h"
#include "ObjectManager.h"
#include "SkillManager.h"

#include "InputSystem.h"

#include "CameraManager.h"

#include "UIManager.h"
#include "UINode.h"

#include "ServerWindow.h"
#include "Button.h"
cDevSystem* cDevSystem::mpDevSystem = NULL;


cDevSystem::cDevSystem()
{
	mpDevSystem = this;
	mTxtColor = NiColorA(0.0f, 0.0f, 1.0f, 1.0f);

	mLastTime = 0;
	mFrames = 0;
	mFPS = 0.0f;

	mNode = 0;
	mTriangle = 0;

	mSolidRenderCount = 0;
	mDamageRenderCount = 0;
	mAlphaRenderCount = 0;
	mAlphaTestRenderCount = 0;

	mTerrainRenderCount = 0;

	mScaleAccumTime[0] = 0;
	mAniLastTime[0] = 0;
	mScaleFactor[0] = 0.0f;

	mScaleAccumTime[1] = 0;
	mAniLastTime[1] = 0;
	mScaleFactor[1] = 0.0f;

}

cDevSystem::~cDevSystem()
{
	Exit();

	mpDevSystem = NULL;
}

/* ------------------------------------------------------------------
* 함수이름 :	Init()
* 목    적 :	초기화
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cDevSystem::Init()
{

	return true;
}

/* ------------------------------------------------------------------
* 함수이름 :	Exit()
* 목    적 :	종료부
* 주의사항 :	
* ------------------------------------------------------------------ */
void cDevSystem::Exit()
{
}

/* ------------------------------------------------------------------
* 함수이름 :	Process( float time )
* 목    적 :	처리부
* 주의사항 :	
* ------------------------------------------------------------------ */
void cDevSystem::Process( unsigned long accumTime )
{
	if( mLastTime <= accumTime )
	{
		mFPS = (float)mFrames / ((float)(accumTime-(mLastTime-1000))*0.001f);
		mLastTime = accumTime+1000;
		mFrames = 0;
	}
	else
		mFrames++;
}

/* ------------------------------------------------------------------
* 함수이름 :	DrawDevInfo()
* 목    적 :	
* 주의사항 :	
* ------------------------------------------------------------------ */
void cDevSystem::DrawDevInfo()
{
	TCHAR	str[256]=  {0,};

	POINT pos;
	pos.x = 10;
	pos.y = 10;

	::_stprintf_s( str, _T("*FPS : %d"), (int)mFPS );
	FONTAGENT->DrawText( cFontAgent::eFont_System, str, ::lstrlen(str), &pos, 0xffffffff, true );

	pos.y += FONTAGENT->GetTextHeight( cFontAgent::eFont_System ) + 8;
	::_stprintf_s( str, _T("*[Solid | AlphaTest | AlphaNoTest | Damage] : %d | %d | %d | %d"), mSolidRenderCount, mAlphaTestRenderCount, mAlphaRenderCount, mDamageRenderCount );
	FONTAGENT->DrawText( cFontAgent::eFont_System, str, ::lstrlen(str), &pos, 0xffffffff, true );

	pos.y += FONTAGENT->GetTextHeight( cFontAgent::eFont_System ) + 8;
	::_stprintf_s( str, _T("*[Terrain RenderCount] : %d"), mTerrainRenderCount );
	FONTAGENT->DrawText( cFontAgent::eFont_System, str, ::lstrlen(str), &pos, 0xffffffff, true );

	pos.y += FONTAGENT->GetTextHeight( cFontAgent::eFont_System ) + 8;
	::_stprintf_s( str, _T("[D:%d] [S:%d] [I:%d] [A:%d]"), SKILLMAN->GetDummySkillCount(),
		SKILLMAN->GetSkillCount(), SKILLMAN->GetInfluenceCount(), SKILLMAN->GetApplyCount() );
	FONTAGENT->DrawText( cFontAgent::eFont_System, str, ::lstrlen(str), &pos, 0xffffffff, true );

	pos.y += FONTAGENT->GetTextHeight( cFontAgent::eFont_System ) + 8;
	::_stprintf_s( str, _T("[width:%d] [height:%d]"), RENDERSYS->GetMonitorWidth(),RENDERSYS->GetMonitorHeight() );
	FONTAGENT->DrawText( cFontAgent::eFont_System, str, ::lstrlen(str), &pos, 0xffffffff, true );

	pos.y += FONTAGENT->GetTextHeight( cFontAgent::eFont_System ) + 8;
	::_stprintf_s( str, _T("[activeFlag:%d] [cur:%x] [active:%x] [foreground:%x]"), THEAPP->IsActive(), THEAPP->GetHWND(), GetActiveWindow(), GetForegroundWindow() );
	FONTAGENT->DrawText( cFontAgent::eFont_System, str, ::lstrlen(str), &pos, 0xffffffff, true );
}
