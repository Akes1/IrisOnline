#include "stdafx.h"

#ifdef MAP_EDITOR
#include "SoundSystem.h"
#include "CameraManager.h"
#include "../Hero.h"

cSoundSystem* cSoundSystem::mSingleton = 0;

cSoundSystem::cSoundSystem()
: m2DSoundMap( 4096 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cSoundSystem::~cSoundSystem() 
{
	// Shutdown the Sound System
	NiAudioSystem::GetAudioSystem()->Shutdown();

	mSingleton = 0;
}

bool cSoundSystem::Init( NiWindowRef hwnd )
{
	NiMilesAudioSystem* as = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();

	as->SetHWnd(hwnd);

	// Put the path to the Miles redist files here.
	if( as->Startup( "MapData/Miles" ) == false )
	{
		assert( 0 && "failed to start audio system" );
		return false;
	}

	///
	as->SetBestSpeakerTypeAvailable();
	as->SetUnitsPerMeter( 100.0f );
	return true;
}

NiAudioSource* cSoundSystem::Load2DSound( unsigned int index, const cString& pathName, bool streamed )
{
	/// 이미 존재하는지 검사
	c2DSoundMap::cIterator pos = m2DSoundMap.Find( index );

	if( pos != m2DSoundMap.End() )
	{
		return pos->mSecond;
	}

	/// 사운드 로딩
	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
	NiAudioSource* src = as->CreateSource( NiAudioSource::TYPE_AMBIENT );
	src->SetStreamed( streamed );
	src->SetFilename( pathName.Cstr() );
	src->SetAllowSharing( true );

	if( src->Load() == false )
	{
		assert( 0 && "failed to load sound" );
		return 0;
	}

	/// 속성을 설정
	src->SetGain( 0.0f );
	src->SetLoopCount( 1 );

	/// 사운드 추가
	m2DSoundMap.Insert( index, src );
	return src;
}

bool cSoundSystem::Play2DSound( unsigned int index )
{
	c2DSoundMap::cConstIterator i = m2DSoundMap.Find( index );

	if( i != m2DSoundMap.End() )
	{
		NiAudioSource* src = i->mSecond;
		assert( src );

		src->Play();
		return true;
	}
	else
		return false;
}

bool cSoundSystem::Stop2DSound( unsigned int index )
{
	c2DSoundMap::cConstIterator i = m2DSoundMap.Find( index );

	if( i != m2DSoundMap.End() )
	{
		NiAudioSource* src = i->mSecond;
		assert( src );

		src->Stop();
		return true;
	}
	else
		return false;
}
#endif /// MAP_EDITOR
