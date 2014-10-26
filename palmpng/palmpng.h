#ifndef PALMPNGLIB_H
#define PALMPNGLIB_H

#define PalmPngHandle void *

//open 'library' with armlet part in resources of type type and starting with id id
//so far its 'armc' and 1000

PalmPngHandle Png_Init(UInt32 type, UInt32 id);

//close 'library'
void Png_Close(PalmPngHandle m);


//decode png data from memory buffer 'png' to Palm Bitmap
//this bitmap will have dimensions set from png image and will be always 16bpp hicolor
//you can choose density of bitmap
//if alpha is not null then if png has alpha channel it will be saved there, if not it will fill buffer with zeros
//alpha will be allocated inside so to function , use it like this
//  unsigned char *alpha; //make sure its aligned to 4 bytes, it will crash otherwise
//  BitmapType *bmp;
//  bmp=(BitmapType *)DecodePngToBitmap(handle,pngimage,kDensityDouble,&alpha);
//  if(bmp)
//  {
//     ....
//     MemPtrFree(BmpGetBits(bmp));
//     DmpDelete(bmp);
//     if(alpha)
//       MemPtrFree(alpha);
//  }

BitmapTypeV3 *DecodePngToBitmap(PalmPngHandle m, const unsigned char *png, UInt16 density, unsigned char **alpha);

//like windrawbitmap but if alpha is present it takes care about transparency
void WinPaintAlphaImage(PalmPngHandle m, BitmapType *bmp,UInt8 *alpha,Coord x,Coord y); 
//like previous but draws to bitmap window (which is byteswapped)
void WinPaintAlphaImageBs(PalmPngHandle m, BitmapType *bmp,UInt8 *alpha,Coord x,Coord y);

//resize an image using bilinear interpolation.
void BmpResizeBilinear(void *m, void *outBitmap, void *inBitmap, Coord inWidth,
                            Coord inHeight, UInt16 inByteWidth, Coord outWidth,
                            Coord outHeight, UInt16 outByteWidth);
                            
void BmpResizeNearestNeighbor(void *m, void *outBitmap, void *inBitmap, Coord inWidth,
                            Coord inHeight, Coord outWidth,
                            Coord outHeight);

void BmpResizeAlphaChannel(void *m, void *outChannel, void *inChannel, Coord inWidth,
                            Coord inHeight, Coord outWidth,
                            Coord outHeight);

#endif