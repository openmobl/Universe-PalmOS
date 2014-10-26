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
#ifndef _FIVEWAY_H_
#define _FIVEWAY_H_

#include	"WAPUniverse.h"

// Internal 5-Way directional characters
#define	fiveWayDirNone		0x0000	// this is not a 5-Way character that we are receiving
#define	fiveWayDirUp		0x0001
#define	fiveWayDirDown		0x0002
#define	fiveWayDirLeft		0x0003
#define	fiveWayDirRight		0x0004
#define	fiveWayDirCenter	0x0005

extern UInt16 FiveWayKeyType(UInt16 chr, UInt16 keyCode, UInt16 modifiers);
extern Boolean FiveWayHandleEvent(EventType *event);

#endif



