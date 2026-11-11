#pragma once

struct sThemeTime
{
	unsigned char		mResetClockHour;
	unsigned char		mResetIntervalHour;
};

class cThemeTimeScript
{
public:
	static cThemeTimeScript* mpThemeTimeScript;

public:
	cThemeTimeScript(void);
	~cThemeTimeScript(void){};

	bool				Init();
	void				Release();

	sThemeTime*			GetThemeTime( unsigned long mapNumber )	{ return (sThemeTime*)mThemeTimeMap.GetAt(mapNumber); }

private:

	typedef tPointerHashMap<unsigned long, void*> cPHashMap;

	/// <unsigned long mapNumber, sThemeTime*>
	cPHashMap			mThemeTimeMap;
};

#define THEMETIMESCRIPT cThemeTimeScript::mpThemeTimeScript