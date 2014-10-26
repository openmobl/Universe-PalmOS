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
#ifndef _XHTML_H_
#define _XHTML_H_

#include 	<PalmOS.h>
#include 	"WAPUniverse.h"
#include	"libhtmlparse.h"

extern Boolean streamLink;
extern Boolean isScript;
extern Boolean isPre;
extern int topTag;
extern Boolean isTextarea;
extern Boolean isSelect;

#define STREAM_TYPE_NONE	0
#define STREAM_TYPE_AUDIO	1
#define STREAM_TYPE_VIDEO	2
#define	STREAM_TYPE_GENERIC	3

#define TT_NONE			0
#define TT_WML			1
#define TT_HTML			2

extern int xhtml_parse(XHTMLPtr xhtml, WebDataPtr webData, GlobalsType *g);
extern int xhtml_free(XHTMLPtr xhtml, GlobalsType *g);

extern Char *getXHTMLAttrValue(Char *arg, struct ArgvTable *args, int numargs);
extern void xhtml_freeAttr(Char *attr);


extern int starttag (char *tag, struct ArgvTable *args, int numargs);
extern int endtag (char *tag);

extern int xhtml_new(XHTMLPtr xhtml, WebDataPtr webData, GlobalsType *g);

// 

#endif
