/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.04.29
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include <NiTGAReader.h>

class cEngineFile;

/// TGA 파일 리더
class cTGAReader : public NiTGAReader
{
public:
	NiPixelData* ReadFile( cEngineFile& file, NiPixelData* optDest );

protected:
	void LoadRLERow( cEngineFile& file, unsigned char *dest, unsigned int destSize );
	void GetColormap( cEngineFile& file );
	bool ReadHeader( cEngineFile& file, unsigned int& width, unsigned int& height, NiPixelFormat& pixelFormat, bool& mipmap, unsigned int& faces );
};
