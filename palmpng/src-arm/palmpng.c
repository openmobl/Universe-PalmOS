#include <PalmOS.h>
#include <Standalone.h>
#include <PceNativeCall.h>
#include "palmpng_defines.h"
#include "palmpng.h"
#include "armtypes.h"
#include "zlib.h"
#include "pealstub.h"
#include "macros.h"
#include "png.h"
#include "resize.h"

STANDALONE_CODE_RESOURCE_ID(1000);

#define save16(buffer,value)			\
{						\
    (buffer)[0]=((value)>>8)&0xFF;		\
    (buffer)[1]=(value)&0xFF;			\
}						

#define save32(buffer,value)			\
{						\
    (buffer)[0]=((value)>>24)&0xFF;		\
    (buffer)[1]=((value)>>16)&0xFF;		\
    (buffer)[2]=((value)>>8)&0xFF;		\
    (buffer)[3]=(value)&0xFF;			\
}						

#define load16(buffer)	( 						\
			    (((UInt16)(((buffer)[0])))<<8) 	| 	\
			    (((UInt16)(((buffer)[1]))))			\
			)

#define load32(buffer)	( 						\
			    (((UInt32)(((buffer)[0])))<<24) 	| 	\
			    (((UInt32)(((buffer)[1])))<<16)	| 	\
			    (((UInt32)(((buffer)[2])))<<8) 	| 	\
			    (((UInt32)(((buffer)[3]))))			\
			)


void *m68k_MemPtrNew = NULL;
void *m68k_Alert = NULL;
void *m68k_AlertNum = NULL;
void *m68k_BmpCreateBitmapV3 = NULL;
void *m68k_BmpCreate = NULL;

int malloc(const int size)
{
    long param;
    int ptr;

    param=ByteSwap32(size);
    ptr=(*gCall68KFuncP)(gEmulStateP, (unsigned long)m68k_MemPtrNew, &param, sizeof(param)+kPceNativeWantA0);
    return(ptr);
}

void free(const void *ptr)
{
    long param;

    param=ByteSwap32((int)ptr);
    (*gCall68KFuncP)(gEmulStateP, (unsigned long)PceNativeTrapNo(sysTrapMemChunkFree), &param, sizeof(param));
}    

void alert(char *txt)
{
    long param;

    param=ByteSwap32((int)txt);
    (*gCall68KFuncP)(gEmulStateP, (unsigned long)m68k_Alert, &param, sizeof(param));
}

void alertnum(char *txt, UInt32 num, UInt32 num2)
{
    struct Param
    {
	unsigned char txt[4];	//char *txt;			
	unsigned char num[4];	//UInt32 num;
    unsigned char num2[4];
    } param;

    save32(param.txt,(UInt32)txt);
    save32(param.num,num);
    save32(param.num2,num2);
    (*gCall68KFuncP)(gEmulStateP, (unsigned long)m68k_AlertNum, &param, sizeof(param));
}

void abort(void)
{
    alert("libPng error");
}


static char *pngbuff;
static int pngread;

static void LoadData(png_structp png_ptr,png_bytep data,png_size_t length)
{
    memcpy(data,pngbuff+pngread,length);
    pngread+=length;
}

static Boolean InfoPNGInt(unsigned char *png, Coord *w, Coord *h)
{
    unsigned char *pngpic;
    png_structp png_ptr; 
    png_infop info_ptr;
    char *tmp;
    unsigned long width,height;
    int col;
    int bpp;
    Boolean ok;

    ok=false;
    pngpic=NULL;
    png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if(png_ptr)
    {
	info_ptr=png_create_info_struct(png_ptr);
	if(info_ptr)
	{
	    pngbuff=png;
	    pngread=0;
	    png_set_read_fn(png_ptr,&tmp,LoadData);

	    png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_BGR|PNG_TRANSFORM_EXPAND|PNG_TRANSFORM_STRIP_16|PNG_TRANSFORM_INVERT_ALPHA,NULL);

	    png_get_IHDR(png_ptr,info_ptr,&width,&height,&bpp,&col,NULL,NULL,NULL);

	    if(w)
	        *w=width;
	    if(h)
	        *h=height;
		
	    ok=true;
	}
	png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
    }
    return(ok);
}


//png is pointer to png image
//depack it (so far only 320x320 images are supported)
//and return raw data

static unsigned char *DepackPNGInt(unsigned char *png, Coord *w, Coord *h, unsigned char **alpha)
{
    unsigned char *pngpic;
    png_structp png_ptr; 
    png_infop info_ptr;
    char *tmp;
    png_bytep *row_pointers;
    int i,j;
    unsigned long width,height;
    int col;
    int bpp;
    unsigned short r,g,b,a;
    unsigned short p;

    pngpic=NULL;
    png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if(png_ptr)
    {
	info_ptr=png_create_info_struct(png_ptr);
	if(info_ptr)
	{
	    pngbuff=png;
	    pngread=0;

	    png_set_read_fn(png_ptr,&tmp,LoadData);
	    png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_BGR|PNG_TRANSFORM_EXPAND|PNG_TRANSFORM_STRIP_16|PNG_TRANSFORM_INVERT_ALPHA,NULL);

	    png_get_IHDR(png_ptr,info_ptr,&width,&height,&bpp,&col,NULL,NULL,NULL);

	    pngpic=(unsigned char *)malloc(width*height*2l);
	    if(alpha)
	        *alpha=(unsigned char *)malloc(width*height);
		
	    if(pngpic)
	    {
	        row_pointers=png_get_rows(png_ptr,info_ptr);	

	        for(i=0;i<height;i++)
	        {
	    	    for(j=0;j<width;j++)
		    {
		        if(col==PNG_COLOR_TYPE_GRAY)
		        {
		    	    b=row_pointers[i][1*j]>>3;
			    g=row_pointers[i][1*j]>>2;
			    r=b;
			    a=0;
			}    
			else if(col==PNG_COLOR_TYPE_GRAY_ALPHA)
			{
			    b=row_pointers[i][2*j]>>3;
			    g=row_pointers[i][2*j]>>2;
			    r=b;
			    a=row_pointers[i][2*j+1]>>3;
			}    
			else if(col==PNG_COLOR_TYPE_RGB)
			{
			    b=row_pointers[i][3*j]>>3;
			    g=row_pointers[i][3*j+1]>>2;
			    r=row_pointers[i][3*j+2]>>3;
			    a=0;
			}    
			else if(col==PNG_COLOR_TYPE_RGB_ALPHA)
			{
			    b=row_pointers[i][4*j]>>3;
			    g=row_pointers[i][4*j+1]>>2;
			    r=row_pointers[i][4*j+2]>>3;
			    a=row_pointers[i][4*j+3]>>3;
		    	}    
			else
			{
			    b=0;
			    g=0;
			    r=0;
			    a=0;
			}    

			p=(r<<11)|(g<<5)|b;
		    
			pngpic[2*(i*width+j)]=p>>8;
			pngpic[2*(i*width+j)+1]=p&0xFF;
			    
			if(alpha && (*alpha))
			    (*alpha)[i*width+j]=a;
			    
			if(w)
			    *w=width;
			if(h)
			    *h=height;
		    }
		}
	    }	
	}
	png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
    }
    return(pngpic);
}

/*int DepackPNG(int png)
{
    return(DepackPNGInt((unsigned char *)png,NULL,NULL,NULL));
}*/

BitmapTypeV3 *pnoDecodePngToBitmap(DecodePngToBitmapParamsType *params)
{
    unsigned char *png;
    unsigned char **alpha;
    UInt16 density;
    BitmapType *bmp;
    BitmapTypeV3 *bmp3;
    unsigned char *bmpdata;
    Coord w,h; //Int16
    Err err;   //UInt16
    Boolean ok;

    struct BmpParam
    {
	unsigned char width[2];		//Coord width;			
	unsigned char height[2];	//Coord height;			
	unsigned char depth;		//UInt8 depth;			
	unsigned char _zarovnanie;	//UInt8 depth;			
	unsigned char colorTableP[4];	//ColorTableType *colorTableP;
	unsigned char error[4];		//UInt16 *error;
    } bmpparam;	

    struct Bmp3Param
    {
	unsigned char bitmapP[4];	//BitmapType *bitmapP;
	unsigned char density[2];	//UInt16 density;
	unsigned char bitsP[4];		//void *bitsP;
	unsigned char colorTableP[4];	//ColorTableType *colorTableP;
    } bmp3param;	

    struct BmpDeleteParam
    {
	unsigned char bitmapP[4];	//BitmapType *bitmapP;
    } bmpdeleteparam;	

    png=(unsigned char *)load32(params->png);
    alpha=(unsigned char **)load32(params->alpha);
    density=load16(params->density);

    bmp3=NULL;
    if(alpha)
    {
	*alpha=NULL;
    }	

    ok=InfoPNGInt(png,&w,&h);
    if(ok)
    {
	save16(bmpparam.width,w);
	save16(bmpparam.height,h);
	bmpparam.depth=16;
	save32(bmpparam.colorTableP,(UInt32)0);
	save32(bmpparam.error,(UInt32)(&err));

	bmp=(BitmapType *)(*gCall68KFuncP)(gEmulStateP, (unsigned long)/*m68k_BmpCreate*/PceNativeTrapNo(sysTrapBmpCreate), &bmpparam, sizeof(bmpparam)+kPceNativeWantA0);
	if(bmp)
	{    
//	    alert("mam bmp");

	    save32(bmpdeleteparam.bitmapP,(UInt32)bmp);
	    bmpdata=(unsigned char *)(*gCall68KFuncP)(gEmulStateP,(unsigned long)PceNativeTrapNo(sysTrapBmpGetBits), &bmpdeleteparam, sizeof(bmpdeleteparam)+kPceNativeWantA0);;

//	    alert("mam bmp data");

	    save32(bmp3param.bitmapP,(UInt32)bmp);
	    save16(bmp3param.density,density);
	    save32(bmp3param.bitsP,(UInt32)bmpdata);
	    save32(bmp3param.colorTableP,(UInt32)0);
	    
	    bmp3=(BitmapTypeV3 *)(*gCall68KFuncP)(gEmulStateP, (unsigned long)m68k_BmpCreateBitmapV3, &bmp3param, sizeof(bmp3param)+kPceNativeWantA0);
//	    alert("mam bmp3");

	    save32(bmpdeleteparam.bitmapP,(UInt32)bmp);
	    (*gCall68KFuncP)(gEmulStateP, (unsigned long)PceNativeTrapNo(sysTrapBmpDelete), &bmpdeleteparam, sizeof(bmpdeleteparam));
//	    alert("deletol som bmp");
		
	    bmp=(BitmapType *)bmp3;
	    bmp3=NULL;
	}    
    
	if(bmp)
	{
//	    alert("depakujem");
	    bmpdata=DepackPNGInt(png,&w,&h,alpha);
	    if(bmpdata)
	    {
//		alert("mam data");
		save32(bmp3param.bitmapP,(UInt32)bmp);
		save16(bmp3param.density,density);
		save32(bmp3param.bitsP,(UInt32)bmpdata);
		save32(bmp3param.colorTableP,(UInt32)0);
	    
//		alert("idem na bmp3");
		bmp3=(BitmapTypeV3 *)(*gCall68KFuncP)(gEmulStateP, (unsigned long)m68k_BmpCreateBitmapV3, &bmp3param, sizeof(bmp3param)+kPceNativeWantA0);
		if(!bmp3)
		{
//		    alert("bmp3 padlo");
		    free(bmpdata);
		    if(alpha && (*alpha))
			free(*alpha);
		}    
	    }
	    save32(bmpdeleteparam.bitmapP,(UInt32)bmp);
	    (*gCall68KFuncP)(gEmulStateP, (unsigned long)PceNativeTrapNo(sysTrapBmpDelete), &bmpdeleteparam, sizeof(bmpdeleteparam));
	}
    }	
    if(alpha && (*alpha))
	*alpha=(unsigned char *)ByteSwap32(*alpha);
    return(bmp3);
}

void pnoWinPaintAlphaImage(WinPaintAlphaImageParamsType *params)
{
    unsigned char 	*bitmap;
    Int16		bmp_w;
    Int16	 	bmp_h;
    unsigned char	*alpha;
    unsigned char 	*window;
    Int16		win_x;
    Int16	 	win_y;
    Int16		win_w;
    Int16	 	win_h;
    Int16		clip_x;
    Int16	 	clip_y;
    Int16		clip_w;
    Int16	 	clip_h;
    Int16		x;
    Int16		y;
    Int16		i,j;
    UInt16		fc,bc,oc;
    UInt16		c1,c2,c3;
    UInt8		tc1,tc2;
    UInt16 		t;
    Boolean 		bswap;
    
    bitmap=(unsigned char *)load32(params->bitmap);
    bmp_w=(Int16)load16(params->bmp_w);
    bmp_h=(Int16)load16(params->bmp_h);
    alpha=(unsigned char *)load32(params->alpha);
    window=(unsigned char *)load32(params->window);
    win_x=(Int16)load16(params->win_x);
    win_y=(Int16)load16(params->win_y);
    win_w=(Int16)load16(params->win_w);
    win_h=(Int16)load16(params->win_h);
    x=(Int16)load16(params->x);
    y=(Int16)load16(params->y);
    clip_x=(Int16)load16(params->clip_x);
    clip_y=(Int16)load16(params->clip_y);
    clip_w=(Int16)load16(params->clip_w);
    clip_h=(Int16)load16(params->clip_h);
    bswap=(Int16)load16(params->bswap);

//    alertnum("win_w",win_w);
//    alertnum("win_h",win_h);
//    alertnum("bmp_w",bmp_w);
//    alertnum("b,p_h",bmp_h);

    for(i=0;i<bmp_h && i+y<clip_y+clip_h;i++)
    {
	for(j=0;j<bmp_w && i+y>=clip_y && j+x<clip_x+clip_w;j++)
	{
	    if(j+x>=clip_x)
	    {
		if(alpha && alpha[i*bmp_w+j])
		{
		    t=alpha[i*bmp_w+j]+1; //so now it is 1..32
		    if(bswap)
			bc=window[2*((i+y)*win_w+j+x)+1]+((UInt16)(window[2*((i+y)*win_w+j+x)])<<8);
		    else
			bc=window[2*((i+y)*win_w+j+x)]+((UInt16)(window[2*((i+y)*win_w+j+x)+1])<<8);
		    if(t==32)
			oc=bc;
		    else
		    {
			fc=bitmap[2*(i*bmp_w+j)+1]+((UInt16)(bitmap[2*(i*bmp_w+j)])<<8);
		
		        tc1=bc&0x1F;
		        tc2=fc&0x1F;
		        c1=(tc1*t+tc2*(32-t))>>5;
		        c1&=0x1F;
    
		        tc1=(bc>>5)&0x3F;
		        tc2=(fc>>5)&0x3F;
		        c2=(tc1*t+tc2*(32-t))>>5;
		        c2&=0x3F;

		        tc1=(bc>>11)&0x1F;
		        tc2=(fc>>11)&0x1F;
		        c3=(tc1*t+tc2*(32-t))>>5;
		        c3&=0x1F;

		        oc=(c3<<11)|(c2<<5)|c1;
		    }
		}	
		else
		{
		    fc=bitmap[2*(i*bmp_w+j)+1]+((UInt16)(bitmap[2*(i*bmp_w+j)])<<8);
		    oc=fc;
		}	

		if(bswap)
		{
		    window[2*((i+y)*win_w+j+x)+1]=oc&0xFF;	
	    	    window[2*((i+y)*win_w+j+x)]=(oc>>8)&0xFF;	
		}
		else    
		{
		    window[2*((i+y)*win_w+j+x)]=oc&0xFF;	
	    	    window[2*((i+y)*win_w+j+x)+1]=(oc>>8)&0xFF;	
		}
	    }    	
	}
    }
}

void pnoBmpResizeBilinear(ResizeParamsType *params)
{
    void    *outBitmap;
    void    *inBitmap;
    Coord   inWidth;
    Coord   inHeight;
    UInt16  inByteWidth;
    Coord   outWidth;
    Coord   outHeight;
    UInt16  outByteWidth;


    outBitmap   = (void *)load32(params->outBitmap);
    inBitmap    = (void *)load32(params->inBitmap);
    inWidth     = (Coord)load16(params->inWidth);
    inHeight    = (Coord)load16(params->inHeight);
    inByteWidth = (UInt16)load16(params->inByteWidth);
    outWidth    = (Coord)load16(params->outWidth);
    outHeight   = (Coord)load16(params->outHeight);
    outByteWidth= (UInt16)load16(params->outByteWidth);
    
    DoBilinear(outBitmap, outWidth, outHeight, outByteWidth,
                inBitmap, inWidth, inHeight, inByteWidth);
    
}

void pnoBmpResizeNearestNeighbor(ResizeParamsType *params)
{
    void    *outBitmap;
    void    *inBitmap;
    Coord   inWidth;
    Coord   inHeight;
    Coord   outWidth;
    Coord   outHeight;


    outBitmap   = (void *)load32(params->outBitmap);
    inBitmap    = (void *)load32(params->inBitmap);
    inWidth     = (Coord)load16(params->inWidth);
    inHeight    = (Coord)load16(params->inHeight);
    outWidth    = (Coord)load16(params->outWidth);
    outHeight   = (Coord)load16(params->outHeight);
    
    DoNearestNeighbor(outBitmap, outWidth, outHeight,
                inBitmap, inWidth, inHeight);
    
}

void pnoBmpResizeAlphaChannel(ResizeParamsType *params)
{
    void    *outBitmap;
    void    *inBitmap;
    Coord   inWidth;
    Coord   inHeight;
    Coord   outWidth;
    Coord   outHeight;


    outBitmap   = (void *)load32(params->outBitmap);
    inBitmap    = (void *)load32(params->inBitmap);
    inWidth     = (Coord)load16(params->inWidth);
    inHeight    = (Coord)load16(params->inHeight);
    outWidth    = (Coord)load16(params->outWidth);
    outHeight   = (Coord)load16(params->outHeight);
    
    ResizeAlpha(outBitmap, outWidth, outHeight,
                inBitmap, inWidth, inHeight);
    
}

