#include "stdafx.h"
#include "KFMTool.h"

#include "EngineFile.h"

static const char* g_pcCurrentKFMVersion = "20.6.0.0";

static void CorrectPathSlash( NiFixedString& kFixedString )
{
	const char* pcString = kFixedString;

	if( kFixedString.GetLength() == 0 )
	{
		assert(0);
		return;
	}

	const char* pcLast = pcString;       
	if (pcLast[0] == NI_PATH_DELIMITER_CHAR)
	{
		// Find the last consecutive slash
		while (1)
		{
			if (pcLast[1] == NI_PATH_DELIMITER_CHAR)
				pcLast++;
			break;
		}

		bool bNetworkPath = false;
		if (pcLast - pcString == 1)
		{
			// There are two slashes, so this is possibly a network path.
			// If another slash occurs, it'll be assumed that this is a 
			// network path. If no other slash occurs, it'll be assumed that
			// this was a malformed path name.
			if (pcLast != strrchr(pcLast, NI_PATH_DELIMITER_CHAR))
			{
				bNetworkPath = true;
			}
		}

		if (bNetworkPath)
			pcLast = &pcString[0];
		else 
			pcLast++;
	}

	NiFixedString kReleaseAtEndOfScope = kFixedString;
	kFixedString = pcLast;
}

cKFMTool::KFM_RC cKFMTool::LoadFile( const cString& pathName )
{
	char acStdFilename[NI_MAX_PATH];
	NiStrcpy( acStdFilename, NI_MAX_PATH, pathName.Cstr() );
	NiPath::Standardize(acStdFilename);

	NiDataStream::GetFactory()->SetCallback( NiDataStreamFactory::ForceCPUReadAccessCallback );

	/// 파일을 로딩
	/*
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
	return KFM_ERR_FILE_IO;
	}

	cEngineFile file( loader.GetFile() );
	*/
	cEngineFile file( pathName.Cstr() );
	if( file == false )
		return KFM_ERR_FILE_IO;

	// Set file to read as little endian
	bool bPlatformLittle = NiSystemDesc::GetSystemDesc().IsLittleEndian();
	file.SetEndianSwap(!bPlatformLittle);

	// Read file header in little endian.
	char acBuf[256];
	file.GetLine(acBuf, 256);
	const unsigned int uiVerLength = 16;
	char acVersion[uiVerLength];
	unsigned int uiVersionEnd = 3;
	bool bBinary = false;

	if (strncmp(acBuf, ";Gamebryo KFM File Version ", 27) == 0)
	{
		unsigned int uiLen = strlen(acBuf);
		bBinary = (acBuf[uiLen-1] == 'b');
		// binary files have a 'b' on the end, so one fewer character
		uiVersionEnd = uiLen - (bBinary ? 28 : 27);
		assert (uiVersionEnd < uiVerLength);
		NiStrncpy(acVersion, uiVerLength, &acBuf[27], uiVersionEnd);
	}
	else
	{
		return KFM_ERR_FILE_VERSION;
	}
	acVersion[uiVersionEnd] = '\0';

	// Check version.
	bool bOldVersion = false;
	unsigned int uiVersion = NiStream::GetVersionFromString(acVersion);
	if( uiVersion < NiStream::GetVersion(1, 2, 0, 0) )
	{
		bOldVersion = true;
	}
	else if (uiVersion > NiStream::GetVersionFromString(
		g_pcCurrentKFMVersion))
	{
		return KFM_ERR_FILE_VERSION;
	}

	// Store base KFM path.
	char* pcPtr = strrchr(acStdFilename, NI_PATH_DELIMITER_CHAR);
	if (pcPtr)
	{
		pcPtr++;
		*pcPtr = '\0';
		SetBaseKFMPath(acStdFilename);
	}
	else
	{
		SetBaseKFMPath(NULL);
	}

	KFM_RC eResult;
	if ( bOldVersion)
	{
		eResult = ReadOldVersionAscii(file, uiVersion);
	}
	else if (bBinary)
	{
		// By default, binary files are little endian
		bool bFileLittle = true;
		// Read endianness byte, if streamed
		if (uiVersion >= NiStream::GetVersion(1,2,6,0))
		{
			StreamLoadBinary(file, bFileLittle);

			if (bFileLittle != bPlatformLittle)
			{
				if (NiBinaryStream::GetEndianMatchHint())
				{
					return KFM_ERR_ENDIAN_MISMATCH;
				}
				else
				{
					assert(0);
					// Give warning to the user
					//					OutputDebugStringA("Warning: ");
					//					OutputDebugStringA(pathName.Cstr());
					//					OutputDebugStringA(" has to be endian swapped.\n");
				}
			}
		}
		file.SetEndianSwap(bFileLittle != bPlatformLittle);

		eResult = ReadBinary(file, uiVersion);
	}
	else
	{
		eResult = ReadAscii(file, uiVersion);
	}

	return eResult;
}

cKFMTool::KFM_RC cKFMTool::ReadBinary( cEngineFile& file, unsigned int version )
{
	cTransition* defaultSyncTrans = (cTransition*)m_pkDefaultSyncTrans;
	cTransition* defaultNonSyncTrans = (cTransition*)m_pkDefaultNonSyncTrans;

	// Note: this function assumes endianness of kFile has already been set.
	char* pcDefaultNIFPath = NULL;
	char* pcDefaultKFPath = NULL;
	if (version < NiStream::GetVersion(1, 2, 3, 0))
	{
		NiBool bDefaultPaths;
		StreamLoadBinary(file, bDefaultPaths);
		if (bDefaultPaths != 0)
		{
			LoadCString(file, pcDefaultNIFPath);
			LoadCString(file, pcDefaultKFPath);
		}
	}

	LoadCStringAsFixedString(file, m_kModelPath);   
	NiStandardizeFilePath(m_kModelPath);
	if (version < NiStream::GetVersion(2, 2, 0, 0))
	{
		CorrectPathSlash(m_kModelPath);
	}

	if (pcDefaultNIFPath)
	{
		// Convert old default paths.
		char acModelPath[NI_MAX_PATH];
		NiStrcpy(acModelPath, NI_MAX_PATH, pcDefaultNIFPath);
		NiStrcat(acModelPath, NI_MAX_PATH, m_kModelPath);
		SetModelPath(acModelPath);
	}

	if (version < NiStream::GetVersion(2, 1, 0, 0))
	{
		LoadCStringAsFixedString(file, m_kModelRoot);
	}
	else
	{
		LoadFixedString(file, m_kModelRoot);
	}

	if (version >= NiStream::GetVersion(1, 2, 2, 0))
	{
		// Load default transition settings.
		BinaryStreamLoadEnum(file, &defaultSyncTrans->m_eType);
		BinaryStreamLoadEnum(file, &defaultNonSyncTrans->m_eType);
		StreamLoadBinary(file, defaultSyncTrans->m_fDuration);
		StreamLoadBinary(file, defaultNonSyncTrans->m_fDuration);
	}

	// Load sequences.
	unsigned int uiNumSequences;
	StreamLoadBinary(file, uiNumSequences);
	unsigned int ui;
	for (ui = 0; ui < uiNumSequences; ui++)
	{
		cSequence* pkSequence = (cSequence*)NiNew Sequence;
		StreamLoadBinary(file, pkSequence->m_uiSequenceID);

		if (version < NiStream::GetVersion(1, 2, 5, 0))
		{
			char* pcName = NULL;
			LoadCString(file, pcName);
			NiFree(pcName);
		}

		LoadCStringAsFixedString(file, pkSequence->m_kFilename);
		NiStandardizeFilePath(pkSequence->m_kFilename);
		if (version < NiStream::GetVersion(2, 2, 0, 0))
		{
			CorrectPathSlash(pkSequence->m_kFilename);
		}

		if (pcDefaultKFPath)
		{
			// Convert old default paths.
			char acFilename[NI_MAX_PATH];
			NiStrcpy(acFilename, NI_MAX_PATH, pcDefaultKFPath);
			NiStrcat(acFilename, NI_MAX_PATH, pkSequence->GetFilename());
			pkSequence->SetFilename(acFilename);
		}

		if (version >= NiStream::GetVersion(2, 5, 0, 0))
		{
			LoadCStringAsFixedString(file, pkSequence->m_kSequenceName);
		}
		else
		{
			StreamLoadBinary(file, pkSequence->m_iAnimIndex);
		}


		// Load transitions.
		unsigned int uiNumTransitions;
		StreamLoadBinary(file, uiNumTransitions);
		for (unsigned int uj = 0; uj < uiNumTransitions; uj++)
		{
			unsigned int uiDesID;
			StreamLoadBinary(file, uiDesID);
			TransitionType eType;
			BinaryStreamLoadEnum(file, &eType);

			cTransition* pkTransition;
			if (eType == TYPE_DEFAULT_SYNC)
			{
				pkTransition = defaultSyncTrans;
			}
			else if (eType == TYPE_DEFAULT_NONSYNC)
			{
				pkTransition = defaultNonSyncTrans;
			}
			else
			{
				pkTransition = (cTransition*)NiNew Transition;
				pkTransition->m_eType = eType;

				StreamLoadBinary(file, pkTransition->m_fDuration);

				unsigned int uiNumBlendPairs;
				StreamLoadBinary(file, uiNumBlendPairs);
				unsigned int uk;
				for (uk = 0; uk < uiNumBlendPairs; uk++)
				{
					cTransition::cBlendPair* pkPair = (cTransition::cBlendPair*)NiNew cTransition::BlendPair;
					if (version < NiStream::GetVersion(2, 1, 0, 0))
					{
						LoadCStringAsFixedString(file, pkPair->m_kStartKey);
						LoadCStringAsFixedString(file, pkPair->m_kTargetKey);
					}
					else
					{
						LoadFixedString(file, pkPair->m_kStartKey);
						LoadFixedString(file, pkPair->m_kTargetKey);
					}

					pkTransition->m_aBlendPairs.Add(pkPair);
				}

				unsigned int uiNumChainSequences;
				StreamLoadBinary(file, uiNumChainSequences);
				if (version < NiStream::GetVersion(1, 2, 4, 0) &&
					uiNumChainSequences > 0)
				{
					// The source sequence used to be stored in chains.
					// This is no longer the case. Thus, we ignore the first
					// item in the array and decrement the array count here.
					assert(uiNumChainSequences > 1);
					cTransition::cChainInfo kTemp;
					StreamLoadBinary(file, kTemp.m_uiSequenceID);
					StreamLoadBinary(file, kTemp.m_fDuration);
					uiNumChainSequences--;
				}
				for (uk = 0; uk < uiNumChainSequences; uk++)
				{
					cTransition::cChainInfo kInfo;
					StreamLoadBinary(file, kInfo.m_uiSequenceID);
					StreamLoadBinary(file, kInfo.m_fDuration);
					pkTransition->m_aChainInfo.Add(kInfo);
				}
			}

			pkSequence->m_mapTransitions.SetAt(uiDesID, pkTransition);
		}

		m_mapSequences.SetAt(pkSequence->m_uiSequenceID, pkSequence);
	}

	// Load sequence groups.
	unsigned int uiNumSequenceGroups;
	StreamLoadBinary(file, uiNumSequenceGroups);
	for (ui = 0; ui < uiNumSequenceGroups; ui++)
	{
		cSequenceGroup* pkGroup = (cSequenceGroup*)NiNew SequenceGroup;
		StreamLoadBinary(file, pkGroup->m_uiGroupID);

		if (version < NiStream::GetVersion(2, 1, 0, 0))
		{
			LoadCStringAsFixedString(file, pkGroup->m_kName);
		}
		else
		{
			LoadFixedString(file, pkGroup->m_kName);
		}

		unsigned int uiNumSequences;
		StreamLoadBinary(file, uiNumSequences);
		for (unsigned int uj = 0; uj < uiNumSequences; uj++)
		{
			cSequenceGroup::cSequenceInfo kInfo;
			StreamLoadBinary(file, kInfo.m_uiSequenceID);
			StreamLoadBinary(file, kInfo.m_iPriority);
			StreamLoadBinary(file, kInfo.m_fWeight);
			StreamLoadBinary(file, kInfo.m_fEaseInTime);
			StreamLoadBinary(file, kInfo.m_fEaseOutTime);
			if (version >= NiStream::GetVersion(1, 2, 1, 0))
			{
				StreamLoadBinary(file, kInfo.m_uiSynchronizeSequenceID);
			}
			else
			{
				kInfo.m_uiSynchronizeSequenceID = SYNC_SEQUENCE_ID_NONE;
			}
			pkGroup->m_aSequenceInfo.Add(kInfo);
		}

		m_mapSequenceGroups.SetAt(pkGroup->m_uiGroupID, pkGroup);
	}

	NiFree(pcDefaultNIFPath);
	NiFree(pcDefaultKFPath);

	return KFM_SUCCESS;
}

void cKFMTool::LoadCString( cEngineFile& file, char*& string )
{
	int iLength;
	StreamLoadBinary(file, iLength);
	if (iLength > 0)
	{
		string = NiAlloc(char, iLength + 1);
		assert(string);
		file.Read(string, iLength);
		string[iLength] = 0;
	}
	else
	{
		string = NULL;
	}
}

void cKFMTool::LoadFixedString( cEngineFile& file, NiFixedString& string )
{
	int iLength;
	StreamLoadBinary(file, iLength);
	assert(iLength < 1024);
	if (iLength > 0)
	{
		char acString[1024];
		file.Read(acString, iLength);
		acString[iLength] = 0;
		string = acString;
	}
	else
	{
		string = NULL;
	}
}

void cKFMTool::LoadCStringAsFixedString( cEngineFile& file, NiFixedString& string )
{
	int iLength;
	StreamLoadBinary(file, iLength);
	assert(iLength < 1024);
	if (iLength > 0)
	{
		char acString[1024];
		file.Read(acString, iLength);
		acString[iLength] = 0;
		string = acString;
	}
	else
	{
		string = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

NiImplementRTTI(NiNodeEx, NiNode);


NiNodeEx::NiNodeEx()
:NiNode()
{
	mNode = 0;
}

NiNodeEx::~NiNodeEx()
{
	mNode = 0;
}

void NiNodeEx::UpdateWorldData()
{
	NiNode::UpdateWorldData();

	if( mNode )
	{
		NiPoint3 trans = mNode->GetWorldTranslate();
		m_kWorld.m_Translate.z = trans.z;
	}
}

NiImplementCreateClone(NiNodeEx);
void NiNodeEx::CopyMembers(NiNodeEx* pkDest, NiCloningProcess& kCloning)
{
	NiNode::CopyMembers(pkDest, kCloning);

	NiObject* pkClone = NULL;
	bool bCloned = kCloning.m_pkCloneMap->GetAt(mNode, pkClone);
	if( bCloned == true )
		pkDest->mNode = (NiNode*)pkClone;
}