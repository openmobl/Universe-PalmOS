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
#include 	<PalmOS.h>
#include	"Bitmap.h"
#include	"formBrowser.h"

/*#define Read32(addr)  \
	( ((((unsigned char *)(addr))[3])) | \
	  ((((unsigned char *)(addr))[2])) | \
	  ((((unsigned char *)(addr))[1])) | \
	  ((((unsigned char *)(addr))[0])) )
#define Read16(addr)  \
	( ((((unsigned char *)(addr))[1])) | \
	  ((((unsigned char *)(addr))[0])) )*/

#define ByteSwap16(n) ( ((((unsigned int) n) << 8) & 0xFF00) | \
                        ((((unsigned int) n) >> 8) & 0x00FF) )

#define ByteSwap32(n) ( ((((unsigned long) n) << 24) & 0xFF000000) |	\
                        ((((unsigned long) n) <<  8) & 0x00FF0000) |	\
                        ((((unsigned long) n) >>  8) & 0x0000FF00) |	\
                        ((((unsigned long) n) >> 24) & 0x000000FF) )

#define Read32(addr)  \
	( ((long)(((unsigned char *)(addr))[3]) << 24) | \
	  ((long)(((unsigned char *)(addr))[2]) << 16) | \
	  ((long)(((unsigned char *)(addr))[1]) <<  8) | \
	  ((long)(((unsigned char *)(addr))[0])) )
#define Read16(addr)  \
	( ((((unsigned char *)(addr))[1]) <<  8) | \
	  ((((unsigned char *)(addr))[0])) )

/*Err DecodeBmp(UInt8 *buf, UInt32 bufSize, UInt32 *width, UInt32 *height, void **bitmap) //WinHandle win)
{
	UInt32 		fileSize;
	UInt32 		picOffset;
	UInt32 		bmpWidth;
	UInt32 		bmpHeight;
	UInt16 		depth;
	UInt32 		compression;
	UInt8  		*imageData,*imageDataBase;
	UInt16		*winData;
	BitmapPtr	winBitmap;
	Int32 		i,j;
	UInt8 		r,g,b;
	UInt32 		bytesPerRow;
	UInt32 		rowModulus;
	UInt16 		err = errNone;

	if ((buf[0] != 'B') || (buf[1] != 'M')) { err = winbmpErrInvalid; goto error_report; }
	fileSize 	= Read32(&buf[2]);
	picOffset 	= Read32(&buf[10]);
	bmpWidth 	= Read32(&buf[18]);
	bmpHeight 	= Read32(&buf[22]);
	depth 		= Read16(&buf[28]); //buf[28]; // Read16(&buf[28]);
	compression 	= Read32(&buf[30]);

	imageDataBase 	= buf + picOffset;
	
	bytesPerRow 	= bmpWidth * 3;
	rowModulus 	= bytesPerRow%4;

	if (rowModulus) bytesPerRow += (4-rowModulus);

	winBitmap 	= BmpCreate(bmpWidth, bmpHeight, 16, NULL, &err); 
	winData 	= BmpGetBits(winBitmap);

	for (j=bmpHeight-1;j>=0;j--)
	{
		imageData = imageDataBase + (j*bytesPerRow);
		for (i=0;i<bmpWidth;i++)
		{
			switch (depth) {
				//case 8:
				//	b = (*imageData++);
				//	g = (*imageData++);
				//	r = (*imageData++);
		    		//	if( j&1 )
		    		//	{
				//		if( r&16 && r < 248 ) r += 8;
				//		if( g&16 && g < 248 ) g += 8;
				//		if( b&32 && b < 252 ) b += 4;
		    		//	}
		    		//	r >>= 5;
		    		//	g >>= 5;
		    		//	b >>= 6;
		    		//	*winData++ = r + (g<<3) + (b<<6);
				//	// *winData++ = ByteSwap16(((UInt16)g<<3)|((UInt16)b<<6)|r);
				//	break;
				//case 4:
				//	//break;
				//case 2:
				//	//break;
				//case 24:
				//case 16:
				//case 1:
				//	{
				//		int w = (*imageData++ & (0x01 << (7 - (i & 7)))) ? 1 : 0;
				//		//RGBColorType rgbP;
				//
				//		//WinIndexToRGB (w, &rgbP);
				//
				//		//r = rgbP.r >> 3;
				//		//b = rgbP.b >> 2;
				//		//g = rgbP.g >> 3;
				//		
				//		// *winData++ = (r<<11) + (g<<5) + b;
				//		if (w == 1) {
				//			 *winData++ |= (0x01 << (7 - (i & 7)));
				//		}
				//	}
				//	break;
				default:
					b = (*imageData++) >> 3;
					g = (*imageData++) >> 2;
					r = (*imageData++) >> 3;
					// *winData++ = ByteSwap16(((UInt16)r<<11)|((UInt16)g<<5)|b);
					*winData++ = (r<<11) + (g<<5) + b;
					break;
			}				
		}
	}

	*width = bmpWidth;
	*height = bmpHeight;

	*bitmap = winBitmap;

	return errNone;
error_report:

	return err;
}*/

typedef struct {
   UInt16 	type;                 		/* Magic identifier            */
   UInt32 	size;                       	/* File size in bytes          */
   UInt16 	reserved1, reserved2;
   UInt32	offset;                     	/* Offset to image data, bytes */
} BmpHeader;

typedef struct {
   UInt32 	size;               		/* Header size in bytes      */
   Int32 	width,height;                	/* Width and height of image */
   UInt16 	planes;       			/* Number of colour planes   */
   UInt16 	bits;         			/* Bits per pixel            */
   UInt32 	compression;        		/* Compression type          */
   UInt32 	imagesize;          		/* Image size in bytes       */
   Int32 	xresolution,yresolution;    	/* Pixels per meter          */
   UInt32 	ncolors;           		/* Number of colours         */
   UInt32 	importantcolours;   		/* Important colours         */
} BmpInfoHeader;

typedef struct {
   unsigned char r,g,b,junk;
} BmpColorIndex;

Err DecodeBmp(UInt8 *buf, UInt32 bufSize, UInt32 *width, UInt32 *height, void **bitmap) //WinHandle win)
{
	UInt32 		fileSize;
	UInt32 		picOffset;
	UInt32 		bmpWidth;
	UInt32 		bmpHeight;
	UInt16 		depth;
	UInt32 		compression;
	UInt32		ncolors;
	UInt8  		*imageData,*imageDataBase;
	UInt16		*winData;
	BitmapPtr	winBitmap;
	Int32 		i,j,k;
	UInt8 		r,g,b,grey;
	UInt32 		bytesPerRow;
	UInt32 		rowModulus;
	UInt16 		err = errNone;

	BmpHeader	header;
	BmpInfoHeader	info;
	BmpColorIndex	colors[255];
	Boolean		gotIndex = false;
	int		colorCount = 0;

	if ((buf[0] != 'B') || (buf[1] != 'M')) { err = winbmpErrInvalid; goto error_report; }
	//fileSize 	= Read32(&buf[2]);
	//picOffset 	= Read32(&buf[10]);
	//bmpWidth 	= Read32(&buf[18]);
	//bmpHeight 	= Read32(&buf[22]);
	//depth 	= Read16(&buf[28]); //buf[28]; // Read16(&buf[28]);
	//compression 	= Read32(&buf[30]);

	MemMove(&header, buf, sizeof(BmpHeader));
	MemMove(&info, buf + sizeof(BmpHeader), sizeof(BmpInfoHeader));

	fileSize 	= Read32(&header.size);
	picOffset 	= Read32(&header.offset);
	bmpWidth 	= Read32(&info.width);
	bmpHeight 	= Read32(&info.height);
	depth 		= Read16(&info.bits); //buf[28]; // Read16(&buf[28]);
	compression 	= Read32(&info.compression);
	ncolors		= Read32(&info.ncolors);

	imageDataBase 	= buf + picOffset;
	
	bytesPerRow 	= bmpWidth * 3;
	rowModulus 	= bytesPerRow%4;

	if (rowModulus) bytesPerRow += (4-rowModulus);

	winBitmap 	= BmpCreate(bmpWidth, bmpHeight, 16, NULL, &err); 
	winData 	= BmpGetBits(winBitmap);

	// get color lookup table, if available
   	for (k=0;k<255;k++) {
		colors[k].r = SysRandom(255) % 256;
		colors[k].g = SysRandom(255) % 256;
		colors[k].b = SysRandom(255) % 256;
		colors[k].junk = SysRandom(255) % 256;
	}

	if (depth == 8) {
		UInt8	*colorTable;

		if (ncolors > 0) {
    			colorCount = ncolors;
    		} else {
    			colorCount = (1 & 0xff) << depth;
    		}

		colorTable = buf + sizeof(BmpHeader) + sizeof(BmpInfoHeader);

		if (colorCount > 0) {
			for (k=0; k<colorCount; k++) {
				colors[k].b = *colorTable++;
				colors[k].g = *colorTable++;
				colors[k].r = *colorTable++;
				colors[k].junk = *colorTable++;
      			}
      			gotIndex = true;
   		}
	}

	for (j=bmpHeight-1;j>=0;j--)
	{
		imageData = imageDataBase + (j*bytesPerRow);
		for (i=0;i<bmpWidth;i++)
		{
			switch (depth) {
				//case 8:
				//	b = (*imageData++);
				//	g = (*imageData++);
				//	r = (*imageData++);
		    		//	if( j&1 )
		    		//	{
				//		if( r&16 && r < 248 ) r += 8;
				//		if( g&16 && g < 248 ) g += 8;
				//		if( b&32 && b < 252 ) b += 4;
		    		//	}
		    		//	r >>= 5;
		    		//	g >>= 5;
		    		//	b >>= 6;
		    		//	*winData++ = r + (g<<3) + (b<<6);
				//	// *winData++ = ByteSwap16(((UInt16)g<<3)|((UInt16)b<<6)|r);
				//	break;
				//case 4:
				//	//break;
				//case 2:
				//	//break;
				//case 24:
				//case 16:
				//case 1:
				//	{
				//		int w = (*imageData++ & (0x01 << (7 - (i & 7)))) ? 1 : 0;
				//		//RGBColorType rgbP;
				//
				//		//WinIndexToRGB (w, &rgbP);
				//
				//		//r = rgbP.r >> 3;
				//		//b = rgbP.b >> 2;
				//		//g = rgbP.g >> 3;
				//		
				//		// *winData++ = (r<<11) + (g<<5) + b;
				//		if (w == 1) {
				//			 *winData++ |= (0x01 << (7 - (i & 7)));
				//		}
				//	}
				//	break;
				case 8:
					grey = (*imageData++);
					if (gotIndex) {
               					//r = colors[grey].r >> 3;
               					//g = colors[grey].g >> 2;
               					//b = colors[grey].b >> 3;
						//*winData++ = (r<<11) + (g<<5) + b;
						
						// Which is best??? /\ or \/
						
						r = colors[grey].r;
               					g = colors[grey].g;
               					b = colors[grey].b;
						*winData++ = ((((int)r & 0xF8) << 8) |       // 1111100000000000 
                         				      (((int)g & 0xFC) << 3) |       // 0000011111100000
                         				      (((int)b & 0xF8) >> 3));       // 0000000000011111
            				} else { // is this correct?
						RGBColorType	rgb;

						WinIndexToRGB(grey, &rgb);

						r = rgb.r;
               					g = rgb.g;
               					b = rgb.b;
						*winData++ = ((((int)r & 0xF8) << 8) |       // 1111100000000000 
                         				      (((int)g & 0xFC) << 3) |       // 0000011111100000
                         				      (((int)b & 0xF8) >> 3));       // 0000000000011111

               					//*winData++ = grey;
            				}
					break;
				case 24:
				default:
					b = (*imageData++) >> 3;
					g = (*imageData++) >> 2;
					r = (*imageData++) >> 3;
					//*winData++ = ByteSwap16(((UInt16)r<<11)|((UInt16)g<<5)|b);
					*winData++ = (r<<11) + (g<<5) + b;
					break;
			}				
		}
	}

	*width = bmpWidth;
	*height = bmpHeight;

	*bitmap = winBitmap;

	return errNone;
error_report:

	return err;
}
