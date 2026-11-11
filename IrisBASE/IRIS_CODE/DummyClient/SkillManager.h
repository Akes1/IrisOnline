#pragma once

class cSkillManager
{
public:
	cSkillManager();
	~cSkillManager();

	void Process( unsigned long deltaTime, unsigned long accumTime );
};
