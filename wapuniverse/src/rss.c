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
#include	"rss.h"
#include	"xhtml.h"
#include	"xhtml_string.h"
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
#include	"image.h"
#include	"Layout.h"
#include	"DOM.h"
#include	"Browser.h"
#include	"Hyperlinks.h"
#include	"Time.h"

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


#define RSS_SLOT_NULL		0
#define RSS_SLOT_TITLE		1
#define RSS_SLOT_LINK		2
#define RSS_SLOT_DESCRIP	3
#define RSS_SLOT_IMG_TITLE	4
#define	RSS_SLOT_IMG_LINK	5
#define RSS_SLOT_IMG_URL	6
#define RSS_SLOT_PUB_DATE	7

#define RSS_WIN_STATE_NORMAL	1
#define RSS_WIN_STATE_RESET	0
#define RSS_WIN_STATE_FINISH	2
#define RSS_WIN_STATE_REDRAW	3

static Boolean textAvail = false;
static Boolean end = false;
static Char *buffer = NULL;

static Boolean item = false, hasImage = false;
static Boolean start = false;

int rss_textstart();
int rss_texts (char *text);
int rss_textend();

int rss_xhtmltag (char *tag, struct ArgvTable *args, int numargs);
int rss_starttag (char *tag, struct ArgvTable *args, int numargs);
int rss_endtag (char *tag);

void RSSShowIndicator(Boolean display)
{
	RectangleType		rect, bounds;
	IndexedColorType	origFore, origBack;

	FrmGetObjectBounds(FrmGetActiveForm(), 
			   FrmGetObjectIndex(FrmGetActiveForm(), fldUrlBox),
			   &bounds);

	rect.topLeft.x = bounds.topLeft.x + bounds.extent.x;
	rect.topLeft.y = bounds.topLeft.y;
	rect.extent.x  = 8;
	rect.extent.y  = bounds.extent.y - 1;

	origFore = WinSetForeColor(UIColorGetTableEntryIndex(UIFieldBackground));
	origBack = WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));

	if (display) {
		WinEraseRectangle(&rect, 0);
		FrmShowObject(FrmGetActiveForm(),
			      FrmGetObjectIndex(FrmGetActiveForm(),
			      buttonRss));
	} else {
		FrmHideObject(FrmGetActiveForm(),
			      FrmGetObjectIndex(FrmGetActiveForm(),
			      buttonRss));
		WinEraseRectangle(&rect, 0);
	}

	WinSetForeColor(origFore);
	WinSetBackColor(origBack);
}

void rss_setWindow(int state, GlobalsType *g)
{
	RectangleType 		rP;
	BitmapPtr		resP = 0;
	ImagePtr		img;
	DOMPtr			ptr;
	
        rP.topLeft.x = 0;
	rP.topLeft.y = g->y + 2;
	rP.extent.x = browserGetWidgetBounds(WIDGET_WIDTH);
	rP.extent.y = g->DrawHeight - g->y - 2;


	if (state == RSS_WIN_STATE_NORMAL) {
		addNewBackground("#5898FF", g);
		g->xhtml.background->noRefresh = true;

		resP = DOMResourceToPtr(bitmapRsc, bmpRssBackground);
		img = addPageImage(NULL, NULL, 0, browserGetWidgetBounds(WIDGET_WIDTH), g->y, g->y + 21, browserGetWidgetBounds(WIDGET_WIDTH), 21, resP, g);
		if (img)
			img->alreadyHires = true;

		g->y += 2;
		g->x += 2;

		start = true;
	} else if (state == RSS_WIN_STATE_REDRAW) { // TODO: Fix
		removeLastBackground(true, g);

		g->pageImages->y1 = g->y - 21;
		g->pageImages->y2 = g->pageImages->y1 + 21;

		ptr = g->DOM;

		for (; ptr; ptr = ptr->next) {
			switch (ptr->type) {
				case DOM_UI_IMAGE:
					if (ptr->ptr.p == g->pageImages) {
						ptr->bounds.topLeft.y = g->y - 21;
					}
					break;
				default:
					break;
			}
		}		
	} else if (state == RSS_WIN_STATE_FINISH) {
		g->y += 2;
		start = false;
	} else {
		start = false;
	}
}

int rss_textstart() {

	return 0;
}

int rss_texts (char *text) {
        GlobalsType 	*g;
	Char		*ent = NULL;
	int 		i = 0, j = 0, w = 0;
	//Boolean		gotTxt = false;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (end == true) return 0;

	if (StrStr(text, "\r\n ")) {
		while (*text == '\r' || *text == '\n' || *text == ' ')
			text++;
	}

	if (text == NULL)
		return 0;

	ent = EntityScanString(text, g);

	if (buffer != NULL) Free(buffer);
	buffer = Malloc(StrLen(ent) + 1);
	ErrFatalDisplayIf(!buffer, "Malloc failed");

	textAvail = true;

	while (i < StrLen(ent) && ent[i] != '\0') {
		if (ent[i] != '\r' && ent[i] != '\n' && ent[i] != '\t' && ent[i] != 0x09) {
				/*if ((ent[i] == ' ') && (gotTxt == true)) {
				if (w == 0) { StrCopy(buffer+j, ent+i); j++; }
				w++;
			} else if ((ent[i] == ' ') && (gotTxt == false)) {
				if ((paragraph == true) && (oneSpace == false)) {
					StrCopy(buffer+j, ent+i); j++;
					oneSpace = true;
				}
			} else {
				gotTxt = true;
				w = 0;
				StrCopy(buffer+j, ent+i);
				j++;
			}*/
			if (ent[i] == ' ') {
				if (w == 0) { StrCopy(buffer+j, ent+i); j++; }
				w++;
			} else {
				w = 0;
				StrCopy(buffer+j, ent+i);
				j++;
			}
		}

		i++;
	}
	buffer[j++] = '\0';

	if ((buffer == '\0') || (StrLen(buffer) == 0)) {
		Free(buffer);
		buffer = NULL;
		return 0;
	}

	Free(ent);

        return 0;
}


int rss_textend() {
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (g->Render == true && buffer != NULL && textAvail == true) {
		xhtml_renderString(&(g->xhtml),buffer,g);
		Free(buffer);
		buffer = NULL;
		textAvail = false;
	}

	textAvail = false;
	return 0;
}



int rss_xhtmltag (char *tag, struct ArgvTable *args, int numargs) {
        
	rss_starttag (tag, args, numargs);
	rss_endtag (tag);

        return 0;
}

int rss_drawTop(Char *title, Char *description, Char *link, Char *imageUrl, Char *imageLink, Char *imageTitle, GlobalsType *g)
{
	Char			*ent = NULL;
	RGBColorType		color;  
    	IndexedColorType   	indexedColor;
	BitmapPtr		resP = 0;
	ImagePtr		img;
	UInt16			width = 0, height = 0;

	color.r   = ( UInt8 ) 255;
    	color.g   = ( UInt8 ) 255;
    	color.b   = ( UInt8 ) 255;
    	indexedColor  = WinRGBToIndex( &color );

	//WinSetTextColor(indexedColor);

	g->Render = true;

	rss_setWindow(RSS_WIN_STATE_NORMAL, g);

	// Title

	ent = EntityScanString(title, g);

	browserSetTitle(XHTMLConvertEncoding((ent != NULL)?ent:(title?title:"Untitled"),g));
	if (g->deckTitle != NULL) Free(g->deckTitle);
	g->deckTitle = Malloc(StrLen((ent != NULL)?ent:(title?title:"Untitled"))+1);
		ErrFatalDisplayIf (!g->deckTitle, "Malloc Failed");
	StrCopy(g->deckTitle,(ent != NULL)?ent:(title?title:"Untitled"));

	// Image
	
	g->imgAlign = Left;

	if (imageUrl != NULL) {
		if (imageLink != NULL) {
			//WinPushDrawState();
			HyperlinkSetLink(true, "", imageLink, NULL, g);
			isLink = true;

			if ((ImageCreateImage(imageUrl,&width,&height,false,imageTitle,g) != true)) {
				ImageCreateEmptyImage(imageTitle, &g->x, &g->y, &width, &height, false, g);
			}

			HyperlinkSetLink(false, NULL, NULL, NULL, g);
			isLink = false;
			//WinPopDrawState();
		} else {
			if ((ImageCreateImage(imageUrl,&width,&height,false,imageTitle,g) != true)) {
				ImageCreateEmptyImage(imageTitle, &g->x, &g->y, &width, &height, false, g);
			}
		}

		//xhtml_renderLn(g);
	}

	g->imgAlign = None;

	strong(true);

	g->font->color = indexedColor;

	xhtml_renderString(&(g->xhtml),(ent != NULL)?ent:(title?title:"Untitled"),g);

	xhtml_renderLn(g);

	if (ent != NULL) {
		Free(ent);
		ent = NULL;
	}

	strong(false);

	// Link/Description

	small(true);

	if (description != NULL) {
		g->indent = 1;

		g->font->color = indexedColor;

		g->x = g->xindent + (g->indent * BrowserIndentValue());

		//if (description != NULL) xhtml_renderString(&(g->xhtml),description,g);

		ent = EntityScanString(description, g);
		xhtml_renderString(&(g->xhtml),(ent != NULL)?ent:description,g);
		if (ent != NULL) {
			Free(ent);
			ent = NULL;
		}
	}

	if (link != NULL) {
		//WinPushDrawState();
		g->x = browserGetWidgetBounds(WIDGET_WIDTH) - 1 - FntCharsWidth("Site", StrLen("Site"));
		HyperlinkSetLink(true, "", link, NULL, g);

		{
		Char *site = NULL;
			site = Malloc(5);
			if (site) {
				StrCopy(site, "Site");
				xhtml_renderString(&(g->xhtml),site,g);
				Free(site);
			}
		}

		HyperlinkSetLink(false, NULL, NULL, NULL, g);

		//WinPopDrawState();
	}

	g->indent = 0;
	xhtml_renderLn(g);

	small(false);

	g->y += 2;

	rss_setWindow(RSS_WIN_STATE_REDRAW, g);

	resP = DOMResourceToPtr(bitmapRsc, bmpRssShadow);
	img = addPageImage(NULL, NULL, 0, browserGetWidgetBounds(WIDGET_WIDTH), g->y, g->y + 2, browserGetWidgetBounds(WIDGET_WIDTH), 2, resP, g);
	if (img)
		img->alreadyHires = true;

	g->y += 2;

	//removeLastBackground(true, g);
	rss_setWindow(RSS_WIN_STATE_FINISH, g);

	return 0;
}

int rss_drawItem(Char *title, Char *description, Char *link, Char *imageUrl, Char *imageLink, 
		 Char *imageTitle, Char *pubDate, GlobalsType *g)
{
	//WinHandle		prevDrawWin;
	RGBColorType		color;  
    	IndexedColorType   	indexedColor;
	Char			*ent = NULL;
	UInt16			width = 0, height = 0;


	g->Render = true;

	addNewBackground("#FFFFFF", g);

	// Title
	if (title) {
		bold(true);

		ent = EntityScanString(title, g);
		xhtml_renderString(&(g->xhtml),(ent != NULL)?ent:title,g);
		if (ent != NULL) {
			Free(ent);
			ent = NULL;
		}

		bold(false);
		xhtml_renderLn(g);
	}

	// Pub Date
	if (pubDate) {
		Char	*temp = NULL;

		small(true);

		setFontColor("#C1C1C1");

		temp = TimeToFriendly(pubDate);

		xhtml_renderString(&(g->xhtml),(temp)?temp:pubDate,g);

		if (temp)
			Free(temp);

		small(false);
		xhtml_renderLn(g);
	}


	// Image
	
	g->imgAlign = Left;

	if (imageUrl != NULL) {
		if (imageLink != NULL) {
			//WinPushDrawState();
			HyperlinkSetLink(true, "", imageLink, NULL, g);
			isLink = true;

			if ((ImageCreateImage(imageUrl,&width,&height,false,imageTitle,g) != true)) {
				ImageCreateEmptyImage(imageTitle, &g->x, &g->y, &width, &height, false, g);
			}

			HyperlinkSetLink(false, NULL, NULL, NULL, g);
			isLink = false;
			//WinPopDrawState();
		} else {
			if ((ImageCreateImage(imageUrl,&width,&height,false,imageTitle,g) != true)) {
				ImageCreateEmptyImage(imageTitle, &g->x, &g->y, &width, &height, false, g);
			}
		}

		//xhtml_renderLn(g);
	}

	g->imgAlign = None;

	small(true);

	// Description
	if (description) {

		ent = EntityScanString(description, g);
		xhtml_renderString(&(g->xhtml),(ent != NULL)?ent:description,g);
		if (ent != NULL) {
			Free(ent);
			ent = NULL;
		}
	}


	// Link
	if (link) {
		if ((g->x + FntCharsWidth("Read More...", StrLen("Read More..."))) > (browserGetWidgetBounds(WIDGET_WIDTH) - 1))
			xhtml_renderLn(g);

		//WinPushDrawState();
		HyperlinkSetLink(true, "", link, NULL, g);

		{
		Char *more = NULL;
			more = Malloc(13);
			if (more) {
				StrCopy(more, "Read More...");
				xhtml_renderString(&(g->xhtml),more,g);
				Free(more);
			}
		}
		//rss_renderString(&(g->xhtml),"Read More...",g);

		HyperlinkSetLink(false, NULL, NULL, NULL, g);
		//WinPopDrawState();
	}

	small(false);

	/*while (g->xindent) {
		xhtml_renderLn(g);
	}*/

	if (g->horizIndent && g->xindent && (g->horizIndent->h > g->fontH) && (g->horizIndent->align == Left)) {
		UInt32	imgStop = g->imgY + g->horizIndent->h;

		g->y = (imgStop + 1) - g->fontH;
		g->x = 0;
	
		removeLastIndent(g);

		g->imgH = 0;
	}

	// ---------------------
	xhtml_renderLn(g);

	color.r   = ( UInt8 ) 193;
    	color.g   = ( UInt8 ) 193;
    	color.b   = ( UInt8 ) 193;
    	indexedColor  = WinRGBToIndex( &color );

	g->y += 6;

	xhtmlDrawHR(browserGetWidgetBounds(DRAW_WIDTH), (isHires())?2:1, false, indexedColor, g);

	g->y -= 6;

	removeLastBackground(true, g);

	//WinSetDrawWindow(prevDrawWin);
    	updateScreen(g);

	browserResizeDimensions(0, 12, g);
	g->x = browserGetWidgetBounds(WIDGET_WIDTH);

	return 0;
}

int rss_render(GlobalsType *g)
{
	RSSPtr	rss;

	g->Align = Left;

	if (!g->rss) {
		rss_setWindow(RSS_WIN_STATE_RESET, g);
		MyErrorFunc("Invalid RSS document structure!", NULL);
		return 1;
	}

	rss = g->rss;

	freeAlignment(g);
	freeFont(g);
	regular(true);
	addNewBackground("#FFFFFF", g);

	while (rss != NULL) {
		
		switch (rss->type) {
			case TOP:
				rss_drawTop(rss->title, rss->description, rss->link, rss->imageUrl, rss->imageLink, rss->imageTitle, g);
				break;
			case ITEM:
				rss_drawItem(rss->title, rss->description, rss->link, rss->imageUrl, rss->imageLink, rss->imageTitle, rss->date, g);
				break;
			default:
				break;
		}

		rss = rss->next;
	}
	
	regular(false);
	freeAlignment(g);
	freeFont(g);
	freeBackground(g);

	return 0;
}

int rss_cleanup(GlobalsType *g)
{
	RSSPtr	rss, temp;

	g->tempRss = NULL;
	rss = g->rss;

	while (rss != NULL) {
		temp = rss;
		rss = rss->next;
	
		temp->type = 0;
		if (temp->title)
			Free(temp->title);
		temp->title = NULL;
		if (temp->link)
			Free(temp->link);
		temp->link = NULL;
		if (temp->description)
			Free(temp->description);
		temp->description = NULL;
		if (temp->imageTitle)
			Free(temp->imageTitle);
		temp->imageTitle = NULL;
		if (temp->imageUrl)
			Free(temp->imageUrl);
		temp->imageUrl = NULL;
		if (temp->imageLink)
			Free(temp->imageLink);
		temp->imageLink = NULL;
		if (temp->date)
			Free(temp->date);
		temp->date = NULL;

		Free(temp);		
	}
	
	g->rss = NULL;

	return 0;	
}

int rss_createBlank(GlobalsType *g)
{
	RSSPtr	rss, temp;

	rss = Malloc(sizeof(RSSType));
	ErrFatalDisplayIf(!rss, "Failed to allocate memory for RSS feed!");
	MemSet(rss, sizeof(RSSType), 0);

	rss->type = TOP;
	rss->title = NULL;
	rss->link = NULL;
	rss->imageTitle = NULL;
	rss->imageUrl = NULL;
	rss->date = NULL;

	temp = g->rss;
	if (temp == NULL) {
		g->rss = rss;
	} else {
		while(temp->next != NULL){
			temp = temp->next;
		}
		temp->next = rss;
	}
 
	g->tempRss = rss;

	return 0;
}

int rss_addItem(GlobalsType *g)
{
	RSSPtr	rss, temp;

	rss = Malloc(sizeof(RSSType));
	ErrFatalDisplayIf(!rss, "Failed to allocate memory for RSS feed!");
	MemSet(rss, sizeof(RSSType), 0);

	rss->type = ITEM;
	rss->title = NULL;
	rss->link = NULL;
	rss->imageTitle = NULL;
	rss->imageUrl = NULL;
	rss->date = NULL;

	temp = g->rss;
	if (temp == NULL) {
		g->rss = rss;
	} else {
		while(temp->next != NULL){
			temp = temp->next;
		}
		temp->next = rss;
	}

	g->tempRss = rss;

	return 0;
}

int rss_fillValue(int slot, Char *value, GlobalsType *g)
{
	if (buffer == NULL)
		return 1;

	if (!g->tempRss)
		return 1;
	
	switch (slot) {
		case RSS_SLOT_TITLE:
			g->tempRss->title = Malloc(StrLen(value)+1);
			if (!g->tempRss->title)
				return 2;
			StrCopy(g->tempRss->title, value);
			break;
		case RSS_SLOT_LINK:
			g->tempRss->link = Malloc(StrLen(value)+1);
			if (!g->tempRss->link)
				return 2;
			StrCopy(g->tempRss->link, value);
			break;
		case RSS_SLOT_DESCRIP:
			g->tempRss->description = Malloc(StrLen(value)+1);
			if (!g->tempRss->description)
				return 2;
			StrCopy(g->tempRss->description, value);
			break;
		case RSS_SLOT_IMG_TITLE:
			g->tempRss->imageTitle = Malloc(StrLen(value)+1);
			if (!g->tempRss->imageTitle)
				return 2;
			StrCopy(g->tempRss->imageTitle, value);
			break;
		case RSS_SLOT_IMG_LINK:
			g->tempRss->imageLink = Malloc(StrLen(value)+1);
			if (!g->tempRss->imageLink)
				return 2;
			StrCopy(g->tempRss->imageLink, value);
			break;
		case RSS_SLOT_IMG_URL:
			g->tempRss->imageUrl = Malloc(StrLen(value)+1);
			if (!g->tempRss->imageUrl)
				return 2;
			StrCopy(g->tempRss->imageUrl, value);
			break;
		case RSS_SLOT_PUB_DATE:
			g->tempRss->date = Malloc(StrLen(value)+1);
			if (!g->tempRss->date)
				return 2;
			StrCopy(g->tempRss->date, value);
			break;
		default:
			break;
	}

	return 0;
}


int rss_starttag (char *tag, struct ArgvTable *args, int numargs) {
        GlobalsType 		*g;
	//char 			*attr;
   	RGBColorType       	color;  
    	IndexedColorType   	indexedColor;

    	color.r   = ( UInt8 ) 255;
    	color.g   = ( UInt8 ) 255;
    	color.b   = ( UInt8 ) 255;
    	indexedColor  = WinRGBToIndex( &color );

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	showProgress(g);

	if (Compare(tag, "rss", 3)) { 						// RSS
		
	} else if (Compare(tag, "title", 5)) { 					// TITLE
		
	} else if (Compare(tag, "description", 11)) {				// DESCRIPTION
		
	} else if (Compare(tag, "item", 4)) { 					// ITEM
		item = true;
		rss_addItem(g);
	} else if (Compare(tag, "image", 5)) {					// IMAGE
		hasImage = true;
	} else if (Compare(tag, "url", 3)) {					// URL
		
	} else if (Compare(tag, "link", 4)) {					// LINK
		
	} else if (Compare(tag, "pubDate", 7)) {				// PUBDATE
		
	} else {								// default
		g->Render = false;
	}

	g->Render = false;

        return 0;
}

int rss_endtag (char *tag) {
        GlobalsType 		*g;
	RGBColorType		color;  
    	IndexedColorType	indexedColor;
	
	color.r   = ( UInt8 ) 0;
	color.g   = ( UInt8 ) 0;
	color.b   = ( UInt8 ) 210;
	indexedColor  = WinRGBToIndex( &color );

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	showProgress(g);

	if (Compare(tag, "rss", 3)) {						// RSS
		end = true;
	} else if (Compare(tag, "title", 5)) { 					// TITLE
		if (hasImage == true) {
			rss_fillValue(RSS_SLOT_IMG_TITLE, buffer, g);
		} else {
			rss_fillValue(RSS_SLOT_TITLE, buffer, g);
		}
		
		if (buffer) Free(buffer);
		buffer = NULL;
		textAvail = false;
	} else if (Compare(tag, "description", 11)) {				// DESCRIPTION
		if (hasImage == true) {
			
		} else {
			rss_fillValue(RSS_SLOT_DESCRIP, buffer, g);
		}
		
		if (buffer) Free(buffer);
		buffer = NULL;
		textAvail = false;
	} else if (Compare(tag, "item", 4)) { 					// ITEM
		item = false;
	} else if (Compare(tag, "image", 5)) {					// IMAGE
		hasImage = false;
	} else if (Compare(tag, "url", 3)) {					// URL
		if (hasImage == true) {
			rss_fillValue(RSS_SLOT_IMG_URL, buffer, g);
		} else {
			
		}
		
		if (buffer) Free(buffer);
		buffer = NULL;
		textAvail = false;
	} else if (Compare(tag, "link", 4)) {					// LINK
		if (hasImage == true) {
			rss_fillValue(RSS_SLOT_IMG_LINK, buffer, g);
		} else {
			rss_fillValue(RSS_SLOT_LINK, buffer, g);
		}
		
		if (buffer) Free(buffer);
		buffer = NULL;
		textAvail = false;
	} else if (Compare(tag, "pubDate", 7)) { 				// PUBDATE
		rss_fillValue(RSS_SLOT_PUB_DATE, buffer, g);
		
		if (buffer) Free(buffer);
		buffer = NULL;
		textAvail = false;
	} else {								// default
		g->Render = false;
	}

	g->Render = false;

        return 0;
}

int rss_xmltag(char *tag, struct ArgvTable *args, int numargs)
{
	Char *attr;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	attr = getXHTMLAttrValue("encoding", args, numargs);
	if (attr) {
	
		if (StrCompareLower(attr, "utf-8")) {
			g->charSet = UTF_8;
		} else if (StrCompareLower(attr, "us-ascii")) {
			g->charSet = US_ASCII;
		} else if (StrCompareLower(attr, "iso-8859-1")) {
			g->charSet = ISO_8859_1;
		} else if (StrCompareLower(attr, "usc-2")) {
			g->charSet = USC_2;
		} else {
			g->charSet = UTF_8;
		}

		xhtml_freeAttr(attr);
	}

	return 0;
}

int rss_commentStart()
{
	return 0;
}

int rss_commentText(char *comment)
{
	return 0;
}

int rss_commentEnd()
{
	return 0;
}

int rss_new(XHTMLPtr xhtml, WebDataPtr webData, GlobalsType *g) {

	xhtml->length = webData->length;

	xhtml->action = ACTION_RENDER;
	
    	xhtml->pageBG = UIColorGetTableEntryIndex(UIFieldBackground);

	xhtml->ontimerUrl = NULL;
	xhtml->timerVal = -1;

	buffer = NULL;

	item = false;
	hasImage = false;
	start = true;
	end = false;

	g->Align = Left;
	g->Render = false;

	return 0;
}

int rss_parse(XHTMLPtr xhtml, WebDataPtr webData, GlobalsType *g) {
	int 			newErr = 0;
	

	if (webData->data == NULL) {
        	return 1;
    	}
	
	newErr = rss_new(xhtml, webData, g);
	if (newErr == 1) {
		MyErrorFunc("Not enough memory to complete request. Please Free up some memory and try again", NULL);
		return 1;
	}


	textCallBack		= rss_texts;
	textStartCallBack	= rss_textstart;
	textEndCallBack		= rss_textend;

	//entityCallBack		= rss_entity;

	XHTMLCallBack		= rss_xhtmltag;
	startCallBack		= rss_starttag;
	endCallBack		= rss_endtag;

	commentCallBack		= rss_commentText;
	commentStartCallBack	= rss_commentStart;
	commentEndCallBack	= rss_commentEnd;

	xmlCallBack		= rss_xmltag;

	rss_createBlank(g);

	(void)parse(webData->data);

	rss_render(g);

	rss_free(xhtml, g);

	if (buffer != NULL) Free(buffer);
	buffer = NULL;

        return 0;
}

int rss_free(XHTMLPtr xhtml, GlobalsType *g)
{	
	//if (xhtml->data != NULL) {
	//	Free(xhtml->data);
	//	xhtml->data = NULL;
	//}

	xhtml->length = 0;

	xhtml->pageBG = UIColorGetTableEntryIndex(UIFieldBackground);

	if (buffer != NULL) Free(buffer);
	buffer = NULL;

	rss_cleanup(g);

    	return 0;
}
