#pragma once

const char* const ZIP_PASSWORD = "#7qlcRkfanwlrodldi#";

struct sPackInfo
{
    TCHAR   path[_MAX_PATH];
    TCHAR   name[_MAX_FNAME];
};

struct sFileInfo
{
    TCHAR   path[_MAX_PATH];
    TCHAR   name[_MAX_FNAME];
    time_t  updateTime;
    WORD    size;
    bool    IsDirectory;
};


class cPackFile
{
public:
    cPackFile();
    ~cPackFile();

public:
    void Init();
    void ReleaseFileArr();

    bool View( const TCHAR* path );
    int  ExtractReady( CZipArchive* zip, const TCHAR* packPath, LPCTSTR pwd );
    bool Extract( CZipArchive* zip, int count, const TCHAR* extPath );

    tPointerArray<sFileInfo*>* GetFileArr()  { return &mFileArr; }

    unsigned int    GetFileCount()                  { return mFileArr.GetSize(); }
    TCHAR*          GetFileName( int n )            { return ( mFileArr[n] ) ? mFileArr[n]->name         : NULL; }
    time_t          GetFileUpdateTime( int n )      { return ( mFileArr[n] ) ? mFileArr[n]->updateTime   : NULL; }
    WORD            GetFileSize( int n )            { return ( mFileArr[n] ) ? mFileArr[n]->size         : 0;    }

private:

    typedef tPointerArray<sFileInfo*> PArray;
    PArray      mFileArr;

    sPackInfo   mPack;
};
