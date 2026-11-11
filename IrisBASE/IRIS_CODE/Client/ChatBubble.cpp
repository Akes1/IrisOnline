#include "StdAfx.h"
#include "chatbubble.h"

#include "Application.h"
#include "RenderSystem.h"
#include "ResourceManager.h"
#include "FontAgent.h"

/// 상하단 테두리
const unsigned int	BUBBLE_FRAME_LR_SPACE = 19;
const unsigned int	BUBBLE_FRAME_TOP_SPACE = 4;
const unsigned int	BUBBLE_FRAME_BOTTOM_SPACE = 14;
const unsigned int	TEXT_FRAME_HEIGHT = 15;
const unsigned int	TEXT_FRAME_SPACE = 4;

const float	DEFAULT_ALPH = 0.7f;


cBubbleBase::cBubbleBase()
{
	mActive = false;
	mLoop = false;

	::memset( mBubbleBox, 0, sizeof(sBubbleBox)*BUBBLE_LINE_MAX );

	mPosX = 0;
	mPosY = 0;
	mLineNum = 0;

	::memset( mChatMsgLen, 0, sizeof(mChatMsgLen) );

	mChatMsgWidth = 0;
	mActiveTime = 0;
	mFontSpace = 0;

	/// 테두리 상/하단부 높이 4/4 pixel
	/// 글자 입력부 높이 15 pixel
	/// 라인 갯수에 따른 높이 계산은 8 + line*15

	cString pathName;
	pathName.Format( "./language/%s/U_Basic_01.tga", cApplication::mLangaugeFolder.Cstr() );
	NiTexture* pTex = RESOURCEMAN->LoadTexture( pathName.Cstr(), false );
	if( pTex )
	{
		float texWid = (float)pTex->GetWidth();
		float texHei = (float)pTex->GetHeight();

		float localTexL,localTexT,localTexR,localTexB;
		for( unsigned int i=0; i<BUBBLE_LINE_MAX; i++ )
		{
			/// 3개의 rectangle을 가진 스크린 객체 생성
			mBubbleBox[i].mpBoxElement = NiNew NiMeshScreenElements( false, true, 1 );
			mBubbleBox[i].mFrameWidth = BUBBLE_FRAME_LR_SPACE * 2;
			mBubbleBox[i].mFrameHeight = (BUBBLE_FRAME_TOP_SPACE + BUBBLE_FRAME_BOTTOM_SPACE) + (TEXT_FRAME_HEIGHT * (i+1));

			/// Property정보를 생성한다.
			NiTexturingProperty* pTextureProp = NiNew NiTexturingProperty;
			pTextureProp->SetBaseTexture( pTex );
			pTextureProp->SetApplyMode( NiTexturingProperty::APPLY_MODULATE );
			mBubbleBox[i].mpBoxElement->AttachProperty( pTextureProp );

			NiVertexColorProperty* pVertex = NiNew NiVertexColorProperty;
			pVertex->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
			pVertex->SetLightingMode( NiVertexColorProperty::LIGHTING_E );
			mBubbleBox[i].mpBoxElement->AttachProperty( pVertex );

			NiAlphaProperty* pAlphaProp = NiNew NiAlphaProperty;
			pAlphaProp->SetAlphaBlending( true );
			mBubbleBox[i].mpBoxElement->AttachProperty( pAlphaProp );

			/// 적용
			mBubbleBox[i].mpBoxElement->UpdateProperties();

			NiMeshUpdateProcess kUpdateProcess;
			mBubbleBox[i].mpBoxElement->Update( kUpdateProcess );

			localTexR = 357.0f / texWid;

			localTexT = 209.0f;
			for( unsigned int ii=0;ii<i;ii++ )
				localTexT += mBubbleBox[ii].mFrameHeight + 5;

			localTexT /= texHei;
			localTexB = localTexT + (float)mBubbleBox[i].mFrameHeight/texHei;

			for( unsigned int j=0; j<ELEMENT_MAX; j++ )
			{
				mBubbleBox[i].mpBoxElement->Insert( 4 );
				mBubbleBox[i].mpBoxElement->SetRectangle( j, 0.0f, 0.0f, 0.0f, 0.0f );

				localTexL = localTexR;
				localTexR = localTexL + (float)((j==1)? 1 : BUBBLE_FRAME_LR_SPACE) / texWid;
				mBubbleBox[i].mpBoxElement->SetTextures( j, 0, localTexL, localTexT, localTexR, localTexB );
			}

			for( int num = 0; num < mBubbleBox[i].mpBoxElement->GetNumPolygons(); ++num)
			{
				mBubbleBox[i].mpBoxElement->SetColors( num, NiColorA(1.0f, 1.0f, 1.0f, DEFAULT_ALPH) );
			}
		}
	}

	int fontHeight = FONTAGENT->GetTextHeight( cFontAgent::eFont_Chat );
	if( TEXT_FRAME_HEIGHT > fontHeight )
		mFontSpace = (TEXT_FRAME_HEIGHT - fontHeight)/2;

	mAlphaColor = 0xffffffff;
}

cBubbleBase::~cBubbleBase()
{
	for( unsigned int i=0; i<BUBBLE_LINE_MAX; i++ )
	{
		if( mBubbleBox[i].mpBoxElement )
		{
			assert( mBubbleBox[i].mpBoxElement->GetRefCount() == 1 );
			mBubbleBox[i].mpBoxElement = 0;
//			NiDelete mBubbleBox[i].mpBoxElement;
		}
	}
}

void cBubbleBase::Update( unsigned long time ) //NiPoint3& pos, unsigned long time )
{
	if( IsActive() == false )
	{
		return;
	}

	/// 경과 시간 체크
	if( mLoop == false && time - mActiveTime >= BUBBLE_ACTIVE_TIME )
	{

		///
		float alpha = (float)((time - mActiveTime) - BUBBLE_ACTIVE_TIME) * 0.001f;
		alpha = DEFAULT_ALPH - alpha*3.5f;

		if( DEFAULT_ALPH < alpha )
		{
			assert(0);
		}

		if( alpha < 0.0f )
		{
			alpha = DEFAULT_ALPH;
			DeActive();
		}

		for( int i=0; i<ELEMENT_MAX;++i )
		{
			for( int num = 0; num < mBubbleBox[i].mpBoxElement->GetNumPolygons(); ++num)
			{
				mBubbleBox[i].mpBoxElement->SetColors( num, NiColorA(1.0f, 1.0f, 1.0f, alpha) );
			}
		}
		mAlphaColor = (DWORD)(((BYTE)(alpha*255))<<24) | 0x00ffffff;
	}
}

void cBubbleBase::SetPos( int screenX, int screenY )
{
	/// 박스 이미지 위치 조정 및 크기 조정
	if( mLineNum <= 0 )
	{
		NiMessageBox("chatBubble", "error info");
		return;
	}

	float InvH = (float)mBubbleBox[mLineNum-1].mFrameHeight / (float)RENDERSYS->GetScreenHeight();

	/// 시작위치 계산 및 보정
	screenX = screenX - mBubbleBox[mLineNum-1].mFrameWidth/2;
	screenY = screenY - mBubbleBox[mLineNum-1].mFrameHeight;

	/// 문자가 찍힐 자리를 계산한다.
	mPosX = screenX;
	mPosY = screenY;

	float x = (float)screenX / (float)RENDERSYS->GetScreenWidth();
	float y = (float)screenY / (float)RENDERSYS->GetScreenHeight();

	/// left
	float elementWidth =  BUBBLE_FRAME_LR_SPACE / (float)RENDERSYS->GetScreenWidth();
	mBubbleBox[mLineNum-1].mpBoxElement->SetRectangle( LEFT_ELEMENT, x, y, elementWidth, InvH );

	/// center
	x += elementWidth;
	elementWidth = mChatMsgWidth / (float)RENDERSYS->GetScreenWidth();
	mBubbleBox[mLineNum-1].mpBoxElement->SetRectangle( CENTER_ELEMENT, x, y, elementWidth, InvH );

	/// right
	x += elementWidth;
	elementWidth =  BUBBLE_FRAME_LR_SPACE / (float)RENDERSYS->GetScreenWidth();
	mBubbleBox[mLineNum-1].mpBoxElement->SetRectangle( RIGHT_ELEMENT, x, y, elementWidth, InvH );
}

unsigned int cBubbleBase::GetBubbleHeight()
{
	return mBubbleBox[mLineNum-1].mFrameHeight;
}

//////////////////////////////////////////////////////////////////////////
cChatBubble::cChatBubble()
{
}

cChatBubble::~cChatBubble()
{
}

void cChatBubble::Draw()
{
	NiRenderer* pRenderer = NiRenderer::GetRenderer();

	if( mLineNum <= 0 )
	{
		assert( 0 );
	}

	if( mLineNum > BUBBLE_LINE_MAX )
	{
		assert( 0 );
	}

	unsigned long color = mChatColor & mAlphaColor;

	/// 말풍선 그리기
	mBubbleBox[mLineNum-1].mpBoxElement->RenderImmediate(pRenderer);

	POINT pos;
	pos.x = mPosX + BUBBLE_FRAME_LR_SPACE + TEXT_FRAME_SPACE;
	pos.y = mPosY + mBubbleBox[mLineNum-1].mFrameHeight - BUBBLE_FRAME_BOTTOM_SPACE;

	/// 글자 찍기
	pos.y -= TEXT_FRAME_HEIGHT - mFontSpace;
	for( unsigned int i = mLineNum; i>0; --i )
	{
		FONTAGENT->DrawText( cFontAgent::eFont_Chat, mChatMsg[i-1], mChatMsgLen[i-1], pos.x, pos.y, color, true );
		pos.y -= TEXT_FRAME_HEIGHT;
	}

}

void cChatBubble::Active( LPTSTR msg, unsigned long color, bool loop )
{
	if( msg == 0 )	
		return;

	mAlphaColor = 0xffffffff;

	unsigned int cpyNum = 0;
	mLineNum = 0;
	mChatMsgWidth = 0;

	unsigned int msgLen = ::_tcslen( msg );
	if( msgLen <= 0 )
	{
		assert( 0 );
		return;
	}

	/// 말풍선 정보를 셋팅한다.
	while( msgLen > BUBBLE_TEXT_LENGTH )
	{
		if( ( msg + BUBBLE_TEXT_LENGTH ) != ::CharNext( ::CharPrev( msg, msg + BUBBLE_TEXT_LENGTH ) ) )
		{
			cpyNum = BUBBLE_TEXT_LENGTH - 1;
		}
		else
		{
			cpyNum = BUBBLE_TEXT_LENGTH;
		}


		Sstrncpy( mChatMsg[mLineNum], BUBBLE_TEXT_LENGTH+1, msg, cpyNum );
		mChatMsgLen[mLineNum] = cpyNum;

		msgLen -= cpyNum;
		msg += cpyNum;
		if( *msg == _T(' ') ) 
		{
			/// 다음 문자 시작이 스페이스면 스킵
			++msg;
			msgLen--;
		}

		if( ++mLineNum >= BUBBLE_LINE_MAX-1 )
		{
			break;
		}
	}

	if( msgLen > 0 )
	{
		if( mLineNum < BUBBLE_LINE_MAX )
		{
			Sstrncpy( mChatMsg[mLineNum], BUBBLE_TEXT_LENGTH+1, msg, BUBBLE_TEXT_LENGTH );
			mChatMsgLen[mLineNum] = msgLen;
			++mLineNum;
		}
		else
		{
			assert(0);
		}
	}

	if( mLineNum > BUBBLE_LINE_MAX )
	{
		assert(0);
	}

	/// msg가 적힐 부분의 넓이
	for( unsigned int i= 0; i<mLineNum; ++i )
	{
		unsigned int msgWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mChatMsg[i], mChatMsgLen[i] )
								+ TEXT_FRAME_SPACE * 2;
		if( mChatMsgWidth < msgWidth ) 
			mChatMsgWidth = msgWidth;
	}

	/// error 처리
	if( mLineNum == 0 )
	{
		assert(0);
		return;
	}

	/// 프레임 넓이 셋팅
	mBubbleBox[mLineNum-1].mFrameWidth = mChatMsgWidth + BUBBLE_FRAME_LR_SPACE*2;

	/// 속성값 적용
	mActive = true;
	mLoop = loop;
	mActiveTime = THEAPP->GetWorldAccumTime();

	mChatColor = color;

	for( int i=0; i<ELEMENT_MAX;++i )
	{
		for( int num = 0; num < mBubbleBox[i].mpBoxElement->GetNumPolygons(); ++num)
		{
			mBubbleBox[i].mpBoxElement->SetColors( num, NiColorA(1.0f, 1.0f, 1.0f, DEFAULT_ALPH) );
		}
	}
}


//////////////////////////////////////////////////////////////////////////

cFuncChatBubble::cFuncChatBubble()
{

}

cFuncChatBubble::~cFuncChatBubble()
{

}

/// 말풍선을 띄우다
void cFuncChatBubble::Active( LPTSTR msg, unsigned long color, bool loop )
{
	if( msg == 0 )	
		return;

	mAlphaColor = 0xffffffff;

	unsigned int msgLen = ::_tcslen( msg );
	if( msgLen <= 0 )
	{
		assert( 0 );
		return;
	}

	unsigned int cpyNum = 0;
	mLineNum = 0;
	mChatMsgWidth = 0;

	for( unsigned int i=0;i<BUBBLE_LINE_MAX;i++ )
		mChatMsg[i].Clear();


	/// 말풍선 정보를 셋팅한다.
	//TCHAR buff[BUBBLE_TEXT_LENGTH+1] = {0,};
	while( msgLen > BUBBLE_TEXT_LENGTH )
	{
		if( ( msg + BUBBLE_TEXT_LENGTH ) != ::CharNext( ::CharPrev( msg, msg + BUBBLE_TEXT_LENGTH ) ) )
		{
			cpyNum = BUBBLE_TEXT_LENGTH - 1;
		}
		else
		{
			cpyNum = BUBBLE_TEXT_LENGTH;
		}

		sTextItem text;

		assert(cpyNum < 512);
		Sstrncpy( text.mText, 512, msg, cpyNum );
		text.mColor = color & mAlphaColor;
		mChatMsg[mLineNum].PushBack( text );
		mChatMsgLen[mLineNum] = cpyNum;

		msgLen -= cpyNum;
		msg += cpyNum;
		if( *msg == _T(' ') ) 
		{
			/// 다음 문자 시작이 스페이스면 스킵
			++msg;
		}

		if( ++mLineNum >= BUBBLE_LINE_MAX-1 )
		{
			break;
		}
	}

	if( msgLen > 0 )
	{
		if( mLineNum < BUBBLE_LINE_MAX )
		{
			sTextItem text;
			Sstrncpy( text.mText, 512, msg, BUBBLE_TEXT_LENGTH );
			text.mColor = color;
			mChatMsg[mLineNum].PushBack( text );

			mChatMsgLen[mLineNum] = msgLen;

			++mLineNum;
		}
		else
		{
			assert(0);
		}
	}

	if( mLineNum > BUBBLE_LINE_MAX )
	{
		assert(0);
	}

	/// msg가 적힐 부분의 넓이
	for( unsigned int i= 0; i<mLineNum; ++i )
	{
		unsigned int msgWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mChatMsg[i][0].mText, mChatMsgLen[i] )
			+ TEXT_FRAME_SPACE * 2;

		if( mChatMsgWidth < msgWidth ) 
			mChatMsgWidth = msgWidth;
	}

	/// error 처리
	if( mLineNum == 0 )
	{
		assert(0);
		return;
	}

	/// 프레임 넓이 셋팅
	mBubbleBox[mLineNum-1].mFrameWidth = mChatMsgWidth + BUBBLE_FRAME_LR_SPACE*2;

	/// 속성값 적용
	mActive = true;
	mLoop = loop;
	mActiveTime = THEAPP->GetWorldAccumTime();

	for( int i=0; i<ELEMENT_MAX;++i )
	{
		for( int num = 0; num < mBubbleBox[i].mpBoxElement->GetNumPolygons(); ++num)
		{
			mBubbleBox[i].mpBoxElement->SetColors( num, NiColorA(1.0f, 1.0f, 1.0f, DEFAULT_ALPH) );
		}
	}
}

void cFuncChatBubble::Active( sTextItem* text, bool loop )
{
	if( text == 0 )
		return;

	mLineNum = 0;
	mChatMsgWidth = 0;

	for( unsigned int i=0;i<BUBBLE_LINE_MAX;i++ )
	{
		mChatMsg[i].Clear();
		mChatMsgLen[i] = 0;
	}

	sTextItem* checkText = text;

	unsigned int remain = BUBBLE_TEXT_LENGTH;
	while( checkText )
	{
		cStringT str;
		unsigned long color = checkText->mColor;

		for(;;)
		{
			if( checkText == 0 )
				break;
			if( color != checkText->mColor )
				break;

			str += checkText->mText;
			checkText = checkText->mpNext;
		}

		LPCTSTR msg = str.Cstr();
		unsigned int msgLen = str.GetLength();
		unsigned int cpyNum = 0;

		while( msgLen > remain )
		{
			if( ( msg + remain ) != ::CharNext( ::CharPrev( msg, msg + remain ) ) )
			{
				cpyNum = remain - 1;
			}
			else
			{
				cpyNum = remain;
			}

			sTextItem text;
			Sstrncpy( text.mText, 512, msg, cpyNum );
			text.mColor = color & mAlphaColor;
			mChatMsg[mLineNum].PushBack( text );
			mChatMsgLen[mLineNum] += cpyNum;

			msgLen -= cpyNum;
			msg += cpyNum;
			if( *msg == _T(' ') ) 
			{
				/// 다음 문자 시작이 스페이스면 스킵
				++msg;
			}

			if( ++mLineNum >= BUBBLE_LINE_MAX-1 )
			{
				break;
			}

			remain = BUBBLE_TEXT_LENGTH;
		}

		if( msgLen > 0 )
		{
			if( mLineNum < BUBBLE_LINE_MAX )
			{
				sTextItem text;
				Sstrncpy( text.mText, 512, msg, BUBBLE_TEXT_LENGTH );
				text.mColor = color & mAlphaColor;
				mChatMsg[mLineNum].PushBack( text );

				mChatMsgLen[mLineNum] += msgLen;
				remain -= msgLen;

				if( remain == 0 )
				{
					remain = BUBBLE_TEXT_LENGTH;
					++mLineNum;
				}
				else
				{
					if( checkText == 0 )
						++mLineNum;
				}
			}
			else
			{
				assert(0);
			}
		}

		if( mLineNum >= BUBBLE_LINE_MAX )
			break;
	}

	if( mLineNum > BUBBLE_LINE_MAX )
	{
		assert(0);
	}

	/// msg가 적힐 부분의 넓이
	for( unsigned int i= 0; i<mLineNum; ++i )
	{
		unsigned int msgWidth = TEXT_FRAME_SPACE * 2;
		for( unsigned int k=0;k<mChatMsg[i].GetSize();k++ )
		{
			mChatMsg[i][k].mWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, mChatMsg[i][k].mText, ::_tcslen( mChatMsg[i][k].mText) );
			msgWidth += mChatMsg[i][k].mWidth;
		}

		if( mChatMsgWidth < msgWidth ) 
			mChatMsgWidth = msgWidth;
	}

	/// error 처리
	if( mLineNum == 0 )
	{
		assert(0);
		return;
	}


	/// 프레임 넓이 셋팅
	mBubbleBox[mLineNum-1].mFrameWidth = mChatMsgWidth + BUBBLE_FRAME_LR_SPACE*2;

	/// 속성값 적용
	mActive = true;
	mLoop = loop;
	mActiveTime = THEAPP->GetWorldAccumTime();

	for( int i=0; i<ELEMENT_MAX;++i )
	{
		for( int num = 0; num < mBubbleBox[i].mpBoxElement->GetNumPolygons(); ++num)
		{
			mBubbleBox[i].mpBoxElement->SetColors( num, NiColorA(1.0f, 1.0f, 1.0f, DEFAULT_ALPH) );
		}
	}
}

void cFuncChatBubble::Draw()
{
	NiRenderer* pRenderer = NiRenderer::GetRenderer();
	if( mLineNum <= 0 )
	{
		assert( 0 );
	}

	if( mLineNum > BUBBLE_LINE_MAX )
	{
		assert( 0 );
	}

	/// 말풍선 그리기
	mBubbleBox[mLineNum-1].mpBoxElement->RenderImmediate(pRenderer);

	POINT pos;
	pos.y = mPosY + mBubbleBox[mLineNum-1].mFrameHeight - BUBBLE_FRAME_BOTTOM_SPACE;

	/// 글자 찍기
	pos.y -= TEXT_FRAME_HEIGHT - mFontSpace;
	for( unsigned int i = mLineNum; i>0; --i )
	{
		pos.x = mPosX + BUBBLE_FRAME_LR_SPACE + TEXT_FRAME_SPACE;

		for( unsigned int k=0;k<mChatMsg[i-1].GetSize();k++ )
		{
			FONTAGENT->DrawText( cFontAgent::eFont_Chat, mChatMsg[i-1][k].mText, mChatMsgLen[i-1], pos.x, pos.y, mChatMsg[i-1][k].mColor, true );

			pos.x += mChatMsg[i-1][k].mWidth;
		}
		pos.y -= TEXT_FRAME_HEIGHT;
	}
}