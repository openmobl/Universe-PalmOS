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
 * Code used from:								*
 * Using C with Web Clipping -- Developing Web Clippings			*
 * "6b Working With Data & Web Clipping"					*
 * http://www.tow.com/publishing/Developing_Web_Clippings/html/ch06b.html	*
 * 									      	*
 ********************************************************************************/

#include	<PalmOS.h>
#include	"WAPUniverse.h"
#include    "MemMgr.h"

#define hexValues "0123456789ABCDEF"
#define kHexChar '%'

// String that defines which characters should be escaped.
// ASCII values 32-127 are displayed below.

#define kAcceptableCharacters	"100000000001011011111111110000001111111111111111111111111110000001111111111111111111111111100000"


static char HexToDec(char h)
{
	if ((h >= 48) && (h <= 57)) return (h - 48);
	else if ((h >= 65) && (h <= 70)) return (h - 55);
	else return -1;
}

static int IsAcceptable(int c)
{	
	return (c >= 32 && c < 128 && (kAcceptableCharacters[c - 32] - '0'));
}

Char *UnEscapeCharacters(char *result)
{
	char *tmp;
	char *resultTmp;
		
	for(tmp = result, resultTmp = result; *tmp != NULL; tmp++, resultTmp++) {
		if(*tmp == kHexChar) {
			tmp++;
				
			if(*tmp) {
				*resultTmp = HexToDec(*tmp++) << 4;
					
				if(*resultTmp == -1) return NULL;
					
				if(*tmp) 
					*resultTmp += HexToDec(*tmp) & 15;
			}
		}
		else *resultTmp = *tmp;
	}
	*resultTmp = 0;
		
	return result;
}

Char *EscapeCharacters(char *str)
{
	char	*tmp, *resultTmp, *result;
	int		numUnacceptable = 0;
	int		x;
		
	// Count the number of unacceptable characters
	for(tmp = str; *tmp; tmp++) {
		if(!IsAcceptable(*tmp)) numUnacceptable++;
	}
		
	// Allocate enough memory for the escaped string
	result = Malloc(StrLen(str) + (2 * numUnacceptable) + 1);
	ErrFatalDisplayIf(!result, "Out of memory");
		
	// Create the new string
	for(tmp = str, resultTmp = result; *tmp; tmp++) {
		if(!IsAcceptable(*tmp)) {
			x = (int) *tmp;
				
			*resultTmp++ = kHexChar;
			*resultTmp++ = hexValues[x >> 4];
			*resultTmp++ = hexValues[x & 15];
		}
		else if (*tmp == ' ') {
			x = (int) *tmp;
				
			*resultTmp++ = kHexChar;
			*resultTmp++ = hexValues[x >> 4];
			*resultTmp++ = hexValues[x & 15];
		}
		else if (*tmp == '+') {
			x = (int) *tmp;
				
			*resultTmp++ = kHexChar;
			*resultTmp++ = hexValues[x >> 4];
			*resultTmp++ = hexValues[x & 15];
		}
		else *resultTmp++ = *tmp;
	} 
	
	// Terminate the new string
	*resultTmp = 0;
		
	return result;
}

// Like StrCat
Char *StrCombine(Char *string1, Char *string2)
{
    UInt32  length1 = 0;
    UInt32  length2 = 0;
    Char    *out    = NULL;
    
    if (!string1 || !string2) {
        return string1;
    }
    
    length1 = StrLen(string1);
    length2 = StrLen(string2);
    
    out = Malloc(length1 + length2 + 1);
    if (!out) {
        return string1;
    }
    
    StrPrintF(out, "%s%s", string1, string2);
    
    out[length1 + length2] = '\0';
    
    Free(string1);
    
    return out;
    
    //return StrCat(string1, string2);
    /*UInt32  length1 = 0;
    UInt32  length2 = 0;
    Char    *out    = NULL;
    
    if (!string1 || !string2) {
        return NULL;
    }

    //while ((string1[length1] != '\0') && (length1 < 1e6))
    //    length1++;

    //while ((string2[length2] != '\0') && (length2 < 1e6))
    //    length2++;
    
    length1 = StrLen(string1);
    length2 = StrLen(string2);

    out = MemMgrChunkCombine((MemPtr)string1, length1,
                             (MemPtr)string2, length2);
                             
    if (!out) {
        return NULL;
    }
    
    out[length1 + length2] = '\0';
    
    return out;*/
}
