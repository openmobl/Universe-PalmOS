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
#ifndef _history_h_
#define _history_h_

#include "WAP.h"
#include "WAPUniverse.h"
#include "../res/WAPUniverse_res.h"
#include <SysEvtMgr.h>

#define dbHistoryDBType   		'Hist'          // type for application database.  must be 4 chars, mixed case.
#define dbHistoryDBName			"Universe-History"
#define dbHistorySize			2 

#define dbHistoryFormat			"[url]:[title|0]" // a string defining the record layout, mostly for aiding the coder

extern int historyPush(Char *url, GlobalsType *g);
extern Char *historyPop();
extern void historyClear(GlobalsType *g);

extern void HistoryCreateHistoryFile(UInt16 mode, GlobalsType *g);
extern Boolean HistoryDeleteEntry(Char *url, UInt16 index, GlobalsType *g);
extern Boolean HistoryAddPageToHistory(Char *url, Char *title, GlobalsType *g);
extern Boolean HistoryGoToURL(Char *url, UInt16 index, Boolean closeForm, GlobalsType *g);
extern Boolean HistoryFindEntry(Char *url, GlobalsType *g);
extern Boolean HistoryClearHistory(GlobalsType *g);

#endif _history_h_
