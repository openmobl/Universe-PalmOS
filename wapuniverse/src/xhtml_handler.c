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
#include	"DOM.h"
#include	"Text.h"
#include	"Area.h"
#include	"Browser.h"
#include	"debug.h"

#include	"xhtml_string.h"
#include	"xhtml_access.h"
#include	"xhtml_tag_misc.h"

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


int xhtml_parse(XHTMLPtr xhtml, WebDataPtr webData, GlobalsType *g) {
	int newErr = 0;

	if (webData->data == NULL) {
        	return 1;
    	}
	
	newErr = xhtml_new(xhtml, webData, g);
	if (newErr == 1) {
		MyErrorFunc("Not enough memory to complete request. Please Free up some memory and try again.", NULL);
		return 1;
	}


	textCallBack		= texts;
	textStartCallBack	= textstart;
	textEndCallBack		= textend;

	//entityCallBack		= entity;

	XHTMLCallBack		= xhtmltag;
	startCallBack		= starttag;
	endCallBack		= endtag;

	commentCallBack		= commentText;
	commentStartCallBack	= commentStartEnd;
	commentEndCallBack	= commentStartEnd;

	xmlCallBack		= xmltag;

	xhtml_debugStart(g);

        //(void)parse(xhtml->data);
	(void)parse(webData->data);

	debugOut("xhtml", "done parse", 0, __FILE__, __LINE__, g);

	xhtml_free(xhtml, g);

	debugOut("xhtml", "done free", 0, __FILE__, __LINE__, g);

	//WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));

	if (xhtml->buffer)
		Free(xhtml->buffer);
	xhtml->buffer = NULL;

	debugOut("xhtml", "buffer free", 0, __FILE__, __LINE__, g);

	xhtml_debugEnd(g);

        return 0;
}

int xhtml_free(XHTMLPtr xhtml, GlobalsType *g)
{	
	//if (xhtml->data != NULL) {
	//	Free(xhtml->data);
	//	xhtml->data = NULL;
	//}

	xhtml->length = 0;
	//xhtml->pageBG = UIColorGetTableEntryIndex(UIFieldBackground);
//	freeBackground(g);
	//if (xhtml->buffer != NULL) Free(xhtml->buffer);
	//xhtml->buffer = NULL;

	regular(false);
	freeAlignment(g);
	freeFont(g);
	freeList(g);
	freeIndent(g);
	PositionFreePosition(g);

    	return 0;
}
