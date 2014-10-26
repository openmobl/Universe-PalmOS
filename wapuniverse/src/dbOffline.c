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
#include 	"dbOffline.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"build.h"
#include	"../res/WAPUniverse_res.h"
#include	"Layout.h"
#include	"Form.h"
#include	"PalmUtils.h"
#include	"Pointer.h"

/*Char *channelContentTypes[]
{
	"application/vnd.wap.wmlc",
	"image/vnd.wap.wbmp",
	"application/vnd.wap.wmlscriptc",
	"image/vnd.palm.pbmp",
	"image/jpeg",
	"image/gif",
	"image/bmp",
	"image/png",
	"text/html",
	"text/html", // Should be different
	"application/vnd.wap.wmlc"
}*/


void browserLoadInternalPage(GlobalsType *g)
{
	FormType	*frmP2;

	if (!StrNCaselessCompare(g->Url, "internal:", 9) ||
	    !StrNCaselessCompare(g->Url, "about:", 6)) {
		browserPushPageContent(g);
		if (g->contentType)
			Free(g->contentType);
		g->contentType = NULL;
		if (g->pageContentType)
			Free(g->pageContentType);
		g->pageContentType = NULL;
		if (g->webData.data)
			Free(g->webData.data);
		g->webData.data = NULL;
		g->webData.length = 0;
	}

	SetFieldFromStr(g->Url, fldUrlBox);
	FldSetInsPtPosition(GetObjectPtr(fldUrlBox), 0);

	if (StrNCaselessCompare(g->Url,"internal:open",13)==0) {
		g->state = BS_IDLE;
  		FormInputDeactivate(g);
  		FrmPopupForm(frmUrl);
	} else if (StrNCaselessCompare(g->Url,"internal:help",15)==0) {
		g->state = BS_IDLE;
		frmP2 = FrmInitForm (frmHelp);
        	FrmDoDialog (frmP2);                    
        	FrmDeleteForm (frmP2);
		g->state = BS_IDLE;
	} else if (StrNCaselessCompare(g->Url,"internal:welcome",16)==0) {
		/*WinHandle	prevDrawWin;
		BitmapPtr	resP=0;

		palmResetState(g);

		g->state = BS_IDLE;
		browserSetTitle("Welcome"); // SetFieldFromStr("Welcome",fieldTitle);
		
    		prevDrawWin = WinGetDrawWindow();
		palmResizeWindow(133,g); 
    		WinSetDrawWindow(g->DrawWin);
		resP=MemHandleLock(DmGetResource(bitmapRsc, bmpWAPUniverseBlue));
    		WinDrawBitmap (resP, 0, 0);
    		MemPtrUnlock(resP);
		WinSetDrawWindow(prevDrawWin);
		updateScreen(g);

		//palmprintf("[", g);
		//palmHyperLink(true,"","<PREV/>",g);
		//palmprintf("Back", g);
		//palmHyperLink(false,NULL,NULL,g);
		//palmprintf("]", g);
		//palmprintln(g);
		palmBold(true);
		g->x = (157 - FntCharsWidth ("Welcome to WAPUniverse", StrLen("Welcome to WAPUniverse")))/2;
		palmprintf("Welcome to WAPUniverse", g);
		palmBold(false);
		palmprintln(g);
		palmprintln(g);
		palmprintf("   WAPUniverse brings the wireless world to the tip of your stylus! It allows you to check email, stocks and top news through your wireless connected device or smartphone.", g);
		palmprintln(g);
		palmprintln(g);

    		prevDrawWin = WinGetDrawWindow();
		palmResizeWindow(144,g); 
    		WinSetDrawWindow(g->DrawWin);
		resP=MemHandleLock(DmGetResource(bitmapRsc, bmpWAPUniverseBlue));
    		WinDrawBitmap (resP, 0, 133);
    		MemPtrUnlock(resP);
		WinSetDrawWindow(prevDrawWin);
		updateScreen(g);

		palmprintf("   Built around the business user WAPUniverse provides its users with a sleek and stylish, yet easy to use interface.", g);
		palmprintln(g);
		palmprintln(g);
		palmprintf("   WAPUniverse also brings higher transfer rates through the WAP protocol so that you can quickly get to the information you need!", g);
		palmprintln(g);
		palmprintln(g);
		palmprintf("For more information goto:", g);
		palmprintln(g);
		palmHyperLink(true,"http://","wap.wapuniverse.com/",g);
		palmprintf("http://www.wapuniverse.com/", g);
		palmHyperLink(false,NULL,NULL,g);
		palmprintln(g);*/
		g->state = BS_IDLE;
	} else if (StrNCaselessCompare(g->Url,"internal:about",14)==0) {//A little taste of how the renderer renders the pages
		/*WinHandle	prevDrawWin;
		BitmapPtr	resP=0;

		palmResetState(g);

		g->state = BS_IDLE;
		browserSetTitle("About WAPUniverse"); // SetFieldFromStr("About WAPUniverse",fieldTitle);
		
    		prevDrawWin = WinGetDrawWindow();
		palmResizeWindow(133,g); 
    		WinSetDrawWindow(g->DrawWin);
		resP=MemHandleLock(DmGetResource(bitmapRsc, bmpWAPUniverseBlue));
    		WinDrawBitmap (resP, 0, 0);
    		MemPtrUnlock(resP);
		WinSetDrawWindow(prevDrawWin);
		updateScreen(g);

		//palmprintf("[", g);
		//palmHyperLink(true,"","<PREV/>",g);
		//palmprintf("Back", g);
		//palmHyperLink(false,NULL,NULL,g);
		//palmprintf("]", g);
		//palmprintln(g);
		palmprintf("WAPUniverse Mobile Internet Browser", g);
		palmprintln(g);
		palmprintf(VERSIONSTR2, g);
		//palmprintf(" US-DOM", g);
		//palmprintln(g);
		palmprintf(" (Palm, WAP/XHTML edition)", g);
		palmprintln(g);
		palmprintf("(c) 2000-2005 Filip Onkelinx", g);
		palmprintln(g);
		palmprintf("Maintained by: Donald C. Kirker", g);
		palmprintln(g);
		palmHyperLink(true,"http://","wap.wapuniverse.com/",g);
		palmprintf("http://www.wapuniverse.com/", g);
		palmHyperLink(false,NULL,NULL,g);
		palmprintln(g);
		palmprintln(g);
		palmprintln(g);
		palmprintln(g);
		palmprintln(g);

    		prevDrawWin = WinGetDrawWindow();
		palmResizeWindow(90,g); 
    		WinSetDrawWindow(g->DrawWin);
		resP=MemHandleLock(DmGetResource(bitmapRsc, bmpWAPUniverseBlue));
    		WinDrawBitmap (resP, 0, 133);
    		MemPtrUnlock(resP);
		WinSetDrawWindow(prevDrawWin);
		updateScreen(g);

		//palmprintf("Security provided by: <NULL>", g);
		palmprintf("No Security.", g);
		palmprintln(g);
		palmprintf("JpegLib ", g); palmprintf(JPEGLIBVER, g);
		palmprintln(g);
		palmprintf("iScriptLib ", g); palmprintf(SCRIPTLIBVER, g); //Should make a call to check
		palmprintln(g);
		palmprintf("libhtmlparse ", g); palmprintf(LIBHTMLPARSEVER, g);
		palmprintln(g);
		palmprintln(g);
		palmprintf("Built on ", g); palmprintf(BUILDDATE, g);
		palmprintf(". Build ", g); palmprintf(BUILDNUMSTR, g); palmprintf(", ", g);
		palmprintf("Revision ", g); palmprintf(REVISION, g); palmprintf(".", g);

#ifdef IS_RELEASE_CANDIDATE
		palmprintln(g);
		palmprintf("Release Candidate ", g); palmprintf(RCVERSIONSTR, g);
		palmprintf(" (", g); palmprintf("RC", g); palmprintf(RCVERSIONSTR, g); palmprintf(")", g);
#endif
*/
		g->state = BS_IDLE;
	} else if (StrNCaselessCompare(g->Url,"about:blank",11)==0) {
		palmResetState(g);

		g->state = BS_IDLE;
		browserSetTitle("Universe");

		PointerDrawPointer(&g->pointer, g);
	} else if (StrNCaselessCompare(g->Url,"about:wapuniverse",17)==0) {
		/*palmResetState(g);

		g->state = BS_IDLE;
		browserSetTitle("WAPUniverse");

		palmprintln(g);
		palmItalic(true);
		palmprintf("\"In the beginning God created the heavens and the earth.\"", g);
		palmItalic(false);
		palmprintln(g);
		g->Align = Right;
		palmSmall(true);
		palmprintf("Genesis 1:1", g);
		palmSmall(false);
		g->Align = Left;*/

		g->state = BS_IDLE;
	} else {
		return;
	}
}

Int16 wsp_getOffline(Int16 index, WebDataType *webData, Boolean remember, GlobalsType *g)
{
    	MemHandle 	handle;
    	UInt32 		len; //, i;
	Char		*hand;

	WspFreePdu(webData);

    	webData->transaction_id = 0;
    	webData->pdu_type       = WSP_PDU_REPLY;
    	webData->status_code    = WSP_STATUS_OK;
    	webData->data           = NULL;
	if (g->isImage != true) g->charSet = UTF_8;

    	ErrFatalDisplayIf(g->offline.mtchDb == NULL, "Channel DB not open [wsp_getOffline]");

    	handle = DmQueryRecord(g->offline.mtchDb, index);

	if (!handle && DmGetLastErr()) { ErrAlert(DmGetLastErr()); return -1; }

    	// version 2 and greater use 4-byte alignment
	hand = MemHandleLock(handle);
    	len = StrLen(hand);

    	len = (g->offline.mtchVersion < 2) ? (len + 2) & ~1 : (len + 4) & ~3;

    	if((webData->length = MemHandleSize(handle) - len) > 0) {
		if(remember)
			StrCopy(g->WapLastUrl,g->Url);

        	webData->data = Malloc(webData->length + 1);
		/*for (i=0; i < webData->length; i++) {
			webData->data[i] = hand[i + len];
		}*/
		MemMove(webData->data, hand + len, webData->length);
		if (g->isImage != true) g->contentLength = webData->length;
	} else {
		MemHandleUnlock(handle);
    		webData->transaction_id = 0;
    		webData->pdu_type       = 0;
    		webData->status_code    = 0;
    		webData->data           = NULL;
		return -1;
	}

	MemHandleUnlock(handle);

    	return 0;
}


Err ChannelOpen(Char *name, Boolean sameChannel, GlobalsType *g)
{
    	Boolean t1 = (g->offline.mtchDb == NULL);
    	Boolean t2 = StrCompare(g->offline.channel, name);

    	if(sameChannel && t2)
        	return browserErrPageNotFound;

    	if(t1 || t2)
    	{
       		UInt16 cardNo = 0;
        	LocalID dbID;

        	ChannelClose(g);
        	StrCopy(g->offline.channel, name);

        	if((dbID = DmFindDatabase(cardNo, g->offline.channel)) != NULL)
        	{
        		// open channel database
            		g->offline.mtchDb = DmOpenDatabase(cardNo, dbID, dmModeReadWrite);
            		//ErrFatalDisplayIf(g->offline.mtchDb == NULL, "Cannot open channel DB [ChannelOpen]");
			if (!g->offline.mtchDb) {
				MyErrorFunc("Could not open the requested channel. It is possible that it is open in another tab. Please close the open copy before loading it in another tab.", NULL);
				return browserErrGeneric;
			}

            		// extract channel database version info
            		DmDatabaseInfo(cardNo, dbID, 0, 0, &g->offline.mtchVersion, 0, 0, 0, 0, 0, 0, 0, 0);
        	}
        	else
        	{
			// display error
            		return browserErrPageNotFound;
        	}
    	}
    
    	return errNone;
}

/**
 * Retrieves channel content by the URL specified. The URL scheme *must* be
 * in form 'mtch:<channel name>/<path>'. It also accepts URL shortcuts in form
 * of 'mtch:<record index>'.
 *
 * @param url the content URL to be retrieved
 * @param sameChannel flag
 * @param ctP pointer to content type holder (may be NULL)
 * @return record index of the currently loaded channel DB or dmMaxRecordIndex
 */
Err ChannelGetIndex(Char *url, Boolean sameChannel, UInt16 *indexP, UInt16 *ctP, GlobalsType *g)
{
    Char *ptr = url + 5, *tmp;
    UInt16 index = 0, count;
    Err err;

    while(*ptr >= '0' && *ptr <= '9')
        index = 10 * index + *ptr++ - '0';

    if(*ptr && *ptr != '#')
    {
        ptr = url + 5;

        if((tmp = StrChr(ptr, '/')) != NULL)
        {
            Char channel[32];

            StrNCopy(channel, ptr, tmp - ptr);
            channel[tmp - ptr] = '\0';

            err = ChannelOpen(channel, sameChannel, g);
            if(err) return err;

            count = DmNumRecords(g->offline.mtchDb);
            for(index = 0; index < count; index++)
            {
                MemHandle handle = DmQueryRecord(g->offline.mtchDb, index);
                Char *str = MemHandleLock(handle);
                Int16 len = StrLen(str);
                Int16 res = StrNCompare(str, tmp, len);

                if(res == 0 && tmp[len] != 0 && tmp[len] != '#') res = 1;
                MemHandleUnlock(handle);

                if(res == 0) goto found;
            }

            // URL not found
            return browserErrPageNotFound;
        }
        else
        {
            // the URL has the following format: mtch:<channel>
            // go to the first page
            err = ChannelOpen(url + 5, sameChannel, g);
            if(err) return err;

            index = 0;
        }
    }


found:
    if(ctP)
    {
        DmRecordInfo(g->offline.mtchDb, index, ctP, NULL, NULL);
        *ctP &= dmRecAttrCategoryMask;

	switch(*ctP) {
		case browserContentTypeWBMP:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("image/vnd.wap.wbmp")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "image/vnd.wap.wbmp");
			break;
		case browserContentTypeWMLScript:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("application/vnd.wap.wmlscriptc")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "application/vnd.wap.wmlscriptc");
			break;
		case browserContentTypeBitmap:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("image/vnd.palm.pbmp")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "image/vnd.palm.pbmp");
			break;
		case browserContentTypeJPEG:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("image/jpeg")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "image/jpeg");
			break;
		case browserContentTypeGIF:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("image/gif")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "image/gif");
			break;
		case browserContentTypePNG:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("image/png")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "image/png");
			break;
		case browserContentTypeBMP:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("image/bmp")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "image/bmp");
			break;
		case browserContentTypeWMLC:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("application/vnd.wap.wmlc")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "application/vnd.wap.wmlc");
			break;
		case browserContentTypeHTML:
		case browserContentTypeXHTML: // Should handle different??
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("text/html")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "text/html");
			break;
		case browserContentTypeRSS:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("application/rss+xml")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "application/rss+xml");
			break;
		default:
			if (g->contentType) Free(g->contentType);
	    		g->contentType = Malloc(StrLen("application/octet-stream")+1);
				ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
			StrCopy(g->contentType, "application/octet-stream");
			break;
	}

	if (g->contentType && (g->isImage != true)) {
		if (g->pageContentType) Free(g->pageContentType);
		g->pageContentType = Malloc(StrLen(g->contentType)+1);
			ErrFatalDisplayIf (!g->pageContentType, "WspDecode: Malloc Failed");
	    	StrCopy(g->pageContentType, g->contentType);
	}
    }

    *indexP = index;
    g->offline.record = index;
    return 0;
}

Err ChannelClose(GlobalsType *g)
{

    if(g->offline.mtchDb)
    {
	//TODO: clear context

        DmCloseDatabase(g->offline.mtchDb);
        g->offline.mtchDb = NULL;
	g->offline.record = 0;
    }

    return 0;
}

// iConnect Channel posting

#define browserPostType           'POST'

DmOpenRef Application_OpenPostDB(const Char *channel, Boolean create)
{
    Err err;
    LocalID dbID;
    UInt16 cardNo = 0;
    DmOpenRef dbP = NULL;
    
    Char buf[40]; // large enough to store DB name prefixed by '.POST'
    UInt32 creator = appFileCreator;

    // form channel POST db name
    StrCopy(buf, channel);
    StrCat(buf, ".POST");

    if((dbID = DmFindDatabase(cardNo, buf)) == NULL && create)
    {
        // obtain channel creator
        dbID = DmFindDatabase(cardNo, channel);
        if(!dbID) return NULL;

        DmDatabaseInfo(cardNo, dbID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &creator);

        err = DmCreateDatabase(cardNo, buf, creator, browserPostType, false);
        ErrFatalDisplayIf(err, "Cannot create POST data DB [Application_OpenPostDB]");

        if((dbID = DmFindDatabase(cardNo, buf)) == NULL)
        {
            ErrFatalDisplay("DmFindDatabase failed [Application_OpenPostDB]");
            return dbP;
        }
    }

    if(dbID && (dbP= DmOpenDatabase(cardNo, dbID, dmModeReadWrite)) == NULL)
        ErrFatalDisplay("DmOpenDatabase failed [Application_OpenPostDB]");

    return dbP;    
}

void Application_PostURL(const Char *channel, const Char *url)
{
    DmOpenRef 		dbP = Application_OpenPostDB(channel, true);
    GlobalsType		*g;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    if(dbP)
    {
        Char *title = (g->deckTitle)?g->deckTitle:""; // "";

        UInt16 index = dmMaxRecordIndex;
        UInt32 size  = StrLen(title) + 1 + StrLen(url) + 1;

        MemHandle handle;
        void *p;

        // append new record
        handle = DmNewRecord(dbP, &index, size);
        ErrFatalDisplayIf(handle == NULL, "DmNewRecord failed [Application_PostURL]");

        p = MemHandleLock(handle);

        DmStrCopy(p, 0, title);
        DmStrCopy(p, StrLen(title) + 1, url);

        MemHandleUnlock(handle);
        DmReleaseRecord(dbP, index, true);

        DmCloseDatabase(dbP);
    }
}


