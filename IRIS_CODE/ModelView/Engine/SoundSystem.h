/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.03.20
* 내    용 : 사운드 시스템
* 주의사항 : 
*===========================================================================*/
#pragma once

struct sSoundInfo
{
	unsigned long	mIndex;
	cString			mFileName;
	float			mVolume;
	float			mRadius;
};


/// 사운드 시스템
class cSoundSystem
{
	static cSoundSystem* mSingleton;

public:
	/// 단일체를 리턴
	static cSoundSystem* GetSingleton();

	cSoundSystem();
	~cSoundSystem();

	/// 초기화
	bool Init();

	void Close();

	/// 처리
	void Process( unsigned long deltaTime, unsigned long accumTime );

	inline bool IsSoundActive() { return mActiveSound; }
	void ActiveSound( bool active );

	/// 
	void StopAll();

	/// 영역 사운드
	NiAudioSource* LoadAreaSound( unsigned int index, const cString& pathName, bool streamed = false );
	NiAudioSource* GetAreaSound( unsigned int index ) const;
	//bool DestroyAreaSound( unsigned int index );

	void SetHWND( NiWindowRef hwnd );

public:
	/// 사운드에 대한 정보 참조
	sSoundInfo*	GetSoundInfo( unsigned long index );

	/// 2D 사운드
	bool Play2DSound( unsigned long index );

protected:
	bool LoadSoundList();

private:
	/// 영역 사운드 맵
	typedef tHashMap<unsigned int, void*> cAreaSoundMap;
	cAreaSoundMap mAreaSoundMap;

	/// 스크립트용 인덱스, 사운드정보
	typedef tPointerHashMap<unsigned long, void*> cPointHashMap;
	cPointHashMap mSoundListMap;

	/// 출력 2D 사운드 맵
	typedef tPointerList<void*> cPlay2DSoundList;
	cPlay2DSoundList mPlayList;

	/// 삭제용..
	typedef tPointerList<void*> cDelList;
	cDelList mDelList;

	bool				mActiveSound;
};

inline
NiAudioSource* cSoundSystem::GetAreaSound( unsigned int index ) const
{
	cAreaSoundMap::cConstIterator i = mAreaSoundMap.Find( index );

	if( i != mAreaSoundMap.End() )
		return (NiAudioSource*)(*i).mSecond;
	else
		return 0;
}

inline
sSoundInfo* cSoundSystem::GetSoundInfo( unsigned long index )
{
	return (sSoundInfo*)mSoundListMap.GetAt( index );
}

inline
cSoundSystem* cSoundSystem::GetSingleton()
{
	return mSingleton;
}

#define SOUNDSYS cSoundSystem::GetSingleton()

/*// 사운드 시스템
class cSoundSystem
{
static cSoundSystem* mpSingleton;

public:
cSoundSystem();
~cSoundSystem();

/// 지움
void Clear();

/// 종료
void Exit();

/// 초기화
bool Init( NiWindowRef hwnd );

/// 처리
void Process( float time );

/// 리스너
NiAVObject* GetListener();

/// 로딩
bool LoadBackground( const cString& pathName );
bool LoadEffect( const cString& pathName );

/// 플레이
//void PlayBackground( unsigned int index );
void PlayEffect( unsigned int index );

void StopAllSounds();
//void RewindAmbientSound();
//void SetAmbientGain(float fGain);

protected:
bool LoadAmbient( const cString& pathName, bool bgm );

public:
/// 단일체를 리턴
static cSoundSystem* GetSingleton();

private:
/// 배경 사운드 배열
//	typedef tArray<NiAudioSource*> cSoundArray;
typedef tArray<void*> cSoundArray;
cSoundArray mBackgroundArray;

/// 효과 사운드 배열
cSoundArray mEffectArray;

//NiAudioSource* mpCurrentAmbient;
};

inline cSoundSystem* cSoundSystem::GetSingleton()
{
return mpSingleton;
}

#define SOUNDSYS cSoundSystem::GetSingleton()
*/
