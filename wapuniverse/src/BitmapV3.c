/***********************************************************************
 *
 Sample Code Disclaimer

 Copyright © 2002-2005 PalmSource, Inc. or its subsidiaries.  All
 rights reserved.

 You may incorporate this sample code (the "Code") into your applications
 for Palm OS(R) platform products and may use the Code to develop
 such applications without restriction.  The Code is provided to you on
 an "AS IS" basis and the responsibility for its operation is 100% yours.
 PALMSOURCE, INC. AND ITS SUBSIDIARIES (COLLECTIVELY, "PALM") DISCLAIM
 ALL WARRANTIES, TERMS AND CONDITIONS WITH RESPECT TO THE CODE, EXPRESS,
 IMPLIED, STATUTORY OR OTHERWISE, INCLUDING WARRANTIES, TERMS OR
 CONDITIONS OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 NONINFRINGEMENT AND SATISFACTORY QUALITY.  You are not permitted to
 redistribute the Code on a stand-alone basis.  TO THE FULL EXTENT ALLOWED BY LAW, 
 PALMSOURCE ALSO EXCLUDES ANY  LIABILITY, WHETHER BASED IN CONTRACT OR TORT 
 (INCLUDING NEGLIGENCE), FOR  INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL OR 
 PUNITIVE DAMAGES OF ANY  KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF 
 BUSINESS, LOSS OF INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING OUT 
 OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CODE.  The Code is 
 subject to Restricted Rights for U.S. government users and export regulations.

  
 SAMPLE NAME:		Bitmap Resource

 FILE:			BitmapRsrc.c

 DATE:			1/31/2005

 DESCRIPTION:		Source Code 
 
 Requires Palm OS 5 SDK (DR12) or later
 *
 *
 *****************************************************************************/
/********************************************************************************
 * Universe Web Browser                                                         *
 * Copyright (c) 2007 OpenMobl Systems                                          *
 * Copyright (c) 2006-2007 Donald C. Kirker                                     *
 * Portions Copyright (c) 1999-2007 Filip Onkelinx                              *
 *                                                                              *
 * http://www.openmobl.com/                                                     *
 * dev-support@openmobl.com                                                     *
 *                                                                              *
 * This program is free software; you can redistribute it and/or                *
 * modify it under the terms of the GNU General Public License                  *
 * as published by the Free Software Foundation; either version 2               *
 * of the License, or (at your option) any later version.                       *
 *                                                                              *
 * This program is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
 * GNU General Public License for more details.                                 *
 *                                                                              *
 * You should have received a copy of the GNU General Public License            *
 * along with this program; if not, write to the Free Software                  *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 	*
 *                                                                              *
 ********************************************************************************/

#include <PalmOS.h>
#include "WAPUniverse.h"
#include "BitmapV3.h"


typedef struct BitmapV3FlagsType
{
	UInt16 	compressed:1;  				// Data format:  0=raw; 1=compressed
	UInt16 	hasColorTable:1;				// if true, color table stored before bits[]
	UInt16 	hasTransparency:1;			// true if transparency is used
	UInt16 	indirect:1;						// true if bits are stored indirectly
	UInt16 	forScreen:1;					// system use only
	UInt16	directColor:1;					// direct color bitmap
	UInt16	indirectColorTable:1;		// if true, color table pointer follows BitmapType structure
	UInt16	noDither:1;						// if true, blitter does not dither
	UInt16 	reserved:8;
} BitmapV3FlagsType;

// This data structure is the PalmOS 5 version 3 BitmapType.
typedef struct BitmapV3Type
{
	// BitmapType
	Int16  				width;
	Int16  				height;
	UInt16  			rowBytes;
	BitmapV3FlagsType			flags;			// see BitmapFlagsType
	UInt8				pixelSize;		// bits per pixel
	UInt8				version;		// data structure version 3
	
	// version 3 fields
	UInt8				size;			// size of this structure in bytes (0x16)
	UInt8				pixelFormat;		// format of the pixel data, see pixelFormatType
	UInt8				unused;
	UInt8				compressionType;	// see BitmapCompressionType
	UInt16				density;					// used by the blitter to scale bitmaps
	UInt32				transparentValue;	// the index or RGB value of the transparent color
	UInt32				nextBitmapOffset;	// byte offset to next bitmap in bitmap family

	// if (flags.hasColorTable)
	//		{
	//		if (flags.indirectColorTable)
	//			ColorTableType* colorTableP;	// pointer to color table
	//		else
	//	  		ColorTableType	colorTable;	// color table, could have 0 entries (2 bytes long)
	//		}
	//
	// if (flags.indirect)
	//	  	void*	  bitsP;			// pointer to actual bits
	// else
	//   	UInt8	  bits[];				// or actual bits
	//
} BitmapV3Type;
typedef BitmapV3Type* BitmapV3Ptr;


#define ErrIf_ ErrFatalDisplayIf

static inline Int32 CalcRowBytes(Int16 width, Int16 pixelSize)
{
	// The number of bits is the width in pixels * the number of
	// bits used per pixel.
	Int32 numberOfBits = (Int32)width * pixelSize;


	// rowBytes is the # of bytes but must be rounded UP to an even
	// number of bytes.  By adding 15 to the number of bits,
	// dividing by 16, and then 
	// multiplying by 2 we arrive at the correct answer.
	// Instead of the painful multiply and divide sequence,
	// we can shift right by 4 and then shift left by 1
	// to achieve the same result.
	Int16 rowBytes = (numberOfBits+15) >> 4 << 1;

	

	return rowBytes;
}

static inline UInt32 RomVersion()
{
	// Determin the ROM version.
	UInt32 romVersion;
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	return romVersion;
}

static inline Boolean HasHighDensityAPIs()
{
	// Determins if a device has the high-density APIs.  Note that
	// some devices might have the APIs but their screens could be
	// standard (low) density.  In this, high-density offscreens
	// can still be created although their use is somewhat limited.
	return SysGetTrapAddress(sysTrapHighDensityDispatch) != 
			SysGetTrapAddress(sysTrapSysUnimplemented);
}

static inline Int32 CalcColorTableSize(const ColorTableType * ct)
{
	// Equivalent to BmpColortableSize() except this code operates
	// directly on a colortable instead of on a colortable embedded in
	// a bitmap.
	if (ct == NULL) return 0;
	else return sizeof(ColorTableType) + sizeof(RGBColorType)*ct->numEntries;
}

BitmapTypeV3 *BmpCreateV3Bitmap(BitmapPtr bitmapP, Int16 density,
					const UInt8 * bits, const ColorTableType * ct)
{
	// If bits is NULL, the bitmap will be created with one
	// chunk of memory.  Otherwise, an indirect bitmap is created and
	// the bits is allocated by the caller and DELETED after the
	// bitmap is deleted (if the bits is part of a resource, then
	// you don't delete, but the resource must be locked while the
	// bitmap is in use).

	// On devices with double-density support, it is preferable that
	// bitmaps be created without color tables (pass NULL for ct).
	// On older devices, bitmaps used as a destination by
	// calling WinCreateBitmapWindow() should contain a color table
	// if the pixel size of the offscreen is different than the pixel size
	// of the screen.
	

	Int16		rowBytes;
	Int32 		pixelDataSize = 0, totalSize = 0, colorTableSize = 0;
	BitmapV3Type	*bmp = NULL;
	UInt8		*ptr;

	Int16 		width;
	Int16 		height;
	Int16 		pixelSize;
	PixelFormatType pixelFormat = pixelFormatIndexed;

	if (!bitmapP) {
		return NULL;
	}

	BmpGetDimensions(bitmapP, &width, &height, NULL);
	pixelSize = BmpGetBitDepth(bitmapP);

	ErrIf_(width<=0 || width>4096 || height<=0 || height>4096,
		"Improper bitmap dimensions");
	ErrIf_(pixelSize!=1 && pixelSize!=2 && pixelSize!=4
			&& pixelSize!=8 && pixelSize!=16,
			"Unsupported pixel size");
	ErrIf_(density!=kDensityLow && density!=kDensityDouble, "Unsupported density");
	ErrIf_(pixelFormat>pixelFormatIndexedLE,
			"Unsupported pixel format");

	// 16-bit bitmaps may include (surprisingly) a 256-entry colortable.
	// 1, 2, 4, and 8-bit bitmaps may include a colortable that
	// has many colors as the pixelSize can allow.
	ErrIf_(ct!=NULL && ((pixelSize==16&&ct->numEntries!=256)
			|| (pixelSize<=8 && ct->numEntries != 1<<pixelSize)),
		"Invalid colortable");

	// If little-pixel-endian is requested, make sure the OS supports it.
	// In theory, you could still use the BitmapRsrc code.  You just can't use
	// the native graphics of the Palm OS on he bitmap.
	ErrIf_((pixelFormat==pixelFormatIndexedLE || pixelFormat==pixelFormat565LE)
			&&  RomVersion() < sysMakeROMVersion(5,0,0,sysROMStageRelease,0)
			&& HasHighDensityAPIs() == false,
		"OS doesn't support little-pixel-endian btmaps");


	// See if the OS supports V3 bitmaps.  If the OS does not, you
	// could still use BitmapRsrc code to work with V3 bitmaps;  you just
	// can't use the native graphics of the Palm OS on V3 bitmaps.
	ErrIf_(HasHighDensityAPIs() == false, "OS doesn't support V3 bitmaps");


	// We'll be nice to the caller about the pixelFormat.
	// The caller can pass either of the big-pixel-endian values to
	// specify big-pixel-endian or either of the little-pixel-endian
	// values to specify little-pixel-endian.  We'll figure out
	// indexed or 565 here based on the pixelSize;
	if (pixelFormat==pixelFormatIndexed || pixelFormat==pixelFormat565)
	{
		pixelFormat = (pixelSize == 16 ? pixelFormat565 : pixelFormatIndexed);
	}
	else
	{
		if (pixelSize == 16) pixelFormat = pixelFormat565LE;
		else if (pixelSize == 8) pixelFormat = pixelFormatIndexed;  // not LE
		else pixelFormat = pixelFormatIndexedLE;
	}


	rowBytes = CalcRowBytes(width,pixelSize);
	if (bits == NULL) pixelDataSize = (Int32)rowBytes * height;
	else pixelDataSize = 4;
	totalSize = sizeof(BitmapV3Type) + pixelDataSize;


	// If we have a user color table, make room for it in the bitmap.
	if (ct != NULL)
	{
		colorTableSize = CalcColorTableSize(ct);
		totalSize += colorTableSize;
	}


	bmp = Malloc(totalSize);
	ErrNonFatalDisplayIf(bmp==NULL, "Out of memory");
	if (bmp == NULL) goto cleanup;

	// This clears out everything in the V3 structure.
	// The important thing is that
	// it sets all the flags to 0 (false), the nextDepthOffset to
	// 0 (this isn't a "bitmap family"), the transparentIndex to
	// 0 (transparent color is white), compressionType to 0
	// (no compression) and the reserved field to 0.
	MemSet(bmp, sizeof(BitmapV3Type), 0);


	bmp->width = width;
	bmp->height = height;
	bmp->rowBytes = rowBytes;
	bmp->pixelSize = pixelSize;
	bmp->version = 3;
	bmp->size = sizeof(BitmapV3Type);
	bmp->pixelFormat = pixelFormat;
	bmp->density = density;
	bmp->flags.hasTransparency = BmpGetTransparentValue(bitmapP, &bmp->transparentValue);
	
	// Point ptr right after the structure.  Then,
	// as we handle color table  we
	// can increment ptr beyond those pieces.  In the end,
	// ptr will point to where the pixels go and we can clear
	// that area of memory.
	ptr = (UInt8*)(bmp+1);
	
	if (ct)
	{
		// Place the color table into the bitmap following the
		// structure.
		MemMove(ptr, ct, colorTableSize);
		ptr += colorTableSize;
		bmp->flags.hasColorTable = 1;
	}
	
	if (pixelSize == 16)
	{
		bmp->flags.directColor = 1;  // might not need to set this for V3
	}

	if (bits == NULL)
	{
		// Now clear the bitmap's pixels to white.  Note that
		// white is 0xFFFF for 16-bit bitmaps but 0 for all
		// other bitmaps.
		MemSet(ptr, pixelDataSize, pixelSize==16?0xFF:0);
	}
	else
	{
		*(const UInt8**)ptr = bits;
		bmp->flags.indirect = true;
	}
		

cleanup:
	return (BitmapTypeV3 *)bmp;
}

Err BmpDeleteV3Bitmap(BitmapPtr bitmapP)
{
	if (bitmapP) {
		Free(bitmapP);
		return errNone;
	}

	return errNone; // return paramater error
}
