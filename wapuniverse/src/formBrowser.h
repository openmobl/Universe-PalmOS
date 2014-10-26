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
#ifndef _formbrowser_h_
#define _formbrowser_h_

#include    <PalmOS.h>
#include    <SysEvtMgr.h>
#include    "WAPUniverse.h"
#include    "wsp.h"

#define	lineheightValue	12	// default line height
#define columnValue	6	// character width

//#define LINEHEIGHT      BrowserLineheightValue()
//#define COL_WIDTH	BrowserColumnValue()
#define TAPTIMEOUT	((SysTicksPerSecond() * 3) / 2)
#define TAPNONE		0
#define TAPIMAGE	1
#define TAPLINK		2

Boolean ehFormBrowser(EventPtr event);
extern void browserSwitchMode(Int16 mode);
extern void palmprintln(GlobalsType *g);
extern void palminsertbr(GlobalsType *g);
extern void palmprintf(char *str, GlobalsType *g);
extern void palmsrcprintf(char *str, GlobalsType *g);
//extern void palmDrawChars(char *str, Int16 textLen, Int16 x, Int16 y, GlobalsType *g);
extern void palmGetPos(int* posx, int* posy, GlobalsType *g);
extern Boolean browserReloadEvt(GlobalsType *g);
extern Boolean browserStopEvt(GlobalsType *g);
extern Boolean browserSrcEvt(GlobalsType *g);
extern Boolean browserURLsEvt();
extern Boolean browserBackEvt(GlobalsType *g);
extern Boolean browserHomeEvt(GlobalsType *g);
extern FieldPtr SetFieldTextFromHandle(UInt16 fieldID, MemHandle txtH);
extern void updateScreen(GlobalsType *g);
extern void followLink(unsigned char *str, GlobalsType *g);
extern void showProgress(GlobalsType *g);
//extern Coord MWinDrawAlignChars( const char *str, UInt32 len, Coord x, Coord y, Alignment align, GlobalsType *g);
extern char *escaped(Char *str);
extern char *unescaped(Char *str);
extern Char *ConvertFromTo(CharEncodingType srcEncoding, Char *sourceStr, Char *destStr);
extern void showProgress(GlobalsType *g);
extern void scrollUpdate( GlobalsType *g);
extern void browserResizeDimensions(Int16 width, Int16 height, GlobalsType *g);
extern void browserSwitchSoftTwo(GlobalsType *g);
extern void browserSetTitle(Char *title);
extern void scrollUp(GlobalsType *g);
extern void scrollDown(GlobalsType *g);
extern void scrollLeft(GlobalsType *g);
extern void scrollRight(GlobalsType *g);
extern void palmResetState(GlobalsType *g);
extern void browserPushPageContent(GlobalsType *g);
extern void browserStateError(GlobalsType *g);
extern void browserPopPageContent(GlobalsType *g);
extern void browserPushPageContent(GlobalsType *g);
extern void browserResetURL(GlobalsType *g);
extern void setTitleFont(Boolean ison);
extern Boolean browserHandleOptionsList(Int16 selection, GlobalsType *g);
extern Boolean scrollField(GlobalsType *g);

extern void BrowserRenderDOM(FormPtr frm, UInt16 gadget, Int16 y1, Int16 y2, GlobalsType *g);

// move to Net.c
extern void NetCloseSocket(NetSocketRef *sock);

#endif
