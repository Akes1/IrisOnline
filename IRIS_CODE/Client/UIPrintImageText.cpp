#include "StdAfx.h"
#include "RenderSystem.h"
#include "FontAgent.h"
#include "Application.h"
#include "UIPrintImageText.h"

cCenterNoticeText::cCenterNoticeText()
: mPosY(0)
, mAutoAddHeight(true)
, mActiveTime(NOTICEMSG_ACTIVE_TIME)
{

}

cCenterNoticeText::~cCenterNoticeText()
{
	DeleteText();
}

void cCenterNoticeText::AddText( unsigned short fontIdx, LPCTSTR msg, unsigned long color, unsigned long height, bool autoAddHeight, unsigned long activeTime )
{
	int len = ::_tcslen( msg );
	if( len == 0 )
		return;

	sNoticeMsgItem* item = new sNoticeMsgItem;

	Sstrncpy( item->msg, MSG_PRINT_BUFFER_LENGTH, msg, len );
	item->fontIdx = fontIdx;
	item->length = len;
	item->color = color;
	item->width = FONTAGENT->GetTextExtent( fontIdx, msg, len );

	item->posX = (int)( (RENDERSYS->GetScreenWidth() - item->width) * 0.5f);
	item->posY = height;

	mPosY = height;
	mAutoAddHeight = autoAddHeight;
	mActiveTime = activeTime;

	/// 시간 저장
	item->startTime = THEAPP->GetWorldAccumTime();

	/// 일정갯수 이상이면 지워내기
	if( mMsgItemList.GetSize() >= MAX_NOTICCOUNT )
	{
		sNoticeMsgItem* p = (sNoticeMsgItem*)mMsgItemList.Front();
		SAFE_DELETE(p);

		mMsgItemList.PopFront();
	}

	/// 삽입
	mMsgItemList.PushBack( item );
}

void cCenterNoticeText::DeleteText()
{
	cMsgItemList::cIterator i = mMsgItemList.Begin();
	cMsgItemList::cIterator end = mMsgItemList.End();
	for( ; i != end; ++i )
	{
		sNoticeMsgItem* p = (sNoticeMsgItem*)(*i);
		SAFE_DELETE(p);
	}
	mMsgItemList.Clear();

	/// 삭제
	cMsgItemList::cIterator di = mMsgDelList.Begin();
	cMsgItemList::cIterator dend = mMsgDelList.End();
	for( ; di != dend; ++di )
	{
		sNoticeMsgItem* p = (sNoticeMsgItem*)(*i);
		SAFE_DELETE( p );
	}
	mMsgDelList.Clear();
}

void cCenterNoticeText::Update( unsigned long deltaTime, unsigned long accumTime )
{
	cMsgItemList::cIterator i = mMsgItemList.Begin();
	cMsgItemList::cIterator end = mMsgItemList.End();
	for( ; i != end; ++i )
	{
		sNoticeMsgItem* item = (sNoticeMsgItem*)(*i);
		if( !item )
			continue;

		/// 일정시간이 지나면 삭제
		if( item->startTime + mActiveTime <= accumTime )
		{
			mMsgDelList.PushBack( item );
		}
	}

	/// 삭제
	cMsgItemList::cIterator di = mMsgDelList.Begin();
	cMsgItemList::cIterator dend = mMsgDelList.End();
	for( ; di != dend; ++di )
	{
		sNoticeMsgItem* p = (sNoticeMsgItem*)(*di);
		if( p )
		{
			mMsgItemList.Remove( p );
			SAFE_DELETE( p );
		}
	}
	mMsgDelList.Clear();
}

void cCenterNoticeText::Draw()
{
	if( mMsgItemList.IsEmpty() == true )
		return;

	unsigned int y = mPosY;
	cMsgItemList::cIterator i = mMsgItemList.Begin();
	cMsgItemList::cIterator end = mMsgItemList.End();
	for( ; i != end; ++i )
	{
		sNoticeMsgItem* item = (sNoticeMsgItem*)(*i);
		if( !item )
			continue;

		if( mAutoAddHeight == true )
		{
			FONTAGENT->DrawText( item->fontIdx, item->msg, item->length, item->posX, y, item->color );
			y += FONTAGENT->GetTextHeight( item->fontIdx );
		}
		else
		{
			FONTAGENT->DrawText( item->fontIdx, item->msg, item->length, item->posX, item->posY, item->color );
		}
	}
}

void cCenterNoticeText::UpdateScreen()
{
	if( mMsgItemList.IsEmpty() == true )
		return;

	cMsgItemList::cIterator i = mMsgItemList.Begin();
	cMsgItemList::cIterator end = mMsgItemList.End();
	for( ; i != end; ++i )
	{
		sNoticeMsgItem* item = (sNoticeMsgItem*)(*i);
		if( item )
		{
			item->posX = (int)( (RENDERSYS->GetScreenWidth() - item->width) * 0.5f);
		}
	}
}
