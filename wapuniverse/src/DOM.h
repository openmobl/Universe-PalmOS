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
#ifndef _DOM_H_
#define _DOM_H_

#include 	<PalmOS.h>
#include 	"WAPUniverse.h"

extern void DOMAddElement(void *ptr, UInt16 type, UInt16 x, UInt16 y, UInt16 width, UInt16 height, GlobalsType *g);
extern void DOMRemoveElement(GlobalsType *g);
extern void DOMDeleteTree(GlobalsType *g);
extern void DOMRenderDOM(FormPtr frm, UInt16 gadget, Int16 x1, Int16 y1, Int16 x2, Int16 y2,
			 Boolean buffer, Boolean drawPointer, GlobalsType *g);
extern void DOMRenderText(TextPtr text, PointType *pt, RectanglePtr bounds, GlobalsType *g);
extern void DOMRenderImage(ImagePtr bmp, PointType *pt, RectanglePtr bounds, GlobalsType *g);
extern void DOMRenderInput(InputPtr input, PointType *pt, RectanglePtr bounds, GlobalsType *g);
extern void DOMRenderSelect(SelectPtr select, PointType *pt, RectanglePtr bounds, GlobalsType *g);
extern void DOMRenderButton(ButtonInputPtr button, PointType *pt, RectanglePtr bounds, GlobalsType *g);
extern void DOMRenderRadio(RadioInputPtr radio, PointType *pt, RectanglePtr bounds, GlobalsType *g);
extern void DOMRenderCheck(CheckInputPtr check, PointType *pt, RectanglePtr bounds, GlobalsType *g);
extern void DOMRenderArea(AreaPtr area, PointType *pt, RectanglePtr bounds, GlobalsType *g);

extern void *DOMResourceToPtr(DmResType resType, DmResID resID);

#endif
