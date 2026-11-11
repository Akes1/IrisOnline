#include "stdafx.h"

#include "Application.h"


int APIENTRY _tWinMain( HINSTANCE hI, HINSTANCE hPI, LPTSTR lpCmdLine, int nCmdShow )
{
	UNREFERENCED_PARAMETER(hPI);
	UNREFERENCED_PARAMETER(lpCmdLine);

	::timeBeginPeriod(1);

	cApplication* theApp = cApplication::Create();
	if( theApp )
	{
		if( theApp->Init( hI ) )
		{
			theApp->Run();
		}
		SAFE_EXIT_DELETE(theApp);
	}

	::timeEndPeriod(1);

	return 0;
}
