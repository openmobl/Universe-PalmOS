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
#ifndef _dboffline_h_
#define _dboffline_h_

#include 	<PalmOS.h>
#include 	"WAPUniverse.h"

#define dbOfflineDBType   		'Offl'          // type for application database.  must be 4 chars, mixed case.
#define dbOfflineDBName			'Universe-Cache'


#define browserContentTypeWMLC        	0
#define browserContentTypeWBMP       	1
#define browserContentTypeWMLScript  	2
#define browserContentTypeBitmap     	3
#define browserContentTypeJPEG	     	4
#define browserContentTypeGIF	     	5
#define browserContentTypeBMP	     	6
#define browserContentTypePNG	     	7
#define browserContentTypeHTML	     	8
#define browserContentTypeXHTML	     	9
#define browserContentTypeWML	     	10
#define browserContentTypeRSS		11

//extern Char *channelContentTypes[];

#define browserErrChannelNotFound (appErrorClass | 1)     // channel not found
#define browserErrPageNotFound    (appErrorClass | 2)     // page not found
#define	browserErrGeneric	  (appErrorClass | 3)	  // generic

extern void browserLoadInternalPage(GlobalsType *g);

extern Int16 wsp_getOffline(Int16 index, WebDataType *webData, Boolean remember, GlobalsType *g);
extern Err ChannelOpen(Char *name, Boolean sameChannel, GlobalsType *g);
extern Err ChannelGetIndex(Char *url, Boolean sameChannel, UInt16 *indexP, UInt16 *ctP, GlobalsType *g);
extern Err ChannelClose(GlobalsType *g);
extern DmOpenRef Application_OpenPostDB(const Char *channel, Boolean create);
extern void Application_PostURL(const Char *channel, const Char *url);

// 

#endif
