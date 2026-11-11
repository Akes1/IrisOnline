// NUMERICAL DESIGN LIMITED PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Numerical Design Limited and may not 
// be copied or disclosed except in accordance with the terms of that 
// agreement.
//
//      Copyright (c) 1996-2005 Numerical Design Limited.
//      All Rights Reserved.
//
// Numerical Design Limited, Chapel Hill, North Carolina 27514
// http://www.ndl.com
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "ShaderHelper.h"

#pragma warning (push, 3)

// This sample only has static library configurations
#define _LIB
#	include <NSBShaderLib.h>
#	include <NSFParserLib.h>
#undef _LIB

#if defined(_DX9)
#	pragma comment(lib, "NiBinaryShaderLibDX9.lib")
#	pragma comment(lib, "NSBShaderLibDX9.lib")
#	pragma comment(lib, "NSFParserLibDX9.lib")
#endif

bool cShaderHelper::SetupShaderSystem( const char* pcProgramDirectory )
{
	assert (NiD3DShaderProgramFactory::GetInstance());
	NiD3DShaderProgramFactory::GetInstance()->AddProgramDirectory(
		pcProgramDirectory);

	// First, we will run the NSF parser. This is done to make sure that
	// any text-based shader files that have been modified are re-compiled
	// to binary before loading all the binary representations.
	if (!RunShaderParsers())
	{
		NiMessageBox("Failed to run shader parsers!", "Error!");
		return false;
	}

	if (!RegisterShaderLibraries())
	{
		NiMessageBox("Failed to register shader libraries!", "Error!");
		return false;
	}

	return true;
}

bool cShaderHelper::RunShaderParsers()
{
	NiShaderFactory::RegisterRunParserCallback(RunParser);

	unsigned int uiCount = NiShaderFactory::LoadAndRunParserLibrary(
		0, ".\\", true);

	char acTemp[256];
	NiSprintf(acTemp, 256, "Parsed %d shaders from NSFShaderParser!\n", 
		uiCount);
	NiOutputDebugString(acTemp);

	return true;
}

bool cShaderHelper::RegisterShaderLibraries()
{
	NiShaderFactory::RegisterClassCreationCallback(LibraryClassCreate);

	unsigned int uiCount = 1;
	char* apcDirectories[1];
	apcDirectories[0] = ".\\";

	if (!NiShaderFactory::LoadAndRegisterShaderLibrary(0,
		uiCount, apcDirectories, true))
	{
		NiMessageBox("Failed to load shader library!", "Error!");
		return false;
	}

	return true;
}

bool cShaderHelper::CleanupShaderLibraries()
{
	// Note that we don't have to do anything here. The scene will clean up
	// the shader instances, and the renderer shutdown code will take care
	// of the factory!
	return true;
}

bool cShaderHelper::LibraryClassCreate(const char* pcLibFile, 
									 NiRenderer* pkRenderer, int iDirectoryCount, char* apcDirectories[], 
									 bool bRecurseSubFolders, NiShaderLibrary** ppkLibrary)
{
	*ppkLibrary = 0;

	// Create the NSB Shader Library.
	return NSBShaderLib_LoadShaderLibrary(pkRenderer, iDirectoryCount, 
		apcDirectories, bRecurseSubFolders, ppkLibrary);
}

unsigned int cShaderHelper::RunParser(const char* pcLibFile, 
									NiRenderer* pkRenderer, const char* pcDirectory, 
									bool bRecurseSubFolders)
{
	// Run the NSF Parser.
	return NSFParserLib_RunShaderParser(pcDirectory, bRecurseSubFolders);
}

#pragma warning (pop)
