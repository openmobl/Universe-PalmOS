#ifndef PALMPNG_H
#define PALMPNG_H

#ifdef PALMPNGDEF_H
typedef struct DecodePngToBitmapParams
{
    unsigned char png[4];
    unsigned char alpha[4];
    unsigned char density[2];
} DecodePngToBitmapParamsType;
#else
typedef struct DecodePngToBitmapParams
{
    const void 		*png;
    unsigned char	**alpha;
    UInt16 		density;
} DecodePngToBitmapParamsType;
#endif

BitmapTypeV3 *pnoDecodePngToBitmap(DecodePngToBitmapParamsType *params);

#ifdef PALMPNGDEF_H
typedef struct WinPaintAlphaImageParams
{
    unsigned char bitmap[4];
    unsigned char bmp_w[2];
    unsigned char bmp_h[2];
    unsigned char alpha[4];
    unsigned char window[4];
    unsigned char win_x[2];
    unsigned char win_y[2];
    unsigned char win_w[2];
    unsigned char win_h[2];
    unsigned char x[2];
    unsigned char y[2];
    unsigned char clip_x[2];
    unsigned char clip_y[2];
    unsigned char clip_w[2];
    unsigned char clip_h[2];
    unsigned char bswap[2];
} WinPaintAlphaImageParamsType;
#else
typedef struct WinPaintAlphaImageParamsParams
{
    unsigned char 	*bitmap;
    Coord		bmp_w;
    Coord	 	bmp_h;
    unsigned char	*alpha;
    unsigned char 	*window;
    Coord		win_x;
    Coord	 	win_y;
    Coord		win_w;
    Coord	 	win_h;
    Coord		x;
    Coord		y;
    Coord		clip_x;
    Coord		clip_y;
    Coord		clip_w;
    Coord		clip_h;
    Int16		bswap;
} WinPaintAlphaImageParamsType;
#endif

void pnoWinPaintAlphaImage(WinPaintAlphaImageParamsType *params);

#ifdef PALMPNGDEF_H
typedef struct ResizeParams
{
    unsigned char  outBitmap[4];
    unsigned char  inBitmap[4];
    
    unsigned char  inWidth[2];
    unsigned char  inHeight[2];
    unsigned char  inByteWidth[2];
    
    unsigned char  outWidth[2];
    unsigned char  outHeight[2];
    unsigned char  outByteWidth[2];
} ResizeParamsType;
#else
typedef struct ResizeParams
{
    void*  outBitmap;
    void*  inBitmap;
    
    Coord  inWidth;
    Coord  inHeight;
    UInt16 inByteWidth;
    
    Coord  outWidth;
    Coord  outHeight;
    UInt16 outByteWidth;
} ResizeParamsType;
#endif

void pnoBmpResizeBilinear(ResizeParamsType *params);
void pnoBmpResizeNearestNeighbor(ResizeParamsType *params);
void pnoBmpResizeAlphaChannel(ResizeParamsType *params);

#endif
