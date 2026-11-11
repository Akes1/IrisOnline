/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 파일 입출력 처리
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 파일 경로를 얻음
/// 인자: "c:/mydir/myfile.ext"
/// 결과: "c:/mydir/"
bool GetFilePath( cString* out, const cString& str);
bool GetFilePath( cStringW* out, const cStringW& str);

/// 파일 이름을 얻음
/// 인자: "c:/mydir/myfile.ext"
/// 결과: "myfile.ext"
bool GetFileName( cString* out, const cString& str);

/// 파일 확장자를 얻음
/// 인자: "c:/mydir/myfile.ext"
/// 결과: "ext"
bool GetFileExtension( cString* out, const cString& str);
cString GetFileExtension( const cString& str);
cString GetFileExtension( const char* str,  unsigned int strLen );

/// 파일 존재 여부
bool FileExist( const cString& pathName );
bool FileExist( const cStringW& pathName );

/// 파일 삭제
bool FileRemove( const cString& pathName );

/// 파일 이름 변경
bool FileRename( const cString& oldName, const cString& newName );

/// 파일 백업
bool FileBackup( const cString& pathName );

/// 읽기 전용 파일
class cFileToRead
{
public:
	cFileToRead();
	virtual ~cFileToRead();

	/// 열기
	virtual bool Open( const cString& pathName );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Close();

	/// 데이타 읽기
	/// 실제로 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	virtual unsigned int Read( void* buffer, unsigned int bytes );

	/// 한 줄 읽기
	/// 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	/// 주의: 줄 끝에 널문자를 추가하므로 최대 bufferSize-1 만큼을 읽어 들인다.
	virtual unsigned int ReadLine( char* buffer, unsigned int bufferSize );

	/// 내부 위치를 이동
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Seek( unsigned int offset, int origin = SEEK_SET );

	/// 내부 위치를 리턴
	/// 실패하면 0을 리턴한다.
	virtual unsigned int Tell() const;

	/// 파일 크기를 리턴
	virtual unsigned int GetSize() const;

	/// 파일 끝에 도달했는지 여부를 리턴
	virtual bool IsEnd() const;

	/// 빈 파일인지 리턴
	bool IsEmpty() const;

//protected:
	/// 파일 사이즈
	unsigned int mSize;

	/// 파일 구조체
	FILE* mFile;
};

inline bool cFileToRead::IsEmpty() const
{
	return mSize == 0;
}

/// 쓰기 전용 파일
class cFileToWrite
{
public:
	cFileToWrite();
	virtual ~cFileToWrite();

	/// 열기
	virtual bool Open( const cString& pathName );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Close();

	/// 데이타 쓰기
	/// 실제로 쓴 데이타 크기를 리턴한다. 실패하면 0을 리턴한다.
	virtual unsigned int Write( const void* buffer, unsigned int bytes );

	/// 데이타를 모두 쓰기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Flush();

protected:
	/// 파일 구조체
	FILE* mFile;
};

//-------------------------------------------------------

class cFileToReadW
{
public:
	cFileToReadW();
	virtual ~cFileToReadW();

	/// 열기
	virtual bool Open( const cStringW& pathName );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Close();

	/// 데이타 읽기
	/// 실제로 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	virtual unsigned int Read( void* buffer, unsigned int bytes );

	/// 한 줄 읽기
	/// 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	/// 주의: 줄 끝에 널문자를 추가하므로 최대 bufferSize-1 만큼을 읽어 들인다.
	virtual unsigned int ReadLine( wchar_t* buffer, unsigned int bufferSize );

	/// 내부 위치를 이동
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Seek( unsigned int offset, int origin = SEEK_SET );

	/// 내부 위치를 리턴
	/// 실패하면 0을 리턴한다.
	virtual unsigned int Tell() const;

	/// 파일 크기를 리턴
	virtual unsigned int GetSize() const;

	/// 파일 끝에 도달했는지 여부를 리턴
	virtual bool IsEnd() const;

	/// 빈 파일인지 리턴
	bool IsEmpty() const;

	//protected:
	/// 파일 사이즈
	unsigned int mSize;

	/// 파일 구조체
	FILE* mFile;
};

inline bool cFileToReadW::IsEmpty() const
{
	return mSize == 0;
}

/// 쓰기 전용 파일
class cFileToWriteW
{
public:
	cFileToWriteW();
	virtual ~cFileToWriteW();

	/// 열기
	virtual bool Open( const cStringW& pathName );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Close();

	/// 데이타 쓰기
	/// 실제로 쓴 데이타 크기를 리턴한다. 실패하면 0을 리턴한다.
	virtual unsigned int Write( const void* buffer, unsigned int bytes );

	/// 데이타를 모두 쓰기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Flush();

protected:
	/// 파일 구조체
	FILE* mFile;
};
