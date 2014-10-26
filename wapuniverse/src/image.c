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

#define ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS

#include	<PalmOS.h>
#include	<PalmCompatibility.h>
#include	<SoundMgr.h>	// for multi-threading
#include	"xhtml.h"
#include	"../res/WAPUniverse_res.h"
#include	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"PalmUtils.h"
#include 	"WAP.h"
#include 	"wsp.h"
#include	"http.h"
#include	"process.h"
#include	"pnoJpeg.h"
#include 	"dbOffline.h"
#include	"Bitmap.h"
#include	"GIFLib.h"
#include	"palmpng.h"
#include	"Cache.h"
#include	"Layout.h"
#include	"URL.h"
#include	"DOM.h"
#include	"Font.h"
#include	"Browser.h"
#include	"callbacks.h"
#include	"debug.h"
#include	"FileURI.h"


Err pnoJpeg_OpenLibrary(UInt16 *refNumP)
{
	Err error;
	Boolean loaded = false;
	
	/* first try to find the library */
	error = SysLibFind(pnoJpegName, refNumP);
	
	/* If not found, load the library instead */
	if (error == sysErrLibNotFound)
	{
		error = SysLibLoad(pnoJpegTypeID, pnoJpegCreatorID, refNumP);
		loaded = true;
	}
	
	if (error == errNone)
	{
		error = pnoJpegOpen(*refNumP);
		if (error != errNone)
		{
			if (loaded)
			{
				SysLibRemove(*refNumP);
			}

			*refNumP = sysInvalidRefNum;
		}
	}
	
	return error;
}

Err pnoJpeg_CloseLibrary(UInt16 refNum)
{
	Err error;
	
	if (refNum == sysInvalidRefNum)
	{
		return sysErrParamErr;
	}

	error = pnoJpegClose(refNum);

	if (error == errNone)
	{
		/* no users left, so unload library */
		SysLibRemove(refNum);
	} 
	else if (error == pnoJpegErrStillOpen)
	{
		/* don't unload library, but mask "still open" from caller  */
		error = errNone;
	}
	
	return error;
}

/*
 * FUNCTION: GIFLib_OpenLibrary
 *
 * DESCRIPTION:
 *
 * User-level call to open the library.  This inline function
 * handles the messy task of finding or loading the library
 * and calling its open function, including handling cleanup
 * if the library could not be opened.
 * 
 * PARAMETERS:
 *
 * refNumP
 *		Pointer to UInt16 variable that will hold the new
 *      library reference number for use in later calls
 *
 * clientContextP
 *		pointer to variable for returning client context.  The client context is
 *		used to maintain client-specific data for multiple client support.  The 
 *		value returned here will be used as a parameter for other library 
 *		functions which require a client context.  
 *
 * CALLED BY: System
 *
 * RETURNS:
 *		errNone
 *		memErrNotEnoughSpace
 *      sysErrLibNotFound
 *      sysErrNoFreeRAM
 *      sysErrNoFreeLibSlots
 *
 * SIDE EFFECTS:
 *		*clientContextP will be set to client context on success, or zero on
 *      error.
 */
 
Err GIFLib_OpenLibrary(UInt16 *refNumP, UInt32 * clientContextP)
{
	Err error;
	Boolean loaded = false;
	
	/* first try to find the library */
	error = SysLibFind(GIFLibName, refNumP);
	
	/* If not found, load the library instead */
	if (error == sysErrLibNotFound)
	{
		error = SysLibLoad(GIFLibTypeID, GIFLibCreatorID, refNumP);
		loaded = true;
	}
	
	if (error == errNone)
	{
		error = GIFLibOpen(*refNumP, clientContextP);
		if (error != errNone)
		{
			if (loaded)
			{
				SysLibRemove(*refNumP);
			}

			*refNumP = sysInvalidRefNum;
		}
	}
	
	return error;
}

/*
 * FUNCTION: GIFLib_CloseLibrary
 *
 * DESCRIPTION:	
 *
 * User-level call to closes the shared library.  This handles removal
 * of the library from system if there are no users remaining.
 *
 * PARAMETERS:
 *
 * refNum
 *		Library reference number obtained from GIFLib_OpenLibrary().
 *
 * clientContext
 *		client context (as returned by the open call)
 *
 * CALLED BY: Whoever wants to close the library
 *
 * RETURNS:
 *		errNone
 *		sysErrParamErr
 */

Err GIFLib_CloseLibrary(UInt16 refNum, UInt32 clientContext)
{
	Err error;
	
	if (refNum == sysInvalidRefNum)
	{
		return sysErrParamErr;
	}

	error = GIFLibClose(refNum, clientContext);

	if (error == errNone)
	{
		/* no users left, so unload library */
		SysLibRemove(refNum);
	} 
	else if (error == GIFLibErrStillOpen)
	{
		/* don't unload library, but mask "still open" from caller  */
		error = errNone;
	}
	
	return error;
}

static int myRound(float value)
{
 	int result = 0;
 
 	result = (int)(value + 0.5);
 
 	return(result);
}


#ifdef SMOOTH_IMG_RESIZE
BitmapPtr BmpResizeBitmap(BitmapPtr srcBmp, UInt16 maxWidth, UInt16 maxHeight, UInt16 *outWidth, UInt16 *outHeight, Boolean freeSrc) //const void *emulStateP, Call68KFuncType *call68KFuncP, void *userData68KP)
{
 	BitmapPtr 	newBmp = NULL;
 	//BitmapPtr 	srcBmp = NULL;
 	UInt16 		newWidth, 	newHeight; 
 	UInt16		imageWidth, imageHeight;
	Coord		getWidth, getHeight;
 	Err 		err = errNone;
 	Int32		outError = 0;
 	float		xStep, yStep;
 	UInt16		*bitsNeu, *bitsAlt;
 	UInt32		x,y,pos, posOriginal; 
 	UInt32		r, g, b;
 	UInt16 		pixel = 0;
 	UInt16 		memberCount;
	UInt16		rowBytes;
 	int		xMaster, yMaster;
 	unsigned int	X, Y;

	if (srcBmp == NULL) {
		return srcBmp;
	}
 
	newWidth = maxWidth;
	newHeight = maxHeight;
	//newWidth = ReadUnaligned32(&pData->maxWidth);
 	//newHeight = ReadUnaligned32(&pData->maxHeight); 
 	//imageWidth = ReadUnaligned32(&pData->imageWidth);
 	//imageHeight = ReadUnaligned32(&pData->imageHeight);
 	//srcBmp = (BitmapPtr)ReadUnaligned32(&pData->outputBmp);

	BmpGetDimensions(srcBmp, &getWidth, &getHeight, &rowBytes);
 
	imageWidth = (UInt16)getWidth;
	imageHeight = (UInt16)getHeight;


	if (maxWidth) {
 		newWidth = maxWidth;
 		newHeight = imageHeight * maxWidth / imageWidth;
	}

	if (maxHeight) {
 		newWidth = imageWidth * maxHeight / imageHeight;
 		newHeight = maxHeight;
	}

 	//newBmp = NativeBmpCreate(call68KFuncP, emulStateP, userData68KP, (Coord)newWidth, (Coord)newHeight, 16, NULL, &err);
	newBmp = BmpCreate((Coord)newWidth, (Coord)newHeight, 16, NULL, &err);

 	if(err != errNone) {
  		outError = err;
  		//WriteUnaligned32( &pData->error, outError);
  		return srcBmp;
 	}
 
 	bitsNeu = (UInt16 *)BmpGetBits(newBmp); // NativeBmpGetBits(emulStateP, call68KFuncP, newBmp);
 	bitsAlt = (UInt16 *)BmpGetBits(srcBmp); // NativeBmpGetBits(emulStateP, call68KFuncP, srcBmp);

 	xStep = (float)(imageWidth) / (float)(newWidth);
 	yStep = (float)(imageHeight) / (float)(newHeight);


	for(y = 0; y < newHeight; y++) {
  		for(x = 0; x < newWidth; x++) {
   			r = 0; g = 0; b = 0;
   			memberCount = 0;

   			pos = (y * newWidth) + x;

   			for(yMaster = myRound(y*yStep); yMaster <= myRound((y+1) * yStep); yMaster++)
   			//for(yMaster = myRound(y*yStep); yMaster <  ( myRound((y+1)*yStep) > myRound(y*yStep) ? myRound((y+1) * yStep) : myRound((y+1) * yStep) + 1 ); yMaster++)
   			{

    				for(xMaster = myRound(x*xStep); xMaster <= myRound((x+1) * xStep); xMaster++)
				//for(xMaster = myRound(x*xStep); xMaster < ( myRound((x+1)*xStep) > myRound(x*xStep) ? myRound((x+1) * xStep) : myRound((x+1) * xStep) + 1  ); xMaster++)
    				{
     					X = xMaster;
	 				Y = yMaster;

     					if(Y  >= imageHeight) Y = imageHeight-1;
     					if(X  >= imageWidth) X = imageWidth-1;

    					memberCount++;
     					posOriginal = ( ( Y) * imageWidth) + (X);
     					pixel = bitsAlt[posOriginal]; // ByteSwap16(bitsAlt[posOriginal]);

     					r += (0x1F &(pixel >> 11));
     					g += (0x3F &(pixel >> 5));
     					b += (0x1F &(pixel >> 0));

    				}
   			}
   
   			if(memberCount) {
   				r /= memberCount;
   				g /= memberCount;
   				b /= memberCount;

   			}

   			pixel = r << 11 | g << 5 | b;

   			bitsNeu[pos] = pixel; //ByteSwap16(pixel);

  		}
 	}
 

 
 	//outError = errNone;
 	//WriteUnaligned32( &pData->error, outError);
 	//WriteUnaligned32( &pData->outputBmp, newBmp);

	BmpGetDimensions(newBmp, &getWidth, &getHeight, &rowBytes);
	*outWidth = (UInt16)getWidth;
	*outHeight = (UInt16)getHeight;

	if (freeSrc) {
		BmpDelete(srcBmp);
		srcBmp = NULL;
	}

	return newBmp;
}
#else
BitmapPtr BmpResizeBitmap(BitmapPtr srcBmp, UInt16 maxWidth, UInt16 maxHeight, UInt16 *outWidth, UInt16 *outHeight, Boolean freeSrc) // void jpegResize(const void *emulStateP, Call68KFuncType *call68KFuncP, void *userData68KP)
{
	//dataExchangePtr pData = userData68KP;
 	BitmapPtr 	newBmp = NULL;
 	//BitmapPtr 	srcBmp = NULL;
 	UInt32 		newWidth, newHeight; 
 	UInt32		imageWidth, imageHeight;
	Coord		getWidth, getHeight;
	UInt16		rowBytes;
 	Err 		err = errNone;
 	Int32		outError = 0;
 	float		xStep, yStep;
 	UInt16		*bitsNeu, *bitsAlt;
 	UInt32		x,y,pos, posOrig; 

 
 
 	//newWidth = ReadUnaligned32(&pData->maxWidth);
 	//newHeight = ReadUnaligned32(&pData->maxHeight); 
 	//imageWidth = ReadUnaligned32(&pData->imageWidth);
 	//imageHeight = ReadUnaligned32(&pData->imageHeight);
 	//srcBmp = (BitmapPtr)ReadUnaligned32(&pData->outputBmp);

	if (srcBmp == NULL) {
		return srcBmp;
	}
 
	newWidth = maxWidth;
	newHeight = maxHeight;

	BmpGetDimensions(srcBmp, &getWidth, &getHeight, &rowBytes);
 
	imageWidth = (UInt16)getWidth;
	imageHeight = (UInt16)getHeight;


	if (maxWidth) {
 		newWidth = maxWidth;
 		newHeight = imageHeight * maxWidth / imageWidth;
	}

	if (maxHeight) {
 		newWidth = imageWidth * maxHeight / imageHeight;
 		newHeight = maxHeight;
	}
 
	//newBmp = NativeBmpCreate(call68KFuncP, emulStateP, userData68KP, (Coord)newWidth, (Coord)newHeight, 16, NULL, &err);
	newBmp = BmpCreate((Coord)newWidth, (Coord)newHeight, 16, NULL, &err);

 	if(err != errNone) {
  		outError = err;
  		//WriteUnaligned32( &pData->error, outError);
  		return srcBmp;
 	}
 
 	xStep = (float)(imageWidth) / (float)(newWidth);
 	yStep = (float)(imageHeight) / (float)(newHeight);
 
 	bitsNeu = (UInt16 *)BmpGetBits(newBmp); // NativeBmpGetBits(emulStateP, call68KFuncP, newBmp);
 	bitsAlt = (UInt16 *)BmpGetBits(srcBmp); // NativeBmpGetBits(emulStateP, call68KFuncP, srcBmp);
 
 	if(bitsNeu == NULL || bitsAlt == NULL) {
  		outError = -4;
  		//WriteUnaligned32(&pData->error, outError);
		return srcBmp;
 	}
 
 	for(y = 0; y < newHeight; y++) {
  		for(x = 0; x < newWidth; x++) {
   			pos = ( y * newWidth) + x;
   			posOrig = ( myRound(y * yStep) * imageWidth) + myRound(x * xStep);
   
   			bitsNeu[pos] = bitsAlt[posOrig];
  		}
 	}
 
 	//outError = errNone;
 	//WriteUnaligned32( &pData->error, outError);
 	//WriteUnaligned32( &pData->outputBmp, newBmp);

	BmpGetDimensions(newBmp, &getWidth, &getHeight, &rowBytes);
	*outWidth = (UInt16)getWidth;
	*outHeight = (UInt16)getHeight;

	if (freeSrc) {
		BmpDelete(srcBmp);
		srcBmp = NULL;
	}

	return newBmp;
}
#endif

Int32 ReadIntMB(UInt8 *buf, UInt16 *offsetP)
{
    Int32 v = 0;
    UInt8 c;
    
    do {
        c = buf[(*offsetP)++];
        v = (v << 7) | (c & 0x7F);
    }
    while((c & 0x80) != 0);

    return v;
}

// TODO: compact these functions


Boolean checkPageImage(Int16 x, Int16 y, Boolean go, GlobalsType *g)
{
	ImagePtr tmp;
	  
  	tmp = g->pageImages;
	while(tmp != NULL)
	{
		if ((y>tmp->y1)&&(y<tmp->y2)&&
				(x>tmp->x1)&&(x<tmp->x2))
		{
			if (go && tmp->location) {
				switch (FrmCustomAlert(alImageDownload, tmp->location, NULL, NULL)) {
      	  				case 0:
						if (tmp->location)
							followLink(tmp->location, g);
      	  					break;

					case 1:      	  	
      	  				default:
      	  					break;
      	  			}
			}
			return(true);			
		}
		tmp = tmp->next;
	}
	return(false);
}

ImagePtr addPageImage(Char *location, Char *alt, Int16 x1, Int16 x2, Int16 y1, Int16 y2, 
		  UInt16 width, UInt16 height, BitmapPtr bitmap, GlobalsType *g)
{
	ImagePtr		tmp, idx;

	tmp = Malloc(sizeof(ImageType));
	if (!tmp) return NULL;

	tmp->x1 = x1;
	tmp->x2 = x2;
	tmp->y1 = y1;
	tmp->y2 = y2;
	tmp->next = NULL;
	tmp->bitmap = bitmap;
	tmp->alt = StrDup(alt);
	tmp->width = width;
	tmp->height = height;
	tmp->link = g->currentLink;
	tmp->alreadyHires = false;
	tmp->location = StrDup(location);
	tmp->mapName = NULL;
	tmp->channel = NULL;
	tmp->download = false;
    tmp->virtual = false;
    
    if (tmp->bitmap) {
            UInt32 bmpSize      = 0;
            UInt32 headerSize   = 0;
            
            BmpGetSizes(tmp->bitmap, &bmpSize, &headerSize);

            g->pageStats.totalMemoryUsed += (bmpSize + headerSize);
    }

	debugOut("addPageImage", tmp->location, 0, __FILE__, __LINE__, g);

	//tmp->location = Malloc(StrLen(location)+1);
	//if (tmp->location)
	//	StrCopy(tmp->location,location);
	
	if ( g->pageImages == NULL) {
		g->pageImages = tmp;
	} else {
		idx = g->pageImages;
		while (idx->next != NULL) {
		   	idx = idx->next;
		}
		idx->next = tmp;
	}

	DOMAddElement(tmp, DOM_UI_IMAGE, x1, y1, width, height, g);

	return tmp;
}

void imageAddMap(Char *mapName, GlobalsType *g)
{
	ImagePtr	tmp;
	Int16		hash = (mapName[0] == '#')?1:0;

	tmp = g->pageImages;

	while (tmp->next) {
		tmp = tmp->next;
	}

	tmp->mapName = StrDup(mapName+hash);
}

Boolean imageCheckArea(Char *name, Int16 x, Int16 y, GlobalsType *g)
{
	MapPtr	map;
	Boolean	ret = false;

	map = g->ImageMaps;

	while (map) {
		if (!StrNCompare(name, map->name, StrLen(name))) {
			AreaTagPtr	area = map->area;
			RectangleType	rect;
			ImagePtr	img = g->pageImages;

			while (img) {
				if (!StrNCompare(img->mapName, name, StrLen(name)))
					break;
				img = img->next;
			}

			while (area) {
				rect.topLeft.x = (area->area.topLeft.x * img->maxWidth / img->origWidth) + img->x1;
				rect.topLeft.y = (area->area.topLeft.y * img->maxWidth / img->origWidth) + img->y1;
				rect.extent.x = (area->area.extent.x * img->maxWidth / img->origWidth);
				rect.extent.y = (area->area.extent.y * img->maxWidth / img->origWidth);

				if (RctPtInRectangle(x,y,&rect)) {
					if (area->href)
						followLink(area->href, g);
					ret = true;
					break;
				}
				area = area->next;
			}
			break;
		}
		map = map->next;
	}

	return ret;
}

void freePageImages(GlobalsType *g)
{
	ImagePtr	tmp;

	while (g->pageImages != NULL){
		tmp = g->pageImages;
		g->pageImages = tmp->next;
		if(tmp->location){
			Free(tmp->location);
			tmp->location = NULL;
		}
		if(tmp->alt){
			Free(tmp->alt);
			tmp->alt = NULL;
		}
		if (tmp->bitmap && !tmp->virtual) {
            UInt32 bmpSize      = 0;
            UInt32 headerSize   = 0;
            
            BmpGetSizes(tmp->bitmap, &bmpSize, &headerSize);
            
			if (BmpDelete(tmp->bitmap) == errNone) {
                g->pageStats.totalMemoryUsed -= (bmpSize + headerSize);
            }
		}
		tmp->bitmap = NULL;
		tmp->virtual = false;
		if (tmp->mapName) {
			Free(tmp->mapName);
			tmp->mapName = NULL;
		}
		if (tmp->channel) {
			Free(tmp->channel);
			tmp->channel = NULL;
		}
		tmp->x1 = 0;
		tmp->x2 = 0;
		tmp->y1 = 0;
		tmp->y2 = 0;
		tmp->width = 0;
		tmp->height = 0;
		tmp->alreadyHires = false;
		Free(tmp);
	}
	g->pageImages = NULL;

	MapFreeMaps(g);
}

void MapAddImageMap(Char *name, GlobalsType *g)
{
	MapPtr	tmp;

	tmp = Malloc(sizeof(MapType));
	if (!tmp)
		return;

	MemSet(tmp, sizeof(MapType), 0);

	tmp->name = StrDup(name);
	tmp->area = NULL;
	tmp->next = g->ImageMaps;
	g->ImageMaps = tmp;
}

void MapAddArea(Char *href, Char *alt, Int16 x, Int16 y, Int16 w, Int16 h, GlobalsType *g)
{
	MapPtr		map;
	AreaTagPtr	area;

	map = g->ImageMaps;
	
	area = Malloc(sizeof(AreaTagType));
	if (!area)
		return;
	MemSet(area, sizeof(AreaTagType), 0);

	area->href = StrDup(href);
	area->alt = StrDup(alt);
	area->area.topLeft.x = x;
	area->area.topLeft.y = y;
	area->area.extent.x = w;
	area->area.extent.y = h;
	area->next = map->area;
	map->area = area;
}

// Takes coordinates in the format: "x1,y1,x2,y2"
void MapParseCoord(Char *coord, Int16 *x, Int16 *y, Int16 *w, Int16 *h)
{
	Char		coordtemp[30];
	Int16		i = 0, j = 0;
	Int16		len = StrLen(coord);
	Int16		x1 = 0, x2 = 0, y1 = 0, y2 = 0;

//TODO: Scale coordinates for scaled images

				
	j = 0;
	while ((coord[i] != ',') && (i < len)) {
		coordtemp[j] = coord[i];
		j++; i++;
	}
	coordtemp[j] = 0;
	x1 = StrAToI(coordtemp);

	i++; // get past the ","
	while ((coord[i] == ' ') && (i < len))
		i++;
				
	j = 0;
	while ((coord[i] != ',') && (i < len)) {
		coordtemp[j] = coord[i];
		j++; i++;
	}
	coordtemp[j] = 0;
	y1 = StrAToI(coordtemp);

	i++; // get past the ","
	while ((coord[i] == ' ') && (i < len))
		i++;
				
	j = 0;
	while ((coord[i] != ',') && (i < len)) {
		coordtemp[j] = coord[i];
		j++; i++;
	}
	coordtemp[j] = 0;
	x2 = StrAToI(coordtemp);

	i++; // get past the ","
	while ((coord[i] == ' ') && (i < len))
		i++;
				
	j = 0;
	while ((coord[i] != ',') && (i < len)) {
		coordtemp[j] = coord[i];
		j++; i++;
	}
	coordtemp[j] = 0;
	y2 = StrAToI(coordtemp);

	*x = x1;
	*y = y1;
	*w = (x2 - x1);
	*h = (y2 - y1);
}

void MapFreeAreas(AreaTagPtr area, GlobalsType *g)
{
	AreaTagPtr	tmp;

	while (area) {
		tmp = area;
		area = tmp->next;

		if (tmp->href) {
			Free(tmp->href);
			tmp->href = NULL;
		}
		if (tmp->alt) {
			Free(tmp->alt);
			tmp->alt = 0;
		}
		tmp->area.topLeft.x = 0;
		tmp->area.topLeft.y = 0;
		tmp->area.extent.x = 0;
		tmp->area.extent.y = 0;

		Free(tmp);
	}

	area = NULL;
}

void MapFreeMaps(GlobalsType *g)
{
	MapPtr	tmp;

	while (g->ImageMaps){
		tmp = g->ImageMaps;
		g->ImageMaps = tmp->next;
		
		if (tmp->name) {
			Free(tmp->name);
			tmp->name = NULL;
		}

		if (tmp->area) {
			MapFreeAreas(tmp->area, g);
			tmp->area = NULL;
		}

		Free(tmp);
	}
	g->pageImages = NULL;
}


BitmapPtr ImageDecode_WBMP(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g)
{
 	Int16		xpos,ypos;
 	UInt16 		w, h;
 	UInt16 		rowBytes,wbmpRowBytes;
 	BitmapType	*bmp;
 	unsigned char  	*tmpPtr= NULL;
 	UInt16		offset = 0;

	offset += 2;
 
	w = ReadIntMB(webData->data, &offset);
	h = ReadIntMB(webData->data, &offset);

	rowBytes = ((w + 15) >> 3) & ~1; // palm bmp even word boundary
        wbmpRowBytes = ((w + 7) >> 3);       // wbmp is byte boundary

	bmp = Malloc(sizeof(BitmapTypeV0) + (rowBytes * h));
	if (!bmp)
		goto error_report;
	MemSet(bmp, sizeof(BitmapTypeV0) + (rowBytes * h), 0);

        bmp->width     = w;
        bmp->height    = h;
        bmp->rowBytes  = rowBytes;

	tmpPtr = ((unsigned char*)bmp) + sizeof(BitmapTypeV0);

	for (ypos=0;ypos<h;ypos++){
		for (xpos=0;xpos<wbmpRowBytes;xpos++){
			*(tmpPtr + xpos + (ypos * rowBytes)) = 
				(webData->data[offset + xpos + (ypos * wbmpRowBytes)]) ^ 0xFF ; // wbmp is inversed 
		}
	}

	*height = h;
	*width = w;

error_report:
	return bmp; //just return for now...
}

BitmapPtr ImageDecode_Pbmp(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g)
{
	BitmapType    		*b = NULL;
	UInt16			w, h;


	b = (BitmapPtr)Malloc(webData->length);
	if (!b)
		return NULL;

	MemMove(b, webData->data, webData->length);

        w = b->width * HiresFactor();
        h = b->height * HiresFactor();

	*height = h;
	*width = w;

	return b;
}

BitmapPtr ImageDecode_Bmp(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g)
{
    BitmapType    		*b = NULL;
    UInt32			w, h;
    Err				err;


	err = DecodeBmp(webData->data, webData->length, &w, &h, (void **)&b);
	if (err) return NULL;

	*height = h;
	*width = w;

	return b;
}


BitmapPtr ImageDecode_Gif(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g)
{
	UInt16			w = 0, h = 0;
	BitmapPtr		bmp = NULL;
	Err			err = errNone;

	if (!gGifRef || (gGifRef == sysInvalidRefNum))
		return NULL;

	err = GIFLibDecompressImage(gGifRef, webData->data, webData->length, &w, &h, &bmp);
	if (err) {
		Char	temp[50];
		StrPrintF(temp, "%ld  %ld", err, gGifRef);
		MyErrorFunc(temp, "gif error");
	}

	*height = h;
	*width = w;

	return bmp;
}

BitmapPtr ImageDecode_Png(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, unsigned char **channel, GlobalsType *g)
{
	UInt16			w = 0, h = 0;
	BitmapPtr		bmp = NULL;

	if (!g->pngLibrary)
		return NULL;

	// decode
	bmp = (BitmapType *)DecodePngToBitmap(g->pngLibrary, webData->data, kDensityLow, channel);
	if (!bmp) {
		if (channel)
			Free(channel);
		return NULL;
	}

	BmpGetDimensions(bmp, &w, &h, NULL);

	*height = h;
	*width = w;

	return bmp;

}


BitmapPtr ImageDecode_Jpeg(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g)
{
    UInt16			w, h;
    BitmapPtr			bmp = NULL;
    pnoJpeg2Ptr    		jpegData = NULL;
    Err				error;

	if (jpegLibRef && jpegLibRef != sysInvalidRefNum) { //If we have the JpegLib open draw an image

		error = pnoJpeg2Create(jpegLibRef, &jpegData);
		if (error)
			return NULL;
		error = pnoJpeg2LoadFromPtr(jpegLibRef, jpegData, webData->data, MemPtrSize(webData->data));
		if (error)
			return NULL;

		error = pnoJpeg2GetInfo(jpegLibRef, jpegData, &w, &h);
		if (error)
			return NULL;

		/*maxw = browserGetWidgetBounds(DRAW_WIDTH);

		imgMaxWidth = 1;
		imgOrigWidth = 1;
		if ((w > browserGetWidgetBounds(DRAW_WIDTH)) && (g_prefs.renderMode == RENDER_WIDE_SCREEN)) {
			g->DrawWidth = w;
			BrowserRefreshScrollBarHorizontal(g);
		} else if (w > maxw && g_prefs.imgResize == true) {

			imgMaxWidth = maxw;
			imgOrigWidth = w;
			error = pnoJpeg2SetMaxDimensions(jpegLibRef, jpegData, maxw, NULL);
			if (error)
				return NULL;

		}*/

		error = pnoJpeg2Read(jpegLibRef, jpegData, &bmp);
		if (error)
			return NULL;


		BmpGetDimensions(bmp, &w, &h, NULL);

		//error = pnoJpeg2GetInfo(jpegLibRef, jpegData, &w, &h);
		//if (error)
		//	return false;

		error = pnoJpeg2Free(jpegLibRef, &jpegData);
		if (error)
			return NULL;

		if (error == errNone)
		{
			*height = h;
			*width = w;
		}
	}

	return bmp;
}

// Use Helper Plugins
BitmapPtr ImageDecode_Plugin(WebDataType *webData, UInt16 *width, UInt16 *height, Boolean isLink, GlobalsType *g)
{
    return NULL;
}


void ImageCreateEmptyImage(char *alt, UInt16 *x, UInt16 *y, UInt16 *imgWidth, 
		 UInt16 *imgHeight, Boolean draw, GlobalsType *g)
{
	BitmapPtr 		resP = NULL;
	RGBColorType       	color;  
	IndexedColorType   	indexedColor;
	Int16			width = 0;
	WinDrawOperation 	oldMode;
	RectangleType		rect;
	FontID			fntID;
	UInt16			res = narrowFontID, id = narrowFont;
	UInt16			x1 = *x, y1 = *y;

	
	WinPushDrawState();
	oldMode = WinSetDrawMode(winOverlay);

	if (g_prefs.fontSize != 2) {
		if (g_prefs.fontSize == 1) {		// small
			id = smallSmallFont;
			res = smallSmallFontID;
		} else {				// normal
			id = narrowFont;
			res = narrowFontID;
		}

		fntID = FntSetFont(id);
	} else {
		fntID = FntSetFont(stdFont);
	}

	if (!*imgWidth || !*imgHeight) {
		if (alt) {
			width += FontCharsWidth (alt, StrLen(alt));
		}

		width += (11 * HiresFactor());


		if (((*x + width) > browserGetWidgetBounds(DRAW_WIDTH)) && !draw) {
			palmprintln(g);
		
			*x = g->x;
			*y = g->y;
			x1 = *x;
			y1 = *y;
		}

		if (alt && FontCharsWidth(alt,StrLen(alt)) > browserGetWidgetBounds(DRAW_WIDTH))
			width = browserGetWidgetBounds(DRAW_WIDTH);	

		if (draw) {
			color.r   = ( UInt8 ) 192;
    			color.g   = ( UInt8 ) 192;
    			color.b   = ( UInt8 ) 192;
    			indexedColor  = WinRGBToIndex( &color );
    			WinSetTextColor( indexedColor );
    			WinSetForeColor( indexedColor );
		}

		rect.topLeft.x = *x + (1 * HiresFactor());
		rect.topLeft.y = *y + (1 * HiresFactor());
		rect.extent.x = width;
		rect.extent.y = (11 * HiresFactor());

		
		if (draw) {
			if (isHires()) {
				WinPushDrawState();
				WinSetCoordinateSystem(kCoordinatesNative);

				WinDrawLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x + rect.extent.x, rect.topLeft.y);
				WinDrawLine(rect.topLeft.x + rect.extent.x, rect.topLeft.y, rect.topLeft.x + rect.extent.x, rect.topLeft.y + rect.extent.y);
				WinDrawLine(rect.topLeft.x, rect.topLeft.y + rect.extent.y, rect.topLeft.x + rect.extent.x, rect.topLeft.y + rect.extent.y);
				WinDrawLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x, rect.topLeft.y + rect.extent.y);

				WinPopDrawState();
			} else {
				WinDrawRectangleFrame(simpleFrame, &rect); 
			} 
		} 
		*x += (2 * HiresFactor());

		if (draw) {	    
			resP = MemHandleLock(DmGetResource(bitmapRsc, bmpNoImage));
			if (resP) {
				WinDrawBitmap (resP, *x, *y+(2 * HiresFactor()));
				MemPtrUnlock(resP);
			}
		}

		*x += (9 * HiresFactor());

		if (alt){ // TODO: FIX ME!!

    			if (draw)
				WinPaintChars(alt,StrLen(alt),*x,*y+(2 * HiresFactor()));
			*x += FontCharsWidth(alt, StrLen(alt)) + (3 * HiresFactor());
		} else {
			*x += (3 * HiresFactor());
		}

		WinSetDrawMode(oldMode);
		WinPopDrawState();

		FntSetFont(fntID);

		if (!draw) {
			//if (13 > g->imgH)
			//	g->imgH = 13;
			browserResizeDimensions(0, 13 * HiresFactor(), g);
			if (g->x > g->DrawWidth)
				g->DrawWidth = g->x;
			addPageImage(NULL, alt, x1, *x, y1, (*y)?*y:(y1 + (13 * HiresFactor())), *x - x1, 13 * HiresFactor(), NULL, g);
			addNewIndent(*x, *y, 13 * HiresFactor(), g);
		}
	} else {
		if (draw) {
			color.r   = ( UInt8 ) 192;
    			color.g   = ( UInt8 ) 192;
    			color.b   = ( UInt8 ) 192;
    			indexedColor  = WinRGBToIndex( &color );
    			WinSetTextColor( indexedColor );
    			WinSetForeColor( indexedColor );

			WinSetDrawMode(winOverlay);

			rect.topLeft.x = *x;// + (1 * HiresFactor());
			rect.topLeft.y = *y;// + (1 * HiresFactor());
			rect.extent.x = *imgWidth; // bounds->extent.x;
			rect.extent.y = *imgHeight; // bounds->extent.y;


			if ((*imgHeight >= (11 * HiresFactor())) && (*imgWidth >= (11 * HiresFactor()))) {
				resP = MemHandleLock(DmGetResource(bitmapRsc, bmpNoImage));
				if (resP) {
					WinDrawBitmap (resP, *x + (2 * HiresFactor()), *y + (2 * HiresFactor()));
					MemPtrUnlock(resP);
				}
			}

			if (alt) {
				Int16		strWidth = *imgWidth - (12 * HiresFactor());
				Int16		strLen = StrLen(alt);
				Boolean		fitInWidth;

				if (strWidth > 0) {
					FntCharsInWidth(alt, &strWidth, &strLen, &fitInWidth);
					WinPaintChars(alt, strLen, *x + (12 * HiresFactor()), *y + (2 * HiresFactor()));
				}
			}

			if (isHires()) {
				WinPushDrawState();
				WinSetCoordinateSystem(kCoordinatesNative);

				WinDrawLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y);
				WinDrawLine(rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y + rect.extent.y - 1);
				WinDrawLine(rect.topLeft.x, rect.topLeft.y + rect.extent.y - 1, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y + rect.extent.y - 1);
				WinDrawLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x, rect.topLeft.y + rect.extent.y - 1);

				WinPopDrawState();
			} else {
				WinDrawRectangleFrame(simpleFrame, &rect); 
			}
		} else {
			browserResizeDimensions(*imgWidth, *imgHeight, g);
			//if (g->x > g->DrawWidth)
			//	g->DrawWidth = g->x;
			addPageImage(NULL, alt, *x, *x + *imgWidth, *y, *y + *imgHeight, *imgWidth, *imgHeight, NULL, g);
			addNewIndent(*x + *imgWidth, *y + *imgHeight, *imgHeight, g);

			*x += *imgWidth;
			*y += *imgHeight;
		}

		WinPopDrawState();

		FntSetFont(fntID);
	}

}

Boolean ImageIsDownloadableImages(GlobalsType *g)
{
	ImagePtr	img = g->pageImages;
	Boolean		downloadableExists = false;

	while (img) {
		if (img->download)
			downloadableExists = true;

		img = img->next;
	}

	return downloadableExists;
}

Boolean ImageDuplicateImageData(ImagePtr img, Char *url, BitmapPtr bmp, GlobalsType *g)
{
	ImagePtr	tmp = img;
	Boolean		ret = false;

	while (tmp) {
		if (!StrCompare(url, tmp->location) && !tmp->bitmap) {
			tmp->virtual = true;
			tmp->bitmap = bmp;

			ret = true;
		}
		tmp = tmp->next;
	}

	return ret;
}

Boolean ImageDownloadImage(WebDataType *webData, Char *url, unsigned char *imgBuf, GlobalsType *g)
{
 	UInt32 		len = 0;
 	Int16 		sock;
 	UInt32 		maxTimer;
	int		retransCount = 0;
	Err		error;
	URLTarget 	*http_url;
	int		wtpFail = 0;
	UInt16 		ctP, indexP;

	g->isImage = true;

	debugOut("ImageDownloadImage", "top", 0, __FILE__, __LINE__, g);

	if (StrNCaselessCompare(url, "file:", 5) == 0) {
		if (FileURIExists(url)) {
       			len = 1;
		} else {
			len = 0;
		}
		debugOut("ImageDownloadImage", "file", 0, __FILE__, __LINE__, g);
	} else if (StrNCaselessCompare(url, "mtch:", 5) == 0) {
		error = ChannelGetIndex(url, false, &indexP, &ctP, g);
		if (!error) {
       			len = 1;
		} else {
			len = 0;
		}
		debugOut("ImageDownloadImage", "Channel", (Int32)error, __FILE__, __LINE__, g);
	} else {
		if ((g->cache.cacheDoNotRead == false) &&
			(CacheFindCachedFile(url, true, g) == true)) {
			len = 1;
			debugOut("ImageDownloadImage", "read from cache", 0, __FILE__, __LINE__, g);
		} else if (g->conn.connectionType == 'h' || g->conn.connectionType == 'd') {
			if (HttpLibRef == 0) {
				len = 0;
				debugOut("ImageDownloadImage", "no http ref", 0, __FILE__, __LINE__, g);
			} else {
				http_url = ParseURL( url );
				if ( http_url != NULL ) {
					g->isImage = true;
                        		if ( HTTPGet( http_url, url, "Universe-HTTP-Cache", true, &g->httpParser ) != 0 ) {
						len = 0;
						debugOut("ImageDownloadImage", "htp get err", 0, __FILE__, __LINE__, g);
                        		} else {
						debugOut("ImageDownloadImage", "http get", 0, __FILE__, __LINE__, g);

						while (g->httpParser.state != PS_Done && g->httpParser.state != PS_Error) {
							EvtResetAutoOffTimer(); // Make sure that the device does not power off during a long download...
										// Eventually add a progress meter too.

							debugOut("img get loop", "proccess evt", 0, __FILE__, __LINE__, g);
							if ( ProcessEventLoopTimeout(0) ) {
								g->httpParser.state = PS_Error;
								// add stop stuff...
								FileClose(g->httpParser.fd);
								g->userCancel = true;
								debugOut("img get loop", "exit", 0, __FILE__, __LINE__, g);
								break;
							}

							debugOut("img get loop", "parse engine", 0, __FILE__, __LINE__, g);
							HTTPParseEngine(g->sock, &g->httpParser);
							debugOut("img get loop", "processes", 0, __FILE__, __LINE__, g);
						}

						debugOut("img get loop", "done", g->httpParser.state, __FILE__, __LINE__, g);

						if (g->httpParser.state == PS_Done)
							len = 1;
						else
							len = 0;
					}
					debugOut("ImgDownloadImage", "clean up", 0, __FILE__, __LINE__, g);
					HTTPCleanUp(&g->sock, &g->httpParser);
					g->sock = -1;
					debugOut("ImgDownloadImage", "destroy url", 0, __FILE__, __LINE__, g);
                        		DestroyURL( http_url );

					debugOut("ImgDownloadImage", "http and url cleaned", 0, __FILE__, __LINE__, g);
                    		}
			}
		} else {
			Err	wapErr = errNone;

			if ((g->wtpState == WTP_Disconnected) && (g->conn.connectionType == 'O')) {
wtp_top:

				g->sock = wapConnect(&(g->conn),url,false,wtpFail,g);
				if ((g->sock == -1) || (g->wtpState == WTP_Disconnected)) {
					if (g->sock != -1) // STUPID!!
						close(g->sock);
					g->sock = -1;
					g->wtpState = WTP_Disconnected;
					len = 0;

					goto wap_bottom;
				}

				wtpFail = WTPHandleConnectResp(g->sock, g->conn.ipaddress, g);
				if (wtpFail == -1) {
					if (g->sock != -1)
						close(g->sock);
					g->sock = -1;
					g->retransmission = false;
					g->wtpState = WTP_Disconnected;
					len = 0;
					goto wap_bottom;
				} else if (wtpFail == 0) {
					g->retransmission = true;
					goto wtp_top;
				} else {
					g->retransmission = false;
				}

				g->retransmission = false;
			}

    			sock = wapGetUrl(&(g->conn), url,false,&wapErr,g);
    			maxTimer = TimGetTicks() + (g_prefs.timeout * SysTicksPerSecond());
			if (wapErr) {
				len = 0;
				goto wap_bottom;
			}
	//TODO: Shouldn't sock be g->sock??
			do { //TODO: this is no good, just a quick hack to get it working: an active loop;; well, it seems to work -DCK, TODO: Need to allow user to cancel
				g->retransmission = false;
    				len = WspGetReply(sock,imgBuf,DEF_RECEIVE_BUF,maxTimer,g);

				if (len == 0) {
					if (g->conn.connectionType == 'O') {
						g->retransmission = true;
						wapErr = errNone;
						reWtpGet(sock,&(g->conn),url,false,&wapErr,g);
						if (wapErr) {
							len = 0;
							break;
						}
					}
				} else if (len == -5) {
					len = 0;
					if (g->conn.connectionType == 'O') {
						if (retransCount >= 8) break;
						retransCount++;
						g->retransmission = true;
						wapErr = errNone;
						reWtpGet(sock,&(g->conn),url,false,&wapErr,g);
						if (wapErr) {
							len = 0;
							break;
						}
					}
				} else if (len == -10) {
					break;
				}
			}
			while(!len);

			NetCloseSocket(&sock);
wap_bottom:
		}
	}

	g->retransmission = false;

	debugOut("ImgDownloadImage", "returning", len, __FILE__, __LINE__, g);

	return (len)?true:false;
}

Boolean ImageHandleImage(UInt32 len, BitmapPtr *bmp, UInt16 *width, UInt16 *height, unsigned char **channel, Char *url, unsigned char *imgBuf, Boolean isLink, WebDataType *webData, int *wspErr, GlobalsType *g)
{
	int		wspRet = 1;
	UInt16		indexP, ctP;
	Err		error;
	Boolean		ret = false;

	debugOut("ImageHandleImage", "top", 0, __FILE__, __LINE__, g);

	if (StrNCaselessCompare(url, "mtch:", 5) == 0) {
		error = ChannelGetIndex(url, false, &indexP, &ctP, g);

		wspRet = wsp_getOffline(indexP, webData, false, g);

		debugOut("ImageHandleImage", "Channel", wspRet, __FILE__, __LINE__, g);
	} else if (StrNCaselessCompare(url, "file:", 5) == 0) {
		wspRet = FileURIOpen(url, webData, false, g);

		debugOut("ImageHandleImage", "File", wspRet, __FILE__, __LINE__, g);
	} else if ((g->cache.cacheDoNotRead == false) &&
		(CacheLoadFileFromCache(webData, url, false, true, g) == true)) {
			wspRet = 0;
		debugOut("ImageHandleImage", "got cache", 0, __FILE__, __LINE__, g);
	} else if (g->conn.connectionType == 'h' || g->conn.connectionType == 'd') {
		wspRet = HTTPHandle(webData, g);
		debugOut("ImageHandleImage", "HTTP Data", wspRet, __FILE__, __LINE__, g);
	} else {
		wspRet = WspDecode(imgBuf,len, webData, g);

		debugOut("ImageHandleImage", "WAP", wspRet, __FILE__, __LINE__, g);
	}

	g->isImage = false;
	if ( wspRet == 0){
		if ((StrNCaselessCompare(g->contentType, "image/", 6)==0) &&
		    (StrNCaselessCompare(url, "mtch:", 5) != 0) &&
		    (StrNCaselessCompare(url, "file:", 5) != 0))
			CacheAddFileToCache(webData, url, true, g);
		  	
		if ((StrNCaselessCompare(g->contentType, "image/gif", 9)==0) ||
		    (StrNCaselessCompare(g->contentType, "image/x-gif", 11)==0)) {
	  		//handle GIF
			*bmp = ImageDecode_Gif(webData, width, height, isLink, g);
			//if (bmp)
			//	BmpCompress(bmp, BitmapCompressionTypePackBits);
		} else if ((StrNCaselessCompare(g->contentType, "image/jpeg", 10)==0) ||
			   (StrNCaselessCompare(g->contentType, "image/jpg", 9)==0)) {
	  		//handle JPEG
	  		*bmp = ImageDecode_Jpeg(webData, width, height, isLink, g);
			//if (bmp)
			//	BmpCompress(bmp, BitmapCompressionTypePackBits);
		} else if ((StrNCaselessCompare(g->contentType, "image/png", 9)==0) ||
			   (StrNCaselessCompare(g->contentType, "image/x-png", 11)==0)) {
			//handle PNG
			*bmp = ImageDecode_Png(webData, width, height, isLink, channel, g);
			//if (bmp)
			//	BmpCompress(bmp, BitmapCompressionTypePackBits);
		} else if (StrNCaselessCompare(g->contentType, "image/vnd.wap.wbmp", 18)==0) {
			//handle WBMP
			*bmp = ImageDecode_WBMP(webData, width, height, isLink, g);
		} else if (StrNCaselessCompare(g->contentType, "image/vnd.palm.pbmp", 19)==0) {	
			//handle Palm bitmap
			*bmp = ImageDecode_Pbmp(webData, width, height, isLink, g);
			//alreadyHires = true;
		} else if ((StrNCaselessCompare(g->contentType, "image/bmp", 9)==0) ||
			   (StrNCaselessCompare(g->contentType, "image/x-bmp", 11)==0) ||
			   (StrNCaselessCompare(g->contentType, "image/x-MS-bmp", 14)==0) ||
			   (StrNCaselessCompare(g->contentType, "image/x-ms-bmp", 14)==0)) {
		  		//handle Bmp
		  		*bmp = ImageDecode_Bmp(webData, width, height, isLink, g); 
		} else {
			*bmp = ImageDecode_Plugin(webData, width, height, isLink, g);
		}
		WspFreePdu(webData);

		if (*bmp)
			ret = true;
		else
			ret = false;

		/*if (bmp) {
			img = addPageImage(src, alt, g->x - imgWidth, g->x, g->y, g->y + imgHeight, imgWidth, imgHeight, bmp, g);
			if (alreadyHires)
				img->alreadyHires = true;
			img->maxWidth = imgMaxWidth;
			img->origWidth = imgOrigWidth;

			if (channel)
				img->channel = channel;

			updateScreen(g);
		}*/

		*wspErr = 0;
	} else {
		/*if (wspRet == WSP_ERR_AUTHORIZE) {
			if (imgBuf)
        			Free(imgBuf);
			if (url)
        			Free(url);

    			return ImageCreateImage(src, alt, g);
		} else if (wspRet == WSP_ERR_REDIRECT || wspRet == HTTP_ERR_REDIRECT) {
			Boolean tempRet = false;

			if (imgBuf)
        			Free(imgBuf);
			if (url)
        			Free(url);

			tempRet = ImageCreateImage(g->urlBuf, alt, g);

			Free(g->urlBuf);

			return tempRet;
		} else {
			ret = false;
		}*/

		*wspErr = wspRet;
	}

	return ret;
}

void ImageThreadDebugOut(char c)
{
#ifdef ENABLE_IMG_THREAD
	WinDrawChar(c,0,0);
	SysTaskDelay(2 * SysTicksPerSecond());
#endif
}

void ImageLoadImages(GlobalsType *g)
{
 	Char 		*url;
 	UInt32 		len = 0;
 	unsigned char 	*imgBuf= NULL;
 	WebDataType 		webData;
 	int 		wspRet = 0;
	Err		err;
	BitmapPtr	bmp = NULL;
	ImagePtr	img = NULL;
	unsigned char	*channel = NULL;
	Int16		imgMaxWidth = 1;
	Int16		imgOrigWidth = 1;
	UInt16		maxw = 0;
	UInt16		width = 0, height = 0, tmpWidth, tmpHeight;

load_top:
	img = g->downloadImage;

	debugOut("ImageLoadImages", "top", 0, __FILE__, __LINE__, g);

	if (img && (!img->download || img->bitmap)) {
		g->downloadImage = img->next;
		goto load_top;
	} else if (!img) {
		return;
	}

	debugOut("ImageLoadImages", "got img struct", 0, __FILE__, __LINE__, g);

	WspInitPdu(&webData);

	debugOut("ImageLoadImages", "pdu freeded, set url", 0, __FILE__, __LINE__, g);

	url = Malloc(MAX_URL_LEN+1);
	if (!url) {
		g->downloadImage = img->next;
		return;
	}
	debugOut("ImageLoadImages", "url alloc", 0, __FILE__, __LINE__, g);
	URLCreateURLStr(img->location,url,g);

	debugOut("ImageLoadImages", "url setup", 0, __FILE__, __LINE__, g);

	if ((g->engineStop == true || g->userCancel == true) && !CacheFindCachedFile(url, true, g)) {
		if (url)
			Free(url);
		g->downloadImage = img->next;
		return;
	}

	debugOut("ImageLoadImages", "setup buffer", 0, __FILE__, __LINE__, g);

   	imgBuf = Malloc(DEF_RECEIVE_BUF+1);       
	if (!imgBuf) { 
		if (url)
			Free(url);
		g->downloadImage = img->next;
		return; 
	} // ErrFatalDisplayIf (!imgBuf, "Malloc Failed");

	debugOut("ImageLoadImages", "downloading...", 0, __FILE__, __LINE__, g);

	len = ImageDownloadImage(&webData, url, imgBuf, g);

	debugOut("ImageLoadImages", "downloaded", len, __FILE__, __LINE__, g);

	if (len <= 0) {
		debugOut("ImageLoadImages", "download error", 0, __FILE__, __LINE__, g);
		img->download = false; // good idea?
		goto cleanup;
	}

	debugOut("ImageLoadImages", "handling", 0, __FILE__, __LINE__, g);

	ImageHandleImage(len, &bmp, &tmpWidth, &tmpHeight, &channel, url, imgBuf, isLink, &webData, &wspRet, g);

	width = img->width;
	height = img->height;

	if (!wspRet && bmp) {
		if (width > img->origWidth) {
			imgMaxWidth = img->maxWidth;
			imgOrigWidth = img->origWidth;

			maxw = imgMaxWidth;

			if (jpegLibRef && jpegLibRef != sysInvalidRefNum) {
				BitmapType	*newBmp;
				UInt32		maxh = height * maxw / width;

				if (BmpGetBitDepth(bmp) < 16)
					goto bottom;

				newBmp = pnoJpeg2Resample(jpegLibRef, bmp, maxw, maxh, &err);
				if (err != errNone) {
					if (bmp) {
						if (newBmp) BmpDelete(newBmp);
						goto bottom;
					} else {
						if (newBmp) BmpDelete(newBmp);
						if (channel) Free(channel);
						goto cleanup;
					}
				}

				if (bmp) BmpDelete(bmp);
				bmp = newBmp;

				width = maxw; height = maxh;
			} else {
				if (BmpGetBitDepth(bmp) < 16)
					goto bottom;
		
				bmp = BmpResizeBitmap(bmp, maxw, NULL, &width, &height, true);
			}
			if (!bmp) {
				if (channel) Free(channel);
				goto cleanup;
			}
		}
bottom:

		img->bitmap = bmp;
		img->channel = channel;
		img->download = false;
        
        if (img->bitmap) {
            UInt32 bmpSize      = 0;
            UInt32 headerSize   = 0;
            
            BmpGetSizes(img->bitmap, &bmpSize, &headerSize);

            g->pageStats.totalMemoryUsed += (bmpSize + headerSize);
            if (img->channel)
                g->pageStats.totalMemoryUsed += MemPtrSize(img->channel);
        }

		ImageDuplicateImageData(img->next, img->location, bmp, g);
	} else {
		if (bmp)
			BmpDelete(bmp);

		if (wspRet == WSP_ERR_AUTHORIZE) {
			if (imgBuf)
        			Free(imgBuf);
			if (url)
        			Free(url);

			//ImageLoadImages(g);

    			//return ImageCreateImage(src, width, height,alt, g);
			return;
		} else if ((wspRet == WSP_ERR_REDIRECT || wspRet == HTTP_ERR_REDIRECT) && g->urlBuf) {
			if (imgBuf)
        			Free(imgBuf);
			if (url)
        			Free(url);

			debugOut("ImageLoadImages, redirect, old loc:", img->location, 0, __FILE__, __LINE__, g);

			if (img->location)
				Free(img->location);
			img->location = StrDup(g->urlBuf);

			Free(g->urlBuf);
			g->urlBuf = NULL;

			debugOut("ImageLoadImages, redirect, new loc:", img->location, 0, __FILE__, __LINE__, g);

			//ImageLoadImages(g);

			/*tempRet = ImageCreateImage(g->urlBuf, width, height, alt, g);

			Free(g->urlBuf);

			return tempRet;*/
			return;
		} else {
			img->download = false;
		}
	}


cleanup:

	if (g->conn.connectionType == 'h' || g->conn.connectionType == 'd')
		HTTPFlushCache();

    	if (imgBuf)
        	Free(imgBuf);
	if (url)
        	Free(url);

	g->downloadImage = img->next;

    	return;
}

/*Boolean ImageLoadThreadHandle(SndStreamRef stream)
{
	GlobalsType	*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (g->downloadImage) {
		ImageLoadImages(g);
		updateScreen(g);
		
		return true;
	} else {
		ImageLoadThreadStop(stream, g);

		return false;
	}

	return false;
}*/

void ImageLoadUpdateScreen(GlobalsType *g)
{
	g->imageThreadLoadCount = 0;
	g->state = BS_UPDATE_IMG;
}

void ImageLoadThreadSyncronize(void)
{
	SysTaskDelay(SysTicksPerSecond());
}

Err ImageLoadThreadRun(void *userDataP, SndStreamRef stream, void *bufferP, UInt32 bufferSize)
{
	GlobalsType	*g; // = (GlobalsType *)userDataP;
	UInt16		width = 1; // for 8bit mono
	UInt32		sysA5;
	UInt32		threadA5;
	UInt16		autoOff = 0;

	autoOff = SysSetAutoOffTime(0);

	FtrSet(wuAppType, ftrThreadA5, SysSetA5(0));
	FtrGet(wuAppType, ftrSysA5, &sysA5);
	SysSetA5(sysA5);


	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	/*
		TODO: Check to see if we have a current action occuring, if so continue
		processing, then render if we get to end while that action is continuing.
	*/

	if (g->downloadImage && (g->state == BS_DOWNLOADING_IMG)) {
		ImageLoadImages(g);
		//updateScreen(g);
		g->imageThreadLoadCount++;
		if (g->imageThreadLoadCount == 5) // Process every 5 images
			ImageLoadUpdateScreen(g);
		MemSet(bufferP, bufferSize * width, 0);
	} else if (g->downloadImage) {
		MemSet(bufferP, bufferSize * width, 0);
	} else if (g->imageThreadLoadCount) { // We have finished all images, but some still need to be loaded
		ImageLoadUpdateScreen(g);
		MemSet(bufferP, bufferSize * width, 0);
	} else {
		MemSet(bufferP, bufferSize * width, 0);

		FtrGet(wuAppType, ftrThreadA5, &threadA5);
		SysSetA5(threadA5);
		FtrSet(wuAppType, ftrThreadA5, 0);

		EvtResetAutoOffTimer();
		SysSetAutoOffTime(autoOff);

		//ImageLoadThreadStop(stream, g);
		g->state = BS_DOWNLOAD_STOP;

		goto thread_run_bottom;
	}

	FtrGet(wuAppType, ftrThreadA5, &threadA5);
	SysSetA5(threadA5);
	FtrSet(wuAppType, ftrThreadA5, 0);

	EvtResetAutoOffTimer();
	SysSetAutoOffTime(autoOff);

thread_run_bottom:
	return errNone;
}

void ImageLoadThreadStart(GlobalsType *g)
{
	Err					error;
	SndStreamRef				stream = NULL;
	//SndStreamVariableBufferCallback		CallbackPtr = (SndStreamVariableBufferCallback)ImageLoadThreadRun;
	SndStreamBufferCallback			CallbackPtr = (SndStreamBufferCallback)ImageLoadThreadRun;
	UInt32					sysA5;

	FtrSet(wuAppType, ftrSysA5, SysSetA5(0));
	FtrGet(wuAppType, ftrSysA5, &sysA5);
	SysSetA5(sysA5);

	debugOut("ImageLoadThreadStart", "create", 0, __FILE__, __LINE__, g);

	//error = SndStreamCreateExtended(&stream, sndOutput, sndFormatPCM, 48000, sndInt8, sndMono, 
	//			CallbackPtr, g, 1024, false);
	error = SndStreamCreate(&stream, sndOutput, 48000, sndUInt8, sndMono, CallbackPtr, g, 100, false);
	if (error != errNone || !stream) {
		debugOut("ImageLoadThreadStart", "create err", (Int32)error, __FILE__, __LINE__, g);
		goto thread_start_cleanup;
	}

	debugOut("ImageLoadThreadStart", "created", 0, __FILE__, __LINE__, g);

	g->imageThreadRef = stream;

	SndStreamSetVolume(stream, 0);

	MemPtrSetOwner(g, 0);

	debugOut("ImageLoadThreadStart", "start", 0, __FILE__, __LINE__, g);

	error = SndStreamStart(stream);
	if (error != errNone) {
		debugOut("ImageLoadThreadStart", "start err", (Int32)error, __FILE__, __LINE__, g);
		goto thread_start_cleanup;
	}

	g->state = BS_DOWNLOADING_IMG;

	debugOut("ImageLoadThreadStart", "running", 0, __FILE__, __LINE__, g);

	return;
thread_start_cleanup:

	g->state = BS_ALT_DOWNLOAD_IMG;

	/*MyErrorFunc("Could not start thread. Images will not load.", NULL); // TODO: Load the other way

	if (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlc",24)) {
		g->state = BS_COMPLETE_WML;
        } else if (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29) || 
		   !StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21) || 
		   !StrNCaselessCompare(g->pageContentType,"application/wml+xml",19) ||
		   !StrNCaselessCompare(g->pageContentType,"text/html",9) ||
		   !StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)) {
			g->state = BS_COMPLETE_HTML;
        }*/
}

void ImageLoadStop(GlobalsType *g)
{
#ifdef ENABLE_IMG_THREAD
	if (g->imageThreadRef && (g->state == BS_DOWNLOADING_IMG)) {
		ImageLoadThreadStop(g->imageThreadRef, g);
	}
#endif
}

void ImageLoadThreadStop(SndStreamRef stream, GlobalsType *g)
{
	if (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlc",24)) {
		g->state = BS_COMPLETE_WML;
        } else if (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29) || 
		   !StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21) || 
		   !StrNCaselessCompare(g->pageContentType,"application/wml+xml",19) ||
		   !StrNCaselessCompare(g->pageContentType,"text/html",9) ||
		   !StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)) {
			g->state = BS_COMPLETE_HTML;
        }

	if (stream) {
		SndStreamDelete(stream);
	}

	g->imageThreadRef = NULL;
	g->imageThreadLoadCount = 0;
}

Boolean ImageCreateImage(char *src, Int16 *width, Int16 *height, Boolean border, Char *alt, GlobalsType *g)
{
 	Char 		*url;
 	UInt32 		len = 0;
 	unsigned char 	*imgBuf= NULL;
 	WebDataType 		webData;
 	Boolean 	ret = true;
 	int 		wspRet = 0;
	Err		err;
	BitmapPtr	bmp = NULL;
	ImagePtr	img = NULL;
	unsigned char	*channel = NULL;
	Int16		imgMaxWidth = 1;
	Int16		imgOrigWidth = 1;
	UInt16		maxw = 0;

	WspInitPdu(&webData);


	if(!src)
		return false;

	// we have a tag like this: <img src="..." width="x" height="y">, so, create a place holder and load later.
	if ((!g_prefs.images && (*width && *height)) || (*width && *height)) {
		url = Malloc(MAX_URL_LEN+1);
		if (!url)
			return false;
		URLCreateURLStr(src,url,g);

		if ((isLink == true) /*&& (g_prefs.imgBrdr == true)*/ && border) {
			g->x++;
			if ((g->y == 0) || (g->browserGlobals.imgYAdd == true)) { g->y++; g->browserGlobals.imgYAdd = false; }
		}

		maxw = browserGetWidgetBounds(DRAW_WIDTH);

		imgMaxWidth = 1;
		imgOrigWidth = 1;
		if ((*width > browserGetWidgetBounds(DRAW_WIDTH)) && (g_prefs.renderMode == RENDER_WIDE_SCREEN)) {
			g->DrawWidth = *width;
			BrowserRefreshScrollBarHorizontal(g);
		} else if (*width > maxw /*&& g_prefs.imgResize == true*/) {
			UInt32		maxh = *height * maxw / *width;

			imgMaxWidth = maxw;
			imgOrigWidth = *width;

			*width = maxw; *height = maxh;	
		} else {
			imgMaxWidth = maxw;
			imgOrigWidth = *width;
		}

		if (((g->x + *width) > browserGetWidgetBounds(DRAW_WIDTH)) && (g->x != 0)) palmprintln(g);

		browserResizeDimensions(*width, ((isLink == true) /*&& (g_prefs.imgBrdr == true)*/ && border)?*height+1:*height, g);

		if ((g_prefs.renderMode == RENDER_SMALL_SCREEN) && (*width > browserGetWidgetBounds(WIDGET_WIDTH)))
			*width = browserGetWidgetBounds(WIDGET_WIDTH);

		if ((isLink == true) /*&& (g_prefs.imgBrdr == true)*/ && border)
			g->x += *width + 1;
		else
			g->x += *width;

		addNewIndent(g->x, g->y, *height, g);

		img = addPageImage(url, alt, g->x - *width, g->x, g->y, g->y + *height, *width, *height, NULL, g);
		if (g_prefs.images)
			img->download = true;
		img->maxWidth = imgMaxWidth;
		img->origWidth = imgOrigWidth;
		updateScreen(g);

		if (url)
			Free(url);

		return true;
	}

	if (!g_prefs.images)
		return false;

	if (!g->Render || isOption) return true; // should we return false?

	url = Malloc(MAX_URL_LEN+1);
	if (!url)
		return false;
	URLCreateURLStr(src,url,g);

	if ((g->engineStop == true || g->userCancel == true) && !CacheFindCachedFile(url, true, g)) {
		if (url)
			Free(url); 
		return false;
	}
	
   	imgBuf = Malloc(DEF_RECEIVE_BUF+1);       
	if (!imgBuf) { 
		if (url)
			Free(url); 
		return false; 
	} // ErrFatalDisplayIf (!imgBuf, "Malloc Failed");

	len = ImageDownloadImage(&webData, url, imgBuf, g);

	if (len <= 0) {
		ret = false;
		goto cleanup;
	}

	ret = ImageHandleImage(len, &bmp, width, height, &channel, url, imgBuf, isLink, &webData, &wspRet, g);


	if (!wspRet && bmp) {
		if ((isLink == true) /*&& (g_prefs.imgBrdr == true)*/ && border) {
			g->x++;
			if ((g->y == 0) || (g->browserGlobals.imgYAdd == true)) {
				g->y++;
				g->browserGlobals.imgYAdd = false;
			}
		}

		maxw = browserGetWidgetBounds(DRAW_WIDTH);

		imgMaxWidth = 1;
		imgOrigWidth = 1;
		if ((*width > browserGetWidgetBounds(DRAW_WIDTH)) && (g_prefs.renderMode == RENDER_WIDE_SCREEN)) {
			g->DrawWidth = *width;
			BrowserRefreshScrollBarHorizontal(g);
		} else if (*width > maxw /*&& g_prefs.imgResize == true*/) {
			imgMaxWidth = maxw;
			imgOrigWidth = *width;

			if (jpegLibRef && jpegLibRef != sysInvalidRefNum) {
				BitmapType	*newBmp;
				UInt32		maxh = *height * maxw / *width;

				if (BmpGetBitDepth(bmp) < 16)
					goto bottom;

				newBmp = pnoJpeg2Resample(jpegLibRef, bmp, maxw, maxh, &err);
				if (err != errNone) {
					if (bmp) {
						if (newBmp) BmpDelete(newBmp);
						goto bottom;
					} else {
						if (newBmp) BmpDelete(newBmp);
						if (channel) Free(channel);
						goto cleanup;
					}
				}

				if (bmp) BmpDelete(bmp);
				bmp = newBmp;

				*width = maxw; *height = maxh;
			} else {
				if (BmpGetBitDepth(bmp) < 16)
					goto bottom;
		
				bmp = BmpResizeBitmap(bmp, maxw, NULL, width, height, true);
			}
			if (!bmp) {
				if (channel) Free(channel);
				goto cleanup;
			}
	
		}
bottom:

		if (((g->x + *width) > browserGetWidgetBounds(DRAW_WIDTH)) && (g->x != 0)) palmprintln(g);

		browserResizeDimensions(*width, ((isLink == true) /*&& (g_prefs.imgBrdr == true)*/ && border)?*height+1:*height, g);

		if ((g_prefs.renderMode == RENDER_SMALL_SCREEN) && (*width > browserGetWidgetBounds(WIDGET_WIDTH)))
			*width = browserGetWidgetBounds(WIDGET_WIDTH);

		if ((isLink == true) /*&& (g_prefs.imgBrdr == true)*/ && border)
			g->x += *width + 1;
		else
			g->x += *width;

		addNewIndent(g->x, g->y, *height, g);

		if (bmp) {
			img = addPageImage(src, alt, g->x - *width, g->x, g->y, g->y + *height, *width, *height, bmp, g);
			if (StrNCaselessCompare(g->contentType, "image/vnd.palm.pbmp", 19)==0)
				img->alreadyHires = true;
			img->maxWidth = imgMaxWidth;
			img->origWidth = imgOrigWidth;

            // TODO: Add to function ImageSetChannel
			if (channel) {
				img->channel = channel;

                g->pageStats.totalMemoryUsed += MemPtrSize(img->channel);
            }

			updateScreen(g);
		}		
	} else {
		if (bmp)
			BmpDelete(bmp);

		if (wspRet == WSP_ERR_AUTHORIZE) {
			if (imgBuf)
        			Free(imgBuf);
			if (url)
        			Free(url);

    			return ImageCreateImage(src, width, height, border, alt, g);
		} else if ((wspRet == WSP_ERR_REDIRECT || wspRet == HTTP_ERR_REDIRECT) && (g->urlBuf)) {
			Boolean tempRet = false;

			if (imgBuf)
        			Free(imgBuf);
			if (url)
        			Free(url);

			tempRet = ImageCreateImage(g->urlBuf, width, height, border, alt, g);

			Free(g->urlBuf);
			g->urlBuf = NULL;

			return tempRet;
		} else {
			ret = false;
		}
	}

cleanup:

	if (g->conn.connectionType == 'h' || g->conn.connectionType == 'd')
		HTTPFlushCache();

    	if (imgBuf)
        	Free(imgBuf);
	if (url)
        	Free(url);

    	return ret; 
}
