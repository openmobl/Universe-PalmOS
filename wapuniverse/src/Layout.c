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
#include	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"PalmUtils.h"
#include 	"../res/WAPUniverse_res.h"
#include	"Layout.h"
#include	"color.h"
#include	"image.h"
#include	"Area.h"
#include	"BmpGlue.h"
#include	"Browser.h"
#include	"Font.h"


void addNewAlignmnet(Alignment align, GlobalsType *g)
{
	AlignPtr	tmp;

	tmp = Malloc(sizeof(AlignType));
	if (!tmp) return;

	tmp->currentAlignment = align;
	tmp->next = g->AlignList;

	g->AlignList = tmp;

	g->Align = align;

	return;
}

void removeLastAlignment(GlobalsType *g)
{
	AlignPtr	tmp;

	if (g->AlignList == NULL)
		return;

	tmp = g->AlignList;
	g->AlignList = tmp->next;

	tmp->currentAlignment = None;
	Free(tmp);

	if (g->AlignList != NULL) {
		g->Align = g->AlignList->currentAlignment;

		if (g->Align == None)
			g->Align = Left;
	} else {
		g->Align = Left;
	}
}

void freeAlignment(GlobalsType *g)
{
	AlignPtr	tmp;

	while (g->AlignList != NULL){
		tmp = g->AlignList;
		g->AlignList = tmp->next;

		tmp->currentAlignment = None;

		Free(tmp);
	}
	g->AlignList = NULL;

	g->Align = Left;
}


void addNewULList(GlobalsType *g)
{
	OLPtr	tmp;

	tmp = Malloc(sizeof(OLType));
	if (!tmp) return;

	tmp->olCount = 0;
	tmp->olStart = 0;
	tmp->olType = 0;
	tmp->isOL = false;
	tmp->isUL = true;
	tmp->indent = g->indent;
	tmp->next = g->organizedList;

	g->organizedList = tmp;

	return;
}

void addNewOLList(Int16 olCount, Int16 olStart, Int16 olType, GlobalsType *g)
{
	OLPtr	tmp;

	tmp = Malloc(sizeof(OLType));
	if (!tmp) return;

	tmp->olCount = olCount;
	tmp->olStart = olStart;
	tmp->olType = olType;
	tmp->isOL = true;
	tmp->isUL = false;
	tmp->indent = g->indent;
	tmp->next = g->organizedList;

	g->organizedList = tmp;

	return;
}

void removeLastList(GlobalsType *g)
{
	OLPtr	tmp;

	if (g->organizedList == NULL)
		return;

	tmp = g->organizedList;
	g->organizedList = tmp->next;

	tmp->olCount = 0;
	tmp->olStart = 0;
	tmp->olType = 0;
	tmp->isUL = false;
	tmp->isOL = false;
	tmp->indent = 0;
	Free(tmp);

	if (g->organizedList != NULL)
		g->indent = g->organizedList->indent;
	else
		g->indent = 0;
}

void freeList(GlobalsType *g)
{
	OLPtr	tmp;

	while (g->organizedList != NULL){
		tmp = g->organizedList;
		g->organizedList = tmp->next;

		tmp->olCount = 0;
		tmp->olStart = 0;
		tmp->olType = 0;

		Free(tmp);
	}
	g->organizedList = NULL;
	g->indent = 0;
}

// these probably shoul not be in here
void scrollToPageLocation(Char *name, GlobalsType *g)
{
  CardPtr tmp;

  	tmp = g->htmlPageLocID;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->card_id)==0)
		{
			/*g->ScrollY = tmp->offset;
    			if (g->ScrollY < 0)
        			g->ScrollY = 0;

    			if (g->ScrollY > (g->DrawHeight-browserGetWidgetBounds(WIDGET_HEIGHT)))
        			g->ScrollY = (g->DrawHeight-browserGetWidgetBounds(WIDGET_HEIGHT));

    			scrollUpdate(g);*/

			BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), tmp->offset, g);

			return;
		}
		tmp = tmp->next;
	}
    	return;
}

void addPageLocation(Char *id, GlobalsType *g)
{
	CardPtr		tmp, idx;

	tmp = Malloc(sizeof(CardType));
	if (!tmp) return;

	tmp->offset = g->y;
	tmp->next = NULL;

	tmp->card_id = Malloc(StrLen(id)+1);
	if (tmp->card_id)
		StrCopy(tmp->card_id,id);
	
	if ( g->htmlPageLocID == NULL) {
		g->htmlPageLocID = tmp;
	} else {
		idx = g->htmlPageLocID;
		while (idx->next != NULL) {
		   	idx = idx->next;
		}
		idx->next = tmp;
	}

	return;
}

void freePageLocation(GlobalsType *g)
{
	CardPtr		tmp;

	while (g->htmlPageLocID != NULL){
		tmp = g->htmlPageLocID;
		g->htmlPageLocID = tmp->next;
		if(tmp->card_id){
			Free(tmp->card_id);
		}
		tmp->offset = 0;
		Free(tmp);
	}
}

// TODO: Move font stuff to Font.c

void addNewFont(FontID font, UInt16 res, GlobalsType *g)
{
	WebFontPtr	tmp;
	//FontPtr 	fntPtr;

	tmp = Malloc(sizeof(WebFontType));
	if (!tmp) return;

	
	tmp->font = font;
	tmp->res = res;
	tmp->italic = (g->font) ? g->font->italic:false; // false; // make inherited
	tmp->underlined = (g->font) ? g->font->underlined:false; // false; // make inherited
	tmp->superScript = (g->font) ? g->font->superScript:false; // false; // make inherited
	tmp->subScript = (g->font) ? g->font->subScript:false; // false; // make inherited
	tmp->strike = (g->font) ? g->font->strike:false; // false; // make inherited
	tmp->color = (g->font) ? g->font->color:UIColorGetTableEntryIndex(UIFieldText);
	tmp->next = g->font;

	g->font = tmp;

	/*if ((font != stdFont) && (font != boldFont) &&
	    (font != largeFont) && (font != largeBoldFont)) {
		fntPtr = MemHandleLock(DmGetResource('nfnt', res));
		FntDefineFont(font, fntPtr);
		FntSetFont(font);
		MemPtrUnlock(fntPtr);
	} else {
		FntSetFont(font);
	}*/

	FntSetFont(font);

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();

	return;
}

void removeLastFont(GlobalsType *g)
{
	WebFontPtr	tmp;
	//FontPtr 	fntPtr;

	if (g->font == NULL)
		return;

	tmp = g->font;
	g->font = tmp->next;

	tmp->font = 0;
	tmp->res = 0;
	tmp->italic = false;
	tmp->underlined = false;
	tmp->superScript = false;
	tmp->subScript = false;
	tmp->strike = false;
	tmp->color = 0;
	Free(tmp);

	if (g->font != NULL) {
		/*if ((g->font->font != stdFont) && (g->font->font != boldFont) &&
		    (g->font->font != largeFont) && (g->font->font != largeBoldFont)) {
			fntPtr = MemHandleLock(DmGetResource('nfnt', g->font->res));
			FntDefineFont(g->font->font, fntPtr);
			FntSetFont(g->font->font);
			MemPtrUnlock(fntPtr);
		} else {
			FntSetFont(g->font->font);
		}*/
		FntSetFont(g->font->font);
	} else {
		//FntSetFont(stdFont);
		/*if (g_prefs.fontSize == 1) {		// small
			//fntPtr = MemHandleLock(DmGetResource('nfnt', smallNormalFontID));
			//FntDefineFont(smallNormalFont, fntPtr);
			FntSetFont(smallNormalFont);
			//MemPtrUnlock(fntPtr);
		} else if (g_prefs.fontSize == 2) {	// large
			FntSetFont(boldFont);
		} else {					// normal
			FntSetFont(stdFont);
		}*/
		palmRegular(true);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void freeFont(GlobalsType *g)
{
	WebFontPtr	tmp;

	while (g->font != NULL){
		tmp = g->font;
		g->font = tmp->next;

		tmp->font = 0;
		tmp->res = 0;
		tmp->italic = false;
		tmp->underlined = false;
		tmp->superScript = false;
		tmp->subScript = false;
		tmp->strike = false;
		tmp->color = 0;

		Free(tmp);
	}
	g->font = NULL;

	FntSetFont(stdFont);

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

UInt16 WinDrawCharsItalics(Char *chars, Int16 len, Coord x, Coord y)
{
	Err 			err = errNone;
	UInt8 			*bitsP = NULL, *bufP = NULL; // , *lineP = NULL;
	WinHandle 		winH = NULL, oldH = NULL;
	Int16 			lp = 0, xp = 0, rp = 0, rgb = 0, jump = 4, sp = 0;
	UInt16 			fontH = FntCharHeight();
	UInt16 			fontW = FntCharsWidth(chars, len);
	BitmapPtr 		bmpP, bmp3P;
	RectangleType 		rect = {{0,0},{0,0}};
	//WinDrawOperation 	oldMode;
	GlobalsType		*g;


	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	WinPushDrawState();

	rect.extent.x = (fontW + (fontH / jump)); // fontW + (fontH / 2);
	rect.extent.y = fontH;

	bmpP = BmpCreate((fontW + (fontH / jump))/*(fontW + (fontH / 2))*/, fontH, 16, NULL, &err);
	BmpSetTransparentValue(bmpP, (UInt32)0xFFFF);
	bitsP = BmpGetBits(bmpP);
	bmp3P = (BitmapPtr)BmpCreateBitmapV3(bmpP, kDensityDouble, bitsP, NULL);


	if (isHires())
		winH = WinCreateBitmapWindow(bmp3P, &err);
	else
		winH = WinCreateBitmapWindow(bmpP, &err);
	oldH = WinSetDrawWindow(winH);

	//oldBG = WinSetBackColor(g->xhtml.pageBG);
	WinSetDrawMode(winOverlay);

	//WinEraseRectangle(&rect, 0);

	WinPaintChars(chars, len, 0, 0);
	//WinSetDrawMode(oldMode);
	WinSetDrawWindow(oldH);
	WinDeleteWindow(winH, false);
	
	bufP = Malloc(sizeof(UInt8) * fontW * 2);
	if (!bufP) {
		//palmDrawChars(chars, len, x, y, g);
		BmpDelete(bmp3P);
		BmpDelete(bmpP);
		WinPopDrawState();
		return (FontCharsWidth(chars, len));
	}
	// this gets us the background color
	MemMove(&rgb, bitsP, sizeof(UInt16));
	
	for(lp = fontH - jump; lp >= 0; lp -= jump) {
		for(sp = 0; sp < jump; sp ++) {
			MemMove(bufP, bitsP + ((lp + sp) * (fontW + (fontH/jump)) * 2), fontW *2);
			MemMove(bitsP + (xp) + ((lp + sp) * (fontW + (fontH/jump)) *2 ), bufP, fontW *2);
			for(rp = 0; rp < xp; rp += 2) {
				MemMove(bitsP + (rp) + ((lp + sp) * (fontW + (fontH/jump)) * 2), &rgb, 2);
			}
		}
		xp += 2;
	}
	
	if (isHires())
		WinPaintBitmap(bmp3P, x, y);
	else
		WinPaintBitmap(bmpP, x, y);
	
	Free(bufP);
	BmpDelete(bmp3P);
	BmpDelete(bmpP);

	WinPopDrawState();

	return ((fontW + (fontH / jump)) / 2);
}

UInt16 getFontHeight(GlobalsType *g)
{
	UInt16	newFontH = 0;

	if (g->font && g->font->subScript) {//if (isSubscript == true) {
		newFontH = FontCharHeight() - 4;
	}

	return FontCharHeight() + newFontH;
}

void palmUnderline(Boolean ison)
{
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		if (g->font) {
			addNewFont(g->font->font, g->font->res, g);
			g->font->underlined = true;
		} else {
			palmRegular(true);
			g->font->underlined = true;
		}
	} else {
		//if (g->font) g->font->underlined = false;
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmBold(Boolean ison)
{
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//FntSetFont(boldFont);
		if (g_prefs.fontSize == 1) 		// small
			addNewFont(smallBoldFont, smallBoldFontID, g);
		else if (g_prefs.fontSize == 2) 	// large
			addNewFont(largeBoldFont, NULL, g);
		else					// normal
			addNewFont(boldFont, NULL, g);
	} else {
		//FntSetFont(stdFont);
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmStrong(Boolean ison)
{
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//FntSetFont(largeBoldFont);
		if (g_prefs.fontSize == 1) 		// small
			addNewFont(/*largeFont*/smallBoldFont, /*NULL*/smallBoldFontID, g);
		else if (g_prefs.fontSize == 2) 	// large
			addNewFont(largeBoldFont, NULL, g);
		else					// normal
			addNewFont(largeBoldFont, NULL, g);
	} else {
		//FntSetFont(stdFont);
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmBig(Boolean ison)
{
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//FntSetFont(largeFont);
		if (g_prefs.fontSize == 1) 		// small
			addNewFont(/*boldFont*/smallBoldFont, /*NULL*/smallBoldFontID, g);
		else if (g_prefs.fontSize == 2) 	// large
			addNewFont(largeBoldFont, NULL, g);
		else					// normal
			addNewFont(largeFont, NULL, g);
	} else {
		//FntSetFont(stdFont);
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmSmall(Boolean ison)
{
	//FontPtr fntnarrowFont;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//fntnarrowFont=MemHandleLock(DmGetResource('nfnt', narrowFontID));
		//FntDefineFont(narrowFont, fntnarrowFont);
		//FntSetFont(narrowFont);
		//MemPtrUnlock(fntnarrowFont);
		if (g_prefs.fontSize == 1) 		// small
			addNewFont(smallSmallFont, smallSmallFontID, g);
		else if (g_prefs.fontSize == 2) 	// large
			addNewFont(stdFont, NULL, g);
		else					// normal
			addNewFont(narrowFont, narrowFontID, g);
	} else {
		// FntSetFont(stdFont);
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmItalic(Boolean ison)
{
	//FontPtr fntitalicFont;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//fntitalicFont=MemHandleLock(DmGetResource('nfnt', italicFontID));
		//FntDefineFont(italicFont, fntitalicFont);
		//FntSetFont(italicFont);
	 	//MemPtrUnlock(fntitalicFont);
		//addNewFont(italicFont, italicFontID, g);
		if (g->font) {
			addNewFont(g->font->font, g->font->res, g);
			g->font->italic = true;
		} else {
			palmRegular(true);
			g->font->italic = true;
		}
	} else {
		//FntSetFont(stdFont);
		//removeLastFont(g);
		if (g->font) {
			g->font->italic = false;
			removeLastFont(g);
		}
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();

}

UInt16 italicGetWidth(Char *str, UInt16 len)
{
	UInt16	hires = (isHires())?2:1;

	return ((hires * FontCharsWidth(str, len)) + ((hires * FontCharHeight()) / 4)) / 2;
}

void palmEmphasis(Boolean ison)
{
	//FontPtr fntemphasisFont;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//fntemphasisFont=MemHandleLock(DmGetResource('nfnt', emphasisFontID));
		//FntDefineFont(emphasisFont, fntemphasisFont);
		//FntSetFont(emphasisFont);
	 	//MemPtrUnlock(fntemphasisFont);
		//addNewFont(emphasisFont, emphasisFontID, g);
		if (g_prefs.fontSize == 1) 		// small
			addNewFont(smallBoldFont, smallBoldFontID, g);
		else if (g_prefs.fontSize == 2) 	// large
			addNewFont(largeBoldFont, NULL, g);
		else					// normal
			addNewFont(boldFont, NULL, g);
		if (g->font) g->font->italic = true;
	} else {
		//FntSetFont(stdFont);
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmPre(Boolean ison)
{
	//FontPtr fntpreFont;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//fntpreFont=MemHandleLock(DmGetResource('nfnt', preFontID));
		//FntDefineFont(preFont, fntpreFont);
		//FntSetFont(preFont);
	 	//MemPtrUnlock(fntpreFont);
		if (g_prefs.fontSize == 1) 		// small
			addNewFont(smallPreformattedFont, smallPreformattedFontID, g);
		else
			addNewFont(preFont, preFontID, g);
	} else {
		//FntSetFont(stdFont);
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmSuperscript(Boolean ison)
{
	//FontPtr fntSupFont;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//fntSupFont=MemHandleLock(DmGetResource('nfnt', supFontID));
		//FntDefineFont(supFont, fntSupFont);
		//FntSetFont(supFont);
		//MemPtrUnlock(fntSupFont);
		addNewFont(supFont, supFontID, g);
		if (g->font) {
			//addNewFont(g->font->font, g->font->res, g);
			g->font->superScript = true;
		} else {
			palmRegular(true);
			g->font->superScript = true;
		}
	} else {
		//FntSetFont(stdFont);
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmSubscript(Boolean ison)
{
	//FontPtr fntSupFont;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//fntSupFont=MemHandleLock(DmGetResource('nfnt', supFontID));
		//FntDefineFont(supFont, fntSupFont);
		//FntSetFont(supFont);
		//MemPtrUnlock(fntSupFont);
		addNewFont(supFont, supFontID, g);
		if (g->font) {
			//addNewFont(g->font->font, g->font->res, g);
			g->font->subScript = true;
		} else {
			palmRegular(true);
			g->font->subScript = true;
		}
	} else {
		//FntSetFont(stdFont);
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmStrike(Boolean ison)
{
	//FontPtr fntSupFont;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison) {
		//fntSupFont=MemHandleLock(DmGetResource('nfnt', supFontID));
		//FntDefineFont(supFont, fntSupFont);
		//FntSetFont(supFont);
		//MemPtrUnlock(fntSupFont);
		//addNewFont(supFont, supFontID, g);
		if (g->font) {
			addNewFont(g->font->font, g->font->res, g);
			g->font->strike = true;
		} else {
			palmRegular(true);
			g->font->strike = true;
		}
	} else {
		//FntSetFont(stdFont);
		removeLastFont(g);
		//if (g->font) g->font->strike = false;
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmRegular(Boolean ison)
{
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	//FntSetFont(stdFont);
	if (ison) {
		if (g_prefs.fontSize == 1) 		// small
			addNewFont(smallNormalFont, smallNormalFontID, g);
		else if (g_prefs.fontSize == 2) 	// large
			addNewFont(boldFont, NULL, g);
		else					// normal
			addNewFont(stdFont, NULL, g);
	} else {
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void palmCurrent(Boolean ison)
{
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	//FntSetFont(stdFont);
	if (ison) {
		if (g->font) {
			addNewFont(g->font->font, g->font->res, g);
		} else {
			if (g_prefs.fontSize == 1) 		// small
				addNewFont(smallNormalFont, smallNormalFontID, g);
			else if (g_prefs.fontSize == 2) 	// large
				addNewFont(boldFont, NULL, g);
			else					// normal
				addNewFont(stdFont, NULL, g);
		}
	} else {
		removeLastFont(g);
	}

	if (FontCharHeight() > g->fontH)
		g->fontH = FontCharHeight();
}

void setFontColor(Char *color)
{
	RGBColorType 		rgb;
	IndexedColorType	index;
	GlobalsType 		*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	rgb = ConvertColorNumberString(color, TYPE_ELEMENT);
	
    	index = WinRGBToIndex( &rgb );

	if (g->font)
		g->font->color = index;
}

void setFontColorFromStyle(Char *color)
{
	Char			*fontColor = NULL;
	GlobalsType 		*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);


	fontColor = extractCSSElement("color:",color);
	if (fontColor != NULL) {
		setFontColor(fontColor);
	}
}

// TODO: Empty function, remove.
void xhtml_ReSetWindow(GlobalsType *g)
{
    	/*IndexedColorType 	oldBG = UIColorGetTableEntryIndex(UIFieldBackground);
   	//Err			error;
    	WinHandle		prevDrawWin;
	RectangleType		rect;

	if (g->xhtml.background->noRefresh)
		return;

	rect.topLeft.x = g->x;
	rect.topLeft.y = g->y;
	rect.extent.x  = browserGetWidgetBounds(WIDGET_WIDTH);
	rect.extent.y  = g->DrawHeight - g->y;

	prevDrawWin = WinGetDrawWindow();

	oldBG = WinSetBackColor(g->xhtml.pageBG);
	
	//WinDeleteWindow(g->DrawWin,false);
	//g->DrawWin = WinCreateOffscreenWindow(browserGetWidgetBounds(WIDGET_WIDTH),g->DrawHeight,nativeFormat,&error);

        WinSetDrawWindow(g->DrawWin);
        //WinEraseWindow();
	WinEraseRectangle(&rect, 0);
        WinSetDrawWindow(prevDrawWin);
	
	WinSetBackColor(oldBG);*/
}


UInt16 browserGetWidgetBounds(UInt16 type)
{
	GlobalsType 	*g;
	RectangleType 	rect;
	UInt16		ret = 0;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	FrmGetObjectBounds(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),g->BrowserAPI.BrowserWidget),&rect);

	ret = rect.extent.x;

	switch (type) {
		case WIDGET_TOPLEFT_X:
			ret = rect.topLeft.x * HiresFactor();
			break;
		case WIDGET_TOPLEFT_Y:
			ret = rect.topLeft.y * HiresFactor();
			break;
		case WIDGET_WIDTH:
			ret = rect.extent.x * HiresFactor();
			break;
		case WIDGET_HEIGHT:
			ret = rect.extent.y * HiresFactor();
			break;
		case DRAW_WIDTH:
			ret = g->DrawWidth;
			break;
		case DRAW_HEIGHT:
			ret = g->DrawHeight;
			//ret = rect.extent.y;
			break;
		default:
			break;
	}

	return ret;
}

UInt16 browserGetScreenWidth(void)
{
	Coord extentX = 160;
	Coord extentY = 160;

	WinGetDisplayExtent( &extentX, &extentY);

	return extentX;
}

UInt16 browserGetScreenHeight(void)
{
	Coord extentX = 160;
	Coord extentY = 160;

	WinGetDisplayExtent( &extentX, &extentY);

	return extentY;
}

Char *extractCSSElement(Char *element, Char *style)
{
	int	i = 0;
	Char	*element_ptr;
	Char	*temp;
	Char	*style_ptr = style;

top:
	element_ptr = StrStr(style_ptr, element);
	temp = element_ptr - 1;

	//TODO: need to check that when we call for 'color:' we don't get 'background-color:' 's value

	if ((element_ptr != NULL) && (element_ptr != style)) {
		if (*temp != ' ') {
			if (*temp != ';') {
				style_ptr = element_ptr + 1;
				goto top;
			}
		}
	} else if (element_ptr == NULL) {
		return NULL;
	}

	if (element_ptr) {
		element_ptr += StrLen(element);

		if (element_ptr[0] == ' ')
			element_ptr++;

		while (element_ptr[i] != ';' && element_ptr[i] != '\0' && i < StrLen(element_ptr))
			i++;

		if (element_ptr[i] == ';' || element_ptr[i] == '\0') {
			element_ptr[i] = '\0';

			return element_ptr;
		} else {
			return NULL;
		}
	}

	return NULL;
}
/*Char *extractBackground(Char *style)
{
	int	i = 0;
	Char	*background;

	background = StrStr(style, "background-color:");

	if (background) {
		background += StrLen("background-color:");

		if (background[0] == ' ')
			background++;

		while (background[i] != ';' && background[i] != '\0' && i < StrLen(background))
			i++;

		if (background[i] == ';' || background[i] == '\0') {
			background[i] = '\0';

			return background;
		} else {
			return NULL;
		}
	}

	return NULL;
}*/

void addNewBackground(Char *color, GlobalsType *g)
{
	BackgroundPtr	tmp;
	RGBColorType 	rgb;

	tmp = Malloc(sizeof(BackgroundType));
	if (!tmp) return;

	if (color != NULL) {
		rgb = ConvertColorNumberString(color, TYPE_PAGE);
		tmp->pageBG = WinRGBToIndex( &rgb );
	} else {
		if (g->xhtml.background != NULL) {
			tmp->pageBG = g->xhtml.background->pageBG;
		} else {
			rgb.r   = ( UInt8 ) 255;
    			rgb.g   = ( UInt8 ) 255;
    			rgb.b   = ( UInt8 ) 255;

			tmp->pageBG = WinRGBToIndex( &rgb );
		}
	}
	if (color)
		tmp->noRefresh = false; // (g->xhtml.background)?g->xhtml.background->noRefresh:false; // do we want this to be inherited?
	else
		tmp->noRefresh = true;

	if (!tmp->noRefresh)
		tmp->areaPtr = AreaAddArea(tmp->pageBG, tmp->noRefresh, NULL, g->x, g->y, g);
	else
		tmp->areaPtr = NULL;

	tmp->next = g->xhtml.background;

	g->xhtml.background = tmp;

	//g->Align = align;

	g->xhtml.pageBG = g->xhtml.background->pageBG;
	//xhtml_ReSetWindow(g);

	return;
}

void removeLastBackground(Boolean redraw, GlobalsType *g)
{
	BackgroundPtr	tmp;
	//RGBColorType	rgb;

	if (g->xhtml.background == NULL)
		return;

	tmp = g->xhtml.background;
	g->xhtml.background = tmp->next;

	if (tmp->areaPtr)
		AreaCloseArea(tmp->areaPtr, g->x, g->y, g);

	tmp->areaPtr = NULL;
	tmp->pageBG = 0;
	tmp->noRefresh = false;
	Free(tmp);
}

void removeLastBodyBackground(Boolean redraw, GlobalsType *g)
{
	BackgroundPtr	tmp;
	//RGBColorType	rgb;

	if (g->xhtml.background == NULL)
		return;

	tmp = g->xhtml.background;
	g->xhtml.background = tmp->next;

	tmp->areaPtr = NULL;
	tmp->pageBG = 0;
	tmp->noRefresh = false;
	Free(tmp);
}

void removeLastPreDefBackground(Boolean redraw, GlobalsType *g)
{
	BackgroundPtr	tmp;
	//RGBColorType	rgb;

	if (g->xhtml.background == NULL)
		return;

	tmp = g->xhtml.background;
	g->xhtml.background = tmp->next;

	if (tmp->areaPtr)
		AreaCloseArea(tmp->areaPtr, browserGetWidgetBounds(DRAW_WIDTH), g->y, g);

	tmp->areaPtr = NULL;
	tmp->pageBG = 0;
	tmp->noRefresh = false;
	Free(tmp);
}

void freeBackground(GlobalsType *g)
{
	BackgroundPtr	tmp;
	RGBColorType	rgb;

	rgb.r   = ( UInt8 ) 255;
    	rgb.g   = ( UInt8 ) 255;
    	rgb.b   = ( UInt8 ) 255;

	while (g->xhtml.background != NULL){
		tmp = g->xhtml.background;
		g->xhtml.background = tmp->next;

		tmp->pageBG = 0;
		tmp->noRefresh = false;

		Free(tmp);
	}
	g->xhtml.background = NULL;

	g->xhtml.pageBG = WinRGBToIndex( &rgb );
}

void setBackgroundColor(Char *color)
{
	RGBColorType 		rgb;
	GlobalsType 		*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (g->xhtml.background) {
		rgb = ConvertColorNumberString(color, TYPE_ELEMENT);
		g->xhtml.background->pageBG = WinRGBToIndex( &rgb );
		g->xhtml.pageBG = g->xhtml.background->pageBG;
		xhtml_ReSetWindow(g);
	}
}

void setBackgroundColorFromStyle(Char *color)
{
	Char			*bgColor = NULL;
	GlobalsType 		*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);


	bgColor = extractCSSElement("background-color:",color);
	if (bgColor != NULL) {
		setBackgroundColor(bgColor);
	}
}

UInt16 xhtmlReadWidth(Char *width)
{
	Char 	*temp = NULL;
	Char	*percent = NULL;
	UInt16	w = StrAToI(width);

	temp = Malloc(StrLen(width)+1);
	if (!temp)
		return StrAToI(width);

	StrCopy(temp, width);

	percent = StrChr(width, '%');

	if (percent) {
		UInt16	percentage = 100;
		*percent = '\0';

		percentage = StrAToI(temp);

		w = browserGetWidgetBounds(DRAW_WIDTH) * percentage / 100;
	}

	return w;
}

void xhtmlDrawHR(UInt16 width, UInt16 size, Boolean shadow, IndexedColorType index, GlobalsType *g)
{
	WinHandle		prevDrawWin, newDrawWin;
	UInt16			i = 0;
	IndexedColorType	shade = 0;
	UInt16			oldCoord = kCoordinatesStandard;
	UInt16  		offFact = 1;
	Boolean			hires = isHires();
	Err			error = 0;
	IndexedColorType	origFore = 0;
	//BitmapPtr		temp = NULL, winBmp = NULL;
	UInt16			y = 0;
			

	prevDrawWin = WinGetDrawWindow();
	newDrawWin = WinCreateOffscreenWindow(width, size, nativeFormat, &error);
	if (!newDrawWin)
		return;
    	WinSetDrawWindow(newDrawWin);

	origFore = WinSetForeColor(index);

	if (hires == true) {
		oldCoord = WinSetCoordinateSystem(kCoordinatesNative);
		offFact = 2;
	}

	while (i < size) {
		WinDrawLine(0, (y * offFact) + i, width * offFact, (y * offFact) + i);

		if (i == (size - 1) && shadow) {
			shade = WinSetForeColor(222);

			WinDrawLine(0, (y * offFact) + i, width * offFact, (y * offFact) + i);

			WinSetForeColor(shade);
		}
		i++;
	}

	if (hires == true)
		WinSetCoordinateSystem(oldCoord);

	WinSetForeColor(origFore);

	WinSetDrawWindow(prevDrawWin);

	/*winBmp = WinGetBitmap(newDrawWin);
	

	temp = Malloc(BmpSize(winBmp));
	if (temp) {
		ImagePtr	img;
		Char		*cTemp = (Char*)temp;

		MemMove(temp, winBmp, headerSizeP);
		MemMove(cTemp + headerSizeP, BmpGetBits(winBmp), dataSizeP);

		img = addPageImage(NULL, NULL, g->x, g->x + width, g->y, g->y + size, width, size, temp, g);
		//if (img)
		//	img->alreadyHires = true;
	}

	WinDeleteWindow(newDrawWin, true);*/


	{ // not a good idea, fail to delete the window
		ImagePtr	img;

		img = addPageImage(NULL, NULL, g->x, g->x + width, g->y, g->y + size, width, size, WinGetBitmap(newDrawWin), g);
	}

    	updateScreen(g);
}

void addNewIndent(UInt16 x, UInt16 y, UInt16 h, GlobalsType *g)
{
	HorizIndentPtr	tmp;

	if (g->imgAlign != Left)
		return;

	tmp = Malloc(sizeof(HorizIndentType));
	if (!tmp) return;


	tmp->x = x;
	tmp->y = y;
	tmp->h = h;
	tmp->imgH = g->imgH;
	tmp->align = g->imgAlign;

	tmp->next = g->horizIndent;
	g->horizIndent = tmp;

	g->xindent = (UInt32)x;
	g->imgY = (UInt32)y;
	g->imgH = 0;

	return;
}

void removeLastIndent(GlobalsType *g)
{
	HorizIndentPtr	tmp;

	if (g->horizIndent == NULL)
		return;

	tmp = g->horizIndent;
	g->horizIndent = tmp->next;

	tmp->x = 0;
	tmp->y = 0;
	tmp->h = 0;
	tmp->imgH = 0;
	tmp->align = None;
	Free(tmp);

	if (g->horizIndent != NULL) {
		g->xindent = (UInt32)g->horizIndent->x;
		g->imgY = (UInt32)g->horizIndent->y;
	} else {
		g->xindent = 0;
		g->imgY = 0;
	}
}

void freeIndent(GlobalsType *g)
{
	HorizIndentPtr	tmp;

	while (g->horizIndent != NULL){
		tmp = g->horizIndent;
		g->horizIndent = tmp->next;

		tmp->x = 0;
		tmp->y = 0;
		tmp->h = 0;
		tmp->imgH = 0;
		tmp->align = None;

		Free(tmp);
	}
	g->horizIndent = NULL;

	g->xindent = 0;
	g->imgY = 0;
}

void PositionAddNewPosition(UInt16 oldX, UInt16 oldY, UInt16 newX, UInt16 newY, UInt16 imgH, UInt16 indent, Boolean null, GlobalsType *g)
{
	PositionPtr	tmp;

	tmp = Malloc(sizeof(PositionType));
	if (!tmp) return;

	if (newX > (browserGetWidgetBounds(WIDGET_WIDTH) * ((isHires())?2:1))) {
		double ratio = 1;
		// rescale
		if (newX <= 1280 && newX > 1024) {
			ratio = newX / 1280;
			newX = browserGetWidgetBounds(WIDGET_WIDTH) * ratio;
			newY = newY * ratio;
		} else if (newX <= 1024 && newX > 800) {
			ratio = newX / 1024;
			newX = browserGetWidgetBounds(WIDGET_WIDTH) * ratio;
			newY = newY * ratio;
		} else if (newX <= 800 && newX > 640) {
			ratio = newX / 800;
			newX = browserGetWidgetBounds(WIDGET_WIDTH) * ratio;
			newY = newY * ratio;
		} else if (newX <= 640 && newX > 480) {
			ratio = newX / 640;
			newX = browserGetWidgetBounds(WIDGET_WIDTH) * ratio;
			newY = newY * ratio;
		} else if (newX <= 480) {
			ratio = newX / 480;
			newX = browserGetWidgetBounds(WIDGET_WIDTH) * ratio;
			newY = newY * ratio;
		}
	}

	tmp->x = oldX;
	tmp->y = oldY;
	tmp->h = imgH;
	tmp->null = null;
	tmp->indent = indent;

	tmp->next = g->position;

	g->position = tmp;

	if (null)
		return;

	g->x 		  = newX;
	g->y 		  = newY;
	g->positionIndent = indent; // += newX;
	g->imgH	= 0;


	return;
}

void PositionRemoveLastPosition(GlobalsType *g)
{
	PositionPtr	tmp;
	Boolean		null;

	if (!g->position) {
		g->positionIndent = 0;
		return;
	}

	tmp = g->position;
	g->position = tmp->next;

	null = tmp->null;


	tmp->x = 0;
	tmp->y = 0;
	tmp->h = 0;
	tmp->null = false;
	tmp->indent = 0;
	Free(tmp);


	if (!g->position)
		return;

	if ((((UInt16)g->x < (UInt16)g->position->x) || ((UInt16)(g->y + g->imgH) < (UInt16)(g->position->y + g->position->h))) &&
	    (null != g->position->null)) {
		g->x = g->position->x;
		g->y = g->position->y;
		g->imgH = g->position->h;
		g->positionIndent = g->position->indent;	
		if (g->positionIndent < 0)
			g->positionIndent = 0;
	} else {
		//if (g->positionIndent != g->position->indent)
			g->positionIndent = g->position->indent;
		if (g->positionIndent < 0)
			g->positionIndent = 0;
	}
}

void PositionFreePosition(GlobalsType *g)
{
	PositionPtr	tmp;

	while (g->position != NULL){
		tmp = g->position;
		g->position = tmp->next;

		tmp->x = 0;	
		tmp->y = 0;
		tmp->h = 0;
		tmp->indent = 0;

		Free(tmp);
	}
	g->position = NULL;

	//g->x = 0;
	//g->y = 0;
	g->imgH = 0;
	g->positionIndent = 0;
}

Boolean PositionIsIgnoreStartTag(Char *tag)
{
	Boolean	ignore = false;

	if (!StrNCaselessCompare(tag, "body", 4) || 
	    !StrNCaselessCompare(tag, "html", 4) ||
	    !StrNCaselessCompare(tag, "head", 4) ||
	    !StrNCaselessCompare(tag, "title", 5) ||
	    !StrNCaselessCompare(tag, "base", 4) ||
	    !StrNCaselessCompare(tag, "meta", 4) ||
	    !StrNCaselessCompare(tag, "br", 2) ||
	    !StrNCaselessCompare(tag, "style", 5) ||
	    !StrNCaselessCompare(tag, "script", 6) ||
	    //!StrNCaselessCompare(tag, "form", 4) ||
	    //!StrNCaselessCompare(tag, "fieldset", 8) ||
	    !StrNCaselessCompare(tag, "option", 6) ||
	    !StrNCaselessCompare(tag, "optgroup", 8) ||
	    //!StrNCaselessCompare(tag, "font", 4) ||
	    !StrNCaselessCompare(tag, "area", 4) ||
	    !StrNCaselessCompare(tag, "frame", 5) ||
	    !StrNCaselessCompare(tag, "link", 4) ||
	    !StrNCaselessCompare(tag, "sub", 3) ||
	    !StrNCaselessCompare(tag, "sup", 3)) {
		ignore = true;
	}

	return ignore;
}

Boolean PositionIsCatchStartTag(Char *tag)
{
	Boolean	catch = false;

	if (!StrNCaselessCompare(tag, "img", 3)) {
		catch = true;
	}

	return catch;
}

Boolean PositionIsIgnoreEndTag(Char *tag)
{
	Boolean	ignore = false;

	if (!StrNCaselessCompare(tag, "body", 4) || 
	    !StrNCaselessCompare(tag, "html", 4) ||
	    !StrNCaselessCompare(tag, "head", 4) ||
	    !StrNCaselessCompare(tag, "title", 5) ||
	    !StrNCaselessCompare(tag, "base", 4) ||
	    !StrNCaselessCompare(tag, "meta", 4) ||
	    !StrNCaselessCompare(tag, "br", 2) ||
	    !StrNCaselessCompare(tag, "style", 5) ||
	    !StrNCaselessCompare(tag, "script", 6) ||
	    //!StrNCaselessCompare(tag, "form", 4) ||
	    //!StrNCaselessCompare(tag, "fieldset", 8) ||
	    !StrNCaselessCompare(tag, "option", 6) ||
	    !StrNCaselessCompare(tag, "optgroup", 8) ||
	    //!StrNCaselessCompare(tag, "font", 4) ||
	    !StrNCaselessCompare(tag, "area", 4) ||
	    !StrNCaselessCompare(tag, "frame", 5) ||
	    !StrNCaselessCompare(tag, "link", 4) ||
	    !StrNCaselessCompare(tag, "sub", 3) ||
	    !StrNCaselessCompare(tag, "sup", 3) ||
	    !StrNCaselessCompare(tag, "img", 3)) {
		ignore = true;
	}

	return ignore;
}

