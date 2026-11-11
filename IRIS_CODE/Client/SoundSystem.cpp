#include "stdafx.h"
#include "SoundSystem.h"

#include "Application.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "OptionManager.h"
#include "cameramanager.h"
#include "objectmanager.h"
#include "hero.h"

#include "GameResourceManager.h"

cSoundSystem* cSoundSystem::mSingleton = 0;

cSoundSystem::cSoundSystem()
: mAreaSoundMap( 256 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mFadeBGM = false;
	mBGMSound = 0;

	mCurrentAreaSound = 0;

	mActiveSound = false;

	///
	mCurrentMapBGM = 0;
	mMapBGMWaitTime = 0;
	mCheckTime = ULONG_MAX;

	mInitedSoundSystem = false;
	mIsActiveAreaSound = false;
}

cSoundSystem::~cSoundSystem() 
{
	mBGMSound = 0;

	cPointHashMap::cIterator i = mSoundListMap.Begin();
	cPointHashMap::cIterator end = mSoundListMap.End();
	for( ; i != end; ++i )
	{
		sSoundInfo* pInfo = (sSoundInfo*)((*i).mSecond);
		SAFE_DELETE(pInfo);
	}
	mSoundListMap.Clear();

	Close();

	// Shutdown the Sound System
	NiAudioSystem::GetAudioSystem()->Shutdown();

	mSingleton = 0;
}

void cSoundSystem::SetHWND( NiWindowRef hwnd )
{
	NiMilesAudioSystem* as = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if( as == 0 )
		return;

	as->SetHWnd(hwnd);
}

bool cSoundSystem::Init()
{
	NiMilesAudioSystem* as = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
	if( as == 0 )
		return false;

	// Put the path to the Miles redist files here.
	if( as->Startup( "./Sound/Miles" ) == false )
	{
		assert( 0 && "failed to start audio system" );
		return false;
	}

	/*/ Get a provider by iterating through the provider list looking first for
	// "DirectSound3D Hardware Support" (hardware accelerated) and, if it's
	// unavailable, then for "Miles Fast 2D Positional Audio" (software
	// emulated), as "best choice" defaults.  If neither provider is
	// available, report an error and return false.
	NiTListIterator i;
	NiProviderInfo* provider = as->GetFirstProvider(i);
	while( provider )
	{
		if( ::strstr(provider->GetProviderName(), "DirectSound3D Hardware Support") )
		{
			break;
		}
		provider = as->GetNextProvider(i);
	}

	if( (!provider) || (!provider->OpenProvider()) )
	{
		provider = as->GetFirstProvider(i);
		while( provider )
		{
			if( ::strstr(provider->GetProviderName(), "Miles Fast 2D Positional Audio") )
			{
				break;
			}
			provider = as->GetNextProvider(i);
		}

		if( (!provider) || (!provider->OpenProvider()) )
		{
			assert( 0 && "providers are not available" );
			return false;
		}
	}

	// Set the Default Provider to be used
	as->SetDefaultProvider( provider );

	// set up the listener with the selected provider
	as->GetListener()->Startup( provider );
	*/
	as->SetBestSpeakerTypeAvailable();
	as->SetUnitsPerMeter( 39.37f );

	if( LoadSoundList() == false )
		return false;

	NiAudioListener* listener = as->GetListener();
	listener->SetUpVector(NiPoint3(0.0f, 0.0f, 1.0f));
	listener->SetDirectionVector(NiPoint3(0.0f, -1.0f, 0.0f));

	as->Update( 0.0f, true );

	mInitedSoundSystem = true;

	return true;
}

void cSoundSystem::Open( unsigned int mapIdx, unsigned char mapMode )
{
	cString str = GAMERESOURCEMAN->GetMapSoundName( mapIdx, mapMode );
	if( str.IsEmpty() )
		return;

	mCurrentMapBGM = RESOURCEMAN->LoadStageSound( str.Cstr() );
	if( mCurrentMapBGM == 0 )
		return;

	mMapBGMWaitTime = GAMERESOURCEMAN->GetMapSoundWaitTime( mapIdx, mapMode );

	mCurrentMapBGM->SetLoopCount( 1 );
	mCurrentMapBGM->SetGain( 0.0f );

	if( mActiveSound )
		mCurrentMapBGM->FadeToGain( OPTIONMAN->GetEnvVolume(), THEAPP->GetWorldAccumTime()*0.001f, 3.0f );

	mCurrentMapBGM->Play();

	mIsActiveAreaSound = false;
}

void cSoundSystem::Close()
{
	if( mCurrentMapBGM )
	{
		mCurrentMapBGM->Stop();
		mCurrentMapBGM = 0;
	}

	if( mCurrentAreaSound )
	{
		mCurrentAreaSound->Stop();
		mCurrentAreaSound = 0;
	}

	if( mAreaSoundMap.IsEmpty() == false )
	{
		cAreaSoundMap::cIterator i, end;
		NiAudioSource* p = 0;

		i = mAreaSoundMap.Begin();
		end = mAreaSoundMap.End();
		for( ; i != end; ++i )
		{
			p = (NiAudioSource*)(*i).mSecond;
			SAFE_NIDELETE( p );
		}
		mAreaSoundMap.Clear();
	}

	if( mPlayList.IsEmpty() == false )
	{
		cPlay2DSoundList::cIterator i = mPlayList.Begin();
		cPlay2DSoundList::cIterator end = mPlayList.End();
		for( ; i != end; ++i )
		{
			sPlayInfo* info = (sPlayInfo*)*i;
			SAFE_DELETE( info );
		}
	}
	mPlayList.Clear();
	mDelList.Clear();

	mOverlapTester.Clear();
}

void cSoundSystem::StopAll()
{
	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
	if( as )
		as->StopAllSources();
}

void cSoundSystem::Process( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	//float fTime = ((float)accumTime * 0.001f);

	mOverlapTester.Clear();

	/// 2d Sound process
	if( mPlayList.IsEmpty() == false )
	{
		cPlay2DSoundList::cIterator i = mPlayList.Begin();
		cPlay2DSoundList::cIterator end = mPlayList.End();
		for( ; i != end; ++i )
		{
			sPlayInfo* info = (sPlayInfo*)*i;
			if( info && info->mSnd )
			{
				if( info->mSnd->GetStatus() != NiAudioSource::PLAYING )
				{
					/// 삭제 등록
					mDelList.Insert( mDelList.End(), info );
				}
			}
		}
	}

	/// 종료된 객체 삭제
	if( mDelList.IsEmpty() == false )
	{
		cDelList::cIterator i = mDelList.Begin();
		cDelList::cIterator end = mDelList.End();
		for( ; i != end; ++i )
		{
			sPlayInfo* info = (sPlayInfo*)*i;
			mPlayList.Remove( info );
			SAFE_DELETE(info);
		}
		mDelList.Clear();
	}	

	///
	if( mCurrentAreaSound )
	{
		float tlen = 0.0f;
		if( mCurrentAreaSound->GetPlayLength( tlen ) == true )
		{
			float t = mCurrentAreaSound->GetPlayTime();
			if( tlen - t <= 3.0f )
			{
				if( mActiveSound )
					mCurrentMapBGM->FadeToGain( OPTIONMAN->GetEnvVolume(), accumTime*0.001f, 3.0f );

				mCurrentAreaSound->FadeToGain( 0.0f, accumTime*0.001f, 3.0f );
			}
		}

		if( mCurrentAreaSound->GetStatus() != NiAudioSource::PLAYING )
		{
			if( mActiveSound )
				mCurrentMapBGM->FadeToGain( OPTIONMAN->GetEnvVolume(), accumTime*0.001f, 3.0f );

			mCurrentAreaSound->Stop();
			mCurrentAreaSound = 0;
		}
	}

	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
	if( as )
	{
		as->Update(accumTime*0.001f, true);
	}
}

void cSoundSystem::ProcessStageBGM( unsigned long accumTime )
{
	float fTime = ((float)accumTime * 0.001f);

	if( mActiveSound == false )
	{
		if( mBGMSound )
		{
			mBGMSound->SetGain(0.0f);

			if( mFadeBGM == true )
			{
				mBGMSound->Stop();
				mFadeBGM = false;
			}
		}

		if( mCurrentMapBGM )
			mCurrentMapBGM->SetGain(0.0f);

		return;
	}

	if( mCurrentMapBGM )
	{
		mCurrentMapBGM->Update( fTime );

		if( mCurrentMapBGM->GetStatus() != NiAudioSource::PLAYING )
		{
			if( mCheckTime == ULONG_MAX )
			{
				mCurrentMapBGM->Stop();
				mCheckTime = mMapBGMWaitTime + accumTime;
			}
			else
			{
				if( mCheckTime <= accumTime )
				{
					mCurrentMapBGM->Play();
					mCurrentMapBGM->SetGain( 0.0f );

					if( mActiveSound )
						mCurrentMapBGM->FadeToGain( OPTIONMAN->GetEnvVolume(), accumTime*0.001f, 3.0f );

					mCheckTime = ULONG_MAX;
				}
			}
		}
	}

	if( mFadeBGM == false )
		return;

	if( mBGMSound )
	{
		mBGMSound->Update( fTime );
		if( mBGMSound->GetGain() == 0.0f )
		{
			mBGMSound->Stop();
			mFadeBGM = false;
		}
		else if( mBGMSound->GetGain() == OPTIONMAN->GetEnvVolume() )
		{
			mFadeBGM = false;
		}
	}
}

NiAudioSource* cSoundSystem::LoadAreaSound( unsigned int index, const cString& pathName )
{
	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
	if( as == 0 )
		return 0;

	/// 이미 존재하는지 검사
	cAreaSoundMap::cIterator pos = mAreaSoundMap.Find( index );

	if( pos != mAreaSoundMap.End() )
	{
		return (NiAudioSource*)(pos->mSecond);
	}

	/// 사운드 로딩
	NiAudioSource* snd = as->CreateSource( NiAudioSource::TYPE_AMBIENT );
	snd->SetFilename( pathName.Cstr() );

	NiFilename filename( pathName.Cstr() );
	if( _strcmpi( filename.GetExt(), ".mp3") == 0 )
	{
		snd->SetStreamed( true );
		snd->SetAllowSharing( false );
	}
	else
	{
		snd->SetStreamed( false );
		snd->SetAllowSharing( true );
	}

	if( snd->Load() == false )
	{
		assert( 0 && "failed to load sound" );
		return 0;
	}

	/// 속성을 설정
	snd->SetGain( 0.0f );
	snd->SetLoopCount( 1 );

	/// 사운드 추가
	mAreaSoundMap.Insert( index, snd );
	return snd;
}

/*
bool cSoundSystem::DestroyAreaSound( unsigned int index )
{
	cAreaSoundMap::cIterator i = mAreaSoundMap.Find( index );

	if( i != mAreaSoundMap.End() )
	{
		NiAudioSource* snd = (*i).mSecond;
		snd->Stop();

		NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
		as->RemoveSource( snd );

		mAreaSoundMap.Erase( i );
		return true;
	}
	return false;
}
*/

bool cSoundSystem::LoadSoundList()
{
	cFileLoader loader;
	cString pathName;
	pathName = "./Script/Resource/SoundList.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load SoundList.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		sSoundInfo* pInfo = new sSoundInfo;

		/// sound index
		if( tokenizer.GetNext( &str ) == false )
			goto _Error; 
		pInfo->mIndex = str.ToInt();

		/// sound file name
		if( tokenizer.GetNext( &str ) == false )
			goto _Error; 
		pInfo->mFileName = str;

		/// volume
		if( tokenizer.GetNext( &str ) == false )
			goto _Error;
		pInfo->mVolume = str.ToFloat();

		/// radius
		if( tokenizer.GetNext( &str ) == false )
			goto _Error;
		pInfo->mRadius = str.ToFloat();

		if( mSoundListMap.Insert( pInfo->mIndex, pInfo ) )
			continue;

_Error:
 		assert( 0 && "failed to load soundlist" );
		return false;
	}

	return true;
}

bool cSoundSystem::LoadBGMList()
{
	/// login 1번
	if( RESOURCEMAN->LoadTotalSound( "BGM_Login01.mp3" ) == false )
		return false;
	mBGMSound = RESOURCEMAN->GetTotalSoundByName( "BGM_Login01.mp3" );
	if( mBGMSound )
	{
		mBGMSound->SetGain( 0.0f );
		mBGMSound->SetLoopCount( NiAudioSource::LOOP_INFINITE );
		mBGMSound->Update(0.0f);
	}
	return true;
}

void cSoundSystem::PlayBGMSound( bool on )
{
	if( IsUsableSoundSystem() == false )
		return;

	if( mBGMSound == 0 )
		return;

	mFadeBGM = true;
	if( on == true )
	{
		mBGMSound->SetGain( 0.0f );
		mBGMSound->FadeToGain( OPTIONMAN->GetEnvVolume(), THEAPP->GetWorldAccumTime()*0.001f, 3.0f );
		mBGMSound->Play();
	}
	else
	{
		mBGMSound->FadeToGain( 0.0f, THEAPP->GetWorldAccumTime()*0.001f, 3.0f );
	}
}

bool cSoundSystem::Play2DSound( unsigned long index )
{
	if( IsUsableSoundSystem() == false )
		return false;


	/// 동일 프레임 중복 제거
	if( mOverlapTester.GetAt(index) != 0 )
		return true;

	sSoundInfo* pInfo = (sSoundInfo*)mSoundListMap.GetAt( index );
	if( pInfo == 0 )
		return false;

	NiAudioSource* snd = RESOURCEMAN->CloneTotalSoundByName( pInfo->mFileName.Cstr() );
	if( snd == 0 )
		return false;

	if( mActiveSound )
		snd->SetGain( pInfo->mVolume * OPTIONMAN->GetEffVolume() );
	else
		snd->SetGain( 0.0f );

	snd->SetLoopCount( 1 );

	snd->Play();

	sPlayInfo* playInfo = new sPlayInfo;
	playInfo->mVolume = pInfo->mVolume;
	playInfo->mSnd = snd;

	/// 사운드 맵에 등록
	mPlayList.Insert( mPlayList.End(), playInfo );

	mOverlapTester.Insert( index, index );

	return true;
}

void cSoundSystem::PlayAreaSound( NiAudioSource* snd )
{
	if( IsUsableSoundSystem() == false )
		return;

	if( mCurrentAreaSound )
		mCurrentAreaSound->FadeToGain( 0.0f, THEAPP->GetWorldAccumTime()*0.001f, 3.0f );

	mCurrentAreaSound = snd;
	if( mCurrentAreaSound )
	{
		mCurrentAreaSound->SetLoopCount( 1 );
		mCurrentAreaSound->Play();
		mCurrentAreaSound->SetGain( 0.0f );

		if( mActiveSound )
			mCurrentAreaSound->FadeToGain(  OPTIONMAN->GetEnvVolume(), THEAPP->GetWorldAccumTime()*0.001f, 3.0f );
	}

	if( mCurrentMapBGM )
		mCurrentMapBGM->FadeToGain( 0.0f, THEAPP->GetWorldAccumTime()*0.001f, 3.0f );
}

void cSoundSystem::ActiveSound( bool active )
{
	if( IsUsableSoundSystem() == false )
		return;

	/// 3D sound 제어
	if( SCENEMAN )
		SCENEMAN->ActiveSoundNodeAll( active );

	if( active == true )
	{
		if( mBGMSound )
			mBGMSound->SetGain( OPTIONMAN->GetEnvVolume() );

		if( mCurrentAreaSound )
			mCurrentAreaSound->SetGain( OPTIONMAN->GetEnvVolume() );

		if( mCurrentMapBGM )
			mCurrentMapBGM->SetGain( OPTIONMAN->GetEnvVolume() );

		if( mPlayList.IsEmpty() == false )
		{
			cPlay2DSoundList::cIterator i = mPlayList.Begin();
			cPlay2DSoundList::cIterator end = mPlayList.End();
			for( ; i != end; ++i )
			{
				sPlayInfo* info = (sPlayInfo*)*i;
				info->mSnd->SetGain( info->mVolume*OPTIONMAN->GetEffVolume() );
			}
		}
	}
	else
	{
		if( mBGMSound )
			mBGMSound->SetGain( 0.0f );

		if( mCurrentAreaSound )
			mCurrentAreaSound->SetGain( 0.0f );

		if( mCurrentMapBGM )
			mCurrentMapBGM->SetGain( 0.0f );

		if( mPlayList.IsEmpty() == false )
		{
			cPlay2DSoundList::cIterator i = mPlayList.Begin();
			cPlay2DSoundList::cIterator end = mPlayList.End();
			for( ; i != end; ++i )
			{
				sPlayInfo* info = (sPlayInfo*)*i;
				info->mSnd->SetGain( 0.0f );
			}
		}
	}

	mActiveSound = active;
}

void cSoundSystem::UpdateSoundGain()
{
	if( mBGMSound )
		mBGMSound->SetGain( OPTIONMAN->GetEnvVolume() );

	if( mCurrentAreaSound )
		mCurrentAreaSound->SetGain( OPTIONMAN->GetEnvVolume() );

	if( mCurrentMapBGM )
		mCurrentMapBGM->SetGain( OPTIONMAN->GetEnvVolume() );
}
