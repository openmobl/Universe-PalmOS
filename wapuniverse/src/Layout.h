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
#ifndef _layout_h_
#define _layout_h_

#include <PalmOS.h>
#include "WAPUniverse.h"

#define WIDGET_NONE		0
#define	WIDGET_TOPLEFT_X	1
#define WIDGET_TOPLEFT_Y	2
#define WIDGET_WIDTH		3
#define WIDGET_HEIGHT		4
#define DRAW_WIDTH		5
#define DRAW_HEIGHT		6

extern void addNewAlignmnet(Alignment align, GlobalsType *g);
extern void removeLastAlignment(GlobalsType *g);
extern void freeAlignment(GlobalsType *g);

extern void addNewULList(GlobalsType *g);
extern void addNewOLList(Int16 olCount, Int16 olStart, Int16 olType, GlobalsType *g);
extern void removeLastList(GlobalsType *g);
extern void freeList(GlobalsType *g);

extern void freePageLocation(GlobalsType *g);
extern void addPageLocation(Char *id, GlobalsType *g);
extern void scrollToPageLocation(Char *name, GlobalsType *g);

extern void addNewFont(FontID font, UInt16 res, GlobalsType *g);
extern void removeLastFont(GlobalsType *g);
extern void freeFont(GlobalsType *g);
extern void setFontColor(Char *color);
extern void setFontColorFromStyle(Char *color);
extern UInt16 getFontHeight(GlobalsType *g);

extern UInt16 WinDrawCharsItalics(Char *chars, Int16 len, Coord x, Coord y);

extern void palmUnderline(Boolean ison);
extern void palmBold(Boolean isong);
extern void palmStrong(Boolean ison);
extern void palmBig(Boolean ison);
extern void palmSmall(Boolean ison);
extern UInt16 italicGetWidth(Char *str, UInt16 len);
extern void palmItalic(Boolean ison);
extern void palmEmphasis(Boolean ison);
extern void palmPre(Boolean ison);
extern void palmSuperscript(Boolean ison);
extern void palmRegular(Boolean ison);
extern void palmSubscript(Boolean ison);
extern void palmStrike(Boolean ison);
extern void palmCurrent(Boolean ison);

extern void xhtml_ReSetWindow(GlobalsType *g);

extern UInt16 browserGetWidgetBounds(UInt16 type);
extern UInt16 browserGetScreenWidth(void);
extern UInt16 browserGetScreenHeight(void);

extern Char *extractCSSElement(Char *element, Char *style);
#define extractBackground(x)	extractCSSElement("background-color:",x)
extern void addNewBackground(Char *color, GlobalsType *g);
extern void removeLastBackground(Boolean  redraw, GlobalsType *g);
extern void removeLastBodyBackground(Boolean  redraw, GlobalsType *g);
extern void removeLastPreDefBackground(Boolean redraw, GlobalsType *g);
extern void freeBackground(GlobalsType *g);
extern void setBackgroundColor(Char *color);
extern void setBackgroundColorFromStyle(Char *color);

extern UInt16 xhtmlReadWidth(Char *width);
extern void xhtmlDrawHR(UInt16 width, UInt16 size, Boolean shadow, IndexedColorType index, GlobalsType *g);

extern void addNewIndent(UInt16 x, UInt16 y, UInt16 h, GlobalsType *g);
extern void removeLastIndent(GlobalsType *g);
extern void freeIndent(GlobalsType *g);

extern void PositionAddNewPosition(UInt16 oldX, UInt16 oldY, UInt16 newX, UInt16 newY, UInt16 imgH, UInt16 indent, Boolean null, GlobalsType *g);
extern void PositionRemoveLastPosition(GlobalsType *g);
extern void PositionFreePosition(GlobalsType *g);
extern Boolean PositionIsIgnoreStartTag(Char *tag);
extern Boolean PositionIsIgnoreEndTag(Char *tag);
extern Boolean PositionIsCatchStartTag(Char *tag);


#endif