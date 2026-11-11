/* ==========================================================================
* 파    일 : ThemeManager.h
* 목    적 : 
* 작 성 자 : 
* 작 성 일 : 
* 주의사항 :
*===========================================================================*/

class cThemeManager
{
public:
	cThemeManager();
	~cThemeManager();

	void Close();
	void Clear();
	void Process( unsigned long deltaTime, unsigned long accumTime );

	///
	void NetworkMsgParser( char* msg );

public:
	static cThemeManager* mpSingleton;	
	static cThemeManager* GetSingleton();

protected:
	void ClearThemeUI();
	void UpdateThemeUI();

private:
	// 테마던전 시간관련
	unsigned long	mThemeExpireTime;

	// 테마 정보 sPlayerThemeList
	typedef tPointerList<void*> cThemeInfoList;
	cThemeInfoList mThemeInfoList;
};

inline cThemeManager* cThemeManager::GetSingleton()
{
	return mpSingleton;
}

#define THEMEMAN cThemeManager::GetSingleton()