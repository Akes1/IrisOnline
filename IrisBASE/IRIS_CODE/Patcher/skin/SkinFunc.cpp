#include "StdAfx.h"
#include "SkinFunc.h"

HRGN CreateRegion( CDC* dc, int cx, int cy, COLORREF colorKey )
{
	// We start with the full rectangular region
	HRGN hrgnBitmap = ::CreateRectRgn(0, 0, cx, cy);

	BOOL inTransparency = FALSE; // Already inside a transparent part?
	int startX = -1; // Start of transparent part

	// For all rows of the bitmap ...
	for( int y = 0; y < cy; y++ )
	{
		// For all pixels of the current row ...
		// (To close any transparent region, we go one pixel beyond the
		// right boundary)
		for( int x = 0; x <= cx; x++ )
		{
			BOOL transparent = FALSE; // Current pixel transparent?

			// Check for positive transparency within image boundaries
			if( (x < cx) && (colorKey == dc->GetPixel(x, y)) )
			{
				transparent = TRUE;
			}

			// Does transparency change?
			if( inTransparency != transparent )
			{
				if( transparent )
				{
					// Transparency starts. Remember x position.
					inTransparency = TRUE;
					startX = x;
				}
				else
				{
					// Transparency ends (at least beyond image boundaries).
					// Create a region for the transparency, one pixel high,
					// beginning at start_x and ending at the current x, and
					// subtract that region from the current bitmap region.
					// The remainding region becomes the current bitmap region.
					HRGN hrgnTransp = ::CreateRectRgn(startX, y, x, y + 1);
					::CombineRgn(hrgnBitmap, hrgnBitmap, hrgnTransp, RGN_DIFF);
					::DeleteObject(hrgnTransp);

					inTransparency = FALSE;
				}
			}
		}
	}
	return hrgnBitmap;
}

/*
void DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, short xStart,
short yStart, COLORREF cTransparentColor)
{
BITMAP     bm;
COLORREF   cColor;
HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
POINT      ptSize;

hdcTemp = CreateCompatibleDC(hdc);
SelectObject(hdcTemp, hBitmap);   // Select the bitmap

GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
ptSize.x = bm.bmWidth;            // Get width of bitmap
ptSize.y = bm.bmHeight;           // Get height of bitmap
DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device

// to logical points

// Create some DCs to hold temporary data.
hdcBack   = CreateCompatibleDC(hdc);
hdcObject = CreateCompatibleDC(hdc);
hdcMem    = CreateCompatibleDC(hdc);
hdcSave   = CreateCompatibleDC(hdc);

// Create a bitmap for each DC. DCs are required for a number of
// GDI functions.

// Monochrome DC
bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

// Monochrome DC
bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

// Each DC must select a bitmap object to store pixel data.
bmBackOld   = (HBITMAP)SelectObject(hdcBack, bmAndBack);
bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
bmMemOld    = (HBITMAP)SelectObject(hdcMem, bmAndMem);
bmSaveOld   = (HBITMAP)SelectObject(hdcSave, bmSave);

// Set proper mapping mode.
SetMapMode(hdcTemp, GetMapMode(hdc));

// Save the bitmap sent here, because it will be overwritten.
BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

// Set the background color of the source DC to the color.
// contained in the parts of the bitmap that should be transparent
cColor = SetBkColor(hdcTemp, cTransparentColor);

// Create the object mask for the bitmap by performing a BitBlt
// from the source bitmap to a monochrome bitmap.
BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
SRCCOPY);

// Set the background color of the source DC back to the original
// color.
SetBkColor(hdcTemp, cColor);

// Create the inverse of the object mask.
BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
NOTSRCCOPY);

// Copy the background of the main DC to the destination.
BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
SRCCOPY);

// Mask out the places where the bitmap will be placed.
BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

// Mask out the transparent colored pixels on the bitmap.
BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

// XOR the bitmap with the background on the destination DC.
BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

// Copy the destination to the screen.
BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
SRCCOPY);

// Place the original bitmap back into the bitmap sent here.
BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

// Delete the memory bitmaps.
DeleteObject(SelectObject(hdcBack, bmBackOld));
DeleteObject(SelectObject(hdcObject, bmObjectOld));
DeleteObject(SelectObject(hdcMem, bmMemOld));
DeleteObject(SelectObject(hdcSave, bmSaveOld));

// Delete the memory DCs.
DeleteDC(hdcMem);
DeleteDC(hdcBack);
DeleteDC(hdcObject);
DeleteDC(hdcSave);
DeleteDC(hdcTemp);
}
//*/
