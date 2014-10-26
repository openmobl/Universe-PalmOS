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
#ifndef _palmutils_h_
#define _palmutils_h_

#include    	<PalmOS.h>
#include	"WAPUniverse.h"

#define	NUL	'\0'

extern void *GetObjectPtr (UInt16 objectID);
extern void DrawCharsToFitWidth (char* s, RectanglePtr r, char c);
extern FieldPtr GetFocusObjectPtr (void);
extern Boolean FldGetFocus(UInt16 objectID);
extern void SetFieldFromStr(const Char *srcP , UInt16 fieldIndex);
extern char *rindex(char *str, char c);
extern void ExitApp(void);
extern Err SetFieldTextFromStr(FieldType *fieldP, Char *string, Boolean redraw);
extern Boolean isHires(void);
extern UInt32 strToInt(Char* strin, UInt16 base);
extern Char *StrToBase64(Char *ostr, Int32 len);
extern void DrawToFit(char *instr, RectangleType rect, GlobalsType *g);
extern UInt16 HiresFactor(void);

extern Char *StrCompareLower(Char *str, const Char *token);
extern Char *StrDup(Char *string);
extern Boolean StrEquals(Char *string1, Char *string2);
extern Char *StrRChr(Char *string, Char token);
extern Char *StrReverse(Char *str);

extern void FldEmulateEnter(UInt16 index);

extern Boolean SysCheck5Way(void);

extern Err NVFSSyncDatabase(DmOpenRef dbRef);
extern Boolean NVFSIsNVFS(void);
extern UInt32 NVFSGetNVFSFreeSize(void);

extern UInt32 GetFreeMemory(UInt32 *totalMemoryP, UInt32 *dynamicMemoryP, Boolean maxChunkSize );

#endif