/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.16
* 내    용 : 메모리 파일, 파일팩 내부 메모리 파일
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "File.h"

/// 읽기 전용 메모리 파일
class cMemFileToRead : public cFileToRead
{
	friend class cFilePack;

public:
	cMemFileToRead();
	virtual ~cMemFileToRead();

	/// 열기
	virtual bool Open( const cString& pathName );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Close();

	/// 데이타 읽기
	/// 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	virtual unsigned int Read( void* buffer, unsigned int bytes );

	/// 한 줄 읽기
	/// 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	/// 주의: 줄 끝에 널문자를 추가하므로 최대 bufferSize-1 만큼을 읽어 들인다.
	virtual unsigned int ReadLine( char* buffer, unsigned int bufferSize );

	/// 내부 위치 이동
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Seek( unsigned int offset, int origin = SEEK_SET );

	/// 내부 위치 얻기
	/// 실패하면 0을 리턴한다.
	virtual unsigned int Tell() const;

	/// 파일 끝에 도달했는지 여부를 리턴
	virtual bool IsEnd() const;

	/// 메모리 버퍼를 리턴
	const tArray<char>& GetBuffer() const;

	/// 메모리 버퍼 포인터를 리턴
	const char* GetBufferPtr();

protected:
	/// 버퍼안에서의 위치
	unsigned int mPos;

	/// 버퍼
	typedef tArray<char> cBuffer;
	cBuffer mBuffer;
};


#if defined(_CLIENT) || defined(_MODELVIEW) || defined(REGEN_TOOL) || defined(MAP_EDITOR)
/// 읽기 전용 파일팩 내부 메모리 파일
class cMemFileInPackToRead : public cMemFileToRead
{
public:
	cMemFileInPackToRead();
	virtual ~cMemFileInPackToRead();

	/// 열기
	bool Open( cFilePack* pack, int fileIndex = -1 );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	int Close();
};
#endif


//----------------------------------------------------------
/// 읽기 전용 메모리 파일
class cMemFileToReadW : public cFileToReadW
{
	friend class cFilePack;

public:
	cMemFileToReadW();
	virtual ~cMemFileToReadW();

	/// 열기
	virtual bool Open( const cStringW& pathName );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Close();

	/// 데이타 읽기
	/// 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	virtual unsigned int Read( void* buffer, unsigned int bytes );

	/// 한 줄 읽기
	/// 읽어 들인 바이트 수를 리턴한다.
	/// 실패하거나 파일 끝에 도달하면 0을 리턴한다.
	/// 주의: 줄 끝에 널문자를 추가하므로 최대 bufferSize-1 만큼을 읽어 들인다.
	virtual unsigned int ReadLine( wchar_t* buffer, unsigned int bufferSize );

	/// 내부 위치 이동
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	virtual int Seek( unsigned int offset, int origin = SEEK_SET );

	/// 내부 위치 얻기
	/// 실패하면 0을 리턴한다.
	virtual unsigned int Tell() const;

	/// 파일 끝에 도달했는지 여부를 리턴
	virtual bool IsEnd() const;

	/// 메모리 버퍼를 리턴
	const tArray<wchar_t>& GetBuffer() const;

	/// 메모리 버퍼 포인터를 리턴
	const wchar_t* GetBufferPtr();

protected:
	/// 버퍼안에서의 위치
	unsigned int mPos;

	/// 버퍼
	typedef tArray<wchar_t> cBuffer;
	cBuffer mBuffer;
};

#if defined(_CLIENT) || defined(_MODELVIEW) || defined(REGEN_TOOL) || defined(MAP_EDITOR)
/// 읽기 전용 파일팩 내부 메모리 파일
class cMemFileInPackToReadW : public cMemFileToReadW
{
public:
	cMemFileInPackToReadW();
	virtual ~cMemFileInPackToReadW();

	/// 열기
	bool Open( cFilePack* pack, int fileIndex = -1 );

	/// 닫기
	/// 성공하면 0, 실패하면 -1을 리턴한다.
	int Close();
};
#endif
