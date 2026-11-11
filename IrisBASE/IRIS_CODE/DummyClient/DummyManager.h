#pragma once

#include "Lexer.h"
enum
{
	/// 옵션파일을 읽을때 사용할 토큰..
	eTOKEN_STARTINDEX = 100,
	eTOKEN_MAXCOUNT,
	eTOKEN_IP,
	eTOKEN_PORT,
	eTOKEN_SERVERIDX,
	eTOKEN_CHANNELIDX,
};

class cDummyLexer : public cLexer
{
public:
	cDummyLexer( const char* pbuffer, unsigned int size );
};


class cDummyClient;

class cDummyManager
{
	static cDummyManager* mpSingleton;

public:
	cDummyManager();
	~cDummyManager();

	static cDummyManager* GetSingleton();


	bool Init( HWND wnd );
	void Exit();

	void Process( unsigned long deltaTime, unsigned long accumTime );

	void NetWorkProc( UINT msg, WPARAM wparam, LPARAM lparam );


	char* GetIP() { return (char*)mIP.Cstr(); }
	unsigned short GetPort() { return mPort; }

	unsigned int checkCount;


protected:
	unsigned int GenerateIdx();
	void RestorIdx( unsigned int idx );

	bool LoadDummyInfo( const char* pathName );

private:
	typedef tPointerHashMap<unsigned long, void*> cDummyClientMap;
	cDummyClientMap mDummyMap;

	unsigned int mDummyStartIndex;
	unsigned int mDummyMaxCount;

	cString mIP;
	unsigned short mPort;

	long mCurServer;
	long mCurChannel;

	unsigned int mCurrentCount;
	unsigned long mLoginGap;

	HWND mWnd;
};


inline
cDummyManager* cDummyManager::GetSingleton()
{
	return mpSingleton;
}

#define DUMMYMAN	cDummyManager::GetSingleton()