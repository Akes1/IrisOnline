#include "stdafx.h"
#include "SceneTree.h"

#include "Ray.h"
#include "Camera.h"
#include "SceneNode.h"

tPool<cSceneTreeNode> cSceneTreeNode::mPool( 2048, 256 );

/// 컬링용
NiFrustumPlanes* cSceneTreeNode::mFrustum = 0;
cSceneNode::eType cSceneTreeNode::mPickType = cSceneNode::eNULL;

/// 충돌 검사용
const cRay* cSceneTreeNode::mRay = 0;
float cSceneTreeNode::mMaxDistance = 0;
const cSphere* cSceneTreeNode::mSphere = 0;
tArray<void*>* cSceneTreeNode::mCollidedArray = 0;
bool cSceneTreeNode::mCameraRay = false;

cSceneTreeNode::cSceneTreeNode( cSceneTree* tree, cSceneTreeNode* parent, const NiPoint3& center, float radius )
: cSphere( center, radius )
, mTree( tree )
, mParent( parent )
{
	mChild[0] = mChild[1] = mChild[2] = mChild[3] = 0;

	float r = (radius * 1.4142135623731f) * 0.5f;//radius * 0.5f;
	float d = r * 0.5f;
	//	float d = r * 1.0f/1.4142135623731f;

	mTestSpheres[0].Set( center.x - d, center.y - d, 0.0f, r );
	mTestSpheres[0].mResult = false;

	mTestSpheres[1].Set( center.x + d, center.y - d, 0.0f, r );
	mTestSpheres[1].mResult = false;

	mTestSpheres[2].Set( center.x - d, center.y + d, 0.0f, r );
	mTestSpheres[2].mResult = false;

	mTestSpheres[3].Set( center.x + d, center.y + d, 0.0f, r );
	mTestSpheres[3].mResult = false;

	mObjectList.Clear();
}

cSceneTreeNode::cSceneTreeNode( cSceneTree* tree, cSceneTreeNode* parent, const cSphere& sphere )
: cSphere( sphere )
, mTree( tree )
, mParent( parent )
{
	mChild[0] = mChild[1] = mChild[2] = mChild[3] = 0;


	/// 외접원
	float r = mRadius * 0.5f;
	float d = r * 1.0f/1.4142135623731f;

	//	float r = (mRadius * 1.4142135623731f) * 0.5f;
	//	float d = r * 0.95f;

	mTestSpheres[0].Set( mCenter.x - d, mCenter.y - d, 0.0f, r );
	mTestSpheres[0].mResult = false;

	mTestSpheres[1].Set( mCenter.x + d, mCenter.y - d, 0.0f, r );
	mTestSpheres[1].mResult = false;

	mTestSpheres[2].Set( mCenter.x - d, mCenter.y + d, 0.0f, r );
	mTestSpheres[2].mResult = false;

	mTestSpheres[3].Set( mCenter.x + d, mCenter.y + d, 0.0f, r );
	mTestSpheres[3].mResult = false;

	mObjectList.Clear();
}

cSceneTreeNode::~cSceneTreeNode()
{
	delete mChild[0];
	delete mChild[1];
	delete mChild[2];
	delete mChild[3];

	mParent = 0;
	mObjectList.Clear();
}

void* cSceneTreeNode::operator new( size_t /*n*/ )
{ 
	return mPool.Alloc();
}

void cSceneTreeNode::operator delete( void* p )
{
	mPool.Free( (cSceneTreeNode*)p );
}

void cSceneTreeNode::PushDirect( cSceneNode* node )
{
	assert(node);
	cSphere s = node->GetBoundSphere();

	if( ContainSphere( s ) )
	{
		node->mContainer = this;
		node->mIteratorValid = true;
		node->mIteratorByContainer = mObjectList.Insert( mObjectList.End(), node );
	}
	else
	{
		if( mParent )
		{
			mParent->PushDirect( node );
		}
		else
		{
			assert(0);
			node->mContainer = this;
			node->mIteratorValid = true;
			node->mIteratorByContainer = mObjectList.Insert( mObjectList.End(), node );
		}
	}
}

void cSceneTreeNode::Push( cSceneNode* node )
{
	assert( node );

	if( node->GetRadius() <= 0.0f )
	{
		node->mContainer = this;
		node->mIteratorValid = true;
		node->mIteratorByContainer = mObjectList.Insert( mObjectList.End(), node );
		return;
	}

	/// 장면 노드가 트리 노드보다 더 크면 상위 노드로 장면 노드를 전달
	if( node->GetRadius() > mRadius )
	{
		if( mParent )
		{
			mParent->PushDirect( node );
		}
		else
		{
			//			assert(0);
			node->mContainer = this;
			node->mIteratorValid = true;
			node->mIteratorByContainer = mObjectList.Insert( mObjectList.End(), node );
		}
		return;
	}

	/// 더 작은 하위노드를 만들지 않는다.
	if( mRadius <= 10000.0f )
	{
		node->mContainer = this;
		node->mIteratorValid = true;
		node->mIteratorByContainer = mObjectList.Insert( mObjectList.End(), node );
		return;
	}

	/// 자식 노드들에 대해 장면 노드 포함 여부를 검사
	int i = 0;
	int count = 0;

	NiPoint3 checkPos = node->GetCenter();
	//float checkR = node->GetRadius();

	cSphere s( node->GetCenter(), node->GetRadius() );
	mTestSpheres[0].mResult = false;
	mTestSpheres[1].mResult = false;
	mTestSpheres[2].mResult = false;
	mTestSpheres[3].mResult = false;

	if( mTestSpheres[0].ContainSphere( s ) )
	{
		i = 0;
		mTestSpheres[0].mResult = true;
		++count;
	}
	if( mTestSpheres[1].ContainSphere( s ) )
	{
		i = 1;
		mTestSpheres[1].mResult = true;
		++count;
	}
	if( mTestSpheres[2].ContainSphere( s ) )
	{
		i = 2;
		mTestSpheres[2].mResult = true;
		++count;
	}
	if( mTestSpheres[3].ContainSphere( s ) )
	{
		i = 3;
		mTestSpheres[3].mResult = true;
		++count;
	}

	/// 장면 노드를 완전히 포함하는 자식 노드의 수를 검사
	switch( count )
	{
	case 0:
		{
			/// 현재 노드에 추가
			node->mContainer = this;
			node->mIteratorValid = true;
			node->mIteratorByContainer = mObjectList.Insert( mObjectList.End(), node );
		}
		break;
	case 1:
		{
			/// 해당 자식 노드에 장면 노드를 전달
			cSceneTreeNode* child = mChild[i];
			if( child == 0 )
				child = mChild[i] = new cSceneTreeNode( mTree, this, mTestSpheres[i] );

			child->Push( node );
		}
		break;
	case 2:
	case 3:
	case 4:
	default:
		{
			/// 가장 가까운 자식 노드에 장면 노드를 전달
			float minDistance = NI_INFINITY;
			int i = 0;
			const NiPoint3& c = node->GetCenter();
			float d;

			if( mTestSpheres[0].mResult )
			{
				d = mTestSpheres[0].GetDistance2( c );
				if( d < minDistance )
				{
					i = 0;
					minDistance = d;
				}
			}
			if( mTestSpheres[1].mResult )
			{
				d = mTestSpheres[1].GetDistance2( c );
				if( d < minDistance )
				{
					i = 1;
					minDistance = d;
				}
			}
			if( mTestSpheres[2].mResult )
			{
				d = mTestSpheres[2].GetDistance2( c );
				if( d < minDistance )
				{
					i = 2;
					minDistance = d;
				}
			}
			if( mTestSpheres[3].mResult )
			{
				d = mTestSpheres[3].GetDistance2( c );
				if( d < minDistance )
				{
					i = 3;
					minDistance = d;
				}
			}

			/// 해당 자식 노드에 장면 노드를 전달
			cSceneTreeNode* child = mChild[i];

			if( child == 0 )
				child = mChild[i] = new cSceneTreeNode( mTree, this, mTestSpheres[i] );

			child->Push( node );
		}
		break;
	}
}

void cSceneTreeNode::Remove( cSceneNode* node )
{
	assert( node );

	if( node->mIteratorValid )
	{
		node->mIteratorValid = false;
		mObjectList.Erase( node->mIteratorByContainer );
	}
}

void cSceneTreeNode::Update( cSceneNode* node )
{
	/// 장면 노드가 속한 트리 노드에 대해 포함 여부를 검사
	const cSphere& s = node->GetBoundSphere();

	if( ContainSphere( s ) == false )
	{
		/// 장면 노드를 트리 노드에서 제거하고 상위 트리 노드에 대해 포함 유무를 검사
		Remove( node );

		if( mParent )
		{
			if( mParent->ContainSphere( s ) )
				mParent->Push( node );
			else
				mTree->Push( node );
		}
		else
		{
			mTree->Push( node );
		}
	}
}

void cSceneTreeNode::Cull()
{
	unsigned int saveActive = mFrustum->GetActivePlaneState();
	unsigned int side = 0;
	unsigned int c = 0;
	{
		unsigned int i;
		for( i = 0; i<NiFrustumPlanes::MAX_PLANES; i++ )
		{
			if( mFrustum->IsPlaneActive(i) == false )
				continue;

			side = WhichSide( mFrustum->GetPlane(i) );

			if( side == NiPlane::POSITIVE_SIDE )
			{
				mFrustum->DisablePlane(i);
				c++;
			}
		}

		saveActive = mFrustum->GetActivePlaneState();

		if( c == NiFrustumPlanes::MAX_PLANES )
		{
			AddToVisibleArray();
		}
		else
		{
			/// 부분 교차 -> 자손들을 계속 검사
			/// 포함된 개체들에 대해 검사
			if( mObjectList.IsEmpty() == false )
			{
				cObjectList::cIterator itr = mObjectList.Begin();
				cObjectList::cIterator iend = mObjectList.End();
				cSceneNode* n = 0;

				for( ; itr != iend; ++itr )
				{
					n = (cSceneNode*)*itr;

					if( n->IsKindof( mPickType ) == false )
						continue;
					if( n->IsEnableFrustumFlag() == false )
						continue;

					if( mFrustum->IsPlaneActive(NiFrustumPlanes::FAR_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::FAR_PLANE) ) == NiPlane::NEGATIVE_SIDE )
						continue;
					if( mFrustum->IsPlaneActive(NiFrustumPlanes::LEFT_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::LEFT_PLANE) ) == NiPlane::NEGATIVE_SIDE )
						continue;
					if( mFrustum->IsPlaneActive(NiFrustumPlanes::RIGHT_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::RIGHT_PLANE) ) == NiPlane::NEGATIVE_SIDE )
						continue;
					if( mFrustum->IsPlaneActive(NiFrustumPlanes::TOP_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::TOP_PLANE) ) == NiPlane::NEGATIVE_SIDE )
						continue;
					if( mFrustum->IsPlaneActive(NiFrustumPlanes::BOTTOM_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::BOTTOM_PLANE) ) == NiPlane::NEGATIVE_SIDE )
						continue;

					if( n->OnVisible() )
						n->AddToVisibleArray();
				}
			}
		}

		/// 자손들에 대해 컬링
		if( mChild[0] )
		{
			mFrustum->SetActivePlaneState( saveActive );
			mChild[0]->Cull();
		}
		if( mChild[1] )
		{
			mFrustum->SetActivePlaneState( saveActive );
			mChild[1]->Cull();
		}
		if( mChild[2] )
		{
			mFrustum->SetActivePlaneState( saveActive );
			mChild[2]->Cull();
		}
		if( mChild[3] )
		{
			mFrustum->SetActivePlaneState( saveActive );
			mChild[3]->Cull();
		}
	}
	/*
	unsigned int saveActive = mFrustum->GetActivePlaneState();
	unsigned int side = 0;
	unsigned int c = 0;
	{
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::FAR_PLANE) )
	{
	side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::FAR_PLANE) );

	if( side == NiPlane::NEGATIVE_SIDE )
	{
	mFrustum->SetActivePlaneState( saveActive );
	return;
	}
	if( side == NiPlane::POSITIVE_SIDE )
	{
	mFrustum->DisablePlane( NiFrustumPlanes::FAR_PLANE );
	++c;
	}
	}
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::LEFT_PLANE) )
	{
	side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::LEFT_PLANE) );

	if( side == NiPlane::NEGATIVE_SIDE )
	{
	mFrustum->SetActivePlaneState( saveActive );
	return;
	}
	if( side == NiPlane::POSITIVE_SIDE )
	{
	mFrustum->DisablePlane( NiFrustumPlanes::LEFT_PLANE );
	++c;
	}
	}
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::RIGHT_PLANE) )
	{
	side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::RIGHT_PLANE) );

	if( side == NiPlane::NEGATIVE_SIDE )
	{
	mFrustum->SetActivePlaneState( saveActive );
	return;
	}
	if( side == NiPlane::POSITIVE_SIDE )
	{
	mFrustum->DisablePlane( NiFrustumPlanes::RIGHT_PLANE );
	++c;
	}
	}
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::TOP_PLANE) )
	{
	side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::TOP_PLANE) );

	if( side == NiPlane::NEGATIVE_SIDE )
	{
	mFrustum->SetActivePlaneState( saveActive );
	return;
	}
	if( side == NiPlane::POSITIVE_SIDE )
	{
	mFrustum->DisablePlane( NiFrustumPlanes::TOP_PLANE );
	++c;
	}
	}
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::BOTTOM_PLANE) )
	{
	side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::BOTTOM_PLANE) );

	if( side == NiPlane::NEGATIVE_SIDE )
	{
	mFrustum->SetActivePlaneState( saveActive );
	return;
	}
	if( side == NiPlane::POSITIVE_SIDE )
	{
	mFrustum->DisablePlane( NiFrustumPlanes::BOTTOM_PLANE );
	++c;
	}
	}
	}

	if( c == 5 )
	{
	/// 완전 포함 -> 자손들을 모두 추가
	AddToVisibleArray();
	}
	else
	{
	/// 부분 교차 -> 자손들을 계속 검사
	/// 포함된 개체들에 대해 검사
	if( mObjectList.IsEmpty() == false )
	{
	cObjectList::cIterator i = mObjectList.Begin();
	cObjectList::cIterator iend = mObjectList.End();
	cSceneNode* n = 0;

	for( ; i != iend; ++i )
	{
	n = (cSceneNode*)*i;

	if( n->IsKindof( mPickType ) == false )
	continue;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::FAR_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::FAR_PLANE) ) == NiPlane::NEGATIVE_SIDE )
	continue;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::LEFT_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::LEFT_PLANE) ) == NiPlane::NEGATIVE_SIDE )
	continue;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::RIGHT_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::RIGHT_PLANE) ) == NiPlane::NEGATIVE_SIDE )
	continue;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::TOP_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::TOP_PLANE) ) == NiPlane::NEGATIVE_SIDE )
	continue;
	if( mFrustum->IsPlaneActive(NiFrustumPlanes::BOTTOM_PLANE) && n->WhichSide( mFrustum->GetPlane(NiFrustumPlanes::BOTTOM_PLANE) ) == NiPlane::NEGATIVE_SIDE )
	continue;

	if( n->OnVisible() )
	n->AddToVisibleArray();
	}
	}

	/// 자손들에 대해 컬링
	if( mChild[0] )
	mChild[0]->Cull();
	if( mChild[1] )
	mChild[1]->Cull();
	if( mChild[2] )
	mChild[2]->Cull();
	if( mChild[3] )
	mChild[3]->Cull();
	}

	mFrustum->SetActivePlaneState( saveActive );
	*/
}

void cSceneTreeNode::AddToVisibleArray() const
{
	/// 포함된 개체들을 추가
	cObjectList::cConstIterator i = mObjectList.Begin();
	cObjectList::cConstIterator iend = mObjectList.End();
	cSceneNode* n = 0;

	for( ; i != iend; ++i )
	{
		n = (cSceneNode*)*i;

		if( n->IsKindof( mPickType ) == false )
			continue;

		if( n->OnVisible() )
			n->AddToVisibleArray();
	}

	// 자식 노드들에 대해 재귀 호출
	if( mChild[0] )
		mChild[0]->AddToVisibleArray();
	if( mChild[1] )
		mChild[1]->AddToVisibleArray();
	if( mChild[2] )
		mChild[2]->AddToVisibleArray();
	if( mChild[3] )
		mChild[3]->AddToVisibleArray();
}

bool cSceneTreeNode::CollideRay_Kind()
{
	bool ret = false;

	if( IntersectRay( *mRay, mMaxDistance ) )
	{
		/// 포함된 장면 노드들에 대해 교차 검사
		if( mObjectList.IsEmpty() == false )
		{
			cObjectList::cIterator i = mObjectList.Begin();
			cObjectList::cIterator iend = mObjectList.End();
			cSceneNode* n = 0;

			for( ; i != iend; ++i )
			{
				n = (cSceneNode*)*i;

				if( n->IsKindof( mPickType ) == false )
					continue;

				if( n->IsViewNode() == false )
					continue;

				if( n->Pick( *mRay ) && n->GetPickDistance() < mMaxDistance )
				{
					mCollidedArray->PushBack( n );
					ret = true;
				}
			}
		}

		/// 자식들에 대해 교차 검사
		if( mChild[0] )
			ret |= mChild[0]->CollideRay_Kind();
		if( mChild[1] )
			ret |= mChild[1]->CollideRay_Kind();
		if( mChild[2] )
			ret |= mChild[2]->CollideRay_Kind();
		if( mChild[3] )
			ret |= mChild[3]->CollideRay_Kind();
	}
	return ret;
}

bool cSceneTreeNode::CollideSphere_Kind()
{
	bool ret = false;

	if( IntersectSphere( *mSphere ) )
	{
		/// 포함된 장면 노드들에 대해 교차 검사
		if( mObjectList.IsEmpty() == false )
		{
			cObjectList::cIterator i = mObjectList.Begin();
			cObjectList::cIterator iend = mObjectList.End();
			cSceneNode* n = 0;

			for( ; i != iend; ++i )
			{
				n = (cSceneNode*)*i;

				if( n->IsKindof( mPickType ) == false )
					continue;

				if( n->GetBoundSphere().IntersectSphere( *mSphere ) )
				{
					n->UpdatePickInfo( (void*)mSphere );

					mCollidedArray->PushBack( n );
					ret = true;
				}
			}
		}

		/// 자식들에 대해 교차 검사
		if( mChild[0] )
			ret |= mChild[0]->CollideSphere_Kind();
		if( mChild[1] )
			ret |= mChild[1]->CollideSphere_Kind();
		if( mChild[2] )
			ret |= mChild[2]->CollideSphere_Kind();
		if( mChild[3] )
			ret |= mChild[3]->CollideSphere_Kind();
	}
	return ret;
}

bool cSceneTreeNode::CollideSphere_Type()
{
	bool ret = false;

	if( IntersectSphere( *mSphere ) )
	{
		/// 포함된 장면 노드들에 대해 교차 검사
		if( mObjectList.IsEmpty() == false )
		{
			cObjectList::cIterator i = mObjectList.Begin();
			cObjectList::cIterator iend = mObjectList.End();
			cSceneNode* n = 0;

			for( ; i != iend; ++i )
			{
				n = (cSceneNode*)*i;

				if( n->GetType() != mPickType )
					continue;

				if( n->GetBoundSphere().IntersectSphere( *mSphere ) )
				{
					n->UpdatePickInfo( (void*)mSphere );

					mCollidedArray->PushBack( n );
					ret = true;
				}
			}
		}

		/// 자식들에 대해 교차 검사
		if( mChild[0] )
			ret |= mChild[0]->CollideSphere_Type();
		if( mChild[1] )
			ret |= mChild[1]->CollideSphere_Type();
		if( mChild[2] )
			ret |= mChild[2]->CollideSphere_Type();
		if( mChild[3] )
			ret |= mChild[3]->CollideSphere_Type();
	}
	return ret;
}

cSceneTree::cSceneTree()
: mRootNode( 0 )
, mCenter( NiPoint3::ZERO )
, mMinRadius( 0.0f )
, mMaxRadius( 0.0f )
{
}

cSceneTree::~cSceneTree()
{
	Clear();
}

void cSceneTree::Clear()
{
	delete mRootNode;
	mRootNode = 0;
}

void cSceneTree::Init( const NiPoint3& center, float minRadius, float maxRadius )
{
	mCenter = center;
	mMinRadius = minRadius;
	mMaxRadius = maxRadius;

	delete mRootNode;
	mRootNode = new cSceneTreeNode( this, 0, center, maxRadius );
}

void cSceneTree::Push( cSceneNode* node )
{
	assert( node );
	assert( mRootNode );

	mRootNode->Push( node );
}

void cSceneTree::Cull( NiVisibleArray* solidArray, NiVisibleArray* alphaArray, NiVisibleArray* alphaTestArray, cCamera* cam, cSceneNode::eType type )
{
	assert( solidArray );
	assert( alphaArray );
	assert( cam );
	assert( mRootNode );

	NiCamera* nicam = (NiCamera*)*cam;
	NiFrustumPlanes frustum( *nicam );
	cSceneNode::mCamera = nicam;
	cSceneNode::mSolidArray = solidArray;
	cSceneNode::mAlphaArray = alphaArray;
	cSceneNode::mAlphaTestArray = alphaTestArray;
	cSceneTreeNode::mFrustum = &frustum;
	cSceneTreeNode::mPickType = type;

	mRootNode->Cull();
}

bool cSceneTree::CollideRay_Kind( tArray<void*>* collidedArray, const cRay& ray, float maxDistance, bool sortByDistance, cSceneNode::eType type )
{
	assert( collidedArray );

	if( IsEmpty() )
		return false;

	cSceneTreeNode::mRay = &ray;
	cSceneTreeNode::mMaxDistance = maxDistance;
	cSceneTreeNode::mPickType = type;
	cSceneTreeNode::mCollidedArray = collidedArray;

	bool ret = mRootNode->CollideRay_Kind();

	if( ret && sortByDistance && collidedArray->GetSize() > 1 )
	{
		::Sort( collidedArray->Begin(), collidedArray->End(), cSceneNodeCompareForPicking() );
	}
	return ret;
}

bool cSceneTree::CollideSphere_Kind( tArray<void*>* collidedArray, const cSphere& sphere, cSceneNode::eType type )
{
	assert( collidedArray );

	if( IsEmpty() )
		return false;

	cSceneTreeNode::mSphere = &sphere;
	cSceneTreeNode::mPickType = type;
	cSceneTreeNode::mCollidedArray = collidedArray;

	return mRootNode->CollideSphere_Kind();
}

bool cSceneTree::CollideSphere_Type( tArray<void*>* collidedArray, const cSphere& sphere, cSceneNode::eType type )
{
	assert( collidedArray );

	if( IsEmpty() )
		return false;

	cSceneTreeNode::mSphere = &sphere;
	cSceneTreeNode::mPickType = type;
	cSceneTreeNode::mCollidedArray = collidedArray;

	return mRootNode->CollideSphere_Type();
}
