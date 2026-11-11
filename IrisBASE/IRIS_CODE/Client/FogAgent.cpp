#include "stdafx.h"
#include "FogAgent.h"

cFogAgent* cFogAgent::mpFogAgent = 0;

cFogAgent::cFogAgent()
{
	mpFogAgent = this;

	mpFogProp = NiNew NiFogProperty;
	mpFogProp->SetFog( true );
	mpFogProp->SetFogColor( NiColor::WHITE );
	mpFogProp->SetDepth( 1.0f );
	mpFogProp->SetFogFunction( NiFogProperty::FOG_Z_LINEAR );
}

cFogAgent::~cFogAgent()
{
	assert(mpFogProp->GetRefCount()==1);
	mpFogProp = 0;
}
//
//void cFogAgent::AttachFogProperty( NiAVObject* pobj )
//{
//	if( !pobj )
//	{
//		return;
//	}
//
//	pobj->AttachProperty( mpFogProp );
//}

//void cFogAgent::SetFog( const NiColor& color, float depth )
//{
//	mpFogProp->SetFogColor( color );
//	mpFogProp->SetDepth( depth );
//}

void cFogAgent::SetFogColor( const NiColor& color )
{
	mpFogProp->SetFogColor( color );
}

void cFogAgent::SetFogDepth( float depth )
{
	mpFogProp->SetDepth( depth );
}
