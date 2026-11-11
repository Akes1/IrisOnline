#include "stdafx.h"
#include "RegenToolView.h"

#include "Resource.h"
#include "RegenToolApp.h"
#include "MapDialog.h"
#include "NpcDialog.h"
#include "MonsterDialog.h"
#include "PortalDialog.h"
#include "GatheringDialog.h"
#include "PatternDialog.h"
#include "DoorDialog.h"
#include "Lines.h"

#include "Application.h"
#include "AppTimer.h"
#include "RenderSystem.h"
#include "InputSystem.h"
#include "MouseAgent.h"
#include "KeyAgent.h"
#include "ResourceManager.h"
#include "WorldManager.h"
#include "SceneManager.h"
#include "CameraManager.h"
#include "FreeCamera.h"
#include "Terrain.h"
#include "NaviMesh.h"
#include "NaviField.h"
#include "Box.h"
#include "PathFinder.h"

#include "NpcSceneNode.h"
#include "MonsterSceneNode.h"
#include "GatheringSceneNode.h"
#include "PortalSceneNode.h"
#include "../mapeditor/PatternSceneNode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(cRegenToolView, CView)

BEGIN_MESSAGE_MAP(cRegenToolView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

#ifdef _DEBUG
void cRegenToolView::AssertValid() const
{
	CView::AssertValid();
}

void cRegenToolView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

cRegenToolView* cRegenToolView::mSingleton = 0;

cRegenToolView::cRegenToolView()
: mApplication( 0 )
, mMapDialog( 0 )
, mNpcDialog( 0 )
, mMonsterDialog( 0 )
, mPortalDialog( 0 )
, mGatheringDialog( 0 )
, mPatternDialog( 0 )
, mDoorDialog(0)
, mCamera( 0 )
, mNaviMesh( 0 )
, mNaviField( 0 )
, mMapIndex( 0 )
, mMapDegree( 0 )
, mDummyNode( 0 )
, mSelNode( 0 )
, mSelLines( 0 )
, mSelPattern( 0 )
, mGroupLines( 0 )
, mSelMonsterGroup( 0 )
, mSelGatheringGroup( 0 )
, mPathFinder( 0 )
, mErrorModelFile( "Error!" )
, mErrorNpcName( "Error!" )
, mErrorMonsterName( "Error!" )
, mErrorGatheringName( "Error!" )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mPickedArray.Reserve( 1024 );

	/// 길찾기를 생성
	mPathFinder = new cPathFinder( 4096 );
}

cRegenToolView::~cRegenToolView()
{
	Clear();

	{
		cMapInfoMap::cIterator i = mMapInfoMap.Begin();
		cMapInfoMap::cIterator iend = mMapInfoMap.End();
		for( ; i != iend; ++i )
		{
			delete i->mSecond;
		}
		mMapInfoMap.Clear();
	}

	///
	{
		cNpcInfoMap::cIterator i = mNpcInfoMap.Begin();
		cNpcInfoMap::cIterator iend = mNpcInfoMap.End();

		for( ; i != iend; ++i )
		{
			delete i->mSecond;
		}
		mNpcInfoMap.Clear();
	}

	///
	{
		cMonsterInfoMap::cIterator i = mMonsterInfoMap.Begin();
		cMonsterInfoMap::cIterator iend = mMonsterInfoMap.End();

		for( ; i != iend; ++i )
		{
			delete i->mSecond;
		}
		mMonsterInfoMap.Clear();
	}
	///
	{
		cGatheringInfoMap::cIterator i = mGatheringInfoMap.Begin();
		cGatheringInfoMap::cIterator iend = mGatheringInfoMap.End();

		for( ; i != iend; ++i )
		{
			delete i->mSecond;
		}
		mGatheringInfoMap.Clear();
	}

	///
	delete mPathFinder;
	delete mGroupLines;
	delete mSelLines;
	delete mNaviMesh;
	delete mNaviField;

	/// 어플리케이션을 제거
	delete mApplication;

//	/// 엔진 종료
//	NiShutdown();

	mSingleton = 0;
}

void cRegenToolView::Clear()
{
	mMapIndex = 0;
	mMapDegree = 0;
	mSelMonsterGroup = 0;
	mSelGatheringGroup = 0;
	mSelNode = 0;
	mSelPattern = 0;
	mDummyNode = 0;
	mNpcNodeMap.Clear();
	mNpcListIdx = 0;

	{
		cPatternMap::cIterator i = mPatternMap.Begin();
		cPatternMap::cIterator iend = mPatternMap.End();

		for( ; i != iend; ++i )
		{
			delete i->mSecond;
		}
		mPatternMap.Clear();
	}

	///
	{
		cMonsterGroupMap::cIterator i = mMonsterGroupMap.Begin();
		cMonsterGroupMap::cIterator iend = mMonsterGroupMap.End();

		for( ; i != iend; ++i )
		{
			delete i->mSecond;
		}
		mMonsterGroupMap.Clear();
	}

	///
	{
		cGatheringGroupMap::cIterator i = mGatheringGroupMap.Begin();
		cGatheringGroupMap::cIterator iend = mGatheringGroupMap.End();

		for( ; i != iend; ++i )
		{
			delete i->mSecond;
		}
		mGatheringGroupMap.Clear();
	}

	///
	{
		cPortalInfoMap::cIterator i = mPortalInfoMap.Begin();
		cPortalInfoMap::cIterator iend = mPortalInfoMap.End();

		for( ; i != iend; ++i )
		{
			delete i->mSecond;
		}
		mPortalInfoMap.Clear();
	}

	///
	{
		cDoorInfoMap::cIterator i = mDoorInfoMap.Begin();
		cDoorInfoMap::cIterator iend = mDoorInfoMap.End();

		for( ; i != iend; ++i )
		{
			delete i->mSecond;
		}
		mDoorInfoMap.Clear();
	}

	///
	mPickedArray.Clear();

	///
	if( mNaviMesh )
		mNaviMesh->Clear();
	if( mNaviField )
		mNaviField->Clear();
	if( mApplication )
		mApplication->Clear();
}

void cRegenToolView::Init()
{
	/// 네비메시를 생성
	mNaviMesh = new cNaviMesh;

	/// 네비필드를 생성
	mNaviField = new cNaviField;

	/// 편집 원을 생성
	mSelLines = new cCircleLines( NiColorA(1.0f, 0.0f, 0.0f) );
	mGroupLines = new cCircleLines( NiColorA(0.0f, 1.0f, 0.0f) );

	///
	LoadMapInfo();
	LoadMapNameList();
	LoadModelFileList();
	LoadNpcNameList();
	LoadNpcList();
	LoadMonsterNameList();
	LoadMonsterList();
	LoadGatheringNameList();
	LoadGatheringList();

	///
	assert( mMapDialog );
	assert( mNpcDialog );
	assert( mMonsterDialog );
	assert( mPortalDialog );
	assert( mGatheringDialog );
	assert( mPatternDialog );
	//assert( mPostDialog );
	assert( mDoorDialog );

	if( mMapDialog )
		mMapDialog->SetMapInfo( &mMapInfoMap );

	mNpcDialog->EnableWindow( 0 );
	mMonsterDialog->EnableWindow( 0 );
	mPortalDialog->EnableWindow( 0 );
	mGatheringDialog->EnableWindow( 0 );
	mPatternDialog->EnableWindow( 0 );
	mDoorDialog->EnableWindow( 0 );

	mPathFinder->Init( mNaviField );

	mNpcListIdx = 0;
}

void cRegenToolView::Update()
{
	Invalidate( FALSE );
	UpdateWindow();
}

void cRegenToolView::Resize( unsigned int width, unsigned int height )
{
	if( RENDERSYS )
	{
		RENDERSYS->SetScreenSize( width, height );		
		RENDERSYS->Reset();

		/// 카메라 설정
		mCamera->SetViewFrustum( width, height );
		mCamera->Update( 0.0f );
	}
}

void cRegenToolView::SetCamFarDistance( float farDist )
{
	mCamera->SetFarDistance( farDist );

	Update();
}

float cRegenToolView::GetCamFarDistance() const
{
	return mCamera->GetFarDistance();
}

void cRegenToolView::ZoomPersp()
{
	if( TERRAIN->IsInited() == false )
		return;

	NiPoint3 min = TERRAIN->GetBoundBox().GetMin();
	NiPoint3 max = TERRAIN->GetBoundBox().GetMax();
	NiPoint3 center = (min + max) * 0.5f;
	float radius = center.x - min.x;
	float fovyRadian = 25.0f * NI_PI / 180.0f;
	float dist = (radius / mCamera->GetAspectRatio()) / ::tanf(fovyRadian);
	NiPoint3 pos = center;
	pos.y -= dist;
	pos.z += (dist * 0.5f);

	mCamera->SetOrtho( false );
	mCamera->SetFarDistance( mCamera->GetFarDistance() );
	mCamera->SetTranslate( pos );
	mCamera->LookAt( center );

	Update();
}

void cRegenToolView::ZoomOrtho()
{
	if( TERRAIN->IsInited() == false )
		return;

	NiPoint3 min = TERRAIN->GetBoundBox().GetMin();
	NiPoint3 max = TERRAIN->GetBoundBox().GetMax();
	NiPoint3 center = (min + max) * 0.5f;
	NiPoint3 pos = center;
	pos.z += 100000.0f;
	float radius = center.x - min.x;

	mCamera->SetOrtho( true );
	mCamera->SetFarDistance( 120000.0f );
	mCamera->SetViewFrustum( -radius, radius, radius, -radius, mCamera->GetNearDistance(), mCamera->GetFarDistance() );
	mCamera->SetTranslate( pos );
	mCamera->LookAt( center );

	Update();
}

void cRegenToolView::SelectNode( cSceneNode* node, bool updateView )
{
	if( node == mDummyNode )
		return;

	if( node && node->IsViewNode() == false )
		return;

	mNpcDialog->SetEnabled( false );
	mMonsterDialog->SetEnabled( false );
	mPortalDialog->SetEnabled( false );
	mGatheringDialog->SetEnabled( false );
	mPatternDialog->SetEnabled( false );
	mDoorDialog->SetEnabled( false );

	mSelNode = node;

	/// 뷰를 갱신
	if( updateView )
		Update();

	///
	if( mSelNode )
	{
		switch( mSelNode->GetType() )
		{
		case SCENENODE_NPC:
			{
				mNpcDialog->SelectNode( (cNpcSceneNode*)mSelNode );
			}
			break;
		case SCENENODE_MONSTER:
			{
				cMonsterGroup* group = (cMonsterGroup*)mSelNode->GetUserData2();

				if( mSelMonsterGroup != group )
				{
					mSelMonsterGroup = group;

					mMonsterDialog->UpdateGroupList( mMonsterGroupMap );
					mMonsterDialog->SelectGroup( group );
				}
				mMonsterDialog->SelectNode( (cMonsterSceneNode*)mSelNode );

				mSelGatheringGroup = 0;
			}
			break;
		case SCENENODE_PORTAL:
			{
				mPortalDialog->SelectNode( (cPortalSceneNode*)mSelNode );
			}
			break;
		case SCENENODE_GATHERING:
			{
				cGatheringGroup* group = (cGatheringGroup*)mSelNode->GetUserData2();

				if( mSelGatheringGroup != group )
				{
					mSelGatheringGroup = group;

					mGatheringDialog->UpdateGroupList( mGatheringGroupMap );
					mGatheringDialog->SelectGroup( group );
				}
				mGatheringDialog->SelectNode( (cGatheringSceneNode*)mSelNode );

				mSelMonsterGroup = 0;
			}
			break;
		case SCENENODE_PATTERN:
			{	
				unsigned int patternIdx = ((cPatternSceneNode*)mSelNode)->GetUserData0();

				cPatternMap::cIterator i = mPatternMap.Find( patternIdx );
				if( i != mPatternMap.End() )
				{
					mSelPattern = (cPattern*)(i->mSecond);
				}

				/// 해당 패턴 자동선택
				mPatternDialog->SelectPattern( mSelPattern );
				mPatternDialog->UpdatePosList( &mSelPattern->mSceneNodeList );
				mPatternDialog->SelectPatternPos( (cPatternSceneNode*)mSelNode );
			}
			break;
		case SCENENODE_DOOR:
			{
				mDoorDialog->SelectNode( (cDoorSceneNode*)mSelNode );
			}
			break;
		default:
			return;
		}
	}
}

void cRegenToolView::DeleteSel()
{
	if( mSelNode == 0 )
		return;
	if( SCENEMAN->IsInited() == false )
		return;

	switch( mSelNode->GetType() )
	{
	case SCENENODE_NPC:
		{
			/// 노드 맵에서 제거
			mNpcNodeMap.Erase( mSelNode->GetUserData0() );

			/// 다이얼로그를 갱신
			mNpcDialog->UpdateList( mNpcNodeMap );

			/// 장면 관리자에서 제거
			SCENEMAN->DestroyNode( mSelNode );

			/// 선택을 해제
			SelectNode( 0 );
		}
		break;
	case SCENENODE_MONSTER:
		{
			if( mSelMonsterGroup == 0 )
				break;

			/// 그룹 리스트에서 제거
			mSelMonsterGroup->RemoveNode( (cMonsterSceneNode*)mSelNode );

			/// 다이얼로그를 갱신
			const cMonsterList& monsterList = mSelMonsterGroup->mNodeList;
			mMonsterDialog->UpdateList( monsterList );

			/// 장면 관리자에서 제거
			SCENEMAN->DestroyNode( mSelNode );

			///
			if( monsterList.IsEmpty() == false )
			{
				SelectNode( *(monsterList.Begin()) );
			}
			else
			{
				/// 선택을 해제
				SelectNode( 0 );
			}
		}
		break;
	case SCENENODE_PORTAL:
		{
			unsigned int index = mSelNode->GetUserData0();

			cPortalInfoMap::cIterator i = mPortalInfoMap.Find( index );

			if( i == mPortalInfoMap.End() )
				break;

			delete i->mSecond;

			/// 정보 맵에서 제거
			mPortalInfoMap.Erase( i );

			/// 다이얼로그를 갱신
			mPortalDialog->UpdateList( mPortalInfoMap );

			/// 장면 관리자에서 제거
			SCENEMAN->DestroyNode( mSelNode );

			/// 선택을 해제
			SelectNode( 0 );
		}
		break;
	case SCENENODE_GATHERING:
		{
			if( mSelGatheringGroup == 0 )
				break;

			/// 그룹 리스트에서 제거
			mSelGatheringGroup->RemoveNode( (cGatheringSceneNode*)mSelNode );

			/// 다이얼로그를 갱신
			const cGatheringList& gatheringList = mSelGatheringGroup->mNodeList;
			mGatheringDialog->UpdateList( gatheringList );

			/// 장면 관리자에서 제거
			SCENEMAN->DestroyNode( mSelNode );

			///
			if( gatheringList.IsEmpty() == false )
			{
				SelectNode( *(gatheringList.Begin()) );
			}
			else
			{
				/// 선택을 해제
				SelectNode( 0 );
			}
		}
		break;
	case SCENENODE_PATTERN:
		{
			sPatternPos* pos = (sPatternPos*)((cPatternSceneNode*)mSelNode)->GetUserData();
			if( pos )
			{
				/// 씬노드 참조 정보 제거
				mSelPattern->RemoveNode( (cPatternSceneNode*)mSelNode );

				mPatternDialog->UpdatePosList( &mSelPattern->mSceneNodeList );

				/// 장면 관리자에서 제거
				SCENEMAN->DestroyNode( mSelNode );

				mSelPattern->UpdateAllPathLine();

				/// 선택을 해제
				if( mSelPattern->mSceneNodeList.IsEmpty() == false )
					SelectNode( *(mSelPattern->mSceneNodeList.Begin()) );
				else
					SelectNode( 0 );
			}
		}
		break;
	case SCENENODE_DOOR:
		{
			unsigned int index = mSelNode->GetUserData0();

			cDoorInfoMap::cIterator i = mDoorInfoMap.Find( index );

			if( i == mDoorInfoMap.End() )
				break;

			delete i->mSecond;

			/// 정보 맵에서 제거
			mDoorInfoMap.Erase( i );

			/// 다이얼로그를 갱신
			mDoorDialog->UpdateList( mDoorInfoMap );

			/// 장면 관리자에서 제거
			SCENEMAN->DestroyNode( mSelNode );

			/// 선택을 해제
			SelectNode( 0 );
		}
		break;
	default:
		return;
	}
}

void cRegenToolView::SetSelTranslate( NiPoint3 trans, bool updateDialog )
{
	if( mSelNode == 0 )
		return;
	if( mNaviMesh->IsInited() == false )
		return;

	mNaviMesh->CalcHeight( &trans.z, trans.x, trans.y );
	mSelNode->SetTranslate( trans );
	mSelNode->Update();

	///
	if( updateDialog )
	{
		switch( mSelNode->GetType() )
		{
		case SCENENODE_NPC:
			mNpcDialog->UpdateNodeTransform( (cNpcSceneNode*)mSelNode );
			break;
		case SCENENODE_MONSTER:
			mMonsterDialog->UpdateNodeTransform( (cMonsterSceneNode*)mSelNode );
			break;
		case SCENENODE_PORTAL:
			mPortalDialog->UpdateNodeTransform( (cPortalSceneNode*)mSelNode );
			break;
		case SCENENODE_GATHERING:
			mGatheringDialog->UpdateNodeTransform( (cGatheringSceneNode*)mSelNode );
			break;
		case SCENENODE_PATTERN:
			ModifyPatternPos( trans );
			break;
		case SCENENODE_DOOR:
			mDoorDialog->UpdateNodeTransform( (cDoorSceneNode*)mSelNode );
			break;
		default:
			return;
		}
	}
	else
	{
		Update();
	}
}

void cRegenToolView::SetSelRotate( float zangle, bool updateDialog )
{
	if( mSelNode == 0 )
		return;
	if( mSelNode->GetType() == SCENENODE_PORTAL )
	{
		cPortalInfo* info = GetPortalInfo( mSelNode->GetUserData0() );
		assert( info );info;
	}

	NiMatrix3 r;
	r.FromEulerAnglesXYZ( 0.0f, 0.0f, zangle );

	mSelNode->SetRotate( r );
	mSelNode->Update();

	///
	if( updateDialog )
	{
		switch( mSelNode->GetType() )
		{
		case SCENENODE_NPC:
			mNpcDialog->UpdateNodeTransform( (cNpcSceneNode*)mSelNode );
			break;
		case SCENENODE_MONSTER:
			mMonsterDialog->UpdateNodeTransform( (cMonsterSceneNode*)mSelNode );
			break;
		case SCENENODE_PORTAL:
			mPortalDialog->UpdateNodeTransform( (cPortalSceneNode*)mSelNode );
			break;
		case SCENENODE_GATHERING:
			mGatheringDialog->UpdateNodeTransform( (cGatheringSceneNode*)mSelNode );
			break;
		case SCENENODE_DOOR:
			mDoorDialog->UpdateNodeTransform( (cDoorSceneNode*)mSelNode );
			break;
		default:
			return;
		}
	}
	else
	{
		Update();
	}
}

void cRegenToolView::SetSelAppendZ( float appendZ, bool updateDialog )
{
	if( mSelNode == 0 )
		return;
	if( mNaviMesh->IsInited() == false )
		return;

	switch( mSelNode->GetType() )
	{
	case SCENENODE_GATHERING:
		{
			cGatheringSceneNode* node = (cGatheringSceneNode*)mSelNode;
			node->SetAppendZ( appendZ );

			NiPoint3 trans = node->GetTranslate();
			mNaviMesh->CalcHeight( &trans.z, trans.x, trans.y );
			node->SetTranslate( trans );
			node->Update();

			///
			if( updateDialog )
				mGatheringDialog->UpdateNodeTransform( node );
			else
				Update();
		}
		break;
	case SCENENODE_DOOR:
		{
			cDoorSceneNode* node = (cDoorSceneNode*)mSelNode;
			node->SetAppendZ( appendZ );

			NiPoint3 trans = node->GetTranslate();
			mNaviMesh->CalcHeight( &trans.z, trans.x, trans.y );
			node->SetTranslate( trans );
			node->Update();

			///
			if( updateDialog )
				mDoorDialog->UpdateNodeTransform( node );
			else
				Update();
		}
		break;
	default:
			return;
	}
}

void cRegenToolView::SetSelDoorInfo( float Twidth, float Bwidth, float LHeight, float RHeight )
{
	if( mSelNode == 0 )
		return;
	if( mNaviMesh->IsInited() == false )
		return;

	if( mSelNode->GetType() == SCENENODE_DOOR )
	{
		cDoorSceneNode* node = (cDoorSceneNode*)mSelNode;
		node->SetRectInfo( Twidth, Bwidth, LHeight, RHeight );

		mSelNode->Update();
		Update();
	}
}

void cRegenToolView::SetSelDoorAppendY( float appendY )
{
	if( mSelNode == 0 )
		return;
	if( mNaviMesh->IsInited() == false )
		return;

	if( mSelNode->GetType() == SCENENODE_DOOR )
	{
		cDoorSceneNode* node = (cDoorSceneNode*)mSelNode;
		node->SetRectAppendY( appendY );

		mSelNode->Update();
		Update();
	}
}

void cRegenToolView::SetSelPotalNameHeight( float nameHeight )
{
	if( mSelNode == 0 )
		return;
	if( mNaviMesh->IsInited() == false )
		return;

	if( mSelNode->GetType() == SCENENODE_PORTAL )
	{
		cPortalSceneNode* node = (cPortalSceneNode*)mSelNode;
		node->SetNameHeight( nameHeight );

		mSelNode->Update();
		Update();
	}
}


NiPoint3 cRegenToolView::GetSelTranslate() const
{
	switch( mSelNode->GetType() )
	{
	case SCENENODE_NPC:
		return mNpcDialog->GetTranslate();
	case SCENENODE_MONSTER:
		return mMonsterDialog->GetTranslate();
	case SCENENODE_PORTAL:
		return mPortalDialog->GetTranslate();
	case SCENENODE_GATHERING:
		return mGatheringDialog->GetTranslate();
	case SCENENODE_DOOR:
		return mDoorDialog->GetTranslate();
	default:
		return NiPoint3::ZERO;
	}
}

float cRegenToolView::GetSelRotate() const
{
	switch( mSelNode->GetType() )
	{
	case SCENENODE_NPC:
		return mNpcDialog->GetRotateAngle();
	case SCENENODE_MONSTER:
		return mMonsterDialog->GetRotateAngle();
	case SCENENODE_PORTAL:
		return mPortalDialog->GetRotateAngle();
	case SCENENODE_GATHERING:
		return mGatheringDialog->GetRotateAngle();
	case SCENENODE_DOOR:
		return mDoorDialog->GetRotateAngle();
	default:
		return 0.0f;
	}
}

float cRegenToolView::GetSelAppendZ() const
{
	if( mSelNode->GetType() == SCENENODE_GATHERING )
	{
		return ((cGatheringSceneNode*)mSelNode)->GetAppendZ();
	}
	else if( mSelNode->GetType() == SCENENODE_DOOR )
	{
		return ((cDoorSceneNode*)mSelNode)->GetAppendZ();
	}

	return 0.0f;
}

const cString& cRegenToolView::GetMapName( unsigned int mapIdx )
{
	cMapNameMap::cConstIterator i = mMapNameMap.Find( mapIdx );

	if( i != mMapNameMap.End() )
		return i->mSecond;
	else
		return mErrorNpcName;
}

const cString& cRegenToolView::GetModelFile( unsigned int modelIndex ) const
{
	cModelFileMap::cConstIterator i = mModelFileMap.Find( modelIndex );

	if( i != mModelFileMap.End() )
		return i->mSecond;
	else
		return mErrorModelFile;
}

const cNpcInfo* cRegenToolView::GetNpcInfo( unsigned int index ) const
{
	cNpcInfoMap::cConstIterator i = mNpcInfoMap.Find( index );

	if( i != mNpcInfoMap.End() )
		return i->mSecond;
	else
		return 0;
}

const cString& cRegenToolView::GetNpcNameByNameIndex( unsigned int nameIndex ) const
{
	cNpcNameMap::cConstIterator i = mNpcNameMap.Find( nameIndex );

	if( i != mNpcNameMap.End() )
		return i->mSecond;
	else
		return mErrorNpcName;
}

const cString& cRegenToolView::GetNpcNameByNpcIndex( unsigned int npcIndex ) const
{
	unsigned int nameIndex = 0;

	{
		cNpcInfoMap::cConstIterator i = mNpcInfoMap.Find( npcIndex );

		if( i == mNpcInfoMap.End() )
			return mErrorNpcName;

		cNpcInfo* info = i->mSecond;
		nameIndex = info->mNameIndex;
	}
	{
		cNpcNameMap::cConstIterator i = mNpcNameMap.Find( nameIndex );

		if( i != mNpcNameMap.End() )
			return i->mSecond;
		else
			return mErrorNpcName;
	}
}

const cMonsterInfo* cRegenToolView::GetMonsterInfo( unsigned int index ) const
{
	cMonsterInfoMap::cConstIterator i = mMonsterInfoMap.Find( index );

	if( i != mMonsterInfoMap.End() )
		return i->mSecond;
	else
		return 0;
}

const cString& cRegenToolView::GetMonsterNameByNameIndex( unsigned int nameIndex ) const
{
	cMonsterNameMap::cConstIterator i = mMonsterNameMap.Find( nameIndex );

	if( i != mMonsterNameMap.End() )
		return i->mSecond;
	else
		return mErrorMonsterName;
}

const cString& cRegenToolView::GetMonsterNameByMonIndex( unsigned int monIndex ) const
{
	unsigned int nameIndex = 0;

	{
		cMonsterInfoMap::cConstIterator i = mMonsterInfoMap.Find( monIndex );

		if( i == mMonsterInfoMap.End() )
			return mErrorMonsterName;

		cMonsterInfo* info = i->mSecond;
		nameIndex = info->mNameIndex;
	}
	{
		cMonsterNameMap::cConstIterator i = mMonsterNameMap.Find( nameIndex );

		if( i != mMonsterNameMap.End() )
			return i->mSecond;
		else
			return mErrorMonsterName;
	}
}

const cGatheringInfo* cRegenToolView::GetGatheringInfo( unsigned int index ) const
{
	cGatheringInfoMap::cConstIterator i = mGatheringInfoMap.Find( index );

	if( i != mGatheringInfoMap.End() )
		return i->mSecond;
	else
		return 0;
}

const cString& cRegenToolView::GetGatheringNameByNameIndex( unsigned int nameIndex ) const
{
	cGatheringNameMap::cConstIterator i = mGatheringNameMap.Find( nameIndex );

	if( i != mGatheringNameMap.End() )
		return i->mSecond;
	else
		return mErrorGatheringName;
}

const cString& cRegenToolView::GetGatheringNameByGatherIndex( unsigned int gatherIndex ) const
{
	const cGatheringInfo* info = GetGatheringInfo( gatherIndex );
	if( info == 0 )
		return mErrorGatheringName;

	cGatheringNameMap::cConstIterator i = mGatheringNameMap.Find( info->mNameIndex );
	if( i != mGatheringNameMap.End() )
		return i->mSecond;
	else
		return mErrorGatheringName;
}

cPortalInfo* cRegenToolView::GetPortalInfo( unsigned int index ) const
{
	cPortalInfoMap::cConstIterator i = mPortalInfoMap.Find( index );

	if( i != mPortalInfoMap.End() )
		return i->mSecond;
	else
		return 0;
}

cDoorInfo* cRegenToolView::GetDoorInfo( unsigned int index ) const
{
	cDoorInfoMap::cConstIterator i = mDoorInfoMap.Find( index );

	if( i != mDoorInfoMap.End() )
		return i->mSecond;
	else
		return 0;
}

void cRegenToolView::RenderLines()
{
	if( mSelMonsterGroup )
	{
		cMonsterList::cIterator i = mSelMonsterGroup->mNodeList.Begin();
		cMonsterList::cIterator iend = mSelMonsterGroup->mNodeList.End();

		for( ; i != iend; ++i )
		{
			mGroupLines->Render( *i, mCamera );
		}
	}

	if( mSelGatheringGroup )
	{
		cGatheringList::cIterator i = mSelGatheringGroup->mNodeList.Begin();
		cGatheringList::cIterator iend = mSelGatheringGroup->mNodeList.End();

		for( ; i != iend; ++i )
		{
			mGroupLines->Render( *i, mCamera );
		}
	}

	if( mSelNode )
	{
		if( mSelNode->GetType() == SCENENODE_DOOR )
		{
			((cDoorSceneNode*)mSelNode)->RenderRect();
		}
		else if( mSelNode->GetType() == SCENENODE_PORTAL )
		{
			((cPortalSceneNode*)mSelNode)->RenderRect();
		}

		if( mSelLines )
			mSelLines->Render( mSelNode, mCamera );
	}

	if( mSelPattern )
	{
		mSelPattern->RenderPattern();
	}
}

BOOL cRegenToolView::PreCreateWindow( CREATESTRUCT& cs ) 
{
	if( !CView::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(0, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), 0);

	return TRUE;
}

void cRegenToolView::OnDraw( CDC* )
{
	/// 처리
	mApplication->Process( false );

	if( mMapDialog && mMapDialog->IsNaviFieldChecked() )
		mNaviField->Process();

	if( mDummyNode )
	{
		if( mSelNode && mSelNode->GetType() == SCENENODE_PORTAL )
		{
			if( mSelNode->GetUserData1() != 0 )
			{
				mDummyNode->SetCulled( true );
			}
			else
			{
				mDummyNode->SetCulled( false );
				mDummyNode->SetTranslate( mSelNode->GetWorldTranslate() );
				mDummyNode->SetRotate( mSelNode->GetWorldRotate() );
				mDummyNode->Update();
			}
		}
		else
		{
			mDummyNode->SetCulled( true );
		}
	}

	/// 렌더링
	if( RENDERSYS )
	{
		RENDERSYS->BeginFrame( false );
		{
			WORLDMAN->Render();

			RenderLines();

			SCENEMAN->Render();

			if( mMapDialog && mMapDialog->IsNaviFieldChecked() )
				mNaviField->Render();
		}
		RENDERSYS->EndFrame();
		RENDERSYS->DisplayFrame();
	}
}

int cRegenToolView::OnCreate( LPCREATESTRUCT cs )
{
	if( CView::OnCreate(cs) == -1 )
		return -1;

	/// 엔진 초기화
//	NiInit();

	/// 어플리케이션을 생성
	mApplication = new cApplication;

	/// 어플리케이션을 초기화
	if( mApplication->Init( GetSafeHwnd() ) == false )
	{
		assert( 0 && "failed to init application" );
		return -1;
	}

	/// 카메라를 생성
	mCamera = CAMERAMAN->CreateFree( true );

	/*// 현재 경로를 설정
	cString path = theApp.GetBaseDir();
	path += "MapData";
	SetCurrentDirectory( path.Cstr() );

	/// 편집자를 생성
	//mObjectEditor = new cObjectEditor;

	/// 안개를 적용
	TERRAIN->SetFog( true, NiColor::WHITE, 0.1f );
	SCENEMAN->SetFog( true, NiColor::WHITE, 0.1f );

	/// 타이머 설정
	SetTimer( 1, 20, 0 );
	*/
	return 0;
}

void cRegenToolView::OnDestroy()
{
	/// 어플리케이션 종료
	mApplication->Exit();

	CView::OnDestroy();
}

BOOL cRegenToolView::OnEraseBkgnd( CDC* ) 
{
	return FALSE;
}

void cRegenToolView::OnSize( UINT type, int cx, int cy )
{
	CView::OnSize(type, cx, cy);
	Resize( cx, cy );
}

void cRegenToolView::OnTimer( UINT_PTR /*nIDEvent*/ )
{
		//switch( mEditMode )
		//{
		//case eEDIT_TERRAIN_BUILD:
		//	mTerrainEditor->OnTimer_Build();
		//	break;
		//}
}

void cRegenToolView::OnMouseMove( UINT /*flags*/, CPoint point )
{
	MOUSE->SetPosition( point.x, point.y );
	
	if( mSelNode )
	{
		if( MOUSE->IsRButtonDown() )
		{
			if( KEY->IsDown(KEY_X) )
			{
				NiPoint3 t = GetSelTranslate();
				t.x += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
				SetSelTranslate( t, true );
			}
			if( KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_C) )
			{
				NiPoint3 t = GetSelTranslate();
				t.y += ((point.x - mMousePos.x) - (point.y - mMousePos.y)) * 10.0f;
				SetSelTranslate( t, true );
			}
			if( KEY->IsDown(KEY_Z) )
			{
				float zangle = GetSelRotate();
				zangle += D3DXToRadian(mMousePos.x - point.x);
				SetSelRotate( zangle, true );
			}
			if( KEY->IsDown(KEY_V) )
			{
				float appendZ = GetSelAppendZ();
				appendZ += (mMousePos.y - point.y);
				SetSelAppendZ( appendZ, true );
			}
		}
	}

	mMousePos = point;
}

BOOL cRegenToolView::OnMouseWheel( UINT /*flags*/, short zdelta, CPoint /*point*/ )
{
	MOUSE->SetDeltaWheel( (float)zdelta * 30 );
	return TRUE;
}

void cRegenToolView::OnLButtonDown( UINT /*flags*/, CPoint point )
{
	MOUSE->SetLButtonDown( point.x, point.y );

	if( KEY->IsDown(KEY_X) || KEY->IsDown(KEY_Y) || KEY->IsDown(KEY_Z) || KEY->IsDown(KEY_C) )
		return;

	if( SCENEMAN->IsInited() == false )
		return;

	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_DYNAMIC ) )
	{
		SelectNode( mPickedArray[0] );
	}
	else if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) && mPickedArray[0]->GetType() != SCENENODE_STATIC )
	{
		SelectNode( mPickedArray[0] );
	}

	/*if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_DYNAMIC ) )
	{
		SelectNode( mPickedArray[0] );
	}
	else if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) && mPickedArray[0]->GetType() == SCENENODE_PORTAL )
	{
		SelectNode( mPickedArray[0] );
	}*/
}

void cRegenToolView::OnLButtonUp( UINT /*flags*/, CPoint point )
{
	MOUSE->SetLButtonUp( point.x, point.y );
	//mTerrainEditor->OnLButtonUp();
}

void cRegenToolView::OnRButtonDown( UINT flags, CPoint point )
{
	if( GetCapture() != this )
	{
		SetCapture();
	}
	MOUSE->SetRButtonDown( point.x, point.y );

	if( mNaviMesh->IsInited() == false )
		return;

	/// 
	if( mPatternDialog->IsCheckedNew() || 
	    (mPatternDialog->IsCheckedNew() && KEY->IsDown( KEY_N )) )
	{
		if( !mSelPattern )
			return;

		NiPoint3 pos;

		if( mNaviMesh->Pick( &pos, point.x, point.y ) )
		{
			cPatternSceneNode* n = AddPatternPos( mSelPattern->mPatternIdx, pos );
			if( n )
				SelectNode( n );
		}
	}

	if( mPatternDialog->IsCheckedModify() ||
		(mPatternDialog->IsCheckedModify() && KEY->IsDown( KEY_M )) )
	{
		NiPoint3 pos;

		if( mNaviMesh->Pick( &pos, point.x, point.y ) )
		{
			ModifyPatternPos( pos );
		}
	}

	if( mSelNode )
	{
		bool ctrl = (flags & MK_CONTROL) != 0;

		if( ctrl )
		{
			NiPoint3 pos;

			if( mNaviMesh->Pick( &pos, point.x, point.y ) )
			{
				SetSelTranslate( pos, true );
			}
		}
	}
}

void cRegenToolView::OnRButtonUp( UINT /*flags*/, CPoint point )
{
	if( GetCapture() == this )
	{
		ReleaseCapture();
	}
	MOUSE->SetRButtonUp( point.x, point.y );
}

void cRegenToolView::OnMButtonDown( UINT /*flags*/, CPoint point )
{
	MOUSE->SetMButtonDown( point.x, point.y );
}

void cRegenToolView::OnMButtonUp( UINT /*flags*/, CPoint point )
{
	MOUSE->SetMButtonUp( point.x, point.y );
}

void cRegenToolView::OnKeyDown( UINT c, UINT /*repeats*/, UINT /*flags*/ )
{
	KEY->SetDown( (eKeyCode)c );

	switch( c )
	{
	case KEY_DELETE:
		if( mSelNode )
		{
			int mb = MessageBox( "Do you want to delete selected object?", "Object", MB_YESNO );

			if( mb == IDYES )
				DeleteSel();
		}
		break;
	case KEY_N:
		{
			mPatternDialog->SetCheckNew( true );
		}
		break;
	case KEY_M:
		{
			mPatternDialog->SetCheckModify( true );
		}
		break;
	}
}

void cRegenToolView::OnKeyUp( UINT c, UINT /*repeats*/, UINT /*flags*/ )
{
	KEY->SetUp( (eKeyCode)c );

	switch( c )
	{
	case KEY_N:
		{
			mPatternDialog->SetCheckNew( false );
		}
		break;
	case KEY_M:
		{
			mPatternDialog->SetCheckModify( false );
		}
		break;
	}
}