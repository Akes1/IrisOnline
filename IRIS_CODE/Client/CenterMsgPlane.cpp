#include "StdAfx.h"
#include ".\centermsgplane.h"

#include "RenderSystem.h"
#include "CameraManager.h"
#include "FontAgent.h"

cCenterMsgPlane::cCenterMsgPlane()
: mBackColor(0)
{
}

cCenterMsgPlane::~cCenterMsgPlane()
{
	Clear();
}

void cCenterMsgPlane::Init( NiPoint3 pos )
{

}

void cCenterMsgPlane::Clear()
{
	cMsgItemList::cIterator i = mMsgList.Begin();
	cMsgItemList::cIterator end = mMsgList.End();
	for( ; i != end; ++i )
	{
		sMsgItem* p = (sMsgItem*)(*i);
		SAFE_DELETE(p);
	}
	mMsgList.Clear();

	i = mDeleteMsgList.Begin();
	end = mDeleteMsgList.End();
	for( ; i != end; ++i )
	{
		sMsgItem* p = (sMsgItem*)(*i);
		SAFE_DELETE(p);
	}
	mDeleteMsgList.Clear();
}


void cCenterMsgPlane::ActiveMsg( LPCTSTR msg, NiPoint3 pos, unsigned long color )
{
	if( msg == 0 )
		return;

	sMsgItem* p = new sMsgItem;
	if( p == 0 )
	{
		assert(0);
		return;
	}
	Sstrncpy( p->Messge, MSG_TEXT_LENGTH, msg, MSG_TEXT_LENGTH-1 );
	p->MsgLen = ::_tcslen( p->Messge );
	if( p->MsgLen == 0 )
	{
		SAFE_DELETE(p);
		return;
	}
	p->MsgWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, p->Messge, p->MsgLen );

	/// 카메라 기준좌표 - y 좌표만 변경
	float x = 0.0f;
	float y = 0.0f;
	WorldPtToScreenPt( CAMERAMAN->GetCurrentNi(), pos, x, y );
	p->PosX = (int)( (RENDERSYS->GetScreenWidth() - p->MsgWidth) * 0.5f);
	p->PosY  = (1.0f - y) * RENDERSYS->GetScreenHeight();
	p->MsgColor = color;

	p->valY = 0.0f;
	p->checkY = 0.0f;
	p->valAlpha = 0.0f;

	/// 메세지가 겹치는 현상을 막기위한 처리
	if( mMsgList.IsEmpty() == false )
	{
		sMsgItem* last = (sMsgItem*)mMsgList.Back();
		if( last )
		{
			int checkHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_System) + 1;
			if( p->PosY - last->PosY < checkHeight )
				p->PrevCheck = true;
		}
	}

	if( mMsgList.PushBack( p ) == false )
	{
		assert(0);
		SAFE_DELETE(p);
		return;
	}
}

void cCenterMsgPlane::Update( unsigned long deltatime )
{
	if( mMsgList.IsEmpty() == true )
		return;

	int checkHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_System) + 1;

	float prevPosY = 0.0f;
	cMsgItemList::cIterator i = mMsgList.Begin();
	cMsgItemList::cIterator end = mMsgList.End();
	for( ; i != end; ++i )
	{
		sMsgItem* p = (sMsgItem*)(*i);
		if( p == 0 )
			continue;

		if( prevPosY != 0.0f )
		{
			if( (int)(p->PosY - prevPosY) < checkHeight )
				continue;
		}

		unsigned long color = p->MsgColor & 0xFF000000;
		float alpha = (float)(color>>24);

		float val = (float)(100.0f * (deltatime * 0.001f));
		if( val > 0 )
		{
			p->PosY = p->PosY - val;
			p->checkY += val;
		}

		// 계속 올라간 값 체크
		if( p->checkY >= 100.0f )
		{
			if( alpha <= val*2 )
			{
				mDeleteMsgList.PushBack(p);
				continue;
			}
			else
				alpha -= val*2;
		}

		p->MsgColor = ((unsigned long)(alpha) << 24) | (p->MsgColor & 0x00FFFFFF);
		mBackColor = ((unsigned long)(alpha) << 24) | 0x00111111;

		prevPosY = p->PosY;
	}

	/// delete
	i = mDeleteMsgList.Begin();
	end = mDeleteMsgList.End();
	for( ; i != end; ++i )
	{
		sMsgItem* p = (sMsgItem*)(*i);
		mMsgList.Remove(p);
		SAFE_DELETE(p);
	}
	mDeleteMsgList.Clear();
}

void cCenterMsgPlane::Draw()
{
	if( mMsgList.IsEmpty() == true )
		return;

	cMsgItemList::cIterator i = mMsgList.Begin();
	cMsgItemList::cIterator end = mMsgList.End();
	for( ; i != end; ++i )
	{
		sMsgItem* p = (sMsgItem*)(*i);
		if( p == 0 )
			continue;

		FONTAGENT->DrawText( cFontAgent::eFont_System, p->Messge, p->MsgLen, p->PosX, (int)p->PosY, p->MsgColor );
	}
}
