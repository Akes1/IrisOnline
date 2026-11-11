#include "stdafx.h"
#include "ImageReader.h"

#include "EngineFile.h"

void cTGAReader::LoadRLERow( cEngineFile& file, unsigned char *dest, unsigned int destSize )
{
	// DO NOT clear m_uiRLECount or m_bRLEncodedRun here - an RLE run can 
	// cross over multiple scanlines, so a run may cross more than one call
	// to this function

	unsigned int uiPixelCol = 0;
	while (uiPixelCol < m_usWidth)
	{
		if (m_uiRLECount == 0)
		{ 
			// Have to restart run.
			unsigned char ucTag = 0;
			StreamLoadBinary(file, (char *)&ucTag, 1);

			m_bRLEncodedRun = (ucTag & 0x80) ? true : false;

			if (ucTag & 0x80) 
			{
				m_bRLEncodedRun = true;
				m_uiRLECount = ucTag - 127; // Single pixel replicated

				// get pixel to be replicated
				StreamLoadBinary(file, (char *)m_aucRLEBits, m_uiBytesPerPixel);
			}
			else
			{
				m_bRLEncodedRun = false;
				m_uiRLECount = ucTag + 1; // Stream of unencoded pixels
			}
		}

		// Have already read count and, if the run is encoded, the
		// pixel that defines the run as well.

		// now copy the run into as many pixels as possible - either the
		// rest of the row, or the entire run, whichever is shorter...
		unsigned int uiRunToCopy = m_usWidth - uiPixelCol;
		if (m_uiRLECount < uiRunToCopy)
			uiRunToCopy = m_uiRLECount;

		assert(uiRunToCopy > 0);
		assert(uiRunToCopy <= m_uiRLECount);
		assert(uiRunToCopy <= (m_usWidth - uiPixelCol));

		if (m_bRLEncodedRun) // Encoded pixel - copy
		{
			for (unsigned int j = 0; j < uiRunToCopy; j++)
			{
				NiMemcpy(dest, destSize, m_aucRLEBits, m_uiBytesPerPixel);

				dest += m_uiBytesPerPixel;
			}
		}
		else  // Unencoded pixel - read next set of pixels
		{
			StreamLoadBinary(file, (char *)dest, 
				m_uiBytesPerPixel*uiRunToCopy);

			dest += m_uiBytesPerPixel*uiRunToCopy;
		}

		// Decrease remaining run length and remaining collumn pixels
		m_uiRLECount -= uiRunToCopy;
		uiPixelCol += uiRunToCopy;
	}

	assert(uiPixelCol == m_usWidth);
}

void cTGAReader::GetColormap( cEngineFile& file )
{
	if (m_uiColorMapMax < m_usPalLength)
	{
		m_uiColorMapMax = m_usPalLength;

		NiDelete[] m_pkColorMap;
		m_pkColorMap = NiNew NiPalette::PaletteEntry[m_uiColorMapMax];
	}

	if (m_uiRawDataMax < (unsigned int)(m_usPalLength << 2))
	{
		m_uiRawDataMax = m_usPalLength << 2;

		NiFree(m_pucRawData);
		m_pucRawData = NiAlloc(unsigned char, m_uiRawDataMax);
	}

	NiPalette::PaletteEntry* pkDest = m_pkColorMap;

	// Read appropriate number of bytes, break into rgb & put in map.
	switch (m_ucCoSize)
	{
	case 8:             // Greyscale, read and triplicate.
		{
			unsigned char* pucRaw = m_pucRawData;

			StreamLoadBinary(file, (char *)m_pucRawData, m_usPalLength);

			for (unsigned int i = 0; i < m_usPalLength; i++)
			{
				pkDest->m_ucRed = pkDest->m_ucGreen = pkDest->m_ucBlue
					= *pucRaw;
				pkDest->m_ucAlpha = 255;

				pkDest++;
				pucRaw++;
			}
		}
		break;

	case 15:            // 5 bits each of red green and blue.
	case 16:            // Watch for byte order.
		{
			unsigned char* pucRaw = m_pucRawData;

			// cannot treat these directly as unsigned shorts, or else
			// the code will not be little/big endian compatible
			StreamLoadBinary(file, (char *)m_pucRawData, m_usPalLength*2);

			for (unsigned int i = 0; i < m_usPalLength; i++)
			{
				pkDest->m_ucRed = ( pucRaw[1] & 0x7C ) << 1;
				pkDest->m_ucGreen = (( pucRaw[1] & 0x03 ) << 6 ) 
					+ (( pucRaw[0] & 0xE0 ) >> 2 );
				pkDest->m_ucBlue = ( pucRaw[0] & 0x1F ) << 3;
				pkDest->m_ucAlpha = 255;

				pkDest++;
				pucRaw += 2;
			}
		}
		break;

	case 24:            // 8 bits each of blue green and red.
		{
			unsigned char* pucRaw = m_pucRawData;

			StreamLoadBinary(file, (char*)m_pucRawData, m_usPalLength*3);

			for (unsigned int i = 0; i < m_usPalLength; i++)
			{
				pkDest->m_ucBlue = *(pucRaw++);
				pkDest->m_ucGreen = *(pucRaw++);
				pkDest->m_ucRed = *(pucRaw++);
				pkDest->m_ucAlpha = 255;

				pkDest++;
			}
		}
		break;

	case 32:
		{
			unsigned char* pucRaw = m_pucRawData;

			StreamLoadBinary(file, (char *)m_pucRawData, m_usPalLength*4);

			for (unsigned int i = 0; i < m_usPalLength; i++)
			{
				pkDest->m_ucBlue = *(pucRaw++);
				pkDest->m_ucGreen = *(pucRaw++);
				pkDest->m_ucRed = *(pucRaw++);
				pkDest->m_ucAlpha = *(pucRaw++);

				pkDest++;
			}
		}
		break;

	default:
		break;
	};
}

bool cTGAReader::ReadHeader( cEngineFile& file, unsigned int& width, 
							unsigned int& height, NiPixelFormat& pixelFormat, bool& mipmap, 
							unsigned int& faces)
{
	// TGA files are always little endian
	bool bPlatformLittle = NiSystemDesc::GetSystemDesc().IsLittleEndian();
	file.SetEndianSwap(!bPlatformLittle);

	// Since the reader is usually owned by a static image converter class,
	// we lock for safety.  We wait as long as possible since locals will
	// be in thread context.
	m_kReadCriticalSection.Lock();
	faces = 1;

	StreamLoadBinary(file, m_ucIDLength);
	StreamLoadBinary(file, m_ucCoMapType);
	StreamLoadBinary(file, m_ucImgType);
	StreamLoadBinary(file, m_usMinPalIndex);
	StreamLoadBinary(file, m_usPalLength);
	StreamLoadBinary(file, m_ucCoSize);
	StreamLoadBinary(file, m_usXOrigin);
	StreamLoadBinary(file, m_usYOrigin);
	StreamLoadBinary(file, m_usWidth);
	StreamLoadBinary(file, m_usHeight);
	StreamLoadBinary(file, m_ucPixelSize);
	StreamLoadBinary(file, m_ucAttBits);
	// skip ID field
	if (m_ucIDLength != 0)
		file.Seek( m_ucIDLength, SEEK_CUR );

	m_bFlipVert = (m_ucAttBits & 0x20) ? false : true;

	switch (m_ucImgType)  
	{
	case TGA_Map:
		m_bColormapped = true;
		m_bRLE = false;
		break;
	case TGA_Mono:
		m_bColormapped = false;
		m_bRLE = false;
		break;
	case TGA_RGB:
		m_bColormapped = false;
		m_bRLE = false;
		break;
	case TGA_RLEMap:
		m_bColormapped = true;
		m_bRLE = true;
		break;
	case TGA_RLEMono:
		m_bColormapped = false;
		m_bRLE = true;
		break;
	case TGA_RLERGB:
		m_bColormapped = false;
		m_bRLE = true;
		break;

	default:
		m_kReadCriticalSection.Unlock();
		return false;
	};

	m_bAlpha = ((m_bColormapped ? m_ucCoSize : m_ucPixelSize) == 32) 
		? true : false;

	if (m_bColormapped)
	{
		if (m_usPalLength == 16)
			m_kFormat = m_bAlpha ? NiPixelFormat::PALA4 : NiPixelFormat::PAL4;
		else
			m_kFormat = m_bAlpha ? NiPixelFormat::PALA8 : NiPixelFormat::PAL8;
	}
	else
	{
		m_kFormat = m_bAlpha ? NiPixelFormat::RGBA32 : NiPixelFormat::RGB24;
	}

	switch (m_ucPixelSize)
	{
	case 4:
		// In this case, we actually have multiple pixels per byte.
		// A value of 0 is an indication of this.
		m_uiBytesPerPixel = 0; 

		assert(m_bColormapped);

		if (m_bColormapped == false)
		{
			m_kReadCriticalSection.Unlock();
			return false;
		}

		m_pfnUnpacker = &cTGAReader::Unpack4BitSourceRowPal;

		break;

	case 8:
		m_uiBytesPerPixel = 1;

		if (m_bColormapped)
		{
			m_pfnUnpacker = &cTGAReader::Unpack8BitSourceRowPal;
		}
		else
		{
			m_pfnUnpacker = &cTGAReader::Unpack8BitSourceRowGray;
		}
		break;
	case 15:
	case 16:
		m_uiBytesPerPixel = 2;

		if (m_bColormapped)
		{
			if (m_bAlpha)
			{
				m_pfnUnpacker 
					= &cTGAReader::Unpack16BitSourceRowIndexedAlpha;
			}
			else
			{
				m_pfnUnpacker = &cTGAReader::Unpack16BitSourceRowIndexed;
			}
		}
		else
		{
			m_pfnUnpacker = &cTGAReader::Unpack16BitSourceRowRGB;
		}
		break;
	case 24:
		m_uiBytesPerPixel = 3;

		m_pfnUnpacker = &cTGAReader::Unpack24BitSourceRow;
		break;
	case 32:
		m_uiBytesPerPixel = 4;

		m_pfnUnpacker = &cTGAReader::Unpack32BitSourceRow;
		break;
	};

	m_bRLEncodedRun = false;
	m_uiRLECount = 0;

	width = m_usWidth;
	height = m_usHeight;
	pixelFormat = m_kFormat;
	mipmap = false;

	m_kReadCriticalSection.Unlock();
	return true;
}

NiPixelData* cTGAReader::ReadFile( cEngineFile& file, NiPixelData* optDest)
{
	bool bMipmap;
	unsigned int uiW, uiH;
	NiPixelFormat kFmt;
	unsigned int uiNumFaces;

	// Since the reader is usually owned by a static image converter class,
	// we lock for safety.  We wait as long as possible since locals will
	// be in thread context.
	m_kReadCriticalSection.Lock();

	if (!ReadHeader(file, uiW, uiH, kFmt, bMipmap, uiNumFaces))
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}

	unsigned int uiRowSize = m_uiBytesPerPixel * m_usWidth;

	if (m_uiBytesPerPixel == 0)
	{
		if (m_ucPixelSize == 4)
		{
			// Assert if width is odd, since that format is not supported.
			assert(!(m_usWidth & 0x1));

			// 4-bit PixelSize is a special case since there
			// are multiple pixels per byte.  To handle this,
			// we divide the width in half to reflect the two
			// pixels in a byte.
			uiRowSize = m_usWidth >> 1;
		}
		else
		{
			// SubByte formats other than the 4-bit case are not handled.
			assert(0);
			m_kReadCriticalSection.Unlock();
			return NULL;
		}
	}

	// If required, read the color map information
	if (m_ucCoMapType != 0)
	{
		unsigned int uiPaletteEntries = m_usMinPalIndex + m_usPalLength;

		if (( uiPaletteEntries + 1 ) >= MAXCOLORS)
		{
			m_kReadCriticalSection.Unlock();
			return NULL;
		}

		GetColormap(file);
	}
	else
	{
		// Color-mapped image must have a palette!
		if (m_bColormapped)
		{
			m_kReadCriticalSection.Unlock();
			return NULL;
		}
	}

	NiPixelData* pkDest;

	if (optDest && (optDest->GetWidth() == m_usWidth) && 
		(optDest->GetHeight() == m_usHeight) &&
		(optDest->GetPixelFormat() == m_kFormat))
	{
		pkDest = optDest;
	}
	else
	{
		pkDest = NiNew NiPixelData(m_usWidth, m_usHeight, m_kFormat);
	}

	// If already have a color map, assign it.
	if (m_bColormapped)
	{
		pkDest->SetPalette(NiNew NiPalette(m_bAlpha, m_usPalLength,
			m_pkColorMap));
	}

	// Resize the temporary data as needed - must be able to fit at least
	// one row of packed pixels
	if (m_uiRawDataMax < uiRowSize)
	{
		m_uiRawDataMax = uiRowSize;

		NiFree(m_pucRawData);
		m_pucRawData = NiAlloc(unsigned char, m_uiRawDataMax);

	}

	// Read the Targa file body and convert to raw
	unsigned char* pucDestPixels = pkDest->GetPixels(0);

	int iRowOffset = pkDest->GetWidth() * pkDest->GetPixelStride();

	if (iRowOffset == 0)
	{
		// Assert if width is odd, since that format is not supported.
		assert(!(pkDest->GetWidth() & 0x1));

		// iRowOffset is 0 in 4 bit case...
		// Handling this oddity here.
		iRowOffset = pkDest->GetWidth() >> 1;
	}

	if (m_bFlipVert)
	{
		pucDestPixels += iRowOffset * (m_usHeight - 1);
		iRowOffset = -iRowOffset;
	}

	if (m_bRLE)
	{
		// The RLE code decodes the raw pixels and then makes another pass to
		// convert the raw pixels to final, 888(8) pixels.  This is two 
		// passes per row, but the performance is still quite high (~5x 
		// better than the existing NI TGA loader), and it allows for fewer
		// RLE handling functions
		for (unsigned int uiRow = 0; uiRow < m_usHeight; uiRow++)
		{
			// Load a row of raw pixels into temporary storage by
			// decoding run lengths
			unsigned int uiRawDataSize = m_uiRawDataMax * 
				sizeof(unsigned char);
			LoadRLERow(file, m_pucRawData, uiRawDataSize);

			// Unpack the 8,15,16,24, or 32 bit indexed or true color pixels
			(this->*m_pfnUnpacker)(m_pucRawData, pucDestPixels);

			pucDestPixels += iRowOffset;
		}
	}
	else
	{
		// Optimized, fast non-RLE reads - generally up to 7.7x faster than
		// the existing NI TGA loader.
		for (unsigned int uiRow = 0; uiRow < m_usHeight; uiRow++)
		{
			// Load a row of raw pixels into temporary storage directly 
			// from the image file
			StreamLoadBinary(file, (char *)m_pucRawData, uiRowSize);

			// Unpack the 8,15,16,24, or 32 bit indexed or true color pixels
			(this->*m_pfnUnpacker)(m_pucRawData, pucDestPixels);

			pucDestPixels += iRowOffset;
		}
	}

	m_kReadCriticalSection.Unlock();
	return pkDest;
}

//////////////////////////////////////////////////////////////////////////

#include "NiDevImageConverter.h"
#include "NiPixelData.h"
#include "NiDDSReader.h"
#include "NiTextureCodec.h"

cDDSReader::cDDSReader()
{

}

cDDSReader::~cDDSReader()
{

}

bool cDDSReader::ReadHeader( cEngineFile& file, unsigned int& uiWidth, unsigned int& uiHeight, NiPixelFormat& kFormat, bool& bMipmap, unsigned int& uiFaces )
{
	// Since the reader is usually owned by a static image converter class,
	// we lock for safety.
	m_kReadCriticalSection.Lock();

	unsigned int uiTemp;

	unsigned int uiMagicNumber;
	StreamLoadBinary(file, uiMagicNumber);
	if (uiMagicNumber != NI_DDS) // "DDS" is the header for all DDS files
	{
		// It's possible this is a still valid file, but just has the
		// reverse endianness.
		unsigned int uiSwapped = uiMagicNumber;
		NiEndian::Swap32((char*)&uiSwapped);
		if (uiSwapped == NI_DDS)
		{
			// swap and continue loading
			file.SetEndianSwap(true);
		}
		else
		{
			m_kReadCriticalSection.Unlock();
			return false;
		}
	}

	// surface description header
	unsigned int uiSize;
	StreamLoadBinary(file, uiSize); // size
	if (uiSize != 124)
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}

	// Value validity flags
	unsigned int uiFlags;
	StreamLoadBinary(file, uiFlags); // flags

	// We ignore the following flags:
	// DDSD_COMP_ALPHABITDEPTH 
	// DDSD_CKDESTBLT 
	// DDSD_CKDESTOVERLAY 
	// DDSD_CKSRCBLT 
	// DDSD_CKSRCOVERLAY 
	// DDSD_LPSURFACE ???
	// DDSD_REFRESHRATE 
	// DDSD_TEXTURESTAGE 
	// DDSD_ZBUFFERBITDEPTH 
	// DDSD_PITCH

	// We need the following flags
	// DDSD_CAPS 
	// DDSD_HEIGHT 
	// DDSD_PIXELFORMAT 
	// DDSD_WIDTH
	const unsigned int uiRequiredFlags = NI_DDSD_CAPS | NI_DDSD_HEIGHT | 
		NI_DDSD_PIXELFORMAT | NI_DDSD_WIDTH;

	if ((uiFlags & uiRequiredFlags) != uiRequiredFlags)
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}

	// We can handle the following flags
	// DDSD_MIPMAPCOUNT 

	// We do not want to see the following flags
	// DDSD_BACKBUFFERCOUNT - unless the count is 0

	StreamLoadBinary(file, m_uiHeight);
	StreamLoadBinary(file, m_uiWidth);

	// Data size
	StreamLoadBinary(file, uiTemp);

	// dwDepth
	StreamLoadBinary(file, uiTemp);

	// Cannot handle volume textures in file
	if ((uiFlags & NI_DDSD_DEPTH) && uiTemp)
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}

	// dwMipMapCount
	unsigned int uiMipmapCount;
	StreamLoadBinary(file, uiMipmapCount);

	// dwReserved
	file.Seek(4*11,NiFile::ms_iSeekCur);

	////////////////
	// pixel format
	StreamLoadBinary(file, uiTemp); // dwSize

	unsigned int uiFormatFlags;
	StreamLoadBinary(file, uiFormatFlags); // dwFlags

	unsigned int uiCompressionCode;
	StreamLoadBinary(file, uiCompressionCode); // dwFourCC

	if ((uiFormatFlags & NI_FOURCC))
	{
		// can only handle DXT1, DXT3, or DXT5 files for now
		switch (uiCompressionCode)
		{
		case NI_DXTC1:
			m_kOriginalFormat = NiPixelFormat::DXT1;
			break;
		case NI_DXTC3:
			m_kOriginalFormat = NiPixelFormat::DXT3;
			break;
		case NI_DXTC5:
			m_kOriginalFormat = NiPixelFormat::DXT5;
			break;
		case NI_R16:
			m_kOriginalFormat = NiPixelFormat::R16;
			break;
		case NI_RG32:
			m_kOriginalFormat = NiPixelFormat::RG32;
			break;
		case NI_RGBA64:
			m_kOriginalFormat = NiPixelFormat::RGBA64;
			break;
		case NI_R32:
			m_kOriginalFormat = NiPixelFormat::R32;
			break;
		case NI_RG64:
			m_kOriginalFormat = NiPixelFormat::RG64;
			break;
		case NI_RGBA128:
			m_kOriginalFormat = NiPixelFormat::RGBA128;
			break;
		default:
			m_kReadCriticalSection.Unlock();
			return false;
			break;
		}

		kFormat = m_kOriginalFormat;
		// Ignore the following fields for compressed textures:
		//  dwRGBBitCount
		//  dwRBitMask
		//  dwGBitMask
		//  dwBBitMask
		//  dwRGBAlphaBitMask

		file.Seek(4*5,NiFile::ms_iSeekCur);
	}
	else if ((uiFormatFlags & NI_RGB))
	{
		unsigned int uiBitsPerPixel;
		StreamLoadBinary(file, uiBitsPerPixel); // dwRGBBitCount

		unsigned int uiRedMask;
		StreamLoadBinary(file, uiRedMask);    // dwRBitMask

		unsigned int uiGreenMask;
		StreamLoadBinary(file, uiGreenMask);    // dwGBitMask

		unsigned int uiBlueMask;
		StreamLoadBinary(file, uiBlueMask);    // dwBBitMask

		unsigned int uiAlphaMask;
		StreamLoadBinary(file, uiAlphaMask);    // dwRGBAlphaBitMask

		if (!(uiFormatFlags & NI_ALPHAPIXELS))
			uiAlphaMask = 0x00000000;

		if (!ValidateRGBABitmasks(uiRedMask, uiGreenMask, 
			uiBlueMask, uiAlphaMask, uiBitsPerPixel))
		{
			m_kReadCriticalSection.Unlock();
			return false;
		}

		kFormat = NiPixelFormat::CreateFromRGBAMasks((unsigned char)uiBitsPerPixel,
			uiRedMask, uiGreenMask, uiBlueMask, uiAlphaMask);

		// We need to tell the pixel format that it is big endian because 
		// we'll be swapping the pixels themselves. Otherwise, we end up 
		// with masks that are non-contiguous and VERY bad things 
		// happen on conversion.
		kFormat.SetLittleEndian(
			NiSystemDesc::GetSystemDesc().IsLittleEndian());

		m_kOriginalFormat = kFormat;
		kFormat = ComputeFinalFormat(kFormat);

	}
	else
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}


	///////////////////////
	// surface description

	// dwCaps
	unsigned int uiCaps1;
	StreamLoadBinary(file, uiCaps1);

	// The following caps are allowed
	// DDSCAPS_COMPLEX - we can handle mipmaps
	// DDSCAPS_TEXTURE - we want to deal with textures only
	// DDSCAPS_MIPMAP - we can handle mipmaps

	if ((uiCaps1 & NI_DDSCAPS_COMPLEX) ||
		(uiCaps1 & NI_DDSCAPS_TEXTURE) ||
		(uiCaps1 & NI_DDSCAPS_MIPMAP))
	{
		// we can support these flags
	}
	else
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}

	// dwCaps2
	unsigned int uiCaps2;
	StreamLoadBinary(file, uiCaps2);

	// We cannot handle volume maps
	if ((uiCaps2 & NI_DDSCAPS2_VOLUME) != 0)
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}

	// We can handle cube maps, but they must all be defined
	if ((uiCaps2 & NI_DDSCAPS2_CUBEMAP) != 0 &&
		(((uiCaps2 & NI_DDSCAPS2_CUBEMAP_POSITIVEX) == 0) || 
		((uiCaps2 & NI_DDSCAPS2_CUBEMAP_NEGATIVEX) == 0) || 
		((uiCaps2 & NI_DDSCAPS2_CUBEMAP_POSITIVEY) == 0) || 
		((uiCaps2 & NI_DDSCAPS2_CUBEMAP_NEGATIVEY) == 0) || 
		((uiCaps2 & NI_DDSCAPS2_CUBEMAP_POSITIVEZ) == 0) || 
		((uiCaps2 & NI_DDSCAPS2_CUBEMAP_NEGATIVEZ) == 0)))
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}

	if ((uiCaps2 & NI_DDSCAPS2_CUBEMAP) != 0)
	{
		uiFaces = 6;
	}
	else
	{
		uiFaces = 1;
	}

	// dwReserved[2]
	// dwReserved2
	file.Seek(4*3,NiFile::ms_iSeekCur);

	// file pointer now points to data start

	uiWidth = m_uiWidth;
	uiHeight = m_uiHeight;

	if ((uiCaps1 & NI_DDSCAPS_COMPLEX) && (uiCaps1 & NI_DDSCAPS_MIPMAP) &&
		(uiMipmapCount != 1))
	{
		bMipmap = true;
		m_uiMipmapLevels = uiMipmapCount;
	}
	else
	{
		bMipmap = false;
		m_uiMipmapLevels = 1;
	}

	m_kReadCriticalSection.Unlock();
	return true;
}

void cDDSReader::Read24Bit( cEngineFile& file, NiPixelData* pkDest, NiPixelFormat kSrcFmt, NiPixelFormat kDestFmt, unsigned int uiMipMapIdx, unsigned int uiFaceIdx )
{
	unsigned char* pucDest = pkDest->GetPixels(uiMipMapIdx, uiFaceIdx);
	unsigned int uiSize = pkDest->GetSizeInBytes(uiMipMapIdx, uiFaceIdx);

	StreamLoadBinary(file, (char *)pucDest, uiSize);

	NiDevImageConverter::PixelBits kSrcBits(kSrcFmt);

	unsigned int uiWidth = pkDest->GetWidth(uiMipMapIdx, uiFaceIdx);
	unsigned int uiHeight = pkDest->GetHeight(uiMipMapIdx, uiFaceIdx);
	if (kSrcFmt != kDestFmt)
	{
		if (kSrcBits.m_uiRM == 0x00ff0000)
		{
			const unsigned char* pucSrc = pucDest;
			// must expand data from BGR to RGB
			for (unsigned int y = 0; y < uiHeight; y++)
			{
				for (unsigned int x = 0; x < uiWidth; x++)
				{
					unsigned char ucR = *(pucSrc+2);
					unsigned char ucG = *(pucSrc+1);
					unsigned char ucB = *pucSrc;
					*(pucDest++) = ucR;
					*(pucDest++) = ucG;
					*(pucDest++) = ucB;

					pucSrc+=3;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
#define PackPixel(val, kBits, cComp) \
	(((((unsigned long)val)>> kBits. m_uc##cComp##Q) \
	<< kBits. m_uc##cComp##S) & kBits. m_ui##cComp##M)

#define UnpackPixel(pS, kBits, cComp) \
	((unsigned char)((((*pS) & kBits. m_ui##cComp##M) \
	>> kBits. m_uc##cComp##S) << kBits. m_uc##cComp##Q))

//---------------------------------------------------------------------------
void cDDSReader::Read32Bit( cEngineFile& file, NiPixelData* pkDest, NiPixelFormat kSrcFmt, NiPixelFormat kDestFmt, unsigned int uiMipMapIdx, unsigned int uiFaceIdx )
{
	unsigned char* pucDest = pkDest->GetPixels(uiMipMapIdx, uiFaceIdx);
	unsigned int uiSize = pkDest->GetSizeInBytes(uiMipMapIdx, uiFaceIdx);

	StreamLoadBinary(file, (char *)pucDest, uiSize);

	NiDevImageConverter::PixelBits kSrcBits(kSrcFmt);

	unsigned int uiWidth = pkDest->GetWidth(uiMipMapIdx, uiFaceIdx);
	unsigned int uiHeight = pkDest->GetHeight(uiMipMapIdx, uiFaceIdx);
	if (kSrcFmt != kDestFmt)
	{
		unsigned int* puiSrc = (unsigned int*) pucDest;

		if (!NiSystemDesc::GetSystemDesc().IsLittleEndian())
		{
			NiEndian::Swap32((char*)puiSrc, uiSize / 4);
		}

		if (kSrcFmt.GetBits(NiPixelFormat::COMP_ALPHA) != 0)
		{
			for (unsigned int y = 0; y < uiHeight; y++)
			{
				for (unsigned int x = 0; x < uiWidth; x++)
				{
					unsigned char R = NiDevImageConverter::UnpackRedChannel(
						*puiSrc, kSrcBits);
					unsigned char G = NiDevImageConverter::UnpackGreenChannel(
						*puiSrc, kSrcBits);
					unsigned char B = NiDevImageConverter::UnpackBlueChannel(
						*puiSrc, kSrcBits);
					unsigned char A = NiDevImageConverter::UnpackAlphaChannel(
						*puiSrc, kSrcBits);
					*(pucDest++) = R;
					*(pucDest++) = G;
					*(pucDest++) = B;
					*(pucDest++) = A;

					puiSrc++;
				}
			}
		}
		else
		{
			for (unsigned int y = 0; y < uiHeight; y++)
			{
				for (unsigned int x = 0; x < uiWidth; x++)
				{
					unsigned char R = NiDevImageConverter::UnpackRedChannel(
						*puiSrc, kSrcBits);
					unsigned char G = NiDevImageConverter::UnpackGreenChannel(
						*puiSrc, kSrcBits);
					unsigned char B = NiDevImageConverter::UnpackBlueChannel(
						*puiSrc, kSrcBits);
					unsigned char A = 255;
					*(pucDest++) = R;
					*(pucDest++) = G;
					*(pucDest++) = B;
					*(pucDest++) = A;

					puiSrc++;
				}
			}
		}
	}
}

void cDDSReader::Read16Bit( cEngineFile& file, NiPixelData* pkDest, NiPixelFormat kSrcFmt, NiPixelFormat kDestFmt, unsigned int uiMipMapIdx, unsigned int uiFaceIdx )
{
	unsigned char pucSrc[512];
	unsigned char* pucDest = pkDest->GetPixels(uiMipMapIdx, uiFaceIdx);
	unsigned int uiSize = pkDest->GetWidth(uiMipMapIdx, uiFaceIdx) * 
		pkDest->GetHeight(uiMipMapIdx, uiFaceIdx) * 2;

	NiDevImageConverter::PixelBits kSrcBits(kSrcFmt);

	for (unsigned int uiBytesWritten = 0; uiBytesWritten < uiSize; )
	{
		unsigned int uiBuffSize = 512;
		if (uiBuffSize > uiSize - uiBytesWritten)
			uiBuffSize = uiSize - uiBytesWritten;

		StreamLoadBinary(file, (char *)pucSrc, uiBuffSize);

		if (!NiSystemDesc::GetSystemDesc().IsLittleEndian())
		{
			NiEndian::Swap16((char*)pucSrc, uiBuffSize / 2);
		}

		if (kDestFmt == NiPixelFormat::RGBA32)
		{
			const unsigned short* pusSrc = (const unsigned short*)pucSrc;
			for (unsigned int y = 0; y < uiBuffSize/2; y++)
			{
				*(pucDest++) = 
					NiDevImageConverter::UnpackRedChannel(*pusSrc, kSrcBits);
				*(pucDest++) = 
					NiDevImageConverter::UnpackGreenChannel(*pusSrc, kSrcBits);
				*(pucDest++) = 
					NiDevImageConverter::UnpackBlueChannel(*pusSrc, kSrcBits);
				*(pucDest++) = 
					NiDevImageConverter::UnpackAlphaChannel(*pusSrc, kSrcBits);
				pusSrc++;
			}
		}
		else if (kDestFmt == NiPixelFormat::RGB24)
		{
			const unsigned short* pusSrc = (const unsigned short*)pucSrc;
			for (unsigned int y = 0; y < uiBuffSize/2; y++)
			{
				*(pucDest++) = 
					NiDevImageConverter::UnpackRedChannel(*pusSrc, kSrcBits);
				*(pucDest++) = 
					NiDevImageConverter::UnpackGreenChannel(*pusSrc, kSrcBits);
				*(pucDest++) = 
					NiDevImageConverter::UnpackBlueChannel(*pusSrc, kSrcBits);
				pusSrc++;
			}
		}
		else
		{
			assert(0);
			return;
		}
		uiBytesWritten += uiBuffSize;
	}
}

NiPixelData* cDDSReader::ReadFile( cEngineFile& file, NiPixelData* optDest )
{
	bool bMipmap;
	unsigned int uiW, uiH, uiMipmapLevels;
	NiPixelFormat kFmt;
	unsigned int uiNumFaces;

	// Since the reader is usually owned by a static image converter class,
	// we lock for safety.
	m_kReadCriticalSection.Lock();

	if (!ReadHeader(file, uiW, uiH, kFmt, bMipmap, uiNumFaces))
	{
		m_kReadCriticalSection.Unlock();
		return NULL;
	}

	NiPixelData* pkDest;

	if (optDest && (optDest->GetWidth() == m_uiWidth) && 
		(optDest->GetHeight() == m_uiHeight) &&
		(optDest->GetPixelFormat() == kFmt) &&
		(optDest->GetNumMipmapLevels() == m_uiMipmapLevels) &&
		(optDest->GetNumFaces() == uiNumFaces))
	{
		pkDest = optDest;
	}
	else
	{
		pkDest = NiNew NiPixelData(m_uiWidth, m_uiHeight, kFmt, 
			m_uiMipmapLevels, uiNumFaces);
	}

	uiMipmapLevels = pkDest->GetNumMipmapLevels();

	if (kFmt == m_kOriginalFormat)
	{
		// The data for the mipmap levels follows directly, one mipmap level
		// after another
		unsigned int i;

		for (unsigned int uiFaceIdx = 0; uiFaceIdx < uiNumFaces; uiFaceIdx++)
		{
			unsigned int uiTrueFaceIdx = RemapFace(uiFaceIdx);
			for (i = 0; i < uiMipmapLevels; i++)
			{
				unsigned char* pucDest = pkDest->GetPixels(i, uiTrueFaceIdx);
				unsigned int uiSize = pkDest->GetSizeInBytes(i, uiTrueFaceIdx);

				StreamLoadBinary(file, (char *)pucDest, uiSize);
			}
		}
	}
	else 
	{
		for (unsigned int uiFaceIdx = 0; uiFaceIdx < uiNumFaces; uiFaceIdx++)
		{
			unsigned int uiTrueFaceIdx = RemapFace(uiFaceIdx);

			for (unsigned int i = 0; i < uiMipmapLevels; i++)
			{
				switch(m_kOriginalFormat.GetBitsPerPixel())
				{
				case 32:
					Read32Bit(file, pkDest, m_kOriginalFormat, kFmt, i,
						uiTrueFaceIdx);
					break;
				case 16:
					Read16Bit(file, pkDest, m_kOriginalFormat, kFmt, i,
						uiTrueFaceIdx);
					break;
				case 24:
					Read24Bit(file, pkDest, m_kOriginalFormat, kFmt, i,
						uiTrueFaceIdx);
					break;
				}
			}
		}
	}

	m_kReadCriticalSection.Unlock();
	return pkDest;
}


//////////////////////////////////////////////////////////////////////////

NiPixelData* cBMPReader::ReadFile(cEngineFile& file, NiPixelData* optDest, bool IsXorHeader /* = false  */)
{
	bool bMipmap;
	unsigned int uiW, uiH;
	NiPixelFormat kFmt;
	unsigned int uiNumFaces;

	// Since the reader is usually owned by a static image converter class,
	// we lock for safety.
	m_kReadCriticalSection.Lock();

	if (!ReadHeader(file, uiW, uiH, kFmt, bMipmap, uiNumFaces, IsXorHeader))
	{
		m_kReadCriticalSection.Unlock();
		return NULL;
	}

	if (m_kFormat == NiPixelFormat::PAL8 && m_uiClrUsed > 256 ||
		m_kFormat == NiPixelFormat::PAL4 && m_uiClrUsed > 16)
	{
		m_kReadCriticalSection.Unlock();
		return NULL;
	}


	NiPixelData* pkDest;

	if (optDest && (optDest->GetWidth() == m_uiWidth) && 
		(optDest->GetHeight() == m_uiHeight) &&
		(optDest->GetPixelFormat() == m_kFormat))
	{
		pkDest = optDest;
	}
	else
	{
		pkDest = NiNew NiPixelData(m_uiWidth, m_uiHeight, m_kFormat);
	}

	// read in color map (B G R Res) - ignore if not palettized image
	if (m_kFormat == NiPixelFormat::PAL8 || m_kFormat == NiPixelFormat::PAL4)
	{
		NIASSERT(m_uiClrUsed);
		NIASSERT(m_usBPP <= 8);

		// assumes that palette has at most 256 entries
		WINDOWS_RGBQUAD akSrcPal[256];
		NiPalette::PaletteEntry* pkPal = (NiPalette::PaletteEntry*)akSrcPal;

		StreamLoadBinary(file, (char *)akSrcPal, 
			m_uiClrUsed * sizeof(WINDOWS_RGBQUAD));

		// Flip the red and blue to match raw data format RGB
		for (unsigned int i = 0; i < m_uiClrUsed; i++)
		{
			unsigned char ucTmp = pkPal->m_ucRed;
			pkPal->m_ucRed = pkPal->m_ucBlue;
			pkPal->m_ucBlue = ucTmp;
			pkPal->m_ucAlpha = 255;

			pkPal++;
		}

		pkDest->SetPalette(NiNew NiPalette(false, 256,
			(NiPalette::PaletteEntry*) akSrcPal));
	}
	else
	{
		// seek past colormap - we do not need it
		file.Seek(m_uiClrUsed * sizeof(WINDOWS_RGBQUAD), NiFile::ms_iSeekCur); 
	}

	// Skip over any extra data to the set of pixels
	int iSkipAmt = (m_uiOffBits - WINDOWS_BMPTOTALHEADER_SIZE
		- m_uiClrUsed * sizeof(WINDOWS_RGBQUAD));

	if (iSkipAmt > 0)
		file.Seek(iSkipAmt, NiFile::ms_iSeekCur); 

	// The image size is the width of a row (padded to 4 byte alignment) times
	// the number of rows in the image
	unsigned int uiSizeImage 
		= ((((m_uiWidth * m_usBPP) + 31) & ~31) >> 3) * m_uiHeight;

	unsigned int uiDestRowStride 
		= pkDest->GetPixelStride() * pkDest->GetWidth();

	// must expand 4 bit per pixel images into 8 BPP
	if (m_usBPP == 4)
	{
		// pad out to an even number of pixels per row
		unsigned int uiInRowStride = (m_uiWidth >> 1) + (m_uiWidth & 0x1);

		uiDestRowStride = uiInRowStride;

		// pad input stride
		unsigned int uiInRowPad = (uiInRowStride & 0x3) 
			? 4 - (uiInRowStride & 0x3) : 0;

		unsigned char *pucInput = NiAlloc(unsigned char,
			uiInRowStride+uiInRowPad);

		unsigned char* pucScanLine; 
		int iRowStep;
		if (m_bFlipVert)
		{
			// flip scanlines while reading

			// points to start of final scanline in image
			pucScanLine = pkDest->GetPixels(0) 
				+ uiDestRowStride * (pkDest->GetHeight() - 1);

			iRowStep = -(int)(uiDestRowStride);
		}
		else
		{
			// read pixel data directly into image 

			// points to start of first scanline in image
			pucScanLine = pkDest->GetPixels(0); 

			iRowStep = uiDestRowStride;
		}

		if (m_kFormat == NiPixelFormat::PAL4)
		{
			if (uiInRowPad)
			{
				for (unsigned int i = 0; i < m_uiHeight; i++, pucScanLine += iRowStep)
				{
					StreamLoadBinary(file, pucScanLine, uiInRowStride);
					unsigned char aucDummy[4];
					StreamLoadBinary(file, aucDummy, uiInRowPad);
				}
			}
			else
			{
				for (unsigned int i = 0; i < m_uiHeight; i++, pucScanLine += iRowStep)
				{
					StreamLoadBinary(file, pucScanLine, uiInRowStride);
				}
			}
		}
		else
		{
			for (unsigned int i = 0; i < m_uiHeight; i++, pucScanLine += iRowStep)
			{
				StreamLoadBinary(file, pucInput, uiInRowStride+uiInRowPad);

				unsigned char *pucPixel = pucScanLine;
				unsigned char *pucInputPixel = pucInput;

				// decode pixels in pairs
				for (unsigned int j=0; j < uiInRowStride; j++)
				{
					*(pucPixel++) = (*pucInputPixel) >> 4;
					*(pucPixel++) = (*pucInputPixel) &  0x0f;
					pucInputPixel++;
				}
			}
		}

		NiFree(pucInput);
	}
	else // 8, 24 or 32-bit data
	{
		if (m_bFlipVert)
		{
			// flip scanlines while reading

			// pad input stride
			// points to start of final scanline in image
			unsigned char* pucScanLine = pkDest->GetPixels(0) 
				+ uiDestRowStride*(m_uiHeight - 1);
			unsigned int i;

			if (uiDestRowStride & 0x3) 
			{
				unsigned int uiInRowPad = 4 - (uiDestRowStride & 0x3);
				unsigned char aucDummy[4];

				for (i = 0; i < m_uiHeight; i++)
				{
					StreamLoadBinary(file, pucScanLine, uiDestRowStride);
					StreamLoadBinary(file, aucDummy, uiInRowPad);
					pucScanLine -= uiDestRowStride;
				}
			}
			else
			{
				for (i = 0; i < m_uiHeight; i++)
				{
					StreamLoadBinary(file, pucScanLine, uiDestRowStride);
					pucScanLine -= uiDestRowStride;
				}
			}
		}
		else
		{
			// read pixel data directly into image 

			// flip scanlines while reading

			// pad input stride
			if (uiDestRowStride & 0x3)
			{
				unsigned int uiInRowPad = 4 - (uiDestRowStride & 0x3);

				// points to start of final scanline in image
				unsigned char* pucScanLine = pkDest->GetPixels(0);

				// Pad will always be fewer than four bytes
				unsigned char aucDummy[4];
				unsigned int i;

				for (i = 0; i < m_uiHeight; i++, 
					pucScanLine += uiDestRowStride)
				{
					StreamLoadBinary(file, pucScanLine, uiDestRowStride);
					StreamLoadBinary(file, aucDummy, uiInRowPad);
				}
			}
			else
			{
				StreamLoadBinary(file, pkDest->GetPixels(0), uiSizeImage);
			}
		}
	}

	// If RGB(A), then flip R and B to match raw data format
	if ((m_kFormat == NiPixelFormat::RGB24) 
		|| (m_kFormat == NiPixelFormat::RGBA32))
	{
		unsigned int uiStride = pkDest->GetPixelStride();
		unsigned int uiSize = m_uiWidth * m_uiHeight;
		unsigned char* pucTmp = pkDest->GetPixels(0);

		for (unsigned int i = 0; i < uiSize; i++, pucTmp += uiStride)
		{
			unsigned char ucSwap = pucTmp[0];
			pucTmp[0] = pucTmp[2];
			pucTmp[2] = ucSwap;
		}
	}

	m_kReadCriticalSection.Unlock();
	return pkDest;
}


bool cBMPReader::ReadHeader(cEngineFile& file, unsigned int& uiWidth, unsigned int& uiHeight, NiPixelFormat& kFormat, bool& bMipmap, unsigned int& uiFaces, bool IsXorHeader /* = false  */)
{
	// Set bmp file as little endian
	bool bPlatformLittle = NiSystemDesc::GetSystemDesc().IsLittleEndian();
	file.SetEndianSwap(!bPlatformLittle);

	uiFaces = 1;
	// temporary data to be used for dummy reads of file values that the
	// importer can ignore
	int iTempInt;
	unsigned int uiTempUnsignedInt;
	unsigned short usTempUnsignedShort;

	// Bitmap File Header
	//     WORD    bfType;
	//     DWORD   bfSize;
	//     WORD    bfReserved1;
	//     WORD    bfReserved2;
	//     DWORD   bfOffBits;

	StreamLoadBinary(file, usTempUnsignedShort); // bfType
	if( IsXorHeader )
		usTempUnsignedShort ^= BMP_HEADER_XOR_KEY ;
	
	// Check the file header for the correct format tag
	if (usTempUnsignedShort != 0x4d42)
		return false;

	// Since the reader is usually owned by a static image converter class,
	// we lock for safety.
	m_kReadCriticalSection.Lock();

	StreamLoadBinary(file, uiTempUnsignedInt); // bfSize (UNUSED)
	StreamLoadBinary(file, usTempUnsignedShort); // bfReserved1 (UNUSED)
	StreamLoadBinary(file, usTempUnsignedShort); // bfReserved2 (UNUSED)
	StreamLoadBinary(file, m_uiOffBits); // bfOffBits

	// Bitmap Info Header
	//     DWORD      biSize;
	//     LONG       biWidth;
	//     LONG       biHeight;
	//     WORD       biPlanes;
	//     WORD       biBitCount;
	//     DWORD      biCompression;
	//     DWORD      biSizeImage;
	//     LONG       biXPelsPerMeter;
	//     LONG       biYPelsPerMeter;
	//     DWORD      biClrUsed;
	//     DWORD      biClrImportant;

	unsigned int uiHeaderSize;
	StreamLoadBinary(file, uiHeaderSize); // biSize

	int iWidth, iHeight;
	StreamLoadBinary(file, iWidth); // biWidth
	StreamLoadBinary(file, iHeight); // biHeight
	StreamLoadBinary(file, usTempUnsignedShort); // biPlanes (UNUSED)
	StreamLoadBinary(file, m_usBPP); // biBitCount

	unsigned int uiCompression;    
	StreamLoadBinary(file, uiCompression); // biCompression

	StreamLoadBinary(file, uiTempUnsignedInt); // biSizeImage
	StreamLoadBinary(file, iTempInt); // biXPelsPerMeter (UNUSED)
	StreamLoadBinary(file, iTempInt); // biYPelsPerMeter (UNUSED)
	StreamLoadBinary(file, m_uiClrUsed); // biClrUsed
	StreamLoadBinary(file, uiTempUnsignedInt); // biClrImportant (UNUSED)

	if( IsXorHeader )
	{
		 m_uiOffBits	   ^= BMP_HEADER_XOR_KEY ;
		 uiHeaderSize	   ^= BMP_HEADER_XOR_KEY ;
		 iWidth			   ^= BMP_HEADER_XOR_KEY ;
		 iHeight		   ^= BMP_HEADER_XOR_KEY ;
		 m_usBPP		   ^= BMP_HEADER_XOR_KEY ;
		 uiCompression	   ^= BMP_HEADER_XOR_KEY ;
		 uiTempUnsignedInt ^= BMP_HEADER_XOR_KEY ;
		 m_uiClrUsed       ^= BMP_HEADER_XOR_KEY ;
	}

	// Check the file version by checking the core header size
	if (uiHeaderSize == sizeof(WINDOWS_BMPCOREHEADER_SIZE))
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}

	// Determine the destination pixel format of the imported image
	// This importer only supports 4, 8, 24, and 32-bit images.
	switch (m_usBPP)
	{
	case 4:
		m_kFormat = NiPixelFormat::PAL4;
		break;
	case 8:
		m_kFormat = NiPixelFormat::PAL8;
		break;
	case 24:
		m_kFormat = NiPixelFormat::RGB24;
		break;
	case 32:
		m_kFormat = NiPixelFormat::RGBA32;
		break;
	default:
		m_kReadCriticalSection.Unlock();
		return false;
	};

	// Importer does not support RLE formats
	if ((uiCompression == WINDOWS_BI_RLE4) || 
		(uiCompression == WINDOWS_BI_RLE8))
	{
		m_kReadCriticalSection.Unlock();
		return false;
	}

	m_uiWidth = (unsigned int)iWidth;

	// If the height is negative, then the image must be flipped vertically
	m_bFlipVert = (iHeight >= 0) ? true : false;

	m_uiHeight = (iHeight > 0) 
		? (unsigned int)iHeight : (unsigned int)(-iHeight);

	// If the colormap size is not specified, then it is based on the number
	// of bits per pixel
	if ((m_uiClrUsed == 0) && (m_usBPP != 24) && (m_usBPP != 32))
		m_uiClrUsed = 1 << m_usBPP;

	uiWidth = m_uiWidth;
	uiHeight = m_uiHeight;
	kFormat = m_kFormat;
	bMipmap = true;

	m_kReadCriticalSection.Unlock();
	return true;
}