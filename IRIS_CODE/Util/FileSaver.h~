/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.04
* 내    용 : 파일 저장자
* 주의사항 : 파일 시스템이 먼저 생성된 후 사용 가능하다.
*===========================================================================*/
#pragma once

class cFileToWrite;
class cFileToWriteW;

/// 파일 저장자
/// 파일을 쉽게 저장할 수 있도록 도와준다.
/// 파일 시스템이 먼저 생성된 후 사용 가능하다.
class cFileSaver
{
public:
	cFileSaver();
	~cFileSaver();

	/// 열기
	bool Open( const cString& pathName );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	int Close();

	/// 데이타 쓰기
	/// 실제로 쓴 데이타 크기를 리턴한다. 실패하면 0을 리턴한다.
	unsigned int Write( const void* buffer, unsigned int bytes );

	/// 기본 자료형을 쓰기
	bool WriteInt( int value );
	bool WriteUnsignedInt( unsigned int value );
	bool WriteFloat( float value );

	/// 데이타를 모두 쓰기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	int Flush();

	/// 파일을 리턴
	cFileToWrite* GetFile() const;

private:
	/// 쓰기 전용 파일
	cFileToWrite* mFile;
};

inline cFileToWrite* cFileSaver::GetFile() const
{
	return mFile;
}


//------------------------------------------
class cFileSaverW
{
public:
	cFileSaverW();
	~cFileSaverW();

	/// 열기
	bool Open( const cStringW& pathName );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	int Close();

	/// 데이타 쓰기
	/// 실제로 쓴 데이타 크기를 리턴한다. 실패하면 0을 리턴한다.
	unsigned int Write( const void* buffer, unsigned int bytes );

	/// 기본 자료형을 쓰기
	bool WriteInt( int value );
	bool WriteUnsignedInt( unsigned int value );
	bool WriteFloat( float value );

	/// 데이타를 모두 쓰기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	int Flush();

	/// 파일을 리턴
	cFileToWriteW* GetFile() const;

private:
	/// 쓰기 전용 파일
	cFileToWriteW* mFile;
};

inline cFileToWriteW* cFileSaverW::GetFile() const
{
	return mFile;
}
