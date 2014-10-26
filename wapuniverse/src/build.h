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
#ifndef _build_h_
#define _build_h_

#define VERSION       	3
#define RELEASE       	0
#define PATCHLEVEL    	10
#define BUILD         	270
#define REVISION        "a"
#define VERSIONSTR    	"v 3.0.10"
#define VERSIONSTR2   	"3.0.10"
#define BUILDNUMSTR     "270"

/* Release Candidate versioning */
#define RCVERSIONSTR	"1"
#define RCVERSION	1

#define IS_RELEASE_CANDIDATE
/* */

#define BUILDDATE	"Wed Mar 12 19:45:00 PST 2007"
#define BUILDSTR	"Build " ## BUILDNUMSTR ## " - " ## BUILDDATE

#define BUILDFORMATSTR	"20070312"

#define CODENAME      	"Mercury"

extern Char *AgentString[];

// Modules
#define JPEGLIBVER	"v 1.0.0"
#define SCRIPTLIBVER	"v 1.2.1"
#define LIBHTMLPARSEVER "v 0.1.14w"

/************************************************************************************************
 *  TODO: Use format string on user agent... 	NOPE!!						*
 *  For example:										*
 *  "WAPUniverse/%v (PalmOS; %s; %l) MobileToken/%b (WAP %w, XHTML; %d)"			*
 *  or												*
 *  "WAPUniverse/%s (PalmOS; %s; %s) MobileToken/%s (WAP %s, XHTML; %s)"  // StrPrintF		*
 *												*
 *  would become:										*
 *												*
 *  "WAPUniverse/3.0.2 (PalmOS; N; en) MobileToken/20070113 					*
 *							(WAP 1.3/2.0, XHTML; WAPUniverse.com)"	*
 *												*
 *  Also, device manufacturers should use the following:					*
 *												*
 *  <device Name>/<firmware ver.> Universe/<universe ver.> PalmOS/<palmos ver.>			*
 *												*
 ************************************************************************************************/


// User agent strings
// Mozilla Compatible user agent
#define AGENT_STRING_DEFAULT		"Mozilla/5.0 (PalmOS; N; ARM; en-us) MobileToken/" ## BUILDFORMATSTR ## " (OpenMobl) Universe/3.0"

// Mozilla Firefox user agent
#define AGENT_STRING_MOZ		"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-us; rv:1.8.1.1) Gecko/" ## BUILDFORMATSTR ## " Firefox/2.0.0.1"

// Nokia agent: Nokia6600/1.0 (4.09.1) SymbianOS/7.0s Series60/2.0 Profile/MIDP-2.0 Configuration/CLDC-1.0
#define AGENT_STIRING_NOK     		"Nokia6600/1.0 (4.09.1) SymbianOS/7.0s (compatible; Universe/3.0.7)"

// IE UA
#define AGENT_STRING_MSIE6		"Mozilla/4.0 (compatible; MSIE6, Windows NT 5.0; SV1)"

// Nokia S60
#define AGENT_STRING_S60		"Mozilla/5.0 (SymbianOS/9.2; U; Series60/3.1 NokiaE70/1.0; Profile/MIDP-2.0 Configuration/CLDC-1.1) AppleWebKit/413 (KHTML, like Gecko) Safari/413"


// depreciated
// -- Unused
#ifdef INCLUDE_OLD_BUILD_H
// Generic User Agent
#   define AGENT_STRING_GENERIC 	"Universe/3.0.6 (PalmOS; N; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)"

// Motorola 68K agent
#   define AGENT_STRING   	     	"Universe/3.0.6 (PalmOS; N; m68K; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)"

// ARM CPU agent
#   define AGENT_STRING_ARM     	"Universe/3.0.6 (PalmOS; N; ARM; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)"

// X-Scale CPU agent
#   define AGENT_STRING_XSCALE  	"Universe/3.0.6 (PalmOS; N; X-Scale; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)"

// PalmOS Emulator agent
#   define AGENT_STRING_EMU     	"Universe/3.0.6 (PalmOS; N; POSE; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)"

// PalmOS Simulator agent
#   define AGENT_STRING_SIM     	"Universe/3.0.6 (PalmOS; N; x86; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)"

// unknown processor agent
#   define AGENT_STRING_NO	     	"Universe/3.0.6 (PalmOS; N; unknown; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)"

// Unwired Planet agent
#   define AGENT_STIRING_UP     	"UP.Browser/4.0 (compatable; Universe/3.0.6; PalmOS; N; en)"

// Mozilla Compatible user agent
#   define AGENT_STRING_MOZ_OLD		"Mozilla/5.0 (compatible; Universe/3.0.6; PalmOS; N; en) MobileToken/20070113 (OpenMobl)"
#endif



// This is regarding the offscreen rendering window

/*#define USE_LARGE_WINDOW
#define USE_FEATURE_PAGEFILE

#define ONE_KILOBYTE		1024 // bytes
#define MEM_BUFFER_OVERHEAD	(ONE_KILOBYTE * 25) // 25kb*/

#endif _build_h_
