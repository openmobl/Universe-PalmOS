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

#include 	<PalmOS.h>
#include	"Font.h"
#include 	"BrowserAPI.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"build.h"
#include	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"
#include	"Layout.h"
#include	"Form.h"
#include	"variable.h"
#include	"Browser.h"

void FontLoadFont(FontID fntID, DmResID resID)
{
	FontPtr 	fntPtr = 0;

	fntPtr = MemHandleLock(DmGetResource('nfnt', resID));
	FntDefineFont(fntID, fntPtr);
	//FntSetFont(fntID);
	//MemPtrUnlock(fntPtr);
}

void FontUnloadFont(FontID fntID, DmResID resID)
{

}

// special Hires Fnt* functions

UInt16 FontCharHeight(void)
{
	UInt16	height = 0;

	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	height = FntCharHeight();

	WinPopDrawState();

	return height;
}

UInt16 FontCharWidth(Char chr)
{
	UInt16	width = 0;

	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	width = FntCharWidth(chr);

	WinPopDrawState();

	return width;
}

UInt16 FontCharsWidth(Char *str, UInt16 length)
{
	UInt16	width = 0;

	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	width = FntCharsWidth(str, length);

	WinPopDrawState();

	return width;
}

void FontCharsInWidth(Char const *string, Int16 *stringWidthP, Int16 *stringLengthP, Boolean *fitWithinWidth)
{
	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	FntCharsInWidth(string, stringWidthP, stringLengthP, fitWithinWidth);

	WinPopDrawState();

	return;
}

UInt16 FontWordWrap(Char const *chars, UInt16 maxWidth)
{
	UInt16	length = 0;

	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	length = FntWordWrap(chars, maxWidth);

	WinPopDrawState();

	return length;
}

