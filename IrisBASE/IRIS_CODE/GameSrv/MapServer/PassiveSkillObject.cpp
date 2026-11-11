
#include "stdafx.h"
#include "PassiveSkillObject.h"
#include "SkillManager.h"

cPassiveSkillObject::cPassiveSkillObject()
{
}

cPassiveSkillObject::~cPassiveSkillObject()
{
}

void* cPassiveSkillObject::operator new( size_t n )
{
	if(n != sizeof(cPassiveSkillObject) ) 
	{
		assert(0);
		return NULL;
	}

	return SKILLMANAGER->AllocSkillObject();
}

void cPassiveSkillObject::operator delete( void* ptr, size_t n )
{
	/// NULL 포인터 검사
	if( ptr == 0)
	{
		assert(0);
		return;
	}

	if(n != sizeof(cPassiveSkillObject) ) 
	{
		assert(0);
		return;
	}

	SKILLMANAGER->FreeSkillObject( static_cast<cPassiveSkillObject*>(ptr) );

	return;
}

void cPassiveSkillObject::Process()
{
}



