// class 명명

class CTemp
{
public:
	// 생성자			
	// 소멸자
	
	// static 함수
	
	// inline 함수
	
	// 멤버 함수
	
protected:
private:	
	// static 함수
	
	// inline 함수
	
	// 멤버 함수	
	
public:
protected:
	// 멤버 변수
	
	
#ifdef _DEBUG	// _DEBUG

public:
	// 디버그 용 함수
private:
	// 디버그 용 변수
	
#endif			// _DEBUG		
}

// example ----------------------------------------------------------------------------------------
class CApplication
{
public:
	static CApplication* ms_pkApplication;

	CApplication();
	~CApplication();

	// static
	static CApplication*	Create();							// 리턴값과 함수 이름사이는 tap으로 구분
	// inline
	inline BOOL		IsActive()		{ return m_bActive; }
	// normal
	BOOL	Init(NiInstanceRef hi);
	void	Run();
	void	Exit();

private:
	BOOL	CreateMainWindow();
	
private:
    NiInstanceRef		m_pInstance;
	CAppWindow*			m_pkWindow;
	BOOL				m_bActive;
	
#ifdef _DEBUG	// _DEBUG

private:
	CDevSystem*		m_pkDevSystem;

#endif			// _DEBUG
};

#define APPLICATION		CApplication::ms_pkApplication
//-------------------------------------------------------------------------------------------------


// #ifdef _DEBUG		

#ifdef	_DEBUG			// _DEBUG

#endif					// endif _DEBUG


// API 함수는 '::' 붙여쓴다
::SetWindowPos( m_hWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE );