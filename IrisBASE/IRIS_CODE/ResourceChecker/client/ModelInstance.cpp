#include "StdAfx.h"
#include "ModelInstance.h"
#include "ResourceManager.h"
#include "KFMTool.h"

cModelInstance::cModelInstance()
: mpKFMTool(0)
{
}

cModelInstance::~cModelInstance()
{
	mSequenceDataMap.RemoveAll();
	mpKFMTool = 0;
}

cModelInstance* cModelInstance::Create( const char* pathName )
{
	assert( pathName );

	/// KFM 툴을 생성
	cKFMTool* kfmTool = NiNew cKFMTool;
	if( kfmTool->LoadFile( pathName ) != NiKFMTool::KFM_SUCCESS )
	{
		assert( 0 );
		MessageBoxA( NULL, pathName, "fail to load file", MB_OK | MB_ICONERROR );
		return 0;
	}

	/// 모델 인스턴스를 생성
	cModelInstance* modelInstance = new cModelInstance;

	/// Stream을 생성하고 모델 인스턴스 초기화
	NiStream* stream = NiNew NiStream;
	if ( modelInstance->Init( kfmTool, *stream ) == false )
	{
		delete modelInstance;
		delete stream;
		MessageBoxA( NULL, pathName, "fail to modelInstance Init", MB_OK | MB_ICONERROR );
		return NULL;
	}
	delete stream;

	return modelInstance;
}

bool cModelInstance::Init( NiKFMTool* kfmTool, NiStream& stream )
{
	mpKFMTool = kfmTool;
	if( kfmTool == 0 )
	{
		assert(0);
		return false;
	}

	NiDataStream::GetFactory()->SetCallback( NiDataStreamFactory::ForceCPUReadAccessCallback );

	/// Load NIF file
	const char* modelPath = mpKFMTool->GetFullModelPath();
	cString tempName = modelPath;
	tempName.TrimLeft( "./\\" );

	///
	if( RESOURCEMAN->LoadNIF( tempName ) == false )
	{
		return false;
	}

	///
	NiNode* pNode = RESOURCEMAN->GetObjectByName( tempName );
	if( pNode )
		CreatePickingData( pNode );

	NiNodeEx* dummyNode = NiNew NiNodeEx;
	dummyNode->SetScale( 1.0f );
	dummyNode->SetRotate( NiMatrix3::IDENTITY );
	dummyNode->SetName("head2_dummy");

	NiNode* pHead = (NiNode*)pNode->GetObjectByName("head_dummy");
	NiNode* pFoot = (NiNode*)pNode->GetObjectByName("foot_dummy");
	if( pHead && pFoot )
	{
		dummyNode->mNode = pHead;
		pFoot->AttachChild( dummyNode );
	}
	else
	{
		NiDelete dummyNode;
	}

	/// Load Sequences Info
	unsigned int* pSequenceIDs;
	unsigned int numIDs;
	mpKFMTool->GetSequenceIDs( pSequenceIDs, numIDs );

	unsigned int	eSequenceID = 0;
	char			lastFileNameKF[NI_MAX_PATH];
	bool			isLoadFileKF = false;

	lastFileNameKF[0] = '\0';
	for( unsigned int i = 0; i < numIDs; ++i )
	{
		eSequenceID = pSequenceIDs[i];

		/// Get KF filename.
		NiFixedString fileNameKF = mpKFMTool->GetFullKFFilename( eSequenceID );

		/// 구동상의 문제는 아니지만, 전체 데이터에 영향을 준다. 별도로 체크!!
		assert(fileNameKF.Exists());

		/// Determine whether or not to load file.
		isLoadFileKF = false;
		if( ::strcmp(lastFileNameKF, fileNameKF) != 0 )
		{
			isLoadFileKF = true;
			NiStrcpy( lastFileNameKF, NI_MAX_PATH, fileNameKF );
		}

		AddSequenceData( eSequenceID, stream, isLoadFileKF );
	}

	NiFree(pSequenceIDs);

	return true;
}

bool cModelInstance::AddSequenceData( unsigned int eSequenceID, NiStream& stream, bool isLoadFileKF )
{
	/// Get sequence information.
	NiKFMTool::Sequence* pKfmSequence = mpKFMTool->GetSequence( eSequenceID );
	if( !pKfmSequence )
	{
		return false;
	}

	/// Get KF filename.
	NiFixedString fileNameKF = mpKFMTool->GetFullKFFilename(eSequenceID);
	assert(fileNameKF.Exists());

	/// Load file, if specified.
	if( isLoadFileKF )
	{
		cFileLoader loader;
		if( loader.Open( (const char*)fileNameKF, true ) == false )
		{
			return false;
		}

		bool isSuccess = stream.Load( (char*)loader.GetBufferPtr(), loader.GetSize() );
		if ( isSuccess == false )
		{
			NILOG( NIMESSAGE_GENERAL_0, "cActorManagerForPartition: Failed to load KF File: %s\n", (const char*)fileNameKF );
			return false;
		}
	}

	// Note: as of Gamebryo 2.5.0, NiKFMTool was modified to store
	// filename/sequencename pairs instead of filename/index pairs to allow
	// for the possibility of sequences being added or removed in a DCC app
	// without having to modify the KFM file.  For backwards compatibility,
	// if there is no name, then we will use the animation index.
	NiSequenceDataPtr spSeqData;
	const NiFixedString& kSeqName = pKfmSequence->GetSequenceName();

	if (kSeqName.Exists())
	{
		spSeqData = NiSequenceData::CreateSequenceDataFromFile(stream,
			kSeqName);

		if (!spSeqData)
		{
			NILOG("NiActorManager: Failed to add "
				"sequence with name %s in %s\n", (const char*)kSeqName, (const char*) fileNameKF);
			return false;
		}
	}
	else
	{
		int iAnimIndex = pKfmSequence->GetAnimIndex();
		spSeqData =
			NiSequenceData::CreateSequenceDataFromFile(stream, iAnimIndex);
		if (!spSeqData)
		{
			NILOG("NiActorManager: Failed to add "
				"sequence at index %d in %s\n", iAnimIndex, (const char*) fileNameKF);
			return false;
		}
	}

	mSequenceDataMap.SetAt( eSequenceID, spSeqData );

	return true;
}

/// 주의사항 :
NiSequenceData* cModelInstance::CloneSequenceData( unsigned int eSequenceID )
{
	NiSequenceDataPtr sequenceData;
	mSequenceDataMap.GetAt( eSequenceID, sequenceData );

	if( sequenceData == NULL )
	{
		assert(0);
		return NULL;
	}

	NiCloningProcess cloning;
	cloning.m_eCopyType = NiObjectNET::COPY_EXACT;

	return NiDynamicCast( NiSequenceData, sequenceData->CreateClone(cloning) );
}

void cModelInstance::CreatePickingData( NiAVObject* pobj )
{
	NiBoundingVolume* pABV = 0;
	NiCollisionData* pData = NiGetCollisionData(pobj);
	if( pData )
	{
		pABV = pData->GetModelSpaceABV();
	}

	NiNode* pNode = NiDynamicCast( NiNode, pobj );
	if( pABV )
	{
		if( pNode )
		{
			NiMesh* pShape = 0;
			switch( pABV->Type() )
			{
			case NiBoundingVolume::BOX_BV:
				pShape = NiDrawableBV::CreateFromBox( ((NiBoxBV*)pABV)->GetBox(), 1.0f );
				break;
			case NiBoundingVolume::CAPSULE_BV:
				pShape = NiDrawableBV::CreateFromCapsule( ((NiCapsuleBV*)pABV)->GetCapsule(), 1.0f );
				break;
			case NiBoundingVolume::SPHERE_BV:
				pShape = NiDrawableBV::CreateFromSphere( ((NiSphereBV*)pABV)->GetSphere(), 1.0f );
				break;
			default:  // New BV type?
				break;
			}

			if( pShape == 0 )
			{
				assert(0);
				return;
			}

			NiCollisionData* pCollisionData = NiNew NiCollisionData(pShape);
			assert(pCollisionData);
			pCollisionData->SetCollisionMode(NiCollisionData::NOTEST);
			pCollisionData->SetPropagationMode(NiCollisionData::PROPAGATE_NEVER);

			pShape->SetName( "PickObj" );
			pNode->AttachChild( pShape );
		}
	}

	/// 자식에 대한 검사..
	if( pNode )
	{
		NiAVObject* pChild = 0;
		for( unsigned int i = 0; i < pNode->GetArrayCount(); i++ )
		{
			pChild = pNode->GetAt(i);
			if( pChild )
				CreatePickingData( pChild );
		}
	}
}