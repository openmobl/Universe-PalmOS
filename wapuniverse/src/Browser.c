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
#include 	<PalmOS.h>
#include	"Browser.h"
#include 	"BrowserAPI.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"build.h"
#include	"../res/WAPUniverse_res.h"
#include	"DOM.h"
#include	"PalmUtils.h"
#include	"Layout.h"
#include	"Form.h"
#include	"Browser.h"
#include	"Hyperlinks.h"
#include	"Pointer.h"
#include	"URL.h"

void BrowserInitialiseGlobals(void)
{
	GlobalsType	*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	g->browserGlobals.retransCount = 0;
	g->browserGlobals.nextTick = 0;
	g->browserGlobals.saved = 0;
	g->browserGlobals.savedclength = 0;
	g->browserGlobals.savedrlength = 0;
	g->browserGlobals.updateMenu = true;
	g->browserGlobals.redrawMenuBar = true;
	g->browserGlobals.tapandhold = false;
	g->browserGlobals.taptick = 0;
	g->browserGlobals.tapx = 0;
	g->browserGlobals.tapy = 0;
	g->browserGlobals.taptype = TAPNONE;
	g->browserGlobals.imgYAdd = true;
	g->browserGlobals.optMenuOpen = false;
	g->browserGlobals.textDirRTL = false;
	g->browserGlobals.haveAlt = false;
}

GlobalsType *BrowserGetGlobals(void)
{
    GlobalsType	*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
    
    return g;
}

void BrowserScrollTo(FormPtr frm, UInt16 gadget, Int16 pos, GlobalsType *g)
{
	RectangleType 	bounds, vacated;
	Int16 		shift;

	BrowserGetObjectBounds(frm, gadget, &bounds);
	//BrowserDeactivateControls(frm);

	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	pos = MAX(0, MIN(g->DrawHeight - bounds.extent.y, pos));
	shift = pos - g->ScrollY;
	if (shift == 0) {
		WinPopDrawState();
		return;
	}

	g->ScrollY = pos;
	if (g_prefs.fiveWayMode == FIVEWAY_MODE_POINTER) {
		WinPopDrawState();

		//BrowserDraw(frm, gadget, vacated.topLeft.y, vacated.topLeft.y + vacated.extent.y + 1);
		//DOMRenderDOM(frm, gadget, g->ScrollX, g->ScrollY,
		//	     g->ScrollX + bounds.extent.x, g->ScrollY + bounds.extent.y,
		//	     /*((g->state != BS_IDLE) && (g->state != BS_ONTIMER) && (g->state != BS_XHTML_ONTIMER))?true:false*/ true, true, g);

		//updateScreen(g);

		PointerScrollUpdate(&g->pointer, g);
	} else {
		if(ABS(shift) < bounds.extent.y) {
       		 	if(shift > 0)
       		     		WinScrollRectangle(&bounds, winUp, shift, &vacated);
			else
				WinScrollRectangle(&bounds, winDown, -shift, &vacated);
		} else {
        		vacated = bounds;
		}

		WinEraseRectangle(&vacated, 0);

		vacated.topLeft.x -= bounds.topLeft.x;
		vacated.topLeft.y -= bounds.topLeft.y;

		WinPopDrawState();

		//BrowserDraw(frm, gadget, vacated.topLeft.y, vacated.topLeft.y + vacated.extent.y + 1);
		DOMRenderDOM(frm, gadget, vacated.topLeft.x, vacated.topLeft.y,
			     vacated.topLeft.x + vacated.extent.x + 1, vacated.topLeft.y + vacated.extent.y + 1,
			     /*((g->state != BS_IDLE) && (g->state != BS_ONTIMER) && (g->state != BS_XHTML_ONTIMER))?true:false*/ true, true, g);

		BrowserUpdateScrollBar(frm, g);
	}
}

void BrowserScrollHorizontalTo(FormPtr frm, UInt16 gadget, Int16 pos, GlobalsType *g)
{
	RectangleType 	bounds, vacated;
	Int16 		shift;

	BrowserGetObjectBounds(frm, gadget, &bounds);
	//BrowserDeactivateControls(frm);

	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	pos = MAX(0, MIN(g->DrawWidth - bounds.extent.x, pos));
	shift = pos - g->ScrollX;
	if (shift == 0) {
		WinPopDrawState();
		return;
	}

	g->ScrollX = pos;
	if (g_prefs.fiveWayMode == FIVEWAY_MODE_POINTER) {
		WinPopDrawState();

		//BrowserDraw(frm, gadget, vacated.topLeft.y, vacated.topLeft.y + vacated.extent.y + 1);
		//DOMRenderDOM(frm, gadget, g->ScrollX, g->ScrollY, g->ScrollX + bounds.extent.x,
		//	     g->ScrollY + vacated.extent.y, /*((g->state != BS_IDLE) && (g->state != BS_ONTIMER) && (g->state != BS_XHTML_ONTIMER))?true:false*/ true, true, g);

		//updateScreen(g);

		PointerScrollUpdate(&g->pointer, g);
	} else {
		if(ABS(shift) < bounds.extent.x) {
        		if(shift > 0)
        	    		WinScrollRectangle(&bounds, winLeft, shift, &vacated);
			else
				WinScrollRectangle(&bounds, winRight, -shift, &vacated);
		} else {
        		vacated = bounds;
		}

		WinEraseRectangle(&vacated, 0);

		vacated.topLeft.x -= bounds.topLeft.x;
		vacated.topLeft.y -= bounds.topLeft.y;

		WinPopDrawState();

		//BrowserDraw(frm, gadget, vacated.topLeft.y, vacated.topLeft.y + vacated.extent.y + 1);
		DOMRenderDOM(frm, gadget, vacated.topLeft.x, vacated.topLeft.y, vacated.topLeft.x + vacated.extent.x + 1,
			     vacated.topLeft.y + vacated.extent.y + 1, /*((g->state != BS_IDLE) && (g->state != BS_ONTIMER) && (g->state != BS_XHTML_ONTIMER))?true:false*/ true, true, g);

		BrowserUpdateScrollBarHorizontal(frm, g);
	}
}

void BrowserUpdateScrollBar(FormPtr frm, GlobalsType *g)
{
	ScrollBarType 	*bar = GetObjectPtr(g->BrowserAPI.BrowserScrollBar);
	Int16 		value, min, max, pageSize;

	SclGetScrollBar(bar, &value, &min, &max, &pageSize);
	SclSetScrollBar(bar, g->ScrollY, min, max, pageSize);
}

void BrowserUpdateScrollBarHorizontal(FormPtr frm, GlobalsType *g)
{
	ScrollBarType 	*bar = GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal);
	Int16 		value, min, max, pageSize;

	SclGetScrollBar(bar, &value, &min, &max, &pageSize);
	SclSetScrollBar(bar, g->ScrollX, min, max, pageSize);
}

void BrowserRefreshScrollBarHorizontal(GlobalsType *g)
{
	SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal), g->ScrollX, 0, g->DrawWidth - browserGetWidgetBounds(WIDGET_WIDTH) + 11, browserGetWidgetBounds(WIDGET_WIDTH) - 11);
}

void BrowserRefreshScrollBars(GlobalsType *g)
{
	RectangleType	bounds;
	UInt16		len, width;

	BrowserGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), &bounds);
	len = g->DrawHeight - bounds.extent.y + (11 * HiresFactor()); // g->y - bounds.extent.y + 11;
	if (g->y > browserGetWidgetBounds(WIDGET_HEIGHT))
		SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBar), g->ScrollY, 0, MAX(len, 0), bounds.extent.y - (11 * HiresFactor()));

	width = g->DrawWidth - bounds.extent.x + (11 * HiresFactor());
	if (g->x > browserGetWidgetBounds(WIDGET_WIDTH))
		SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal), g->ScrollX, 0, MAX(width, 0), bounds.extent.x - (11 * HiresFactor()));
}

void BrowserPopupList(Int16 x, Int16 y, UInt16 type, void *ptr, GlobalsType *g)
{
	Int16		selection	= -1;
	Int16		listX		= 0;
	Int16		listY		= 0;
	UInt16		listID		= 0;
	Char		*url 		= NULL;
	Char		*ptrUrl		= NULL;


	listX = ((x - g->ScrollX) + browserGetWidgetBounds(WIDGET_TOPLEFT_X)) / HiresFactor();
	listY = ((y - g->ScrollY) + browserGetWidgetBounds(WIDGET_TOPLEFT_Y)) / HiresFactor();

	switch (type) {
		case DOM_UI_TEXT:
			listID = listLinkTap;
			ptrUrl = ((TextPtr)ptr)->link->url;
			break;
		case DOM_UI_IMAGE:
			listID = listLinkTapImage;
			ptrUrl = ((ImagePtr)ptr)->link->url;
			break;
		default:
			break;
	}

	LstSetPosition(GetObjectPtr(listID),listX, listY);

	selection = LstPopupList(GetObjectPtr(listID));

	if ((type == DOM_UI_TEXT) || (type == DOM_UI_IMAGE)) {
		if (!ptr) {
			return;
		}

		switch (selection) {
			case 0: // open in new tab
				if (g->OpenUrl)
					Free(g->OpenUrl);
				g->OpenUrl = Malloc(MAX_URL_LEN + 1);
				if (!g->OpenUrl) {
					MyErrorFunc("Failed to load the url. No memory", NULL);
					break;
				}
				URLCreateURLStr(ptrUrl, g->OpenUrl, g);

				if (g->selectedLink) {
					PointerChangeMode(&g->pointer, pointerModeNone);

					HyperlinkDeSelectLink(g->selectedLink, g);
					g->selectedLink = NULL;

					PointerUpdatePointer(&g->pointer, g);
				}

				FrmUpdateForm(frmBrowser, frmUpdateNewTab);
				break;
			case 1: // copy URL
				url = Malloc(MAX_URL_LEN + 1);
				if (!url) {
					MyErrorFunc("Failed to copy the URL. No memory", NULL);
					break;
				}
				URLCreateURLStr(ptrUrl, url, g);

				ClipboardAddItem(clipboardText, url, StrLen(url));
				break;
			case 2: // Download the image
				checkPageImage(x, y, true, g);
				break;
			case -1:
			default:
				break;
		}
	}
}


void BrowserInverseLinkText(TextPtr text, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	Char			*str = NULL;
	UInt16			textLen = 0;
	RectangleType		erase;
	UInt16			x = pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16			y = pt->y + bounds->topLeft.y - g->ScrollY;
	Boolean			italics = false;
	WinDrawOperation 	oldMode;
	RGBColorType		textColor;
	RGBColorType		foreColor;
	IndexedColorType	backColor;
	RGBColorType		textColorSet = {0,255,255,255};
	RGBColorType		foreColorSet = {0,255,255,255};
	FontID			fntID = stdFont;

	if (!text)
		return;

	str	= text->text;
	textLen = StrLen(text->text);
	italics	= (text)?text->italic:false;

	WinPushDrawState();

	fntID = FntSetFont(text->font);

	oldMode = WinSetDrawMode(winOverlay);

	UIColorGetTableEntryRGB(UIFieldText, &textColor);
	UIColorGetTableEntryRGB(UIObjectForeground, &foreColor);

	WinSetTextColorRGB(&textColorSet, &textColor);
	backColor = WinSetBackColor(text->color);
	WinSetForeColorRGB(&foreColorSet, &foreColor);

	erase.topLeft.x = x;
	erase.topLeft.y = y;
	erase.extent.x = bounds->extent.x;
	erase.extent.y = bounds->extent.y;
	WinEraseRectangle(&erase, 0);

	// TODO: We actually need to override the default text dir
	// for instance, certain characters are RTLos , we need to
	// handle LTR for this by reversing
	if (text->direction == directionRTL) {
		str = StrReverse(str);
		if (!str)
			str = text->text;
	}
 
	if (italics) {
		WinDrawCharsItalics(str, textLen, x, y);
	} else {
		WinPaintChars(str,textLen,x,y);
	}

	if (text->strike) {
		UInt16		charWidth = FntCharsWidth(str, textLen);
		UInt16		charHeight = FntCharHeight();

		WinDrawLine(x, (y + (charHeight / 2)), (x + charWidth), (y + (charHeight / 2)));
	}

	if (text->underlined) {
		UInt16		charWidth = FntCharsWidth(str, textLen);
		UInt16		charHeight = FntCharHeight();

		WinDrawLine(x, (y + charHeight - (1 * HiresFactor())), (x + charWidth), (y + charHeight - (1 * HiresFactor())));
	}

	WinSetDrawMode(oldMode);

	FntSetFont(fntID);

	WinPopDrawState();
}

void BrowserInverseLinkImage(ImagePtr img, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	UInt16			x = pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16			y = pt->y + bounds->topLeft.y - g->ScrollY;
	IndexedColorType	foreColor;
	RGBColorType		color;
	RectangleType		border;

	color.r   = ( UInt8 ) 0;
    	color.g   = ( UInt8 ) 0;
    	color.b   = ( UInt8 ) 210;

    	foreColor  = WinRGBToIndex(&color);

	WinPushDrawState();

	border.topLeft.x = x;
	border.topLeft.y = y;
	border.extent.x = bounds->extent.x - 1;
	border.extent.y = bounds->extent.y - 1;

	foreColor = WinSetForeColor(foreColor);

	//WinEraseRectangleFrame(rectangleFrame, &border);
	WinDrawLine(border.topLeft.x, border.topLeft.y, border.topLeft.x + border.extent.x, border.topLeft.y);
	WinDrawLine(border.topLeft.x, border.topLeft.y + border.extent.y, border.topLeft.x + border.extent.x, border.topLeft.y + border.extent.y);
	WinDrawLine(border.topLeft.x, border.topLeft.y, border.topLeft.x, border.topLeft.y + border.extent.y);
	WinDrawLine(border.topLeft.x + border.extent.x, border.topLeft.y, border.topLeft.x + border.extent.x, border.topLeft.y + border.extent.y);

	WinSetForeColor(foreColor);

	WinPopDrawState();
}

Boolean BrowserHandlePenOver(Int16 x, Int16 y, GlobalsType *g)
{
	DOMPtr		ptr;
	Boolean		ret = false;

	if (isHires()) {
		x *= 2;
		y *= 2;
	}

	ptr = g->DOM;

	if (!ptr)
		return false;

	x += g->ScrollX;
	y += g->ScrollY;

	for (; ptr && !ret; ptr = ptr->next) {
        	if (RctPtInRectangle(x, y, &ptr->bounds)) {
            		switch(ptr->type) {
				case DOM_UI_TEXT:
					if (ptr->ptr.t->link &&
					    !(ptr->ptr.t->link->id && !ptr->ptr.t->link->url)) {
						//PointerClearScreen(false, g);
						PointerChangeMode(&g->pointer, pointerModeHover);

						HyperlinkSelectLink(ptr->ptr.t->link, g->ScrollX, g->ScrollX + browserGetWidgetBounds(WIDGET_WIDTH),
								    g->ScrollY, g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT), g);

						ret = true;
					}
					break;
				case DOM_UI_INPUT:
					break;
				case DOM_UI_SELECT:
					break;
				case DOM_UI_IMAGE:
					if (ptr->ptr.p->link &&
					    !(ptr->ptr.p->link->id && !ptr->ptr.p->link->url)) {
						//PointerClearScreen(false, g);
						PointerChangeMode(&g->pointer, pointerModeHover);

						HyperlinkSelectLink(ptr->ptr.p->link, g->ScrollX, g->ScrollX + browserGetWidgetBounds(WIDGET_WIDTH),
								    g->ScrollY, g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT), g);

						ret = true;
					}
					break;
				case DOM_UI_BUTTON:
					break;
				case DOM_UI_RADIO:
					break;
				case DOM_UI_CHECK:
					break;
				case DOM_UI_AREA:
				default:
					break;
			}
		}
	}

	if (!ret && g->selectedLink) {
		PointerChangeMode(&g->pointer, pointerModeNone);

		HyperlinkDeSelectLink(g->selectedLink, g);
		g->selectedLink = NULL;
	}

	//if (ret) {
	//	PointerDrawPointer(&g->pointer, g);
	//} else {
		PointerUpdatePointer(&g->pointer, g);
	//}

	return ret;
}

Boolean BrowserHandlePenTap(Int16 x, Int16 y, Boolean penDown, GlobalsType *g)
{
	DOMPtr		ptr;
	Boolean		ret = false;

	if (isHires()) {
		x *= 2;
		y *= 2;
	}

	ptr = g->DOM;

	if (!ptr)
		return false;

	x += g->ScrollX;
	y += g->ScrollY;

	if (!penDown && g->selectedLink) {
		PointerChangeMode(&g->pointer, pointerModeNone);

		HyperlinkDeSelectLink(g->selectedLink, g);
		g->selectedLink = NULL;

		PointerUpdatePointer(&g->pointer, g);
	} else if (!penDown) {
		PointerChangeMode(&g->pointer, pointerModeNone);
	}

	//if (penDown)
	//	PointerClearScreen(false, g);

	for (; ptr && (ret == false); ptr = ptr->next) {
        	if (RctPtInRectangle(x, y, &ptr->bounds)) {
            		switch(ptr->type) {
				case DOM_UI_TEXT: // TODO: popup option menu to follow link or copy url when link and tap&hold
					if (!penDown) {
						PointerUpdatePointer(&g->pointer, g);
						ret = HyperlinkCheckLink(ptr->ptr.t->link, g);
					}

					if (penDown && ptr->ptr.t->link &&
					    !(ptr->ptr.t->link->id && !ptr->ptr.t->link->url)) {
						PointerChangeMode(&g->pointer, pointerModeHover);

						HyperlinkSelectLink(ptr->ptr.t->link, g->ScrollX, g->ScrollX + browserGetWidgetBounds(WIDGET_WIDTH),
								    g->ScrollY, g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT), g);

						if (!g->browserGlobals.tapandhold && penDown) {
							g->browserGlobals.tapandhold = true;
							g->browserGlobals.taptick = TimGetTicks();
							g->browserGlobals.tapx = x;
							g->browserGlobals.tapy = y;
							g->browserGlobals.taptype = TAPLINK;
						}

					}
					break;
				case DOM_UI_INPUT:
					if (penDown) {
						PointerClearScreen(false, g);
						ret = checkInput(ptr->ptr.i, g);
					}
					break;
				case DOM_UI_SELECT:
					if (!penDown)
						ret = checkSelect(ptr->ptr.s, g);
					break;
				case DOM_UI_IMAGE:
					if (!g->browserGlobals.tapandhold && penDown) {
						g->browserGlobals.tapandhold = true;
						g->browserGlobals.taptick = TimGetTicks();
						g->browserGlobals.tapx = x;
						g->browserGlobals.tapy = y;
						g->browserGlobals.taptype = TAPIMAGE;
					}

					if (penDown && ptr->ptr.p->link &&
					    !(ptr->ptr.p->link->id && !ptr->ptr.p->link->url)) {
						PointerChangeMode(&g->pointer, pointerModeHover);

						HyperlinkSelectLink(ptr->ptr.p->link, g->ScrollX,
								    g->ScrollX + browserGetWidgetBounds(WIDGET_WIDTH),
								    g->ScrollY, g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT),
								    g);
					}					

					if (!penDown) {
						ret = imageCheckArea(ptr->ptr.p->mapName, x, y, g);
						if (!ret)
							ret = HyperlinkCheckLink(ptr->ptr.p->link, g);
					}

					break;
				case DOM_UI_BUTTON:
					if (!penDown)
						ret = checkFormButton(ptr->ptr.b, g);
					break;
				case DOM_UI_RADIO:
					if (!penDown)
						ret = checkRadioButton(ptr->ptr.r, g);
					break;
				case DOM_UI_CHECK:
					if (!penDown)
						ret = checkCheckBox(ptr->ptr.c, g);
					break;
				case DOM_UI_AREA:
				default:
					break;
			}
		}
	}

	//if (penDown) {
	//	PointerDrawPointer(&g->pointer, g);
	//} else {
		PointerUpdatePointer(&g->pointer, g);
	//}

	return ret;
}

void BrowserUpdateScreen(GlobalsType *g)
{
	if (g->browserGlobals.optMenuOpen == true) {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
		g->browserGlobals.optMenuOpen = false;
	}

	browserSetTitle(g->browserGlobals.browserTitle);

	if ((g_prefs.renderMode == RENDER_WIDE_SCREEN) && (g->DrawWidth > browserGetWidgetBounds(WIDGET_WIDTH))) {
		RectangleType	bounds;
		UInt16		width;

		BrowserGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), &bounds);

		width = g->DrawWidth - bounds.extent.x + (11 * HiresFactor());
		SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal), g->ScrollX, 0, MAX(width, 0), bounds.extent.x - (11 * HiresFactor()));

		goto display_change_skip;
	}

	if (g->BrowserAPI.lastWidgetWidth != browserGetWidgetBounds(WIDGET_WIDTH)) {
		FormInputDeactivate(g);
		//browserReloadEvt(g);
		//followLink(g->Url, g);
		if (g->pageContentType && 
		    (StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlc",24)==0)) {
			palmResetState(g);
			g->state=BS_RENDER;
			g->refreshEvent = true;
		} else if (g->pageContentType && 
			   ((StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29)==0) || 
			    (StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21)==0) || 
			    (StrNCaselessCompare(g->pageContentType,"application/wml+xml",19)==0) ||
			    (StrNCaselessCompare(g->pageContentType,"text/html",9)==0) ||
		            (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)==0))) {
				palmResetState(g);
            			g->state = BS_XHTML_PARSE_RENDER;
				g->refreshEvent = true;
            	} else if (g->pageContentType && (StrNCaselessCompare(g->pageContentType,"application/rss+xml",19)==0)) {
				palmResetState(g);
            			g->state = BS_RSS_PARSE_RENDER;
		}
	} else {
		scrollUpdate(g);
	}
display_change_skip:

	g->BrowserAPI.lastWidgetWidth = browserGetWidgetBounds(WIDGET_WIDTH);
	browserSwitchMode(g_prefs.mode);
}

// frm and index pointless, will remove
void BrowserGetObjectBounds(FormType *frm, UInt16 index, RectangleType *rect)
{
	rect->topLeft.x = browserGetWidgetBounds(WIDGET_TOPLEFT_X);
	rect->topLeft.y = browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
	rect->extent.x = browserGetWidgetBounds(WIDGET_WIDTH);
	rect->extent.y = browserGetWidgetBounds(WIDGET_HEIGHT);
}

UInt16 BrowserIndentValue(void)
{
	return indentMarginValue * HiresFactor();
}

UInt16 BrowserLineheightValue(void)
{
	return lineheightValue * HiresFactor();
}

UInt16 BrowserColumnValue(void)
{
	return columnValue * HiresFactor();
}

/* Some fun little tools :) */
void BrowserDisplayPageStats(GlobalsType *g)
{
	Char	*stats;

	stats = Malloc(15 * 1024);
	if (!stats)
		return;

	StrPrintF(stats, "Page Stats:\n   Size: %f kb\n   Load time: %f\n\n", 0, (double)(g->pageStats.timePageLoadFinish - g->pageStats.timePageLoadStart));

	MyErrorFunc(stats, NULL);
}

