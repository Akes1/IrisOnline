#pragma once

class cTimeManager : public NiMemObject
{
public:
	enum TimeMode
	{
		LOOP = 0,
		CONTINUOUS,
		NUM_MODES
	};

	enum 
	{
		TM_SCALE_FACTOR = 0,
		TM_START_TIME,
		TM_END_TIME,
		TM_TIME_MODE,
		TM_ENABLING
	};

	cTimeManager();

	float GetCurrentTime();
	void SetCurrentTime(float fTime);

	float GetScaleFactor() const;
	void SetScaleFactor(float fScale);

	float IncrementTime(float fIncrement);

	float GetStartTime() const;
	void SetStartTime(float fStart);

	float GetEndTime() const;
	void SetEndTime(float fEnd);

	bool IsEnabled() const;
	void Enable();
	void Disable();

	TimeMode GetTimeMode() const;
	void SetTimeMode(TimeMode eMode);

protected:
	float m_fAccumTime;
	float m_fScaleFactor;
	float m_fStartTime;
	float m_fEndTime;
	bool m_bTimingEnabled;
	TimeMode m_eTimeMode;

	float m_fLastTime;
};

//---------------------------------------------------------------------------
inline float cTimeManager::GetScaleFactor() const
{ 
	return m_fScaleFactor;
}
//---------------------------------------------------------------------------
inline void cTimeManager::SetScaleFactor(float fScale)
{ 
	m_fScaleFactor = fScale;
//	MainThreadUpdateAllViews(MAKELPARAM(NIF_TIMINGINFOCHANGED, TM_SCALE_FACTOR));
}
//---------------------------------------------------------------------------
inline float cTimeManager::GetStartTime() const
{ 
	return m_fStartTime;
}
//---------------------------------------------------------------------------
inline void cTimeManager::SetStartTime(float fStart)
{ 
	m_fStartTime = fStart;
//	MainThreadUpdateAllViews(MAKELPARAM(NIF_TIMINGINFOCHANGED,TM_START_TIME));
}
//---------------------------------------------------------------------------
inline float cTimeManager::GetEndTime() const
{ 
	return m_fEndTime;
}
//---------------------------------------------------------------------------
inline void cTimeManager::SetEndTime(float fEnd)
{ 
	m_fEndTime = fEnd;
//	MainThreadUpdateAllViews(MAKELPARAM(NIF_TIMINGINFOCHANGED, TM_END_TIME));
}
//---------------------------------------------------------------------------
inline bool cTimeManager::IsEnabled() const
{
	return m_bTimingEnabled;
}
//---------------------------------------------------------------------------
inline void cTimeManager::Enable()
{
	m_bTimingEnabled = true;
	m_fLastTime = -NI_INFINITY;
//	MainThreadUpdateAllViews(MAKELPARAM(NIF_TIMINGINFOCHANGED, TM_ENABLING));
}
//---------------------------------------------------------------------------
inline void cTimeManager::Disable()
{
	m_bTimingEnabled = false;
//	MainThreadUpdateAllViews(MAKELPARAM(NIF_TIMINGINFOCHANGED, TM_ENABLING));
}
//---------------------------------------------------------------------------
inline cTimeManager::TimeMode cTimeManager::GetTimeMode() const
{
	return m_eTimeMode;
}
//---------------------------------------------------------------------------
inline void cTimeManager::SetTimeMode(TimeMode eMode)
{
	NIASSERT(eMode >= 0 && eMode < NUM_MODES);
	m_eTimeMode = eMode;
//	MainThreadUpdateAllViews(MAKELPARAM(NIF_TIMINGINFOCHANGED, TM_TIME_MODE));
}
//---------------------------------------------------------------------------
