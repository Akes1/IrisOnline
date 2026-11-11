#include "StdAfx.h"
#include "ResourceManager.h"

#include "RenderSystem.h"
#include "Stream.h"
 #include "EngineFile.h"
// #include "ImageReader.h"
// #include "CallbackStreamPAK.h"
 
#include "FilePackSystem.h"

cResourceManager* cResourceManager::mpSingleton = 0;
NiCriticalSection cResourceManager::mObjectListCriticalSection;
NiCriticalSection cResourceManager::mSoundListCriticalSection;

NiFile* ResourceFileCreateFunc( const char *pcName, NiFile::OpenMode /*eMode*/, unsigned int /*uiBufferSize*/ )
{
	cEngineFile* file = NiNew cEngineFile( pcName );
	if( *file == true )
		return file;

	return 0;
}

bool ResourceFileAccessFunc( const char *pcName, NiFile::OpenMode eMode )
{
	return FILESYSTEM->FileExist( pcName );
}

cResourceManager::cResourceManager()
: mD3DTextureMap( 128 )
, mShadowTexture( 0 )
, mFieldTargetTexture( 0 )
, mStageImageTexture( 0 )
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	/// 초기값을 파일로 변경하면서 속도 측정
	//	mNodeArray.SetSize( 800 );
	//	mNodeArray.SetGrowBy( 50 );
	//	mModelArray.SetSize( 50 );
	//	mModelArray.SetGrowBy( 10 );

	/// 텍스처 팔레트를 생성
	mTexturePalette = NiNew cTexturePalette( 2048 );
	mMapTexturePalette = NiNew cTexturePalette( 2048 );

	///
// 	mpStreamPAK = NiNew cCallbackStreamPACK();
// 	mpStreamPAK->SetTexturePalette( mTexturePalette );
}

cResourceManager::~cResourceManager()
{
	mShadowTexture = 0;
	mFieldTargetTexture = 0;

	///
// 	mpStreamPAK->RemoveAllObjects();
// 	SAFE_NIDELETE(mpStreamPAK);


	mObjectListCriticalSection.Lock();
	mObjectMap.RemoveAll();
	mObjectListCriticalSection.Unlock();

	RemoveAllModels();

	mMapObjNameArray.Clear();

	mTexturePalette->RemoveAllTextures();
	mTexturePalette = 0;

	mStageImageTexture = 0;

	mMapTexturePalette->RemoveAllTextures();
	mMapTexturePalette = 0;

	///
	{
		cD3DTextureMap::cIterator i = mD3DTextureMap.Begin();
		cD3DTextureMap::cIterator iend = mD3DTextureMap.End();

		for( ; i != iend; ++i )
		{
			LPDIRECT3DTEXTURE9 tex = (LPDIRECT3DTEXTURE9)i->mSecond;
			tex->Release();
		}
		mD3DTextureMap.Clear();
	}

	mSoundListCriticalSection.Lock();
	if( mStageSound.IsEmpty() == false )
	{
		cSoundMap::cIterator i, end;
		NiAudioSource* p = 0;

		i = mStageSound.Begin();
		end = mStageSound.End();
		for( ; i != end; ++i )
		{
			p = (NiAudioSource*)(*i).mSecond;
			SAFE_NIDELETE( p );
		}
		mStageSound.Clear();
	}

	if( mTotalSound.IsEmpty() == false )
	{
		cSoundMap::cIterator i, end;
		NiAudioSource* p = 0;

		i = mTotalSound.Begin();
		end = mTotalSound.End();
		for( ; i != end; ++i )
		{
			p = (NiAudioSource*)(*i).mSecond;
			SAFE_NIDELETE( p );
		}
		mTotalSound.Clear();
	}
	mSoundListCriticalSection.Unlock();

	mPartsNameMap.RemoveAll();
	mDummyNameMap.RemoveAll();

	///
	mpSingleton = 0;
}

void cResourceManager::Clean()
{
 	mShadowTexture = 0;
 	mFieldTargetTexture = 0;

	mObjectListCriticalSection.Lock();
	mObjectMap.RemoveAll();
	mObjectListCriticalSection.Unlock();

	RemoveAllModels();

	mTexturePalette->RemoveAllTextures();
// 	mTexturePalette = 0;
 
 	mStageImageTexture = 0;

	for( unsigned int i = 0, end = mMapObjNameArray.GetSize(); i < end; ++i )
	{
		RemoveObject( mMapObjNameArray[i].Cstr() );
	}
	mMapObjNameArray.Clear();


	mMapTexturePalette->RemoveAllTextures();
//	mMapTexturePalette = 0;

	///
	{
		cD3DTextureMap::cIterator i = mD3DTextureMap.Begin();
		cD3DTextureMap::cIterator iend = mD3DTextureMap.End();

		for( ; i != iend; ++i )
		{
			LPDIRECT3DTEXTURE9 tex = (LPDIRECT3DTEXTURE9)i->mSecond;
			tex->Release();
		}
		mD3DTextureMap.Clear();
	}

	mSoundListCriticalSection.Lock();
	if( mStageSound.IsEmpty() == false )
	{
		cSoundMap::cIterator i, end;
		NiAudioSource* p = 0;

		i = mStageSound.Begin();
		end = mStageSound.End();
		for( ; i != end; ++i )
		{
			p = (NiAudioSource*)(*i).mSecond;
			SAFE_NIDELETE( p );
		}
		mStageSound.Clear();
	}

	if( mTotalSound.IsEmpty() == false )
	{
		cSoundMap::cIterator i, end;
		NiAudioSource* p = 0;

		i = mTotalSound.Begin();
		end = mTotalSound.End();
		for( ; i != end; ++i )
		{
			p = (NiAudioSource*)(*i).mSecond;
			SAFE_NIDELETE( p );
		}
		mTotalSound.Clear();
	}
	mSoundListCriticalSection.Unlock();

	mPartsNameMap.RemoveAll();
	mDummyNameMap.RemoveAll();
}

bool cResourceManager::Init()
{
	NiFile::SetFileAccessFunc( ResourceFileAccessFunc );
	NiFile::SetFileCreateFunc( ResourceFileCreateFunc );

	/// 그림자 텍스처를 로딩
	mShadowTexture = LoadTexture( "./Data/2DData/shadow.tga", false );
	assert( mShadowTexture );

	if( mShadowTexture == 0 )
		return false;

	mFieldTargetTexture = LoadTexture( "./Data/Effect/Eff_PC_FieldSkill_01.tga", false );
	assert( mFieldTargetTexture );

	if( mFieldTargetTexture == 0 )
		return false;

	/// parts
	mPartsNameMap.SetAt( ePART_HAIR,	"hair" );
	mPartsNameMap.SetAt( ePART_FACE,	"face" );
	mPartsNameMap.SetAt( ePART_BODY1,	"body1" );
	mPartsNameMap.SetAt( ePART_BODY2,	"body2" );
	mPartsNameMap.SetAt( ePART_HAND,	"hand" );
	mPartsNameMap.SetAt( ePART_FOOT,	"foot" );
	mPartsNameMap.SetAt( ePART_EAR,		"ear" );

	/// dummy
	mDummyNameMap.SetAt( eLINK_HEAD,	"head_dummy" );
	mDummyNameMap.SetAt( eLINK_BODY,	"body_dummy" );
	mDummyNameMap.SetAt( eLINK_FOOT,	"foot_dummy" );
	mDummyNameMap.SetAt( eLINK_RHAND,	"Rhand_dummy" );
	mDummyNameMap.SetAt( eLINK_LHAND,	"Lhand_dummy" );
	mDummyNameMap.SetAt( eLINK_LARM,	"Larm_dummy" );

	mDummyNameMap.SetAt( eLINK_HEAD2, "head2_dummy");

	mDummyNameMap.SetAt( eLINK_RWEAPON,	"weapon_dummy" );
	mDummyNameMap.SetAt( eLINK_LWEAPON,	"weaponL_dummy" );

	return true;
}

void cResourceManager::Close()
{
	mStageImageTexture = 0;

	for( unsigned int i = 0, end = mMapObjNameArray.GetSize(); i < end; ++i )
	{
		RemoveObject( mMapObjNameArray[i].Cstr() );
	}
	mMapObjNameArray.Clear();

	mMapTexturePalette->RemoveAllTextures();

	mSoundListCriticalSection.Lock();
	if( mStageSound.IsEmpty() == false )
	{
		cSoundMap::cIterator i, end;
		NiAudioSource* p = 0;

		i = mStageSound.Begin();
		end = mStageSound.End();
		for( ; i != end; ++i )
		{
			p = (NiAudioSource*)(*i).mSecond;
			SAFE_NIDELETE( p );
		}
		mStageSound.Clear();
	}
	mSoundListCriticalSection.Unlock();
}
 
// void cResourceManager::Exit()
// {
// 	/// 만약 쓰레드가 돌고 있다면 종료한다.
// 	FinishThread();
// }
 
NiTexture* cResourceManager::LoadTexture( const cString& pathName, bool useMipMap )
{
	/// 이미 존재하는지 검사
	NiTexture* nitex = 0;
	nitex = mTexturePalette->GetTexture( pathName.Cstr(), 0 );

	if( nitex == 0 )
		nitex = mTexturePalette->LoadTexture( pathName.Cstr() );

	return nitex;
}
 
LPDIRECT3DTEXTURE9 cResourceManager::LoadD3DTexture( const cString& pathName, bool useMipMap, DWORD filter, DWORD mipFilter )
{
	NiFilename filename( pathName.Cstr() );

	///
	cD3DTextureMap::cIterator i = mD3DTextureMap.Find( filename.GetFilename() );

	if( i != mD3DTextureMap.End() )
	{
		return (LPDIRECT3DTEXTURE9)i->mSecond;
	}

	/// 텍스처 로딩
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 );
		return 0;
	}

	LPDIRECT3DTEXTURE9 tex = 0;
	cRenderer* renderer = RENDERSYS->GetRenderer();
	HRESULT ret = ::D3DXCreateTextureFromFileInMemoryEx( renderer->GetD3DDevice(), loader.GetBufferPtr(), loader.GetSize(),
		0, 0, useMipMap ? 0 : 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, filter, mipFilter, 0, 0, 0,
		&tex );

	if( FAILED(ret) || tex == 0 )
	{
		assert( 0 );
		return 0;
	}

	/// 텍스처 맵에 추가
	mD3DTextureMap.Insert( filename.GetFilename(), tex );
	return tex;
}

// LPDIRECT3DTEXTURE9 cResourceManager::GetD3DTexture( const cString& pathName )
// {
// 	NiFilename filename( pathName.Cstr() );
// 
// 	cD3DTextureMap::cIterator i = mD3DTextureMap.Find( filename.GetFilename() );
// 
// 	return i != mD3DTextureMap.End() ? (LPDIRECT3DTEXTURE9)i->mSecond : 0;
// }

NiAudioSource* cResourceManager::LoadStageSound( const cString& fileName )
{
	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
	if( as == 0 )
		return 0;

	cString tempName = fileName;
	tempName.TrimLeft( "./\\" );

	/// 이미 로딩되었는지 검사
	NiAudioSource* src = GetStageSoundByName( tempName );
	if( src )
		return src;

	/// 엔진 노드를 생성
	src = as->CreateSource( NiAudioSource::TYPE_AMBIENT );

	cString str;
	str.Format( "./Sound/%s", fileName.Cstr() );
	src->SetFilename( str.Cstr() );

	NiFilename filename( tempName.Cstr() );
	if( _strcmpi( filename.GetExt(), ".mp3") == 0 )
	{
		src->SetStreamed( true );
		src->SetAllowSharing( false );
	}
	else
	{
		src->SetStreamed( false );
		src->SetAllowSharing( true );
	}

	/// 사운드를 로딩
	if( src->Load() == false )
	{
		assert( 0 );
		return 0;
	}

	mSoundListCriticalSection.Lock();
	{
		if( mStageSound.Insert( filename.GetFilename(), src ) == false )
		{
			assert(0);
			mSoundListCriticalSection.Unlock();
			return 0;
		}
	}
	mSoundListCriticalSection.Unlock();

	return src;

}
 
NiAudioSource* cResourceManager::LoadTotalSound( const cString& fileName )
{
	NiAudioSystem* as = NiAudioSystem::GetAudioSystem();
	if( as == 0 )
		return 0;

	cString tempName = fileName;
	tempName.TrimLeft( "./\\" );

	/// 이미 로딩되었는지 검사
	NiAudioSource* src = GetTotalSoundByName( tempName );
	if( src )
		return src;

	/// 엔진 노드를 생성
	src = as->CreateSource( NiAudioSource::TYPE_AMBIENT );

	cString str;
	str.Format( "./Sound/%s", fileName.Cstr() );
	src->SetFilename( str.Cstr() );

	NiFilename filename( tempName.Cstr() );
	if( _strcmpi( filename.GetExt(), ".mp3") == 0 )
	{
		src->SetStreamed( true );
		src->SetAllowSharing( false );
	}
	else
	{
		src->SetStreamed( false );
		src->SetAllowSharing( true );
	}

	/// 사운드를 로딩
	if( src->Load() == false )
	{
		assert( 0 );
		return 0;
	}

	mSoundListCriticalSection.Lock();
	{
		if( mTotalSound.Insert( filename.GetFilename(), src ) == false )
		{
			assert(0);
			mSoundListCriticalSection.Unlock();
			return 0;
		}
	}
	mSoundListCriticalSection.Unlock();

	return src;
}

bool cResourceManager::LoadNIF( const cString& pathName, NiAVObject** nifRoot )
{
	cString tempName = pathName;
	tempName.TrimLeft( "./\\" );

	/// 이미 로딩되었는지 검사
	NiNode* n = GetObjectByName( tempName );
	if( n )
	{
		if( nifRoot )
			*nifRoot = n;
		return true;
	}

#ifdef _DEBUG
	cString check;
	check.Format("\n%s\n", pathName.Cstr() );
	//OutputDebugStringA(check.Cstr());
#endif

	NiDataStream::GetFactory()->SetCallback( NiDataStreamFactory::ForceCPUReadAccessCallback );

	///
	cFileLoader loader;
	if( loader.Open( tempName.Cstr(), true ) == false )
	{
		MessageBoxA( NULL, pathName.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	///
	cStream stream( tempName.Cstr(), mTexturePalette );
	if( stream.Load( (char*)loader.GetBufferPtr(), loader.GetSize() ) == false )
	{
		MessageBoxA( NULL, pathName.Cstr(), "fail to stream load", MB_OK | MB_ICONERROR );
		return false;
	}

	NiAVObject* obj = NiDynamicCast(NiAVObject, stream.GetObjectAt( 0 ));
	if( NiIsKindOf(NiNode, obj) )
	{
		n = NiDynamicCast( NiNode, obj );
	}
	else
	{
		n = NiTCreate<NiNode>(); 

		n->SetSelectiveUpdate(true);
		n->SetSelectiveUpdateTransforms(true);
		n->SetSelectiveUpdatePropertyControllers(true);
		n->SetSelectiveUpdateRigid(false);


		n->SetName( ((NiAVObject*)obj)->GetName() );
		n->AttachChild( (NiAVObject*)obj );

		n->Update(0.0f);
		NiMesh::CompleteSceneModifiers(n);
		n->UpdateProperties();
		n->UpdateEffects();
		n->UpdateNodeBound();
	}

	if( n == 0 )
	{
		assert(0);
		return false;
	}
	AddObject( n, tempName.Cstr(), false );

	stream.RemoveAllObjects();

	if( nifRoot )
		*nifRoot = n;

	CheckStreams( n );

	return true;
}
 
bool cResourceManager::LoadMapNIF( const cString& pathName )
{
	cString tempName = pathName;
	tempName.TrimLeft( "./\\" );

	/// 이미 로딩되었는지 검사
	if( GetObjectByName( tempName ) )
	{
		return true;
	}

#ifdef _DEBUG
	cString check;
	check.Format("\n%s\n", pathName.Cstr() );
	//OutputDebugStringA(check.Cstr());
#endif

	NiDataStream::GetFactory()->SetCallback( NiDataStreamFactory::ForceCPUReadAccessCallback );

	///
	cFileLoader loader;
	if( loader.Open( tempName.Cstr(), true ) == false )
		return false;

	///
	cStream stream( tempName.Cstr(), mMapTexturePalette );
	if( stream.Load( (char*)loader.GetBufferPtr(), loader.GetSize() ) == false )
		return false;

	NiNode* n = 0;
	NiAVObject* obj = NiDynamicCast(NiAVObject, stream.GetObjectAt( 0 ));
	if( NiIsKindOf(NiNode, obj) )
	{
		n = NiDynamicCast( NiNode, obj );
	}
	else
	{
		n = NiTCreate<NiNode>(); 

		n->SetSelectiveUpdate(true);
		n->SetSelectiveUpdateTransforms(true);
		n->SetSelectiveUpdatePropertyControllers(true);
		n->SetSelectiveUpdateRigid(false);


		n->SetName( ((NiAVObject*)obj)->GetName() );
		n->AttachChild( (NiAVObject*)obj );

		n->Update(0.0f);
		NiMesh::CompleteSceneModifiers(n);
		n->UpdateProperties();
		n->UpdateEffects();
		n->UpdateNodeBound();
	}

	if( n == 0 )
	{
		assert(0);
		return false;
	}

	AddObject( n, tempName.Cstr(), true );

	stream.RemoveAllObjects();

	CheckStreams( n );

	return true;
}
 
NiTexture* cResourceManager::LoadMapTexture( const cString& pathName, bool useMipMap )
{
	/// 이미 존재하는지 검사
	NiTexture* nitex = 0;
	nitex = mMapTexturePalette->GetTexture( pathName.Cstr(), 0 );

	if( nitex == 0 )
		nitex = mMapTexturePalette->LoadTexture( pathName.Cstr() );

	return nitex;
}
 
// /// 주의!! 일단, 로딩큐에 파일명만을 채운다.
// void cResourceManager::LoadThreadNIF( const char* fileName )
// {
// 	if( !GetObjectByName( fileName ) )
// 	{
// 		mpStreamPAK->AddLoadFile( fileName );
// 	}
// }
 
cModelInstance* cResourceManager::LoadKFM( const cString& pathName )
{
	cString tempName = pathName;
	tempName.TrimLeft( "./\\" );

	/// 이미 로딩되었는지 검사
	cModelInstance* model = GetModelByName( tempName );
	if( model )
		return model;

	///
	model = cModelInstance::Create( tempName.Cstr() );
	if( model == 0 )
	{
		MessageBoxA( NULL, pathName.Cstr(), "fail to Create ModelInstance", MB_OK | MB_ICONERROR );
		return 0;
	}

	AddModel( model, tempName );

	return model;
}
 
// /// 주의!! 로딩쓰레드와 메인쓰레드에서 동시에 호출되므로 사용시 항상 주의
void cResourceManager::AddObject( NiNode* n, const cString& pathName, bool mapObj )
{
	if( n == 0 ) 
	{
		assert( 0 );
		return;
	}

	NiFilename filename( pathName.Cstr() );

	mObjectListCriticalSection.Lock();
	{
		mObjectMap.SetAt( filename.GetFilename(), n );

		if( mapObj == true )
			mMapObjNameArray.PushBack( filename.GetFilename() );
	}
	mObjectListCriticalSection.Unlock();
}

void cResourceManager::AddModel( cModelInstance* model, const cString& pathName )
{
	if( model == 0 ) 
	{
		assert( 0 );
		return;
	}

	NiFilename filename(pathName.Cstr());
	mModelMap.SetAt(filename.GetFilename(), model );
}

/// 주의!! 로딩쓰레드와 메인쓰레드에서 동시에 호출되므로 사용시 항상 주의
void cResourceManager::RemoveObject( const char* objectName )
{
	assert(::strlen(objectName));
	if( ::strlen(objectName) == 0 )
	{
		return;
	}

	NiFilename filename( objectName );

	mObjectListCriticalSection.Lock();
	if( mObjectMap.RemoveAt( filename.GetFilename() ) == false )
	{
//		assert(0);
	}
	mObjectListCriticalSection.Unlock();
}

void cResourceManager::RemoveAllModels()
{
	cModelInstance* pModel = 0;
	const char* pcName;
	NiTMapIterator kIter = mModelMap.GetFirstPos();
	while (kIter)
	{
		mModelMap.GetNext(kIter, pcName, pModel);
		SAFE_NIDELETE( pModel );
	}
	mModelMap.RemoveAll();
}
 
NiAudioSource* cResourceManager::GetStageSoundByName( const cString& fileName )
{
	NiFilename filename( fileName.Cstr() );

	NiAudioSource* src = 0;
	mSoundListCriticalSection.Lock();
	{
		src = mStageSound.GetAt( filename.GetFilename() );
	}
	mSoundListCriticalSection.Unlock();

	return src;
}
 
NiAudioSource* cResourceManager::GetTotalSoundByName( const cString& fileName )
{
	NiFilename filename( fileName.Cstr() );

	NiAudioSource* src = 0;
	mSoundListCriticalSection.Lock();
	{
		src = mTotalSound.GetAt( filename.GetFilename() );
	}
	mSoundListCriticalSection.Unlock();

	return src;
}


NiNode* cResourceManager::GetObjectByName( const cString& pathName )
{
	NiFilename filename( pathName.Cstr() );

	NiNodePtr n = 0;
	mObjectListCriticalSection.Lock();
	if( mObjectMap.GetAt( filename.GetFilename(), n ) )
	{
		mObjectListCriticalSection.Unlock();
		return n;
	}

	mObjectListCriticalSection.Unlock();
	return 0;
}
 
cModelInstance* cResourceManager::GetModelByName( const cString& pathName )
{
	NiFilename filename( pathName.Cstr() );

	cModelInstance* model = 0;
	if( mModelMap.GetAt( filename.GetFilename(), model ) )
		return model;

	return 0;
}
 
NiAudioSource* cResourceManager::CloneStageSoundByName( const cString& fileName )
{
	NiAudioSource* src = LoadStageSound(fileName);
	if( src == 0 )
		return 0;

	NiCloningProcess cloning;
	cloning.m_eCopyType = NiObjectNET::COPY_EXACT;

	return NiDynamicCast( NiAudioSource, src->CreateClone(cloning) );
}

NiAudioSource* cResourceManager::CloneTotalSoundByName( const cString& fileName )
{
	NiAudioSource* src = LoadTotalSound( fileName );
	if( src == 0 )
		return 0;

	NiCloningProcess cloning;
	cloning.m_eCopyType = NiObjectNET::COPY_EXACT;

	return NiDynamicCast( NiAudioSource, src->CreateClone(cloning) );
}
 
NiNode* cResourceManager::CloneObjectByName( const cString& pathName )
{
	NiNode* n = GetObjectByName( pathName );

	if( n )
	{
		NiCloningProcess cloning;
		cloning.m_eCopyType = NiObjectNET::COPY_EXACT;
		cloning.m_cAppendChar = '%';

		return NiDynamicCast( NiNode, n->Clone(cloning) );
	}

	return 0;
}

NiNodePtr cResourceManager::DeepCopyObjectByName( const cString& pathName )
{
	cString tempName = pathName;
	tempName.TrimLeft( "./\\" );

	NiNode* n = GetObjectByName( tempName );
	if( n )
	{
		(void)NiMemMarker(NI_MEM_MARKER_END, __FUNCTION__, this);

		cStream stream( tempName.Cstr(), mTexturePalette );
		stream.InsertObject( n );

		/* stream object to memory block */
		char* pBuffer = 0;
		int iBufferSize = 0;
		NIVERIFY(stream.Save(pBuffer, iBufferSize));

		/* stream memory block back to object */
		stream.Load(pBuffer, iBufferSize);
		NiObjectPtr spCopy = stream.GetObjectAt(0);
		NiFree(pBuffer);

#if defined(NI_MEMORY_DEBUGGER)
		(void)NiMemMarker(NI_MEM_MARKER_END, __FUNCTION__, this);
#endif

		return NiDynamicCast(NiNode,spCopy);
	}

	return 0;
}

// void cResourceManager::Process()
// {
// 	/// 쓰레드 로딩 루틴..
// 	mpStreamPAK->Process();
// }
// 
// void cResourceManager::FinishThread()
// {
// 	mpStreamPAK->ClearQueue();
// 
// 	NiStream::LoadState loadState;
// 	NiStream::ThreadStatus status;
// 	status = mpStreamPAK->BackgroundLoadPoll( &loadState );
// 	while( status != NiStream::IDLE )
// 	{
// 		/// 쓰레드 종료를 좀더 효율적으로 하기위해서 필요..
// 		NiSleep( 1 ); 
// 
// 		status = mpStreamPAK->BackgroundLoadPoll( &loadState );
// 	}
// 
// 	mpStreamPAK->BackgroundLoadFinish();
// 	mpStreamPAK->RemoveAllObjects();
// }
// 
// 
// void cResourceManager::ChangeStageImageTexture( NiTexture* tex )
// {
// 	if( mStageImageTexture )
// 	{
// 		assert(mStageImageTexture->GetRefCount() == 1);
// 		mStageImageTexture = 0;
// 	}
// 
// 	mStageImageTexture = tex;
// }
 
const char* cResourceManager::GetManagedPartName( unsigned int partIdx )
{
	const char* partsName = NULL;
	if( mPartsNameMap.GetAt( partIdx, partsName ) == false )
		return "";

	return partsName;
}
 
const char* cResourceManager::GetManagedDummyName( unsigned int dummyIdx )
{
	const char* dummyName = NULL;
	if( mDummyNameMap.GetAt( dummyIdx, dummyName ) == false )
		return "";

	return dummyName;
}
 
void cResourceManager::CheckStreams( NiAVObject* obj )
{
	if( NiIsKindOf( NiMesh, obj) == true )
	{
		NiMesh* mesh = (NiMesh*)obj;
		for( unsigned int i=0, iend = mesh->GetStreamRefCount(); i<iend; ++i )
		{
			NiDataStreamRef* ref = mesh->GetStreamRefAt(i);
			if( ref )
			{
				NiDX9DataStream* stream = NiDynamicCast( NiDX9DataStream, ref->GetDataStream() );
				if( stream )
				{
					NiUInt8 mask = stream->GetAccessMask();
					mask |= NiDataStream::ACCESS_CPU_READ;
					stream->SetAccessMask( mask );
				}
			}
		}
	}
	else if( NiIsKindOf(NiNode, obj) == true )
	{
		NiNode* node = (NiNode*)obj;
		for( unsigned int i=0, iend = node->GetChildCount(); i<iend; ++i )
		{
			NiAVObject* child = node->GetAt(i);
			if( child )
				CheckStreams( child );
		}
	}
}
