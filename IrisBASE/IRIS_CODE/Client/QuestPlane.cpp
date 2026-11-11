#include "StdAfx.h"
#include "QuestPlane.h"
#include "Application.h"

#include "RenderSystem.h"
#include "CameraManager.h"
#include "FontAgent.h"

cQuestPlane::cQuestPlane()
: mQuestColor(0)
, mPosX(0)
, mPosY(0)
, mActiveTime(0)
, mMsgWidth(0)
, mActive(false)
, mAlpha(0)
{
	memset( mQuestMsg, 0, sizeof(mQuestMsg) );
}

cQuestPlane::~cQuestPlane()
{

}

void cQuestPlane::Active( LPCTSTR msg, unsigned long color, NiPoint3& pos )
{
	if( msg == 0 )	
		return;

	mMsgLen = ::_tcslen( msg );
	if( mMsgLen <= 0 )
		return;

	/// 문자열 복사
	Sstrncpy( mQuestMsg, QUEST_TEXT_LENGTH, msg, QUEST_TEXT_LENGTH - 1);

	/// 너비
	mMsgWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Cache, mQuestMsg, mMsgLen );

	///
	mActive = true;
	mActiveTime = THEAPP->GetWorldAccumTime();
    mQuestColor = color;

	/// 캐릭터의 위치를 따라가기 위함.
	float x = 0.0f;
	float y = 0.0f;
	int screenX = 0;
	int screenY = 0;

	/// 카메라 기준좌표 - y 좌표만 변경
	WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), pos, x, y );

	screenX = (int)( (RENDERSYS->GetScreenWidth() - mMsgWidth) * 0.5f);
	screenY = (int)( (1.0f - y) * RENDERSYS->GetScreenHeight() );
	mPosY = screenY;
	mPosX = screenX;

	//mAlpha = 1.0f;

	//mQuestColor = ( mQuestColor & 0x00FFFFFF) | ( (int)( 0.5f * 255 ) << 24 );
}

void cQuestPlane::Draw()
{
	FONTAGENT->DrawText( cFontAgent::eFont_Cache, mQuestMsg, mMsgLen, mPosX, mPosY, mQuestColor );
}

void cQuestPlane::Update( unsigned long time )
{
	if( IsActive() == false )
		return;

	/// 퀘스트 문자열이 보여진후 일정시간후에 사라짐
	///if( time - mActiveTime >= QUEST_ACTIVE_TIME )
	//	DeActive();

	//mAlpha -= 0.1f;
	//mQuestColor = ( mQuestColor & 0x00FFFFFF) | ( (int)( mAlpha * 255 ) << 24 );

	/*if( time - mActiveTime >= QUEST_ACTIVE_TIME )
	{
		float alpha = (float)((time - mActiveTime) - QUEST_ACTIVE_TIME ) * 0.001f;
		alpha = 1.0f - (alpha * 10.5f);

		mQuestColor = ( mQuestColor & 0x00FFFFFF) | ( (int)( alpha * 255 ) << 24 );

		if( alpha < 0.0f )
		{
			alpha = 1.0f;
			DeActive();
		}
	}*/

	if( time - mActiveTime >= QUEST_ACTIVE_TIME )
		DeActive();

	if( mPosY > 0 )
		mPosY-=1;
}