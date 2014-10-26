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
#ifndef _TAB_H_
#define _TAB_H_

#include 	<PalmOS.h>
#include 	"WAPUniverse.h"


extern Boolean TabInitializeTabs(Char *url, Boolean active, GlobalsType *g);
extern Boolean TabCreateTab(Char *url, Boolean active, GlobalsType *g);
extern Boolean TabSwitchTab(UInt32 tabNumber, GlobalsType *g);
extern Boolean TabDeleteTab(UInt32 tabNumber, Boolean refresh, GlobalsType *g);
extern Boolean TabDeleteTabs(GlobalsType *g);
extern Boolean TabHandlePen(Coord x, Coord y, GlobalsType *g);
extern Boolean TabIsValidTab(UInt32 tabNumber, GlobalsType *g);
extern TabStackType *TabGetTabStack(UInt32 tabNumber, GlobalsType *g);
extern void TabDrawTabs(Boolean blankActive, GlobalsType *g);
extern void TabDrawProgress(BitmapPtr resP, GlobalsType *g);

// Internal functions
//extern Boolean TabSwapResources(UInt32 tabNumber, Boolean stackToGlobals, GlobalsType *g);
extern void TabSwapResources(UInt32 tabNumber, GlobalsType *g1, GlobalsType *g2);
extern Boolean TabFreeResources(TabStackType *stack, GlobalsType *g);

#endif
