// XPerformanceGuard.h 
// code by Testors / 2005/02/07 03:29 
#if (defined(_DEVSYS) || defined(_DEBUG))

#pragma once 
/*
#pragma warning (push, 3)
#include <vector> 
#include <mmsystem.h> 
#include <winbase.h> 
#pragma warning (pop)
*/
class XPerformanceGuard 
{ 
public:
	XPerformanceGuard( const char *szName, int nLimit ) : m_szName( szName ) 
	{     
		m_nStartTime    = timeGetTime();            // 코드 시작 시간 세팅 
		m_nTimeLimit    = m_nStartTime + nLimit; 
		m_nEndTime        = 0; 
	} 

	~XPerformanceGuard() 
	{ 
		m_nEndTime = timeGetTime(); 

		// 수행된 시간이 허용치를 초과했다면 퍼포먼스 경고 출력 
		if( m_nEndTime > m_nTimeLimit ) printPerformanceWarning(); 
	} 

	void Profile( const char *szName ) 
	{ 
		m_vResult.push_back( _TAG( szName, timeGetTime() ) ); 
	} 

protected: 

	void oprint( const char *str, ... ) 
	{ 
		char szBuf[1024]; 
		va_list va; 
		va_start( va, str ); 
		_vsnprintf_s( szBuf, sizeof(szBuf), str, va ); 
		va_end( va ); 
		OutputDebugStringA( szBuf ); 
	} 

	// 각각의 수행 시간을 출력 
	void printPerformanceWarning() 
	{ 
		int nTmp = m_nStartTime; 
		char szBar[29] = "============================"; 
		memcpy( szBar, m_szName, strlen(m_szName) > sizeof(szBar)-1 ? sizeof(szBar)-1 : strlen(m_szName) ); 
		if( strlen(m_szName) < sizeof(szBar)-1 ) szBar[strlen(m_szName)] = ' '; 
		oprint( "\n== PERFOMANCE CAUTION : %s\n", szBar ); 
		for( size_t i = 0; i < m_vResult.size(); ++i )  
		{ 
			oprint( "%3d. %-25s : %d ms\n", i, m_vResult[i].szName, m_vResult[i].nTime - nTmp ); 
			nTmp = m_vResult[i].nTime; 
		} 
		oprint( "     %-25s : %d ms\n", "TOTAL", m_nEndTime - m_nStartTime ); 
		oprint( "====================================================\n\n" ); 
	} 

	struct _TAG 
	{ 
		_TAG( const char *name, int t ) : szName( name ), nTime( t )    {} 

		const char *    szName; 
		int                nTime; 
	}; 

	const char*            m_szName; 
	int                    m_nStartTime; 
	int                    m_nEndTime; 
	int                    m_nTimeLimit; 
	int                    m_nTM; 
	std::vector< _TAG >    m_vResult; 
}; 

#define XPG_SET( x,y )    XPerformanceGuard    ___x_performance_guard((x),(y)) 
#define XPG_DO( x )        ___x_performance_guard.Profile((x)) 
#else
#define XPG_SET( x,y )
#define XPG_DO( x )
#endif


/*
ex) XPG_SET( "Performance Test", 5000 );
	XPG_DO( "Check Position Name" );
*/
