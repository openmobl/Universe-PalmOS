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
#include 	"URL.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include	"formBrowser.h"
#include	"WAP.h"
#include	"build.h"
#include	"../res/WAPUniverse_res.h"
#include	"variable.h"
#include	"String.h"
#include	"Form.h"
#include	"PalmUtils.h"

UInt16 URLGetURILength(Char *url)
{
	UInt16	i = 0;

	while (url[i] && (url[i] != ':'))
		i++;
	if (url[i] == ':')
		i++;
	if ((url[i] == '/') && (url[i + 1] == '/'))
		i += 2;

	return i;
}

// TODO: Create a better URL parser, break into parts and reassemble
int URLCreateURLStr(char target[], char url[], GlobalsType *g)
{
	UInt16 		j = 0, i = 0, t = 0;
	Boolean		haveTrailingSlash = false;


  	if (!target) {
        	return(-1);
  	} else if ((StrNCaselessCompare(target, "device:", 7)==0) || (StrNCaselessCompare(target, "proxy:", 6)==0) ||
		   (StrNCaselessCompare(target, "internal:", 9)==0) || (StrNCaselessCompare(target, "about:", 6)==0)) {
  	//got a device: URL (like UP.Browser), or internal
		gstrcopy(url,target);
	  	stripDoubleDots(url);
		stripSingleDots(url);
	  	return(0);
  	} else if ((StrNCaselessCompare(target, "wsp:", 4)==0)) {
		gstrcopy(url, "http://");
	  	gstrcopy(url + 7, target + 6);
	  	stripDoubleDots(url);
		stripSingleDots(url);

		if (StrNCaselessCompare(url, "http://", 7)==0) {
			i = 7;
			while ((i < StrLen(url)) && (url[i] != '\0') && (haveTrailingSlash == false)) {
				if (url[i] == '/')
					haveTrailingSlash = true;
				i++;
			}

			if (haveTrailingSlash == false)
				StrCopy(url+StrLen(url), "/");
		}

          	return(0);
  	} else if (target[4] == ':') { // looks like we got 'http://' or 'mtch:'
	  	gstrcopy(url,target);
	  	stripDoubleDots(url);
		stripSingleDots(url);

		//if (!StrNCaselessCompare(url, "http://", 7) || !StrNCaselessCompare(url, "file://", 7)) {
			i = URLGetURILength(url); // 7;
			while ((i < StrLen(url)) && (url[i] != '\0') && (haveTrailingSlash == false)) {
				if (url[i] == '/')
					haveTrailingSlash = true;
				i++;
			}

			if (haveTrailingSlash == false)
				StrCopy(url+StrLen(url), "/");
		/*} else if (!StrNCaselessCompare(url, "https://", 8)) {
			i = 8;
			while ((i < StrLen(url)) && (url[i] != '\0') && (haveTrailingSlash == false)) {
				if (url[i] == '/')
					haveTrailingSlash = true;
				i++;
			}

			if (haveTrailingSlash == false)
				StrCopy(url+StrLen(url), "/");
		}*/

          	return(0);
  	} else if (target[5] == ':') { // looks like we got 'https://' 
	  	gstrcopy(url,target);
	  	stripDoubleDots(url);
		stripSingleDots(url);

		//if (StrNCaselessCompare(url, "https://", 8)==0) {
			i = URLGetURILength(url); // 8;
			while ((i < StrLen(url)) && (url[i] != '\0') && (haveTrailingSlash == false)) {
				if (url[i] == '/')
					haveTrailingSlash = true;
				i++;
			}

			if (haveTrailingSlash == false)
				StrCopy(url+StrLen(url), "/");
		//}

          	return(0);
  	} else if (target[0] == '/') { // looks like we have relative to server
		int	hash = 0;

	  	// yep, copy 'http://'
	  	while ( (g->WapLastUrl[i]!= 0) && (i < URLGetURILength(g->WapLastUrl)/*7*/)){
    	  		url[i]=g->WapLastUrl[i++];
	  	}

		/*while ((g->WapLastUrl[i] != 0) && (i < 10) && (g->WapLastUrl[i] != ':')) {
			url[i] = g->WapLastUrl[i++];
		}

		// assumed that if g->WapLastUrl[i + 1] == '/' then the same for i + 2
		if ((g->WapLastUrl[i] == ':') && (g->WapLastUrl[i + 1] == '/')) {
			url[i] = g->WapLastUrl[i++];
			while ((g->WapLastUrl[i] == '/') && (g->WapLastUrl[i] != 0)) {
				url[i] = g->WapLastUrl[i++];
			}
		} else if (g->WapLastUrl[i] == ':') {
			url[i] = g->WapLastUrl[i++];
		}
MyErrorFunc(url, "url");*/
		hash = i;
  	  	// We need till next '/'
	  	while ( (g->WapLastUrl[i]!= 0) && (g->WapLastUrl[i]!= '/')){
    	  		url[i]=g->WapLastUrl[i++];
	  	}

		if (g->WapLastUrl[i] == 0) {
			i = hash;

			// check for '#', last URL could be like mtch:Sample#card
	  		while ( (g->WapLastUrl[i]!= 0) && (g->WapLastUrl[i]!= '#')){
    	  			url[i]=g->WapLastUrl[i++];
	  		}
		}
	  	gstrcopy(&url[i],target);
	  	stripDoubleDots(url);
		stripSingleDots(url);
	  	return(0);
  	} else if(target[0] == '#') {
    		Char *fragment = StrChr(g->WapLastUrl, '#');
    		if(fragment) {
        		UInt32 len = fragment - g->WapLastUrl;
        		StrNCopy(url, g->WapLastUrl, len);
        		StrCopy(url + len, target);
    		} else {
        		StrCopy(url, g->WapLastUrl);
        		StrCat(url, target);
    		}
    		return(0);
  	} else {  // well ... relative to last URL

		// TODO: fix this, stripSingleDot
		if (StrNCaselessCompare(target, "./", 2)==0)
			t = 2;
		else
			t = 0;

  	  	if (!StrNCaselessCompare(g->WapLastUrl, "device:", 7) || !StrNCaselessCompare(g->WapLastUrl, "proxy:", 6) ||
		    !StrChr(g->WapLastUrl + 9, '\\') || !StrChr(g->WapLastUrl + 9, '/')) {
	  	//got a device: URL (like UP.Browser)
		  	j = 0;
		  	while ((g->WapLastUrl[i] != 0) && (g->WapLastUrl[i] != '#')) {
	    	  		url[i] = g->WapLastUrl[i++];
	    	  		if ((g->WapLastUrl[i] == '/') && (i > 9)) {
	    	  			j = i;
	    	  		}
		  	}
		  	if (j != 0) {
		  		gstrcopy(&url[j+1],target + t);
		  	} else {
				gstrcopy(&url[i],"/");
				gstrcopy(&url[i+1],target + t);
		  	}
		  	stripDoubleDots(url);
			stripSingleDots(url);
		  	return(0);
	  	}

	  	while (g->WapLastUrl[i] != 0) {
	  		if (g->WapLastUrl[i] == '/') {
				j = i;
			}
    	  		url[i]=g->WapLastUrl[i++];
	  	}

		/*{
			int start = 0;
			int c = StrLen(url);

			haveTrailingSlash = false;

			if (url[3] == ':')
				start = 6;
			else if (url[4] == ':')
				start = 7;
			else if (url[5] == ':')
				start = 8;

			while (!haveTrailingSlash && (c > start)) {
				if (url[c] == '/')
					haveTrailingSlash = true;
				c--;
			}
			if (!haveTrailingSlash) {
				gstrcopy(&url[i+1],"/");
				i++;
			}
		}*/

	  	gstrcopy(&url[j/*i*/+1],target + t);
	  	stripDoubleDots(url);
		stripSingleDots(url);
	  	return(0);
  	}	
}

/*

    This function creates a new buffer and fills it with the passed url. Then
    this function canabalizes the url and replaces the first reference of '#'
    and '?' with NUL. This does not make sure that it is working with a valid
    url.

*/
Char *URLStripFragmentAndQuery(Char *url)
{
	Char	*newUrl;
	Char	*fragment = NULL;
	Char	*query = NULL;

	newUrl = StrDup(url);
	if (!newUrl)
		return NULL;

	if ((fragment = StrChr(newUrl, '#'))) {
		*fragment = NUL;
	}

	if ((query = StrChr(newUrl, '?'))) {
		*query = NUL;
	}

	return newUrl;
}

/*void escapeUrl(Char* url)
{
		
		
	//begin voodoo magic...
	while (*url)
	{
	  switch (*url)
	  {
	    case ' ':
			*url = '+';
			break;
	  }
	    
	  url++;
	}
}*/

/**
 * Collapses .. references in a path. That is, when invoked with
 * the string b/c/../d, on exit the string will be b/d.
 * @param s a path without leading slash to collapse.
 */
void stripDoubleDots(char *s) {
  UInt16 i;
  UInt16 j = 0;
  UInt16 mark = 0, parent_mark = 0;
  UInt16 dots = 0, seg_len = 0;
  Boolean nothing_collapsed = true;

  char *s2 = (char *) Malloc(StrLen(s) + 1);

  if(s2) {

    for(i = 0; /*s[i]*/i < StrLen(s); i++) {
      
      s2[j++] = s[i];
           
      if(nothing_collapsed) {

	/* nothing collapsed yet. */
	if(s[i] == '/' && i > 0) {

	  if(dots == 2 && seg_len == 3) {

	    /* found two dots -> collapse to parent. */
	    j = parent_mark;
	    nothing_collapsed = false;

	  } else {

	    parent_mark = mark;
	    mark = j;

	  }
	  
	  dots = 0;
	  seg_len = 0;
	}

	seg_len++;
	
	if(s[i] == '.')
	  dots++;
	else
	  dots = 0;
	
      }

    }
    
    if(i > 0 && nothing_collapsed && dots == 2 && seg_len == 3) {

      /* found two dots at end of string -> collapse to parent. */
      j = parent_mark;
      nothing_collapsed = false;

    }

    s2[j] = '\0';

    /* try to collapse again. */
    if(nothing_collapsed == false)
      stripDoubleDots(s2);

    StrCopy(s, s2);

    Free(s2);

  }

}

/**
 * Removes . references from a path.
 * @param s path to remove . references from.
 */
void stripSingleDots(char *s) {
  char *s2 = (char *) Malloc(StrLen(s) + 1);

  if(s2) {

    UInt16 i, j = 0;
    UInt16 looking_for_dot = 0;

    for(i = 0; /*s[i]*/i < StrLen(s); i++) {

      s2[j++] = s[i];
      
      if(s[i] == '/') {

	if(looking_for_dot && i > 1 && s[i - 1] == '.' && s[i - 2] == '/')
	  j -= 2;

	looking_for_dot++;

      }

    }

    s2[j] = '\0';
    StrCopy(s, s2);
    Free(s2);

  }

}

void AddToBuf(Char **buf, UInt32 *bufSize, Char *str, UInt32 len)
{
    UInt32 size;


    if(*buf == NULL)
    {
        *buf = Malloc(len + 1);
        ErrFatalDisplayIf(*buf == NULL, "Malloc failed [AddToBuf]");
    }
    else if((size = MemPtrSize(*buf)) < *bufSize + len + 1)
    {
        Char *ptr = *buf;

        *buf = Malloc(size + len + 2);
        ErrFatalDisplayIf(*buf == NULL, "Malloc failed [AddToBuf]");
        MemMove(*buf, ptr, *bufSize);
        Free(ptr);
    }

    StrNCopy(*buf + *bufSize, str, len);
    *bufSize += len;
    *(*buf + *bufSize) = '\0';
}

Boolean isMustEscapeChar(int b) {
  return(/* Non US ASCII */
	 (b > 0x8f && b < 0xff) ||
	 /* US ASCII coded characters */
	 b < 0x1f || b == 0x7f || 
	 /* Space */
	 b == ' ' ||
	 /* Reserved */
	 b == ';' || b == '/' || b == '?' || b == ':' || 
	 /*b == '@' ||*/ b == '&' || b == '=' || b == '+' ||
	 b == '$' || b == ',' || b == '\'' || b == '!' ||
	 b == '~' ||
	 /* Unwise */
	 b == '{' || b == '}' || b == '|' || b == '\\' ||
	 b == '^' || b == '[' || b == ']' || b == '`' ||
	 b == '(' || b == ')' ||
	 /* Delims */
	 b == '<' || b == '>' || b == '#' || b == '%' || 
	 b == '"');
}

void AppendEncoded(Char **buf, UInt32 *bufSize, Char *str)
{
    Char code[4];

    while(*str)
    {
        /*if(('A' <= *str && *str <= 'Z') ||       // 'A'..'Z'
           ('a' <= *str && *str <= 'z') ||       // 'a'..'z'
           ('0' <= *str && *str <= '9') ||       // '0'..'9'
           (StrChr("-_.!~*()\'", *str) != NULL)) // unreserved*/
	if (!isMustEscapeChar(*str))
        {
            AddToBuf(buf, bufSize, str, 1);
        }
        else if(*str == ' ')                     // space
        {
            AddToBuf(buf, bufSize, "+", 1);
        }
        else
        {
            code[0] = '%';
            code[1] = "0123456789ABCDEF"[(*str >> 4) & 0x0F];
            code[2] = "0123456789ABCDEF"[ *str       & 0x0F];
            code[3] = 0;

            AddToBuf(buf, bufSize, code, 3);           
        }

        str++;
    }
}

/*static Boolean IsEncoded(Char *string)
{
	Char *temp = NULL;
	Boolean part1 = false, part2 = false;

	if ((temp = StrChr(string, '%'))) {

		if ((((temp[1]) >= '0') && ((temp[1]) <= '9')) ||
			(((temp[1]) >= 'A') && ((temp[1]) <= 'F')))
				part1 = true;

		if ((((temp[2]) >= '0') && ((temp[2]) <= '9')) ||
			(((temp[2]) >= 'A') && ((temp[2]) <= 'F')))
				part2 = true;

		if ((part1 == true) && (part2 == true))
			return true;
		else
			return false;
	}

	return false;
}*/

Boolean URLHasValue(Char *value, GlobalsType *g)
{
	Char	*ptr = NULL;
	Boolean	result = false;

	ptr = expandVar(value, g);
	if (ptr) {
		if (StrLen(ptr))
			result = true;

		Free(ptr);
	} else {
		result = false;
	}

	return result;
}

Char *URLCreateEncodedURL(Char *href, VarType *params, GlobalsType *g)
{
	Char	*buf = NULL;
	UInt32	bufSize = 0;
	VarPtr	tmp;

	// NOTE: This could be problematic

	if (href != NULL)
		AddToBuf(&buf, &bufSize, href, StrLen(href));

	for(tmp = params; tmp; tmp = tmp->next) {
		Char *ptr;

		if (g->post && (tmp == params))
			AddToBuf(&buf, &bufSize, (tmp == params) ? "" : "&", (tmp == params) ? 0 : 1);
		else
			AddToBuf(&buf, &bufSize, (tmp == params) ? "?" : "&", 1);

		ptr = expandVar(tmp->name, g);
		AppendEncoded(&buf, &bufSize, ptr);
		Free(ptr);

		AddToBuf(&buf, &bufSize, "=", 1);

		ptr = expandVar(tmp->value, g);
		AppendEncoded(&buf, &bufSize, UnEscapeCharacters(ptr));
		Free(ptr);
	}

	return buf;
}


Char *URLCreateHTMLEncodedURL(Char *href, NamedValueType *params, Char *name, GlobalsType *g)
{
    Char *buf = NULL;
    UInt32 bufSize = 0;
    NamedValuePtr tmp;

    // NOTE: This could be problematic

    if (href != NULL)
    	AddToBuf(&buf, &bufSize, href, StrLen(href));

    for(tmp = params; tmp; tmp = tmp->next)
    {
        Char *ptr;
    	Char *val = NULL;
    	Int16 count = 0;

	if (StrChr(getVar(tmp->name,g), MULTI_VAR_IND)) {
		while ((val = disectMultiVar(getVar(tmp->name,g), &count))) {
			if (val[0] == 0) break;

			if ((g->post || StrChr(href, '?')) && (tmp == params)) // not sure if this is good? At least for HTML forms...
        			AddToBuf(&buf, &bufSize, (tmp == params) ? "" : "&", (tmp == params) ? 0 : 1);
			else
        			AddToBuf(&buf, &bufSize, (tmp == params) ? "?" : "&", 1);

        		ptr = expandVar(formEatVarName(tmp->name, g), g);
        		AppendEncoded(&buf, &bufSize, ptr);
			Free(ptr);

        		AddToBuf(&buf, &bufSize, "=", 1);

        		ptr = expandVar(val, g);
        		//AppendEncoded(&buf, &bufSize, UnEscapeCharacters(ptr));
			AppendEncoded(&buf, &bufSize, ptr);
			Free(ptr);

			Free(val);
			val = NULL;
		}
	} else {
		//if ((tmp->type == BUTTON) && (!getVar(tmp->name,g))) goto bottom;
		if ((tmp->type == BUTTON) && name && (StrCompare(name, tmp->name)!=0)) goto bottom;

		if (!URLHasValue(tmp->value, g))
			goto bottom;
 
		if ((g->post || StrChr(href, '?')) && (tmp == params)) // not sure if this is good? At least for HTML forms...
        		AddToBuf(&buf, &bufSize, (tmp == params) ? "" : "&", (tmp == params) ? 0 : 1);
		else
        		AddToBuf(&buf, &bufSize, (tmp == params) ? "?" : "&", 1);

        	ptr = expandVar(formEatVarName(tmp->name, g), g);
        	AppendEncoded(&buf, &bufSize, ptr);
		Free(ptr);

        	AddToBuf(&buf, &bufSize, "=", 1);
        	ptr = expandVar(tmp->value, g);
        	//AppendEncoded(&buf, &bufSize, UnEscapeCharacters(ptr));
		AppendEncoded(&buf, &bufSize, ptr);
		Free(ptr);
bottom:
	}
    }

    return buf;
}
