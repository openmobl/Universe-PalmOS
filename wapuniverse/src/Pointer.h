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
#ifndef __POINTER_H__
#define __POINTER_H__

#include 	<PalmOS.h>
#include	"WAPUniverse.h"

// pointer flags
#define	pointerFlagsNone	0x00
#define	pointerGetsPenUp	0x10
#define	pointerPenDown		0x01
#define	pointerPenUp		0x02

extern void PointerInitialise(WindowPointerType *pointer, GlobalsType *g);
extern void PointerCleanUp(WindowPointerType *pointer, GlobalsType *g);
extern void PointerDrawPointer(WindowPointerType *pointer, GlobalsType *g);
extern void PointerUpdatePointer(WindowPointerType *pointer, GlobalsType *g);
extern void PointerChangeMode(WindowPointerType *pointer, PointerMode mode);
extern Boolean PointerIsAtEdge(WindowPointerType *pointer, WinDirectionType direction, BrowserAPIType *browser);
extern void PointerSetValueInBounds(PointType *point, RectangleType *bounds, Coord space);
extern void PointerScroll(WindowPointerType *pointer, WinDirectionType direction, GlobalsType *g);
extern void PointerSetPosition(Coord x, Coord y, WindowPointerType *pointer, Boolean mouseOver, GlobalsType *g);
extern void PointerSelectLocation(WindowPointerType *pointer, Int16 flags, GlobalsType *g);
extern void PointerAddMouseOver(Int16 x, Int16 y);
extern void PointerClearScreen(Boolean drawPointer, GlobalsType *g);
extern void PointerScrollUpdate(WindowPointerType *pointer, GlobalsType *g);
extern void PointerAddMouseEvent(UInt16 event, Int16 x, Int16 y);
extern void PointerSpeedAccelerate(WindowPointerType *pointer, UInt16 speed, GlobalsType *g);
extern void PointerSpeedReset(WindowPointerType *pointer, UInt16 speed, GlobalsType *g);
extern void PointerSetHolding(WindowPointerType *pointer, Boolean holding, GlobalsType *g);

#endif
