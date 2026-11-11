#include "stdafx.h"
#include "TimeManager.h"

//---------------------------------------------------------------------------
cTimeManager::cTimeManager() 
: m_fAccumTime(0.0f),
m_fScaleFactor(1.0f), 
m_fStartTime(0.0f), 
m_fEndTime(0.0f),
m_bTimingEnabled(false), 
m_eTimeMode(LOOP), 
m_fLastTime(-NI_INFINITY)
{ 
}
//---------------------------------------------------------------------------
float cTimeManager::GetCurrentTime()
{
	if (!m_bTimingEnabled)
	{
		return m_fAccumTime;
	}

	float fCurrentTime = NiGetCurrentTimeInSec();
	if (m_fLastTime == -NI_INFINITY)
	{
		m_fLastTime = fCurrentTime;
	}

	float fDelta = fCurrentTime - m_fLastTime;
	fDelta *= m_fScaleFactor;

	m_fAccumTime += fDelta;

	if (m_fAccumTime < m_fStartTime)
	{
		m_fAccumTime = m_fStartTime;
	}
	else if (m_eTimeMode == LOOP && m_fAccumTime > m_fEndTime)
	{
		if (m_fStartTime == m_fEndTime)
		{
			m_fAccumTime = m_fEndTime;
		}
		else
		{
			m_fAccumTime -= m_fEndTime - m_fStartTime;
		}
	}

	m_fLastTime = fCurrentTime;
	return m_fAccumTime;
}
//---------------------------------------------------------------------------
void  cTimeManager::SetCurrentTime(float fTime)
{ 
	if(m_fAccumTime != fTime)
	{
		if (m_eTimeMode == LOOP)
		{
			if (fTime > m_fEndTime)
			{
				fTime -= m_fEndTime - m_fStartTime;
			}
			else if (fTime < m_fStartTime)
			{
				fTime += m_fEndTime - m_fStartTime;
			}
		}

		m_fAccumTime = fTime;
		m_fLastTime = -NI_INFINITY;
	}
}
//---------------------------------------------------------------------------
float cTimeManager::IncrementTime(float fIncrement)
{
	m_fAccumTime += fIncrement;
	if (m_eTimeMode == LOOP)
	{
		if (m_fAccumTime > m_fEndTime)
		{
			m_fAccumTime -= m_fEndTime - m_fStartTime;
		}
		else if (m_fAccumTime < m_fStartTime)
		{
			m_fAccumTime += m_fEndTime - m_fStartTime;
		}
	}

	return m_fAccumTime;
}
//--------------------------------------------------------------------------
