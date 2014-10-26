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
#ifndef _wmls_h_
#define _wmls_h_

#include <PalmOS.h>
#include "wmls.h"
#include "iscript.h"
#include "WAPUniverse.h"

extern Err WMLSLibraryCall(UInt16 lindex, UInt8 findex, UInt16 *argcP, ScriptSlot *sp, ScriptSlot *rp) SEC_1;
extern Err WMLSUnloadModule(void *moduleP) SEC_1;
extern Err WMLSLoadModule(const Char *uri, UInt16 len, void **modulePP, UInt16 *offsetP) SEC_1;
extern Boolean callWMLScriptFunc(const Char *uri) SEC_1;

#endif
