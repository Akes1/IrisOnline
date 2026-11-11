/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.05
* 내    용 : 실행
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 실행 타입
enum eDoingType
{
	eDOING_NULL				= 0,
	eDOING_TERRAIN			= (1 <<  0),
	eDOING_OBJECT			= (1 <<  1),
	eDOING_NAVIMESH			= (1 <<  2),
	eDOING_NAVIFIELD		= (1 <<  3),
	eDOING_TERRAIN_BUILD	= (1 <<  4)	| eDOING_TERRAIN,
	eDOING_TERRAIN_PAINT	= (1 <<  5)	| eDOING_TERRAIN,
	eDOING_TERRAIN_DETAIL	= (1 <<  6) | eDOING_TERRAIN,
	eDOING_OBJECT_CREATE	= (1 <<  7)	| eDOING_OBJECT,
	eDOING_OBJECT_SELECT	= (1 <<  8)	| eDOING_OBJECT,
	eDOING_OBJECT_DELETE	= (1 <<  9)	| eDOING_OBJECT,
	eDOING_OBJECT_COPY		= (1 << 10)	| eDOING_OBJECT,
	eDOING_OBJECT_TRANSLATE	= (1 << 11)	| eDOING_OBJECT,
	eDOING_OBJECT_ROTATE	= (1 << 12)	| eDOING_OBJECT,
	eDOING_OBJECT_SCALE		= (1 << 13)	| eDOING_OBJECT,
	eDOING_NAVIMESH_BUILD	= (1 << 14)	| eDOING_NAVIMESH,
	eDOING_NAVIFIELD_PAINT	= (1 << 15)	| eDOING_NAVIFIELD,
	eDOING_FORCE_32BIT		= 0x7FFFFFFF
};

/// 실행
class cDoing
{
	friend class cDoingManager;

protected:
	cDoing( eDoingType type = eDOING_NULL );
	virtual ~cDoing();

	virtual void Undo() = 0;
	virtual void Redo() = 0;

protected:
	/// 타입
	eDoingType mType;
};

inline
cDoing::cDoing( eDoingType type )
: mType( type )
{
}

inline
cDoing::~cDoing()
{
}
