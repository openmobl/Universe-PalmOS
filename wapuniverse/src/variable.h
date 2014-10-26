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
#ifndef _variable_h_
#define _variable_h_

#include    <PalmOS.h>
#include    <SysEvtMgr.h>
#include    "WAPUniverse.h"
#include    "wsp.h"

#define MULTI_VAR_IND	0x10


extern int  addVar(Char *name, GlobalsType *g);
extern void setVar(Char *name, Char *value, GlobalsType *g);
extern Char *getVar(Char *name, GlobalsType *g);
extern Boolean checkVar(Char *name, GlobalsType *g);
extern int disposeVar(GlobalsType *g);
extern unsigned char *expandVar(unsigned char *instr, GlobalsType *g);

extern Char *InputgetVar(Char *name, GlobalsType *g);

extern int  hyperlinkAddVar(Char *name, GlobalsType *g);
extern void hyperlinkSetVar(Char *name, Char *value, GlobalsType *g);
extern Char *hyperlinkGetVar(Char *name, GlobalsType *g);
extern int  hyperlinkAddPostVar(Char *name, GlobalsType *g);
extern void hyperlinkSetPostVar(Char *name, Char *value, GlobalsType *g);
extern Char *hyperlinkGetPostVar(Char *name, GlobalsType *g);

extern int  optionAddVar(Char *name, GlobalsType *g);
extern void optionSetVar(Char *name, Char *value, GlobalsType *g);
extern Char *optionGetVar(Char *name, GlobalsType *g);
extern int  optionAddPostVar(Char *name, GlobalsType *g);
extern void optionSetPostVar(Char *name, Char *value, GlobalsType *g);
extern Char *optionGetPostVar(Char *name, GlobalsType *g);

extern Boolean compareMultiVar(Char *name, Char *value, GlobalsType *g);
extern void removeMultiVar(Char *name, Char *value, GlobalsType *g);
extern void setMultiVar(Char *name, Char *value, GlobalsType *g);
extern Char *disectMultiVar(Char *value, Int16 *i);

extern Char *convertToVarString(Char *input);


#endif
