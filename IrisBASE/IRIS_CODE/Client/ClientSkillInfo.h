//
//
//

#pragma once

#include "Skill_Common.h"

class cDramaturgyInfo;

struct sClientSkillInfo : public sPlayerSkillBaseInfo
{
	/// 연출할 수 있는 그룹정보의 Table 
	/// - 그룹내에 UserType으로 어떤 연출을 사용할 지 결정. 
	/// - 만약에 지정된 인덱스가 없다면 에러를 낸다.
	/// < user kind ID, 연출정보* > - 참조만 한다. 모든 객체는 따로관리된다.
	//	tPointerHashMap<unsigned int,cDramaturgyInfo*> DramaturgyGroup;
	tPointerHashMap<unsigned int,void*> DramaturgyGroup;

	/// IconImageNumber
	unsigned int	SkillIconImageNumber;
	unsigned int	BufIconImageNumber;

	/// 스킬이름
	cStringT skillname;
};
