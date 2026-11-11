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
#pragma once

/// 셰이더 도우미
class cShaderHelper
{
public:
    bool SetupShaderSystem(const char* pcProgramDirectory);
    bool RunShaderParsers();
    bool RegisterShaderLibraries();
    bool CleanupShaderLibraries();

protected:
    static bool LibraryClassCreate(const char* pcLibFile,
        NiRenderer* pkRenderer, int iDirectoryCount, char* apcDirectories[], 
        bool bRecurseSubFolders, NiShaderLibrary** ppkLibrary);

    static unsigned int RunParser(const char* pcLibFile, 
        NiRenderer* pkRenderer, const char* pcDirectory, 
        bool bRecurseSubFolders);
};
