#include "stdafx.h"
#include "GameFile.h"


cGameFile::cGameFile()
{
	mfp = NULL;
	mMode = eFILE_MODE_TXT;
}

cGameFile::~cGameFile()
{
	CloseGameFile();
}

/* ------------------------------------------------------------------
* 함수이름 :	
* 목    적 :	
* 주의사항 :	
* ------------------------------------------------------------------ */
bool cGameFile::OpenGameFile( const char* pfilename, int mode /* = eFILE_MODE_TXT  */ )
{
	mMode = mode;
	switch( mMode )
	{
	case eFILE_MODE_TXT:
		{
			mfp = 0;
			if( fopen_s( &mfp, pfilename, "r") != 0 )
				return false;
		}
		break;
	case eFILE_MODE_BIN:
		break;
	}

	return true;
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
void cGameFile::CloseGameFile()
{
	if( mfp )
	{
		switch( mMode )
		{
		case eFILE_MODE_TXT:
			{
				fclose( mfp );
			}
			break;
		case eFILE_MODE_BIN:
			break;
		}
	}

	mfp = NULL;
	mMode = eFILE_MODE_TXT;
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
int cGameFile::GetInt()
{
	return atoi( GetString() );
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
short cGameFile::GetShort()
{
	return (short)atoi( GetString() );
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
char cGameFile::GetBYTE()
{
	return (char)atoi( GetString() );
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cGameFile::GetBOOL()
{
	return atoi( GetString() )?true:false;
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
unsigned short cGameFile::GetWORD()
{
	return (unsigned short)atoi( GetString() );
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
unsigned long cGameFile::GetDWORD()
{
	return (unsigned long)atoi( GetString() );
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
float cGameFile::GetFloat()
{
	return (float)atof( GetString() );
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
double cGameFile::GetDouble()
{
	return (double)atof( GetString() );
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
char* cGameFile::GetString()
{
	static char buf[512] = {0,};

	switch( mMode )
	{
	case eFILE_MODE_TXT:
		{
			fscanf_s( mfp, "%s", buf );
		}
		break;
	case eFILE_MODE_BIN:
		{

		}
		break;
	}

	return buf;
}

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	사용금지
 * ------------------------------------------------------------------ */
/*
char* cGameFile::GetStringInQuotation()
{
	static char buf[512] = {0,};
	int c = 0;

	switch( mMode )
	{
	case eFILE_MODE_TXT:
		{
			int count = 0;
			while( (c = fgetc(mfp)) != '"' )
			{
				count++;
				if(count > 1000)
				{
					assert(0 && "Do script file " 
				}
			}
			int i = 0;
			while( !feof(mfp) )
			{
				buf[i] = (char)fgetc( mfp );
				if( buf[i] == '"' )
				{
					buf[i] = 0;
					fgetc( mfp );
					return buf;
				}
				++i;
			}
		}
		break;
	}

	return buf;
}
*/

/* ------------------------------------------------------------------
 * 함수이름 :	
 * 목    적 :	
 * 주의사항 :	
 * ------------------------------------------------------------------ */
bool cGameFile::IsEOF()
{
	switch( mMode )
	{
	case eFILE_MODE_TXT:	
		{
			return feof(mfp)?true:false;
		}
		break;
	case eFILE_MODE_BIN:
		break;
	}
	return TRUE;
}

/* --------------------------------------------------------------------------
* 함수이름 : GetLineX
* 목    적 : 한줄 단위로 읽음(주석 처리 할라고)
* 주의사항 :
*---------------------------------------------------------------------------*/
void cGameFile::GetLineX(char * pBuf, int len)
{
	switch( mMode )
	{
	case eFILE_MODE_TXT:	
		{
			fgets(pBuf, len, mfp);
		}
		break;
	case eFILE_MODE_BIN:
		break;
	}

}
