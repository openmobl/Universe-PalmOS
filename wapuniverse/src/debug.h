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
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include 	<PalmOS.h>
#include 	"WAPUniverse.h"
#include	"libhtmlparse.h"


#define debugOffMask        0x0000
#define debugGeneralMask	0x0001
#define debugParseMask		0x0010
#define	debugCommsMask		0x0100

//#define DEBUG_FULL
#define DEBUG_HTML
#define DEBUG_GENERAL

extern void debugStart(GlobalsType *g);
extern void debugOut(Char *str1, Char *str2, Int32 num1, Char *file, Int16 line, GlobalsType *g);
extern void debugEnd(GlobalsType *g);

extern void xhtml_debugStart(GlobalsType *g);
extern void xhtml_debugOut(Char *tag, struct ArgvTable *attributes, int numAttr, Boolean end, Char *comment, GlobalsType *g);
extern void xhtml_debugEnd(GlobalsType *g);

extern void DebugShowLoadInfo(GlobalsType *g);
extern void DebugShowHTTPHeaders(GlobalsType *g);
extern void DebugShowData(Char *title, Char *data);
extern void DebugShowMemUsage(GlobalsType *g);
extern void DebugShowGlobals(GlobalsType *g);

// These will pass in special flags soon

#define DebugOutFileURI		debugOut
#define DebugOutComms		debugOut
#define DebugOutInternal	debugOut

#endif
