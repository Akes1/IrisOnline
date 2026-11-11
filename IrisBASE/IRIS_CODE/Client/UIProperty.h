/* ==========================================================================
* 파    일 : UIProperty.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-13
* 주의사항 :
*===========================================================================*/
#pragma once

class cUINodeProperty;
class cParser;

class cUIProperty
{
public:
	cUIProperty();
	~cUIProperty();

	/// 지우기
	void Clear();

	/// 로딩
	bool Load( const cString& pathName );

	/// 노드 속성
	cUINodeProperty* GetNodeProperty( const cString& name ) const;

protected:
	/// 스크립트 로딩
	bool LoadScripts( const cString& path, cParser& parser );
	bool LoadScript( const cString& pathName );

private:
	/// 속성 맵
//	typedef tPointerHashMap<cString, cUINodeProperty*> cNodePropertyMap;
	typedef tPointerHashMap<cString, void*> cNodePropertyMap;
	cNodePropertyMap mNodePropertyMap;
};
