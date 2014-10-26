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
#ifndef _HYPERLINKS_H_
#define _HYPERLINKS_H_

#include	<PalmOS.h>
#include	"WAPUniverse.h"

extern void HyperlinkSetUrl(Char *prefix, Char *url, GlobalsType *g);
extern int HyperlinkAddLink(Int16 x1, Int16 y1, Char *id, GlobalsType *g);
extern void HyperlinkSetEnd(Int16 x2, Int16 y2, GlobalsType *g);
extern Boolean HyperlinkCheckLink(HyperLinkPtr link, GlobalsType *g);
extern int HyperlinkFreeLinks(GlobalsType *g);
extern Boolean HyperlinkIsId(GlobalsType *g);
extern void HyperlinkSetPost(Boolean post, GlobalsType *g);
extern void HyperlinkSetLink(Boolean ison, Char *prefix, Char *url, Char *id, GlobalsType *g);
extern void HyperlinkSelectLink(HyperLinkPtr link, Int16 x1, Int16 x2, Int16 y1, Int16 y2, GlobalsType *g);
extern void HyperlinkDeSelectLink(HyperLinkPtr link, GlobalsType *g);
extern Boolean HyperlinkHandlePenHold(Int16 x, Int16 y, GlobalsType *g);

#endif
