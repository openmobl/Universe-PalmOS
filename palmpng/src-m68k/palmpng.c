#include <PalmOS.h>
#include <PalmOSGlue.h>
#include "stdio.h"
#include "peal.h"
#include "macros.h"
#include "../src-arm/palmpng.h"
#include "../palmpng.h"
//#include "../test/rsrc/resources.h"

static void *m68k_MemPtrNew(const UInt32 size)
{
    void *ptr;

    ptr=MemGluePtrNew(size);
    return(ptr);
}

static void m68k_Alert(const char *txt)
{
//    FrmCustomAlert(AL_DEBUG,txt,"","");
WinDrawChars(txt, StrLen(txt), 0, 0);
}

static void m68k_AlertNum(const char *txt, UInt32 num, UInt32 num2)
{
    char x[256];

    StrPrintF(x,"%s  %ld %ld    ",txt,num, num2);
    m68k_Alert(x);
//    FrmCustomAlert(AL_DEVICE,x,"","");
}

static BitmapTypeV3 *m68k_BmpCreateBitmapV3(const BitmapType *bitmapP, UInt16 density, const void *bitsP, const ColorTableType *colorTableP)
{
    BitmapTypeV3 *bmp;

//    m68k_Alert("pred bmp");
    bmp=BmpCreateBitmapV3(bitmapP,density,bitsP,NULL);
//    m68k_Alert("po bmp");
    return(bmp);
}

static BitmapType *m68k_BmpCreate(Coord width, Coord height, UInt8 bpp, ColorTableType *colorTableP, Err *error)
{
    BitmapType *bmp;

    bmp=BmpCreate(width,height,bpp,colorTableP,error);
    return(bmp);
}

BitmapTypeV3 *DecodePngToBitmap(void *m, const unsigned char *png, UInt16 density, unsigned char **alpha)
{
    BitmapTypeV3 *bmp;
    void *func;
    DecodePngToBitmapParamsType param;
    unsigned char **temp;

    bmp=NULL;
    if(m)
    {
	func=PealLookupSymbol((PealModule *)m,"pnoDecodePngToBitmap");
        if(func)
	{
	    //this solves aligning problem with alpha
	    temp=(unsigned char **)MemPtrNew(sizeof(unsigned char *));
	    if(temp)
	    {
		param.png=png;
		param.alpha=temp;
		param.density=density;
		bmp=(BitmapTypeV3 *)PealCall((PealModule *)m,func,&param);
		if(alpha)
		    *alpha=*temp;
		MemPtrFree(temp);
	    }	
	}	
    }	
    return(bmp);
}

void WinPaintAlphaImage(void *m, BitmapType *bmp, unsigned char *alpha, Coord x, Coord y)
{
    BitmapType *winbmp;
    void *func;
    WinPaintAlphaImageParamsType param;
    WinHandle win;
    UInt8 *bmpdata;
    UInt8 *windata;
    RectangleType r;
    RectangleType clip;
    Coord w,h;
    Coord winw,winh;
    UInt16 mul;    

    win=WinGetDrawWindow();
    if(m && win)
    {
	winbmp=WinGetBitmap(win);
	windata=BmpGetBits(winbmp);
	BmpGetDimensions(winbmp,&winw,&winh,NULL);
	WinGetDrawWindowBounds(&r);
	if((BmpGetDensity(winbmp)==kDensityDouble) && (WinGetCoordinateSystem()==kCoordinatesStandard))
	    mul=2;
	else
	    mul=1;    
	WinGetClip(&clip);

	bmpdata=BmpGetBits(bmp);
	BmpGetDimensions(bmp,&w,&h,NULL);


	func=PealLookupSymbol((PealModule *)m,"pnoWinPaintAlphaImage");
	if(func)
	{
	    param.bitmap=bmpdata;
	    param.bmp_w=w;
	    param.bmp_h=h;
	    param.alpha=alpha;
	    param.window=windata;
	    param.win_x=r.topLeft.x;
	    param.win_y=r.topLeft.y;
	    param.win_w=winw;
	    param.win_h=winh;
	    param.x=mul*x;
	    param.y=mul*y;
	    param.clip_x=mul*clip.topLeft.x;
	    param.clip_y=mul*clip.topLeft.y;
	    param.clip_w=mul*clip.extent.x;
	    param.clip_h=mul*clip.extent.y;
	    param.bswap=0;
	    PealCall((PealModule *)m,func,&param);
	}	
    }	
}

void WinPaintAlphaImageBs(void *m, BitmapType *bmp, unsigned char *alpha, Coord x, Coord y)
{
    BitmapType *winbmp;
    void *func;
    WinPaintAlphaImageParamsType param;
    WinHandle win;
    UInt8 *bmpdata;
    UInt8 *windata;
    RectangleType r;
    RectangleType clip;
    Coord w,h;
    Coord winw,winh;
    UInt16 mul;    

    win=WinGetDrawWindow();
    if(m && win)
    {
	winbmp=WinGetBitmap(win);
	windata=BmpGetBits(winbmp);
	BmpGetDimensions(winbmp,&winw,&winh,NULL);
	WinGetDrawWindowBounds(&r);
	if((BmpGetDensity(winbmp)==kDensityDouble) && (WinGetCoordinateSystem()==kCoordinatesStandard))
	    mul=2;
	else
	    mul=1;    
	WinGetClip(&clip);

	bmpdata=BmpGetBits(bmp);
	BmpGetDimensions(bmp,&w,&h,NULL);


	func=PealLookupSymbol((PealModule *)m,"pnoWinPaintAlphaImage");
	if(func)
	{
	    param.bitmap=bmpdata;
	    param.bmp_w=w;
	    param.bmp_h=h;
	    param.alpha=alpha;
	    param.window=windata;
	    param.win_x=r.topLeft.x;
	    param.win_y=r.topLeft.y;
	    param.win_w=winw;
	    param.win_h=winh;
	    param.x=mul*x;
	    param.y=mul*y;
	    param.clip_x=mul*clip.topLeft.x;
	    param.clip_y=mul*clip.topLeft.y;
	    param.clip_w=mul*clip.extent.x;
	    param.clip_h=mul*clip.extent.y;
	    param.bswap=1;
	    PealCall((PealModule *)m,func,&param);
	}	
    }	
}

void BmpResizeBilinear(void *m, void *outBitmap, void *inBitmap, Coord inWidth,
                            Coord inHeight, UInt16 inByteWidth, Coord outWidth,
                            Coord outHeight, UInt16 outByteWidth)
{
    void                *func;
    ResizeParamsType    param;

    if (m) {
        func = PealLookupSymbol((PealModule *)m,"pnoBmpResizeBilinear");
        if(func) {
            param.outBitmap     = outBitmap;
            param.inBitmap      = inBitmap;
            param.inWidth       = inWidth;
            param.inHeight      = inHeight;
            param.inByteWidth   = inByteWidth;
            param.outWidth      = outWidth;
            param.outHeight     = outHeight;
            param.outByteWidth  = outByteWidth;
        
            PealCall((PealModule *)m,func,&param);
        }
    }
}

void BmpResizeNearestNeighbor(void *m, void *outBitmap, void *inBitmap, Coord inWidth,
                            Coord inHeight, Coord outWidth,
                            Coord outHeight)
{
    void                *func;
    ResizeParamsType    param;

    if (m) {
        func = PealLookupSymbol((PealModule *)m,"pnoBmpResizeNearestNeighbor");
        if(func) {
            param.outBitmap     = outBitmap;
            param.inBitmap      = inBitmap;
            param.inWidth       = inWidth;
            param.inHeight      = inHeight;
            param.inByteWidth   = 0;
            param.outWidth      = outWidth;
            param.outHeight     = outHeight;
            param.outByteWidth  = 0;
        
            PealCall((PealModule *)m,func,&param);
        }
    }
}

void BmpResizeAlphaChannel(void *m, void *outChannel, void *inChannel, Coord inWidth,
                            Coord inHeight, Coord outWidth,
                            Coord outHeight)
{
    void                *func;
    ResizeParamsType    param;

    if (m) {
        func = PealLookupSymbol((PealModule *)m,"pnoBmpResizeAlphaChannel");
        if(func) {
            param.outBitmap     = outChannel;
            param.inBitmap      = inChannel;
            param.inWidth       = inWidth;
            param.inHeight      = inHeight;
            param.inByteWidth   = 0;
            param.outWidth      = outWidth;
            param.outHeight     = outHeight;
            param.outByteWidth  = 0;
        
            PealCall((PealModule *)m,func,&param);
        }
    }
}

void *Png_Init(UInt32 type, UInt32 id)
{
    UInt32 processorType;
    Err err;
    UInt32 *fptr;
    PealModule *m;

    m=NULL;

    err=FtrGet(sysFileCSystem, sysFtrNumProcessorID, &processorType);
    if((!err) && (sysFtrNumProcessorIsARM(processorType)))
    {
	m=PealLoadFromResources(type,id);
	if(m)
	{	
	    (void *)fptr=PealLookupSymbol(m,"m68k_MemPtrNew");
	    if(fptr)
    		*fptr=ByteSwap32((UInt32)m68k_MemPtrNew);

	    (void *)fptr=PealLookupSymbol(m,"m68k_Alert");
	    if(fptr)
		*fptr=ByteSwap32((UInt32)m68k_Alert);

	    (void *)fptr=PealLookupSymbol(m,"m68k_AlertNum");
	    if(fptr)
		*fptr=ByteSwap32((UInt32)m68k_AlertNum);

	    (void *)fptr=PealLookupSymbol(m,"m68k_BmpCreateBitmapV3");
	    if(fptr)
		*fptr=ByteSwap32((UInt32)m68k_BmpCreateBitmapV3);

	    (void *)fptr=PealLookupSymbol(m,"m68k_BmpCreate");
	    if(fptr)
		*fptr=ByteSwap32((UInt32)m68k_BmpCreate);

	}    
    }    
    return((void *)m);
}

void Png_Close(void *m)
{
    if(m)
	PealUnload((PealModule *)m);
}


