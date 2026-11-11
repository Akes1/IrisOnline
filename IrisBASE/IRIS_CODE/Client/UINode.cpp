#include "StdAfx.h"
#include "UINode.h"

#include "UINodeData.h"
#include "UINodeProperty.h"
#include "UIContainer.h"
#include "UIEvent.h"
#include "UIImage.h"
#include "UIRootNode.h"
#include "UIIDList.h"
#include "UIID.h"

#include "FontAgent.h"
#include "RenderSystem.h"

cUINode::cUINode( eUINodeType type )
: mType( type )
, mpParent( 0 )
, mID( ULONG_MAX )
, mFrontID( ULONG_MAX )
, mpImage( 0 )
, mVisible( true )
, mEnabled( true )
, mPressed( false )
, mMove( false )
, mAlpha( 1.0f )
, mNetLock( false )
, mLockTry( -1 )
, mBackupVisible( true )
, mOutLine( false )
, mFontType( cFontAgent::eFont_UI )
{
	mSnapType = eSNAP_NONE;

	mRelativeRect.Set(0, 0, 0, 0);
	mAbsoluteRect.Set(0, 0, 0, 0);

	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();

	mDefaultColor = COLOR_DEFAULT;

	/// 스크립트의 좌표는 해당 사이즈를 기준으로 한다
	mDefaultWidth = DEFAULT_SCREENWIDTH;
	mDefaultHeight = DEFAULT_SCREENHEIGHT;

	mExceptionEvent     = false;
	mProcessEnable	    = false;
	mIsSetCursorDefault = true;

	mUseBoldText = false;

	mHoverTipIndex = 0;
}

cUINode::~cUINode()
{
	Destroy();
}

///
bool cUINode::CreateByProperty( cUINodeProperty* pProperty, cUIContainer* pparent )
{
	assert( UIMAN && "null ui manager" );
	assert( GetRoot() && "null ui root node" );

	/// 아이디를 검사
	if( mID != ULONG_MAX )
	{
		assert( 0 && "ui node is already created" );
		return false;
	}

	/// 아이디 설정
	/// 프로퍼티의 아이디는 아이디리스트에서 따로가져온다.
	cUIIDList* pIDList = UIMAN->GetUIIDList();
	if( pIDList == 0 )
	{
		assert( 0 && "null ID List" );
		return false;
	}

	/// 아이디 설정
	mID = pIDList->GetPropID( pProperty->mPropertyName );

	/// 스킨 이름을 설정
	mSkinName = pProperty->mSkinName;

	/// 공통적인 부분을 세팅 
	if( SetupNode( pparent ) == false )
		return false;

	if( OnCreate(pProperty) == false )
		return false;

	if( mpParent->IsKindof( eUINODE_ROOT ) == true )
	{
		GetRoot()->AddProcessChild( this );
		GetRoot()->AddSearchChild( this );
	}

	return IsValid();
}

///
bool cUINode::CreateByPropertyId( cUINodeProperty* pProperty, cUIContainer* pparent, unsigned long id )
{
	assert( UIMAN && "null ui manager" );
	assert( GetRoot() && "null ui root node" );

	/// 아이디를 검사
	if( mID != ULONG_MAX )
	{
		assert( 0 && "ui node is already created" );
		return false;
	}

	/// 스킨 이름을 설정
	mSkinName = pProperty->mSkinName;

	/// 아이디 설정
	mID = id;

	/// 공통적인 부분을 세팅 
	if( SetupNode( pparent ) == false )
		return false;

	if( OnCreate(pProperty) == false )
		return false;

	// 
	if( mpParent->IsKindof( eUINODE_ROOT ) == true )
	{
		GetRoot()->AddProcessChild( this );
		GetRoot()->AddSearchChild( this );
	}

	return IsValid();
}

// 주의사항 : 스킨이 필요한 일반적인 윈도우는 이걸로 생성 
bool cUINode::CreateBySkinName( const cString& skinName, cUIContainer* pparent, unsigned long id )
{
	assert( UIMAN && "null ui manager" );
	assert( GetRoot() && "null ui root node" );

	/// 아이디를 검사
	if( mID != ULONG_MAX )
	{
		assert( 0 && "ui node is already created" );
		return false;
	}

	/// 스킨 이름을 설정
	mSkinName = skinName;

	/// 아이디 설정
	mID = id;

	/// 공통적인 부분을 세팅 
	if( SetupNode( pparent ) == false )
		return false;

	if( OnCreate() == false )
		return false;

	if( mpParent->IsKindof( eUINODE_ROOT ) == true )
	{
		GetRoot()->AddProcessChild( this );
		GetRoot()->AddSearchChild( this );
	}

	return IsValid();
}

bool cUINode::CreateByPropertyName( const cString& propertyName, cUIContainer* pparent )
{
	assert( UIMAN && "null ui manager" );
	assert( GetRoot() && "null ui root node" );

	/// 아이디를 검사
	if( mID != ULONG_MAX )
	{
		assert( 0 && "ui node is already created" );
		return false;
	}

	cUIProperty* prop = UIMAN->GetProperty();
	cUINodeProperty* nodeProp = prop->GetNodeProperty( propertyName );
	if( nodeProp == 0 )
	{
		assert( 0 && "failed to find ui node property" );
		return false;
	}

	/// 프로퍼티의 아이디는 아이디리스트에서 따로가져온다.
	cUIIDList* pIDList = UIMAN->GetUIIDList();
	if( pIDList == 0 )
	{
		assert( 0 && "null ID List" );
		return false;
	}

	/// 아이디를 설정
	mID = pIDList->GetPropID( nodeProp->mPropertyName );

	/// 스킨 이름을 설정
	mSkinName = nodeProp->mSkinName;

	/// 공통적인 부분을 세팅
	if( SetupNode( pparent ) == false )
		return false;

	if( OnCreate(nodeProp) == false )
		return false;

	if( mpParent->IsKindof( eUINODE_ROOT ) == true )
	{
		GetRoot()->AddProcessChild( this );
		GetRoot()->AddSearchChild( this );
	}

	return IsValid();
}

/// 주의사항 : 각 노드 생성방법의 공통적인 부분을 세팅 
bool cUINode::SetupNode( cUIContainer* pparent )
{
	/// 부모를 설정
	assert( mpParent == 0 && "parent already exists!" );

	if( pparent == 0 )
	{
		mpParent = GetRoot();
	}
	else if( pparent->IsKindof( eUINODE_CONTAINER ) )
	{
		mpParent = pparent;
	}
	else
	{
		assert( 0 && "parent should be kind of container");
		return false;
	}

	/// 스킨을 적용
	if( ApplySkin() == false )
	{
		assert( 0 && "failed to apply node skin");
		return false;
	}

	/// 부모에 추가
	mpParent->AddChild( this );

	/// 부모가 최상위루트가 아니면 알파값을 가져와서 세팅 
	if( mpParent != GetRoot() )
	{
		SetAlpha( mpParent->GetAlpha() );
	}

	/// 초반에 윈도우 생성시 visible 세팅 
	if( mType == eUINODE_WINDOW || mType == eUINODE_TABWINDOW )
	{
		mVisible = false;
	}

	return true;
}

/// 주의사항 : 
void cUINode::Destroy()
{
	/// 부모로부터 제거
	if( mpParent )
	{
		mpParent->RemoveChild( this );
	}

	if( mID )
	{
		OnDestroy();

		/// 마무리~
		mID = ULONG_MAX;
	}

	if( mFrontID != ULONG_MAX )
	{
		UIMAN->RemoveFrontNode( mFrontID );

		mFrontID = ULONG_MAX;
	}

	/// 이미지 제거
	delete mpImage;
	mpImage = 0;
}

/// 주의사항 : 스킨에서 읽어들인 값  적용
bool cUINode::ApplySkin()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert( 0 && "null skin" );
		return false;
	}

	/// 노드 스킨 검색
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert( 0 && "failed to find node skin" );
		return false;
	}
	
	/// 노드 스킨 설정
	if( SetSkin( pnodeSkin ) == false )
	{
		return false;
	}
	return true;
}

/// 주의사항 : 
bool cUINode::SetSkin( const cUINodeSkin* skin )
{
	if( skin == 0 )
	{
		assert( 0 && "null skin" );
		return false;
	}

	/// 노드 스킨의 적합성 검사
	if( skin->IsValid() == false )
	{
		assert( 0 && "invalid skin" );
		return false;
	}

	/// 이미지 설정
	if( skin->mpTexture )
	{
		if( mpImage == 0 )
		{
			mpImage = new cUIImage( skin->mpTexture );
		}
		else
		{
			assert(0);
		}

		/// 컬러를 지정
		SetColor( skin->mColor );
//		mpImage->SetColor( skin->mColor );
		
		/// 텍스처 영역 설정
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int tw = skin->mSkinInfo->mTexWidth;
		unsigned int th = skin->mSkinInfo->mTexHeight;

		mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );
	}
	else
	{
		delete mpImage;
		mpImage = 0;
	}

	/// 루트의 바로 자식인 윈도우들은 기존좌표를 저장한채 해상도를 변경
	if( mpParent == 0 )
	{
		assert(0);
		return false;
	}

	int x = skin->mSkinInfo->mX;
	int y = skin->mSkinInfo->mY;
	unsigned int w = skin->mSkinInfo->mWidth;
	unsigned int h = skin->mSkinInfo->mHeight;

	/// 부모가 루트 타입이면 모두 갱신
	if( mpParent->GetType() == eUINODE_ROOT )
	{
		x = ( GetScreenWidth() * skin->mSkinInfo->mX ) / mDefaultWidth;
		y = ( GetScreenHeight() * skin->mSkinInfo->mY ) / mDefaultHeight;

		/// 오른쪽이나 아랫쪽에 윈도우가 붙어있으면 해상도 변경시에도
		/// 붙어있도록 한다 
		if( (skin->mSkinInfo->mX + skin->mSkinInfo->mWidth) >= (int)mDefaultWidth )
		{
			x = GetScreenWidth() - w;
		}

		if( (skin->mSkinInfo->mY + skin->mSkinInfo->mHeight) >= (int)mDefaultHeight )
		{
			y = GetScreenHeight() - h;
		}
	}

	mHoverTipIndex = skin->mSkinInfo->mHoverTipIndex;

	/// 적용
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );
	return true;
}

/// 주의사항 : 해상도 변경시 호출 
void cUINode::UpdateSkin()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
	}

	/// 루트의 바로 자식인 윈도우들은 기존좌표를 저장한채 해상도를 변경
	if( mpParent == 0 )
	{
		assert(0);
		return;
	}

	/// 사이즈는 새로운 상대좌표 * 새로운 해상도 
	int x = pnodeSkin->mSkinInfo->mX;
	int y = pnodeSkin->mSkinInfo->mY;
	unsigned int w = pnodeSkin->mSkinInfo->mWidth;
	unsigned int h = pnodeSkin->mSkinInfo->mHeight;

	/// 부모가 루트 타입이면 모두 갱신
	if( mpParent->GetType() == eUINODE_ROOT )
	{
  		/*x = ( GetScreenWidth() * GetAbsoluteRect().mLeft ) / mOldScreenWidth;
		y = ( GetScreenHeight() * GetAbsoluteRect().mTop ) / mOldScreenHeight;*/
		x = ( GetScreenWidth() * pnodeSkin->mSkinInfo->mX ) / mDefaultWidth;
		y = ( GetScreenHeight() * pnodeSkin->mSkinInfo->mY ) / mDefaultHeight;

		/// 오른쪽이나 아랫쪽에 윈도우가 붙어있으면 해상도 변경시에도
		/// 붙어있도록 한다 
		if( GetAbsoluteRect().mRight >= (int)mOldScreenWidth )
		{
			x = GetScreenWidth() - w;
		}

		if( GetAbsoluteRect().mBottom >= (int)mOldScreenHeight )
		{
			y = GetScreenHeight() - h;
		}
	}

	mHoverTipIndex = pnodeSkin->mSkinInfo->mHoverTipIndex;

	/// 적용
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );

	/// 기존 해상도 저장 
	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();
}

/// 주의사항 : 
bool cUINode::IsValid() const
{
	if( mpParent == 0 )
	{
		assert( 0 && "null parent" );
		return false;
	}
	if( mpImage && mpImage->IsValid() == false )
	{
		assert( 0 && "invalid image" );
		return false;
	}
	if( mpImage && mpImage->GetTextureRect().IsValid() == false )
	{
		assert( 0 && "invalid tex rect" );
		return false;
	}
	return true;
}

///
void cUINode::SetColor( float r, float g, float b, float a )
{
	SetColor( NiColorA(r,g,b,a) );
}

///
void cUINode::SetColor( const NiColorA& color )
{
	if( mpImage )
	{
		mpImage->SetColor( color );
	}
}

/// 주의사항 : 
void cUINode::SetAlpha( float a )
{
	mAlpha = a;

	if( mpImage )
	{
		mpImage->SetAlpha( mAlpha );
	}
}

/// 주의사항 : 
void cUINode::UpdateRect()
{
	/// 상대 영역좌표가 부모를 넘지 않게 설정한다 (이상한 값이 못오게 함)
	unsigned int rWidth = mRelativeRect.GetWidth();
	unsigned int rHeight = mRelativeRect.GetHeight();

	/// 화면 사이즈 못나가게 보정
	if( mRelativeRect.mLeft < 0 )
	{
		mRelativeRect.mLeft = 0;
		mRelativeRect.mRight = mRelativeRect.mLeft + rWidth;
	}
	if( mRelativeRect.mRight > (int)GetScreenWidth() )
	{
		mRelativeRect.mRight = GetScreenWidth();
		mRelativeRect.mLeft = mRelativeRect.mRight - rWidth;

		if( mRelativeRect.mLeft < 0 )
			mRelativeRect.mLeft = 0;
	}
	if( mRelativeRect.mTop < 0 )
	{
		mRelativeRect.mTop = 0;
		mRelativeRect.mBottom = mRelativeRect.mTop + rHeight;
	}
	if( mRelativeRect.mBottom > (int)GetScreenHeight() )
	{
		mRelativeRect.mBottom = GetScreenHeight();
		mRelativeRect.mTop = mRelativeRect.mBottom - rHeight;	

		if( mRelativeRect.mTop < 0 )
			mRelativeRect.mTop = 0;
	}

	cUIRect& parentAbsolute = mpParent->mAbsoluteRect;
	mAbsoluteRect = mRelativeRect + parentAbsolute.mUpperLeft;

	/// 절대 영역좌표가 전체 화면 을 넘지 않도록 설정
	unsigned int abWidth = mAbsoluteRect.GetWidth();
	unsigned int abHeight = mAbsoluteRect.GetHeight();

	/// 컨테이너인 경우에는
	if( mMove && IsKindof( eUINODE_CONTAINER ) )
	{
		/// 백그라운드용 컨테이너면 그냥 리턴 
		cUIContainer* pCon = (cUIContainer*)this;
		if( pCon->IsOnlyBackground() == true )
			return;

		if( this->mpParent == GetRoot() )
		{
			mAbsoluteRect = mRelativeRect;

			/// UI 자석기능
			ApplySnap();

			if( ::abs(mAbsoluteRect.mLeft) <= (AUTO_MAGNATIC_DIST / 2) )
			{
				mAbsoluteRect.mLeft = 0;
				mAbsoluteRect.mRight = mAbsoluteRect.mLeft + abWidth;
			}
			if( GetScreenWidth() - mAbsoluteRect.mRight <= (AUTO_MAGNATIC_DIST / 2) )
			{
				mAbsoluteRect.mRight = GetScreenWidth();
				mAbsoluteRect.mLeft = mAbsoluteRect.mRight - abWidth;
			}
			if( ::abs(mAbsoluteRect.mTop) <= (AUTO_MAGNATIC_DIST / 2) )
			{
				mAbsoluteRect.mTop = 0;
				mAbsoluteRect.mBottom = mAbsoluteRect.mTop + abHeight;
			}
			if( GetScreenHeight() - mAbsoluteRect.mBottom <= (AUTO_MAGNATIC_DIST / 2) )
			{
				mAbsoluteRect.mBottom = GetScreenHeight();
				mAbsoluteRect.mTop = mAbsoluteRect.mBottom - abHeight;
			}
		}

		/// 루트에 대한 위치이므로, 동일하게 해줌.
		//mRelativeRect = mAbsoluteRect;
		mMove = false;
	}

	/// 이미지 영역을 갱신
	if( mpImage )
	{
		mpImage->SetScreenRect( mAbsoluteRect );
	}
}

/// 주의사항 :UI 컨테이너가 이동시에 자석기능 
void cUINode::ApplySnap()
{
	unsigned int abWidth = mAbsoluteRect.GetWidth();
	unsigned int abHeight = mAbsoluteRect.GetHeight();

	/// 현재 이동한 윈도우가 붙어야할 윈도우를 찾는다.(x축검사)
	cUIContainer* interConX = UIMAN->UpdateNearContainer( (cUIContainer*)this, true );
	if( interConX )
	{
		/// 상대 컨테이너의 오른쪽에 붙어라 
		if( mSnapType == eSNAP_RIGHT )
		{
			mAbsoluteRect.mLeft = interConX->GetAbsoluteRect().mRight;
			mAbsoluteRect.mRight = mAbsoluteRect.mLeft + abWidth;
		}
		/// 상대 컨테이너의 왼쪽에 붙어라 
		if( mSnapType == eSNAP_LEFT )
		{	
			mAbsoluteRect.mRight = interConX->GetAbsoluteRect().mLeft;
			mAbsoluteRect.mLeft = mAbsoluteRect.mRight - abWidth;
		}
		/// 상대컨테이너와 LEFT 값이 같으면 확 붙여라 ( LEFT 정렬)
		if( mSnapType == eSNAP_LEFT_OUT )
		{
			mAbsoluteRect.mLeft = interConX->GetAbsoluteRect().mLeft;
			mAbsoluteRect.mRight = mAbsoluteRect.mLeft + abWidth;
		}
		/// 상대컨테이너와 RIGHT 값이 같으면 확 붙여라 ( RIGHT 정렬)
		if( mSnapType == eSNAP_RIGHT_OUT )
		{
			mAbsoluteRect.mRight = interConX->GetAbsoluteRect().mRight;
			mAbsoluteRect.mLeft = mAbsoluteRect.mRight - abWidth;
		}
	}

	/// y 축검사
	cUIContainer* interConY = UIMAN->UpdateNearContainer( (cUIContainer*)this, false );
	if( interConY )
	{
		/// 상대 컨테이너의 아랫쪽에 붙여라
		if( mSnapType == eSNAP_BOTTOM )
		{
			mAbsoluteRect.mTop = interConY->GetAbsoluteRect().mBottom;
			mAbsoluteRect.mBottom = mAbsoluteRect.mTop + abHeight;
		}
		/// 상대 컨테이너의 위쪽에 붙여라 
		if( mSnapType == eSNAP_TOP )
		{
			mAbsoluteRect.mBottom = interConY->GetAbsoluteRect().mTop;
			mAbsoluteRect.mTop = mAbsoluteRect.mBottom - abHeight;
		}
		/// 상대컨테이너와 TOP 값이 같으면 확 붙여라 ( TOP 정렬)
		if( mSnapType == eSNAP_TOP_OUT )
		{
			mAbsoluteRect.mTop = interConY->GetAbsoluteRect().mTop;
			mAbsoluteRect.mBottom = mAbsoluteRect.mTop + abHeight;
		}
		/// 상대컨테이너와 BOTTOM 값이 같으면 확 붙여라 ( BOTTOM 정렬)
		if( mSnapType == eSNAP_BOTTOM_OUT )
		{
			mAbsoluteRect.mBottom = interConY->GetAbsoluteRect().mBottom;
			mAbsoluteRect.mTop = mAbsoluteRect.mBottom - abHeight; 
		}
	}
}

bool cUINode::ContainPoint( const cUIPos& pos ) const
{
	if( mVisible == false )
		return false;

	return mAbsoluteRect.ContainPoint( pos );
}

/// 주의사항 : 
void cUINode::Move( const cUIPos& value )
{
	mRelativeRect += value;
	mMove = true;
	UpdateRect();
}

void cUINode::Show()
{
	mVisible = true;
}

void cUINode::Hide()
{
	ReleaseCapture();

	mVisible = false;
}

/// 주의사항 : 자기자신의 눌림여부 설정 
void cUINode::SetPress( bool press )
{
	if( mEnabled == false )
		return;

	if( mPressed != press )
		mPressed = press;
}

cUINode* cUINode::GetFocus()
{
	return UIMAN->GetFocusedNode();
}

cUINode* cUINode::GetHover()
{
	return UIMAN->GetHoverNode();
}

void cUINode::CheckEventNode()
{
	UIMAN->CheckEventNode( this );
}

void cUINode::SetCapture()
{
	UIMAN->SetCaptureNode( this );
}

void cUINode::ReleaseCapture()
{
	UIMAN->ReleaseCaptureNode( this );
}

void cUINode::ReleaseHover()
{
	UIMAN->ReleaseHoverNode( this );
}

void cUINode::SetFocus()
{
	UIMAN->SetFocusedNode( this );
}

void cUINode::ReleaseFocus()
{
	UIMAN->ReleaseFocusedNode( this );
}

void cUINode::ReleaseEventNode()
{
	UIMAN->ReleaseEventNode();
}

cUIRootNode* cUINode::GetRoot()
{
	return UIMAN->GetRootNode();
}

cUINode* cUINode::GetCapture()
{
	return UIMAN->GetCaptureNode();
}

unsigned int cUINode::GetScreenWidth()
{
	return RENDERSYS->GetScreenWidth();
}

unsigned int cUINode::GetScreenHeight()
{
	return RENDERSYS->GetScreenHeight();
}

bool cUINode::SendEvent( cUINode* eventHandler, eUIEventType eventType, cUINode* caller, unsigned int id, bool shift )
{
	assert( eventHandler );
	assert( caller );

	cUIEvent event;
	event.mType = eventType;
	event.mpCaller = caller;
	event.mID = id;
	event.mShift = shift;
	
	return eventHandler->HandleEvent( event );
}

/// 주의사항 : 
bool cUINode::HandleEvent( const cUIEvent& event )
{
	/// 비활성화된 경우에는 부모에 이벤트를 넘김 LSG070228
	if( mEnabled == false || mVisible == false )
	{
		return mpParent->HandleEvent( event );
	}

	switch( event.mType )
	{
	case eUIEVENT_CAPTURE_LOST:
		OnCaptureLost();
		break;

	case eUIEVENT_FOCUS_LOST:
		OnFocusLost();
		break;

	case eUIEVENT_MOUSE_MOVE:
		OnMouseMove( event.mPos );
		break;

	case eUIEVENT_MOUSE_WHEEL:
		OnMouseWheel( event.mPos, event.mWheel );
		break;

	case eUIEVENT_LBUTTON_DOWN:
		OnLButtonDown( event.mPos, event.mControl, event.mAlt, event.mShift );
		break;

	case eUIEVENT_RBUTTON_DOWN:
		OnRButtonDown( event.mPos, event.mControl, event.mAlt, event.mShift  );
		break;

	case eUIEVENT_LBUTTON_UP:
		OnLButtonUp( event.mPos );
		break;

	case eUIEVENT_RBUTTON_UP:
		OnRButtonUp( event.mPos );
		break;

	case eUIEVENT_LBUTTON_DBLCLK:
		OnLButtonDoubleClick( event.mPos );
		break;

	case eUIEVENT_MOUSE_HOVERED:
		OnMouseHovered( event.mPos );
		break;

	case eUIEVENT_MOUSE_LEFT:
		OnMouseLeft( event.mPos );
		break;

	case eUIEVENT_KEY_DOWN:
		OnKeyDown( event.mCode );
		break;

	case eUIEVENT_KEY_UP:
		OnKeyUp( event.mCode );
		break;

	case eUIEVENT_CHAR:
		OnChar( event.mWparam );
		break;

	case eUIEVENT_IME_NOTIFY:
		OnImeNotify( event.mWparam );
		break;

	case eUIEVENT_IME_STARTCOMPOSITION:
		OnImeStartComposition( event.mWparam, event.mLparam );
		break;

	case eUIEVENT_IME_ENDCOMPOSITION:
		OnImeEndComposition( event.mWparam, event.mLparam );
		break;

	case eUIEVENT_IME_COMPOSITION:
		OnImeComposition( event.mWparam, event.mLparam );
		break;

	default:
		return false;
	}
	return true;
}

void cUINode::OnLButtonDown( const cUIPos& /*pos*/, bool /*ctrl*/, bool /*alt*/, bool /*shift*/ )
{
	UIMAN->GotoFrontNode( this );
}

void cUINode::OnMouseHovered( const cUIPos& pos )
{
	if( mHoverTipIndex > 0 )
		UIMAN->ShowUITip( pos, mHoverTipIndex );
}

void cUINode::OnMouseLeft( const cUIPos& )
{
	if( mHoverTipIndex > 0 )
		UIMAN->HideTip();
}
