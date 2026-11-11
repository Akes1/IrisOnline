#include "stdafx.h"
#include "Object.h"

#include "ObjectManager.h"

cObject::cObject( unsigned char type, cObjectManager* manager )
{
	assert(manager);
	mManager = manager;

	mType = type;

	mTrans.m_Translate = NiPoint3::ZERO;
	mTrans.m_Rotate = NiMatrix3::IDENTITY;
	mTrans.m_fScale = 1.0f;

	mTarget.type = eOBJECTTYPE_NONE;
	mTarget.index = 0;
}

cObject::~cObject()
{
}

cObject* cObject::GetTargetObject()
{
	return mManager->GetObject( mTarget.type, mTarget.index );
}


