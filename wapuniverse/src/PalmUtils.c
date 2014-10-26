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
#include    	<PalmOS.h>
#include	<TxtGlue.h>
#include	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"Browser.h"
#include	"Font.h"

#ifdef HAVE_PALMONE_SDK
#include	<common/system/palmOneNavigator.h>
#include	<common/system/palmOneChars.h>
#include 	<KeyMgr.h>
#include 	<68K/Hs.h>
#include 	<68K/Libraries/PmKeyLib/PmKeyLib.h>
#include    <PmPalmOSNVFS.h>
#endif

char *rindex(char *str, char c)
{
char *tmp;

  tmp = str + StrLen(str);//strlen(str);
  while(*tmp > str){
  		if(*tmp==c)
			return(tmp);
		tmp--;
  }
  return(NULL);
}

void *GetObjectPtr (UInt16 objectID)
{
    FormPtr         frm = FrmGetActiveForm ();
    return(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, objectID)));
}

void SetFieldFromStr(const Char *srcP , UInt16 fieldIndex)
{
  FieldPtr        fldP;
  FormPtr         frm = FrmGetActiveForm ();
  MemHandle       txtH;
  MemHandle       oldTxtH;

  if (!srcP)
	return;

  fldP = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, fieldIndex));
  ErrNonFatalDisplayIf (!fldP, "missing field");
  oldTxtH = (MemHandle) FldGetTextHandle (fldP);
  // get some space in which to stash the string
  txtH = MemHandleNew (StrLen (srcP) + 1);
  ErrFatalDisplayIf (!txtH, "MemHandleNew Failed");
  StrCopy (MemHandleLock (txtH), srcP);
  // unlock the string MemHandle 
  MemHandleUnlock (txtH);
  FldSetTextHandle (fldP, (MemHandle) txtH);
  FldDrawField (fldP);
  // free the MemHandle AFTER we call FldSetTextHandle.
  if (oldTxtH)
    MemHandleFree (oldTxtH);

}

// draw strings at top of rectangle r, but don't overwrite
// right-edge of r
void DrawCharsToFitWidth (char* s, RectanglePtr r, char c)
{
    char            t[40];
    Int16          stringLength = StrLen (s) + 1;
    Int16          pixelWidth = r->extent.x;
    Boolean         truncate;

    t[0] = c;
    StrCopy (&t[1], s);

    // FontCharsInWidth will update stringLength to the 
    // maximum without exceeding the width
    FntCharsInWidth (t, &pixelWidth, &stringLength, &truncate);
    WinDrawChars (t, stringLength, r->topLeft.x, r->topLeft.y);
}


FieldPtr GetFocusObjectPtr (void)
{
    FormPtr         frm;
    UInt16            focus;

    // get a poInt16er to tha active form and the index of the form object with
    // focus
    frm = FrmGetActiveForm ();
    focus = FrmGetFocus (frm);

    // if no object has the focus return NULL poInt16er
    if (focus == noFocus)
        return(NULL);

    // return a poInt16er to the object with focus
    return(FrmGetObjectPtr (frm, focus));
}

Boolean FldGetFocus(UInt16 objectID)
{
	FieldAttrType	attrP;

	FldGetAttributes(GetObjectPtr(objectID), &attrP);

	if (attrP.hasFocus)
		return true;

	return false;
}

//set field txt from string
Err SetFieldTextFromStr(FieldType *fieldP, Char *string, Boolean redraw)
{
	Err 		err = errNone;
	MemHandle 	textHandle = FldGetTextHandle(fieldP);

	if(textHandle)
	{
		FldSetTextHandle(fieldP, NULL);
		err = MemHandleResize(textHandle, StrLen(string)+1);
		if(err != errNone) {
			FldSetTextHandle(fieldP, textHandle);
			goto Done;
		}
	}
	else
	{
		textHandle = MemHandleNew(StrLen(string)+1);
		if(!textHandle) {
			err = memErrNotEnoughSpace;
			goto Done;
		}
	}
	
	StrCopy((Char*)MemHandleLock(textHandle), string);
	MemHandleUnlock(textHandle);
	
	FldSetTextHandle(fieldP, textHandle);
	
	if(redraw)
		FldDrawField(fieldP);

Done:	
	return err;
}


Boolean isHires(void)
{
	Boolean		ret = false;
	Err		err;
	UInt32		attr;
	UInt32 		version;

	err = FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version);
	
	if (!err && (version >= 4)) {
		WinScreenGetAttribute(winScreenDensity, &attr); 
		if (attr == kDensityDouble) { 
			ret = true;
		}
	} else {
		ret = false;
	}

	return ret;
}

UInt32 strToInt(Char* strin, UInt16 base)
{
	UInt16 len;
	UInt16 c;
	UInt32 conv_val;
	UInt32 mult;
	UInt16 x;
	UInt16 err=1;

	conv_val=0;
	len = StrLen(strin);
	c=0;

	while(c<len-1) {
		// this calculates the multiplier for each digit position
		mult=base;
		x=len-c;

		while(x>2) {
			mult = mult*base;
			x--;
		}

		if(TxtCharIsDigit(strin[c])) {
			conv_val = conv_val+(strin[c]-48)*mult;
			if((strin[c]-48)>base-1)err=0;
		}

		if(TxtCharIsAlpha(strin[c])) {
			if(TxtCharIsLower(strin[c])) {
				conv_val = conv_val+(strin[c]-87)*mult;
				if((strin[c]-87)>base-1) err=0;
			} else {
				conv_val = conv_val+(strin[c]-55)*mult;
				if((strin[c]-55)>base-1) err=0;
			}
		}

		c++;
	}

	if(TxtCharIsDigit(strin[len-1])) {
		conv_val = conv_val+strin[len-1]-48;
		if((strin[len-1]-48)>base-1)err=0;
	}

	if(TxtCharIsAlpha(strin[len-1])) {
		if(TxtCharIsLower(strin[len-1])) {
			conv_val = conv_val+strin[len-1]-87;
			if((strin[len-1]-87)>base-1)err=0;
		} else {
			conv_val = conv_val+strin[len-1]-55;
			if((strin[len-1]-55)>base-1)err=0;
		}
	}

	if(err==0) conv_val=0;

	return conv_val;
}

Char *StrToBase64(Char *ostr, Int32 len)
{
    static const unsigned char base64[64] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    long triplets;
    long lines;
    long orig_len;
    unsigned char *data;
    long from, to;
    int left_on_line;

    if (len == 0) {
        /* Always terminate with CR LF */
        //StrCopy(data, "\015\012");
        //return data;
	return "\015\012";
    }

    /* The lines must be 76 characters each (or less), and each
     * triplet will expand to 4 characters, so we can fit 19
     * triplets on one line.  We need a CR LF after each line,
     * which will add 2 octets per 19 triplets (rounded up). */
    triplets = (len + 2) / 3;   /* round up */
    lines = (triplets + 18) / 19;

    //octstr_grow(ostr, triplets * 4 + lines * 2);
    orig_len = len;
    data = ostr;

    len = triplets * 4 + lines * 2;
    data[len] = '\0';

    /* This function works back-to-front, so that encoded data will
     * not overwrite source data.
     * from points to the start of the last triplet (which may be
     * an odd-sized one), and to points to the start of where the
     * last quad should go.  */
    from = (triplets - 1) * 3;
    to = (triplets - 1) * 4 + (lines - 1) * 2;

    /* First write the CR LF after the last quad */
    data[to + 5] = 10;   /* LF */
    data[to + 4] = 13;   /* CR */
    left_on_line = triplets - ((lines - 1) * 19);

    /* base64 encoding is in 3-octet units.  To handle leftover
     * octets, conceptually we have to zero-pad up to the next
     * 6-bit unit, and pad with '=' characters for missing 6-bit
     * units.
     * We do it by first completing the first triplet with 
     * zero-octets, and after the loop replacing some of the
     * result characters with '=' characters.
     * There is enough room for this, because even with a 1 or 2
     * octet source string, space for four octets of output
     * will be reserved.
     */
    switch (orig_len % 3) {
    case 0:
        break;
    case 1:
        data[orig_len] = 0;
        data[orig_len + 1] = 0;
        break;
    case 2:
        data[orig_len + 1] = 0;
        break;
    }

    /* Now we only have perfect triplets. */
    while (from >= 0) {
        long whole_triplet;

        /* Add a newline, if necessary */
        if (left_on_line == 0) {
            to -= 2;
            data[to + 5] = 10;  /* LF */
            data[to + 4] = 13;  /* CR */
            left_on_line = 19;
        }

        whole_triplet = ((long)data[from] << 16) |
                        (data[from + 1] << 8) |
                        data[from + 2];
        data[to + 3] = base64[whole_triplet % 64];
        data[to + 2] = base64[(whole_triplet >> 6) % 64];
        data[to + 1] = base64[(whole_triplet >> 12) % 64];
        data[to] = base64[(whole_triplet >> 18) % 64];

        to -= 4;
        from -= 3;
        left_on_line--;
    }

    //gw_assert(left_on_line == 0);
    //gw_assert(from == -3);
    //gw_assert(to == -4);

    /* Insert padding characters in the last quad.  Remember that
     * there is a CR LF between the last quad and the end of the
     * string. */
    switch (orig_len % 3) {
    case 0:
        break;
    case 1:
        //gw_assert(data[ostr->len - 3] == 'A');
        //gw_assert(data[ostr->len - 4] == 'A');
        data[len - 3] = '=';
        data[len - 4] = '=';
        break;
    case 2:
        //gw_assert(data[ostr->len - 3] == 'A');
        data[len - 3] = '=';
        break;
    }

    return data;

}

#define FONTHEIGHT	(BrowserLineheightValue() - 1)

static Boolean IsWhitespace(Char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '-';
}

static Boolean IsLinefeed(Char ch)
{
    return ch == '\n' || ch == '\r';
}

void DrawToFit(char *instr, RectangleType rect, GlobalsType *g)
{
    Int16 		length, width, adj = 0;
    Boolean 		fits;
    Char 		*str = instr;
    Int16		lineCount = 1;
    WinDrawOperation	oldMode;


    width = rect.extent.x;
    length = StrLen(str);

    FontCharsInWidth(str, &width, &length, &fits);
    while(!fits || (StrChr(str, '\r') || StrChr(str, '\n')))
    {
        length = FontWordWrap(str, width);

        // check if there is a space-char so we we can wrap
        if(length > 0 && IsWhitespace(*(str + length - 1)))
        {
	    adj = 0;
	    if ((lineCount * FONTHEIGHT) > rect.extent.y) { fits = true; break;}
	    if (IsLinefeed(*(str + length - 1))) { adj = 1; }
	    oldMode = WinSetDrawMode(winOverlay);
     	    WinPaintChars(str, length - adj, rect.topLeft.x, rect.topLeft.y + ((lineCount - 1) * FONTHEIGHT));
	    WinSetDrawMode(oldMode);
	    lineCount++;

            str += length;
	    if (IsLinefeed(*str)) { str++; lineCount++; }
	    adj = 0;
        } else if (length > 0 && IsLinefeed(*(str + length - 1))) {
	    adj = 0;
	    if ((lineCount * FONTHEIGHT) > rect.extent.y) { fits = true; break;}
	    if (IsLinefeed(*(str + length - 1))) { adj = 1; }
	    oldMode = WinSetDrawMode(winOverlay);
     	    WinPaintChars(str, length - adj, rect.topLeft.x, rect.topLeft.y + ((lineCount - 1) * FONTHEIGHT));
	    WinSetDrawMode(oldMode);
	    lineCount++;

            str += length;
	    if (IsLinefeed(*str)) { str++; lineCount++; }
	    adj = 0;
        } else {
            Char *ptr = str;
            while(IsWhitespace(*ptr)) ptr++;

            // no space-character in string, try if newline would help
            width  = rect.extent.x;
            length = StrLen(ptr);

            FontCharsInWidth(ptr, &width, &length, &fits);
            if(!fits)
            {
                length = FontWordWrap(ptr, width);

                // the string still does not fit after a newline, check again for space-char
                if(length > 0 && IsWhitespace(*(ptr + length - 1)))
                {
                    // there is a space-char at position 'length'
		    lineCount++;
	    	    adj = 0;
	    	    if ((lineCount * FONTHEIGHT) > rect.extent.y) { fits = true; break; }
		    if (IsLinefeed(*(str + length - 1))) { adj = 1; }
		    oldMode = WinSetDrawMode(winOverlay);
                    WinPaintChars(str, length - adj, rect.topLeft.x, rect.topLeft.y + ((lineCount - 1) * FONTHEIGHT));
	    	    WinSetDrawMode(oldMode);
		    lineCount++;
                    
                    str = ptr + length;
	    	    if (IsLinefeed(*str)) { str++; lineCount++; }
	    	    adj = 0;
                }
                else
                {
                    // we'll need to cut ...
                    width = rect.extent.x;
                    length = StrLen(str);
	    	    adj = 0;

                    FontCharsInWidth(str, &width, &length, &fits);
                    
                    if ((lineCount * FONTHEIGHT) > rect.extent.y) { fits = true; break; }
		    if (IsLinefeed(*(str + length - 1))) { adj = 1; }
		    oldMode = WinSetDrawMode(winOverlay);
	    	    WinPaintChars(str, length - adj, rect.topLeft.x, rect.topLeft.y + ((lineCount - 1) * FONTHEIGHT));
		    WinSetDrawMode(oldMode);
		    lineCount++;

                    str += length;
		    if (IsLinefeed(*str)) { str++; lineCount++; } 
		    adj = 0;                   
                }
            }
            else
            {
                // the complete string fits after a newline.
	    	adj = 0; 
                lineCount++;
		if ((lineCount * FONTHEIGHT) > rect.extent.y) { fits = true; break; }
		if (IsLinefeed(*(str + length - 1))) { adj = 1; }
		oldMode = WinSetDrawMode(winOverlay);
	        WinPaintChars(str, length - adj, rect.topLeft.x, rect.topLeft.y + ((lineCount - 1) * FONTHEIGHT));
	        WinSetDrawMode(oldMode);
		lineCount++;
	    	adj = 0;
                return;
            }
        }
        
        width = rect.extent.x;
        length = StrLen(str);

        FontCharsInWidth(str, &width, &length, &fits);                
    }

    if ((lineCount * FONTHEIGHT) > rect.extent.y) goto end;
    oldMode = WinSetDrawMode(winOverlay);
    WinPaintChars(str, length, rect.topLeft.x, rect.topLeft.y + ((lineCount - 1) * FONTHEIGHT));
    WinSetDrawMode(oldMode);
    lineCount++;

end:
    //if(IsWhitespace(*(str + length))) g->x += FontCharWidth(' '); // laf fix
}

Char *StrCompareLower(Char *str, const Char *token)
{
	Char	*dest, *off;
	UInt32	offset = 0;

	dest = Malloc(StrLen(str) + 1);
	if (!dest)
		return NULL;

	dest = StrToLower(dest, str);

	off = StrStr(dest, token);

	offset = (off - dest);

	Free(dest);

	return str + offset;
}

Char *StrDup(Char *string)
{
	Char	*tempString = NULL;
	UInt16	stringLen;

	if (!string)
		return NULL;

	stringLen = StrLen(string);
	tempString = Malloc(stringLen + 1);
	if (!tempString)
		return NULL;

	StrCopy(tempString, string);

	return tempString;
}

Boolean StrEquals(Char *string1, Char *string2)
{
	int	ret = 0;

	if (!string1 || !string2)
		return false;

	ret = StrCompare(string1, string2);

	if (!ret) {
		return true;
	} else {
		return false;
	}
}

Char *StrRChr(Char *string, Char token)
{
	UInt16	length;
	UInt16	i;

	if (!string)
		return NULL;

	length = StrLen(string);

	i = length;

	while ((string[i] != token) && (i > 0))
		i--;

	if (i == 0)
		return NULL;

	return string + i;
}

// TODO: Make multi-byte char safe
Char *StrReverse(Char *str)
{
	/*Char	*out = NULL;
	UInt32	length = StrLen(str);
	UInt32	i = 0, j = 0, blocks = 0;
	UInt16	*bytes = NULL;

	out = StrDup(str);
	if (!out)
		return NULL;
	bytes = (UInt16 *)Malloc(length);
	if (!bytes) {
		Free(out);
		return NULL;
	}

	for (i = 0, j = 0; i < length && j < length; j++) {
		i += TxtGetNextChar(str, i, &bytes[j]);
		if (i >= length)
			break;
	}
	blocks = j;

	for (i = blocks, j = 0; i >= 0 && j < length; i--) {
		j += TxtSetNextChar(out, j, bytes[i]);
	}
	out[length] = '\0';

	Free(bytes);

	return out;*/

// Non-multi-byte char safe!!!
	Char 	*out = NULL;
	UInt32	length = StrLen(str);
	UInt32	i = 0, j = 0;

	out = StrDup(str);
	if (!out)
		return NULL;
	
	for (i = 0, j = (length - 1); i < length; i++, j--) {
		out[i] = str[j];
	}
	out[length] = '\0';

	return out;
}

// TODO: Change to pull out totlaMem, free mem, total heap, free heap, and return grand total
UInt32 GetFreeMemory(UInt32 *totalMemoryP, UInt32 *dynamicMemoryP, Boolean maxChunkSize )
{
	UInt32  heapFree;
 	UInt32  max;
 	UInt16  i;
 	UInt16  nCards;
 	UInt16  cardNo;
 	UInt16  heapID;
 	UInt32  freeMemory = 0;
 	UInt32  totalMemory = 0;
 	UInt32  dynamicMemory = 0;

 	// Iterate through each card to support devices with multiple cards.
 	nCards = MemNumCards();
 	for (cardNo = 0; cardNo < nCards; cardNo++) {
  		// Iterate through the RAM heaps on a card (excludes ROM).
  		for (i=0; i < MemNumRAMHeaps(cardNo); i++) {
   			// Obtain the ID of the heap.
   			heapID = MemHeapID(cardNo, i);

   			// If the heap is dynamic, increment the dynamic memory total.
   			if( !MemHeapCheck(heapID) ) {
    				// If the heap is dynamic, increment the dynamic memory total.
    				if (MemHeapDynamic(heapID)) {
     					dynamicMemory += MemHeapSize(heapID);

					heapFree = 0;
     					max = 0;
     					MemHeapFreeBytes(heapID, &heapFree, &max);
					if (maxChunkSize) {
     						freeMemory += max;
					} else {
     						freeMemory += heapFree;
					}
    				} else {
     					// Calculate the total memory and free memory of the heap.
     					//totalMemory += MemHeapSize(heapID);

     					heapFree = 0;
     					max = 0;
     					MemHeapFreeBytes(heapID, &heapFree, &max);
     					//freeMemory += heapFree;
					if (maxChunkSize) {
						totalMemory += max;
					} else {
						totalMemory += heapFree;
					}
    				}
   			}
  		}
 	}

	if (totalMemoryP) *totalMemoryP = totalMemory;
 	if (dynamicMemoryP) *dynamicMemoryP = dynamicMemory;

	return (freeMemory);
}


void PrintFreeMem(Char *text)
{
	UInt32		dynamicMemoryP;
	UInt32		free = GetFreeMemory(NULL, &dynamicMemoryP, true);
	UInt32		free2 = GetFreeMemory(NULL, NULL, false);
	char		temp[500];
	RectangleType	rect = {{0,0},{160,160}};

	WinSetClip(&rect);
	StrPrintF(temp, "%ld %ld %ld  %s", dynamicMemoryP, free, free2, text);
	WinDrawChars(temp, StrLen(temp), 0, 0);
	WinResetClip();
}

Boolean SysCheck5Way(void)
{
#ifdef HAVE_PALMONE_SDK
	/*Boolean		ret = false;
	Err		err;
	UInt32 		version;

	err = FtrGet(navFtrCreator, navFtrVersion, &version);
	
	if (!err && (version >= navVersion)) {
		ret = true;
	} else {
		ret = false;
	}

	return ret;*/

	UInt32           romVersion;
  
	FtrGet (sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion >= 0x05400000) {
		return true; // assume that PalmOS 5.4+ supports 5way
	} else {
		return false;
	}
#else
	return false;
#endif
}

void FldEmulateEnter(UInt16 index)
{
	EventType	event;

	event.eType = fldEnterEvent; // emulate a fldEnterEvent
	event.tapCount = 1;
	event.data.fldEnter.fieldID = index;
	event.data.fldEnter.pField = GetObjectPtr(index); 
	EvtAddEventToQueue(&event);
}

UInt16 HiresFactor(void)
{
	if (isHires())
		return 2;
	else
		return 1;
}

UInt32 NVFSGetNVFSFreeSize(void)
{
#ifdef HAVE_PALMONE_SDK
#define STORAGE_HEAP_ID 1
    UInt32  free = 0;
    UInt32  max = 0;

    MemHeapFreeBytes(STORAGE_HEAP_ID | dbCacheFlag, &free, &max);
    
    return free;
#else
    return 0;
#endif
}

Boolean NVFSIsNVFS(void)
{
#ifdef HAVE_PALMONE_SDK
    UInt32  dbCache = 0;
    
    FtrGet(sysFtrCreator, sysFtrNumDmAutoBackup, &dbCache);
    
    return (dbCache == 1);
#else
    return false;
#endif
}

Err NVFSSyncDatabase(DmOpenRef dbRef)
{
#ifdef HAVE_PALMONE_SDK
    if (NVFSIsNVFS()) {
        return DmSyncDatabase(dbRef);
    }
#endif

    return errNone;
}

/*UInt32 SystemGetA4(void)
{
	asm("move.l %a4,%d0");
}

UInt32 SystemGetA5(void)
{
	asm("move.l %a5,%d0");
}

void SystemSetA4(UInt32 newVal)
{
	asm("move.l 4(%a7),%a4");
}

void SystemSetA5(UInt32 newVal)
{
	asm("move.l 4(%a7),%a5");
}*/

/*void SystemSetA4(UInt32 newVal)
{
	asm("move.l (%sp),%a4");
}

void SystemSetA5(UInt32 newVal)
{
	asm("move.l (%sp),%a5");
}*/

//Close APP
void ExitApp(void)
{
	EventType event;
	
	event.eType = appStopEvent;
	EvtAddEventToQueue(&event);
}
