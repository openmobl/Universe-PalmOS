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
#ifndef _BROWSER_H_
#define _BROWSER_H_

#include 	<PalmOS.h>
#include 	"WAPUniverse.h"

extern void BrowserInitialiseGlobals(void);
extern void BrowserScrollTo(FormPtr frm, UInt16 gadget, Int16 pos, GlobalsType *g);
extern void BrowserScrollHorizontalTo(FormPtr frm, UInt16 gadget, Int16 pos, GlobalsType *g);
extern void BrowserUpdateScrollBar(FormPtr frm, GlobalsType *g);
extern void BrowserUpdateScrollBarHorizontal(FormPtr frm, GlobalsType *g);
extern void BrowserRefreshScrollBarHorizontal(GlobalsType *g);
extern Boolean BrowserHandlePenTap(Int16 x, Int16 y, Boolean penDown, GlobalsType *g);
extern Boolean BrowserHandlePenOver(Int16 x, Int16 y, GlobalsType *g);
extern void BrowserUpdateScreen(GlobalsType *g);
extern void BrowserGetObjectBounds(FormType *frm, UInt16 index, RectangleType *rect);
extern void BrowserRefreshScrollBars(GlobalsType *g);
extern void BrowserPopupList(Int16 x, Int16 y, UInt16 type, void *ptr, GlobalsType *g);
extern void BrowserInverseLinkImage(ImagePtr img, PointType *pt, RectanglePtr bounds, GlobalsType *g);
extern void BrowserInverseLinkText(TextPtr text, PointType *pt, RectanglePtr bounds, GlobalsType *g);

extern UInt16 BrowserIndentValue(void);
extern UInt16 BrowserLineheightValue(void);
extern UInt16 BrowserColumnValue(void);

extern GlobalsType *BrowserGetGlobals(void);

#endif
