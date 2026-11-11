/* ====================================================================
* 파    일 :	Object.h
* 목    적 :	관리를 위한 기본 게임 객체
* 작 성 자 :	정성대
* 작 성 일 :	2006.08.29
*
* 추가내용 :
*		추가날짜	작업자	추가내용
*		2006.08.29	정성대	생성
* 
* 주의사항 :	
* =================================================================== */
#pragma once

/// 오브젝트
/// 게임 내 모든 오브젝트들의 최상위 클래스
class cObject
{
public:

	//struct sObjectPosition
	//{
	//	float X;
	//	float Y;
	//};

public:
	/// 아이디를 설정
	void SetID( unsigned int id );

	/// 아이디를 리턴
	unsigned int GetID() const;

	/// 위치를 설정
	//void SetPos( float x, float y ) const;

	/// 위치를 리턴
	//float GetXPos() const;
	//float GetYPos() const;

protected:
	cObject();
	virtual ~cObject();

protected:
	/// 오브젝트 아이디
	unsigned int mObjectID;

	///
	//sObjectPosition mPos;
};

inline unsigned int	cObject::GetID() const
{
	return mObjectID;
}

inline void cObject::SetID( unsigned int id )
{
	mObjectID = id;
}

//inline float cObject::GetXPos()
//{
//	return mPos.X;
//}
//
//inline float cObject::GetYPos()					{ return mPos.Y; }
//inline void cObject::SetPos( float x, float y )	{ mPos.X = x; mPos.Y = y; }
