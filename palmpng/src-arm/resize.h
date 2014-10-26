#ifndef _BILINEAR_H_
#define _BILINEAR_H_

extern void DoBilinear( void* outBitmap,
    Coord outWidth, Coord outHeight, UInt16 outByteWidth,
    void* inBitmap, Coord inWidth, Coord inHeight, UInt16 inByteWidth );

extern void DoNearestNeighbor( void* outBitmap, Coord outWidth, Coord outHeight,
                               void* inBitmap, Coord inWidth, Coord inHeight );
                               
extern void ResizeAlpha( void* outChannel, Coord outWidth, Coord outHeight,
                        void* inChannel, Coord inWidth, Coord inHeight );
#endif
