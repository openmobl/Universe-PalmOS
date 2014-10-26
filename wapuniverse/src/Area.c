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
#include	"Area.h"
#include	"Layout.h"
#include	"DOM.h"

AreaPtr AreaAddArea(IndexedColorType bgColor, Boolean noRefresh, BitmapPtr image, UInt16 x, UInt16 y, GlobalsType *g)
{
	AreaPtr		tmp;

	tmp = Malloc(sizeof(AreaType));
	if (!tmp)
		return NULL;
	
	MemSet(tmp, sizeof(AreaType), 0);

	tmp->color = bgColor;
	tmp->image = image;
	tmp->noRefresh = noRefresh;
	tmp->x1 = (g->Align == Left)?x:0;
	tmp->y1 = y;
	tmp->x2 = browserGetWidgetBounds(DRAW_WIDTH) - x;
	tmp->y2 = y; // browserGetWidgetBounds(WIDGET_HEIGHT) - y;
	
	tmp->next = g->areaPtr;
	g->areaPtr = tmp;

	DOMAddElement(tmp, DOM_UI_AREA, tmp->x1, tmp->y1, tmp->x2, tmp->y2, g);

	return tmp;
}

void AreaSetWidth(AreaPtr area, UInt16 width, GlobalsType *g)
{
	DOMPtr	ptr;

	if (!area)
		return;

	switch (g->Align) {
		case Center:
			area->x1 = (browserGetWidgetBounds(DRAW_WIDTH) - width) / 2;
			break;
		case Right:
			area->x1 = browserGetWidgetBounds(DRAW_WIDTH) - width;
			break;
		case Left:
		default:
			break;
	}

	if (width) area->x2 = area->x1 + width;

	ptr = g->DOM;
	for (; ptr; ptr = ptr->next) {
		if (ptr->type == DOM_UI_AREA) {
			if (ptr->ptr.a == area) {
				ptr->bounds.extent.x = width;
				ptr->bounds.topLeft.x = area->x1;

				break;
			}
		}
	}
}

void AreaCloseArea(AreaPtr area, UInt16 x, UInt16 y, GlobalsType *g)
{
	DOMPtr	ptr;

	if (!area)
		return;

	if ((g->Align != Left) && (x != (browserGetWidgetBounds(DRAW_WIDTH) - (2 * area->x1))))
		x = browserGetWidgetBounds(DRAW_WIDTH) - (2 * area->x1);

	if (x && !area->x2) area->x2 = x;
	if (y) area->y2 = y;

	ptr = g->DOM;
	for (; ptr; ptr = ptr->next) {
		if (ptr->type == DOM_UI_AREA) {
			if (ptr->ptr.a == area) {
				if (((x - ptr->bounds.topLeft.x) > 0) && (!ptr->bounds.extent.x))
					ptr->bounds.extent.x = x - ptr->bounds.topLeft.x;
				ptr->bounds.extent.y = y - ptr->bounds.topLeft.y;

				if (ptr->bounds.extent.y <= 0) {
					ptr->bounds.extent.y = g->imgH;
				}

				break;
			}
		}
	}
}

void AreaApplyMinimumResolution(AreaPtr area, GlobalsType *g)
{
	DOMPtr	ptr;
	UInt16	x = 640;
	UInt16	y = 480;

	if (!area)
		return;

	if (area->x2 < (x + area->x1))
		area->x2 = area->x1 + x;
	if (area->y2 < (y + area->y1))
		area->y2 = area->y1 + y;

	ptr = g->DOM;
	for (; ptr; ptr = ptr->next) {
		if (ptr->type == DOM_UI_AREA) {
			if (ptr->ptr.a == area) {
				//if ((x - ptr->bounds.topLeft.x) > 0)
				//	ptr->bounds.extent.x = x - ptr->bounds.topLeft.x;
				//ptr->bounds.extent.y = y - ptr->bounds.topLeft.y;

				//if (ptr->bounds.extent.y <= 0) {
				//	ptr->bounds.extent.y = g->imgH;
				//}

				if (ptr->bounds.extent.x < x)
					ptr->bounds.extent.x = x;
				if (ptr->bounds.extent.y < y)
					ptr->bounds.extent.y = y;

				break;
			}
		}
	}
}

void AreaFreeAreas(GlobalsType *g)
{
	AreaPtr		tmp;

	while (g->areaPtr) {
		tmp = g->areaPtr;
		g->areaPtr = tmp->next;

		if (tmp->image)
			BmpDelete(tmp->image);
		
		MemSet(tmp, sizeof(AreaType), 0);

		Free(tmp);
	}

	g->areaPtr = NULL;
}

