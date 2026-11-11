#include "stdafx.h"
#include "SceneTree.h"

#include "Ray.h"
#include "SceneNode.h"

tPool<cSceneTreeNode> cSceneTreeNode::mPool( 2048, 256 );

/// 컬링용
NiFrustumPlanes* cSceneTreeNode::mFrustum = 0;
cSceneCuller* cSceneTreeNode::mCuller = 0;
eSceneNodeType cSceneTreeNode::mPickType = SCENENODE_NULL;
NiVisibleArray* cSceneTreeNode::mSolidArray = 0;
NiVisibleArray* cSceneTreeNode::mAlphaArray = 0;
NiVisibleArray* cSceneTreeNode::mAlphaTestArray = 0;

/// 충돌 검사용
const cRay* cSceneTreeNode::mRay = 0;
float cSceneTreeNode::mMaxDistance = 0;
const cSphere* cSceneTreeNode::mSphere = 0;
tArray<cSceneNode*>* cSceneTreeNode::mCollideArray = 0;

cSceneTreeNode::cSceneTreeNode( cSceneTree* tree, cSceneTreeNode* parent, const NiPoint3& center, float radius )
: cSphere( center, radius )
, mTree( tree )
, mParent( parent )
{
	mChild[0] = mChild[1] = mChild[2] = mChild[3] = 0;

	float d = ( radius / 1.4142135623731f ) * 0.5f;
	float r = radius * 0.55f;

	mTestSpheres[0].Set( center.x - d, center.y - d, 0.0f, r );
	mTestSpheres[1].Set( center.x + d, center.y - d, 0.0f, r );
	mTestSpheres[2].Set( center.x - d, center.y + d, 0.0f, r );
	mTestSpheres[3].Set( center.x + d, center.y + d, 0.0f, r );
}

cSceneTreeNode::cSceneTreeNode( cSceneTree* tree, cSceneTreeNode* parent, const cSphere& sphere )
: cSphere( sphere )
, mTree( tree )
, mParent( parent )
{
	mChild[0] = mChild[1] = mChild[2] = mChild[3] = 0;

	float d = ( mRadius / 1.4142135623731f ) * 0.5f;
	float r = mRadius * 0.55f;

	mTestSpheres[0].Set( mCenter.x - d, mCenter.y - d, 0.0f, r );
	mTestSpheres[1].Set( mCenter.x + d, mCenter.y - d, 0.0f, r );
	mTestSpheres[2].Set( mCenter.x - d, mCenter.y + d, 0.0f, r );
	mTestSpheres[3].Set( mCenter.x + d, mCenter.y + d, 0.0f, r );
}

cSceneTreeNode::~cSceneTreeNode()
{
	delete mChild[0];
	delete mChild[1];
	delete mChild[2];
	delete mChild[3];
}

void* cSceneTreeNode::operator new( size_t /*n*/ )
{ 
	return mPool.Alloc();
}

void cSceneTreeNode::operator delete( void* p )
{
	mPool.Free( (cSceneTreeNode*)p );
}

void cSceneTreeNode::Clear()
{
	mObjectList.Clear();

	if( mChild[0] )
		mChild[0]->Clear();
	if( mChild[1] )
		mChild[1]->Clear();
	if( mChild[2] )
		mChild[2]->Clear();
	if( mChild[3] )
		mChild[3]->Clear();
}

void cSceneTreeNode::Push( cSceneNode* node )
{
	assert( node );

	/// 말단 노드인 경우 장면 노드를 추가하고 바로 리턴
	if( mRadius <= mTree->GetMinRadius() )
	{
		node->mContainer = this;
		node->mIteratorByContainer = mObjectList.Insert( mObjectList.End(), node );
		return;
	}

	/// 장면 노드가 트리 노드보다 더 크면 상위 노드로 장면 노드를 전달
	if( node->GetRadius() > mRadius )
	{
		if( mParent )
		{
			mParent->Push( node );
		}
		else
		{
			/// 루트 노드에 추가
			node->mContainer = this;
			node->mIteratorByContainer = mObjectList.Insert( mObjectList.End(), node );
		}
		return;
	}

	/// 자식 노드들에 대해 장면 노드 포함 여부를 검사
	int i = 0;
	int count = 0;
//	const cSphere& sphere = node->GetBoundSphere();
	cSphere sphere( node->GetCenter(), node->GetRadius() );

	mTestSpheres[0].mResult = false;
	mTestSpheres[1].mResult = false;
	mTestSpheres[2].mResult = false;
	mTestSpheres[3].mResult = false;

	if( mTestSpheres[0].ContainSphere( sphere ) )
	{
		i = 0;
		mTestSpheres[0].mResult = true;
		++count;
	}
	if( mTestSpheres[1].ContainSphere( sphere ) )
	{
		i = 1;
		mTestSpheres[1].mResult = true;
		++count;
	}
	if( mTestSpheres[2].ContainSphere( sphere ) )
	{
		i = 2;
		mTestSpheres[2].mResult = true;
		++count;
	}
	if( mTestSpheres[3].ContainSphere( sphere ) )
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
			node->mIteratorByContainer = mObjectList.Insert( mObjectList.End(), node );
		}
		break;
	case 1:
		{
			/// 해당 자식 노드에 장면 노드를 전달
			cSceneTreeNode* child = mChild[i];

			if( child == 0 )
			{
				child = mChild[i] = new cSceneTreeNode( mTree, this, mTestSpheres[i] );
			}
			child->Push( node );
		}
		break;
	case 2:
	case 3:
	case 4:
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

			cSceneTreeNode* child = mChild[i];

			if( child == 0 )
			{
				child = mChild[i] = new cSceneTreeNode( mTree, this, mTestSpheres[i] );
			}
			child->Push( node );
		}
		break;
	}
}

void cSceneTreeNode::Remove( cSceneNode* node )
{
	assert( node );

	mObjectList.Erase( node->mIteratorByContainer );

	node->mIteratorByContainer = mObjectList.End();
}

void cSceneTreeNode::Update( cSceneNode* node )
{
	assert( node );

	/// 장면 노드가 속한 트리 노드에 대해 포함 여부를 검사
	const cSphere& sphere = node->GetBoundSphere();

	if( ContainSphere( sphere ) == false )
	{
		/// 장면 노드를 트리 노드에서 제거하고 상위 트리 노드에 대해 포함 유무를 검사
		Remove( node );

		if( mParent )
		{
			if( mParent->ContainSphere( sphere ) )
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
		if( mFrustum->IsPlaneActive(NiFrustumPlanes::FAR_PLANE) )
		{
			side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::FAR_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				// The object is not visible since it is on the negative
				// side of the plane.
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				// The object is fully on the positive side of the plane,
				// so there is no need to compare child objects to this plane.
				mFrustum->DisablePlane(NiFrustumPlanes::FAR_PLANE);
				++c;
			}
		}
		if( mFrustum->IsPlaneActive(NiFrustumPlanes::LEFT_PLANE) )
		{
			side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::LEFT_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				mFrustum->DisablePlane(NiFrustumPlanes::LEFT_PLANE);
				++c;
			}
		}
		if( mFrustum->IsPlaneActive(NiFrustumPlanes::RIGHT_PLANE) )
		{
			side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::RIGHT_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				mFrustum->DisablePlane(NiFrustumPlanes::RIGHT_PLANE);
				++c;
			}
		}
		if( mFrustum->IsPlaneActive(NiFrustumPlanes::TOP_PLANE) )
		{
			side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::TOP_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				mFrustum->DisablePlane(NiFrustumPlanes::TOP_PLANE);
				++c;
			}
		}
		if( mFrustum->IsPlaneActive(NiFrustumPlanes::BOTTOM_PLANE) )
		{
			side = WhichSide( mFrustum->GetPlane(NiFrustumPlanes::BOTTOM_PLANE) );

			if( side == NiPlane::NEGATIVE_SIDE )
			{
				mFrustum->SetActivePlaneState(saveActive);
				return;
			}
			if( side == NiPlane::POSITIVE_SIDE )
			{
				mFrustum->DisablePlane(NiFrustumPlanes::BOTTOM_PLANE);
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
		/// 교차 -> 컬링 검사를 계속
		/// 포함된 개체들에 대해 검사
		if( mObjectList.IsEmpty() == false )
		{
			cObjectList::cIterator i = mObjectList.Begin();
			cObjectList::cIterator end = mObjectList.End();

			for( ; i != end; ++i )
			{
				cSceneNode* n = *i;

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

				if( n->OnVisible( *mCuller ) )
					n->AddToVisibleArray( mSolidArray, mAlphaArray, mAlphaTestArray );
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
}

void cSceneTreeNode::AddToVisibleArray() const
{
	/// 포함된 개체들을 추가
	cObjectList::cConstIterator i = mObjectList.Begin();
	cObjectList::cConstIterator end = mObjectList.End();

	for( ; i != end; ++i )
	{
		cSceneNode* n = *i;

		if( n->IsKindof( mPickType ) == false )
			continue;

		if( n->OnVisible( *mCuller ) )
			n->AddToVisibleArray( mSolidArray, mAlphaArray, mAlphaTestArray );
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

bool cSceneTreeNode::CollideRay()
{
	bool ret = false;

	if( IntersectRay( *mRay, mMaxDistance ) )
	{
		/// 포함된 장면 노드들에 대해 교차 검사
		if( mObjectList.IsEmpty() == false )
		{
			cObjectList::cIterator i = mObjectList.Begin();
			cObjectList::cIterator end = mObjectList.End();

			for( ; i != end; ++i )
			{
				cSceneNode* n = *i;

				if( n->IsKindof( mPickType ) == false )
					continue;

				bool pick =  n->Pick( *mRay );
				bool dist = n->GetPickDistance() < mMaxDistance;

				if( pick && dist )
				{
					mCollideArray->PushBack( n );
					ret = true;
				}
			}
		}

		/// 자식들에 대해 교차 검사
		if( mChild[0] )
			ret |= mChild[0]->CollideRay();
		if( mChild[1] )
			ret |= mChild[1]->CollideRay();
		if( mChild[2] )
			ret |= mChild[2]->CollideRay();
		if( mChild[3] )
			ret |= mChild[3]->CollideRay();
	}
	return ret;
}

bool cSceneTreeNode::CollideSphere()
{
	bool ret = false;

	if( IntersectSphere( *mSphere ) )
	{
		/// 포함된 장면 노드들에 대해 교차 검사
		if( mObjectList.IsEmpty() == false )
		{
			cObjectList::cIterator i = mObjectList.Begin();
			cObjectList::cIterator end = mObjectList.End();

			for( ; i != end; ++i )
			{
				cSceneNode* n = *i;

				if( n->IsKindof( mPickType ) == false )
					continue;

				//if( n->GetBoundSphere().IntersectSphere( *mSphere ) )
				if( cSphere(n->GetCenter(), n->GetRadius()).IntersectSphere( *mSphere ) )
				{
					mCollideArray->PushBack( n );
					ret = true;
				}
			}
		}

		/// 자식들에 대해 교차 검사
		if( mChild[0] )
			ret |= mChild[0]->CollideSphere();
		if( mChild[1] )
			ret |= mChild[1]->CollideSphere();
		if( mChild[2] )
			ret |= mChild[2]->CollideSphere();
		if( mChild[3] )
			ret |= mChild[3]->CollideSphere();
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

void cSceneTree::Cull( NiVisibleArray* solidArray, NiVisibleArray* alphaArray, NiVisibleArray* alphaTestArray, NiFrustumPlanes& frustum, cSceneCuller& culler, eSceneNodeType type )
{
	assert( solidArray );
	assert( alphaArray );
	assert( alphaTestArray );
	assert( mRootNode );

	frustum.EnableAllPlanes();
	cSceneTreeNode::mFrustum = &frustum;
	cSceneTreeNode::mCuller = &culler;
	cSceneTreeNode::mPickType = type;
	cSceneTreeNode::mSolidArray = solidArray;
	cSceneTreeNode::mAlphaArray = alphaArray;
	cSceneTreeNode::mAlphaTestArray = alphaTestArray;
	mRootNode->Cull();
}

bool cSceneTree::CollideRay( tArray<cSceneNode*>* collideArray, const cRay& ray, float maxDistance, bool sortByDistance, eSceneNodeType type )
{
	assert( collideArray );
	assert( mRootNode );

	cSceneTreeNode::mRay = &ray;
	cSceneTreeNode::mMaxDistance = maxDistance;
	cSceneTreeNode::mPickType = type;
	cSceneTreeNode::mCollideArray = collideArray;
	bool ret = mRootNode->CollideRay();

	if( ret && sortByDistance && collideArray->GetSize() > 1 )
	{
		::Sort( collideArray->Begin(), collideArray->End(), cSceneNodeCompareForPicking() );
	}
	return ret;
}

bool cSceneTree::CollideSphere( tArray<cSceneNode*>* collideArray, const cSphere& sphere, eSceneNodeType type )
{
	assert( collideArray );
	assert( mRootNode );

	cSceneTreeNode::mSphere = &sphere;
	cSceneTreeNode::mPickType = type;
	cSceneTreeNode::mCollideArray = collideArray;
	return mRootNode->CollideSphere();
}
