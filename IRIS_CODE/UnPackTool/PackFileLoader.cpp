#include "StdAfx.h"
#include "PackFileLoader.h"

cPackFile::cPackFile()
{
    Init();
}

cPackFile::~cPackFile()
{
    ReleaseFileArr();
}

void cPackFile::Init()
{
    mFileArr.Clear();
    memset( &mPack, 0, sizeof( mPack ) );
}

void cPackFile::ReleaseFileArr()
{
    /// file list
    for( unsigned int i = 0; i < mFileArr.GetSize(); i++ )
    {
        sFileInfo* p = (sFileInfo*)mFileArr[i];
        SAFE_DELETE( p );
    }
    mFileArr.Clear();
}

bool cPackFile::View( const TCHAR* path )
{
    ReleaseFileArr();

    CZipArchive zip;
    zip.Open( path, CZipArchive::zipOpenReadOnly );
    zip.SetPassword(ZIP_PASSWORD);

    CZipFileHeader header;
    for( WORD i = 0; i < zip.GetCount(); i++ )
    {
        zip.GetFileInfo( header, i );

        sFileInfo* file = new sFileInfo;

        file->updateTime  = header.GetTime();
        file->size        = header.m_uUncomprSize;

        CString fullPath;
        fullPath.Format( "%s", header.GetFileName() );
        int pos = fullPath.ReverseFind( '\\' );
        strcpy_s( file->name, fullPath.Right(fullPath.GetLength() - pos - 1) );
        strcpy_s( file->path, fullPath.Left( pos ) );

        mFileArr.PushBack( file );
    }
    zip.Close();

    return true;
}

int cPackFile::ExtractReady( CZipArchive* zip, const TCHAR* packPath, LPCTSTR pwd )
{
    zip->Open( packPath, CZipArchive::zipOpenReadOnly );
    zip->SetPassword( pwd );
    
    return zip->GetCount();
}

bool cPackFile::Extract( CZipArchive* zip, int count, const TCHAR* extPath )
{
/*
    CZipFileHeader header;
    zip->GetFileInfo( header, count );

    CString str;
    str.Format("%s", header.GetFileName() );
    int pos = str.Find('\\');

    CString filePath = str.Mid( pos + 1, str.GetLength() );

//    header.SetFileName( filePath );
*/
    return zip->ExtractFile( count, extPath );
 }