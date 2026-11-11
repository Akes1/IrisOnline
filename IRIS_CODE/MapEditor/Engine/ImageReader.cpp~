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
							unsigned int& faces )
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

NiPixelData* cTGAReader::ReadFile( cEngineFile& file, NiPixelData* optDest )
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
