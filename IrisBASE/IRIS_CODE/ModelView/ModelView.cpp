// CModelView.cpp : CModelView 클래스의 구현
//
#include "stdafx.h"
#include "ModelApp.h"
#include "ModelView.h"

#include "Engine/Application.h"
#include "Engine/RenderSystem.h"

#include "CharacterForm.h"
#include "MonsterForm.h"
#include "NpcForm.h"

#include "CameraManager.h"
#include "LookAtCamera.h"

#include "SceneManager.h"
#include "PlayerSceneNode.h"
#include "MonsterSceneNode.h"
#include "NpcSceneNode.h"
#include "DramaturgyManager.h"
#include "ResourceManager.h"

#include "InputSystem.h"
#include "MouseAgent.h"
#include "KeyAgent.h"

#include ".\modelview.h"

// CModelView

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CModelView, CView)

BEGIN_MESSAGE_MAP(CModelView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_CONTEXTMENU()
//	ON_WM_SETFOCUS()
ON_WM_KEYDOWN()
END_MESSAGE_MAP()

#ifdef _DEBUG
void CModelView::AssertValid() const
{
	CView::AssertValid();
}

void CModelView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

CModelView* CModelView::mSingleton = 0;

CModelView::CModelView()
: mApplication(0)
, mLookAtCamera(0)
, mInit(0)
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mCurStage = eSTAGE_CHARACTER;
}

CModelView::~CModelView()
{
	/// 어플리케이션을 제거
	delete mApplication;

	/// 엔진 종료
	NiShutdown();

}

// CModelView 메시지 처리기

BOOL CModelView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

int CModelView::OnCreate( LPCREATESTRUCT cs )
{
	if( CView::OnCreate(cs) == -1 )
		return -1;

	/// 엔진 초기화
	NiInit();

	/// 어플리케이션을 생성
	mApplication = new cApplication;


	/// 어플리케이션을 초기화
	if( mApplication->Init( GetSafeHwnd() ) == false )
	{
		assert( 0 && "failed to init application" );
		return -1;
	}

	/// 타이머 설정
	SetTimer( 1, 20, 0 );

	mLookAtCamera = CAMERAMAN->CreateLookAt( true );

	return 0;
}

void CModelView::OnDestroy()
{
	/// 어플리케이션 종료
	mApplication->Exit();

	CView::OnDestroy();
}

BOOL CModelView::OnEraseBkgnd( CDC* ) 
{
	return FALSE;
}

void CModelView::OnSize( UINT type, int cx, int cy )
{
	CView::OnSize(type, cx, cy);
	Resize( cx, cy );
}

void CModelView::OnDraw( CDC* )
{
	mApplication->Process();

	/// 렌더링
	if( RENDERSYS )
	{
		RENDERSYS->BeginFrame();
		{
			SCENEMAN->Render();

//			if( mSettingViewDialog->IsHUDChecked() )
//				DEVSYS->Render();
		}
		RENDERSYS->EndFrame();
		RENDERSYS->DisplayFrame();
	}
}

void CModelView::OnMouseMove( UINT /*flags*/, CPoint point )
{
	MOUSE->SetPosition( point.x, point.y );
}

BOOL CModelView::OnMouseWheel( UINT /*flags*/, short zdelta, CPoint /*point*/ )
{
	MOUSE->SetDeltaWheel( (float)zdelta * 30 );
	return TRUE;
}

void CModelView::OnLButtonDown( UINT /*flags*/, CPoint point )
{
	MOUSE->SetLButtonDown( point.x, point.y );

	if( KEY->IsDown(KEY_X) || KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_Z) || KEY->IsDown(KEY_C) )
		return;

	if( SCENEMAN->IsInited() == false )
		return;
}

void CModelView::OnLButtonUp( UINT /*flags*/, CPoint point )
{
	MOUSE->SetLButtonUp( point.x, point.y );
}

void CModelView::OnRButtonDown( UINT flags, CPoint point )
{
	if( GetCapture() != this )
	{
		SetCapture();
	}

	bool ctrl = (flags & MK_CONTROL) != 0;
	if( ctrl == false )
		MOUSE->SetRButtonDown( point.x, point.y );
	else
		CView::OnRButtonDown( flags, point );
}

void CModelView::OnRButtonUp( UINT flags, CPoint point )
{
	if( GetCapture() == this )
	{
		ReleaseCapture();
	}

	bool ctrl = (flags & MK_CONTROL) != 0;
	if( ctrl == false )
		MOUSE->SetRButtonUp( point.x, point.y );
	else
		CView::OnRButtonUp( flags, point );
}

void CModelView::OnMButtonDown( UINT /*flags*/, CPoint point )
{
	MOUSE->SetMButtonDown( point.x, point.y );
}

void CModelView::OnMButtonUp( UINT /*flags*/, CPoint point )
{
	MOUSE->SetMButtonUp( point.x, point.y );
}

void CModelView::Resize( unsigned int width, unsigned int height )
{
	if( RENDERSYS )
	{
		if( mInit )
		{
			RENDERSYS->SetScreenSize( width, height );		
			RENDERSYS->Reset();

			//
			mLookAtCamera->SetViewFrustum( width, height );
			mLookAtCamera->Update( 0.0f );
		}
	}
}

void CModelView::Init()
{
	mInit = true;

	CHARACTERFORM->Init();
	MONSTERFORM->Init();
	NPCFORM->Init();

	CreateCharacter( 0, 0, 0 );

	RECT rt;
	GetClientRect( &rt );
	Resize( rt.right - rt.left, rt.bottom - rt.top );
}

void CModelView::ChangeStage( unsigned char stage )
{
	if( mInit == false )
		return;

	if( stage == eSTAGE_CHARACTER )
	{
		CHARACTERFORM->Init();
		CreateCharacter( 0, 0, 0 );
	}
	else if( stage == eSTAGE_MONSTER )
	{
		MONSTERFORM->Init();
		CreateMonster( MONSTERFORM->GetCurrentMonsterIdx(), MONSTERFORM->GetBaseModelIndex() );
	}
	else if( stage == eSTAGE_NPC )
	{
		NPCFORM->Init();
		CreateNpc( NPCFORM->GetBaseModelIndex() );
	}

	mCurStage = stage;
}


void CModelView::Update()
{
	Invalidate( FALSE );
	UpdateWindow();
}


void CModelView::CreateCharacter( unsigned long modelIndex, unsigned char race, unsigned char gender )
{
	cPlayerSceneNode* node = SCENEMAN->CreatePlayer( modelIndex );
	if( node == 0 )
	{
		NiMessageBox( "Failed to Create player", "Error" );
		return;
	}

	float rd = node->GetRadius();
	mLookAtCamera->SetMinCameraDist( 3.419296f * rd );
	mLookAtCamera->SetLookAt( node->GetObjectCenter() );

	CHARACTERFORM->UpdateAnimationList( race, gender );
	node->SetTargetAnimation( 0 );

	SCENEMAN->SetDramaProcess( false );
}

void CModelView::CreateMonster( unsigned long monIdx, unsigned long modelIndex )
{
	cMonsterSceneNode* node = SCENEMAN->CreateMonster( monIdx, modelIndex );
	if( node == 0 )
	{
		AfxMessageBox( "Failed to Create Monster" );
		return;
	}

	float rd = node->GetRadius();
	mLookAtCamera->SetMinCameraDist( 3.419296f * rd );
	mLookAtCamera->SetLookAt( node->GetObjectCenter() );

	unsigned int seq = MONSTERFORM->UpdateAnimationList( modelIndex );
//	node->SetBaseAnimationIndex( seq );
	node->SetTargetAnimation( seq );

	SCENEMAN->SetDramaProcess( false );
}

void CModelView::CreateNpc( unsigned long modelIndex )
{
	cNpcSceneNode* node = SCENEMAN->CreateNPC( modelIndex );
	if( node == 0 )
	{
		AfxMessageBox( "Failed to Create Monster" );
		return;
	}

	float rd = node->GetRadius();
	mLookAtCamera->SetMinCameraDist( 3.419296f * rd );
	mLookAtCamera->SetLookAt( node->GetObjectCenter() );

	unsigned int seq = NPCFORM->UpdateAnimationList( modelIndex );
//	node->SetBaseAnimationIndex( seq );
	node->SetTargetAnimation( seq );
}

void CModelView::ChangeAnimation( unsigned int seq )
{
	cDynamicSceneNode* node = SCENEMAN->GetCurrentSceneNode();
	if( node == 0 )
	{
		assert(0);
		return;
	}

	node->UpdateTargetAnimation( seq );

	NiTransform trans;
	trans.MakeIdentity();
}

bool CModelView::ChangeCharacterParts( unsigned long partIdx, const char* filename, bool hideEar )
{
	cDynamicSceneNode* node = SCENEMAN->GetCurrentSceneNode();
	if( node->GetType() != cSceneNode::ePLAYER )
	{
		assert(0);
		return false;
	}
	cPlayerSceneNode* player = (cPlayerSceneNode*)node;
	player->HideEar( hideEar );

	return player->ChangeParts( partIdx, filename );
}

bool CModelView::ChangeCharacterPartsTexture( unsigned long partIdx, const char* filename )
{
	cDynamicSceneNode* node = SCENEMAN->GetCurrentSceneNode();
	if( node->GetType() != cSceneNode::ePLAYER )
	{
		assert(0);
		return false;
	}
	cPlayerSceneNode* player = (cPlayerSceneNode*)node;

	return player->ChangePartsTexture( partIdx, filename );
}


unsigned int CModelView::LinkCharacterWeapon( unsigned long linkIdx, const char* filename, float scale )
{
	cDynamicSceneNode* node = SCENEMAN->GetCurrentSceneNode();
	if( node->GetType() != cSceneNode::ePLAYER )
	{
		assert(0);
		return UINT_MAX;
	}
	cPlayerSceneNode* player = (cPlayerSceneNode*)node;
	unsigned int index = player->LinkObject( linkIdx, filename );

	NiAVObject* obj = player->GetLinkObject( index );
	if( obj )
		obj->SetScale( scale );

	return index;
}

void CModelView::UnLinkCharacterWeapon( unsigned int left, unsigned int right )
{
	cDynamicSceneNode* node = SCENEMAN->GetCurrentSceneNode();
	if( !node && node->GetType() != cSceneNode::ePLAYER )
	{
		assert(0);
		return;
	}
	cPlayerSceneNode* player = (cPlayerSceneNode*)node;

	if( left != (unsigned int)-1 )
		player->UnLinkObject( left );

	if( right != (unsigned int)-1 )
		player->UnLinkObject( right );
}

unsigned int CModelView::LinkCharacterCostume( unsigned long linkIdx, const char* filename )
{
	cDynamicSceneNode* node = SCENEMAN->GetCurrentSceneNode();
	if( node->GetType() != cSceneNode::ePLAYER )
	{
		assert(0);
		return UINT_MAX;
	}

	cPlayerSceneNode* player = (cPlayerSceneNode*)node;
	return player->LinkObject( linkIdx, filename );
}

void CModelView::UnLinkCharacterCostume( unsigned int costume1, unsigned int costume2 )
{
	cDynamicSceneNode* node = SCENEMAN->GetCurrentSceneNode();
	if( !node && node->GetType() != cSceneNode::ePLAYER )
	{
		assert(0);
		return;
	}
	cPlayerSceneNode* player = (cPlayerSceneNode*)node;

	if( costume1 != (unsigned int)-1 )
		player->UnLinkObject( costume1 );

	if( costume2 != (unsigned int)-1 )
		player->UnLinkObject( costume2 );
}


void CModelView::OnContextMenu( CWnd* /*pWnd*/, CPoint pos )
{
	//TRACE("CRollupCtrl::OnContextMenu\n");

	if( m_cmenuCtxt.m_hMenu)		m_cmenuCtxt.DestroyMenu();

	if( m_cmenuCtxt.CreatePopupMenu() )
	{
		GetCursorPos(&pos);	//Cursor position even with keyboard 'Context key'

		unsigned int count = SCENEMAN->GetLightCount();
		unsigned int currentIdx = SCENEMAN->GetCurrentLightIndex();
		for( unsigned int i = 0;i<count; i++ )
		{
			char str[64] = {0,};
			::sprintf_s( str, "Light%2d", i );

			m_cmenuCtxt.AppendMenu(MF_STRING, 100+i, str	);

			if( i == currentIdx )
				m_cmenuCtxt.CheckMenuItem(100+i, MF_CHECKED);
		}

		m_cmenuCtxt.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pos.x, pos.y, this);
	}
}

BOOL CModelView::OnCommand(WPARAM wparam, LPARAM lparam)
{
	unsigned int idx = LOWORD(wparam) - 100;
	SCENEMAN->ChangeLight( idx );
	return CWnd::OnCommand(wparam, lparam);
}

void CModelView::ChangeBackColor( NiColor cr )
{
	RENDERSYS->SetClearColor( cr );
}
/*
void CModelView::OnSetFocus(CWnd* pOldWnd)
{
	if( pOldWnd )
		pOldWnd->SetFocus();
}
*/

void CModelView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if( nChar == VK_F5 )
	{
		if( SCENEMAN->IsInited() == false )
			return;

		DRAMATURGYMAN->ReLoadDramaturgyFileList();
	}
	else if( nChar == VK_F6 )
	{
		NiMessageBox( "Reload Effect Resource??", "Notify" );
		RESOURCEMAN->Close();
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
