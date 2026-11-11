/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.03.20
* 내    용 : 사운드 시스템
* 주의사항 : 
*===========================================================================*/
#pragma once

#ifdef MAP_EDITOR
/// 사운드 시스템
class cSoundSystem
{
	static cSoundSystem* mSingleton;

public:
	cSoundSystem();
	~cSoundSystem();

	/// 초기화
	bool Init( NiWindowRef hwnd );

	/// 2D 사운드
	NiAudioSource* Load2DSound( unsigned int index, const cString& pathName, bool streamed = false );
	NiAudioSource* Get2DSound( unsigned int index ) const;
	bool Play2DSound( unsigned int index );
	bool Stop2DSound( unsigned int index );

public:
	/// 단일체를 리턴
	static cSoundSystem* GetSingleton();

private:
	/// 2D 사운드 맵
	typedef tHashMap<unsigned int, NiAudioSourcePtr> c2DSoundMap;
	c2DSoundMap m2DSoundMap;
};

inline
NiAudioSource* cSoundSystem::Get2DSound( unsigned int index ) const
{
	c2DSoundMap::cConstIterator i = m2DSoundMap.Find( index );

	if( i != m2DSoundMap.End() )
		return i->mSecond;
	else
		return 0;
}

inline
cSoundSystem* cSoundSystem::GetSingleton()
{
	return mSingleton;
}

#define SOUNDSYS cSoundSystem::GetSingleton()
#endif /// MAP_EDITOR
