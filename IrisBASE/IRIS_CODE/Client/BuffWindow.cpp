#include "Stdafx.h"
#include "BuffWindow.h"

#include "FontAgent.h"
#include "UISkinLexer.h"
#include "SkillManager.h"
#include "SkillScript.h"
#include "icon.h"
#include "GameUIManager.h"
#include "ObjectManager.h"
#include "hero.h"
#include "GameResourceManager.h"
#include "Skill_Common.h"

cBuffWindow::cBuffWindow()
{
	mUpdateList = false;

	mStartX = mStartY = 0;

	mOverInfo.mOverIconIdx = (unsigned int)-1;
	mOverCheck = false;
	mChangeOverIcon = false;

	mRButtonClickIconIdx = (unsigned int)-1;

	mIsHeroBuffPage = false;

	mRowSpace = 0;
	mColumnSpace = 0;

	mBuffTipShow = false;

	mRow = 0;
	mColumn = 0;
	mIconSize = 0;
	mScale = 1;

	mImageFont = 0;

	/// 다른 윈도우들이 안붙게 한다
	SetExceptSnap( true );
}

cBuffWindow::~cBuffWindow()
{
	cBuffIconList::cIterator i = mIconList.Begin();
	cBuffIconList::cIterator iend = mIconList.End();

	for( ; i != iend; ++i )
	{
		sBuffIcon* p = (sBuffIcon*)(*i);
		delete p;
	}
	mIconList.Clear();
}

void cBuffWindow::Clear()
{
	mUpdateList = false;

	mOverInfo.mOverIconIdx = (unsigned int)-1;
	mOverCheck = false;
	mChangeOverIcon = false;

	mRButtonClickIconIdx = (unsigned int)-1;

	cBuffIconList::cIterator i = mIconList.Begin();
	cBuffIconList::cIterator iend = mIconList.End();

	for( ; i != iend; ++i )
	{
		sBuffIcon* p = (sBuffIcon*)(*i);
		delete p;
	}
	mIconList.Clear();
}

void cBuffWindow::Open()
{
}

void cBuffWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

bool cBuffWindow::OnCreate( cUINodeProperty* pproperty )
{
	mImageFont = UIMAN->GetImageFont( eIAMGEFONT_ITEMCOUNT );

	return cUIWindow::OnCreate( pproperty );
}


void cBuffWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mIconList.IsEmpty() )
		Hide();
}

void cBuffWindow::OnHide()
{
	cUIWindow::OnHide();

	mOverInfo.mOverIconIdx = (unsigned int)-1;
	mChangeOverIcon = false;
	mOverCheck = false;
}


bool cBuffWindow::SetSkin( const cUINodeSkin* pskin )
{
	cBuffWindowSkin* p = (cBuffWindowSkin*)pskin;

	mHoldView = p->mHoldView;

	mStartX = p->mSkinInfo->mX;
	mStartY = p->mSkinInfo->mY;

	/// 부모가 루트 타입이면 모두 갱신
	if( mpParent->GetType() == eUINODE_ROOT )
	{
		mStartX = GetScreenWidth() - mDefaultWidth + p->mSkinInfo->mX;
	}

	mRow = p->mRow;
	mColumn = p->mColumn;

	if( mRowSpace == 0 )
		mRowSpace = p->mRowSpace;
	if( mColumnSpace == 0 )
		mColumnSpace = p->mColumnSpace;

	mIconSize = p->mIconSize;
	mScale = p->mScale;

	SetRelativeRect( cUIRect(mStartX,mStartY,mStartX,mStartY) );

	mTimerEnabled = true;
	mTimeOut = 100;

	return true;
}

void cBuffWindow::UpdateSkin()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
		return;
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
		return;
	}

	/// 사이즈는 새로운 상대좌표 * 새로운 해상도 
	mStartX = pnodeSkin->mSkinInfo->mX;
	mStartY = pnodeSkin->mSkinInfo->mY;

	/// 부모가 루트 타입이면 모두 갱신
	if( mpParent->GetType() == eUINODE_ROOT )
	{
		mStartX = GetScreenWidth() - mDefaultWidth + pnodeSkin->mSkinInfo->mX;
	}

	/// 적용
	SetRelativeRect( cUIRect(mStartX,mStartY,mStartX,mStartY) );

	/// 기존 해상도 저장 
	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();

	mUpdateList = true;
}

void cBuffWindow::OnMouseMove( const cUIPos& pos )
{
	/// 1초 단위 체크
	if( mOverCheck == false )
		return;

	mOverCheck = false;

	unsigned int x = mStartX;
	unsigned int y = mStartY;

	if( mpParent )
	{
		x += mpParent->GetAbsoluteRect().mLeft;
		y += mpParent->GetAbsoluteRect().mTop;
	}

	unsigned int col;
	for( col=0; col<mColumn; col++ )
	{
		unsigned int s = x + col*(mIconSize + mColumnSpace);
		if( s <= (unsigned int)pos.mX && (unsigned int)pos.mX <= s + mIconSize )
			break;
	}
	if( col == mColumn )
		return;

	col = mColumn-1 - col;

	unsigned int row;
	for( row=0; row<mRow; row++ )
	{
		unsigned int s = y + row*(mIconSize + mRowSpace);
		if( s <= (unsigned int)pos.mY && (unsigned int)pos.mY <= s + mIconSize )
			break;
	}
	if( row == mRow )
		return;

	unsigned int over = row*mColumn + col;
	if( mOverInfo.mOverIconIdx != over )
	{
		mChangeOverIcon = true;
		mOverInfo.mOverIconIdx = over;
		mOverInfo.mPos = pos;
	}
}

void cBuffWindow::OnMouseLeft( const cUIPos& )
{
	mOverInfo.mOverIconIdx = (unsigned int)-1;
	mChangeOverIcon = false;

	HideTip();
}

void cBuffWindow::OnTimer()
{
	mOverCheck = true;
}

void cBuffWindow::OnRButtonDown( const cUIPos& pos, bool /*ctrl*/, bool /*alt*/, bool /*shift*/ )
{
	if( HERO->IsJumping() == true )
		return;

	if( IsNetLock() == true )
		return;

	unsigned int x = mStartX;
	unsigned int y = mStartY;

	if( mpParent )
	{
		x += mpParent->GetAbsoluteRect().mLeft;
		y += mpParent->GetAbsoluteRect().mTop;
	}

	unsigned int col;
	for( col=0; col<mColumn; col++ )
	{
		unsigned int s = x + col*(mIconSize + mColumnSpace);
		if( s <= (unsigned int)pos.mX && (unsigned int)pos.mX <= s + mIconSize )
			break;
	}
	if( col == mColumn )
		return;

	col = mColumn-1 - col;

	unsigned int row;
	for( row=0; row<mRow; row++ )
	{
		unsigned int s = y + row*(mIconSize + mRowSpace);
		if( s <= (unsigned int)pos.mY && (unsigned int)pos.mY <= s + mIconSize )
			break;
	}
	if( row == mRow )
		return;

	unsigned int over = row*mColumn + col;

	mRButtonClickIconIdx = over;
}

void cBuffWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mVisible == false )
		return;

	unsigned int count = 0;
	cBuffIconList::cIterator i = (--mIconList.End());
	cBuffIconList::cIterator iend = (--mIconList.Begin());
	for( ; i != iend; --i )
	{
		sBuffIcon* p = (sBuffIcon*)(*i);
		if( p == 0 )
		{
			assert(0);
			return;
		}

		sInfluenceScript* pScript = SKILLSCRIPT->GetInfluenceInfo( p->mInfluenceIdx );
		if( pScript == 0 )
		{
			assert(0);
			return;
		}

		/// 깜빡이기
		if( p->mStartBlink )
		{
			if( p->mBlinkTime == 0 )
				p->mBlinkTime = accumTime;

			unsigned long checkTime = (p->mShow==true)? 1000:500;
			if( checkTime <= accumTime - p->mBlinkTime )
			{
				p->mBlinkTime = accumTime;
				p->mShow = !p->mShow;
			}
		}

		/// 남은 시간 획득
		cInfluenceObject* pInf = SKILLMAN->GetInfluenceObject( p->mUniqueIdx );
		if( pScript->mType == eINFLUENCETYPE_BUF || pScript->mType == eINFLUENCETYPE_DEBUF ||  pScript->mType == eINFLUENCETYPE_CASH )
		{
			if( pInf && pInf->GetIsRealTime() == false )
			{
				unsigned int restTime = pInf->GetRestTime()/1000;
				if( restTime == 0 )
				{
					p->mUnit.Clear();
				}
				else
				{
					unsigned int d = (restTime / 60) / 60 / 24;
					unsigned int h = (restTime / 60) / 60 % 24;
					unsigned int m = (restTime / 60) % 60;
					unsigned int s = (restTime % 60) % 60 + 1;

					/// 반올림해서 출력
					if( d != 0 )
						d++;
					if( h != 0 )
						h++;
					if( m != 0 )
						m++;

					if( d != 0 )
						p->mUnit.Format( GAMERESOURCEMAN->GetGameText( 74 ), d );
					else if( h != 0 )
						p->mUnit.Format( GAMERESOURCEMAN->GetGameText( 70 ), h );
					else if( m != 0 )
						p->mUnit.Format( GAMERESOURCEMAN->GetGameText( 71 ), m );
					else
						p->mUnit.Format( GAMERESOURCEMAN->GetGameText( 72 ), s );
				}
			}
		}
		else
			p->mUnit.Clear();

		if( pInf )
			p->mGaurdCnt = pInf->GetGuardCnt();

		if( count == mRow*mColumn )
			continue;

		/// 영역 재설정
		if( mUpdateList )
		{
			unsigned int sx = mStartX;
			unsigned int sy = mStartY;
			if( mpParent )
			{
				cUIRect parentAbsolute = mpParent->GetAbsoluteRect();
				sx += parentAbsolute.mLeft;
				sy += parentAbsolute.mTop;
			}

			unsigned int x = sx + ((mColumn-(count%mColumn)-1) * (mIconSize+mColumnSpace));
			unsigned int y = sy + ((count/mColumn) * (mIconSize+mRowSpace));

			p->mImage.SetScreenRect( cUIRect(x, y, x+mIconSize, y+mIconSize) );
			p->mTextX = x;
			p->mTextY = y+mIconSize + 1;
		}

		if( mChangeOverIcon && mOverInfo.mOverIconIdx == count )
		{
			EventOverIcon( p, mOverInfo.mPos );
			mChangeOverIcon = false;
		}

		if( mRButtonClickIconIdx == count )
		{
			mRButtonClickIconIdx = (unsigned int)-1;

			if( pScript->mTypeDetail == eINFLUENCETYPEDETAIL_NAME )
				return;

			if( p->mAuraType == false )
			{
				if( pScript->mType == eINFLUENCETYPE_BUF )
				{
					MSG_REQ_INFLUENCE_DELETE Msg;
					Msg.Category = NM_SKILL;
					Msg.Protocol = NM_SKILL_INFLUENCE_DELETE_REQ;
					Msg.mUniqueIdx = p->mUniqueIdx;
					NETWORK->SendNetworkMsg( (char*)&Msg, sizeof( Msg ) );

					/// 락
					NetLock();
				}
				else if( pScript->mType == eINFLUENCETYPE_CASH )
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_DELETE_CASHBUFF, eSKIN_YESNO,
						GAMERESOURCEMAN->GetGameText( 6501 ),
						GAMERESOURCEMAN->GetGameText( 6500 ),
						(void*)p->mUniqueIdx );
				}
			}
		}

		count++;
	}
}

void cBuffWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mUpdateList )
	{
		/// window 재계산
		cUIRect r;
		r.mRight = mStartX + mColumn*(mIconSize+mColumnSpace) - mColumnSpace;
		r.mTop = mStartY;

		if( mIconList.GetSize() < mColumn )
			r.mLeft = mStartX + (mColumn - mIconList.GetSize())*(mIconSize+mColumnSpace);
		else
			r.mLeft = mStartX;

		unsigned int row = min((mIconList.GetSize()-1)/mColumn + 1, mRow);
		r.mBottom = mStartY + row * (mIconSize+mRowSpace) - mRowSpace;

		SetRelativeRect( r );
	}

	unsigned int count = 0;
	cBuffIconList::cIterator i = (--mIconList.End());
	cBuffIconList::cIterator iend = (--mIconList.Begin());
	for( ; i != iend; --i )
	{
		if( count == mRow*mColumn )
			break;

		sBuffIcon* p = (sBuffIcon*)(*i);
		if( !p )
		{
			assert(0);
			return;
		}

		if( p->mShow )
		{
			p->mImage.Draw();

			if( p->mGaurdCnt != 0 )
			{
				cUIRect rc;
				rc.mLeft = p->mImage.GetScreenPosX();
				rc.mRight = rc.mLeft + p->mImage.GetWidth();
				rc.mTop = p->mImage.GetScreenPosY();
				rc.mBottom = rc.mTop + p->mImage.GetHeight();

				if( p->mGaurdCnt > 999 )
				{
					cStringT str = _T("*");
					int w = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, const_cast<LPTSTR>(str.Cstr()), str.GetLength() );
					int h = FONTAGENT->GetTextHeight( cFontAgent::eFont_UI );
					pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(str.Cstr()), rc.mRight - w - 3, rc.mBottom - h + 1, COLOR_WHITE );
				}
				else
				{
					cString str;
					str.Format( "%d", p->mGaurdCnt );

					if( mImageFont )
						mImageFont->DrawText( rc.mRight - mImageFont->GetTextWidth(p->mGaurdCnt) - 1, rc.mBottom - mImageFont->GetTextHeight() - 1, str.Cstr() );
				}
			}
		}

		pKeeper->AddFontItem( cFontAgent::eFont_System, p->mUnit.Cstr(), p->mTextX, p->mTextY, COLOR_WHITE );

		count++;
	}
	mUpdateList = false;

	pKeeper->DrawAll();
}

void cBuffWindow::AddIcon( unsigned long uniqueIdx, unsigned long influenceIdx )
{
	sInfluenceScript* pInfo = SKILLSCRIPT->GetInfluenceInfo( influenceIdx );
	if( pInfo == 0 )
	{
		assert(0);
		return;
	}
	cIconParam* pParam = UIMAN->GetIconParam( pInfo->mIconIdx );
	if( pParam == 0 )
	{
		assert(0);
		return;
	}

	sBuffIcon* p = new sBuffIcon;
	p->mTotalTime = pInfo->mContinuanceTime;

	p->mAuraType = (pInfo->mTypeDetail == eINFLUENCETYPEDETAIL_AURA);

	p->mUniqueIdx = uniqueIdx;
	p->mInfluenceIdx = influenceIdx;

	/// 이미지 셋팅
	p->mImage.SetTexture( pParam->mpTexture );

	/// 텍스처 영역 설정
	unsigned int tx = pParam->mTexPos.mX;
	unsigned int ty = pParam->mTexPos.mY;
	p->mImage.SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );

	mIconList.PushBack( p );

	HideTip();

	mUpdateList = true;

	/// npc 윈도우가 보이면 우선 예약
	if( HERO->IsOpenNpcTalk() == false && GAMEUI->IsDuelMode() == false )
		Show();
	else
		UIMAN->SetRestoreShow( this );

	mRButtonClickIconIdx = (unsigned int)-1;
}

void cBuffWindow::DelIcon( unsigned long uniqueIdx )
{
	cBuffIconList::cIterator i = mIconList.Begin();
	cBuffIconList::cIterator iend = mIconList.End();
	for( ; i != iend; ++i )
	{
		sBuffIcon* p = (sBuffIcon*)(*i);
		if( !p )
		{
			assert(0);
			return;
		}
		if( p->mUniqueIdx == uniqueIdx )
		{
			delete p;
			mIconList.Erase( i );
			break;
		}
	}

	HideTip();

	mUpdateList = true;
	if( mIconList.IsEmpty() )
		Hide();

	mRButtonClickIconIdx = (unsigned int)-1;
}

void cBuffWindow::BlinkIcon( unsigned long uniqueIdx, unsigned long influenceIdx )
{
	cBuffIconList::cIterator i = mIconList.Begin();
	cBuffIconList::cIterator iend = mIconList.End();
	for( ; i != iend; ++i )
	{
		sBuffIcon* p = (sBuffIcon*)(*i);
		if( !p )
		{
			assert(0);
			return;
		}

		if( p->mInfluenceIdx == influenceIdx && p->mUniqueIdx == uniqueIdx )
		{
			p->mStartBlink = true;
			break;
		}
	}
}

void cBuffWindow::EventOverIcon( sBuffBaseIcon* p, const cUIPos& pos )
{
	cBuffToolTipWindow* pTip = GAMEUI->GetBuffTip();
	if( pTip && p )
	{
		mBuffTipShow = true;
		UIMAN->ShowBuffTip( pTip, pos, p->mUniqueIdx, p->mInfluenceIdx, mIsHeroBuffPage );
	}
}

void cBuffWindow::HideTip()
{
	if( mBuffTipShow == true )
	{
		mBuffTipShow = false;
		UIMAN->HideTip();
		mOverInfo.mOverIconIdx = (unsigned long)-1;
	}
}

void cBuffWindow::OnNetLock( int lockTry )
{
	cUINode::NetLock( lockTry );
}

void cBuffWindow::OnNetUnLock( int lockTry )
{
	cUINode::NetUnLock( lockTry );
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
cBuffWindowSkin::cBuffWindowSkin( eUINodeType type )
: cUIWindowSkin(type)
{
	mRowSpace = 0;
	mColumnSpace = 0;

	mRow = 0;
	mColumn = 0;

	mIconSize = 0;
	mScale = 1.0f;
}

cBuffWindowSkin::~cBuffWindowSkin()
{
}

bool cBuffWindowSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			/// 블록 탈출 ^^
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_ICON_SIZE:
			{
				mIconSize = parser.ParseInt();
			}
			break;
		case eTOKEN_BUFF_SPACE:
			{
				mColumnSpace = parser.ParseInt();
				mRowSpace = parser.ParseInt();
			}
			break;
		case eTOKEN_BUFF_ROW:
			{
				mRow = parser.ParseInt();
			}
			break;
		case eTOKEN_BUFF_COLUMN:
			{
				mColumn = parser.ParseInt();
			}
			break;
		case eTOKEN_BUFF_SCALE:
			{
				mScale = (float)parser.ParseInt()/100.0f;
			}
			break;
		case eTOKEN_HOLDVIEW:
			{
				if( parser.ParseString().Compare( "true" ) == 0 )
					mHoldView = true;
				else
					mHoldView = false;
			}
			break;
		default:
			if( cUINodeSkin::ParseLine( parser, token ) == false )
			{
				return false;
			}
			break;
		}
	}
	return true;
}
