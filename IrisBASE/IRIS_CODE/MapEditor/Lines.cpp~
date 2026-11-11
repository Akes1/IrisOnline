#include "stdafx.h"
#include "Lines.h"

#include "Engine/Ray.h"
#include "Engine/Camera.h"
#include "Engine/SceneNode.h"
#include "Engine/Terrain.h"
#include "Engine/NaviMesh.h"
#include "Engine/NaviField.h"

cCircleLines::cCircleLines( const NiColorA& /*color0*/, const NiColorA& /*color1*/ )
{
	mLines = NiNew NiMesh;
	mLines->SetPrimitiveType(NiPrimitiveType::PRIMITIVE_LINES);

	mLines->AddStream( 
		NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, 40, 
		NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	//mLines->AddStream( 
	//	NiCommonSemantics::COLOR(), 0, NiDataStreamElement::F_FLOAT32_4, 40, 
	//	NiDataStream::ACCESS_CPU_READ | NiDataStream::ACCESS_CPU_WRITE_STATIC | NiDataStream::ACCESS_GPU_READ,
	//	NiDataStream::USAGE_VERTEX );

	//NiDataStreamElementLock kColorLock( mLines, NiCommonSemantics::COLOR(), 0,
	//	NiDataStreamElement::F_FLOAT32_4, NiDataStream::LOCK_WRITE );
	//assert( kColorLock.IsLocked() );
	//NiTStridedRandomAccessIterator<NiColorA> kColorIter =  kColorLock.begin<NiColorA>();

	//for( unsigned int i = 0; i < 40; i += 2 )
	//{
	//	kColorIter[i] = color0;
	//	kColorIter[i+1] = color1;
	//}

	//kColorLock.Unlock();

	SetActiveCount( 40 );

	mMatProp = NiNew NiMaterialProperty;
	mMatProp->SetAmbientColor( NiColor::WHITE );
	mMatProp->SetDiffuseColor( NiColor::WHITE );
	mMatProp->SetSpecularColor( NiColor::WHITE );
	mMatProp->SetEmittance( NiColor::WHITE );

	mVertColorProp = NiNew NiVertexColorProperty;
	mVertColorProp->SetSourceMode( NiVertexColorProperty::SOURCE_EMISSIVE );
	mVertColorProp->SetLightingMode( NiVertexColorProperty::LIGHTING_E );

	mWireProp = NiNew NiWireframeProperty;
	mWireProp->SetWireframe( false );

	mLines->AttachProperty( mMatProp );
	mLines->AttachProperty( mVertColorProp );
	mLines->AttachProperty( mWireProp );

	mLines->UpdateProperties();
	mLines->Update( 0.0f );

}

void cCircleLines::Process( const NiPoint3& center, float radius, cTerrain* terrain )
{
	assert( terrain );

	float x, y;
	NiPoint3 org;
	NiPoint3 pos;

	NiDataStreamElementLock kPositionLock( mLines, NiCommonSemantics::POSITION(), 0,
		NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( kPositionLock.IsLocked() );

	NiTStridedRandomAccessIterator<NiPoint3> kPositionIter =  kPositionLock.begin<NiPoint3>();

	for( unsigned int i = 0, ang = 0; ang < 360; ++i, ang += 18 )
	{
		x = NiCos( ang * NI_PI / 180.0f );
		y = NiSin( ang * NI_PI / 180.0f );

		pos.x = org.x = center.x + x * radius;
		pos.y = org.y = center.y + y * radius;
		org.z = 1000000.0f;
		pos.z = 0.0f;

		terrain->Pick( &pos, cRay(org, -NiPoint3::UNIT_Z) );

		kPositionIter[(2*i)+1] = kPositionIter[(2*(i+1))%40] = NiPoint3(pos.x, pos.y, pos.z+20.0f);
	}

	kPositionLock.Unlock();

	SetActiveCount( 40 );
}

void cCircleLines::Process( const NiPoint3& center, float radius, cNaviMesh* naviMesh )
{
	assert( naviMesh );

	float x, y;
	NiPoint3 org;
	NiPoint3 pos;

	NiDataStreamElementLock kPositionLock( mLines, NiCommonSemantics::POSITION(), 0,
		NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( kPositionLock.IsLocked() );

	NiTStridedRandomAccessIterator<NiPoint3> kPositionIter =  kPositionLock.begin<NiPoint3>();

	for( unsigned int i = 0, ang = 0; ang < 360; ++i, ang += 18 )
	{
		x = NiCos( ang * NI_PI / 180.0f );
		y = NiSin( ang * NI_PI / 180.0f );

		pos.x = org.x = center.x + x * radius;
		pos.y = org.y = center.y + y * radius;
		org.z = 1000000.0f;
		pos.z = 0.0f;

		naviMesh->Pick( &pos, cRay(org, -NiPoint3::UNIT_Z) );

		kPositionIter[(2*i)+1] = kPositionIter[(2*(i+1))%40] = NiPoint3(pos.x, pos.y, pos.z+100.0f);
	}

	kPositionLock.Unlock();

	SetActiveCount( 40 );
}

void cCircleLines::Process( const NiPoint3& center, float radius, cNaviField* naviField )
{
	assert( naviField );

	float x, y;
	NiPoint3 org;
	NiPoint3 pos;

	NiDataStreamElementLock kPositionLock( mLines, NiCommonSemantics::POSITION(), 0,
		NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( kPositionLock.IsLocked() );

	NiTStridedRandomAccessIterator<NiPoint3> kPositionIter =  kPositionLock.begin<NiPoint3>();

	for( unsigned int i = 0, ang = 0; ang < 360; ++i, ang += 18 )
	{
		x = NiCos( ang * NI_PI / 180.0f );
		y = NiSin( ang * NI_PI / 180.0f );

		pos.x = org.x = center.x + x * radius;
		pos.y = org.y = center.y + y * radius;
		org.z = 1000000.0f;
		pos.z = 0.0f;

		naviField->Pick( &pos, cRay(org, -NiPoint3::UNIT_Z) );

		kPositionIter[(2*i)+1] = kPositionIter[(2*(i+1))%40] = NiPoint3(pos.x, pos.y, pos.z+100.0f);
	}

	kPositionLock.Unlock();

	SetActiveCount( 40 );
}

void cCircleLines::Process( cSceneNode* n, cCamera* cam )
{
	NiPoint3 right = cam->GetWorldRightVector();
	NiPoint3 up = cam->GetWorldUpVector();

	const NiPoint3& c = n->GetCenter();
	float r = n->GetRadius();
	float x, y;
	NiPoint3 dir;

	NiDataStreamElementLock kPositionLock( mLines, NiCommonSemantics::POSITION(), 0,
		NiDataStreamElement::F_FLOAT32_3, NiDataStream::LOCK_WRITE );
	assert( kPositionLock.IsLocked() );

	NiTStridedRandomAccessIterator<NiPoint3> kPositionIter =  kPositionLock.begin<NiPoint3>();

	for( unsigned int i = 0, ang = 0; ang < 360; ++i, ang += 18 )
	{
		x = NiCos( ang * NI_PI / 180.0f );
		y = NiSin( ang * NI_PI / 180.0f );

		dir = right * x + up * y;

		kPositionIter[(2*i)+1] = kPositionIter[(2*(i+1))%40] = c + dir * r;
	}

	kPositionLock.Unlock();

	SetActiveCount( 40 );
}

void cCircleLines::Render()
{
	mLines->RenderImmediate( NiRenderer::GetRenderer() );
}

void cCircleLines::SetActiveCount( unsigned int count )
{
	NiDataStreamRef* pkRef = mLines->FindStreamRef( NiCommonSemantics::POSITION() );
	NIASSERT(pkRef);
	pkRef->SetActiveCount(0, count);

	//pkRef = mLines->FindStreamRef( NiCommonSemantics::COLOR() );
	//NIASSERT(pkRef);
	//pkRef->SetActiveCount(0, count);
}
