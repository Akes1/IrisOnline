#include "stdafx.h"
#include "EngineFile.h"

NiImplementDerivedBinaryStream(cEngineFile, FileRead, FileWrite);

cEngineFile::cEngineFile( const char* pathName ) //cFileToRead* fileToRead )
{
	mFileLoader = new cFileLoader;
	if( mFileLoader->Open( pathName, true ) == false )
	{
		m_bGood = false;
		SAFE_DELETE(mFileLoader);
		return;
	}

	m_bGood = true;

	mFileToRead = mFileLoader->GetFile();
	assert( mFileToRead );

	SetEndianSwap( false );

	m_eMode = READ_ONLY;

	m_pBuffer = 0;
	m_uiBufferAllocSize = 0;
	m_uiPos = m_uiBufferReadSize = 0;

	mPathName = pathName;
}

cEngineFile::~cEngineFile()
{
	mFileToRead = 0;
	SAFE_DELETE(mFileLoader);

	m_pBuffer = 0;
}

void cEngineFile::Seek( unsigned int offset, int whence )
{
	mFileToRead->Seek( offset, whence );
}

unsigned int cEngineFile::FileRead( void* buffer, unsigned int bytes )
{
	return mFileToRead->Read( buffer, bytes );
}

unsigned int cEngineFile::GetLine( char* buffer, unsigned int maxBytes )
{
	unsigned int uiBytesRead = 0;
	unsigned int i = 0;

	assert(maxBytes > 0);

	while (i + 1 < maxBytes)
	{
		char c;
		unsigned int uiRead = Read(&c, 1);
		uiBytesRead += uiRead;

		if (uiRead != 1 || c == '\n')
			break;

		if (c != '\r')
			buffer[i++] = c;
	}

	buffer[i] = 0;

	return uiBytesRead;
}


unsigned int cEngineFile::GetFileSize() const
{
	return mFileToRead->GetSize();
}