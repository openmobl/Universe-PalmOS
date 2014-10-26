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
#include 	"http.h"
#include 	"build.h"
#include 	"WAPUniverse.h"
#include 	"wtp.h"
#include 	"wsp.h"
#include 	"WAP.h"
#include 	"../res/WAPUniverse_res.h"
#include 	"dbConn.h"
#include 	"dbUrl.h"
#include 	"formBrowser.h"
#include 	"PalmUtils.h"
#include 	"wml.h"
#include	"base64.h"
#include	"process.h"
#include	"Cookie.h"
#include	"debug.h"
#include	"Net.h"
#include	"Error.h"
#include	"Pointer.h"
#include	"Browser.h"
#include	"BrowserAPI.h"
#include	"Layout.h"
#include	"palmpng.h"
#include	"DOM.h"
#include	"Form.h"
#include	"HyperLinks.h"
#include	"Tab.h"

void PointerInitialise(WindowPointerType *pointer, GlobalsType *g)
{
	RectangleType	bounds;
	void		*png = NULL;
	int		i = 0;

	if ((g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER) && pointer)
		return;

	BrowserGetObjectBounds(FrmGetActiveForm(),
			       FrmGetObjectIndex(FrmGetActiveForm(),
						 g->BrowserAPI.BrowserWidget),
			       &bounds);

	pointer->x = bounds.topLeft.x + 3;
	pointer->y = bounds.topLeft.y + 3;

	pointer->space = (isHires())?7:10;
	pointer->mode = pointerModeNone;

	pointer->lastLr = 0;
	pointer->lastTb = 0;

	pointer->speed = g_prefs.fiveWaySpeed;
	pointer->holding = false;

	for (i = pointerModeNone; i < pointerModeLast; i++) {
		if (g->pngLibrary) {
			if (isHires())
				png = MemHandleLock(DmGetResource(pngRsrc, pngPointerBase + ((i + 1) * 2)));
			else
				png = MemHandleLock(DmGetResource(pngRsrc, pngPointerBase + ((i * 2) + 1)));
			if (png) {
				pointer->image[i].bitmap = 
					(BitmapType *)DecodePngToBitmap(g->pngLibrary,
					png, kDensityLow, &pointer->image[i].channel);
			} else {
				pointer->image[i].bitmap = NULL;
				pointer->image[i].channel = NULL;
			}
			MemPtrUnlock(png);
	
			png = NULL;
		} else {
			pointer->image[i].bitmap = NULL;
			pointer->image[i].channel = NULL;
		}
	}

	PointerDrawPointer(pointer, g);
}

void PointerCleanUp(WindowPointerType *pointer, GlobalsType *g)
{
	int	i = 0;

	for (i = pointerModeNone; i < pointerModeLast; i++) {
		if (pointer->image[i].bitmap)
			BmpDelete(pointer->image[i].bitmap);
		if (pointer->image[i].channel)
			Free(pointer->image[i].channel);
		pointer->image[i].bitmap = NULL;
		pointer->image[i].channel = NULL;
	}
}

void PointerUpdatePointer(WindowPointerType *pointer, GlobalsType *g)
{
	updateScreen(g);
}

void PointerChangeMode(WindowPointerType *pointer, PointerMode mode)
{
	pointer->mode = mode;
}

void PointerDrawPointer(WindowPointerType *pointer, GlobalsType *g)
{
	BitmapPtr	resP;
	PointerMode	mode;
	RectangleType	rect;

	if ((g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER) && pointer)
		return;

	WinPushDrawState();

	BrowserGetObjectBounds(FrmGetActiveForm(),
			       FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget),
			       &rect);

	mode = pointer->mode;

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	WinSetClip(&rect);

	if (pointer->image[mode].bitmap &&
	    pointer->image[mode].channel) {
		WinPaintAlphaImage(g->pngLibrary, pointer->image[mode].bitmap,
				   pointer->image[mode].channel,
				   pointer->x, pointer->y);
	} else {
		switch (mode) {
			case pointerModeHover:
				resP = MemHandleLock(DmGetResource(bitmapRsc, bmpPointerHand));
				WinDrawBitmap(resP, pointer->x, pointer->y);
				MemPtrUnlock(resP);
				break;
			case pointerModeWait:
			case pointerModeNone:
			default:
				resP = MemHandleLock(DmGetResource(bitmapRsc, bmpPointerArrow));
				WinDrawBitmap(resP, pointer->x, pointer->y);
				MemPtrUnlock(resP);
				break;
		}
	}

	WinResetClip();

	WinPopDrawState();
}

void PointerClearScreen(Boolean drawPointer, GlobalsType *g)
{
	if (g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER)
		return;

	DOMRenderDOM(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget),
		     0, 0, g->ScrollX + browserGetWidgetBounds(WIDGET_WIDTH),
		     g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT), true, drawPointer, g);
}

void PointerSpeedAccelerate(WindowPointerType *pointer, UInt16 speed, GlobalsType *g)
{
	pointer->speed += speed;

	// maybe just pointer->speed = MIN(browserGetWidgetBounds(WIDGET_HEIGHT), browserGetWidgetBounds(WIDGET_WIDTH));

	if (pointer->lastTb && (pointer->speed > browserGetWidgetBounds(WIDGET_HEIGHT))) {
		pointer->speed = browserGetWidgetBounds(WIDGET_HEIGHT);
	} else if (pointer->lastLr && (pointer->speed > browserGetWidgetBounds(WIDGET_WIDTH))) {
		pointer->speed = browserGetWidgetBounds(WIDGET_WIDTH);
	}
}

void PointerSpeedReset(WindowPointerType *pointer, UInt16 speed, GlobalsType *g)
{
	pointer->speed = speed;
}

void PointerSetHolding(WindowPointerType *pointer, Boolean holding, GlobalsType *g)
{
	pointer->holding = holding;
}

void PointerAddMouseOver(Int16 x, Int16 y)
{
	PointerAddMouseEvent(mouseOverEvent, x, y);
}

void PointerAddMouseEvent(UInt16 event, Int16 x, Int16 y)
{
	EventType eventT;
	
	eventT.eType	= event;
	eventT.screenX	= x / HiresFactor();
	eventT.screenY	= y / HiresFactor();

	EvtAddEventToQueue(&eventT);
}

void PointerScrollUpdate(WindowPointerType *pointer, GlobalsType *g)
{
	Int16	xofs = 0, yofs = 0;

	if ((g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER) && pointer)
		return;

	switch (pointer->mode) {
		case pointerModeHover:
			xofs += 2;
			yofs += 1;
			break;
		case pointerModeNone:
		case pointerModeWait:
			xofs += 1;
			yofs += 1;
		default:
			break;
	}

	PointerAddMouseOver(pointer->x + xofs, pointer->y + yofs);
}

Boolean PointerIsAtEdge(WindowPointerType *pointer, WinDirectionType direction, BrowserAPIType *browser)
{
	RectangleType	bounds;
	GlobalsType	*g;
	Boolean		ret = false;
	Int8		lr = 0;
	Int8		tb = 0;
	/*Boolean		x = false;
	Boolean		y = false;*/

	if ((g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER) && pointer)
		return false;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	BrowserGetObjectBounds(FrmGetActiveForm(),
			       FrmGetObjectIndex(FrmGetActiveForm(),
						 g->BrowserAPI.BrowserWidget),
			       &bounds);

	/*if (pointer->x >= ((bounds.topLeft.x + bounds.extent.x) - (pointer->space * HiresFactor()) - 1) &&
	    (direction == winRight)) {
		ret = true;
		x = true;
	} else if ((pointer->x == bounds.topLeft.x) && (direction == winLeft)) {
		ret = true;
	} else if (pointer->y >= ((bounds.topLeft.y + bounds.extent.y) - (pointer->space * HiresFactor()) -1) &&
	    (direction == winDown)) {
		ret = true;
		y = true;
	} else if ((pointer->y == bounds.topLeft.y) && (direction == winUp)) {
		ret = true;
	}*/


	switch (direction) {
		case winUp:
			if (pointer->y < (bounds.topLeft.y + ((bounds.extent.y * 3)/ 4)))
				ret = true;
			tb = -1;
			lr = 0;
			break;
		case winDown:
			if (pointer->y > (bounds.topLeft.y + (bounds.extent.y/ 4)))
				ret = true;
			tb = 1;
			lr = 0;
			break;
		case winLeft:
			if (pointer->x < (bounds.topLeft.x + ((bounds.extent.x * 3)/ 4)))
				ret = true;
			tb = 0;
			lr = -1;
			break;
		case winRight:
			if (pointer->x > (bounds.topLeft.x + (bounds.extent.x/ 4)))
				ret = true;
			tb = 0;
			lr = 1;
			break;
		default:
			break;
	}

	if (pointer->lastLr && !lr)
		pointer->flipCount = 0;
	if (pointer->lastTb && !tb)
		pointer->flipCount = 0;

	// incriment flip count on three consecutive different directions, i.e. up down up
	if (((lr * pointer->lastLr) == -1) || ((tb * pointer->lastTb) == -1)) {
		pointer->flipCount++;
	} else {
		pointer->flipCount = 0;
	}

	// remember last direction for minor adjustments
	pointer->lastLr = lr;
	pointer->lastTb = tb;

	// the user was trying to focus on a link that we scrolled over, go to it
	if (pointer->flipCount > 1)
		return false;

	if (ret) {
		if (((g->ScrollX + bounds.extent.x) >= g->DrawWidth) && (direction == winRight))
			ret = false;
		if (((g->ScrollY + bounds.extent.y) >= g->DrawHeight) && (direction == winDown))
			ret = false;
		if (!g->ScrollX && (direction == winLeft))
			ret = false;
		if (!g->ScrollY && (direction == winUp))
			ret = false;
	}

	return ret;
}

void PointerSetValueInBounds(PointType *point, RectangleType *bounds, Coord space)
{
	if (g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER)
		return;

	if (point->x < bounds->topLeft.x)
		point->x = bounds->topLeft.x;
	if (point->y < bounds->topLeft.y)
		point->y = bounds->topLeft.y;

	space /= 5;

	if ((point->x + (space * HiresFactor())) > (bounds->topLeft.x + bounds->extent.x))
		point->x = (bounds->topLeft.x + bounds->extent.x) - (space * HiresFactor());
	if ((point->y + (space * HiresFactor())) > (bounds->topLeft.y + bounds->extent.y))
		point->y = (bounds->topLeft.y + bounds->extent.y) - (space * HiresFactor());
}

void PointerSetPosition(Coord x, Coord y, WindowPointerType *pointer, Boolean mouseOver, GlobalsType *g)
{
	PointType	point;
	RectangleType	bounds;
	Int16		xofs = 0, yofs = 0;

	switch (pointer->mode) {
		case pointerModeHover:
			xofs += 2;
			yofs += 1;
			break;
		case pointerModeNone:
		case pointerModeWait:
			xofs += 1;
			yofs += 1;
		default:
			break;
	}

	if ((g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER) && pointer) {
		if (mouseOver)
			PointerAddMouseOver(x + xofs, y + yofs);
		return;
	}

	point.x = pointer->x;
	point.y = pointer->y;

	BrowserGetObjectBounds(FrmGetActiveForm(),
			       FrmGetObjectIndex(FrmGetActiveForm(),
						 g->BrowserAPI.BrowserWidget),
			       &bounds);

	point.x = x;
	point.y = y;
	PointerSetValueInBounds(&point, &bounds, pointer->space);
	pointer->x = point.x;
	pointer->y = point.y;

	//updateScreen(g);

	if (mouseOver)
		PointerAddMouseOver(x + xofs, y + yofs);
}

static Boolean PointerPenDownEvt(Int16 xpos, Int16 ypos, GlobalsType *g)
{
	if (WinGetActiveWindow() != FrmGetWindowHandle(FrmGetFormPtr(frmBrowser)))
		return false;

	if ((g->browserGlobals.optMenuOpen == true) && (ypos > (browserGetScreenHeight()-100)) && (ypos < (browserGetScreenHeight() - 14))) {
		if ((xpos > (browserGetScreenWidth() - 2)) || ((ypos > (browserGetScreenHeight()-100)) && (ypos < (browserGetScreenHeight()-82))))
			return true;

		return false;
	}

	if ((g->browserGlobals.optMenuOpen == true) && !((ypos > (browserGetScreenHeight() - 14)) && (ypos < browserGetScreenHeight()) && (xpos < 65))) {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
		g->browserGlobals.optMenuOpen = false;
	}

	if((xpos > (browserGetWidgetBounds(WIDGET_WIDTH) / HiresFactor())) || (ypos > ((browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor()) + 
		(browserGetWidgetBounds(WIDGET_HEIGHT) / HiresFactor()))) || (ypos < ((browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor()) + 2)))
			return(false);

	if (g->InputFldActive) {
		RectangleType		bounds;
		
		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fldInput), &bounds);
		if (RctPtInRectangle(xpos, ypos, &bounds)) {
			//FldEmulateEnter(fldInput);
			return false; // FldHandleEvent(GetObjectPtr(fldInput), event);
		} else {
			FormInputDeactivate(g);
		}
	}

	xpos -= browserGetWidgetBounds(WIDGET_TOPLEFT_X) / HiresFactor();
	ypos -= browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor();

	return BrowserHandlePenTap(xpos, ypos, true, g);
}

static Boolean PointerPenUpEvt(Int16 xpos, Int16 ypos, GlobalsType *g)
{
	if (WinGetActiveWindow() != FrmGetWindowHandle(FrmGetFormPtr(frmBrowser)))
		return false;

	if (g->InputFldActive) {
		RectangleType		bounds;
		
		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fldInput), &bounds);
		if (RctPtInRectangle(xpos, ypos, &bounds)) {
			return false; // FldHandleEvent(GetObjectPtr(fldInput), event);
		}
	}

	if (g->browserGlobals.tapandhold == true) {
		g->browserGlobals.tapandhold = false;
		g->browserGlobals.taptick = 0;
		g->browserGlobals.tapx = 0; g->browserGlobals.tapy = 0;
		g->browserGlobals.taptype = TAPNONE;
	}
	
	if (g->browserGlobals.optMenuOpen == true) {
		if (((ypos < browserGetScreenHeight()-100) || (ypos > (browserGetScreenHeight() - 14))) &&
			!(((ypos > (browserGetScreenHeight() - 14)) && (ypos < browserGetScreenHeight())) && ((xpos > 0) && (xpos < 65))) ) {
			FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
			FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
			FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
			WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
			g->browserGlobals.optMenuOpen = false;
		}
		return(false);
	}

	if((xpos > (browserGetWidgetBounds(WIDGET_WIDTH) / HiresFactor())) || (ypos > ((browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor()) + 
		(browserGetWidgetBounds(WIDGET_HEIGHT) / HiresFactor()))) || (ypos < (browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor()))) {
			if (g->selectedLink) {
				HyperlinkDeSelectLink(g->selectedLink, g);
				g->selectedLink = NULL;
			}

			if (ypos < (browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor())) {
				return TabHandlePen(xpos, ypos, g);
			}
			
			return(false);
	}

	// user clicked in the browser area, check if he/she clicked a hyperlink

	// let's see if we have a link under this xpos/ypos position

	xpos -= browserGetWidgetBounds(WIDGET_TOPLEFT_X) / HiresFactor();
	ypos -= browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor();

	return BrowserHandlePenTap(xpos, ypos, false, g);
}

void PointerSelectLocation(WindowPointerType *pointer, Int16 flags, GlobalsType *g)
{
	Coord		x = pointer->x;
	Coord		y = pointer->y;

	if ((g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER) && pointer)
		return;

	x /= HiresFactor();
	y /= HiresFactor();
	
	switch (pointer->mode) {
		case pointerModeHover:
			x += 2;
			y += 1;
			break;
		case pointerModeNone:
		case pointerModeWait:
			x += 1;
			y += 1;
		default:
			break;
	}

	/*x -= browserGetWidgetBounds(WIDGET_TOPLEFT_X) / HiresFactor();
	y -= browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor();*/

	if ((flags & pointerGetsPenUp) || (flags & pointerPenUp)) {
		if ((flags & pointerPenDown) == pointerPenDown) {
			PointerPenDownEvt(x, y, g); // PointerAddMouseEvent(penDownEvent, x, y); // BrowserHandlePenTap(x, y, true, g);
		} else if ((flags & pointerPenUp) == pointerPenUp) {
			PointerPenUpEvt(x, y, g); // PointerAddMouseEvent(penUpEvent, x, y); // BrowserHandlePenTap(x, y, false, g);
		}
	} else {
		PointerPenDownEvt(x, y, g); // PointerAddMouseEvent(penDownEvent, x, y); // BrowserHandlePenTap(x, y, true, g);	/* penDown */
		PointerPenUpEvt(x, y, g); // PointerAddMouseEvent(penUpEvent, x, y); // BrowserHandlePenTap(x, y, false, g);	/* penUp */
	}
}

void PointerScroll(WindowPointerType *pointer, WinDirectionType direction, GlobalsType *g)
{
	PointType	point;
	RectangleType	bounds;
	Int16		xofs = 0, yofs = 0, flipFact = 1;

	switch (pointer->mode) {
		case pointerModeHover:
			xofs += 2;
			yofs += 1;
			break;
		case pointerModeNone:
		case pointerModeWait:
			xofs += 1;
			yofs += 1;
		default:
			break;
	}

	if ((g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER) && pointer)
		return;

	point.x = pointer->x;
	point.y = pointer->y;

	BrowserGetObjectBounds(FrmGetActiveForm(),
			       FrmGetObjectIndex(FrmGetActiveForm(),
						 g->BrowserAPI.BrowserWidget),
			       &bounds);

	if (PointerIsAtEdge(pointer, direction, &g->BrowserAPI)) {
		if (pointer->lastState == pointerNavigating) {
			PointerSpeedReset(&g->pointer, g_prefs.fiveWaySpeed, g);
		}

		pointer->lastState = pointerScrolling;

		switch (direction) {
			case winUp:
				BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
								    g->BrowserAPI.BrowserWidget),
								    g->ScrollY - pointer->speed
								    /*BrowserLineheightValue()*/, g);
				break;
			case winDown:
				BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
								    g->BrowserAPI.BrowserWidget),
								    g->ScrollY + pointer->speed
								    /*BrowserLineheightValue()*/, g);
				break;
			case winRight:
				BrowserScrollHorizontalTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
								    g->BrowserAPI.BrowserWidget),
								    g->ScrollX + pointer->speed
								    /*BrowserLineheightValue()*/, g);
				break;
			case winLeft:
				BrowserScrollHorizontalTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
								    g->BrowserAPI.BrowserWidget),
								    g->ScrollX - pointer->speed
								    /*BrowserLineheightValue()*/, g);
				break;
			default:
				break;
		}

		PointerAddMouseOver(pointer->x + xofs, pointer->y + yofs);
	} else {
		if (pointer->lastState == pointerScrolling) {
			PointerSpeedReset(&g->pointer, g_prefs.fiveWaySpeed, g);
		}

		pointer->lastState = pointerNavigating;

		// TODO: Check to see if the user's scrolling triggered a "flip", if so, decrement scroll value
		if (pointer->flipCount > 1)
			flipFact = pointer->flipCount;

		switch (direction) {
			case winUp:
				point.y -= pointer->speed / flipFact; // ((pointer->space / 2) * HiresFactor());
				PointerSetValueInBounds(&point, &bounds, pointer->space);
				pointer->x = point.x;
				pointer->y = point.y;
				break;
			case winDown:
				point.y += pointer->speed / flipFact; // ((pointer->space / 2) * HiresFactor());
				PointerSetValueInBounds(&point, &bounds, pointer->space);
				pointer->x = point.x;
				pointer->y = point.y;
				break;
			case winRight:
				point.x += pointer->speed / flipFact; // ((pointer->space / 2) * HiresFactor());
				PointerSetValueInBounds(&point, &bounds, pointer->space);
				pointer->x = point.x;
				pointer->y = point.y;
				break;
			case winLeft:
				point.x -= pointer->speed / flipFact; // ((pointer->space / 2) * HiresFactor());
				PointerSetValueInBounds(&point, &bounds, pointer->space);
				pointer->x = point.x;
				pointer->y = point.y;
				break;
			default:
				break;
		}

		PointerAddMouseOver(pointer->x + xofs, pointer->y + yofs);
	}

	updateScreen(g);
}
