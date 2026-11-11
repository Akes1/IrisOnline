#include "stdafx.h"
#include "FilePackSystem.h"

#include "FileFinder.h"
#include "File.h"
#include "MemFile.h"
#include "FilePack.h"

#define PACK_PASSWORD	_T("dkdlfltmehdwjq10aks")
#define MAX_PACKS		128

class PackKey
{
private:
	char mKey[20];
public:
	PackKey( );
	char* GetKey(){ return mKey; }
	int    GetKeyLenth(){ return ::strlen( mKey ); }
};

PackKey::PackKey() 
{
	mKey[0] =    41;
	mKey[1] =   -82;
	mKey[2] =   109;
	mKey[3] =   -73;
	mKey[4] =    70;
	mKey[5] =    77;
	mKey[6] =   -48;
	mKey[7] =   -10;
	mKey[8] =   -24;
	mKey[9] =    92;
	mKey[10] =   75;
	mKey[11] =  106;
	mKey[12] =   -7;
	mKey[13] = -114;
	mKey[14] =  -17;
	mKey[15] =  -73;
	mKey[16] = -111;
	mKey[17] =  -65;
	mKey[18] =  -92;
	mKey[19] =    0;
}

enum eFolder
{
	FOLDER_BASE = 0,
	FOLDER_LAUNCHER,
	FOLDER_SOUND,
	FOLDER_DATA,
	FOLDER_MAP,
	FOLDER_SCRIPT,
	FOLDER_LANGUAGE,
	FOLDER_COUNT
};

PackKey g_packKey;

const charT* gFolderNames[FOLDER_COUNT] =
{
	_T(".\\"), _T("Launcher\\"), _T("Sound\\"), _T("Data\\"), _T("Map\\"), _T("Script\\"), _T("Language\\")
};

cFilePackSystem::cFilePackSystem()
: mFileIndexMap( 16384 )
{
	mFilePackArray.Reserve( MAX_PACKS );
}

cFilePackSystem::~cFilePackSystem()
{
	for( unsigned int i = 0, iend = mFilePackArray.GetSize(); i < iend; ++i )
	{
		delete (cFilePack*)mFilePackArray[i];
	}

	mFilePackArray.Clear();
}

bool cFilePackSystem::Init()
{
	/// 파일 팩을 검색
	cFileFinder fileFinder;
	cStringT findPath;
	cStringT packName;
	cStringT pathName;

	for( unsigned int i = FOLDER_DATA; i < FOLDER_COUNT; ++i )
	{
		findPath.Format( _T("%s*.pack"), gFolderNames[i] );

		if( fileFinder.FindFirst( findPath.Cstr() ) == false )
			continue;

		do
		{
			cFilePack* pack = new cFilePack;

			/// 파일 팩을 엶
			packName.Format( _T("%s%s"), gFolderNames[i], fileFinder.GetFileName() );

			if( pack->Open( packName.Cstr(), cFilePack::OPEN_READONLY ) == false )
			{
				delete pack;
				MessageBox(NULL, _T("Failed Open pack file.. reinstall client."), _T("Error"), MB_OK | MB_ICONEXCLAMATION );
				continue;
			}

			/// 패스워드를 설정
			//pack->SetPassword( PACK_PASSWORD );
			pack->SetPasswordEx( g_packKey.GetKey(), g_packKey.GetKeyLenth() );

			/// 파일 팩을 추가
			unsigned int packIndex = mFilePackArray.GetSize();
			unsigned int fileIndex;

			mFilePackArray.PushBack( pack );

			/// 파일 인덱스를 맵에 추가
			cFileInPackInfo fipi;

			for( unsigned int j = 0, jend = pack->GetNumFiles(); j < jend; ++j )
			{
				pack->GetFileInfo( &fipi, j );

				if( fipi.IsDirectory() )
					continue;

				fileIndex = j;
				fileIndex <<= 8;
				fileIndex += packIndex;

#ifdef _UNICODE
				///
				char cstr[MAX_PATH];
				ConvertToAscii( fipi.GetPathName().Cstr(), cstr, MAX_PATH );
				cString tempName = cstr;
#else
				///
				cString tempName = fipi.GetPathName().Cstr();
#endif
				tempName.Replace( '/', '\\' );
				tempName.TrimLeft( ".\\" );
				tempName.ToLower();

				/// 명확성 검사. fipi.pathname 과 filename을 비교하여 현재 맞는 경로에 있는 pack 파일을 열었는지 검사.
				/// 게임 설치 폴더 안에 pack 파일이 다른 곳으로 옮겨 졌거나, 다른 이름으로 저장되었다면 현재 pack 파일 무시.
				if( _tcsnicmp( packName.Cstr(), fipi.GetPathName().Cstr(), _tcslen(packName.Cstr()) - 5 ) != 0 )
					break;

				mFileIndexMap.Insert( tempName, fileIndex );
			}
		}
		while( fileFinder.FindNext() );
	}
	fileFinder.Close();
	return true;
}

cFileToRead* cFilePackSystem::OpenFileToRead( const cString& pathName )
{
#if defined(_DEBUG) || defined(_DEVSYS) || defined(_DEVMODE) || defined(_GMTOOL)
	/// 로컬 시스템에서 로딩
	cFileToRead* file = cFileSystem::OpenFileToRead( pathName );
	if( file )
		return file;

	/// 팩킹 시스템에서 로딩
	cString tempName = pathName;
	tempName.Replace( '/', '\\' );
	tempName.TrimLeft( ".\\" );
	tempName.ToLower();

	cFileIndexMap::cIterator i = mFileIndexMap.Find( tempName );

	if( i == mFileIndexMap.End() )
		return 0;

	cMemFileInPackToRead* fileInPack = new cMemFileInPackToRead;
	unsigned int packIndex = i->mSecond & 0xFF;
	int fileIndex = int(i->mSecond >> 8);

	if( fileInPack->Open( (cFilePack*)mFilePackArray[packIndex], fileIndex ) == false )
	{
		delete fileInPack;
		return 0;
	}
	return fileInPack;
#else
	/// 팩킹 시스템에서 로딩
	cString tempName = pathName;
	tempName.Replace( '/', '\\' );
	tempName.TrimLeft( ".\\" );
	tempName.ToLower();

	cFileIndexMap::cIterator i = mFileIndexMap.Find( tempName );

	if( i != mFileIndexMap.End() )
	{
		cMemFileInPackToRead* fileInPack = new cMemFileInPackToRead;
		unsigned int packIndex = i->mSecond & 0xFF;
		int fileIndex = int(i->mSecond >> 8);

		if( fileInPack->Open( (cFilePack*)mFilePackArray[packIndex], fileIndex ) == true )
		{
			return fileInPack;
		}
		delete fileInPack;
	}

	return 0;
#endif
}

cMemFileToRead* cFilePackSystem::OpenMemFileToRead( const cString& pathName )
{
#if defined(_DEBUG) || defined(_DEVSYS) || defined(_DEVMODE) || defined(_GMTOOL)
	/// 로컬 시스템에서 로딩
	cMemFileToRead* file = cFileSystem::OpenMemFileToRead( pathName );
	if( file )
		return file;

	/// 팩킹 시스템에서 로딩
	cString tempName = pathName;
	tempName.Replace( '/', '\\' );
	tempName.TrimLeft( ".\\" );
	tempName.ToLower();

	cFileIndexMap::cIterator i = mFileIndexMap.Find( tempName );

	if( i == mFileIndexMap.End() )
		return 0;

	cMemFileInPackToRead* fileInPack = new cMemFileInPackToRead;
	unsigned int packIndex = i->mSecond & 0xFF;
	int fileIndex = int(i->mSecond >> 8);

	if( fileInPack->Open( (cFilePack*)mFilePackArray[packIndex], fileIndex ) == false )
	{
		delete fileInPack;
		return 0;
	}
	return fileInPack;

#else
	/// 팩킹 시스템에서 로딩
	cString tempName = pathName;
	tempName.Replace( '/', '\\' );
	tempName.TrimLeft( ".\\" );
	tempName.ToLower();

	cFileIndexMap::cIterator i = mFileIndexMap.Find( tempName );

	if( i != mFileIndexMap.End() )
	{
		cMemFileInPackToRead* fileInPack = new cMemFileInPackToRead;
		unsigned int packIndex = i->mSecond & 0xFF;
		int fileIndex = int(i->mSecond >> 8);

		if( fileInPack->Open( (cFilePack*)mFilePackArray[packIndex], fileIndex ) == true )
		{
			return fileInPack;
		}
		delete fileInPack;
	}

	return 0;
#endif
}

bool cFilePackSystem::FileExist( const cString& pathName )
{
	cString tempName = pathName;
	tempName.Replace( '/', '\\' );
	tempName.TrimLeft( ".\\" );
	tempName.ToLower();

	cFileIndexMap::cIterator i = mFileIndexMap.Find( tempName );

	if( i == mFileIndexMap.End() )
	{
		return cFileSystem::FileExist( pathName );
	}

	return true;
}
