#include "stdafx.h"
#include "Lines.h"

#include "Camera.h"
#include "SceneNode.h"

cCircleLines::cCircleLines( const NiColorA& color )
{
	mLines = NiNew NiMesh;
	mLines->SetPrimitiveType(NiPrimitiveType::PRIMITIVE_LINES);

	mLines->AddStream( 
		NiCommonSemantics::POSITION(), 0, NiDataStreamElement::F_FLOAT32_3, 40, 
		NiDataStream::ACCESS_CPU_WRITE_VOLATILE | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	mLines->AddStream( 
		NiCommonSemantics::COLOR(), 0, NiDataStreamElement::F_FLOAT32_4, 40, 
		NiDataStream::ACCESS_CPU_READ | NiDataStream::ACCESS_CPU_WRITE_STATIC | NiDataStream::ACCESS_GPU_READ,
		NiDataStream::USAGE_VERTEX );

	NiDataStreamElementLock kColorLock( mLines, NiCommonSemantics::COLOR(), 0,
		NiDataStreamElement::F_FLOAT32_4, NiDataStream::LOCK_WRITE );
	assert( kColorLock.IsLocked() );
	NiTStridedRandomAccessIterator<NiColorA> kColorIter =  kColorLock.begin<NiColorA>();

	for( unsigned int i = 0; i < 40; i += 2 )
	{
		kColorIter[i] = NiColorA::BLACK;
		kColorIter[i+1] = color;
	}

	kColorLock.Unlock();

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

void cCircleLines::Render( cSceneNode* n, cCamera* cam )
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

	mLines->RenderImmediate( NiRenderer::GetRenderer() );
}
