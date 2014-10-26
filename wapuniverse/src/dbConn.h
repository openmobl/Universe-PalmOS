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
#ifndef _dbconn_h_
#define _dbconn_h_

#include "WAPUniverse.h"

#define dbConnDBType   		'ConW'          // type for application database.  must be 4 chars, mixed case.
#define dbConnDBName   		"Universe-Conn" // name for application database.  up to 31 characters.
#define dbConnNewConnSize	6       	// initial size for new database record.



void dbConnPackConnection (dbConnConnection * conn, MemHandle connDBEntry) SEC_1;
void dbConnUnPackConnection (dbConnConnection * conn, dbConnPackedConnection * packedConnection) SEC_1;
void dbConnOpenOrCreateDB(UInt16 mode, GlobalsType *g) SEC_1;
Boolean dbConnCreateRecord (GlobalsType *g) SEC_1;
Boolean dbConnDeleteCurrentRecord ( GlobalsType *g) SEC_1;
Err dbConnSortRecord (DmOpenRef dbP, UInt16 *indexP) SEC_1;
void dbConnReadCurrentConnection (GlobalsType *g) SEC_1;
void dbConnGetConnectionsList (GlobalsType *g) SEC_1;
void dbConnDisposeConnectionsList(GlobalsType *g) SEC_1;
void dbConnGetConnection(char *connstr, dbConnConnection *conn, GlobalsType *g) SEC_1;
UInt16 dbConnNumRecords(GlobalsType *g) SEC_1;

#endif
