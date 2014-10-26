#include <PalmOS.h>
#include <PalmOSGlue.h>
#include "rsrc/resources.h"
#include "../palmpng.h"

#undef ERROR_CHECK_LEVEL
#define ERROR_CHECK_LEVEL ERROR_CHECK_FULL

static void MemInfo(void)
{
    UInt32 x1,x2;
    char x[256];
    
    MemHeapFreeBytes(0,&x1,&x2);
    StrPrintF(x,"total: %ld, free: %ld",x1,x2);
    FrmCustomAlert(AL_DEBUG,x,NULL,NULL);
}

Err BmpResizeImage(void *m, BitmapType **bitmapP, Coord width, Coord height,
                        UInt16 density, unsigned char **channel)
{
    UInt16      imgWidth            = 0;
    UInt16      imgHeight           = 0;
    Char        *newChannel         = NULL;
    BitmapPtr   newBitmap           = NULL;
    BitmapPtr   newBitmapV3         = NULL;
    UInt16      byteWidth_bmp       = 0;
    UInt16      byteWidth_bmpNew    = 0;
    Err         error               = errNone;
    
    newBitmap   = BmpCreate(width, height, 16, BmpGetColortable(*bitmapP), &error);
    if (!newBitmap) {
        return error;
    }
    
    newBitmapV3 = BmpCreateBitmapV3(newBitmap, density,
                    BmpGetBits(newBitmap), NULL);//BmpGetColortable((BitmapType *)*bitmapP));
    if (!newBitmapV3) {
        BmpDelete(newBitmap);
        return error; // umm... bad...
    }

    BmpGetDimensions((BitmapType*)*bitmapP, &imgWidth, &imgHeight, &byteWidth_bmp);
    BmpGetDimensions((BitmapType*)newBitmapV3, NULL, NULL, &byteWidth_bmpNew);

    BmpResizeBilinear(m, BmpGetBits((BitmapType*)newBitmapV3),
                        BmpGetBits((BitmapType*)*bitmapP),
                        imgWidth, imgHeight, byteWidth_bmp,
                        width, height, byteWidth_bmpNew);
    
    if (channel && *channel) {
        newChannel = MemGluePtrNew(width * height);
        if (newChannel) {
            BmpResizeAlphaChannel(m, newChannel, *channel,
                            imgWidth, imgHeight, width, height);
                            
            MemPtrFree(*channel);
            *channel = newChannel;
        }
    }
    
    BmpDelete(*bitmapP);
    *bitmapP = newBitmapV3;
    
    return errNone;
}

static void ShowPng(void *m,UInt16 id)
{
    MemHandle hn;		    
    BitmapTypeV3 *bmp3;
    unsigned char *alpha;

//    MemInfo();
    hn=DmGetResource('png_',id);
    bmp3=DecodePngToBitmap(m,MemHandleLock(hn),kDensityDouble,NULL);
    if(bmp3)
    {
        WinSetDrawWindow(WinGetDisplayWindow());
        WinDrawBitmap((BitmapType *)bmp3,0,0);
	MemPtrFree(BmpGetBits((BitmapType *)bmp3));
	BmpDelete((BitmapType *)bmp3);
    }	
    MemHandleUnlock(hn);
    DmReleaseResource(hn);
//    MemInfo();
SysTaskDelay(SysTicksPerSecond() * 2);
    hn=DmGetResource('png_',PngFace);
    bmp3=DecodePngToBitmap(m,MemHandleLock(hn),kDensityDouble,&alpha);
    if(bmp3)
    {
        WinSetDrawWindow(WinGetDisplayWindow());
        WinPaintAlphaImage(m,(BitmapType *)bmp3,alpha,40,40);
	MemPtrFree(BmpGetBits((BitmapType *)bmp3));
	BmpDelete((BitmapType *)bmp3);
	MemPtrFree(alpha);
    }	
    MemHandleUnlock(hn);
    DmReleaseResource(hn);
SysTaskDelay(SysTicksPerSecond() * 2);

    hn=DmGetResource('png_',PngBlue);
    bmp3=DecodePngToBitmap(m,MemHandleLock(hn),kDensityDouble,&alpha);
    if(bmp3)
    {
        WinSetDrawWindow(WinGetDisplayWindow());
//        WinDrawBitmap((BitmapType *)bmp3,100,100);
        WinPaintAlphaImage(m,(BitmapType *)bmp3,alpha,0,0);
SysTaskDelay(SysTicksPerSecond() * 2);
        //now resize...
        {
            //UInt16          oldW = 225;
            //UInt16          oldH = 206;
            UInt16          newW = 0;
            UInt16          newH = 0;
            //BitmapType      *bmp;
            //BitmapTypeV3    *bmpv3 = NULL;

            newW = SysRandom(TimGetTicks()) % 320;
            SysTaskDelay(10); // need to allow the seed to change
            newH = SysRandom(TimGetTicks()) % 320;
            
            BmpResizeImage(m, (BitmapType **)&bmp3, newW, newH, kDensityDouble, &alpha);
            WinPaintAlphaImage(m, (BitmapType *)bmp3, alpha, 0, 50);

/*            bmp = BmpCreate(newW,newH,16,NULL,NULL);

//{Char temp[50]; StrPrintF(temp, "%dx%d %ld", newW, newH, newW*newH); WinDrawChars(temp, StrLen(temp), 0, 148);}

            if (bmp) {
                bmpv3 = BmpCreateBitmapV3(bmp,kDensityDouble,BmpGetBits(bmp),NULL);
                if (bmpv3) {
                    UInt16  byteWidth_bmp3  = 0;
                    UInt16  byteWidth_bmpv3 = 0;
                    unsigned char   *alpha2;

                    BmpGetDimensions((BitmapType*)bmp3, NULL, NULL, &byteWidth_bmp3);
                    BmpGetDimensions((BitmapType*)bmpv3, NULL, NULL, &byteWidth_bmpv3);

                    BmpResizeBilinear(m, BmpGetBits((BitmapType*)bmpv3),
                                    BmpGetBits((BitmapType*)bmp3),
                                    oldW, oldH, byteWidth_bmp3,
                                    newW, newH, byteWidth_bmpv3);

                    //WinDrawBitmap((BitmapType *)bmpv3,0,0);


                    alpha2 = MemGluePtrNew(newW * newH);
                    if (alpha) {
                        BmpResizeAlphaChannel(m, alpha2, alpha, oldW, oldH,
                                        newW, newH);

                        WinPaintAlphaImage(m,(BitmapType *)bmpv3,alpha2,0,50);

                        MemPtrFree(alpha2);
                    }
                    //MemPtrFree(BmpGetBits((BitmapType *)bmpv3));
                    BmpDelete((BitmapType *)bmpv3);
                }
            }*/
SysTaskDelay(SysTicksPerSecond() * 2);
        }
        
	//MemPtrFree(BmpGetBits((BitmapType *)bmp3));
	BmpDelete((BitmapType *)bmp3);
	MemPtrFree(alpha);
    }	
    MemHandleUnlock(hn);
    DmReleaseResource(hn);
}    

UInt32 PilotMain( UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
    unsigned long rom;
    void *m;
    UInt32 w,h,d;
    Boolean c;
    
    if(cmd==sysAppLaunchCmdNormalLaunch)
    {
	FtrGet(sysFtrCreator,sysFtrNumROMVersion,&rom);
	if(rom<sysMakeROMVersion(5,0,0,0,0))
    	    FrmCustomAlert(AL_DEVICE,"PalmOS 5.0 needed",NULL,NULL);
	else if(rom<0x02000000)
	{
    	    FrmCustomAlert(AL_DEVICE,"PalmOS 5.0 needed",NULL,NULL);
    	    AppLaunchWithCommand(sysFileCDefaultApp,cmd,NULL);
	}
	else
	{  
	    m=Png_Init('armc',1000);
	    if(m)
	    {	
    		WinScreenMode(winScreenModeGet,&w,&h,&d,&c);
		d=16;
		WinScreenMode(winScreenModeSet,&w,&h,&d,&c);

		ShowPng(m,Png1);
		SysTaskDelay(2*SysTicksPerSecond());
		ShowPng(m,Png2);
		SysTaskDelay(2*SysTicksPerSecond());
		ShowPng(m,Png3);
		SysTaskDelay(2*SysTicksPerSecond());
		ShowPng(m,Png4);
		SysTaskDelay(2*SysTicksPerSecond());
		ShowPng(m,Png5);
		SysTaskDelay(2*SysTicksPerSecond());
		ShowPng(m,Png6);
		SysTaskDelay(2*SysTicksPerSecond());
		Png_Close(m);
		WinScreenMode(winScreenModeSetToDefaults,&w,&h,&d,&c);
	    }	
	}   
    }    
    return(0); 
}

