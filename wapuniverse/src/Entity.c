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
/********************************************************************************
 * Code from ViewML Browser, www.viewml.com					*
 * 									      	*
 ********************************************************************************/
#include	<PalmOS.h>
#include	"WAPUniverse.h"
#include	"PalmUtils.h"
#include	"wml.h"

typedef struct
{
  char *tag;
  char *conversion;
} TagEntry;

/* 
   Some of these tags were contributed by Johannes Thoma <joe@mond.at> to viewml
*/

// TODO: add multibyte international character entities??
static TagEntry tag_list[] =
{
  { "&amp",	"&"    },
  { "&apos",	"\'"   },
  { "&rsquo",   "\'"   },
  { "&rdquo",   "\'"   },
  { "&lsquo",   "\'"   },
  { "&ast",	"\x2a" },
  { "&bull",	"•"    }, //"\x2022" },
  { "&cent",	"\xa2" },
  { "&pound",	"\xa3" },
  { "&curren",	"\xa4" },
  { "&yen",	"\xa5" },
  { "&brvbar",	"\xa6" },
  { "&sect",	"\xa7" },
  { "&uml",	"\xa8" },
  { "&copy",	"\xa9" },
  { "&ordf",	"\xaa" },
  { "&laquo",	"\xab" },
  { "&not",	"\xac" },
  { "&shy",	"\xad" },
  { "&macr",	"\xaf" },
  { "&deg",	"\xb0" },
  { "&plusmn",	"\xb1" },
  { "&sup2",	"\xb2" },
  { "&sup3",	"\xb3" },
  { "&acute",	"\xb4" },
  { "&micro",	"\xb5" },
  { "&para",	"\xb6" },
  { "&middot",	"\xb7" },
  { "&cedil",	"\xb8" },
  { "&sup1",	"\xb9" },
  { "&ordm",	"\xba" },
  { "&raquo",	"\xbb" },
  { "&fraq14",	"\xbc" },
  { "&fraq12",	"\xbd" },
  { "&fraq34",	"\xbe" },
  { "&iquest",	"\xbf" },
  { "&emdash",	"-"    },
  { "&emsp",	" "    },
  { "&endash",	"-"    },
  { "&ensp",	" "    },
  { "&gt",	">"    },
  { "&hellip",	"…"    }, //"\x2026" },
  { "&lt",	"<"    },
  { "&nbsp",	"\xa0" },
  { "&quot",	"\""   },
  { "&reg",	"\xae" },
  { "&trade",	"™"    }, // "\x2122" },
  { "&AElig",	"\xc6" },	/* capital AE diphthong (ligature) */
  { "&Aacute",	"\xc1" },	/* capital A, acute accent */
  { "&Acirc",	"\xc2" },	/* capital A, circumflex accent */
  { "&Agrave",	"\xc0" },	/* capital A, grave accent */
  { "&Aring",	"\xc5" },	/* capital A, ring */
  { "&Atilde",	"\xc3" },	/* capital A, tilde */
  { "&Auml",	"\xc4" },	/* capital A, dieresis or umlaut mark*/
  { "&Ccedil",	"\xc7" },	/* capital C, cedilla */
  { "&ETH",	"\xd0" },	/* capital Eth, Icelandic */
  { "&Eacute",	"\xc9" },	/* capital E, acute accent */
  { "&Ecirc",	"\xca" },	/* capital E, circumflex accent */
  { "&Egrave",	"\xc8" },	/* capital E, grave accent */
  { "&Euml",	"\xcb" },	/* capital E, dieresis or umlaut mark*/
  { "&Iacute",	"\xcd" },	/* capital I, acute accent */
  { "&Icirc",	"\xce" },	/* capital I, circumflex accent */
  { "&Igrave",	"\xcc" },	/* capital I, grave accent */
  { "&Iuml",	"\xcf" },	/* capital I, dieresis or umlaut mark*/
  { "&Ntilde",	"\xd1" },	/* capital N, tilde */
  { "&Oacute",	"\xd3" },	/* capital O, acute accent */
  { "&Ocirc",	"\xd4" },	/* capital O, circumflex accent */
  { "&Ograve",	"\xd2" },	/* capital O, grave accent */
  { "&Oslash",	"\xd8" },	/* capital O, slash */
  { "&Otilde",	"\xd5" },	/* capital O, tilde */
  { "&Ouml",	"\xd6" },	/* capital O, dieresis or umlaut mark*/
  { "&THORN",	"\xdd" },	/* capital THORN, Icelandic */
  { "&Uacute",	"\xda" },	/* capital U, acute accent */
  { "&Ucirc",	"\xdb" },	/* capital U, circumflex accent */
  { "&Ugrave",	"\xd9" },	/* capital U, grave accent */
  { "&Uuml",	"\xdc" },	/* capital U, dieresis or umlaut mark*/
  { "&Yacute",	"\xdd" },	/* capital Y, acute accent */
  { "&aacute",	"\xe1" },	/* small a, acute accent */
  { "&acirc",	"\xe2" },	/* small a, circumflex accent */
  { "&aelig",	"\xe6" },	/* small ae diphthong (ligature) */
  { "&agrave",	"\xe0" },	/* small a, grave accent */
  { "&aring",	"\xe5" },	/* small a, ring */
  { "&atilde",	"\xe3" },	/* small a, tilde */
  { "&auml",	"\xe4" },	/* small a, dieresis or umlaut mark */
  { "&ccedil",	"\xe7" },	/* small c, cedilla */
  { "&eacute",	"\xe9" },	/* small e, acute accent */
  { "&ecirc",	"\xea" },	/* small e, circumflex accent */
  { "&egrave",	"\xe8" },	/* small e, grave accent */
  { "&eth",	"\xf0" },	/* small eth, Icelandic */
  { "&euml",	"\xeb" },	/* small e, dieresis or umlaut mark */
  { "&iacute",	"\xed" },	/* small i, acute accent */
  { "&icirc",	"\xee" },	/* small i, circumflex accent */
  { "&igrave",	"\xec" },	/* small i, grave accent */
  { "&iuml",	"\xef" },	/* small i, dieresis or umlaut mark */
  { "&ntilde",	"\xf1" },	/* small n, tilde */
  { "&oacute",	"\xf3" },	/* small o, acute accent */
  { "&ocirc",	"\xf4" },	/* small o, circumflex accent */
  { "&ograve",	"\xf2" },	/* small o, grave accent */
  { "&oslash",	"\xf8" },	/* small o, slash */
  { "&otilde",	"\xf5" },	/* small o, tilde */
  { "&ouml",	"\xf6" },	/* small o, dieresis or umlaut mark */
  { "&szlig",	"\xdf" },	/* small sharp s, German (szligature)-> */
  { "&thorn",	"\xfe" },	/* small thorn, Icelandic */
  { "&uacute",	"\xfa" },	/* small u, acute accent */
  { "&ucirc",	"\xfb" },	/* small u, circumflex accent */
  { "&ugrave",	"\xf9" },	/* small u, grave accent */
  { "&uuml",	"\xfc" },	/* small u, dieresis or umlaut mark */
  { "&yacute",	"\xfd" },  	/* small y, acute accent */
  { "&yuml",	"\xff" },  	/* small y, dieresis or umlaut mark */

  { "&",	"&"    },   	// must be last entry in table
  { NULL,	NULL   }
};


Char *ConvertLatinEntityToCharset(Char *sourceStr)
{
	char 		dchr[maxCharBytes]; // char in device encoding
	char 		uchr[maxCharBytes+1]; // char in unicode (UTF8)
	UInt16 		dBytes;
	UInt16 		uBytes = maxCharBytes;
	Char 		*result = NULL;
	GlobalsType 	*g;
	Err 		err;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	dBytes = TxtSetNextChar(dchr, 0, sourceStr[0]);

    	switch (g->charSet) {
		case UTF_8:
			err = TxtConvertEncoding(true, NULL, dchr, &dBytes, charEncodingPalmLatin, 
				uchr, &uBytes, charEncodingUTF8, "?", 1);
			break;
		/*case US_ASCII:
			err = TxtConvertEncoding(true, NULL, dchr, &dBytes, charEncodingPalmLatin, 
				uchr, &uBytes, charEncodingAscii, "?", 1);
			break;
		// For some reason this is having issues
		case ISO_8859_1:
			err = TxtConvertEncoding(true, NULL, dchr, &dBytes, charEncodingPalmLatin, 
				uchr, &uBytes, charEncodingISO8859_1, "?", 1);
			break;*/
		case USC_2:
			err = TxtConvertEncoding(true, NULL, dchr, &dBytes, charEncodingPalmLatin, 
				uchr, &uBytes, charEncodingUCS2, "?", 1);
			break;
		default:
			err = TxtConvertEncoding(true, NULL, dchr, &dBytes, charEncodingPalmLatin, 
				uchr, &uBytes, charEncodingPalmLatin, "?", 1);
			break;
    	}

	uchr[uBytes] = 0;

	if (uchr[0] == NULL)
		return NULL;

	result = Malloc(uBytes + 1);
	StrCopy(result, uchr);

	return result;
}

static Int16 EntityCountLength(Char *entity)
{
	Int16	i = 0;
	UInt32	len = StrLen(entity);

	/*while ((entity[i] >= '0') && (entity[i] <= '9')) {
		i++;
	}*/

	while ((i < len) && (((entity[i] >= '0') && (entity[i] <= '9')) ||
			     ((entity[i] >= 'A') && (entity[i] <= 'Z')) ||
			     ((entity[i] >= 'a') && (entity[i] <= 'z'))) &&
			    (entity[i] != ' ') && (entity[i] != NUL) &&
			    (entity[i] != ';'))
		i++;

	return i;
}

Char *EntityConvertEntity(Char *tag, Char *entity, Int16 *entLen)
{
  	int 	count,taglen;
	int 	eLen = 0;
	UInt32	len = StrLen(tag);

	*entLen = 0;

  	// check tag for number
  	if(*(tag + 1) == '#') {
		// TODO: handle multi byte Unicode entities i.e. &#x8056;

		// check tag for a hex
		if(*(tag + 2) == 'x' || *(tag + 2) == 'X') {
			char	temp[50];
			int	i = 3;

			while ((i < len) && (((tag[i] >= '0') && (tag[i] <= '9')) ||
					     ((tag[i] >= 'A') && (tag[i] <= 'Z')) ||
					     ((tag[i] >= 'a') && (tag[i] <= 'z')))) {
				temp[i - 3] = tag[i];
				i++;	
			}
			temp[i - 3] = NUL;

    			entity[0] = strToInt(/*tag + 3*/temp, 16);
			entity[1] = NUL;

			eLen = 3 + EntityCountLength(tag + 3);
			if (tag[eLen] == ';')
				eLen++;
			*entLen = eLen;

			return entity;
  		} else {
    			entity[0] = StrAToI(tag + 2);
			entity[1] = NUL;

			eLen = 2 + EntityCountLength(tag + 2);
			if (tag[eLen] == ';')
				eLen++;
			*entLen = eLen;

			return entity;
		}
  	} else {
		// scan for tag
    		count = 0;
    		while(tag_list[count].tag != NULL) {
      			taglen = StrLen(tag_list[count].tag);
      			if(StrNCompare(tag,tag_list[count].tag,taglen) == 0) {
    				entity[0] = tag_list[count].conversion[0];
				entity[1] = NUL;
				eLen = StrLen(tag_list[count].tag);
				if (tag[eLen] == ';')
					eLen++;
				*entLen = eLen;
				return entity;
      			}
      			//++count;
			count++;
    		}

    		// no matching tag - substitute a space code
    		if(tag_list[count].tag == NULL) {
			StrCopy(entity, tag);
			return entity;
    		}
  	}

  	return entity;
}

Boolean EntityVerify(Char *entity)
{
	UInt32	i = 0;
	UInt32	len = StrLen(entity);
	UInt32	count = 0;
	UInt32	start = 0;
	Boolean	isEnt = false;

	if (entity[0] != '&')
		return false;

	while (((entity[i + 1] != '&') || (entity[i + 1] != ';') ||
		(entity[i + 1] != ' ') || (entity[i + 1] != '\0')) &&
	       ((i + 1) < len)) {
		i++;
	}

	if (!i)
		return false;

	i = 1;
	if (entity[i] == '#') {
		i++;
		if ((entity[i] == 'x') || (entity[i] == 'X'))
			i++;

		start = i;
		while ((i < len) && (((entity[i] >= '0') && (entity[i] <= '9')) ||
				     ((entity[i] >= 'A') && (entity[i] <= 'Z')) ||
				     ((entity[i] >= 'a') && (entity[i] <= 'z'))) &&
				    (entity[i] != ' ') && (entity[i] != NUL) &&
				    (entity[i] != ';'))
			i++;

		if ((entity[i] != ' ') && (entity[i] != ';'))
			return false;

		if ((i - start) > 0)
			isEnt = true;
	} else {
		count = 0;

		while (tag_list[count].tag != NULL) {
      			if (!StrNCompare(entity, tag_list[count].tag, StrLen(tag_list[count].tag))) {
    				isEnt = true;
				break;
      			}
      			count++;
    		}
			
		if (!tag_list[count].tag)
			isEnt = false;
	}

	return isEnt;

	/* TODO: Don't do this. We want to handle, say, &copy2006 or &#45bleh */
	/*for (i = 0; (i + 1) < len; i++) {
		if ((entity[i + 1] == '&') || (entity[i + 1] == ';') ||
		    (entity[i + 1] == ' ') || (entity[i + 1] == '\0')) {
			isEnt = true;
			break;
		}
	}

	if (!isEnt)
		return false;

	if (!i || (i > 10)) { //I doubt that there are any entities > 10
		return false;
	}

	count = 0;

	while (tag_list[count].tag != NULL) {
      		if (!StrNCompare(entity, tag_list[count].tag, StrLen(tag_list[count].tag))) {
    			isEnt = true;
			break;
      		}
      		count++;
    	}
			
	if (!tag_list[count].tag)
		isEnt = false;

	return isEnt;*/
}

Char *EntityAdvancePointer(Char *string)
{
	while ((*string != ';') &&
	       (*string != ' ') &&
	       (*string != NUL))
		string++;

	if (*string == ';')
		string++;

	return string;
}

Char *EntityResizeBuffer(Char *buffer, Char *padLoc, UInt32 diff)
{
	Char	*temp;
	UInt32	buffLen = 0;

	if (!buffer)
		return NULL;

	buffLen = StrLen(buffer);

	temp = Malloc(buffLen + diff + 1);
	if (!temp)
		return buffer;
	MemSet(temp, buffLen + diff + 1, 0);
	
	StrNCopy(temp, buffer, padLoc - buffer);
	StrNCopy(temp + StrLen(temp) + diff, padLoc, buffer + buffLen - padLoc);

	Free(buffer);

	return temp;
}

Char *EntityScanString(Char *string, GlobalsType *g)
{
	Char	*pos = NULL;
	Char	*result = NULL;
	Char	*entity = NULL;
	Char	temp[50];
	UInt16	entLen = 0;
	UInt32	len = StrLen(string);
	UInt32	diff = 0;

	result = StrDup(string);
	ErrFatalDisplayIf(!result, "Malloc failed [EntityScanString]");

	pos = result;

	while ((pos = StrChr(pos, '&'))) {
		if (EntityVerify(pos)) {
			EntityConvertEntity(pos, temp, &entLen);
			entity = ConvertLatinEntityToCharset(temp);
			if (StrLen(entity) > entLen) {
				UInt32	posOff = (pos - result);

				result = EntityResizeBuffer(result, pos, StrLen(entity) - entLen);
				pos = result + posOff;
			}
			StrNCopy(pos, entity, StrLen(entity));
			pos += StrLen(entity);
			Free(entity);

			if ((entLen - StrLen(entity)) > 0)
				diff = entLen - StrLen(entity);

			MemMove(pos, pos + diff, ((result + len) - pos) - /*entLen*/ diff + 1);
		} else {
			pos++;
		}
	}

	return result;
}
