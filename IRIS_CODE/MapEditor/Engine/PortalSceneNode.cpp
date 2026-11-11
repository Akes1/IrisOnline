#include "StdAfx.h"
#include "PortalSceneNode.h"

#include "NaviMesh.h"
#include "NifAnimationInfo.h"


cPortalSceneNode::cPortalSceneNode( eSceneNodeType type )
: cStaticSceneNode( type )
{
	mNameHeight = 0;
}

bool cPortalSceneNode::Init( cPortalSceneNodeParam& param )
{
	mNameHeight = param.mNameHeight;

	if( cStaticSceneNode::Init( param ) == false )
		return false;

	// 생성
	mNameLine = NiNew NiMesh;
	mNameLine->SetPrimitiveType( NiPrimitiveType::PRIMITIVE_TRISTRIPS );

	mNameLine->AddStream( NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, 4,
							NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ, 
							NiDataStream::USAGE_VERTEX );

	NiMaterialProperty* matProp = NiNew NiMaterialProperty;
	matProp->SetAmbientColor( NiColor::BLACK );
	matProp->SetDiffuseColor( NiColor::BLACK );
	matProp->SetSpecularColor( NiColor::BLACK );
	matProp->SetEmittance( NiColor( 0.5f, 0.0f, 1.0f ) );

	NiVertexColorProperty* colorProp = NiNew NiVertexColorProperty;
	colorProp->SetSourceMode( NiVertexColorProperty::SOURCE_IGNORE );
	colorProp->SetLightingMode( NiVertexColorProperty::LIGHTING_E_A_D );

	NiStencilProperty* stencil = NiNew NiStencilProperty;
	stencil->SetDrawMode( NiStencilProperty::DRAW_BOTH );
	stencil->SetStencilOn( false );

	mNameLine->AttachProperty( matProp );
	mNameLine->AttachProperty( colorProp );
	mNameLine->AttachProperty( stencil );

	mNameLine->UpdateProperties();
	mNameLine->Update( 0.0f );

	CalcRect();
	return true;
}

void cPortalSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
	if( mNeedUpdateTransform )
		CalcRect();

	ProcessVisibleLevel( deltaTime );
	if( mCheckFrustum == false )
		return;

	/// 반복 동작을 가진 경우 엔진 노드를 처리
	if( mNifAniInfo )
	{
		mNeedUpdateTransform = true;
		mNifAniInfo->Update( deltaTime, accumTime );
	}

	cSceneNode::Process( deltaTime, accumTime );
}

void cPortalSceneNode::RenderRect()
{
	if( mNameLine )
		mNameLine->RenderImmediate( NiRenderer::GetRenderer() );
}

void cPortalSceneNode::CalcRect()
{
	if( mNameLine == 0 )
		return;

	NiPoint3 pos = GetTranslate();
	NiMatrix3 rot = GetRotate();

	/// 회전
	NiPoint3 posDir = NiPoint3::UNIT_Y;
	posDir = rot * posDir;
	posDir.Unitize();
	pos = pos + posDir;

	//
	NiPoint3 rect[4];
	for( unsigned int i=0;i<4;i++ )
		rect[i] = NiPoint3::ZERO;

	/// 렉트 계산
	rect[0].x = -0.5f * POTALLINE_WIDTH;
	rect[0].z = -0.5f * POTALLINE_HEIGHT;
	rect[0].y = 0.0f;

	rect[1].x = 0.5f * POTALLINE_WIDTH;
	rect[1].z = -0.5f * POTALLINE_HEIGHT;
	rect[1].y = 0.0f;

	rect[2].x = 0.5f * POTALLINE_WIDTH;
	rect[2].z = 0.5f * POTALLINE_HEIGHT;
	rect[2].y = 0.0f;

	rect[3].x = -0.5f * POTALLINE_WIDTH;
	rect[3].z = 0.5f * POTALLINE_HEIGHT;
	rect[3].y = 0.0f;

	for( unsigned int i=0; i<4; ++i )
	{
		rect[i] = rot * rect[i];

		rect[i] += pos;
		rect[i].z += mNameHeight;
	}

	NiDataStreamElementLock posLock( mNameLine, NiCommonSemantics::POSITION(), 0, 
									NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( posLock.IsLocked() );

	NiTStridedRandomAccessIterator<NiPoint3> posIter = posLock.begin<NiPoint3>();

	posIter[0] = rect[1]; 
	posIter[1] = rect[0];
	posIter[2] = rect[2];
	posIter[3] = rect[3];

	NiDataStreamRef* ref = mNameLine->FindStreamRef( NiCommonSemantics::POSITION() );
	NIASSERT( ref );
	if( ref )
		ref->SetActiveCount( 0, 4 );

}

void cPortalSceneNode::SetNameHeight( float height )
{
	mNameHeight = height;

	mNeedUpdateTransform = true;
}

//////////////////////////////////////////////////////////////////////////
cDoorSceneNode::cDoorSceneNode( eSceneNodeType type )
: cStaticSceneNode( type )
, mAppendZ(0.0f)
{
	mRectTopWidth = mRectBottomWidth = mRectLeftHeight = mRectRightHeight = 10.0f;
	mRectAppendY = 0.0f;

	for( unsigned int i=0;i<4;i++ )
		mRectPos[i] = NiPoint3::ZERO;
}

bool cDoorSceneNode::Init( cDoorSceneNodeParam& param )
{
	mAppendZ = param.mAppendZ;
	param.mTranslate.z += mAppendZ;

	mRectTopWidth = param.mRectTopWidth;
	mRectBottomWidth = param.mRectBottomWidth;
	mRectLeftHeight = param.mRectLeftHeight;
	mRectRightHeight = param.mRectRightHeight;

	if( cStaticSceneNode::Init( param ) == false )
		return false;


	/// 경로 만들기
	mLimitRect = NiNew NiMesh;
	mLimitRect->SetPrimitiveType( NiPrimitiveType::PRIMITIVE_TRISTRIPS );

	mLimitRect->AddStream( NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, 4,
		NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ, 
		NiDataStream::USAGE_VERTEX );

	NiMaterialProperty* matProp = NiNew NiMaterialProperty;
	matProp->SetAmbientColor( NiColor::BLACK );
	matProp->SetDiffuseColor( NiColor::BLACK );
	matProp->SetSpecularColor( NiColor::BLACK );
	matProp->SetEmittance( NiColor( 1.0f, 0.0f, 0.0f ) );

	NiVertexColorProperty* colorProp = NiNew NiVertexColorProperty;
	colorProp->SetSourceMode( NiVertexColorProperty::SOURCE_IGNORE );
	colorProp->SetLightingMode( NiVertexColorProperty::LIGHTING_E_A_D );
	
	NiStencilProperty* stencil = NiNew NiStencilProperty;
	stencil->SetDrawMode( NiStencilProperty::DRAW_BOTH );
	stencil->SetStencilOn( false );

	mLimitRect->AttachProperty( matProp );
	mLimitRect->AttachProperty( colorProp );
	mLimitRect->AttachProperty( stencil );

	mLimitRect->UpdateProperties();
	mLimitRect->Update( 0.0f );

	CalcRect();

	return true;
}

void cDoorSceneNode::Process( unsigned long deltaTime, unsigned long accumTime )
{
	if( mNeedUpdateTransform )
		CalcRect();

	ProcessVisibleLevel( deltaTime );
	if( mCheckFrustum == false )
		return;

	/// 반복 동작을 가진 경우 엔진 노드를 처리
	if( mNifAniInfo )
	{
		mNeedUpdateTransform = true;
		mNifAniInfo->Update( deltaTime, accumTime );
	}

	cSceneNode::Process( deltaTime, accumTime );
}

void cDoorSceneNode::SetTranslate( const NiPoint3& trans )
{
	NiPoint3 pos = trans;
	pos.z += mAppendZ;
	GetNiObj()->SetTranslate( pos );

	mNeedUpdateTransform = true;
}

void cDoorSceneNode::RenderRect()
{
	if( mLimitRect )
		mLimitRect->RenderImmediate( NiRenderer::GetRenderer() );
}

void cDoorSceneNode::CalcRect()
{
	if( mLimitRect == 0 )
		return;

	NiPoint3 pos = GetTranslate();
	NiMatrix3 rot = GetRotate();

	/// position calc
	NiPoint3 posDir = NiPoint3::UNIT_Y;
	posDir = rot*posDir;
	posDir.Unitize();
	pos = pos + posDir*mRectAppendY;

	/// rect calc
	NiPoint3 rect[4];
	mRectPos[0].x = -0.5f * mRectTopWidth;
	mRectPos[0].y = -0.5f * mRectLeftHeight;
	mRectPos[0].z = 0.0f;

	mRectPos[1].x = 0.5f * mRectTopWidth;
	mRectPos[1].y = -0.5f * mRectRightHeight;
	mRectPos[1].z = 0.0f;

	mRectPos[2].x = 0.5f * mRectBottomWidth;
	mRectPos[2].y = 0.5f * mRectRightHeight;
	mRectPos[2].z = 0.0f;

	mRectPos[3].x = -0.5f * mRectBottomWidth;
	mRectPos[3].y = 0.5f * mRectLeftHeight;
	mRectPos[3].z = 0.0f;

	for( unsigned int i=0; i<4; ++i )
	{
		/// rot
		mRectPos[i] = rot * mRectPos[i];

		mRectPos[i] += pos;
		mRectPos[i].z += 100.0f;
	}

	NiDataStreamElementLock posLock( mLimitRect, NiCommonSemantics::POSITION(), 0, 
		NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( posLock.IsLocked() );

	NiTStridedRandomAccessIterator<NiPoint3> posIter = posLock.begin<NiPoint3>();

	posIter[0] = mRectPos[1]; 
	posIter[1] = mRectPos[0];
	posIter[2] = mRectPos[2];
	posIter[3] = mRectPos[3];

	NiDataStreamRef* ref = mLimitRect->FindStreamRef( NiCommonSemantics::POSITION() );
	NIASSERT( ref );
	if( ref )
		ref->SetActiveCount( 0, 4 );
}

void cDoorSceneNode::GetRectInfo( float& topWidth, float& bottomWidth, float& leftHeight, float& rightHeight )
{
	topWidth = mRectTopWidth;
	bottomWidth = mRectBottomWidth;

	leftHeight = mRectLeftHeight;
	rightHeight = mRectRightHeight;
}

void cDoorSceneNode::SetRectInfo( float topWidth, float bottomWidth, float leftHeight, float rightHeight )
{
	mRectTopWidth = topWidth;
	mRectBottomWidth = bottomWidth;

	mRectLeftHeight = leftHeight;
	mRectRightHeight = rightHeight;

	mNeedUpdateTransform = true;
}

void cDoorSceneNode::SetRectAppendY( float appendY )
{
	mRectAppendY = appendY;

	mNeedUpdateTransform = true;
}
