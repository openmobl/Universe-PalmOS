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
/********************************************************************************
 * Developer's Note:                                                            *
 * This is not a true DOM structure. This is an application specific linked     *
 * list of all of the elements that WAPUniverse uses to construct a webpage.    *
 * Eventually WAPUniverse will use a true DOM.                                  *
 ********************************************************************************/

#include 	<PalmOS.h>
#include	"DOM.h"
#include 	"BrowserAPI.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"build.h"
#include	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"
#include	"Layout.h"
#include	"Form.h"
#include	"variable.h"
#include	"Browser.h"
#include	"Font.h"
#include	"palmpng.h"
#include	"Pointer.h"
#include	"BitmapV3.h"


void DOMAddElement(void *ptr, UInt16 type, UInt16 x, UInt16 y, UInt16 width, UInt16 height, GlobalsType *g)
{
	DOMPtr	tmp, idx;

	tmp = Malloc(sizeof(DOMType));
	if (!tmp)
		return; // need to throw an error since an element is missing? Nah. Just won't get rendered

	tmp->type = type;
	switch (type) {
		case DOM_UI_TEXT:
			tmp->ptr.t = ptr;
			break;
		case DOM_UI_INPUT:
			tmp->ptr.i = ptr;
			break;
		case DOM_UI_SELECT:
			tmp->ptr.s = ptr;
			break;
		case DOM_UI_IMAGE:
			tmp->ptr.p = ptr;
			break;
		case DOM_UI_BUTTON:
			tmp->ptr.b = ptr;
			break;
		case DOM_UI_RADIO:
			tmp->ptr.r = ptr;
			break;
		case DOM_UI_CHECK:
			tmp->ptr.c = ptr;
			break;
		case DOM_UI_AREA:
			tmp->ptr.a = ptr;
			break;
		default:
			break;
	}
	RctSetRectangle(&tmp->bounds, x, y, width, height);

	tmp->next = NULL;

	if (!g->DOM) {
		g->DOM = tmp;
	} else {
		idx = g->DOM;
		while (idx->next != NULL) {
		   	idx = idx->next;
		}
		idx->next = tmp;
	}
}

void DOMRemoveElement(GlobalsType *g)
{

}

void DOMInsertElement(GlobalsType *g)
{

}

void DOMDeleteTree(GlobalsType *g)
{
	DOMPtr	tmp;

	while (g->DOM != NULL){
		tmp = g->DOM;
		g->DOM = tmp->next;

		//tmp->bounds = {{0, 0}, {0, 0}};
		tmp->type   = DOM_UI_NULL;
		//tmp->ptr    = NULL;

		Free(tmp);
	}
	g->DOM = NULL;
}

void DOMRenderDOM(FormPtr frm, UInt16 gadget, Int16 x1, Int16 y1, Int16 x2, Int16 y2,
		  Boolean buffer, Boolean drawPointer, GlobalsType *g)
{
	DOMPtr			ptr;
	PointType 		pt;
    	RectangleType 		rect;
	RectangleType		win;
	WinHandle		tmpDrawWin = NULL;
	WinHandle		prevDrawWin = NULL;
	Err			error = errNone;
	IndexedColorType	background;


	if (!g->DOM)
		goto dom_render_cleanup;

	if (g->InputFldActive)
		goto dom_render_cleanup;
	if (WinGetActiveWindow() != FrmGetWindowHandle(FrmGetFormPtr(frmBrowser))) // TODO: create a variable with browser form id and chack
		goto dom_render_cleanup;

    	// set clipping bounds
    	BrowserGetObjectBounds(frm, gadget, &rect);

	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	if (buffer) {
    		win.topLeft.x = browserGetWidgetBounds(WIDGET_TOPLEFT_X);
    		win.topLeft.y = browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
    		win.extent.x = browserGetWidgetBounds(WIDGET_WIDTH);
    		win.extent.y = browserGetWidgetBounds(WIDGET_HEIGHT);

		prevDrawWin = WinGetDrawWindow();
		tmpDrawWin = WinCreateOffscreenWindow(browserGetScreenWidth(), browserGetScreenHeight(),
					     nativeFormat, &error);
		if (!error) {
			WinSetDrawWindow(tmpDrawWin);
        		WinEraseWindow();
			WinCopyRectangle(prevDrawWin, tmpDrawWin, &win,
					 browserGetWidgetBounds(WIDGET_TOPLEFT_X), 
					 browserGetWidgetBounds(WIDGET_TOPLEFT_Y),
					 winPaint);
		}
	}


    	pt = rect.topLeft;

	rect.topLeft.x += x1;
    	rect.topLeft.y += y1;
    	rect.extent.x = MIN(x2, rect.extent.x) - x1;
    	rect.extent.y = MIN(y2, rect.extent.y) - y1;

    	//WinSetClip(&rect);

	background = WinSetForeColor(0);
	WinEraseRectangle(&rect, 0);
	WinSetForeColor(background);

	ptr = g->DOM;

	for (; ptr; ptr = ptr->next) {
		RectangleType bounds = ptr->bounds;

		if((bounds.topLeft.y + bounds.extent.y) < (y1 + g->ScrollY))
			continue;

		if(bounds.topLeft.y > (y2 + g->ScrollY))
			break;

		/*if((bounds.topLeft.x + bounds.extent.x) < (x1 + g->ScrollX))
			continue;

		if(bounds.topLeft.x > (x2 + g->ScrollX))
			break;*/

		switch (ptr->type) {
			case DOM_UI_TEXT:
				DOMRenderText(ptr->ptr.t, &pt, &bounds, g);
				break;
			case DOM_UI_INPUT:
				DOMRenderInput(ptr->ptr.i, &pt, &bounds, g);
				break;
			case DOM_UI_SELECT:
				DOMRenderSelect(ptr->ptr.s, &pt, &bounds, g);
				break;
			case DOM_UI_IMAGE:
				DOMRenderImage(ptr->ptr.p, &pt, &bounds, g);
				break;
			case DOM_UI_BUTTON:
				DOMRenderButton(ptr->ptr.b, &pt, &bounds, g);
				break;
			case DOM_UI_RADIO:
				DOMRenderRadio(ptr->ptr.r, &pt, &bounds, g);
				break;
			case DOM_UI_CHECK:
				DOMRenderCheck(ptr->ptr.c, &pt, &bounds, g);
				break;
			case DOM_UI_AREA:
				DOMRenderArea(ptr->ptr.a, &pt, &bounds, g);
				break;
			default:
				break;
		}

	}

	if (drawPointer)
		PointerDrawPointer(&g->pointer, g);

	//WinResetClip();

	if (!error && buffer && tmpDrawWin && prevDrawWin) {
		WinCopyRectangle(tmpDrawWin, prevDrawWin, &win,
				 browserGetWidgetBounds(WIDGET_TOPLEFT_X), 
				 browserGetWidgetBounds(WIDGET_TOPLEFT_Y),
				 winPaint);
		WinSetDrawWindow(prevDrawWin);
        	WinDeleteWindow(tmpDrawWin, false);
	}

	WinPopDrawState();

	return;

dom_render_cleanup:
	if (drawPointer) {
		PointerDrawPointer(&g->pointer, g);
	}

	return;
}

void DOMRenderText(TextPtr text, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	Char			*str = NULL;
	UInt16			textLen = 0;
	UInt16			x = pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16			y = pt->y + bounds->topLeft.y - g->ScrollY;
	Boolean			italics = false;
	WinDrawOperation 	oldMode;
	IndexedColorType	textColor = UIColorGetTableEntryIndex(UIFieldText);
	IndexedColorType	foreColor = UIColorGetTableEntryIndex(UIObjectForeground);
	FontID			fntID = stdFont;

	if (!text)
		return;

	str	= text->text;
	textLen = StrLen(text->text);
	italics	= (text)?text->italic:false;

	if (text->link && (text->link == g->selectedLink)) {
		BrowserInverseLinkText(text, pt, bounds, g);
	} else {
		/*if ((text->font != stdFont) && (text->font != boldFont) &&
		    (text->font != largeFont) && (text->font != largeBoldFont)) {
			fntPtr = MemHandleLock(DmGetResource('nfnt', text->res));
			FntDefineFont(text->font, fntPtr);
			fntID = FntSetFont(text->font);
			MemPtrUnlock(fntPtr);
		} else {
			fntID = FntSetFont(text->font);
		}*/

		fntID = FntSetFont(text->font);
	
	
		oldMode = WinSetDrawMode(winOverlay);
	
		textColor = WinSetTextColor(text->color);
		foreColor = WinSetForeColor(text->color);
	
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

		WinSetTextColor(textColor);
		WinSetForeColor(foreColor);

		WinSetDrawMode(oldMode);
	
		FntSetFont(fntID);
	
		if (text->direction == directionRTL) {
			if (str != text->text)
				Free(str);
		}
	}
}

void DOMRenderImage(ImagePtr bmp, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	UInt16			x	= pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16			y	= pt->y + bounds->topLeft.y - g->ScrollY;
	//WinDrawOperation 	oldMode	= winPaint;
	BitmapPtr		resP	= NULL;

	if (!bmp)
		return;

DOMRenderBitmap_top:
	if (bmp->bitmap) {
		if (isHires() && !bmp->alreadyHires) {
			BitmapTypeV3 *bmp3;
	
			if (BmpGetDensity(bmp->bitmap) == kDensityDouble) {
				bmp->alreadyHires = true;
				goto DOMRenderBitmap_top;
			}

			//oldMode = WinSetDrawMode(winOverlay);

			bmp3 = BmpCreateV3Bitmap(bmp->bitmap, kDensityDouble, BmpGetBits(bmp->bitmap), BmpGetColortable(bmp->bitmap));
			if (!bmp3) {
				return;
			}

			if (bmp->channel && g->pngLibrary) {
				WinPaintAlphaImage(g->pngLibrary, (BitmapType *)bmp3, bmp->channel, x, y);
			} else {
				WinPaintBitmap((BitmapType *)bmp3,x,y);
			}

			BmpDeleteV3Bitmap((BitmapType *)bmp3);

			//WinSetDrawMode(oldMode);
		} else {
			//oldMode = WinSetDrawMode(winOverlay);

			if (bmp->channel && g->pngLibrary) {
				WinPaintAlphaImage(g->pngLibrary, bmp->bitmap, bmp->channel, x, y);
			} else {
				WinPaintBitmap(bmp->bitmap, x, y);
			}

			//WinSetDrawMode(oldMode);
		}
	} else if (bmp->download) {
		RectangleType		rect;
		RGBColorType		color;
		IndexedColorType	indexedColor;
		FontID			fntID;
		UInt16			res = narrowFontID, id = narrowFont;

		WinPushDrawState();

		if (g_prefs.fontSize != 2) {
			if (g_prefs.fontSize == 1) {		// small
				id = smallSmallFont;
				res = smallSmallFontID;
			} else {				// normal
				id = narrowFont;
				res = narrowFontID;
			}

			fntID = FntSetFont(id);
		} else {
			fntID = FntSetFont(stdFont);
		}

		color.r   = ( UInt8 ) 192;
    		color.g   = ( UInt8 ) 192;
    		color.b   = ( UInt8 ) 192;
    		indexedColor  = WinRGBToIndex( &color );
    		WinSetTextColor( indexedColor );
    		WinSetForeColor( indexedColor );

		WinSetDrawMode(winOverlay);

		rect.topLeft.x = x;// + (1 * HiresFactor());
		rect.topLeft.y = y;// + (1 * HiresFactor());
		rect.extent.x = bmp->width; // bounds->extent.x;
		rect.extent.y = bmp->height; // bounds->extent.y;


		if ((bmp->height >= (11 * HiresFactor())) && (bmp->width >= (11 * HiresFactor()))) {
			resP = MemHandleLock(DmGetResource(bitmapRsc, bmpLoadImage));
			if (resP) {
				WinDrawBitmap (resP, x + (2 * HiresFactor()), y + (2 * HiresFactor()));
				MemPtrUnlock(resP);
			}
		}

		if (bmp->alt) {
			Int16		strWidth = bmp->width - (12 * HiresFactor());
			Int16		strLen = StrLen(bmp->alt);
			Boolean		fitInWidth;

			if (strWidth > 0) {
				FntCharsInWidth(bmp->alt, &strWidth, &strLen, &fitInWidth);
				WinPaintChars(bmp->alt, strLen, x + (12 * HiresFactor()), y + (2 * HiresFactor()));
			}
		}

		if (isHires()) {
			WinPushDrawState();
			WinSetCoordinateSystem(kCoordinatesNative);

			WinDrawLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y);
			WinDrawLine(rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y + rect.extent.y - 1);
			WinDrawLine(rect.topLeft.x, rect.topLeft.y + rect.extent.y - 1, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y + rect.extent.y - 1);
			WinDrawLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x, rect.topLeft.y + rect.extent.y - 1);

			WinPopDrawState();
		} else {
			WinDrawRectangleFrame(simpleFrame, &rect); 
		}

		FntSetFont(fntID);

		WinPopDrawState();
	} else {
		UInt16	xPtr = x;
		UInt16	yPtr = y;

		ImageCreateEmptyImage(bmp->alt, &xPtr, &yPtr, &bmp->width, &bmp->height, true, g);
	}

	if (bmp->link && (bmp->link == g->selectedLink))
		BrowserInverseLinkImage(bmp, pt, bounds, g);
}

void DOMRenderInput(InputPtr input, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	UInt16			x = pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16			y = pt->y + bounds->topLeft.y - g->ScrollY;
	UInt16			width = bounds->extent.x;
	UInt16			height = bounds->extent.y;
	RectangleType 		rect;
	IndexedColorType   	indexedColor;
	Char			*value = NULL;
	FontID			currFont;

	if (!input)
		return;

	rect.topLeft.x = x;
	rect.extent.x = width;
	rect.topLeft.y = y;
	rect.extent.y = height;

	if (input->passwordType)
		currFont = FntSetFont(passwordFont);
	else
		currFont = FntSetFont(stdFont);

	value = getVar(input->name, g);

	if (!input->bkgrndTransparent) {
		indexedColor = WinSetBackColor(input->bkgrndColor);
		WinEraseRectangle(&rect, 0);
		WinSetBackColor(indexedColor);
	}

	// TODO: If the border is 5, make it 5
	if (input->border > 0) {
		//WinDrawRectangleFrame(simpleFrame, &rect);
		WinDrawLine(rect.topLeft.x - 1, rect.topLeft.y - 1, rect.topLeft.x + rect.extent.x, rect.topLeft.y - 1);
		WinDrawLine(rect.topLeft.x + rect.extent.x, rect.topLeft.y - 1, rect.topLeft.x + rect.extent.x, rect.topLeft.y + rect.extent.y);
		WinDrawLine(rect.topLeft.x - 1, rect.topLeft.y + rect.extent.y, rect.topLeft.x + rect.extent.x, rect.topLeft.y + rect.extent.y);
		WinDrawLine(rect.topLeft.x - 1, rect.topLeft.y - 1, rect.topLeft.x - 1, rect.topLeft.y + rect.extent.y);
	}

	rect.topLeft.x += 1;
	rect.extent.x -= 2;

	if (value)
		DrawToFit(value, rect, g);

	FntSetFont(currFont);
}

void DOMRenderSelect(SelectPtr select, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	UInt16			x = pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16			y = pt->y + bounds->topLeft.y - g->ScrollY;
	FontID			currFont;
	Int16 			stringWidthP = 79;
	Int16 			stringLengthP;
	Boolean 		fitWithinWidth;
	WinDrawOperation	oldMode;
	Char			*txt = NULL;
	Char			*value = NULL;
	OptionPtr		options;

	if (!select)
		return;

	value = getVar(select->name, g);
	if (value) {
		options = select->options;
		txt = NULL;
		while(options != NULL) {
			if (!StrCompare(value, options->value))
				txt = options->txt;
			options = options->next;
		}
	}

	oldMode = WinSetDrawMode(winOverlay);

	currFont = FntSetFont(stdFont);

	selectDrawTrigger(x, y);

	if (txt) {
		stringLengthP = StrLen(txt);

		FontCharsInWidth(txt, &stringWidthP, &stringLengthP, &fitWithinWidth);
		WinPaintChars(txt, stringLengthP, x + (11 * HiresFactor()), y);
	}

	FntSetFont(currFont);

	WinSetDrawMode(oldMode);
}

void DOMRenderButton(ButtonInputPtr button, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	UInt16	xPtr = pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16	yPtr = pt->y + bounds->topLeft.y - g->ScrollY;

	if (!button)
		return;

	if (button->image)
		return;

	formDrawButton(button->value, &xPtr, &yPtr, true, g);
}

void DOMRenderRadio(RadioInputPtr radio, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	UInt16	xPtr = pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16	yPtr = pt->y + bounds->topLeft.y - g->ScrollY;

	if (!radio)
		return;

	if (!StrNCompare(getVar(radio->name, g), radio->value, StrLen(radio->value))) {
		formDrawFullRadio(&xPtr, &yPtr, true, g);
	} else {
		formDrawEmptyRadio(&xPtr, &yPtr, true, g);
	}
}

void DOMRenderCheck(CheckInputPtr check, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	UInt16	xPtr = pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16	yPtr = pt->y + bounds->topLeft.y - g->ScrollY;

	if (!check)
		return;

	if (compareMultiVar(check->name, check->value, g)) {
		formDrawCheckedCheckbox(&xPtr, &yPtr, true, g);
	} else {
		formDrawUnCheckedCheckbox(&xPtr, &yPtr, true, g);
	}
}

void DOMRenderArea(AreaPtr area, PointType *pt, RectanglePtr bounds, GlobalsType *g)
{
	UInt16			x = pt->x + bounds->topLeft.x - g->ScrollX;
	UInt16			y = pt->y + bounds->topLeft.y - g->ScrollY;
	IndexedColorType 	oldBG = UIColorGetTableEntryIndex(UIFieldBackground);
	RectangleType		rect;

	if (!area)
		return;

	if (!area->noRefresh) {
		rect.topLeft.x = x;
		rect.topLeft.y = y;
		rect.extent.x  = bounds->extent.x;
		rect.extent.y  = bounds->extent.y;

		oldBG = WinSetBackColor(area->color);

		WinEraseRectangle(&rect, 0);

		WinSetBackColor(oldBG);
	}
}

void *DOMResourceToPtr(DmResType resType, DmResID resID)
{
	MemPtr	resP;
	void	*ret;

	resP = MemHandleLock(DmGetResource(resType, resID));
    	
	ret = Malloc(MemPtrSize(resP));
	if (!ret) {
		MemPtrUnlock(resP);
		return NULL;
	}

	MemMove(ret, resP, MemPtrSize(resP));
	
    	MemPtrUnlock(resP);

	return ret;
}
