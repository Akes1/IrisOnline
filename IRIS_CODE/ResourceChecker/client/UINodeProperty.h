/* ==========================================================================
* 파    일 : UINodeProperty.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-13
* 주의사항 :
*===========================================================================*/
#pragma once

#include "UINode.h"

class cToken;
class cParser;

class cUINodeProperty
{
public:
	cUINodeProperty( const cString& name, eUINodeType type = eUINODE_NULL );
	virtual ~cUINodeProperty();

	/// 속성 로딩
	bool Load( cParser& parser );

	/// 유효성 검사 
	virtual bool IsValid() const;

	/// 자식 추가 
	void AddChild( cUINodeProperty* child );

	/// 타입
	eUINodeType GetType() const;

	/// 자식 리스트
//	const tPointerList<cUINodeProperty*>& GetChildList() const;
	const tPointerList<void*>& GetChildList() const;

protected:
	/// 한 줄 파싱
	bool ParseLine( cParser& parser, cToken& token );

protected:
	/// 자식 리스트
//	typedef tPointerList<cUINodeProperty*> cNodeList;
	typedef tPointerList<void*> cNodeList;
	cNodeList mChildList;

public:
	/// 자기자신의 프로퍼티 이름 
	cString mPropertyName;

	/// 스킨 이름
	cString mSkinName;

protected:
	/// 타입
	eUINodeType mType;
};

inline eUINodeType cUINodeProperty::GetType() const
{
	return mType;
}

inline const tPointerList<void*>& cUINodeProperty::GetChildList() const
{
	return mChildList;
}
