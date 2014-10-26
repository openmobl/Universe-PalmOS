#include "armtypes.h"

//#include "colors.h"

#ifndef RED_BITS

#define RED_BITS                     5
#define GREEN_BITS                   6
#define BLUE_BITS                    5
#define MAX_RED                      ( ( 1 << RED_BITS ) - 1 )
#define MAX_GREEN                    ( ( 1 << GREEN_BITS ) - 1 )
#define MAX_BLUE                     ( ( 1 << BLUE_BITS ) - 1 )

#endif

UInt16 ByteSwap16(UInt16 n)
{
    return ((((unsigned int) n) << 8) & 0xFF00) |
           ((((unsigned int) n) >> 8) & 0x00FF);
}


//#define ARMLET

/* byte widths must be even */
void DoBilinear( void* outBitmap,
    Coord outWidth, Coord outHeight, UInt16 outByteWidth,
    void* inBitmap, Coord inWidth, Coord inHeight, UInt16 inByteWidth )
{
    Int32 y;
    Int32 x;
    Int32 W;
    Int32 H;
    Int32 w;
    Int32 h;
    UInt16* in;
    UInt16* out;
    Int32  inDByteWidth;
    Int32  outDByteWidth;
    Int32 m;
    Int32 n;
    Int32 v[ 4 ][ 3 ];

    W = outWidth - 1;
    H = outHeight - 1;
    w = inWidth - 1;
    h = inHeight - 1;
    in  = inBitmap;
    out = outBitmap;
    inDByteWidth  = inByteWidth / 2;
    outDByteWidth = outByteWidth / 2;

    for ( y = 0 ; y < outHeight ; y++ ) {
        UInt16* outLine;
        UInt16* inLine;
        Int32   prevM;

        n = ( y * h ) / H;

        inLine  = in  + inDByteWidth * n;
        outLine = out + outDByteWidth * y;

        prevM = -1;

        for ( x = 0 ; x < outWidth ; x++, outLine++ ) {
            Int32  vMult[ 4 ];
            Int32  vOut[ 3 ];
            Int32  WH;
            UInt16  val;

            m = ( x * w ) / W;

            vMult[ 0 ] = ((n+1)*H-h*y)*((m+1)*W-w*x);
            vMult[ 1 ] = (w*x-m*W)*((n+1)*H-h*y);
            vMult[ 2 ] = (h*y-n*H)*((m+1)*W-w*x);
            vMult[ 3 ] = (n*H-h*y)*(m*W-w*x);

            if ( m != prevM ) {
                UInt16* p;

                p = inLine + m;

#ifdef ARMLET
                val = p[ 0 ];
#else
                val = ByteSwap16( p[ 0 ] );
#endif

                v[ 0 ][ 0 ] = val >> ( GREEN_BITS + BLUE_BITS );
                v[ 0 ][ 1 ] = ( val >> ( BLUE_BITS ) ) & MAX_GREEN;
                v[ 0 ][ 2 ] = val & MAX_BLUE;

                if ( m + 1 < w ) {
#ifdef ARMLET
                    val = p[ 1 ];
#else
                    val = ByteSwap16( p[ 1 ] );
#endif

                    v[ 1 ][ 0 ] = val >> ( GREEN_BITS + BLUE_BITS );
                    v[ 1 ][ 1 ] = ( val >> ( BLUE_BITS ) ) & MAX_GREEN;
                    v[ 1 ][ 2 ] = val & MAX_BLUE;
                }

                if ( n < h ) {
#ifdef ARMLET
                    val = p[ inDByteWidth ];
#else
                    val = ByteSwap16( p[ inDByteWidth ] );
#endif

                    v[ 2 ][ 0 ] = val >> ( GREEN_BITS + BLUE_BITS );
                    v[ 2 ][ 1 ] = ( val >> ( BLUE_BITS ) ) & MAX_GREEN;
                    v[ 2 ][ 2 ] = val & MAX_BLUE;

                    if ( m < w ) {
#ifdef ARMLET
                        val = p[ inDByteWidth + 1 ];
#else
                        val = ByteSwap16( p[ inDByteWidth + 1 ] );
#endif

                        v[ 3 ][ 0 ] = val >> ( GREEN_BITS + BLUE_BITS );
                        v[ 3 ][ 1 ] = ( val >> ( BLUE_BITS ) ) & MAX_GREEN;
                        v[ 3 ][ 2 ] = val & MAX_BLUE;
                    }
                }
            }

            WH = W * H;

            vOut[ 0 ] = ( v[ 0 ][ 0 ] * vMult[ 0 ] +
                        v[ 1 ][ 0 ] * vMult[ 1 ] +
                        v[ 2 ][ 0 ] * vMult[ 2 ] +
                        v[ 3 ][ 0 ] * vMult[ 3 ] + WH / 2 ) / ( WH );
            vOut[ 1 ] = ( v[ 0 ][ 1 ] * vMult[ 0 ] +
                        v[ 1 ][ 1 ] * vMult[ 1 ] +
                        v[ 2 ][ 1 ] * vMult[ 2 ] +
                        v[ 3 ][ 1 ] * vMult[ 3 ] + WH / 2 ) / ( WH );
            vOut[ 2 ] = ( v[ 0 ][ 2 ] * vMult[ 0 ] +
                        v[ 1 ][ 2 ] * vMult[ 1 ] +
                        v[ 2 ][ 2 ] * vMult[ 2 ] +
                        v[ 3 ][ 2 ] * vMult[ 3 ] + WH / 2 ) / ( WH );

            val = ( vOut[ 0 ] << ( GREEN_BITS + BLUE_BITS ) ) |
                  ( vOut[ 1 ] << ( BLUE_BITS ) ) |
                  ( vOut[ 2 ] );

#ifdef ARMLET
            *outLine = val;
#else
            *outLine = ByteSwap16( val );
#endif
            prevM = m;
        }

    }
}

static int myRound(float value)
{
 	int result = 0;
 
 	result = (int)(value + 0.5);
 
 	return(result);
}

void DoNearestNeighbor( void* outBitmap, Coord outWidth, Coord outHeight,
                        void* inBitmap, Coord inWidth, Coord inHeight )
{
    Int32       y;
    Int32       x;
    Int32       W;
    Int32       H;
    Int32       w;
    Int32       h;
    UInt16*     in;
    UInt16*     out;
    UInt32      pos, posOrig;
    float		xStep, yStep;
    Int32 	ox,oy;	

    W = outWidth;
    //if (W % 2)
    //    W - 1;
    H = outHeight;
    //if (H % 2)
    //    H - 1;
    w = inWidth;
    //if (w % 2)
    //    w - 1;
    h = inHeight;
    //if (h % 2)
    //    h - 1;
    in  = inBitmap;
    out = outBitmap;

    xStep = (float)(w) / (float)(W);
 	yStep = (float)(h) / (float)(H);

 	for(y = 0; y < H; y++) {
  		for(x = 0; x < W; x++) {
   			pos = (y * W) + x;
			ox=myRound(x * xStep);
			oy=myRound(y * yStep);
//			if(ox<0)
//			    ox=0;
//			if(oy<0)
//			    oy=0;
			if(ox>=w)
			    ox=w-1;
			if(oy>=h)
			    oy=h-1;
   			posOrig = ( oy * w) + ox;

   			out[pos] = in[posOrig];
  		}
 	}
}

void ResizeAlpha( void* outChannel, Coord outWidth, Coord outHeight,
                        void* inChannel, Coord inWidth, Coord inHeight )
{
    Int32       y;
    Int32       x;
    Int32       W;
    Int32       H;
    Int32       w;
    Int32       h;
    UInt8*     in;
    UInt8*     out;
    UInt32      pos, posOrig;
 	float		xStep, yStep;
    Int32 	ox,oy;	

    W = outWidth;
    //if (W % 2)
    //    W - 1;
    H = outHeight;
    //if (H % 2)
    //    H - 1;
    w = inWidth;
    //if (w % 2)
    //    w - 1;
    h = inHeight;
    //if (h % 2)
    //    h - 1;
    in  = inChannel;
    out = outChannel;

    xStep = (float)(w) / (float)(W);
    yStep = (float)(h) / (float)(H);

	for(y = 0; y < H; y++) {
  		for(x = 0; x < W; x++) {
   			pos = (y * W) + x;
			ox=myRound(x * xStep);
			oy=myRound(y * yStep);
//			if(ox<0)
//			    ox=0;
//			if(oy<0)
//			    oy=0;
			if(ox>=w)
			    ox=w-1;
			if(oy>=h)
			    oy=h-1;
   			posOrig = ( oy * w) + ox;

   			out[pos] = in[posOrig];
  		}
 	}
}




