/* ==========================================================================
* 파    일 : TipManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-12-16
* 주의사항 :
*===========================================================================*/

const int MAX_TIPCOUNT = 100;
const unsigned long TIP_UPDATETIME = 1000 * 60 * 5; // 1분
const unsigned int TIPIMG_W = 630;
const unsigned int TIPIMG_H = 126;

struct sLoadTipData
{
	/// 텍스쳐 명
	cString mTexName;
	
	/// 텍스쳐 위치
	unsigned int mTexX;
	unsigned int mTexY;

	sLoadTipData()
	{
		mTexX = 0;
		mTexY = 0;
	}
};

class cTipManager
{
public:
	cTipManager();
	~cTipManager();

	bool Init();
//	void Process( unsigned long accumTime );

	/// 
	sLoadTipData* GetLoadTipData();

public:
	static cTipManager*	mpSingleton;
	static cTipManager* GetSingleton();

private:
	bool LoadGameTip( const cString& pathname );
	bool LoadLoadTip( const cString& pathname );

private:
	/// 채팅 팁 관리
	typedef tArray<unsigned long> cIndexArr;
	cIndexArr mTipArr[MAX_TIPCOUNT];

	/// 로딩 팁 관리
	typedef tPointerArray<void*> cLoadTipArr;
	cLoadTipArr mLoadTipArr;

	///
	unsigned long mLastAccumTime;
};

inline cTipManager* cTipManager::GetSingleton() 
{
	return mpSingleton;
}

#define TIPMAN cTipManager::GetSingleton()