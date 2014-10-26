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
#include	"Adjustor.h"
#include	"Layout.h"
#include	"PalmUtils.h"
#include	"Browser.h"

// The Adjustor is responsible for adjusting non-Left aligned content before
// a page break. The Adjustor also adjusts the values for interactable objects
// (such as links, images, form objects, etc.)


void AdjAlignLine(GlobalsType *g) {
	RectangleType		adjustZone;
	RectangleType		eraseZone;
	RectangleType		rect;
	Coord			destX = 0;
	Coord			destY = g->y;
	Coord			xOff = ((g->horizIndent != NULL)?g->horizIndent->x:0);// / 2;
	UInt16			zoneH = (g->imgH > g->fontH)?g->imgH:g->fontH;
	UInt16			tempH = (g->horizIndent != NULL)?g->horizIndent->imgH:0;

	if ((g->x > browserGetWidgetBounds(DRAW_WIDTH)) &&
	    (g->Align != Left))
		g->x = browserGetWidgetBounds(DRAW_WIDTH) - 1;


	if ((tempH > g->fontH) && (tempH > g->imgH)) {
		zoneH = tempH;
	} else if (g->fontH > g->imgH) {
		zoneH = g->fontH;
	} else {
		zoneH = g->imgH;
	}

	//if (g->horizIndent) {
	//	if (g->horizIndent->y < g->y)
	//		destY = g->horizIndent->y;
	//}
	
	adjustZone.topLeft.x = (g->horizIndent != NULL)?g->horizIndent->x:0;
	adjustZone.topLeft.y = g->y; // (g->horizIndent != NULL)?g->horizIndent->y:g->y;
	adjustZone.extent.x = g->x - ((g->horizIndent != NULL)?g->horizIndent->x:0);
	adjustZone.extent.y = zoneH;

	switch (g->Align) {
		case Center:
			destX = ((browserGetWidgetBounds(DRAW_WIDTH) - 1) - g->x) / 2;
			destX += xOff;
			break;
		case Right:
			destX = (browserGetWidgetBounds(DRAW_WIDTH) - 1) - g->x;
			destX += xOff;
			break;
		case Left:
		default:
			return; // no need to do anything
			break;
	}

	eraseZone.topLeft.x = ((g->horizIndent != NULL)?g->horizIndent->x:0) - g->ScrollX + browserGetWidgetBounds(WIDGET_TOPLEFT_X);
	eraseZone.topLeft.y = g->y - g->ScrollX + browserGetWidgetBounds(WIDGET_TOPLEFT_Y); // (g->horizIndent != NULL)?g->horizIndent->y:g->y;
	eraseZone.extent.x = browserGetWidgetBounds(DRAW_WIDTH); //destX - xOff; // for now just erase the whole line
	eraseZone.extent.y = zoneH;

	BrowserGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), &rect);

	WinPushDrawState();
	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	
	WinSetClip(&rect);
	WinEraseRectangle(&eraseZone, 0);
	//WinEraseRectangle(&adjustZone, 0);
	WinResetClip();

	WinPopDrawState();

	destX -= xOff;

	// Handle object adjustment
	AdjFixHyperlinks(destX, destY, g);
	AdjFixImages(destX, destY, g);
	AdjFixInput(destX, destY, g);
	AdjFixSelect(destX, destY, g);
	AdjFixRadioInput(destX, destY, g);
	AdjFixCheckInput(destX, destY, g);
	AdjFixButtons(destX, destY, g);
	AdjFixDOM(destX, destY, g);

	updateScreen(g);

}

void AdjFixHyperlinks(Coord adjX, Coord adjY, GlobalsType *g)
{
	HyperLinkPtr 	tmp;
	UInt16		y1, y2, diff;
	UInt16		ymax = g->y + ((g->imgH > g->fontH)?g->imgH:g->fontH);
	
  	tmp = g->HyperLinks;

	while(tmp != NULL)
	{
		y1 = tmp->y1; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
		y2 = tmp->y2; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);

		diff = tmp->x2 - tmp->x1;

		if ((y1 >= g->y) && (y1 <= ymax)) {
			tmp->x1 += adjX;
		}

		if ((y2 >= (g->y + 1)) && (y2 <= ymax) && (y2 != 0)) {
			tmp->x2 = tmp->x1 + diff;
		}

		tmp = tmp->next;
	}
}

void AdjFixImages(Coord adjX, Coord adjY, GlobalsType *g)
{
	ImagePtr 	tmp;
	UInt16		y1, y2, diff;
	UInt16		ymax = g->y + ((g->imgH > g->fontH)?g->imgH:g->fontH);
	
  	tmp = g->pageImages;

	while(tmp != NULL)
	{
		y1 = tmp->y1; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
		y2 = tmp->y2; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);

		diff = tmp->x2 - tmp->x1;

		if ((y1 >= g->y) && (y1 <= ymax)) {
			tmp->x1 += adjX;
		}

		if ((y2 >= (g->y + 1)) && (y2 <= ymax) && (y2 != 0)) {
			tmp->x2 = tmp->x1 + diff;
		}

		tmp = tmp->next;
	}
}

void AdjFixInput(Coord adjX, Coord adjY, GlobalsType *g)
{
	InputPtr 	tmp;
	UInt16		y1, y2, diff;
	UInt16		ymax = g->y + ((g->imgH > g->fontH)?g->imgH:g->fontH);
	
  	tmp = g->Input;

	while(tmp != NULL)
	{
		y1 = tmp->y1; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
		y2 = tmp->y2; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);

		diff = tmp->x2 - tmp->x1;

		if ((y1 >= g->y) && (y1 <= ymax)) {
			tmp->x1 += adjX;
		}

		if ((y2 >= (g->y + 1)) && (y2 <= ymax) && (y2 != 0)) {
			tmp->x2 = tmp->x1 + diff;
		}

		tmp = tmp->next;
	}
}

void AdjFixSelect(Coord adjX, Coord adjY, GlobalsType *g)
{
	SelectPtr 	tmp;
	UInt16		y1, y2, diff;
	UInt16		ymax = g->y + ((g->imgH > g->fontH)?g->imgH:g->fontH);
	
  	tmp = g->Select;

	while(tmp != NULL)
	{
		y1 = tmp->y1; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
		y2 = tmp->y2; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);

		diff = tmp->x2 - tmp->x1;

		if ((y1 >= g->y) && (y1 <= ymax)) {
			tmp->x1 += adjX;
		}

		if ((y2 >= (g->y + 1)) && (y2 <= ymax) && (y2 != 0)) {
			tmp->x2 = tmp->x1 + diff;
		}

		tmp = tmp->next;
	}
}

void AdjFixRadioInput(Coord adjX, Coord adjY, GlobalsType *g)
{
	RadioInputPtr 	tmp;
	UInt16		y1, y2, diff;
	UInt16		ymax = g->y + ((g->imgH > g->fontH)?g->imgH:g->fontH);
	
  	tmp = g->radioBtn;

	while(tmp != NULL)
	{
		y1 = tmp->y1; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
		y2 = tmp->y2; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);

		diff = tmp->x2 - tmp->x1;

		if ((y1 >= g->y) && (y1 <= ymax)) {
			tmp->x1 += adjX;
		}

		if ((y2 >= (g->y + 1)) && (y2 <= ymax) && (y2 != 0)) {
			tmp->x2 = tmp->x1 + diff;
		}

		tmp = tmp->next;
	}
}

void AdjFixCheckInput(Coord adjX, Coord adjY, GlobalsType *g)
{
	CheckInputPtr 	tmp;
	UInt16		y1, y2, diff;
	UInt16		ymax = g->y + ((g->imgH > g->fontH)?g->imgH:g->fontH);
	
  	tmp = g->checkBox;

	while(tmp != NULL)
	{
		y1 = tmp->y1; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
		y2 = tmp->y2; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);

		diff = tmp->x2 - tmp->x1;

		if ((y1 >= g->y) && (y1 <= ymax)) {
			tmp->x1 += adjX;
		}

		if ((y2 >= (g->y + 1)) && (y2 <= ymax) && (y2 != 0)) {
			tmp->x2 = tmp->x1 + diff;
		}

		tmp = tmp->next;
	}
}

void AdjFixButtons(Coord adjX, Coord adjY, GlobalsType *g)
{
	ButtonInputPtr 	tmp;
	UInt16		y1, y2, diff;
	UInt16		ymax = g->y + ((g->imgH > g->fontH)?g->imgH:g->fontH);
	
  	tmp = g->formButton;

	while(tmp != NULL)
	{
		y1 = tmp->y1; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
		y2 = tmp->y2; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);

		diff = tmp->x2 - tmp->x1;

		if ((y1 >= g->y) && (y1 <= ymax)) {
			tmp->x1 += adjX;
		}

		if ((y2 >= (g->y + 1)) && (y2 <= ymax)) {
			tmp->x2 = tmp->x1 + diff;
		}

		tmp = tmp->next;
	}
}

void AdjFixDOM(Coord adjX, Coord adjY, GlobalsType *g)
{
	DOMPtr	 	tmp;
	UInt16		y1, y2, diff;
	UInt16		ymax = g->y + ((g->imgH > g->fontH)?g->imgH:g->fontH);
	
  	tmp = g->DOM;

	while(tmp != NULL)
	{
		y1 = tmp->bounds.topLeft.y; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);
		y2 = tmp->bounds.topLeft.y + tmp->bounds.extent.y; // - browserGetWidgetBounds(WIDGET_TOPLEFT_Y);

		diff = tmp->bounds.extent.x;

		if ((y1 >= g->y) && (y1 <= ymax)) {
			tmp->bounds.topLeft.x += adjX;
		}

		//if ((y2 >= (g->y + 1)) && (y2 <= ymax) && (y2 != 0)) {
		//	tmp->x2 = tmp->x1 + diff;
		//}

		tmp = tmp->next;
	}
}



