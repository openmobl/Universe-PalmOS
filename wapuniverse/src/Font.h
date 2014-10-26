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
#ifndef _FONT_H_
#define _FONT_H_

#include 	<PalmOS.h>
#include 	"WAPUniverse.h"

extern void FontLoadFont(FontID fntID, DmResID resID);
extern void FontUnloadFont(FontID fntID, DmResID resID);
extern UInt16 FontCharHeight(void);
extern UInt16 FontCharWidth(Char chr);
extern UInt16 FontCharsWidth(Char *str, UInt16 length);
extern void FontCharsInWidth(Char const *string, Int16 *stringWidthP, Int16 *stringLengthP, Boolean *fitWithinWidth);
extern UInt16 FontWordWrap(Char const *chars, UInt16 maxWidth);

#endif
