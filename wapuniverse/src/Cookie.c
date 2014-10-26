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
#include 	"Cookie.h"
#include 	"WAPUniverse.h"
#include 	"http.h"
#include	"formBrowser.h"
#include	"build.h"
#include	"PalmUtils.h"
#include	"../res/WAPUniverse_res.h"

// Some of the cookie code is adapted from cURL
/********************************************************************************
 * cURL Copyright (c) 1996 - 2006, Daniel Stenberg, <daniel@haxx.se>.		*
 *  										*
 * All rights reserved.								*
 *  										*
 * Permission to use, copy, modify, and distribute this software for any purpose*
 * with or without fee is hereby granted, provided that the above copyright	*
 * notice and this permission notice appear in all copies.			*
 *  										*
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR	*
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,	*
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.	*
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,	*
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR	*
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE 	*
 * USE OR OTHER DEALINGS IN THE SOFTWARE.					*
 *  										*
 * Except as contained in this notice, the name of a copyright holder shall not	*
 * be used in advertising or otherwise to promote the sale, use or other	*
 * dealings in this Software without prior written authorization of the		*
 * copyright holder.								*
 ********************************************************************************/

#define my_isspace(x) ((x == ' ') || (x == '\t'))

static Boolean tailmatch(Char *little, Char *bigone)
{
	UInt16	littlelen = StrLen(little);
	UInt16	biglen = StrLen(bigone);

	if (littlelen > biglen)
		return false;

	return (Boolean)StrEquals(little, bigone + biglen - littlelen);
}

static Boolean checkprefix(Char *string1, Char *string2)
{
	int	ret = 0;

	if (!string1 || !string2)
		return false;

	if (StrLen(string1) > StrLen(string2))
		return false;

	ret = StrCompare(string1, string2);

	if (ret <= 0) {
		return true;
	} else {
		return false;
	}

	return false;
}

static int RequestCookie(char* error, char* additional, char* last)
{

    return (FrmCustomAlert(alAcceptCookie, error, additional ? additional : "", last ? last : ""));
}

// Create the database to store non-expired cookies
Boolean CookieCreateJar(CookiePtr cookie, UInt16 mode, GlobalsType *g)
{
	Int16           error;        // error code
	UInt16          cardNo;       // card containing the application database
	LocalID         dbID;         // handle for application database
	UInt16          dbAttrs;      // database attributes


 	g->DbCookieJar = DmOpenDatabaseByTypeCreator(dbCookieDBType, wuAppType, mode);
  	if (!g->DbCookieJar) {
      		// The database doesn't exist, create it now.
      		error = DmCreateDatabase (0, dbCookieDBName, wuAppType,
                                dbCookieDBType, false);

      		// Check for fatal error.
      		if (error)
			goto _jar_open_cleanp;

      		// Find the application's database.
      		g->DbCookieJar = DmOpenDatabaseByTypeCreator(dbCookieDBType,
                                                wuAppType, mode);

      		// Get info about the database
      		error = DmOpenDatabaseInfo(g->DbCookieJar, &dbID, NULL, NULL, &cardNo, NULL);

      		// Check for fatal error.
      		if (error)
			goto _jar_open_cleanp;

      		// Get attributes for the database
      		error = DmDatabaseInfo (0, dbID, NULL, &dbAttrs, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL);

      		// Check for fatal error.
      		if (error)
			goto _jar_open_cleanp;

      		// Set the new attributes in the database
      		error = DmSetDatabaseInfo(0, dbID, NULL, &dbAttrs, NULL, NULL,
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

      		// Check for fatal error.
      		if (error)
			goto _jar_open_cleanp;

		return true;
    	}

	return true;

_jar_open_cleanp:
	if (g->DbCookieJar)
		DmCloseDatabase(g->DbCookieJar);

	return false;
}

Boolean CookieCreateRecord(GlobalsType *g, UInt16 *recIndex)
{
	MemPtr          p;
	MemHandle       cookieRec;
	UInt16          index = 0;
	Err             error;
	UInt16          attr;

	// Create a new first record in the database
	cookieRec = DmNewRecord(g->DbCookieJar, &index, dbCookieSize);

	// Lock down the block containing the new record.
	p = MemHandleLock(cookieRec);

	// Write a zero to the first byte of the record to null terminate the new
	// URL string.
	error = DmSet(p, 0, dbCookieSize, (UInt8) 0);

	// Check for fatal error.
	//ErrFatalDisplayIf(error, "Could not write to new record.");
	if (error) {
		MemHandleUnlock(cookieRec);
		return(false);
	}

	// Unlock the block of the new record.
	MemPtrUnlock(p);

  	// P11. Set the category of the new record to the current category.
  	DmRecordInfo(g->DbCookieJar, index, &attr, NULL, NULL);
  	attr &= ~dmRecAttrCategoryMask; // Remove all category bits
  	// no category support for connection settings (yet?), place
  	// the new record in the unfiled category.
  	attr |= dmUnfiledCategory;
  	DmSetRecordInfo(g->DbCookieJar, index, &attr, NULL);


	// Release the record to the database manager.  The true value indicates
	// that
	// the record contains "dirty" data.  Release Record will set the record's 
	// dirty flag and update the database modification count.
	DmReleaseRecord(g->DbCookieJar, index, true);

	*recIndex = index;

	return (true);
}

// Open the database that stores the non-expired cookies and write them to the cookie structure
Boolean CookieOpenJar(CookiePtr cookie, UInt16 mode, GlobalsType *g)
{
	UInt16		nrRecs, i, pos;
	MemHandle	cookieRec;
	Char		*pcookie;
	Boolean		loaded = false;
	CookiePtr	tmp;

_jar_open_top:

	if (g->DbCookieJar) {
		nrRecs = DmNumRecordsInCategory(g->DbCookieJar, dmAllCategories);
		pos = 0;
		for (i = 0; i < nrRecs; i++) {
      			cookieRec = DmQueryNextInCategory(g->DbCookieJar, &pos, dmAllCategories);
      			pcookie = MemHandleLock(cookieRec);

			tmp = Malloc(sizeof(CookieType));
			if (!tmp) {
				goto _jar_open_cleanup;
			}

			tmp->host = NULL;
			tmp->path = NULL;
			tmp->name = NULL;
			tmp->value = NULL;
			tmp->expires = NULL;
			tmp->maxage = NULL;
			tmp->version = NULL;
			tmp->secure = false;
			tmp->tailmatch = false;
			tmp->private = false;

			tmp->host = StrDup(pcookie);
			pcookie += (StrLen(pcookie) + 1);
			tmp->path = StrDup(pcookie);
			pcookie += (StrLen(pcookie) + 1);
			tmp->name = StrDup(pcookie);
			pcookie += (StrLen(pcookie) + 1);
			tmp->value = StrDup(pcookie);
			pcookie += (StrLen(pcookie) + 1);
			tmp->expires = StrDup(pcookie);
			pcookie += (StrLen(pcookie) + 1);
			tmp->maxage = StrDup(pcookie);
			pcookie += (StrLen(pcookie) + 1);
			tmp->version = StrDup(pcookie);
			pcookie += (StrLen(pcookie) + 1);

			tmp->secure = *pcookie;
			pcookie++;
			tmp->tailmatch = *pcookie;

			tmp->next = g->CookieJar; // cookie;
			g->CookieJar = tmp; // cookie = tmp;
      			
     			MemHandleUnlock(cookieRec);

      			pos++;
    		}

		loaded = true;
	} else {
		if (CookieCreateJar(cookie, mode, g))
			goto _jar_open_top;
	}

	return loaded;

_jar_open_cleanup:
	MemHandleUnlock(cookieRec);

	return loaded;
}

Boolean CookieJarWriteCookie(Char *host, Char *path, Char *name, Char *value, Char *expires, 
			     Char *maxage, Char *version, Boolean secure, Boolean tailmatch, GlobalsType *g)
{
	UInt32		recordLength;
	UInt32		offset = 0;
	Err		error;
	MemHandle	currec;
	Char		*s;
	Boolean		handled = false;

	recordLength = ((host)?StrLen(host):0) + ((path)?StrLen(path):0) + ((name)?StrLen(name):0) + 
		       ((value)?StrLen(value):0) + ((expires)?StrLen(expires):0) + ((maxage)?StrLen(maxage):0) + 
		       ((version)?StrLen(version):0) + 9;
               
    if (recordLength > NVFSGetNVFSFreeSize()) { // free dbcache too small...
        return false;
    }

	currec = DmGetRecord(g->DbCookieJar, 0);

	if (MemHandleResize(currec, recordLength) == 0) {
      		s = MemHandleLock(currec);
      		offset = 0;

		if (host) {
      			error = DmStrCopy(s, offset, (Char *)host);
      			offset += StrLen(host) + 1;
		} else {
			error = DmStrCopy(s, offset, (Char *)"");
			offset++;
		}

		if (path) {
      			error = DmStrCopy(s, offset, (Char *)path);
      			offset += StrLen(path) + 1;
		} else {
			error = DmStrCopy(s, offset, (Char *)"");
			offset++;
		}

		if (name) {
      			error = DmStrCopy(s, offset, (Char *)name);
      			offset += StrLen(name) + 1;
		} else {
			error = DmStrCopy(s, offset, (Char *)"");
			offset++;
		}

		if (value) {
      			error = DmStrCopy(s, offset, (Char *)value);
      			offset += StrLen(value) + 1;
		} else {
			error = DmStrCopy(s, offset, (Char *)"");
			offset++;
		}

		if (expires) {
      			error = DmStrCopy(s, offset, (Char *)expires);
      			offset += StrLen(expires) + 1;
		} else {
			error = DmStrCopy(s, offset, (Char *)"");
			offset++;
		}

		if (maxage) {
      			error = DmStrCopy(s, offset, (Char *)maxage);
      			offset += StrLen(maxage) + 1;
		} else {
			error = DmStrCopy(s, offset, (Char *)"");
			offset++;
		}

		if (version) {
      			error = DmStrCopy(s, offset, (Char *)version);
      			offset += StrLen(version) + 1;
		} else {
			error = DmStrCopy(s, offset, (Char *)"");
			offset++;
		}

		DmWrite (s, offset, (Char *) &secure, 1);
		offset++;

		DmWrite (s, offset, (Char *) &tailmatch, 1);
		offset++;

      		MemHandleUnlock(currec);

		handled = true;
    	} else {
	       	ErrDisplay ("***MemHandleResize Failed");
	}

	return handled;
}

// Copy non-expired cookies to the jar and close the jar
Boolean CookieCloseJar(CookiePtr cookie, GlobalsType *g)
{
	CookiePtr	tmp;
	Boolean		closed = false;
	UInt16		recIndex = 0;

	if (g->DbCookieJar) {
		CookieCleanCookieJarStore(g);


		tmp = cookie;

		while (tmp != NULL) {
			if (!tmp->private) {
				if (!CookieCreateRecord(g, &recIndex)) {
					closed = false;
					goto _cookie_clean_struct;
				}
				CookieJarWriteCookie(tmp->host, tmp->path, tmp->name, tmp->value, tmp->expires, 
			     		tmp->maxage, tmp->version, tmp->secure, tmp->tailmatch, g);
			}
			tmp = tmp->next;
		}

		tmp = NULL;

		DmCloseDatabase(g->DbCookieJar);
		closed = true;
	} else {
		closed = false;
	}

_cookie_clean_struct:
	while(g->CookieJar != NULL){
		tmp = g->CookieJar; //cookie;
		g->CookieJar = tmp->next; // cookie = tmp->next;

		if (tmp->host)
			Free(tmp->host);
		if (tmp->path)
			Free(tmp->path);
		if (tmp->name)
			Free(tmp->name);
		if (tmp->value)
			Free(tmp->value);
		if (tmp->expires)
			Free(tmp->expires);
		if (tmp->maxage)
			Free(tmp->maxage);
		if (tmp->version)
			Free(tmp->version);
		tmp->secure = false;
		tmp->tailmatch = false;

		Free(tmp);
	}

	return closed;
}


static Boolean CookieCheckFormat(Char *pair)
{
	int	i = 0, j = 0, k = 0;

	while (my_isspace(pair[i]) && (i < StrLen(pair))) i++;

	j = i;

	while ((pair[i] != '=') && (i < StrLen(pair)))
		i++;

	while (my_isspace(pair[i + k]) && ((i + k) < StrLen(pair))) k++;

	if ((i - j - k) == 0)
		return false;

	k = j = 0;

	while (my_isspace(pair[i]) && (i < StrLen(pair))) i++;

	j = i;

	while ((pair[i] != ';') && (i < StrLen(pair)))
		i++;

	while (my_isspace(pair[i + k]) && (i < StrLen(pair))) k++;

	if ((i - j - k) == 0)
		return false;

	return true;
}

static Boolean CookieFillPair(Char *pair, Char *name, Char *value)
{
	UInt16	i = 0, j = 0;
	UInt16	len = StrLen(pair);

	if (!CookieCheckFormat(pair))
		return false;

	while((pair[i] != '=') && (i < len)) {
		name[j] = pair[i];
		i++;
		j++;
	}
	name[j++] = 0;

	if (pair[i] != '=')
		return false;
	if (pair[i] == '=') i++;

	j = 0;
	
	while((pair[i] != ';') && (pair[i] != 0) && (i < len)) {
		value[j] = pair[i];
		i++;
		j++;
	}
	value[j++] = 0;

	return true;
}

CookiePtr CookieAlreadyExists(CookiePtr cookie, Char *host, Char *path, Char *name, Boolean private, GlobalsType *g)
{
	CookiePtr	tmp;
	CookiePtr	ptr = NULL;

  	tmp = cookie;
	while(tmp != NULL)
	{
		if ((StrCompare(host,tmp->host)==0) && (StrCompare(path,tmp->path)==0) &&
		    (StrCompare(name,tmp->name)==0) && (tmp->private == private))
		{
			ptr = tmp;

			return ptr;		
		}
		tmp = tmp->next;
	}
	return ptr;
}

void CookieSetLastCookieUrl(Char *url, GlobalsType *g)
{
	if (!url)
		MemSet(g->CookieLastUrl, sizeof(g->CookieLastUrl), 0);
	else
		StrCopy(g->CookieLastUrl, url);
}

Boolean CookieCheckLastCookieUrl(Char *url, GlobalsType *g)
{
	if (!StrNCaselessCompare(url, g->CookieLastUrl, StrLen(url)))
		return true;
	else
		return false;
}

// Parse a Set-Cookie HTTP header into the Cookie structure
Boolean CookieCreateCookie(CookiePtr cookie, Char *header, Char *domain, Char *path, Boolean private, GlobalsType *g)
{
	Char		*headerPtr = header;
	Char		*semiColon, *equals;
	Char		name[COOKIE_MAX_NAME], *value;
	CookiePtr	tmp = NULL;
	CookiePtr	ptr = NULL;

	if (g_prefs.cookie == COOKIE_PREF_DECLINE) {
		return false;
	} else if (g_prefs.cookie == COOKIE_PREF_ASK) {
		if (CookieCheckLastCookieUrl(g->Url, g))
			goto cookie_create_top;

		switch (RequestCookie(domain, path, NULL)){
      	  		case 0:
				return false;
      	  			break;
			case 1:
				CookieSetLastCookieUrl(g->Url, g);
				break;
      	  		default:
				return false;
      	  			break;
      	  	}
	}
cookie_create_top:

	//if ((StrNCaselessCompare(header, "Set-Cookie: ", 12) == 0) && (!g->privateBrowsing)) {
	if (!StrNCaselessCompare(header, "Set-Cookie", 10)) {
		headerPtr += 10;
        if (!StrNCaselessCompare(headerPtr, ": ", 2)) {
            headerPtr += 2;
        }

    		while(*headerPtr && my_isspace(*headerPtr))
     			headerPtr++;

		value = Malloc(COOKIE_MAX_LINE);
		if (!value)
			return false;

		semiColon = StrChr(headerPtr, ';');
		if (!semiColon)
			return false;

		tmp = Malloc(sizeof(CookieType));
		if (!tmp)	
			return false;

		tmp->host = NULL;
		tmp->path = NULL;
		tmp->name = NULL;
		tmp->value = NULL;
		tmp->expires = NULL;
		tmp->maxage = NULL;
		tmp->version = NULL;
		tmp->secure = false;
		tmp->tailmatch = false;
		tmp->private = private;

		do {
			equals = StrChr(headerPtr, '=');

			// a <name>=<value> or secure
      			if(equals && (!semiColon || (semiColon > equals))) {
				name[0] = value[0] = 0;

				if (CookieFillPair(headerPtr, name, value)) { // check formatting
					Char 	*valuePtr = NULL;
					UInt16	len = StrLen(value);

					
					while (len && my_isspace(value[len - 1])) {
						value[len - 1] = 0;
						len--;
					}

					valuePtr = value;
					while (my_isspace(*valuePtr)) {
						valuePtr++;
					}

					if (StrEquals("path", name)) {
						tmp->path = StrDup(valuePtr);
						if (!tmp->path)
							goto _bad_cookie_cleanup;
					} else if (StrEquals("domain", name)) {
						Char *domainPtr = valuePtr;
						int dotCount = 0;

						// Read note below from cURL, is this necessary?
						//if ('.' == valuePtr[0]) {
						//	domainPtr++;
						//}

						do {
							domainPtr = StrChr(domainPtr, '.');
							if(domainPtr) {
								domainPtr++;
								dotCount++;
							}
						} while(domainPtr);

						// The original Netscape cookie spec defined that this domain name
						// MUST have three dots (or two if one of the seven holy TLDs),
						// but it seems that these kinds of cookies are in use "out there"
						// so we cannot be that strict. [cURL] therefore lowered the check
						// to not allow less than two dots.

						if (dotCount < 2) {
							goto _bad_cookie_cleanup;
						} else {
							if ('.' == valuePtr[0]) {
								valuePtr++;
							}

							if(!domain || tailmatch(valuePtr, domain)) {
								Char *tailPtr = valuePtr;

								if(tailPtr[0] == '.') {
									tailPtr++;
								}

								tmp->host = StrDup(tailPtr);
								if(!tmp->host) {
                  							goto _bad_cookie_cleanup;
                						}
                						tmp->tailmatch = true;
              						} else {
                						goto _bad_cookie_cleanup;
              						}
						}
					} else if (StrEquals("version", name)) {
						tmp->version = StrDup(valuePtr);
						if (!tmp->version)
							goto _bad_cookie_cleanup;
					} else if (StrEquals("max-age", name)) {
						tmp->maxage = StrDup(valuePtr);
						if (!tmp->maxage)
							goto _bad_cookie_cleanup;
					} else if (StrEquals("expires", name)) {
						tmp->expires = StrDup(valuePtr);
						if (!tmp->expires)
							goto _bad_cookie_cleanup;	
					} else if (!tmp->name) {
						tmp->name = StrDup(name);
						tmp->value = StrDup(valuePtr);
						if(!tmp->name || !tmp->value) {
							goto _bad_cookie_cleanup;
						}
					}
					// copy into strings
				} else {
					// not properly formatted
				}
			} else {
				if (StrNCaselessCompare(headerPtr, "secure", 6) == 0) {
					//tmp->secure = true;
				}
			}

      			if(!semiColon || !*semiColon) {
        			// we already know there are no more cookies
        			semiColon = NULL;
        			continue;
      			}

      			headerPtr = semiColon + 1;
      			while(headerPtr && *headerPtr && my_isspace(*headerPtr))
        			headerPtr++;
      			semiColon = StrChr(headerPtr, ';');

        		// There are no more semicolons, but there's a final name=value pair coming up
      			if(!semiColon && *headerPtr)
        			semiColon = StrChr(headerPtr, '\0');

		} while (semiColon);

		if (!tmp->host) {
			tmp->host = StrDup(domain);
			if (!tmp->host) {
				goto _bad_cookie_cleanup;
			}
		}

		if(!tmp->path && path) {
			char *endslash = StrRChr(path, '/');
			if(endslash) {
				UInt16 pathlen = endslash - path + 1;
				tmp->path = Malloc(pathlen + 1);
				if(tmp->path) {
					MemMove(tmp->path, path, pathlen);
					tmp->path[pathlen] = 0;
				} else {
					goto _bad_cookie_cleanup;
				}
			}
		}

		Free(value);

		if(!tmp->name) {
			goto _bad_cookie_cleanup;
		}

	} else {
		return false;
	}

	if ((ptr = CookieAlreadyExists(cookie, tmp->host, tmp->path, tmp->name, private, g))) {
		if (ptr->value)
			Free(ptr->value);
		if (ptr->expires)
			Free(ptr->expires);
		if (ptr->maxage)
			Free(ptr->maxage);
		if (ptr->version)
			Free(ptr->version);
		ptr->secure = tmp->secure;
		ptr->tailmatch = tmp->tailmatch;

		ptr->value = StrDup(tmp->value);
		ptr->expires = StrDup(tmp->expires);
		ptr->version = StrDup(tmp->version);
		ptr->maxage = StrDup(tmp->maxage);

		goto _duplicate_cookie_cleanup;
	} else {
		tmp->next = g->CookieJar; // cookie;
		g->CookieJar = tmp; //cookie = tmp;
	}

	//{char temp[5000]; StrPrintF(temp, "%s %s %s %s",g->CookieJar->host,g->CookieJar->path,g->CookieJar->name,g->CookieJar->value); MyErrorFunc(temp, header);}

	return true;

_duplicate_cookie_cleanup:

	if (tmp) {
		if (tmp->host)
			Free(tmp->host);
		if (tmp->path)
			Free(tmp->path);
		if (tmp->name)
			Free(tmp->name);
		if (tmp->value)
			Free(tmp->value);
		if (tmp->expires)
			Free(tmp->expires);
		if (tmp->maxage)
			Free(tmp->maxage);
		if (tmp->version)
			Free(tmp->version);
		tmp->secure = false;
		tmp->tailmatch = false;

		Free(tmp);
	}	

	return true;

_bad_cookie_cleanup:

	if (tmp) {
		if (tmp->host)
			Free(tmp->host);
		if (tmp->path)
			Free(tmp->path);
		if (tmp->name)
			Free(tmp->name);
		if (tmp->value)
			Free(tmp->value);
		if (tmp->expires)
			Free(tmp->expires);
		if (tmp->maxage)
			Free(tmp->maxage);
		if (tmp->version)
			Free(tmp->version);
		tmp->secure = false;
		tmp->tailmatch = false;

		Free(tmp);
	}	

	return false;
}

CookiePtr CookieCreateLinkedList(CookiePtr cookie, Char *host, Char *path, Boolean secure, Boolean private, GlobalsType *g)
{
	CookiePtr	tmp;
	CookiePtr	ptr;
	CookiePtr	newPtr = NULL;

	if (g_prefs.cookie == COOKIE_PREF_DECLINE) {
		return NULL;
	} /*else if (g_prefs.cookie == COOKIE_PREF_ASK) { // TODO: Should we ask??
		switch (RequestCookie(domain, path, NULL)){
      	  		case 0:
				return false;
      	  			break;
			case 1:
				break;
      	  		default:
				return false;
      	  			break;
      	  	}
	}*/

	ptr = cookie;

	while (ptr) {

		if ((ptr->secure == secure) && (ptr->private == private)) {

			if(!ptr->host ||
			   (ptr->tailmatch && tailmatch(ptr->host, host)) ||
			   (!ptr->tailmatch && StrEquals(host, ptr->host)) ) {

				if(!ptr->path ||
					checkprefix(ptr->path, path) ) {

					tmp = Malloc(sizeof(CookieType));
					if (!tmp) {
						return newPtr;
					}
					MemSet(tmp, sizeof(CookieType), 0);

					tmp->name = StrDup(ptr->name);
					tmp->value = StrDup(ptr->value);
					
					tmp->next = newPtr;
					newPtr = tmp;
				}
			}
		}
		ptr = ptr->next;
	}

	return newPtr;
}

Char *CookieCreateCookieHeader(CookiePtr cookie, GlobalsType *g)
{
	Char		*cookieStr = NULL;
	Char		*cookieTemp = NULL;
	Char		*tempStr = NULL;
	UInt16		stringLen = 0;
	CookiePtr	ptr;

	ptr = cookie;

	while (ptr) {
		if (ptr->name && ptr->value) {
			stringLen = StrLen(ptr->name) + StrLen(ptr->value) + 2 + ((cookieStr)?2:0);
			tempStr = Malloc(stringLen);
			if (!tempStr)
				return cookieStr;
			MemSet(tempStr, stringLen, 0);

			if (cookieStr) {
				StrCopy(tempStr, "; ");
			}

			StrCopy(tempStr+StrLen(tempStr), ptr->name);
			tempStr[StrLen(tempStr)] = '=';
			StrCopy(tempStr+StrLen(tempStr), ptr->value);

			stringLen = StrLen(tempStr) + ((cookieStr)?StrLen(cookieStr):0) + 1;
			cookieTemp = Malloc(stringLen);
			if (!cookieTemp) {
				Free(tempStr);
				return cookieStr;
			}
			MemSet(cookieTemp, stringLen, 0);

			if (cookieStr)
				StrCopy(cookieTemp, cookieStr);
			StrCopy(cookieTemp + StrLen(cookieTemp), tempStr);

			Free(tempStr);

			if (cookieStr)
				Free(cookieStr);

			cookieStr = cookieTemp;
		}
		ptr = ptr->next;
	}

	return cookieStr;
}

Boolean CookieCleanCookiePtr(CookiePtr cookie, GlobalsType *g)
{
	CookiePtr	tmp;
	Boolean		cleared = false;
	Err		error = errNone;

	while(cookie != NULL){
		tmp = cookie; // cookie;
		cookie = tmp->next; // cookie = tmp->next;

		if (tmp->host)
			Free(tmp->host);
		if (tmp->path)
			Free(tmp->path);
		if (tmp->name)
			Free(tmp->name);
		if (tmp->value)
			Free(tmp->value);
		if (tmp->expires)
			Free(tmp->expires);
		if (tmp->maxage)
			Free(tmp->maxage);
		if (tmp->version)
			Free(tmp->version);
		tmp->secure = false;
		tmp->tailmatch = false;

		Free(tmp);
	}

	if (error == errNone)
		cleared = true;
	
	cookie = NULL;

  	return cleared;
}

// Find a cookie from the Cookie structure
Char *cookieRetrieveCookie(CookiePtr cookie, Char *host, Char *path, Boolean secure, GlobalsType *g)
{
	return NULL;
}

// Remove a cookie from the Cookie structure
Boolean CookieEatCookie(CookiePtr cookie, Char *host, Char *path, GlobalsType *g)
{
	return false;
}

Boolean CookieCleanPrivateCookies(CookiePtr *cookie, GlobalsType *g)
{
	CookiePtr	tmp, prev, next;
	Boolean		cleared = false;

	tmp = *cookie; // cookie;
	prev = NULL;

	while(tmp != NULL){
		if (tmp->private) {
			if (tmp->host)
				Free(tmp->host);
			if (tmp->path)
				Free(tmp->path);
			if (tmp->name)
				Free(tmp->name);
			if (tmp->value)
				Free(tmp->value);
			if (tmp->expires)
				Free(tmp->expires);
			if (tmp->maxage)
				Free(tmp->maxage);
			if (tmp->version)
				Free(tmp->version);
			tmp->secure = false;
			tmp->tailmatch = false;

			if (!tmp->next) {
				Free(tmp);
				tmp = NULL;
				if (prev)
					prev->next = tmp;
				else
					*cookie = tmp;
			} else {
				next = tmp->next;
				MemMove(tmp, next, sizeof(CookieType));

				Free(next);
			}

			/*if (lastPtr) {
				lastPtr->next = tmp->next;
			} else if (!lastPtr && (tmp->next)) {
				g->CookieJar = tmp->next;
			}

			next = tmp->next;

			Free(tmp);*/

			cleared = true;
		} else {
			prev = tmp;
			tmp = tmp->next;
		}
	}

	return cleared;
}

Boolean CookieCleanCookieJarStore(GlobalsType *g)
{
	UInt16          nrRecs, i, pos;
	MemHandle       cookieRec;
	Boolean		cleared = false;
	Err		error = errNone;

	nrRecs = DmNumRecordsInCategory(g->DbCookieJar, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
		nrRecs = DmNumRecordsInCategory(g->DbCookieJar, dmAllCategories);
		pos = 0; i = 0;
      		cookieRec = DmQueryNextInCategory(g->DbCookieJar, &pos, dmAllCategories);

		error = DmDeleteRecord(g->DbCookieJar, pos);
		error = DmRemoveRecord(g->DbCookieJar, pos);

      		pos++;
    	}

	if (error == errNone)
		cleared = true;
	

  	return cleared;
}

Boolean CookieCleanCookieJar(CookiePtr cookie, GlobalsType *g)
{
	CookiePtr	tmp;
	UInt16          nrRecs, i, pos;
	MemHandle       cookieRec;
	Boolean		cleared = false;
	Err		error = errNone;

	while(g->CookieJar != NULL){
		tmp = g->CookieJar; // cookie;
		g->CookieJar = tmp->next; // cookie = tmp->next;

		if (tmp->host)
			Free(tmp->host);
		if (tmp->path)
			Free(tmp->path);
		if (tmp->name)
			Free(tmp->name);
		if (tmp->value)
			Free(tmp->value);
		if (tmp->expires)
			Free(tmp->expires);
		if (tmp->maxage)
			Free(tmp->maxage);
		if (tmp->version)
			Free(tmp->version);
		tmp->secure = false;
		tmp->tailmatch = false;

		Free(tmp);
	}

	nrRecs = DmNumRecordsInCategory(g->DbCookieJar, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
		nrRecs = DmNumRecordsInCategory(g->DbCookieJar, dmAllCategories);
		pos = 0; i = 0;
      		cookieRec = DmQueryNextInCategory(g->DbCookieJar, &pos, dmAllCategories);

		error = DmDeleteRecord(g->DbCookieJar, pos);
		error = DmRemoveRecord(g->DbCookieJar, pos);

      		pos++;
    	}

	if (error == errNone)
		cleared = true;
	

  	return cleared;
}

