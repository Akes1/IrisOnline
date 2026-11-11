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

struct sPlayInfo
{
	float mVolume;
	NiAudioSourcePtr mSnd;

	sPlayInfo()
	{
		mVolume = 1.0f;
		mSnd = 0;
	}
	~sPlayInfo()
	{
		assert( mSnd->GetRefCount() == 1 );
		mSnd = 0;
//		SAFE_NIDELETE( mSnd );
	}
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

	void Open( unsigned int mapIdx, unsigned char mapMode );
	void Close();

	/// 처리
	void Process( unsigned long deltaTime, unsigned long accumTime );

	void ProcessStageBGM( unsigned long accumTime );
	inline bool IsBGMFading() { return mFadeBGM; }

	inline bool IsSoundActive() { return mActiveSound; }
	void ActiveSound( bool active );

	void UpdateSoundGain();

	/// 
	void StopAll();

	/// 영역 사운드
	NiAudioSource* LoadAreaSound( unsigned int index, const cString& pathName );
	NiAudioSource* GetAreaSound( unsigned int index ) const;
	//bool DestroyAreaSound( unsigned int index );

	void SetHWND( NiWindowRef hwnd );

	inline bool IsUsableSoundSystem() { return mInitedSoundSystem; }

public:
	/// 사운드에 대한 정보 참조
	sSoundInfo*	GetSoundInfo( unsigned long index );

	/// 2D 사운드
	bool Play2DSound( unsigned long index );

	///
	bool LoadBGMList();
	void PlayBGMSound( bool on );

	///
	void PlayAreaSound( NiAudioSource* snd );

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

	typedef tPointerHashMap<unsigned long, unsigned long> cOverlapMap;
	cOverlapMap mOverlapTester;

	/// 출력 Area Sound
	NiAudioSource*		mCurrentAreaSound;

	/// 삭제용..
	typedef tPointerList<void*> cDelList;
	cDelList mDelList;

	NiAudioSource*		mBGMSound;
	bool				mFadeBGM;

	bool				mActiveSound;

	///
	NiAudioSource*		mCurrentMapBGM;

	bool				mIsActiveAreaSound;
	unsigned long		mMapBGMWaitTime;
	unsigned long		mCheckTime;

	bool mInitedSoundSystem;
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
