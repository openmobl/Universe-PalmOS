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
#include	<PalmOS.h>
#include	<PalmCompatibility.h>
#include 	"wml.h"
#include 	"WAP.h"
#include 	"formBrowser.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include 	"../res/WAPUniverse_res.h"
//#include	"JpegLibPalm.h"
#include	"pnoJpeg.h"
#include 	"PalmUtils.h"
#include	"history.h"
#include	"variable.h"
#include 	"dbOffline.h"
#include	"Bitmap.h"
#include	"http.h"
#include	"Form.h"
#include	"image.h"
#include	"Layout.h"
#include	"Cache.h"
#include	"URL.h"
#include	"Hyperlinks.h"

// 

// TODO: There is a lot of stuff here that needs to be cleaned up!


#ifndef __palmos__
    #include <stdio.h>
#endif __palmos__

static char *wmltag[] = { 
    "-", 			// 05
    "-", 			// 06
    "-", 			// 07
    "-", 			// 08
    "-", 			// 09
    "-", 			// 0A
    "-", 			// 0B
    "-", 			// 0C
    "-", 			// 0D
    "-", 			// 0E
    "-", 			// 0F
    "-", 			// 10
    "-", 			// 11
    "-", 			// 12
    "-", 			// 13
    "-", 			// 14
    "-", 			// 15
    "-", 			// 16
    "-", 			// 17
    "-", 			// 18
    "-", 			// 19
    "-", 			// 1A
    "pre",			// 1B
    "a",  			// 1C
    "td", 			// 1D
    "tr", 			// 1E
    "table", 			// 1F
    "p", 			// 20
    "postfield", 		// 21
    "anchor", 			// 22
    "access", 			// 23
    "b",  			// 24
    "big", 			// 25
    "br", 			// 26
    "card", 			// 27
    "do", 			// 28
    "em", 			// 29
    "fieldset", 		// 2A
    "go", 			// 2B
    "head", 			// 2C
    "i", 			// 2D
    "img", 			// 2E
    "input", 			// 2F
    "meta", 			// 30
    "noop", 			// 31
    "prev", 			// 32
    "onevent", 			// 33
    "optgroup", 		// 34
    "option", 			// 35
    "refresh", 			// 36
    "select", 			// 37
    "small", 			// 38
    "strong",			// 39
    "-", 			// 3A
    "template",			// 3B
    "timer", 			// 3C
    "u", 			// 3D
    "setvar", 			// 3E
    "wml" 			// 3F
};

static char *attrStart[] = { 
    "accept-charset=\"", 				// 05
    "align=\"bottom\"", 				// 06
    "align=\"center\"", 				// 07
    "align=\"left\"", 					// 08
    "align=\"middle\"", 				// 09
    "align=\"right\"", 					// 0A
    "align=\"top\"", 					// 0B
    "alt=\"", 						// 0C
    "content=\"", 					// 0D
    "-", 						// 0E
    "domain=\"", 					// 0F

    "emptyok=\"false\"", 				// 10
    "emptyok=\"true\"", 				// 11
    "format=\"", 					// 12
    "height=\"", 					// 13
    "hspace=\"", 					// 14
    "ivalue=\"", 					// 15
    "iname=\"", 					// 16
    "-", 						// 17
    "label=\"", 					// 18
    "localsrc=\"", 					// 19
    "maxlength=\"", 					// 1A
    "method=\"get\"", 					// 1B
    "method=\"post\"", 					// 1C
    "mode=\"nowrap\"", 					// 1D
    "mode=\"wrap\"", 					// 1E
    "multiple=\"false\"", 				// 1F

    "multiple=\"true\"", 				// 20
    "name=\"", 						// 21
    "newcontext=\"false\"", 				// 22
    "newcontext=\"true\"", 				// 23
    "onpick=\"", 					// 24
    "onenterbackward=\"", 				// 25
    "onenterforward=\"", 				// 26
    "ontimer=\"", 					// 27
    "optimal=\"false\"", 				// 28
    "optimal=\"true\"", 				// 29
    "path=\"", 						// 2A
    "-", 						// 2B
    "-", 						// 2C
    "-", 						// 2D
    "scheme=\"", 					// 2E
    "sendreferer=\"false\"", 				// 2F

    "sendreferer=\"true\"", 				// 30
    "size=\"", 						// 31
    "src=\"", 						// 32
    "ordered=\"true\"", 				// 33
    "ordered=\"false\"", 				// 34
    "tabindex=\"", 					// 35
    "title=\"", 					// 36
    "type=\"", 						// 37
    "type=\"accept\"", 					// 38
    "type=\"delete\"", 					// 39
    "type=\"help\"", 					// 3A
    "type=\"password\"", 				// 3B
    "type=\"onpick\"", 					// 3C
    "type=\"onenterbackward\"", 			// 3D
    "type=\"onenterforward\"", 				// 3E
    "type=\"ontimer\"", 				// 3F

    "-", 						// 40
    "-", 						// 41
    "-", 						// 42
    "-", 						// 43
    "-", 						// 44
    "type=\"options\"", 				// 45
    "type=\"prev\"", 					// 46
    "type=\"reset\"", 					// 47
    "type=\"text\"", 					// 48
    "type=\"vnd.", 					// 49
    "href=\"", 						// 4A
    "href=\"http://", 					// 4B
    "href=\"https://", 					// 4C
    "value=\"", 					// 4D
    "vspace=\"", 					// 4E
    "width=\"", 					// 4F

    "xml:lang=\"", 					// 50
    "-", 						// 51
    "align=\"", 					// 52
    "columns=\"", 					// 53
    "class=\"", 					// 54
    "id=\"", 						// 55
    "forua=\"false\"", 					// 56
    "forua=\"true\"", 					// 57
    "src=\"http://", 					// 58
    "src=\"https://", 					// 59
    "http-equiv=\"", 					// 5A
    "http-equiv=\"Content-Type\"", 			// 5B
    "content=\"application/vnd.wap.wmlc;charset=", 	// 5C
    "http-equiv=\"Expires\"", 				// 5D
    "accesskey=\"", 					// 5E
    "enctype=\"", 					// 5F
    "enctype=\"application/x-www-form-urlencoded\"", 	// 60
    "enctype=\"multipart/form-data\"", 			// 61
};



static char *attrVal[] = { 
    ".com/", 			// 85
    ".edu/", 			// 86
    ".net/", 			// 87
    ".org/", 			// 88
    "accept", 			// 89
    "bottom", 			// 8A
    "clear", 			// 8B
    "delete", 			// 8C
    "help", 			// 8D
    "http://", 			// 8E
    "http://www.", 		// 8F

    "https://", 		// 90
    "https://www.", 		// 91
    "?", 			// 92
    "middle", 			// 93
    "nowrap", 			// 94
    "onpick", 			// 95
    "onenterbackward", 		// 96
    "onenterforward", 		// 97
    "ontimer", 			// 98
    "options", 			// 99
    "password", 		// 9A
    "reset", 			// 9B
    "?", 			// 9C
    "text", 			// 9D
    "top", 			// 9E
    "unknown", 			// 9F
    "wrap", 			// A0
    "www.", 			// A1
};

Boolean needTimer = false;
Boolean isLink = false;
Boolean isAnchor = false;
Boolean postStart = false;
Boolean onenterforward = false;
Boolean onenterbackward = false;
Boolean onevent = false;
Boolean isRefresh = false;
Boolean isOption = false;
Boolean isOnpickLink = false;
Boolean isOnenterRefresh = false;
Boolean	isOnforwardPost = false;
int	off = 0;

char *getAttributes(WMLDeckPtr wmldck, unsigned char attr, GlobalsType *g);

#ifdef __palmos__
#define underline 		palmUnderline
#define bold 			palmBold
#define strong 			palmStrong
#define big 			palmBig
#define small 			palmSmall
#define italic 			palmItalic
#define em 			palmEmphasis
#define pre 			palmPre
#define wmlInput 		palmWmlInput
#define wmlInputPassword 	palmWmlInputPassword
#define wmlInputValue 		palmWmlInputValue
#define getPos 			palmGetPos
#define wmlSelectOptionTxt 	palmWmlSelectOptionTxt
#define wmlSelect 		palmWmlSelect 
#define wmlSelectValue 		palmWmlSelectValue
#define wmlSelectOption 	palmWmlSelectOption
#define wmlSelectEnd 		palmWmlSelectEnd
#endif __palmos__


int pushwmlstack( WMLDeckPtr wmldck)
{
    unsigned char *ptr;
    int i;

    if (wmldck->stack == NULL) {
        wmldck->maxstack = 10;
        wmldck->stack=Malloc(wmldck->maxstack);
	    ErrFatalDisplayIf (!wmldck->stack, "Malloc(stack)");
    }
    if (wmldck->stackDepth >= wmldck->maxstack) {   	
        i = wmldck->maxstack;
        wmldck->maxstack+=10;
        ptr = wmldck->stack;
        wmldck->stack=Malloc(wmldck->maxstack);
	    ErrFatalDisplayIf (!wmldck->stack, "Malloc(stack-resize)");
        while (i>0)
           wmldck->stack[--i]=ptr[i];
        Free(ptr);
    }
    wmldck->stack[wmldck->stackDepth++] = wmldck->tag;

    return(0);    
}

void  freewmlstack(WMLDeckPtr wmldck)
{
    wmldck->stackDepth=0;
    if (wmldck->stack != NULL){
        Free(wmldck->stack);
        wmldck->stack = NULL;
    }
     
}

unsigned char popwmlstack(WMLDeckPtr wmldck)
{
    unsigned char c;

    if (wmldck->stack == NULL)
        return(-1);
    wmldck->stackDepth--; 
    c = wmldck->stack[wmldck->stackDepth];        
    if (wmldck->stackDepth == 0) {
        Free(wmldck->stack);
        wmldck->stack = NULL;
    }
    return(c);    
}

void renderLn(GlobalsType *g)
{
    browserResizeDimensions(0, (g->x == 0) ? FntCharHeight() : 0, g);

#ifndef __palmos__	
    fprintf(stdout,"\n");
#else
    palmprintln(g);
#endif __palmos__

}

void renderBr(GlobalsType *g)
{
    browserResizeDimensions(0, (g->x == 0) ? FntCharHeight() : 0, g);

#ifndef __palmos__	
    fprintf(stdout,"\n");
#else
    palminsertbr(g);
#endif __palmos__

}


void renderString(WMLDeckPtr wmldck, char *string, GlobalsType *g)
{
    if (wmldck->action == ACTION_RENDER)
#ifndef __palmos__	
        fprintf(stdout,string);
#else
        palmprintf(string,g);
#endif __palmos__

}


void drawDo(char *label,  GlobalsType *g)
{
	HyperlinkSetLink(true, NULL, NULL, NULL, g);

	palmprintf((label)?label:"Untitled", g);
}

void srcString(WMLDeckPtr wmldck, char *string, GlobalsType *g)
{
    if (wmldck->action == ACTION_SRC)
#ifndef __palmos__	
        fprintf(stdout,string);
#else
        palmsrcprintf(string,g);
#endif __palmos__

}

void srcIndent(WMLDeckPtr wmldck, GlobalsType *g)
{
    int i;

    if (wmldck->action == ACTION_SRC)
        for (i=0;i<wmldck->stackDepth;i++)
            srcString(wmldck," ",g);

}

void srcTag(WMLDeckPtr wmldck, GlobalsType *g)
{
    if ((wmldck->tag >=5) && (wmldck->tag <= 0x3F )) {
        srcString(wmldck,"<",g);  
        srcString(wmldck,wmltag[wmldck->tag-5],g);    
    }
}


void srcAttr(WMLDeckPtr wmldck, unsigned char attr_id, char *val,GlobalsType *g)
{
    if ((attr_id >=5) && (attr_id <= 0x5F )) {
        srcString(wmldck," ",g);  
        srcString(wmldck,attrStart[attr_id-05],g);    
        if (val[0]!=0) { // TODO: Need to close empty strings, i.e. value=""
            srcString(wmldck,val,g);
            srcString(wmldck,"\"",g);
        }

    }
}

Char *WmlConvertEncoding(Char *instr, WMLDeckPtr wmldck)
{
	Char *str = instr;

	switch (wmldck->charSet) {
		case UTF_8:
			str = ConvertFromTo(charEncodingUTF8, instr, str);
			break;
		/*case US_ASCII:
			str = ConvertFromTo(charEncodingAscii, instr, str);
			break;
		case ISO_8859_1:
			str = ConvertFromTo(charEncodingISO8859_1, instr, str);
			break;*/
		case USC_2:
			str = ConvertFromTo(charEncodingUCS2, instr, str);
			break;
		default:
			str = ConvertFromTo(charEncodingPalmLatin, instr, str);
			break;
	}

	return str;
}

void noAccess(WMLDeckPtr wmldck, GlobalsType *g)
{
	FormInputDeactivate(g);
	wmldck->action = 0;
	wbxml_free_wmldck(&(g->wmldck));
	browserSetTitle("WAPUniverse"); // SetFieldFromStr2("WAPUniverse",fieldTitle);
	g->progress= 0;
    	showProgress(g);
	g->state = BS_IDLE;

	// maybe call a back event, or at least pop the history

	FrmAlert(alNoAccess);

	return;
}

//This is not accurate because if you goto a page that sets access to its domain, get denied than refresh you get access.
// i.e. ths scenario:
//  Bob launches http://www.nodomain.com/
//  He then follows a link to http://secure.nodomain.com which checks for the refering url to be secure.nodomain.com.
//  It does not match so he is denied.
//  Bob presses refresh, allowing him into the page http://secure.nodomain.com/
//
//  THIS SHOULDN'T HAPPEN!!!!!
//
//  maybe call a back event, or at least pop the history
// 
Boolean checkAccess(Char *attr, unsigned char *url, int type)
{
unsigned char *accessUrl;
int i = 0, s = 0;
Boolean stop = false;

	if (type == 1) { // Extract Domain
		i = 7;
		accessUrl = Malloc(StrLen(url));
		if (!accessUrl)
			return false;
		while ( (stop == false) && (i < StrLen(url)) ){
			if ( (url[i] == '/') && (i > 8)) {
				stop = true;
			} else {
				accessUrl[i-7]=url[i++];
			}
		}
		if( StrNCaselessCompare(accessUrl,attr,StrLen(attr)) == 0 )
		{
			Free(accessUrl);
			return false;
		} else {
			Free(accessUrl);
		}
	} else if (type == 2) { // Extract Path
		i = 6;
		while ( (stop == false) && (i < StrLen(url)) ){
			if ( (url[i] == '/') && (i > 8)) {
				stop = true;
				s = i;
			} else {
				i++;
			}
		}
		if( StrNCaselessCompare(url+s,attr,StrLen(attr)) == 0 )
		{
			return false;
		}
	}

	return true;
}

int checkCards(int startoffset, WMLDeckPtr wmldck, GlobalsType *g)
{
    int			off;
    CardPtr		tmp,idx;
    Char		*cptr;
    RGBColorType	color;  
    IndexedColorType	indexedColor;
    Char		*attr;
    int			u = 0;
    Boolean		gotDomain = false;
    Boolean		noaccess = false;

	//For DO tags
	color.r   = ( UInt8 ) 0;//Set the amount of Red to use (0 - 255)
	color.g   = ( UInt8 ) 0;//Set the amount of Green to use (0 - 255)
	color.b   = ( UInt8 ) 210;//255;//Set the amount of Blue to use (0 - 255)
	indexedColor  = WinRGBToIndex( &color );

    
	switch (wmldck->tag) {
		case WML_card :
			off = wmldck->offset;
			tmp = Malloc(sizeof(CardType));
			ErrFatalDisplayIf (!tmp, "Malloc Failed");
			tmp->offset = wmldck->offset;
			tmp->next = NULL;
			cptr = getAttributes(wmldck,WML_id,g);
			if (cptr){
				tmp->card_id = Malloc(StrLen(cptr)+1);
				ErrFatalDisplayIf (!tmp->card_id, "Malloc2 Failed");
				StrCopy(tmp->card_id,cptr);
			} else {
				tmp->card_id = NULL;
			}
			if ( wmldck->card == NULL) {
				wmldck->card = tmp;
			} else {
				idx = wmldck->card;
				while (idx->next != NULL) {
		   			idx = idx->next;
				}
				idx->next = tmp;
			}						
			wmldck->offset = off;
			getAttributes(wmldck,END,g);
			break;  

		case WML_template : 
			wmldck->template = 1;
			attr = getAttributes(wmldck,END,g);
			break;
		case WML_access://TODO: better access support (see note above)
			if (g->History->index > 0) {
				u = g->History->index;
				u--;
			}
			attr = getAttributes(wmldck,WML_domain,g);
			if (attr){
				if (g->History->index > 0) {
					noaccess = checkAccess(attr, g->History->hist[u].url, 1);
					if (noaccess == true) {
						noAccess(wmldck, g);
						return(-15);
					}
				} else {
					noAccess(wmldck, g);
					return(-15);
				}
				gotDomain = true;
			}
			wmldck->offset=startoffset;
			attr = getAttributes(wmldck,WML_path,g);
			if (attr){
				if (gotDomain == true) {
					if (g->History->index > 0) {
						noaccess = checkAccess(attr, g->History->hist[u].url, 2);
						if (noaccess == true) {
							noAccess(wmldck, g);
							return(-15);
						}
					} else {
						noAccess(wmldck, g);
						return(-15);
					}
				} else {
					noAccess(wmldck, g);
					return(-15);
				}
			}
			wmldck->offset=startoffset;
			attr = getAttributes(wmldck,END,g);
			break;
     		default:
        		getAttributes(wmldck,END,g);
    	}
	return(0);      
}


int getAttributeValue(WMLDeckPtr wmldck, char attrstr[])
{
    unsigned char c;
    int l,j;

            attrstr[0] = 0;
            j=0;
            c = wmldck->data[wmldck->offset++];
            while ((c > 128) || (c==ENTITY) || (c == STR_I) || (c == STR_T)   
                   || (c==SWITCH_PAGE) || (c==PI) 
                   || ((c >= EXT_I_0) && (c <= EXT_I_2))
                   || ((c >= EXT_T_0) && (c <= EXT_T_2))) {

                switch (c) {
                case ENTITY:
                    WspReadInt(wmldck->data,&(wmldck->offset)); 
                    //DisplayError("Unhandled WML ENTITY");
					attrstr[j++]='<';
					attrstr[j++]='!';
					attrstr[j++]='>';
                    attrstr[j] = 0;
                    break;

                case STR_I:
                case EXT_I_0:
                case EXT_I_1:
                case EXT_I_2:
		    if (c != STR_I)
		    	attrstr[j++]=c;
                    while (wmldck->data[wmldck->offset] != 0) {
                        attrstr[j++]=wmldck->data[wmldck->offset++];
                    }
		    if (c != STR_I)
			attrstr[j++]=END;
                    attrstr[j] = 0;
		    if (c == STR_I) // Apply this to variables too?
			attrstr = WmlConvertEncoding(attrstr, wmldck);
                    wmldck->offset++;   
                    break;

                case PI: // processing instruction
                    //DisplayError("Unhandled WML PI");                
                    break; 

                case EXT_T_0:
                case EXT_T_1:
                case EXT_T_2: // 
                    l = WspReadInt(wmldck->data,&(wmldck->offset));
		    attrstr[j++] = c - 0x40;
		    /*switch(c) {
			case EXT_T_0:
				break;
			case EXT_T_1:
				break;
			case EXT_T_2:
				break;
		    }*/

                    while (wmldck->strTable[l] !=0)
                        attrstr[j++] = wmldck->strTable[l++];
                    //attrstr[j]=0;
                    //MyErrorFunc(attrstr+1,"var");              
		    attrstr[j++]=END;
                    attrstr[j] = 0;
                    // !! TODO : handle escape/noescape
		    if (c==EXT_T_1)
			;//escaped
		    if (c==EXT_T_2)
			;//unescaped
                    break;
                case STR_T: 
                    l = WspReadInt(wmldck->data,&(wmldck->offset));
                    while (wmldck->strTable[l] !=0)
                        attrstr[j++] = wmldck->strTable[l++];                   
                    attrstr[j] = 0;
		    attrstr = WmlConvertEncoding(attrstr, wmldck);
                    break;  
                case EXT_0:
                case EXT_1:
                case EXT_2:
                    //Single byte extension tokens		
                    l = wmldck->data[wmldck->offset++];
                    break;
                case OPAQUE: 
                    //DisplayError("UNHANDELD OPAQUE");
                    break;

                default:
                    if ((c >=0x85) && (c <= 0xA1 )) {
                        l=0;
                        while (attrVal[c-0x85][l]!=0)
                            attrstr[j++] = attrVal[c-0x85][l++];                    
                        attrstr[j] = 0;
                    } else {
                        //DisplayError("UNKNOWN ATTRIBUTE");//handle unknowns silently 
                    }
                } //switch	
                c = wmldck->data[wmldck->offset++];
            } //while
        wmldck->offset--;
		return(0);
}

char *getAttributes(WMLDeckPtr wmldck, unsigned char attr, GlobalsType *g)
{
//    	static Char 	attrstr[128];
//	char 		tmpstr[128];
    	unsigned char 	c, attr_id;

	wmldck->attrstr[0]=0;
    	if (wmldck->data[wmldck->offset] & WBXML_has_attributes) {
        	wmldck->offset++;
        	c= wmldck->data[wmldck->offset++];
        	while (c != END) {
            		attr_id = c;
				if (attr_id == attr){
   			   		getAttributeValue(wmldck,wmldck->attrstr);
			   		return(wmldck->attrstr);
				} else {
   			   		getAttributeValue(wmldck,wmldck->tmpstr);
				}
            		if (wmldck->action == ACTION_SRC)
                		srcAttr(wmldck, attr_id,wmldck->tmpstr,g);
            		c = wmldck->data[wmldck->offset++];
        	}
        	wmldck->offset--;
    	}
    	return(NULL);
}

int readElement(WMLDeckPtr wmldck, GlobalsType *g)
{
    int			startoffset;
    unsigned char	c;
    Char 		*attr;
    Char 		*name;
    Char 		*alt;
    Char 		*val;
    Char 		*src;
    //char		accessUrl[MAX_URL_LEN+1];
    //int		accessInt = 0;
    //Int16  		linkLen;
    RGBColorType       	color;  
    IndexedColorType   	indexedColor;
    int 		ret = 0;
    Boolean		haveAlt = false;
    UInt16		width = 0, height = 0;
    Int16		size;
    Int32		maxLength;

    //For DO tags
    color.r   = ( UInt8 ) 0;
    color.g   = ( UInt8 ) 0;
    color.b   = ( UInt8 ) 210;
    indexedColor  = WinRGBToIndex( &color );

    startoffset = wmldck->offset;
    wmldck->tag = wmldck->data[wmldck->offset] & WBXML_tag_id;
    c = wmldck->data[wmldck->offset];

    if (wmldck->action == ACTION_RENDER) showProgress(g);


    if ((wmldck->tag >=5) && (wmldck->tag <= 0x3F )) {
        switch (wmldck->action) {
        case ACTION_RENDER :
		switch(wmldck->tag) {
			case WML_template:
				//wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_meta:
				if ( (attr = getAttributes(wmldck,WML_http_equiv,g))) {
					if (StrNCaselessCompare(attr, "cache-control", 13)==0) {
						if ( (attr = getAttributes(wmldck,WML_content,g))) {
							if (StrNCaselessCompare(attr, "no-cache", 8)==0 ||
							    StrNCaselessCompare(attr, "no-store", 8)==0) {
								g->cache.cacheDoNotRead = true;
								CacheDeleteEntry(g->Url, false, g);
							}
						}
					}
				}
				break;
    			case WML_card :
        			attr = getAttributes(wmldck,WML_title,g);
				if (attr){
					browserSetTitle(WmlConvertEncoding(attr,wmldck)); // SetFieldFromStr2(attr,fieldTitle);
	        			//if (g->deckTitle)
	        			//	Free(g->deckTitle);
	        			g->deckTitle = Malloc(StrLen(attr)+1);
	        			ErrFatalDisplayIf (!g->deckTitle, "Malloc Failed");
					StrCopy(g->deckTitle,attr);
	        		} else {
					browserSetTitle("WAPUniverse"); // SetFieldFromStr2("WAPUniverse",fieldTitle);
	        			if (g->deckTitle)
	        				Free(g->deckTitle);
	        			g->deckTitle = Malloc(StrLen("Untitled")+1);
	        			ErrFatalDisplayIf (!g->deckTitle, "Malloc Failed");
					StrCopy(g->deckTitle,"Untitled");
	        		}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,WML_ontimer,g);
				if (attr){
					if (wmldck->ontimerUrl)
						Free(wmldck->ontimerUrl);
					wmldck->ontimerUrl = Malloc(StrLen(attr)+1);
					ErrFatalDisplayIf (!wmldck->ontimerUrl, "Malloc Failed");
					StrCopy(wmldck->ontimerUrl,attr);						
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,WML_newcontext_true,g);
				if (attr){
					disposeVar(g);
					while (historyPop(g) != NULL);	
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,WML_onenterforward,g);
				if (attr){
					followLink(attr, g);
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_postfield:
				if (g->hasEvent) {
					if (g->post) {
						UInt32 buflen = (g->postBuf != NULL) ? StrLen(g->postBuf) : 0;

						isOnforwardPost = true;

						//AddToBuf(&g->postBuf, &buflen, ((g->postBuf != NULL) ? "&" : "\0"), StrLen((g->postBuf != NULL) ? "&" : "\0"));
						if (g->postBuf) AddToBuf(&g->postBuf, &buflen, "&", StrLen("&"));
						else { g->postBuf = Malloc(1024); }

						wmldck->offset=startoffset;
						name = expandVar(getAttributes(wmldck,WML_name,g),g);
						AppendEncoded(&g->postBuf, &buflen, name);
						Free(name);

						AddToBuf(&g->postBuf, &buflen, "=", StrLen("="));

						wmldck->offset=startoffset;
						val = expandVar(getAttributes(wmldck,WML_value,g),g);
						AppendEncoded(&g->postBuf, &buflen, val);
						Free(val);
					} else {
						UInt32 buflen = StrLen(g->onforward);

						AddToBuf(&g->onforward, &buflen, (!StrChr(g->onforward, '?')) ? "?" : "&", 
							StrLen((!StrChr(g->onforward, '?')) ? "?" : "&"));

						wmldck->offset=startoffset;
						name = expandVar(getAttributes(wmldck,WML_name,g),g);
						AppendEncoded(&g->onforward, &buflen, name);
						Free(name);

						AddToBuf(&g->onforward, &buflen, "=", StrLen("="));

						wmldck->offset=startoffset;
						val = expandVar(getAttributes(wmldck,WML_value,g),g);
						AppendEncoded(&g->onforward, &buflen, val);
						Free(val);
					}
                		} else if ((isLink == true) && !g->hasEvent) { 
                        		name = Malloc(200); //I highly doubt a variable name will be longer than 200 characters
						ErrFatalDisplayIf (!name, "Malloc Failed");
		    			StrCopy(name, getAttributes(wmldck,WML_name,g));
                    			if (name) {
                        			wmldck->offset=startoffset;
                        			val = getAttributes(wmldck,WML_value,g);
                        			hyperlinkAddPostVar(name,g);
                        			hyperlinkSetPostVar(name,val,g);
						Free(name);
                    			}
				} else if ((isOnpickLink == true) && !g->hasEvent) {
                        		name = Malloc(200); //I highly doubt a variable name will be longer than 200 characters
						ErrFatalDisplayIf (!name, "Malloc Failed");
		    			StrCopy(name, getAttributes(wmldck,WML_name,g));
                    			if (name) {
                        			wmldck->offset=startoffset;
                        			val = getAttributes(wmldck,WML_value,g);
                        			optionAddPostVar(name,g);
                        			optionSetPostVar(name,val,g);
						Free(name);
                    			}
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_onevent:
				onevent = true;
				isOnforwardPost = false;
				attr = getAttributes(wmldck,WML_type_ontimer,g);
				if (attr){
					g->onEvent = ONTIMER;
					needTimer = true;
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,WML_type_onenterforward,g);
				if (attr){
					if ((g->onenterback == false) && (g->refreshEvent == false)) {
						g->onEvent = ONENTERFORWARD;
						onenterforward = true;
						g->hasEvent = true;
					} else {
						g->onEvent = IGNORE;
						onenterforward = false;
					}
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,WML_type_onenterback,g);
				if (attr){
					if ((g->onenterback == true) && (g->refreshEvent == false)) {
						g->onEvent = ONENTERBACKWARD;
						onenterbackward = true;
						g->hasEvent = true;
					} else {
						g->onEvent = IGNORE;
						onenterbackward = false;
					}
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,WML_type_onpick,g);
				if (attr){
					g->onEvent = ONPICK;
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_setvar:
                		if (g->hasEvent) {		//((onenterforward == true) || (g->onEvent == 2)) || ((onenterbackward == true) || (g->onEvent == 4))) {
                    			//wmldck->offset=startoffset;
					name = Malloc(200); //I highly doubt a variable name will be longer than 200 characters
						ErrFatalDisplayIf (!name, "Malloc Failed");
		    			StrCopy(name, getAttributes(wmldck,WML_name,g));
                    			if (name) {
                        			wmldck->offset=startoffset;
                        			val = getAttributes(wmldck,WML_value,g);
                        			addVar(name,g);
                        			setVar(name,val,g);
						Free(name);
                    			}
                		} else if (((isLink == true) || (isRefresh == true)) && !g->hasEvent) { 	//(g->onEvent == 0) && (onevent == false)) {
                        		name = Malloc(200); //I highly doubt a variable name will be longer than 200 characters
						ErrFatalDisplayIf (!name, "Malloc Failed");
		    			StrCopy(name, getAttributes(wmldck,WML_name,g));
                    			if (name) {
                        			wmldck->offset=startoffset;
                        			val = getAttributes(wmldck,WML_value,g);
                        			hyperlinkAddVar(name,g);
                        			hyperlinkSetVar(name,val,g);
						Free(name);
                    			}
				} else if ((isOnpickLink == true) && !g->hasEvent) { 	//(g->onEvent == 0) && (onevent == false)) {
                        		name = Malloc(200); //I highly doubt a variable name will be longer than 200 characters
						ErrFatalDisplayIf (!name, "Malloc Failed");
		    			StrCopy(name, getAttributes(wmldck,WML_name,g));
                    			if (name) {
                        			wmldck->offset=startoffset;
                        			val = getAttributes(wmldck,WML_value,g);
                        			optionAddVar(name,g);
                        			optionSetVar(name,val,g);
						Free(name);
                    			}
				}
                		wmldck->offset=startoffset;
                		attr = getAttributes(wmldck,END,g);
                		break;
			case WML_timer:
				attr = getAttributes(wmldck,WML_value,g);
				if (attr){
						wmldck->timerVal=StrAToI(attr);
				}else{
                        		DisplayError("Timer but no Value!"); 				
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;		
			case WML_br:
				off = 0;
				if (g->Render == true) renderBr(g);
	            		attr = getAttributes(wmldck,END,g);
				break;
			case WML_pre:
				pre(true);
			case WML_p:
				wmldck->offset=startoffset;
				
				if ((g->x > 0) || (g->imgH)) renderLn(g);

				if (getAttributes(wmldck,WML_align_center,g)){
					addNewAlignmnet(Center, g);
					goto render;
				} 
				wmldck->offset=startoffset;
				if (getAttributes(wmldck,WML_align_right,g)) {
					addNewAlignmnet(Right, g);
					goto render;
				}
				addNewAlignmnet(Left, g);
			render:
				
				/*switch (g->Align) {
					case Center:
						g->x = 152 / 2;
						break;
					case Right:
						g->x = 152;
						break;
					case Left:
					default:
    						g->x = 0;
						break;
    				}*/

				g->x = 0;

				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,WML_src,g);
				if (attr) {
					width = 0; height = 0;
					ImageCreateImage(attr, &width, &height, false, NULL,g);
				}
				else {
					wmldck->offset=startoffset;
					attr = getAttributes(wmldck,WML_src_http,g);
					if (attr) {
						src = Malloc(StrLen(attr)+StrLen("http://")+1);
						if (src) {
							StrPrintF (src,"http://%s",attr);
							width = 0; height = 0;
							ImageCreateImage(src, &width, &height, false, NULL, g);
							Free(src);
						}
					} else {
						wmldck->offset=startoffset;
						attr = getAttributes(wmldck,WML_src_https,g);
						if (attr) {
							src = Malloc(StrLen(attr)+StrLen("https://")+1);
							if (src) {
								StrPrintF (src,"https://%s",attr);
								width = 0; height = 0;
								ImageCreateImage(src, &width, &height, false, NULL, g);
								Free(src);
							}
						}
					}
				}
				wmldck->offset=startoffset;

	            		attr = getAttributes(wmldck,END,g);
				break;
			case WML_img:
				haveAlt = false;
				attr = getAttributes(wmldck,WML_localsrc,g);
				if (attr){
				//we have a localsrc, img should be blank
				} else {
					wmldck->offset=startoffset;
					attr = getAttributes(wmldck,WML_src,g);
					if (attr) {
						width = 0; height = 0;
						if ( ImageCreateImage(attr, &width, &height, false, NULL/*alt*/, g) != true ) {
							haveAlt = true;
						}
					}
					else {
						wmldck->offset=startoffset;
						attr = getAttributes(wmldck,WML_src_http,g);
						if (attr) {
							src = Malloc(StrLen(attr)+StrLen("http://")+1);
							if (src) {
								StrPrintF (src,"http://%s",attr);
								width = 0; height = 0;
								if ( ImageCreateImage(src, &width, &height, false, NULL/*alt*/, g) != true ) {
									haveAlt = true;
								}
							Free(src);
							} else {
								haveAlt = true;
							}
						} else {
							wmldck->offset=startoffset;
							attr = getAttributes(wmldck,WML_src_https,g);
							if (attr) {
								src = Malloc(StrLen(attr)+StrLen("https://")+1);
								if (src) {
									StrPrintF (src,"https://%s",attr);
									width = 0; height = 0;
									if ( ImageCreateImage(src, &width, &height, false, NULL/*alt*/, g) != true ) {
										haveAlt = true;
									}
								Free(src);
								} else {
									haveAlt = true;
								}
							} else {
								haveAlt = true;
							}
						}
					}
				}

				wmldck->offset=startoffset;
				
				if (haveAlt == true) {
					wmldck->offset=startoffset;
					alt = getAttributes(wmldck,WML_alt,g);
					if (alt) {
						ImageCreateEmptyImage(alt, &g->x, &g->y, &width, &height, false, g);
					} else {
						ImageCreateEmptyImage(NULL, &g->x, &g->y, &width, &height, false, g);
					}
				}
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_u:
				if (wmldck->data[wmldck->offset] & 0x40) {
					//g->needSpace=1; // not needed now?
					underline(true);
				}
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_em:
				if (wmldck->data[wmldck->offset] & 0x40) {
					//g->needSpace=1; // not needed now?
					em(true);
				}
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_b:
				if (wmldck->data[wmldck->offset] & 0x40) {
					//g->needSpace=1; // not needed now?
					bold(true);
				}
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_strong:
				if (wmldck->data[wmldck->offset] & 0x40) {
					//g->needSpace=1; // not needed now?
					strong(true);
				}
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_big:
				if (wmldck->data[wmldck->offset] & 0x40) {
					//g->needSpace=1; // not needed now?
					big(true);
				}
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_small:
				if (wmldck->data[wmldck->offset] & 0x40) {
					//g->needSpace=1; // not needed now?
					small(true);
				}
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_i:
				if (wmldck->data[wmldck->offset] & 0x40) {
					//g->needSpace=1; // not needed now?
					italic(true);
				}
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_prev:
				if (isLink == true){
					HyperlinkSetLink(false, "", "<PREV/>", NULL, g);
				}
				//if (g->hasEvent) { 		//((onenterforward == true) || (g->onEvent == 2)) || ((onenterbackward == true) || (g->onEvent == 4))){
				switch (g->onEvent) {
					case ONTIMER: // Not sure if this will ever happen??
					    	if (wmldck->ontimerUrl)
							Free(wmldck->ontimerUrl);
						wmldck->ontimerUrl = Malloc(StrLen("<PREV/>")+1);
						ErrFatalDisplayIf (!wmldck->ontimerUrl, "Malloc Failed");
						StrCopy(wmldck->ontimerUrl,"<PREV/>");
						g->onEvent = NOEVENT;
						break;
					case ONENTERFORWARD:
					case ONENTERBACKWARD:
						if (g->onforward) Free(g->onforward);
						g->onforward = Malloc(StrLen("<PREV/>")+1);
							ErrFatalDisplayIf (!g->onforward, "Malloc Failed");
                				StrCopy(g->onforward, "<PREV/>");
						break;
					case ONPICK:
						palmWmlSelectOptionOnPick("", "<PREV/>", g);
						isOnpickLink = true;
						break;
					case IGNORE:
					case NOEVENT:
					default:
						break;
				}
				//}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_refresh:
				if (isLink == true){
					HyperlinkSetLink(false, "", "<REFRESH/>", NULL, g);
				}
				switch (g->onEvent) {
					case ONTIMER: // Not sure if this will ever happen??
						g->onEvent = NOEVENT;
						break;
					case ONENTERFORWARD:
					case ONENTERBACKWARD:
						//isRefresh = true;
						isOnenterRefresh = true;
						break;
					case ONPICK:
						palmWmlSelectOptionOnPick("", "<REFRESH/>", g);
						isOnpickLink = true;
						break;
					case IGNORE:
					case NOEVENT:
					default:
						if (onevent) break;
						isRefresh = true;
						break;
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_do: //TODO: Cleanup,  see bottom of case


				attr = getAttributes(wmldck,WML_label,g);
				if (attr){ // TODO: should look for "prev" type??
					//hyperLink(true,NULL,NULL,g);
					//g->needSpace=1;
					isLink = true;
					drawDo (attr, g);
				} else {
					wmldck->offset=startoffset;
					attr = getAttributes(wmldck,WML_title,g);
					if (attr){
						//hyperLink(true,NULL,NULL,g);
						//g->needSpace=1;
						isLink = true;
						drawDo (attr, g);
					} else {
						wmldck->offset=startoffset;
						attr = getAttributes(wmldck,WML_name,g);
						if (attr){
							//hyperLink(true,NULL,NULL,g);
							//g->needSpace=1;
							isLink = true;
							drawDo (attr, g);
						} else {
							//hyperLink(true,NULL,NULL,g);
							//g->needSpace=1;
							isLink = true;

							wmldck->offset=startoffset;
							attr = getAttributes(wmldck,WML_type,g);
							if (attr){
								drawDo(attr, g);
								goto bottom;
							}
							wmldck->offset=startoffset;
							attr = getAttributes(wmldck,WML_type_options,g);
							if (attr){
								drawDo("Options", g);
								goto bottom;
							}
							wmldck->offset=startoffset;
							attr = getAttributes(wmldck,WML_type_accept,g);
							if (attr){
								drawDo("Accept", g);
								goto bottom;
							}
							wmldck->offset=startoffset;
							attr = getAttributes(wmldck,WML_type_prev,g);
							if (attr){
								drawDo("Prev", g);
								goto bottom;
							}
							wmldck->offset=startoffset;
							attr = getAttributes(wmldck,WML_type_help,g);
							if (attr){
								drawDo("Help", g);
								goto bottom;
							}
							wmldck->offset=startoffset;
							attr = getAttributes(wmldck,WML_type_reset,g);
							if (attr){
								drawDo("Reset", g);
								goto bottom;
							}
							wmldck->offset=startoffset;
							attr = getAttributes(wmldck,WML_type_delete,g);
							if (attr){
								drawDo("Delete", g);
								goto bottom;
							}

							drawDo("Unknown", g);
						bottom:
						}
					}
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);			
				break;				
			case WML_anchor:
				g->needSpace=1;
				isLink = true;
				isAnchor = true;
				
				HyperlinkSetLink(true, NULL, NULL, NULL, g);

				wmldck->offset=startoffset;
				if ( (attr = getAttributes(wmldck,WML_accesskey,g))){
				//This will come soon as we start to implement WML 1.2
				//There will be a function like: setAccessKey(unsigned char key, char URL);
					//palmprintf("[", g);
					//palmprintf(attr, g);
					//palmprintf("]", g);
					//palmprintf(": ", g);
					//setAccessKey(attr, g->HyperLinks->url);
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_noop:
				//Do not do anything. This is a "NO" "OP"eration.//Well, this is really not nothing
				if (isLink == true){
					HyperlinkSetLink(false, "", "<NOOP/>", NULL, g);
				}
				//if (g->hasEvent) {		//((onenterforward == true) || (g->onEvent == 2)) || ((g->onenterback == true) || (g->onEvent == 4))) {
				switch (g->onEvent) {
					case ONTIMER: // Not sure if this will ever happen??
					    	if (wmldck->ontimerUrl)
							Free(wmldck->ontimerUrl);
						wmldck->timerVal = -1;
						g->onEvent = NOEVENT;
						break;
					case ONENTERFORWARD:
					case ONENTERBACKWARD:
						if (g->onforward) Free(g->onforward);
						g->hasEvent = false;
						break;
					case ONPICK:
						//palmWmlSelectOptionOnPick("", "<NOOP/>", g); // I do not think this exists
						break;
					case IGNORE:
					case NOEVENT:
					default:
						break;
				}
				//}
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_go:
				//isLink = true;
				if ((attr = getAttributes(wmldck,WML_method_post,g))){
		        		g->post = true;
					if (g->onEvent == ONPICK)
						palmWmlSelectOptionPost(true, g);
					else
		        			HyperlinkSetPost(true, g);
		        		postStart = true;
				} else {
					wmldck->offset=startoffset;
					if ((attr = getAttributes(wmldck,WML_method_get,g))){
			        		g->post = false;
					} else {
						g->post = false;
					}
				}

				wmldck->offset=startoffset;
				if ((attr = getAttributes(wmldck,WML_href_http,g))){
					switch (g->onEvent) {
						case ONTIMER:
							if (wmldck->ontimerUrl)
								Free(wmldck->ontimerUrl);
							wmldck->ontimerUrl = Malloc(StrLen(attr)+StrLen("http://")+1);
							ErrFatalDisplayIf (!wmldck->ontimerUrl, "Malloc Failed");
							StrCopy(wmldck->ontimerUrl,"http://");
							StrCopy(wmldck->ontimerUrl+StrLen("http://"),attr);
							g->onEvent = NOEVENT;
							break;
						case ONENTERFORWARD:
						case ONENTERBACKWARD:
							if (g->onforward) Free(g->onforward);
							g->onforward = Malloc(StrLen(attr)+StrLen("http://")+1);
								ErrFatalDisplayIf (!g->onforward, "Malloc Failed");
                					StrCopy(g->onforward, "http://");
                					StrCopy(g->onforward+StrLen("http://"), attr);
							break;
						case ONPICK:
							palmWmlSelectOptionOnPick("http://", attr, g);
							isOnpickLink = true;
							break;
						case IGNORE:
							break;
						case NOEVENT:
						default:
							if (onevent) break;
							HyperlinkSetLink(false, "http://", attr, NULL, g);
							isLink = true;
							break;
					}
				} else {
					wmldck->offset=startoffset;
	 				if ((attr = getAttributes(wmldck,WML_href,g))){
						switch (g->onEvent) {
							case ONTIMER:
				    				if (wmldck->ontimerUrl)
									Free(wmldck->ontimerUrl);
								wmldck->ontimerUrl = Malloc(StrLen(attr)+1);
								ErrFatalDisplayIf (!wmldck->ontimerUrl, "Malloc Failed");
								StrCopy(wmldck->ontimerUrl,attr);
								g->onEvent = NOEVENT;
								break;
							case ONENTERFORWARD:
							case ONENTERBACKWARD:
								if (g->onforward) Free(g->onforward);
								g->onforward = Malloc(StrLen(attr)+1);
									ErrFatalDisplayIf (!g->onforward, "Malloc Failed");
								StrCopy(g->onforward, attr);
								break;
							case ONPICK:
								palmWmlSelectOptionOnPick("", attr, g);
								isOnpickLink = true;
								break;
							case IGNORE:
								break;
							case NOEVENT:
							default:
								if (onevent) break;
								HyperlinkSetLink(false, "", attr, NULL, g);
								isLink = true;
								break;
						}	
					} else {
						wmldck->offset=startoffset;
	 				    	if ((attr = getAttributes(wmldck,WML_href_https,g))){
							switch (g->onEvent) {
								case ONTIMER:
					    				if (wmldck->ontimerUrl)
										Free(wmldck->ontimerUrl);
									wmldck->ontimerUrl = Malloc(StrLen(attr)+StrLen("https://")+1);
									ErrFatalDisplayIf (!wmldck->ontimerUrl, "Malloc Failed");
									StrCopy(wmldck->ontimerUrl,"https://");
									StrCopy(wmldck->ontimerUrl+StrLen("https://"),attr);
									g->onEvent = NOEVENT;
									break;
								case ONENTERFORWARD:
								case ONENTERBACKWARD:
									if (g->onforward) Free(g->onforward);
									g->onforward = Malloc(StrLen(attr)+StrLen("https://")+1);
										ErrFatalDisplayIf (!g->onforward, "Malloc Failed");
                							StrCopy(g->onforward, "https://");
                							StrCopy(g->onforward+StrLen("https://"), attr);
									break;
								case ONPICK:
									palmWmlSelectOptionOnPick("https://", attr, g);
									isOnpickLink = true;
									break;
								case IGNORE:
									break;
								case NOEVENT:
								default:
									if (onevent) break;
									HyperlinkSetLink(false, "https://", attr, NULL, g);
									isLink = true;
									break;
							}
						}
					}
				}
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_a:	
				g->needSpace=1;
				isLink = true;
				if ( (attr = getAttributes(wmldck,WML_href_http,g))){
			    		HyperlinkSetLink(true, "http://", attr, NULL, g);
				} else {
					wmldck->offset=startoffset;
	 				if ( (attr = getAttributes(wmldck,WML_href,g))){
				    		HyperlinkSetLink(true, "", attr, NULL, g);				
					} else {
						wmldck->offset=startoffset;
	 				    if ( (attr = getAttributes(wmldck,WML_href_https,g))){
				    	    	HyperlinkSetLink(true, "https://", attr, NULL, g);				
					    }
					}
				}
				wmldck->offset=startoffset;
				/*if ( (attr = getAttributes(wmldck,WML_accesskey,g))){
				//This will come soon as we start to implement WML 1.2/1.3
				//There will be a function like: setAccessKey(unsigned char key, char URL);
					palmprintf("[", g);
					palmprintf(attr, g);
					palmprintf("]", g);
					palmprintf(": ", g);
					setAccessKey(attr, g->HyperLinks->url);
				}*/
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_input:
				size = -1;
				maxLength = -1;

				if ((attr = getAttributes(wmldck,WML_maxlength,g))){
					maxLength = StrAToI(attr);
				}
				wmldck->offset=startoffset;
				if ((attr = getAttributes(wmldck,WML_size,g))){
					size = (Int16)StrAToI(attr);
				}

				wmldck->offset=startoffset;
				if ((attr = getAttributes(wmldck,WML_name,g))){	
	                		wmlInput(attr,false,size,maxLength,g);
	                		wmldck->offset=startoffset;
					if ( (attr = getAttributes(wmldck,WML_type_password,g))){	
		                		palmWmlInputType(true, g);
					} 							
					else {
						wmldck->offset=startoffset;
		                		palmWmlInputType(false, g);
					}	
				}
				wmldck->offset=startoffset;
				if ( (attr = getAttributes(wmldck,WML_value,g))){
					//wmlInputValue(attr,g);

					if (g->Input->name) {
						if (InputgetVar(g->Input->name, g) != NULL) {
							wmlInputValue(getVar(g->Input->name,g),g);
						} else {
	                				wmlInputValue(attr,g);
						}
					} else {
	                			wmlInputValue(attr,g);
					}

				} else {
					wmldck->offset=startoffset;
					if ((attr = getAttributes(wmldck,WML_name,g))) {
						wmlInputValue(getVar(attr,g),g);
					}
				}						
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);
			    	break;
			case WML_select:
				if ( (attr = getAttributes(wmldck,WML_name,g))){
	                		wmlSelect(attr,false,g);
				} else {
					wmldck->offset=startoffset;
					wmlSelect("na",false,g); 
					// This is not a good way to do this. Some sites
					// such as www.nytimes.com/wap use only <select>. We need to handle it
					// but if we get more than one titleless select we might be in trouble.
				}						
				wmldck->offset=startoffset;
				if ( (attr = getAttributes(wmldck,WML_ivalue,g))){
	                		wmlSelectValue(attr,g);
				} 							
				wmldck->offset=startoffset;
				if ( (attr = getAttributes(wmldck,WML_value,g))){
	                		//wmlSelectValue(attr,g);

					if (g->Select->name) {
						if (InputgetVar(g->Select->name, g) != NULL) {
							wmlSelectValue(getVar(g->Select->name,g),g);
						} else {
	                				wmlSelectValue(attr,g);
						}
					} else {
	                			wmlSelectValue(attr,g);
					}
				} else {
					wmldck->offset=startoffset;
					if ((attr = getAttributes(wmldck,WML_name,g))) {
						wmlSelectValue(getVar(attr,g),g);
					}
				}						
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);			
				break;
			case WML_option:
				g->Render=false;
				isOption = true;
				if ((attr = getAttributes(wmldck,WML_value,g))){
	                		wmlSelectOption(attr,g);
				} else {
					wmldck->offset=startoffset;
					wmlSelectOption("na",g);
				}
				wmldck->offset=startoffset;
				if ((attr = getAttributes(wmldck,WML_onpick,g))){
	                		//wmlSelectOption(attr,g);
	                		palmWmlSelectOptionOnPick("", attr, g);
				}
				wmldck->offset=startoffset;
				if ((attr = getAttributes(wmldck,WML_title,g))) {
					wmlSelectOptionTxt(attr,g);
				}					
				wmldck->offset=startoffset;
				attr = getAttributes(wmldck,END,g);							
				break;		
			case WML_td://TODO: Add table support, this will look very crappy if alignment is not left!
				g->needSpace=2;
				attr = getAttributes(wmldck,END,g);
				break;
			case WML_tr://TODO: Add table support
				renderLn(g);
				attr = getAttributes(wmldck,END,g);
				break;					
			default:
				attr = getAttributes(wmldck,END,g);
			}
            break;

        case ACTION_SRC :
            srcTag(wmldck,g);                   
            getAttributes(wmldck,END,g);
            break;
        case ACTION_PARSE :
            ret = checkCards(startoffset,wmldck,g);
	    if (ret == (-15))
		return(-15);
            break;
        default :
            getAttributes(wmldck,END,g);

        }       
    }
    // check for content
    if (c & WBXML_has_content) {
        pushwmlstack(wmldck);
        srcString(wmldck,">\n",g);
        srcIndent(wmldck,g);
    } else {
        srcString(wmldck,"/>\n",g);
        srcIndent(wmldck,g);
    }
    return(0);
}


int wbxml_parse_header(WebDataPtr webData, WMLDeckPtr wmldck)
{
    int j;
    int strTableSz;
    GlobalsType *g;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    wmldck->stackDepth = 0;
    wmldck->stack = NULL;
    wmldck->offset = 0;
    wmldck->ontimerUrl = NULL;
    wmldck->data = webData->data;
    wmldck->version = wmldck->data[wmldck->offset++];
    wmldck->publicID = wmldck->data[wmldck->offset++];
    if (wmldck->publicID==0)
        wmldck->offset++;

    // get the CharSet
    wmldck->charSet = wmldck->data[wmldck->offset++]; 
    g->charSet = wmldck->charSet;
    // get the StringTable size, allocate memory and load it.
    strTableSz = WspReadInt(wmldck->data,&(wmldck->offset));
    wmldck->strTable=Malloc(strTableSz+1);
	wmldck->attrstr=Malloc(TMP_BLOCK_SIZE);
	wmldck->tmpstr=Malloc(TMP_BLOCK_SIZE);
    if ((!wmldck->strTable)||(!wmldck->attrstr) || (!wmldck->tmpstr) ) {
        ErrDisplay("Malloc");
        return(WML_ERR_MEM);
    }
    for (j=0;j<strTableSz;j++)
        wmldck->strTable[j] = wmldck->data[wmldck->offset++];
    wmldck->strTable[j]= 0;
    wmldck->decksize = webData->length - wmldck->offset;
    wmldck->data = (webData->data)+wmldck->offset;
    wmldck->offset = 0;

    g->xhtml.pageBG = UIColorGetTableEntryIndex(UIFieldBackground);
    freeAlignment(g);
    freeFont(g);
    return(0);
}


int wbxml_parse(WebDataPtr webData, int action, WMLDeckPtr wmldck, int offset, GlobalsType *g)
{
    unsigned char	*temp,c;
    int			j;
    RGBColorType	color;  
    IndexedColorType	indexedColor;
    int			ret = 0;
	
	//For DO tags
	color.r   = ( UInt8 ) 0;//Set the amount of Red to use (0 - 255)
	color.g   = ( UInt8 ) 0;//Set the amount of Green to use (0 - 255)
	color.b   = ( UInt8 ) 210;//255;//Set the amount of Blue to use (0 - 255)
	indexedColor  = WinRGBToIndex( &color );


    if (webData->data==NULL) {
        return -1;
    }
    temp = Malloc(TMP_BLOCK_SIZE); // was DEF_RECEIVE_BUF
    ErrFatalDisplayIf (!temp, "Malloc Failed");
    wbxml_parse_header(webData, wmldck);

    palmRegular(true);

    off = 0;

 
    wmldck->action = action;
    if (wmldck->template == 1)
    	wmldck->offset = 0;
    else
	wmldck->offset = offset;


restart:

    if (wmldck->action == ACTION_SRC) {
	srcString(wmldck,"<?xml version=\"1.0\"?>\n",g);
	srcString(wmldck,"<!DOCTYPE wml PUBLIC ",g);
	switch (wmldck->publicID) {
		case 0x02: // WML 1.0
			srcString(wmldck,"\"-//WAPFORUM//DTD WML 1.0//EN\"\n",g);
			srcString(wmldck," \"http://www.wapforum.org/DTD/wml_1.0.xml\">\n",g);
			break;
		case 0x04: // WML 1.1
			srcString(wmldck,"\"-//WAPFORUM//DTD WML 1.1//EN\"\n",g);
			srcString(wmldck," \"http://www.wapforum.org/DTD/wml_1.1.xml\">\n",g);
			break;
		case 0x08: // Channel 1.1
			srcString(wmldck,"\"-//WAPFORUM//DTD CHANNEL 1.1//EN >\"\n",g);
			//srcString(wmldck," \"http://www.wapforum.org/DTD/channel_1.1.xml\">\n",g);
			break;
		case 0x09: // WML 1.2
			srcString(wmldck,"\"-//WAPFORUM//DTD WML 1.2//EN\"\n",g);
			srcString(wmldck," \"http://www.wapforum.org/DTD/wml_1.2.xml\">\n",g);
			break;
		case 0x0A: // WML 1.3
			srcString(wmldck,"\"-//WAPFORUM//DTD WML 1.3//EN\"\n",g);
			srcString(wmldck," \"http://www.wapforum.org/DTD/wml_1.3.xml\">\n",g);
			break;
		case 0x0D: // Channel 1.2
			srcString(wmldck,"\"-//WAPFORUM//DTD CHANNEL 1.2//EN >\"\n",g);
			//srcString(wmldck," \"http://www.wapforum.org/DTD/channel_1.2.xml\">\n",g);
			break;
		default:
			srcString(wmldck,"\"-//WAPFORUM//DTD WML 1.1//EN\"\n",g);
			srcString(wmldck," \"http://www.wapforum.org/DTD/wml_1.1.xml\">\n",g);
			break;
	}
    }

    // Parse the WBXML bytecode
    while (wmldck->offset < wmldck->decksize) {

	showProgress(g);

        switch (wmldck->data[wmldck->offset]) {
        case SWITCH_PAGE :
            //DisplayError("SWITCH_PAGE WMLC");
            break;
        case END:
	    off = 0;
            c= popwmlstack(wmldck);
            if (wmldck->action== ACTION_SRC) {
                srcString(wmldck,"\n",g);
                srcIndent(wmldck,g);
                srcString(wmldck,"</",g);
                srcString(wmldck,wmltag[c-5],g);
                srcString(wmldck,">",g);
            }
	   if (wmldck->action== ACTION_RENDER) {
			switch(c) {
				case WML_option:
	                		if (temp) { 
						temp = WmlConvertEncoding(temp, wmldck); 
						wmlSelectOptionTxt(temp,g); 
					}
					g->Render = true;
					isOption = false;
					isOnpickLink = false;
					break;
				case WML_select:
					wmlSelectEnd(g);
					g->Render=true;
					isOption = false;
					break;
				case WML_p:
                    			/*switch (g->Align) {
						case Center:
							if(g->x != (152 / 2)) renderLn(g);
							break;
						case Right:
							if(g->x != 152) renderLn(g);
							break;
						case Left:
						default:
							if(g->x > 0) renderLn(g);
							break;
					}*/

					if(g->x > 0) renderLn(g);

					removeLastAlignment(g); // according to WAP specs I shouldn't have this, but...
                    			g->x = 0;
					g->y += 5;
					break;
				case WML_u:
					underline(false);
					break;
				case WML_strong:
					strong(false);
					break;
				case WML_big:
					big(false);
					break;
				case WML_small:
					small(false);
					break;
				case WML_i:
					italic(false);
					break;
				case WML_em:
					em(false);
					break;
				case WML_b:
					bold(false);
					break;
				case WML_pre:
					pre(false);
					break;
				case WML_img:
					break;
				case WML_tr:
				case WML_td:
					break;
				case WML_onevent:
					g->onEvent = NOEVENT;
					needTimer = false;
					onenterforward = false;
					onenterbackward = false;
					onevent = false;
					isOnpickLink = false;
					if (isOnenterRefresh)
						g->hasEvent = false;
					isOnenterRefresh = false;
					isOnforwardPost = false;
					break;
				case WML_go:
					if (isOnforwardPost != true) g->post = false;
					if (isLink == true) {
						HyperlinkSetLink(false, NULL, NULL, NULL, g);
						g->needSpace=1;
					}
					isLink = false;
					isOnpickLink = false;
					break;
				case WML_refresh:
					isRefresh = false;
				case WML_noop:
				case WML_prev:
					if (isLink == true) {
						HyperlinkSetLink(false, NULL, NULL, NULL, g);
						g->needSpace=1;
					}
					isLink = false;
					break;
				case WML_do:
					isLink = false;
					HyperlinkSetLink(false, NULL, NULL, NULL, g);
					g->x += 3;
					break;
				case WML_anchor:
					isAnchor = false;
				case WML_a:	
					isLink = false;
					HyperlinkSetLink(false, NULL, NULL, NULL, g);
					g->needSpace=1;
					break;
				case WML_template:
					wmldck->offset = offset;
					goto restart;
					break;
				case WML_card:
					if (wmldck->template == 1)
						wmldck->offset = wmldck->decksize;
					break;
				default:
					if (isOption == true) {
						wmlSelectOptionTxt("- null -",g);
						g->Render=true;
						isOption = false;
					}
					break;
				}
			}
            if (wmldck->stackDepth == 0){  // end of <WML>, needed because angelOne seems to sent some trailing garbage.
                freewmlstack(wmldck);
				Free(temp);
                return(0);
            }
            break;
        case ENTITY:  
		    //DisplayError("ENTITY WMLC");
            wmldck->offset++;
            j = WspReadInt(wmldck->data,&(wmldck->offset)); // UCS-4 character code
			temp[0]=j; temp[1]=0;
			// TODO : should map UCS-4 to something printable ??						
            if (wmldck->action== ACTION_SRC)
                srcString(wmldck,temp,g);
            if ((wmldck->action== ACTION_RENDER)&& (g->Render==true)){
                renderString(wmldck,temp,g);}
            wmldck->offset--;
            break;
        case STR_I: //Inline String follows
            wmldck->offset++;
            j=0;
	    if (g->Render==true) off = 0;
            while (wmldck->data[wmldck->offset] != 0) {
                temp[(j++) + off]=wmldck->data[wmldck->offset++];   
            }
            temp[j + off] = 0;
	    off += j;
            if (wmldck->action== ACTION_SRC)
                srcString(wmldck,temp,g);
            if ((wmldck->action== ACTION_RENDER)&& (g->Render==true)){
                renderString(wmldck,temp,g);}
            break;
        case EXT_I_0://variable
	    if (g->Render==true) off = 0;
       	    for(j = 0; wmldck->data[++wmldck->offset]; j++)
                temp[j + off] = wmldck->data[wmldck->offset];   
            temp[j + off] = 0;
	    off += j;
            if (wmldck->action== ACTION_SRC) {
            	srcString(wmldck,"$(",g);
                srcString(wmldck,temp,g);
                srcString(wmldck,":esc)",g);
            }
            if ((wmldck->action== ACTION_RENDER) && (g->Render==true)){
		Char *newvar = NULL;
                //renderString(wmldck,temp,g);
		addVar(temp,g);
		newvar = escaped(getVar(temp,g));
		renderString(wmldck,newvar,g);
		if (newvar) Free(newvar);
	    }
            break;
        case EXT_I_1://variable
	    if (g->Render==true) off = 0;
       	    for(j = 0; wmldck->data[++wmldck->offset]; j++)
                temp[j + off] = wmldck->data[wmldck->offset];   
            temp[j + off] = 0;
	    off += j;
            if (wmldck->action== ACTION_SRC) {
            	srcString(wmldck,"$(",g);
                srcString(wmldck,temp,g);
                srcString(wmldck,":unesc)",g);
            }
            if ((wmldck->action== ACTION_RENDER) && (g->Render==true)){
		Char *newvar = NULL;
                //renderString(wmldck,temp,g);
		addVar(temp,g);
		newvar = unescaped(getVar(temp,g));
		renderString(wmldck,newvar,g);
		if (newvar) Free(newvar);
	    }
            break;
        case EXT_I_2://variable
	    if (g->Render==true) off = 0;
       	    for(j = 0; wmldck->data[++wmldck->offset]; j++)
                temp[j + off] = wmldck->data[wmldck->offset];   
            temp[j + off] = 0;
	    off += j;
            if (wmldck->action== ACTION_SRC) {
            	srcString(wmldck,"$(",g);
                srcString(wmldck,temp,g);
                srcString(wmldck,")",g);
            }
            if ((wmldck->action== ACTION_RENDER) && (g->Render==true)){
                //renderString(wmldck,temp,g);
		addVar(temp,g);
		renderString(wmldck,getVar(temp,g),g);
	    }
        	break;
        case EXT_T_0://variable
        	wmldck->offset++;
	    if (g->Render==true) off = 0;
            j = WspReadInt(wmldck->data,&(wmldck->offset));
            StrCopy(temp + off,wmldck->strTable+j);
	    off = StrLen(temp);
            if (wmldck->action== ACTION_SRC) {
            	srcString(wmldck,"$(",g);
                srcString(wmldck,temp,g);
                srcString(wmldck,":esc)",g);
            }
            if ((wmldck->action== ACTION_RENDER) && (g->Render==true)){
		Char *newvar = NULL;
                //renderString(wmldck,temp,g);
		addVar(temp,g);
		newvar = escaped(getVar(temp,g));
		renderString(wmldck,newvar,g);
		if (newvar) Free(newvar);
	    }
            wmldck->offset--;
            break;
        case EXT_T_1://variable
        	wmldck->offset++;
	    if (g->Render==true) off = 0;
            j = WspReadInt(wmldck->data,&(wmldck->offset));
            StrCopy(temp + off,wmldck->strTable+j);
	    off = StrLen(temp);
            if (wmldck->action== ACTION_SRC) {
            	srcString(wmldck,"$(",g);
                srcString(wmldck,temp,g);
                srcString(wmldck,":unesc)",g);
            }
            if ((wmldck->action== ACTION_RENDER) && (g->Render==true)){
		Char *newvar = NULL;
                //renderString(wmldck,temp,g);
		addVar(temp,g);
		newvar = unescaped(getVar(temp,g));
		renderString(wmldck,newvar,g);
		if (newvar) Free(newvar);
	    }
            wmldck->offset--;
            break;
        case EXT_T_2://variable
        	wmldck->offset++;
	    if (g->Render==true) off = 0;
            j = WspReadInt(wmldck->data,&(wmldck->offset));
            StrCopy(temp + off,wmldck->strTable+j);
	    off = StrLen(temp);
            if (wmldck->action== ACTION_SRC) {
            	srcString(wmldck,"$(",g);
                srcString(wmldck,temp,g);
                srcString(wmldck,")",g);
            }
            if ((wmldck->action== ACTION_RENDER) && (g->Render==true)){
                //renderString(wmldck,temp,g);
		addVar(temp,g);
		renderString(wmldck,getVar(temp,g),g);
	    } 
            wmldck->offset--;
        	break;
        case EXT_0:
        case EXT_1:
        case EXT_2:
        case OPAQUE:
            //DisplayError("WML OPAQUE Extension in WMLC");
            break;
        case PI:
            //DisplayError("PI not supported in WMLC");
            break;
        case STR_T: 
            wmldck->offset++;
	    if (g->Render==true) off = 0;
            j = WspReadInt(wmldck->data,&(wmldck->offset));
			StrCopy(temp + off,wmldck->strTable+j);
	    off = StrLen(temp);
            if (wmldck->action== ACTION_SRC)
                srcString(wmldck,temp,g);
            if ((wmldck->action== ACTION_RENDER) && (g->Render==true)){
                renderString(wmldck,temp,g);}
            wmldck->offset--;
            break;
        default:
	    off = 0;
            ret = readElement(wmldck,g);
	    if (ret == (-15))
		return(-15);
	    if (ret == (-16))
		return(-16);
	    break;
        }
        wmldck->offset++;
    }

    freewmlstack(wmldck);
	Free(temp);

    return(0);
}

int wbxml_free_wmldck(WMLDeckPtr wmldck)
{
	CardPtr tmp;

	if (wmldck->attrstr)
        	Free(wmldck->attrstr);
    	wmldck->attrstr = NULL;
    	if (wmldck->tmpstr)
      		Free(wmldck->tmpstr);
    	wmldck->tmpstr = NULL;
    	if (wmldck->strTable)
        	Free(wmldck->strTable);
    	wmldck->strTable = NULL;
    	if (wmldck->stack)
        	Free(wmldck->stack);
    	wmldck->stack = NULL;
    	if (wmldck->ontimerUrl)
        	Free(wmldck->ontimerUrl);
   	wmldck->ontimerUrl = NULL;
    	while (wmldck->card) {
		tmp = wmldck->card;
		wmldck->card = tmp->next;

		if (tmp->card_id) {
			Free(tmp->card_id);
		}

		Free(tmp);
    	}
    	wmldck->template = 0;
    	wmldck->charSet = 0;
	
   	return(0);
}


