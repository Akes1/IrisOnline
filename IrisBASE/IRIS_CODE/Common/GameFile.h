/* ====================================================================
 * 파    일 :	GameFile.h
 * 목    적 :	
 * 작 성 자 :	정성대
 * 작 성 일 :	2006.08.29
 *
 * 추가내용 :
 *		추가날짜	작업자	추가내용
 *		2006.08.29	정성대	생성
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once


enum eFILE_MODE
{
	eFILE_MODE_TXT,
	eFILE_MODE_BIN,
};

class cGameFile 
{
public:
	cGameFile();
	virtual ~cGameFile();

	bool OpenGameFile( const char* pfilename, int mode = eFILE_MODE_TXT );
	void CloseGameFile();

	int GetInt();
	short GetShort();
	char GetBYTE();
	bool GetBOOL();
	unsigned short GetWORD();
	unsigned long GetDWORD();
	float GetFloat();
	double GetDouble();
	char* GetString();
	/// char* GetStringInQuotation();
	bool IsEOF();

	/// 061011 PKH 한줄단위 처리(주석?)
	void	GetLineX(char * pBuf, int len);

protected:
	FILE*	mfp;
	int		mMode;
};