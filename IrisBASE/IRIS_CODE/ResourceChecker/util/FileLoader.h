/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.17
* 내    용 : 파일 적재자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cFileToRead;

/// 파일 적재자
/// 파일을 쉽게 읽을 수 있도록 도와준다.
/// 파일 시스템이 먼저 생성된 후 사용 가능하다.
class cFileLoader
{
public:
	cFileLoader();
	~cFileLoader();

	/// 열기
	bool Open( const cString& pathName, bool useMemory  );

	bool OpenNoPack( const cString& pathName, bool useMemory );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	int Close();

	/// 데이타 읽기
	/// 실제로 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	unsigned int Read( void* buffer, unsigned int bytes );

	/// 기본 자료형을 읽기
	bool ReadInt( int* value );
	bool ReadUnsignedInt( unsigned int* value );
	bool ReadFloat( float* value );

	/// 한 줄 읽기
	/// 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	/// 주의: 줄 끝에 널문자를 추가하므로 최대 bufferSize-1 만큼을 읽어 들인다.
	unsigned int ReadLine( char* buffer, unsigned int bufferSize );

	/// 내부 위치를 이동
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	int Seek( unsigned int offset, int origin = SEEK_SET );

	/// 내부 위치를 리턴
	/// 실패하면 0을 리턴한다.
	unsigned int Tell() const;

	/// 파일 크기를 리턴
	unsigned int GetSize() const;

	/// 파일을 리턴
	cFileToRead* GetFile() const;

	/// 메모리 버퍼 포인터를 리턴
	/// 메모리를 사용하지 않는 경우 0을 리턴
	const char* GetBufferPtr();

	/// 파일 끝에 도달했는지 여부를 리턴
	bool IsEnd() const;

	/// 빈 파일인지 리턴
	bool IsEmpty() const;

	/// 메모리 사용 여부를 리턴
	bool IsMemUsed() const;

private:
	/// 메모리 사용 여부
	bool mMemUsed;

	/// 읽기 전용 파일
	cFileToRead* mFile;
};

inline cFileToRead* cFileLoader::GetFile() const
{
	return mFile;
}

inline bool cFileLoader::IsMemUsed() const
{
	return mMemUsed;
}
