/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.04.29
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include <NiTGAReader.h>
#include <NiBMPReader.h>

class cEngineFile;

#define BMP_HEADER_XOR_KEY      0x1318
#define BMP_FILE_HEADER_SIZE	14		/// BMP FILE 헤더사이즈
#define BMP_INFO_HEADER_SIZE	40		/// BMP INFO 헤더사이즈
#define BMP_HEADER_TOTAL_SIZE   54		/// 총 헤더사이즈( BMP FILE 헤더사이즈 + BMP INFO 헤더사이즈 )


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

/// DDS 파일 리더
class cDDSReader : public NiDDSReader
{
public:
	cDDSReader();
	~cDDSReader();

	NiPixelData* ReadFile( cEngineFile& file, NiPixelData* optDest );

protected:
	bool ReadHeader( cEngineFile& file, unsigned int& uiWidth, unsigned int& uiHeight, NiPixelFormat& kFormat, bool& bMipmap, unsigned int& uiFaces );

	static void Read24Bit( cEngineFile& file, NiPixelData* pkDest, NiPixelFormat kSrcFmt, NiPixelFormat kDestFmt, unsigned int uiMipMapIdx, unsigned int uiFaceIdx );
	static void Read32Bit( cEngineFile& file, NiPixelData* pkDest, NiPixelFormat kSrcFmt, NiPixelFormat kDestFmt, unsigned int uiMipMapIdx, unsigned int uiFaceIdx );
	static void Read16Bit( cEngineFile& file, NiPixelData* pkDest, NiPixelFormat kSrcFmt, NiPixelFormat kDestFmt, unsigned int uiMipMapIdx, unsigned int uiFaceIdx );
};

/// BMP 파일 리더 [4/16/2009 Jo]
class cBMPReader : public NiBMPReader
{
public:
	NiPixelData* ReadFile( cEngineFile& file, NiPixelData* optDest, bool IsXorHeader = false );

protected:
	bool ReadHeader(cEngineFile& file, unsigned int& uiWidth, unsigned int& uiHeight, NiPixelFormat& kFormat, bool& bMipmap, unsigned int& uiFaces, bool IsXorHeader = false );
};