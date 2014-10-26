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
#include	<PalmOS.h>
#include	"xhtml.h"
#include	"../res/WAPUniverse_res.h"
#include	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"variable.h"
#include	"history.h"
#include	"Form.h"
#include	"PalmUtils.h"
#include	"Entity.h"
#include	"http.h"
#include	"color.h"
#include	"process.h"
#include	"image.h"
#include	"Layout.h"
#include	"Cache.h"
#include	"URL.h"

#include 	"libhtmlparse.h"

int xhtmltag (char *tag, struct ArgvTable *args, int numargs) {
        
	starttag (tag, args, numargs);
	endtag (tag);

        return 0;
}

int xmltag(char *tag, struct ArgvTable *args, int numargs)
{
	Char *attr;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	attr = getXHTMLAttrValue("encoding", args, numargs);
	if (attr) {

		if (StrCompareLower(attr, "utf-8")) {
			g->charSet = UTF_8;
		} else if (StrCompareLower(attr, "us-ascii")) {
			g->charSet = US_ASCII;
		} else if (StrCompareLower(attr, "iso-8859-1")) {
			g->charSet = ISO_8859_1;
		} else if (StrCompareLower(attr, "usc-2")) {
			g->charSet = USC_2;
		} else {
			g->charSet = UTF_8;
		}

		xhtml_freeAttr(attr);
	}

	return 0;
}


