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
#include 	"http.h"
#include 	"build.h"
#include 	"WAPUniverse.h"
#include 	"wtp.h"
#include 	"wsp.h"
#include 	"WAP.h"
#include 	"../res/WAPUniverse_res.h"
#include 	"dbConn.h"
#include 	"dbUrl.h"
#include 	"formBrowser.h"
#include 	"PalmUtils.h"
#include 	"wml.h"
#include	"base64.h"
#include	"process.h"
#include	"Cookie.h"
#include	"debug.h"
#include	"Net.h"
#include	"Error.h"
#include	"Media.h"
#include	"URL.h"
#include	"xhtml.h"
#include	"xhtml_string.h"
#include	"DOM.h"
#include	"Hyperlinks.h"
#include	"Layout.h"
#include	"Form.h"


Boolean MediaExtractURLAndMIME(Char *mediaLink, Char *url, Char *mime)
{
	Char	*pos = NULL;

	if (StrNCompare(mediaLink, "media:", 6))
		return false;

	StrCopy(mime, mediaLink + 6);
	pos = StrChr(mime, ':');
	*pos = NUL;

	pos = StrChr(mediaLink + 6, ':');
	StrCopy(url, pos + 1);

	return true;
}

void MediaCreateEmbededLink(Char *media, Char *mime, GlobalsType *g)
{
	BitmapPtr 	resP;
	Char		*tmpUrl = NULL;
	Char		*url;
	ImagePtr	img = NULL;

	url = Malloc(MAX_URL_LEN + StrLen(mime) + 8);
	if (!url)
		return;
	tmpUrl = Malloc(MAX_URL_LEN+1);

	if (!mime)
		return;

	URLCreateURLStr(media,tmpUrl,g);
	StrCopy(url, mime);
	StrCopy(url + StrLen(url), ":");
	StrCopy(url + StrLen(url), tmpUrl);
	Free(tmpUrl);

	streamLink = true;
	HyperlinkSetLink(true, "media:", url, NULL, g);


    	resP = DOMResourceToPtr(bitmapRsc, bmpStreamBarPlay);
	img = addPageImage(NULL, NULL, g->x, g->x + (40 * HiresFactor()),
				       g->y, g->y + (20 * HiresFactor()),
				       (40 * HiresFactor()), (20 * HiresFactor()), resP, g);
	if (img)
		img->alreadyHires = true;

	g->x += (40 * HiresFactor());

	browserResizeDimensions(40 * HiresFactor(), 20 * HiresFactor(), g);
	HyperlinkSetLink(false, NULL, NULL, NULL, g);
	streamLink = false;

    	updateScreen(g);

	/*xhtml_renderLn(g);*/

	Free(url);
}

Err MediaLaunchKinomaURL(Char *url, Char *mime)
{
	ExgSocketType		exgSocket;
	UInt32			sizeSent;
	UInt16			appCard;
	LocalID			appID;
	DmSearchStateType	searchState;
	Char			*pVer;
	DmOpenRef		dmRef;
	MemHandle		hRsc;
	Boolean			isPlayer4;
	Err			err;

	err = DmGetNextDatabaseByTypeCreator(true, &searchState,
					     sysFileTApplication, kinomaAppCreator, true,
					     &appCard, &appID);
	if (err != errNone) {
		MyErrorFunc("Could not stream data. Kinoma Player v4 is not installed.", NULL);
		goto kinoma_cleanup;
	}

	dmRef = DmOpenDatabase(appCard, appID, dmModeReadOnly);
	if (!dmRef) {
		err = -1;
		MyErrorFunc("Could not stream data. Kinoma Player v4 is not installed or failed to open.", NULL);
		goto kinoma_cleanup;
	}

	hRsc = DmGet1Resource('tver', 1000);
	pVer = MemHandleLock(hRsc);
	isPlayer4 = (*pVer == '4');
	MemHandleUnlock(hRsc);
	DmCloseDatabase(dmRef);
	if (!isPlayer4) {
		err = -1;
		MyErrorFunc("Could not stream data. Kinoma Player v4 is not installed.", NULL);
		goto kinoma_cleanup;
	}

	MemSet(&exgSocket, sizeof(exgSocket), 0);
	exgSocket.length = StrLen(url) + 1;
	exgSocket.target = kinomaAppCreator;
	exgSocket.goToCreator = kinomaAppCreator;
	exgSocket.localMode = true;
	exgSocket.noStatus = true;
	exgSocket.type = mime;
	
	err = ExgPut(&exgSocket);
	if (err == errNone) {
		sizeSent = ExgSend(&exgSocket, url, exgSocket.length, &err);
		ExgDisconnect(&exgSocket, err);
	} else {
		MyErrorFunc("Could not stream data. Unable to communicate with the application.", NULL);
	}

kinoma_cleanup:
	return err;
}
