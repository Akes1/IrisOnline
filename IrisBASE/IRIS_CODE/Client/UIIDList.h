/* ==========================================================================
* 파    일 : UIIDList.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-04-03
* 주의사항 :
*===========================================================================*/
#pragma once

class cUIIDList
{
public:
	cUIIDList();
	~cUIIDList();
	/// 로딩
	bool Load( const cString& pathName );
	/// 프로퍼티 이름에 해당하는 id 리턴
	unsigned int GetPropID( const cString& propname ) const;

private:
	/// 프로퍼티 이름, 아이디 값 해쉬맵 
	typedef tHashMap<cString, unsigned int> cIDHashMap;
	cIDHashMap	mIDHashMap;
};
