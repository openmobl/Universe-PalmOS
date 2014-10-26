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
#include	"xhtml.h"
#include	"../res/WAPUniverse_res.h"
#include	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"variable.h"
#include	"history.h"
#include	"Form.h"
#include	"PalmUtils.h"
#include	"Entity.h"
#include	"http.h"
#include	"color.h"
#include	"process.h"
#include	"image.h"
#include	"Layout.h"
#include	"Cache.h"
#include	"URL.h"

#include 	"libhtmlparse.h"

#ifdef __palmos__
#define underline 		palmUnderline
#define bold 			palmBold
#define strong 			palmStrong
#define big 			palmBig
#define small 			palmSmall
#define italic 			palmItalic
#define em 			palmEmphasis
#define pre 			palmPre
#define regular			palmRegular
#define	superscript		palmSuperscript
#define subscript		palmSubscript
#define strike			palmStrike
#define hyperLink 		palmHyperLink
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


#define Compare(x,y,z) ((StrNCaselessCompare(x,y,z) == 0) && !(StrLen(x) > z)) ? true : false
#define CompareJustTxt(x,y,z) (StrNCaselessCompare(x,y,z) == 0) ? true : false

void xhtml_renderLn(GlobalsType *g)
{
	browserResizeDimensions(0, (g->x == 0) ? FntCharHeight() : 0, g);

    	if (g->Render == true)
		palmprintln(g);
}

void xhtml_renderBr(GlobalsType *g)
{
	browserResizeDimensions(0, (g->x == 0) ? FntCharHeight() : 0, g);

    	if (g->Render == true)
		palminsertbr(g);
}


void xhtml_renderString(XHTMLPtr xhtml, char *string, GlobalsType *g)
{
    	if ((xhtml->action == ACTION_RENDER) && (g->Render == true) /*&& (xhtml->end != true)*/)
        	palmprintf(string,g);
}

Char *getXHTMLAttrValue(Char *arg, struct ArgvTable *args, int numargs) {
	int 		i = 0;
	GlobalsType 	*g;
	Char		*entity = NULL;
	Char		*varString = NULL;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	for(i=0; i < numargs; i++) {
		if (Compare(arg, args[i].arg, StrLen(arg))) {
			//return (topTag == TT_WML) ? (convertToVarString(EntityScanString(args[i].val, g))) : (EntityScanString(args[i].val, g)); // args[i].val;
			
			if (!args[i].val) {
				return StrDup(args[i].arg); // something like <options ... selected> spit back out
			}

			if (topTag == TT_WML) {
				entity = EntityScanString(args[i].val, g);
				if (!entity)
					return NULL;
				varString = convertToVarString(entity);
				Free(entity);
				return varString;
			} else {
				return EntityScanString(args[i].val, g);
			}
		}
	}
	return NULL;
}

void xhtml_freeAttr(Char *attr) {
	if (attr)
		Free(attr);
}

Char *XHTMLConvertEncoding(Char *instr, GlobalsType *g)
{
	Char *str = instr;

    	if ((g->webData.data != NULL) && (g->charSet != 0)) { // ((g->charSet != 0) || (g->wmldck.charSet != 0))) {
    		switch (g->charSet) { // switch ((g->wmldck.charSet) ? g->wmldck.charSet:g->charSet) {
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
    	}


	return str;
}


int textstart()
{

	return 0;
}

/*int texts (char *text) {
        GlobalsType 	*g;
	Char		*ent = NULL;
	Boolean		whiteSpace = false;
	int 		i = 0, j = 0, w = 0;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if ((g->xhtml.end == true) || (isScript == true)) return 0;

	if (isPre == false) {
		while(StrNCompare(text, "\r\n", 2) == 0) { text++; text++; } // or text += 2;

		while (*text == ' ' || *text == '	' || *text == '\t' || *text == 0x09) { // should check for '\t' ??
			if (*text == ' ') whiteSpace = true;
			else whiteSpace = false;
			text++;
		}
		if (whiteSpace == true && g->xhtml.paragraph == true) text--;

		if (*text == '\0') return 0;
	}

	ent = EntityScanString(text, g);

	if (g->xhtml.buffer != NULL) Free(g->xhtml.buffer);
	g->xhtml.buffer = Malloc(StrLen(ent) + 1);
	ErrFatalDisplayIf(!g->xhtml.buffer, "Malloc failed");

	g->xhtml.textAvail = true;

	if (isPre == false) {
		while (i < StrLen(ent) && ent[i] != '\0') {
			if (ent[i] != '\r' && ent[i] != '\n' && ent[i] != '\t' && ent[i] != 0x09) {

				if (ent[i] == ' ') {
					if (w == 0) { StrCopy(g->xhtml.buffer+j, ent+i); j++; }
					w++;
				} else {
					w = 0;
					StrCopy(g->xhtml.buffer+j, ent+i);
					j++;
				}
			}

			i++;
		}
	} else if (isPre == true) {
		while (i < StrLen(ent) && ent[i] != '\0') {
			if (ent[i] != '\r') {
				StrCopy(g->xhtml.buffer+j, ent+i);
				j++;
			}

			i++;
		}
	}

	Free(ent);

        return 0;
}*/

int texts (char *text)
{
        GlobalsType 	*g;
	Char		*ent = NULL;
	int 		i = 0, j = 0, w = 0;
	//Boolean		gotTxt = false;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if ((g->xhtml.end == true) || (isScript == true)) return 0;

	// handled below
	//if (StrStr(text, "\r\n ") && isPre != true && isTextarea != true) {
	//	while (*text == '\r' || *text == '\n' || *text == ' ')
	//		text++;
	//}

	if (text == NULL)
		return 0;

	ent = EntityScanString(text, g);

	if (g->xhtml.buffer != NULL) Free(g->xhtml.buffer);
	g->xhtml.buffer = Malloc(StrLen(ent) + 1);
	ErrFatalDisplayIf(!g->xhtml.buffer, "Malloc failed");

	g->xhtml.textAvail = true;

	if (isPre == false && isTextarea == false) {
		while (i < StrLen(ent) && ent[i] != '\0') {
			if (ent[i] != '\r' && ent[i] != '\n' && ent[i] != '\t' && ent[i] != 0x09) {

				/*if ((ent[i] == ' ') && (gotTxt == true)) {
					if (w == 0) { StrCopy(g->xhtml.buffer+j, ent+i); j++; }
					w++;
				} else if ((ent[i] == ' ') && (gotTxt == false)) {
					if ((g->xhtml.paragraph == true) && (oneSpace == false)) {
						StrCopy(g->xhtml.buffer+j, ent+i); j++;
						oneSpace = true;
					}
				} else {
					gotTxt = true;
					w = 0;
					StrCopy(g->xhtml.buffer+j, ent+i);
					j++;
				}*/
				if (ent[i] == ' ') {
					if (w == 0) {
						StrCopy(g->xhtml.buffer+j, ent+i);
						j++;
					}
					w++;
				} else {
					w = 0;
					StrCopy(g->xhtml.buffer+j, ent+i);
					j++;
				}
			}

			i++;
		}
		g->xhtml.buffer[j++] = '\0';

		if ((g->xhtml.buffer == '\0') || (StrLen(g->xhtml.buffer) == 0)) {
			Free(g->xhtml.buffer);
			g->xhtml.buffer = NULL;
			return 0;
		}
	} else if (isPre == true) {
		while (i < StrLen(ent) && ent[i] != '\0') {
			if (ent[i] != '\r') {
				StrCopy(g->xhtml.buffer+j, ent+i);
				j++;
			}

			i++;
		}
		g->xhtml.buffer[j++] = '\0';
	} else if (isTextarea == true) {
		if (ent[i] == '\r' && ent[i++] == '\n') i += 2;
		else if (ent[i] == '\r') i++;
		else if (ent[i] == '\n') i++;

		while (i < StrLen(ent) && ent[i] != '\0') {
			if (ent[i] != '\r') {
				StrCopy(g->xhtml.buffer+j, ent+i);
				j++;
			}

			i++;
		}
		g->xhtml.buffer[j++] = '\0';
	}

	if (topTag == TT_WML) {
		Char	*varString = convertToVarString(g->xhtml.buffer);
		Char	*test = g->xhtml.buffer;
		
		g->xhtml.buffer = expandVar(varString, g);

		if (varString != test) // test to see if memory allocation in convertToVarString failed
			Free(varString);
	}

	Free(ent);

        return 0;
}


int textend()
{
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (g->Render == true && g->xhtml.buffer != NULL && g->xhtml.textAvail == true && isSelect == false) {
		xhtml_renderString(&(g->xhtml),g->xhtml.buffer,g);
		Free(g->xhtml.buffer);
		g->xhtml.buffer = NULL;
		g->xhtml.textAvail = false;
	}

	g->xhtml.textAvail = false;
	return 0;
}

int commentStartEnd()
{
	return 0;
}

int commentText(char *comment)
{
	return 0;
}
