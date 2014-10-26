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
#ifndef _dburl_h_
#define _dburl_h_

#include "WAPUniverse.h"


#define dbUrlDBType   		'UrlW'          // type for application database.  must be 4 chars, mixed case.
#define dbUrlDBName   		"Universe-Url"    // name for application database.  up to 31 characters.
#define dbUrlNewURLSize	6       		// initial size for new database record.

#define urlDbCorruptWarning "Your Bookmarks database was corrupted. " \
                            "The default categories have been restored. " \
                            "Some bookmarks may be missing or corrupted."

extern void dbUrlPackURL (dbUrlURL * url, MemHandle urlDBEntry) SEC_1;
extern void dbUrlUnPackURL (dbUrlURL * url, dbUrlPackedURL *packedURL) SEC_1;
extern Boolean dbUrlCreateRecord (GlobalsType *g, UInt16 *recIndex) SEC_1;
extern void dbUrlReadCurrentUrl (GlobalsType *g) SEC_1 ;
extern Err dbUrlSortRecord (DmOpenRef dbP, UInt16 *indexP) SEC_1;
extern Boolean dbUrlDeleteCurrentRecord (GlobalsType *g) SEC_1;
extern Err dbUrlInitAppInfo (DmOpenRef dbP, GlobalsType *g) SEC_1;
extern Boolean dbUrlCheckAppInfo(DmOpenRef dbP, GlobalsType *g);
extern void dbUrlOpenOrCreateDB(UInt16 mode, GlobalsType *g) SEC_1;
extern UInt16 dbUrlNumRecords(GlobalsType *g) SEC_1;

#endif
