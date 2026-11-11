#include "stdafx.h"
#include "SoundSystem.h"

#include "Application.h"
#include "ResourceManager.h"
#include "SceneManager.h"

cSoundSystem* cSoundSystem::mSingleton = 0;

cSoundSystem::cSoundSystem()
: mAreaSoundMap( 256 )
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;

	mActiveSound = true;
}

cSoundSystem::~cSoundSystem() 
{
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
		NiMessageBox( "Failed to Start AudioSystem", "Error" );
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
	as->SetUnitsPerMeter( 100.0f );

	if( LoadSoundList() == false )
		return false;

	return true;
}

void cSoundSystem::Close()
{
	if( mPlayList.IsEmpty() == false )
	{
		cPlay2DSoundList::cIterator i = mPlayList.Begin();
		cPlay2DSoundList::cIterator end = mPlayList.End();
		for( ; i != end; ++i )
		{
			NiAudioSource* o = (NiAudioSource*)*i;
			SAFE_NIDELETE(o);
		}
		mPlayList.Clear();
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

	mDelList.Clear();
}

void cSoundSystem::StopAll()
{
	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();

	if( as )
		as->StopAllSources();
}

void cSoundSystem::Process( unsigned long /*deltaTime*/, unsigned long /*accumTime*/ )
{
	/// 2d Sound process
	if( mPlayList.IsEmpty() == false )
	{
		cPlay2DSoundList::cIterator i = mPlayList.Begin();
		cPlay2DSoundList::cIterator end = mPlayList.End();
		for( ; i != end; ++i )
		{
			NiAudioSource* o = (NiAudioSource*)*i;
			if( o )
			{
				if( o->GetStatus() != NiAudioSource::PLAYING )
				{
					/// 삭제 등록
					mDelList.Insert( mDelList.End(), o );
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
			NiAudioSource* o = (NiAudioSource*)*i;

			mPlayList.Remove( o );
			SAFE_NIDELETE(o);
		}
		mDelList.Clear();
	}	
}

NiAudioSource* cSoundSystem::LoadAreaSound( unsigned int index, const cString& pathName, bool streamed )
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
	snd->SetStreamed( streamed );
	snd->SetFilename( pathName.Cstr() );
	snd->SetAllowSharing( true );

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

bool cSoundSystem::Play2DSound( unsigned long index )
{
	sSoundInfo* pInfo = (sSoundInfo*)mSoundListMap.GetAt( index );
	if( pInfo == 0 )
		return false;

	NiAudioSource* snd = RESOURCEMAN->CloneTotalSoundByName( pInfo->mFileName.Cstr() );
	if( snd == 0 )
		return false;

	if( mActiveSound )
		snd->SetGain( 1.0f );
	else
		snd->SetGain( 0.0f );

	snd->SetLoopCount( 1 );

	snd->Play();

	/// 사운드 맵에 등록
	mPlayList.Insert( mPlayList.End(), snd );

	return true;
}

void cSoundSystem::ActiveSound( bool active )
{
	mActiveSound = active;
	if( active == true )
	{
		if( mPlayList.IsEmpty() == false )
		{
			cPlay2DSoundList::cIterator i = mPlayList.Begin();
			cPlay2DSoundList::cIterator end = mPlayList.End();
			for( ; i != end; ++i )
			{
				NiAudioSource* o = (NiAudioSource*)*i;
				o->SetGain( 1.0f );
			}
		}
	}
	else
	{
		if( mPlayList.IsEmpty() == false )
		{
			cPlay2DSoundList::cIterator i = mPlayList.Begin();
			cPlay2DSoundList::cIterator end = mPlayList.End();
			for( ; i != end; ++i )
			{
				NiAudioSource* o = (NiAudioSource*)*i;
				o->SetGain( 0.0f );
			}
		}
	}
}

/*
void cSoundSystem::Process( float accumTime )
{
NiMilesAudioSystem* as = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
//NiMilesListener* listener = as->GetListener();
//listener->Update();

// Update the sound system.
// 'True' parameter indicates that all sources should be updated.
//as->Update( time, false );
as->Update( accumTime, true );
}
*/

/*
#include "Lexer.h"
#include "Parser.h"

cSoundSystem* cSoundSystem::mpSingleton = 0;

cSoundSystem::cSoundSystem()
//: mpCurrentAmbient(0)
{
assert( mpSingleton == 0 && "bad singleton!" );
mpSingleton = this;

mBackgroundArray.Reserve( 128 );
mEffectArray.Reserve( 512 );
}

cSoundSystem::~cSoundSystem() 
{
Clear();

// Shutdown the Sound System
NiAudioSystem::GetAudioSystem()->Shutdown();

mpSingleton = 0;
}

NiAVObject* cSoundSystem::GetListener()
{
return NiAudioSystem::GetAudioSystem()->GetListener();
}

void cSoundSystem::Clear()
{
for( unsigned int i = 0, end = mBackgroundArray.GetSize(); i < end; ++i )
{
NiDelete (NiAudioSource*)mBackgroundArray[i];
}
for( unsigned int i = 0, end = mEffectArray.GetSize(); i < end; ++i )
{
NiDelete (NiAudioSource*)mEffectArray[i];
}

mBackgroundArray.Clear();
mEffectArray.Clear();
}

void cSoundSystem::Exit()
{
Clear();

// Shutdown the Sound System
NiAudioSystem::GetAudioSystem()->Shutdown();
}

bool cSoundSystem::Init( NiWindowRef hwnd )
{
NiMilesAudioSystem* as = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();

as->SetHWnd(hwnd);

// Put the path to the Miles redist files here.
if( as->Startup( "Temp_New" ) == false )
{
assert( 0 && "failed to start audio system" );
return false;
}

// Get a provider by iterating through the provider list looking first for
// "DirectSound3D Hardware Support" (hardware accelerated) and, if it's
// unavailable, then for "Miles Fast 2D Positional Audio" (software
// emulated), as "best choice" defaults.  If neither provider is
// available, report an error and return false.
NiTListIterator i;
NiProviderInfo* provider = as->GetFirstProvider(i);

while( provider )
{
if( strstr(provider->GetProviderName(), "DirectSound3D Hardware Support") )
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
if( strstr(provider->GetProviderName(), "Miles Fast 2D Positional Audio") )
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

as->SetBestSpeakerTypeAvailable();
as->SetUnitsPerMeter( 100.f );

/// test
StopAllSounds();

return true;
}

void cSoundSystem::Process( float /*time )
{
/*
if( mAmbientArray.IsEmpty() == false && mpCurrentAmbient )
{
bool respawn = false;

switch( mpCurrentAmbient->GetStatus() )
{
case NiAudioSource::DONE:
case NiAudioSource::STOPPED:
respawn = true;
break;
}

if( respawn )
{
unsigned int numAmbients = mAmbientArray.GetSize();
if( numAmbients > 2 )
{
unsigned int i = rand() % numAmbients;
mpCurrentAmbient = mAmbientArray[i];
}
mpCurrentAmbient->Play();
}
}

//NiMilesAudioSystem* as = (NiMilesAudioSystem*)NiAudioSystem::GetAudioSystem();
////NiMilesListener* listener = as->GetListener();
////listener->Update();

//// Update the sound system.
//// 'True' parameter indicates that all sources should be updated.
////as->Update( time, false );
//as->Update( time, true );
}

bool cSoundSystem::LoadBackground( const cString& pathName )
{
/// 지우기
Clear();

/// 파일 열기
cFileLoader loader;

if( loader.Open( pathName, true ) == false )
{
return false;
}

/// 렉서에 밥주고 파싱
cToken token;
cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
cParser parser( &lexer, pathName );

if( parser.ExpectTokenString( "sounds" ) == false )
return false;

if( parser.ExpectTokenString( "{" ) == false )
return false;

cString tempName;

while( lexer.IsEnd() == false )
{
lexer.GetNextToken( &token );

if( token == "}" )
break;
if( token.mType == eTOKEN_ERROR )
return false;
if( token.mType == eTOKEN_NULL )
continue;

token.ToInt();
parser.ParseString( &token );
tempName = "./Temp_New/";
tempName += token;

if( LoadAmbient( tempName, true ) == false )
{
assert( 0 && "failed to load sound" );
return false;
}
}
return true;
}

bool cSoundSystem::LoadEffect( const cString& pathName )
{
/// 지우기
Clear();

/// 파일 열기
cFileLoader loader;

if( loader.Open( pathName, true ) == false )
{
return false;
}

/// 렉서에 밥주고 파싱
cToken token;
cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
cParser parser( &lexer, pathName );

if( parser.ExpectTokenString( "sounds" ) == false )
return false;

if( parser.ExpectTokenString( "{" ) == false )
return false;

cString tempName;

while( lexer.IsEnd() == false )
{
lexer.GetNextToken( &token );

if( token == "}" )
break;
if( token.mType == eTOKEN_ERROR )
return false;
if( token.mType == eTOKEN_NULL )
continue;

token.ToInt();
parser.ParseString( &token );
tempName = "./Temp_New/";
tempName += token;

if( LoadAmbient( tempName, false ) == false )
{
assert( 0 && "failed to load sound" );
return false;
}
}
return true;
}

bool cSoundSystem::LoadAmbient( const cString& pathName, bool bgm )
{
NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
NiAudioSource* o = as->CreateSource( NiAudioSource::TYPE_AMBIENT );

o->SetFilename( pathName.Cstr() );
o->SetStreamed( bgm );
o->SetAllowSharing( true );

/// 사운드를 로딩
if( o->Load() == false )
{
assert( 0 && "failed to load sound" );
return false;
}

/// 속성을 설정
o->SetGain( 1.f );
o->SetLoopCount( 1 );
//o->Update( 0.f );

if( bgm )
mBackgroundArray.PushBack( o );
else
mEffectArray.PushBack( o );

return true;
}

//void cSoundSystem::ToggleSounds()
//{
//	assert(mpCurrentAmbient);
//
//	NiAudioSource::Status eStatus = mpCurrentAmbient->GetStatus();
//	if (eStatus == NiAudioSource::PLAYING)
//		mpCurrentAmbient->Stop();
//	else // if (eStatus == NiAudioSource::STOPPED)
//		mpCurrentAmbient->Play();
//}

void cSoundSystem::PlayEffect( unsigned int i )
{
if( i >= mEffectArray.GetSize() )
return;

NiAudioSource* o = (NiAudioSource*)mEffectArray[i];

//if( o->GetStatus() != NiAudioSource::PLAYING )
{
o->Update( 0.f );
o->Play();
}
}

void cSoundSystem::StopAllSounds()
{
//NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
NiAudioSystem::GetAudioSystem()->StopAllSources();
}

//void cSoundSystem::RewindAmbientSound()
//{
//	mpCurrentAmbient->Rewind();
//}
//void cSoundSystem::AttachGunFire(NiNode* pkNode)
//{
//	if (pkNode)
//	{
//		pkNode->AttachChild(m_spGunFireSound);
//	}
//	else
//	{
//		// Detach
//		if (m_spGunFireSound->GetParent())
//			m_spGunFireSound->GetParent()->DetachChild(m_spGunFireSound);
//	}
//}
//void cSoundSystem::PlayGunFire()
//{
//	m_spGunFireSound->Play();
//}
//void cSoundSystem::StopGunFire()
//{
//	NiAudioSource::Status eStatus = m_spGunFireSound->GetStatus();
//	if (eStatus == NiAudioSource::PLAYING)
//		m_spGunFireSound->Stop();
//}
//void cSoundSystem::SetAmbientGain(float fGain)
//{
//	assert(mpCurrentAmbient);
//	
//	mpCurrentAmbient->SetGain(fGain);
//}
*/
