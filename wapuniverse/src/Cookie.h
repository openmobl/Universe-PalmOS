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
#ifndef _COOKIE_H_
#define _COOKIE_H_

#include 	<PalmOS.h>
#include 	"WAPUniverse.h"

#define dbCookieDBType   	'CJar'          // type for application database.  must be 4 chars, mixed case.
#define dbCookieDBName		"Universe-CookieJar"
#define dbCookieSize		14 
#define dbCookieFormat		"[host::Char*]:[path::Char*]:[name::Char*]:[value::Char*]:[expiration]:[maxage]:[version]:[secure]:[tailmatch]" // a string defining the record layout, mostly for aiding the coder

#define COOKIE_MAX_LINE		5000
#define COOKIE_MAX_NAME 	1024

#define COOKIE_PREF_DECLINE	0
#define COOKIE_PREF_ACCEPT	1
#define COOKIE_PREF_ASK		2

extern Boolean CookieCreateJar(CookiePtr cookie, UInt16 mode, GlobalsType *g);
extern Boolean CookieOpenJar(CookiePtr cookie, UInt16 mode, GlobalsType *g);
extern Boolean CookieJarWriteCookie(Char *host, Char *path, Char *name, Char *value, Char *expires, 
			     Char *maxage, Char *version, Boolean secure, Boolean tailmatch, GlobalsType *g);
extern Boolean CookieCloseJar(CookiePtr cookie, GlobalsType *g);
extern Boolean CookieCreateCookie(CookiePtr cookie, Char *header, Char *domain, Char *path, Boolean private, GlobalsType *g);
extern Char *CookieRetrieveCookie(CookiePtr cookie, Char *host, Char *path, Boolean secure, GlobalsType *g);
extern Boolean CookieEatCookie(CookiePtr cookie, Char *host, Char *path, GlobalsType *g);
extern CookiePtr CookieCreateLinkedList(CookiePtr cookie, Char *host, Char *path, Boolean secure, Boolean private, GlobalsType *g);
extern Char *CookieCreateCookieHeader(CookiePtr cookie, GlobalsType *g);
extern Boolean CookieCleanPrivateCookies(CookiePtr *cookie, GlobalsType *g);
extern Boolean CookieCleanCookiePtr(CookiePtr cookie, GlobalsType *g);
extern Boolean CookieCleanCookieJarStore(GlobalsType *g);
extern Boolean CookieCleanCookieJar(CookiePtr cookie, GlobalsType *g);
extern void CookieSetLastCookieUrl(Char *url, GlobalsType *g);
extern Boolean CookieCheckLastCookieUrl(Char *url, GlobalsType *g);


#endif
