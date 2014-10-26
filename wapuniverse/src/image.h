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
#ifndef _IMAGE_H_
#define _IMAGE_H_
#include	<PalmOS.h>


//#define SMOOTH_IMG_RESIZE	// uncomment for smoother image resizing, really slow....

extern Err pnoJpeg_CloseLibrary(UInt16 refNum);
extern Err pnoJpeg_OpenLibrary(UInt16 *refNumP);
extern Err GIFLib_OpenLibrary(UInt16 *refNumP, UInt32 * clientContextP);
extern Err GIFLib_CloseLibrary(UInt16 refNum, UInt32 clientContext);

extern BitmapPtr BmpResizeBitmap(BitmapPtr srcBmp, UInt16 maxWidth, UInt16 maxHeight, 
		UInt16 *outWidth, UInt16 *outHeight, Boolean freeSrc);

//extern void drawNoImage (WMLDeckPtr wmldck, char *alt, UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g);
extern void ImageCreateEmptyImage(char *alt, UInt16 *x, UInt16 *y, UInt16 *imgWidth, 
		 UInt16 *imgHeight, Boolean draw, GlobalsType *g);
extern Boolean ImageCreateImage(char *src, Int16 *width, Int16 *height, Boolean border, Char *alt, GlobalsType *g);

// "Modules" for handling different formats
extern BitmapPtr ImageDecode_WBMP(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g);
extern BitmapPtr ImageDecode_Pbmp(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g);
extern BitmapPtr ImageDecode_Bmp(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g);
extern BitmapPtr ImageDecode_Gif(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g);
extern BitmapPtr ImageDecode_Png(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, unsigned char **channel, GlobalsType *g);
extern BitmapPtr ImageDecode_Jpeg(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g);
extern BitmapPtr ImageDecode_Plugin(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g);

extern ImagePtr addPageImage(Char *location, Char *alt, Int16 x1, Int16 x2, Int16 y1, Int16 y2, 
		  	 UInt16 width, UInt16 height, BitmapPtr bitmap, GlobalsType *g);
extern void freePageImages(GlobalsType *g);
extern Boolean checkPageImage(Int16 x, Int16 y, Boolean go, GlobalsType *g);
extern void imageAddMap(Char *mapName, GlobalsType *g);
extern Boolean imageCheckArea(Char *name, Int16 x, Int16 y, GlobalsType *g);

extern Boolean ImageIsDownloadableImages(GlobalsType *g);
extern void ImageLoadImages(GlobalsType *g);
extern void ImageLoadThreadStart(GlobalsType *g);
extern void ImageLoadThreadStop(SndStreamRef stream, GlobalsType *g);
extern void ImageLoadStop(GlobalsType *g);

extern void MapAddImageMap(Char *name, GlobalsType *g);
extern void MapAddArea(Char *href, Char *alt, Int16 x, Int16 y, Int16 w, Int16 h, GlobalsType *g);
extern void MapParseCoord(Char *coord, Int16 *x, Int16 *y, Int16 *w, Int16 *h);
extern void MapFreeAreas(AreaTagPtr area, GlobalsType *g);
extern void MapFreeMaps(GlobalsType *g);


#endif
