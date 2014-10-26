#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <png.h>

#define false 	0
#define true 	1

static unsigned char *pngbuff;
static int pngread;

static void LoadData(png_structp png_ptr,png_bytep data,png_size_t length)
{
    memcpy(data,pngbuff+pngread,length);
    pngread+=length;
}

static int filed;

static void SaveData(png_structp png_ptr,png_bytep data,png_size_t length)
{
    write(filed,data,length);
}

static void FlushData(png_structp png_ptr)
{
//    flush(filed);
}

/*
static short InfoPNGInt(unsigned char *png, short *w, short *h)
{
    unsigned char *pngpic;
    png_structp png_ptr; 
    png_infop info_ptr;
    png_infop end_info;
    char *tmp;
    unsigned long width,height;
    int col;
    int bpp;
    short ok;

    ok=false;
    pngpic=NULL;
    png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if(png_ptr)
    {
	end_info=NULL;
	info_ptr=png_create_info_struct(png_ptr);
	if(info_ptr)
	{
	    end_info=png_create_info_struct(png_ptr);
	    if(end_info)
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
	}
	png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
    }
    return(ok);
}
*/

//png is pointer to png image
//depack it (so far only 320x320 images are supported)
//and return raw data

static unsigned char *DecompressPng(unsigned char *png, short *w, short *h, unsigned char **alpha)
{
    unsigned char *pngpic;
    png_structp png_ptr; 
    png_infop info_ptr;
    png_infop end_info;
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
	end_info=NULL;
	info_ptr=png_create_info_struct(png_ptr);
	if(info_ptr)
	{
	    end_info=png_create_info_struct(png_ptr);
	    if(end_info)
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
			        r=row_pointers[i][1*j]>>3;
			        a=0;
			    }    
			    else if(col==PNG_COLOR_TYPE_GRAY_ALPHA)
			    {
			        b=row_pointers[i][2*j]>>3;
			        g=row_pointers[i][2*j]>>2;
			        r=row_pointers[i][2*j]>>3;
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
	}
	png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
    }
    return(pngpic);
}

static int CompressPng(int fd, unsigned char *image, short w, short h, unsigned char *alpha)
{
    unsigned char *pngpic;
    png_structp png_ptr; 
    png_infop info_ptr;
    png_infop end_info;
    int i,j;
    unsigned short r,g,b;
    unsigned short p;

    pngpic=NULL;
    png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if(png_ptr)
    {
	end_info=NULL;
	info_ptr=png_create_info_struct(png_ptr);
	if(info_ptr)
	{
	    end_info=png_create_info_struct(png_ptr);
	    if(end_info)
	    {
		filed=fd;
	    
		png_set_write_fn(png_ptr,&fd,SaveData,FlushData);
		png_set_filter(png_ptr,0,PNG_ALL_FILTERS);
		png_set_compression_level(png_ptr,Z_BEST_COMPRESSION);
		png_set_IHDR(png_ptr,info_ptr,w,h,8,PNG_COLOR_TYPE_RGB_ALPHA,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);

		png_write_info(png_ptr,info_ptr);
		png_set_invert_alpha(png_ptr);
		png_set_packing(png_ptr);
		png_set_bgr(png_ptr);

		pngpic=(unsigned char *)malloc(w*4);
		for(i=0;i<h;i++)
		{
		    for(j=0;j<w;j++)
		    {
			p=256*image[2*(w*i+j)]+image[2*(w*i+j)+1];
			r=(p>>11)&31;
			g=(p>>5)&63;
			b=p&31;
			
			pngpic[4*j+0]=b<<3;
			pngpic[4*j+1]=g<<2;
			pngpic[4*j+2]=r<<3;
			if(alpha)
			    pngpic[4*j+3]=alpha[i*w+j]<<3;
			else    
			    pngpic[4*j+3]=0;
		    }
		    png_write_row(png_ptr,pngpic);
	    	}
		free(pngpic);
		
		png_write_end(png_ptr,info_ptr);
	    }	
	}
	png_destroy_write_struct(&png_ptr,&info_ptr);
    }
    return(0);
}

int main (int argc, char *argv[])
{
    int i;
    int fd;
    unsigned char *png;
    struct stat buffer;
    int osize;
    short width,height;
    unsigned char *image;
    unsigned char *alpha;
    char temp[1024];    

    for(i=1;i<argc;i++)
    {
	printf("...processing: %s\n",argv[i]);
	fd=open(argv[i],O_RDONLY);
	if(fd!=-1)
	{
	    fstat(fd,&buffer);
	    osize=buffer.st_size;
	    image=NULL;
	    alpha=NULL;
	    width=0;
	    height=0;
	    png=(unsigned char *)mmap(0,osize,PROT_READ,MAP_PRIVATE,fd,0);
	    if(png)
	    {
		image=DecompressPng(png,&width,&height,&alpha);	    
	    	munmap(png,osize);
	    }
	    close(fd);
	    
	    if(image)
	    {
		sprintf(temp,"%s",argv[i]);
		filed=open(temp,O_WRONLY|O_CREAT|O_TRUNC);
		if(filed!=-1)
		    CompressPng(filed,image,width,height,alpha);
		close(filed);
		free(image);
		if(alpha)
		    free(alpha);
	    }		
	}    
    }	
    return(0);
}
