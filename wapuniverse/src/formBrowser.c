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

#define ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS

#include	<PalmOS.h>
#include	<SysEvtMgr.h>
#include	<SystemResources.h>
#include	<TelephonyMgr.h>
#include 	<TelephonyMgrTypes.h>
#include 	<NetMgr.h>
#include	<FrmGlue.h>

#ifdef HAVE_PALMONE_SDK
#include	<common/system/palmOneNavigator.h>
#include	<common/system/palmOneChars.h>
#include	<common/system/HSKeyTypes.h>
#include 	<KeyMgr.h>
#include 	<68K/Hs.h>
#include 	<68K/Libraries/PmKeyLib/PmKeyLib.h>
#endif

#ifdef HAVE_SONY_SDK
#include	<SonyChars.h>
#endif

#include 	"WAPUniverse.h"
#include	"build.h"
#include 	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"
#include	"formBrowser.h"
#include	"formUrlList.h"
#include	"formConnectionList.h"
#include	"formEditConnection.h"
#include	"formEditUrl.h"
#include	"frmScroll.h"
#include	"dbConn.h"
#include	"dbOffline.h"
#include	"history.h"
#include	"MenuHnd.h"
#include 	"wml.h"
#include	"wmls.h"
#include	"wtp.h"
#include	"variable.h"
#include	"xhtml.h"
#include	"rss.h"
#include	"http.h"
#include	"Form.h"
#include	"FiveWay.h"
#include	"String.h"
#include	"vfs.h"
#include	"Adjustor.h"
#include	"Layout.h"
#include	"Cache.h"
#include	"DIA.h"
#include	"resize.h"
#include	"URL.h"
#include	"BrowserAPI.h"
#include	"Text.h"
#include	"DOM.h"
#include	"Browser.h"
#include	"Area.h"
#include	"Font.h"
#include	"Tab.h"
#include	"debug.h"
#include	"Hyperlinks.h"
#include	"Cookie.h"
#include	"Net.h"
#include	"Media.h"
#include	"Pointer.h"
#include	"Download.h"
#include	"FileURI.h"

#ifdef __GNUC__
#include "callbacks.h"
#endif

extern int WTPDisconnect (Int16 sock, GlobalsType *g);
static Boolean browserPenUpEvt(EventPtr event, GlobalsType *g);
static Boolean browserPenDownEvt(EventPtr event, GlobalsType *g);
static Boolean browserPenOverEvt(EventPtr event, GlobalsType *g);
void scrollUpdate(GlobalsType *g);
Boolean runBrowserState(GlobalsType *g);
Char *ConvertFromTo(CharEncodingType srcEncoding, Char *sourceStr, Char *destStr);
void browserLoadInternalPage(GlobalsType *g);




void setTitleFont(Boolean ison)
{
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (ison)
		FntSetFont(smallNormalFont);
	else
		FntSetFont(stdFont);
}

static UInt16 browserGetTitleWidth(void)
{
	RectangleType	bounds;

	FrmGetObjectBounds(FrmGetActiveForm(), 
			   FrmGetObjectIndex(FrmGetActiveForm(), BrowserTitle),
			   &bounds);

	return bounds.extent.x;
}

/*static UInt16 browserGetTitleHeight(void)
{
	RectangleType	bounds;

	FrmGetObjectBounds(FrmGetActiveForm(), 
			   FrmGetObjectIndex(FrmGetActiveForm(), BrowserTitle),
			   &bounds);

	return bounds.extent.y;
}*/

static UInt16 browserGetTitleX(void)
{
	RectangleType	bounds;

	FrmGetObjectBounds(FrmGetActiveForm(), 
			   FrmGetObjectIndex(FrmGetActiveForm(), BrowserTitle),
			   &bounds);

	return bounds.topLeft.x;
}

static UInt16 browserGetTitleY(void)
{
	RectangleType	bounds;

	FrmGetObjectBounds(FrmGetActiveForm(), 
			   FrmGetObjectIndex(FrmGetActiveForm(), BrowserTitle),
			   &bounds);

	return bounds.topLeft.y;
}

void browserSetTitle(Char *title)
{
	WinDrawOperation 	oldMode;
    	BitmapPtr 		resP = 0;
	RGBColorType 		rgb;
	IndexedColorType	index, /*origFore = 0,*/ origText = 0;
	Int16			titleX = browserGetTitleX(); // 16;
	Int16			titleY = browserGetTitleY(); // 0; // 1
	GlobalsType		*g;
	//FontID			currFont = FntGetFont();

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (WinGetActiveWindow() != FrmGetWindowHandle(FrmGetFormPtr(frmBrowser)))
		return;

	rgb.r = 128;
	rgb.g = 128;
	rgb.b = 128;

	if (!title)
		return;

	if (g->browserGlobals.browserTitle)
		Free(g->browserGlobals.browserTitle);

	g->browserGlobals.browserTitle = StrDup(title);
	
    	index = WinRGBToIndex( &rgb );
	origText = WinSetTextColor(index);

	if (!g->rssFeed)
		RSSShowIndicator(false);

	if (g->state == BS_HTTP_RECEIVE && !g->isImage &&
	    (g->pageContentType && (StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlscriptc",30)!=0) &&
	    (StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlc",24)!=0) &&
	    (StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29)!=0) && 
	    (StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21)!=0) && 
	    (StrNCaselessCompare(g->pageContentType,"application/wml+xml",19)!=0) &&
	    (StrNCaselessCompare(g->pageContentType,"text/html",9)!=0) &&
	    (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)!=0) &&
	    (StrNCaselessCompare(g->pageContentType,"application/rss+xml",19)!=0))
	    && (http_clength > 0) && (StrCompare(title, "Receiving URL")==0) && (http_clength == g->browserGlobals.savedclength)) {

		//RectangleType 	r;

		if (g->browserGlobals.updateMenu == true || http_received > g->browserGlobals.savedrlength) {

			if (g->browserGlobals.redrawMenuBar) {
				resP = MemHandleLock(DmGetResource(bitmapRsc, bmpMenu));
    				WinDrawBitmap (resP, 0, 0);
    				MemPtrUnlock(resP);
				resP = MemHandleLock(DmGetResource(bitmapRsc, bmpTopLeft));
    				WinDrawBitmap (resP, browserGetScreenWidth() - 4, 0);
    				MemPtrUnlock(resP);

				g->progress--;
				showProgress(g);

				g->browserGlobals.redrawMenuBar = false;
			}

			g->browserGlobals.updateMenu = false;
		
			if (http_received > g->browserGlobals.savedrlength) {
				Char	received[50];

				setTitleFont(true);

				StrPrintF(received,"%ld K", http_received / 1024);

				/*
				resP = MemHandleLock(DmGetResource(bitmapRsc, bmpMenuChunk));
    				WinDrawBitmap (resP, browserGetScreenWidth() - FontCharsWidth(received, StrLen(received)) - 36, 0);
    				MemPtrUnlock(resP);
				*/

				TabDrawTabs(true, g);

				oldMode = WinSetDrawMode(winOverlay);
				WinPaintChars(received,StrLen(received),/*browserGetScreenWidth()*/
					      browserGetTitleX() + browserGetTitleWidth() - FontCharsWidth(received, StrLen(received)) - 1,
					      /*2*/ browserGetTitleY());
				WinSetDrawMode(oldMode);

				setTitleFont(false);
			} else {
				resP = MemHandleLock(DmGetResource(bitmapRsc, bmpMenu));
    				WinDrawBitmap (resP, 0, 0);
    				MemPtrUnlock(resP);
				resP = MemHandleLock(DmGetResource(bitmapRsc, bmpTopLeft));
    				WinDrawBitmap (resP, browserGetScreenWidth() - 4, 0);
    				MemPtrUnlock(resP);

				TabDrawTabs(true, g);

				g->progress--;
				showProgress(g);
			}
		}

		/*
		origFore = WinSetForeColor(index);

		r.topLeft.x = browserGetTitleX() + 1; //16;
    		r.topLeft.y = browserGetTitleY() + 2; //3;
    		r.extent.x = browserGetTitleWidth() - 15; //76;
    		r.extent.y = 6;
		WinEraseRectangleFrame(simpleFrame, &r);

		r.topLeft.x = browserGetTitleX() + 2; // 17;
    		r.topLeft.y = browserGetTitleY() + 3; // 4;
		r.extent.x = (http_received * (browserGetTitleWidth() - 17)) / http_clength;
    		r.extent.y = 4;
		WinEraseRectangle(&r, 0);

		WinSetForeColor(origFore);
		*/

		g->browserGlobals.savedrlength = http_received;

	} else {
		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpMenu));
    		WinDrawBitmap (resP, 0, 0);
    		MemPtrUnlock(resP);
		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpTopLeft));
    		WinDrawBitmap (resP, browserGetScreenWidth() - 4, 0);
    		MemPtrUnlock(resP);

		g->progress--;
		showProgress(g);

		TabDrawTabs(true, g);

		//currFont = FntSetFont(boldFont);
		setTitleFont(true);

		oldMode = WinSetDrawMode(winOverlay);
		WinPaintChars(title,FntWordWrap(title,/*96*/ browserGetTitleWidth()),titleX,titleY);
		WinSetDrawMode(oldMode);

		setTitleFont(false);
		//FntSetFont(currFont);

	}

	WinSetTextColor(origText);

	if (g->rssFeed)
		RSSShowIndicator(true);

	g->browserGlobals.savedclength = http_clength;
}

void browserSwitchMode(Int16 mode)
{
    	BitmapPtr 	resP = 0;
	RectangleType	rect;
	GlobalsType	*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (mode == MODE_ONEHAND) {
		/*FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonBack));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonReload));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonStop));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonHome));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonOpenURL));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonBook));*/

		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), &rect);
		rect.extent.y = browserGetScreenHeight() - (12 + 14 + 14) - 7;
		FrmSetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), &rect);
		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserScrollBar), &rect);
		rect.extent.y = browserGetScreenHeight() - (12 + 14 + 14) - 13;
		FrmSetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserScrollBar), &rect);
		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserScrollBarHorizontal), &rect);
		rect.topLeft.y = browserGetScreenHeight() - (7 + 14);
		FrmSetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserScrollBarHorizontal), &rect);

		FrmDrawForm(FrmGetActiveForm());
		//browserSetTitle(g->browserGlobals.browserTitle);
		TabDrawTabs(false, g);
		updateScreen(g);

    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpBar));
    		WinDrawBitmap (resP, 0, browserGetScreenHeight() - 14);
    		MemPtrUnlock(resP);

    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpDivide));
    		WinDrawBitmap (resP, 66, browserGetScreenHeight() - 12);
    		MemPtrUnlock(resP);

    		/*resP = MemHandleLock(DmGetResource(bitmapRsc, bmpProgress1));
    		WinDrawBitmap (resP, browserGetScreenWidth() - 25, browserGetScreenHeight() - 13);
    		MemPtrUnlock(resP);

    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpLine));
    		WinDrawBitmap (resP, 0, browserGetScreenHeight() - 15);
    		MemPtrUnlock(resP);*/

		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwoa));
	} else {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwoa));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwob));

		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), &rect);
		rect.extent.y = browserGetScreenHeight() - (12 + 14) - 7;
		FrmSetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), &rect);
		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserScrollBar), &rect);
		rect.extent.y = browserGetScreenHeight() - (12 + 14) - 13;
		FrmSetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserScrollBar), &rect);
		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserScrollBarHorizontal), &rect);
		rect.topLeft.y = browserGetScreenHeight() - 7;
		FrmSetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserScrollBarHorizontal), &rect);

		FrmDrawForm(FrmGetActiveForm());
		//browserSetTitle(g->browserGlobals.browserTitle);
		TabDrawTabs(false, g);
		updateScreen(g);

    		/*resP = MemHandleLock(DmGetResource(bitmapRsc, bmpBar));
    		WinDrawBitmap (resP, 0, browserGetScreenHeight() - 14);
    		MemPtrUnlock(resP);

    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpDivide));
    		WinDrawBitmap (resP, 29, browserGetScreenHeight() - 12);
    		WinDrawBitmap (resP, 58, browserGetScreenHeight() - 12);
    		MemPtrUnlock(resP);

    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpProgress1));
    		WinDrawBitmap (resP, browserGetScreenWidth() - 25, browserGetScreenHeight() - 13);
    		MemPtrUnlock(resP);

    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpLine));
    		WinDrawBitmap (resP, 0, browserGetScreenHeight() - 15);
    		MemPtrUnlock(resP);*/

		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonBack));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonReload));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonStop));
		//FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonHome));
		//FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonOpenURL));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonBook));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonGo));
	}
}

void browserSwitchSoftTwo(GlobalsType *g)
{
	if (g_prefs.mode == MODE_ONEHAND) {

		if (g->state == BS_IDLE) {
			if (!FrmGlueGetObjectUsable(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwoa))) {
				FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwob));
				FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwoa));
			}
		} else {
			if (!FrmGlueGetObjectUsable(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwob))) {
				FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwoa));
				FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwob));
			}
		}
	}
}

void MyErrorFunc (char* error, char* additional)
{
	if (additional == NULL)
    		FrmCustomAlert (alErrorTwo, error, "", NULL);
	else
    		FrmCustomAlert (alError, error, additional ? additional : "", NULL);
}

void updateScreen(GlobalsType *g)
{
	//DOMRenderDOM(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), g->ScrollX, g->ScrollY, g->ScrollX + browserGetWidgetBounds(WIDGET_WIDTH), g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT), true, true, g);

	scrollUpdate(g);
}

int getCard(Char *name, GlobalsType *g)
{
  CardPtr tmp;

  	tmp = g->wmldck.card;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->card_id)==0)
		{
			return tmp->offset;
		}
		tmp = tmp->next;
	}
    	return(0);
}

/*char *escaped(Char *str)
{
	Char *temp = Malloc(StrLen(str)+1);
	
	if (!temp) return str;

	StrCopy(temp, str);

	temp = EscapeCharacters(temp);

 	return temp;
}

char *unescaped(Char *str)
{
	Char *temp = Malloc(StrLen(str)+1);

	if (!temp) return str;

	StrCopy(temp, str);

	temp = UnEscapeCharacters(temp);

 	return temp;
}*/

/*char *escaped(Char *str)
{
	//Char *temp = Malloc(StrLen(str)+1);
	Char *new;
	
	//if (!temp) return str;

	//StrCopy(temp, str);

	//new = EscapeCharacters(temp);
	new = EscapeCharacters(str);

	//Free(new);
	//Free(temp);

 	return new;
}*/

// buggy??
char *escaped(Char *str)
{
 	return EscapeCharacters(str);
}

char *unescaped(Char *str)
{
	Char *temp = Malloc(StrLen(str)+1);
	Char *new;

	if (!temp) return str;

	StrCopy(temp, str);

	new = UnEscapeCharacters(temp);

	//Free(new);
	//Free(temp);

 	return new;
}

void palmResetCardState(GlobalsType *g) {

    	g->DrawHeight = 0; // browserGetWidgetBounds(WIDGET_HEIGHT);
	g->DrawWidth = browserGetWidgetBounds(WIDGET_WIDTH);
	g->BrowserAPI.lastWidgetWidth = browserGetWidgetBounds(WIDGET_WIDTH);
    	g->ScrollY = 0;
	g->ScrollX = 0;
    	SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBar),0,0,0,browserGetWidgetBounds(WIDGET_HEIGHT) - (11 * HiresFactor()));
	SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal),0,0,0,browserGetWidgetBounds(WIDGET_WIDTH) - (11 * HiresFactor()));
    	g->x=0;g->y=0;

	DOMDeleteTree(g);

	g->engineStop = false;
	g->userCancel = false;

	g->browserGlobals.tapandhold = false;
	g->browserGlobals.taptick = 0;
	g->browserGlobals.tapx = 0;
	g->browserGlobals.tapy = 0;

	return;
}

void palmResetState(GlobalsType *g) {
	RectangleType 	r;

	r.topLeft.x = browserGetWidgetBounds(WIDGET_TOPLEFT_X) / HiresFactor();
	r.topLeft.y = browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor();
	r.extent.x = browserGetWidgetBounds(WIDGET_WIDTH) / HiresFactor();
	r.extent.y = browserGetWidgetBounds(WIDGET_HEIGHT) / HiresFactor();

	if (g->browserGlobals.optMenuOpen == true) {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
		g->browserGlobals.optMenuOpen = false;
	}

    	g->DrawHeight = 0; // browserGetWidgetBounds(WIDGET_HEIGHT);
	g->DrawWidth = browserGetWidgetBounds(WIDGET_WIDTH);
	g->BrowserAPI.lastWidgetWidth = browserGetWidgetBounds(WIDGET_WIDTH);
    	g->ScrollY = 0;
	g->ScrollX = 0;
	WinEraseRectangle(&r,0);
    	SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBar),0,0,0,browserGetWidgetBounds(WIDGET_HEIGHT) - (11 * HiresFactor()));
	SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal),0,0,0,browserGetWidgetBounds(WIDGET_WIDTH) - (11 * HiresFactor()));
    	g->x=0;g->y=0;

	g->engineStop = false;
	g->userCancel = false;

	DOMDeleteTree(g);
	g->InputFldActive=NULL;
	//WspFreePdu(&(g->webData));
	HyperlinkFreeLinks(g);
	//disposeVar(g);
	disposeInput(g);
	disposeSelect(g);
	destroyForms(g);
	TextFreeText(g);
	freePageLocation(g);
	freePageImages(g);
	AreaFreeAreas(g);
	SARdeleteData(g);
	wbxml_free_wmldck(&(g->wmldck));
	xhtml_free(&(g->xhtml), g);
	rss_free(&(g->xhtml), g);


	if (g->rssFeed) {
		Free(g->rssFeed);
		g->rssFeed = NULL;
	}
	RSSShowIndicator(false);

	g->browserGlobals.tapandhold = false;
	g->browserGlobals.taptick = 0;
	g->browserGlobals.tapx = 0;
	g->browserGlobals.tapy = 0;

	return;
}

void followLink(unsigned char *str, GlobalsType *g)
{
	CardPtr			tmpCrd;
  	Char			*urlStr;
  	Err			err;
  	UInt16			cardNo;
  	LocalID			dbID;
  	DmSearchStateType	searchState;
  	UInt32			creator;
  	UInt32			resultP = 0;
  	Char			*iDialNumberP;
  	//Char			*posC;
  	//Char			*newUrl;
  
  	FormInputDeactivate(g);

	if (!str)
		return;

	if ((g->pageContentType && (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29) || 
	     !StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21) || 
	     !StrNCaselessCompare(g->pageContentType,"application/wml+xml",19) ||
	     !StrNCaselessCompare(g->pageContentType,"text/html",9) ||
	     !StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16) ||
	     !StrNCaselessCompare(g->pageContentType,"application/rss+xml",19)))
	    && !StrNCaselessCompare(str,"#",1)) {

				scrollToPageLocation(str+1, g);
				return;
	} else if (!StrNCaselessCompare(str,"<NOOP/>",7)) {
		g->state = BS_IDLE;
  		return;
  	} else if (!StrNCaselessCompare(str,"mailto:",7)) {
		if (ExgGetDefaultApplication (&creator, exgRegSchemeID, "mailto")){
	  		MyErrorFunc("You do not have compatible e-mail software installed.","mailto");
	  		return;
	  	} else {
	  		DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication,
	        	creator, true, &cardNo, &dbID);
	    		err = SysAppLaunch(cardNo, dbID, 0, sysAppLaunchCmdGoToURL, str, &resultP);
	    		return;
	   	}
  	  	return;
  	} else if (!StrNCaselessCompare(str,"rtsp://",7)) {
		if (ExgGetDefaultApplication (&creator, exgRegSchemeID, "rtsp")){
	  		MyErrorFunc("Could not find an application to view the streaming media.","Media Stream");
	  		return;
	  	} else {
	  		DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication,
	        	creator, true, &cardNo, &dbID);
	    		err = SysAppLaunch(cardNo, dbID, 0, sysAppLaunchCmdGoToURL, str, &resultP);
	    		return;
	   	}
  	  	return;
  	} else if (!StrNCaselessCompare(str,"mms://",6)) {
		if (ExgGetDefaultApplication (&creator, exgRegSchemeID, "mms")){
	  		MyErrorFunc("Could not find an application to view the streaming media.","Media Stream");
	  		return;
	  	} else {
	  		DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication,
	        	creator, true, &cardNo, &dbID);
	    		err = SysAppLaunch(cardNo, dbID, 0, sysAppLaunchCmdGoToURL, str, &resultP);
	    		return;
	   	}
  	  	return;
  	} else if (!StrNCaselessCompare(str, "media:", 6)) {
		Char	url[MAX_URL_LEN + 1];
		Char	mime[MAX_URL_LEN + 1];

		MemSet(url, sizeof(url), 0);
		MemSet(mime, sizeof(mime), 0);

		if (MediaExtractURLAndMIME(str, url, mime)) {
			MediaLaunchKinomaURL(url, mime);
		} else {
			MyErrorFunc("Could not open embeded data. Invalid media url format.", NULL);
		}
		
		//MyErrorFunc("Feature not available", "Media Stream");
		return;
	} else if (!StrNCaselessCompare(str, "palm:", 5)) {
        	UInt16 	cmd = 0;
        	UInt32 	crid = 0;
        	UInt32 	type = 'appl';
		Char	tmp[31];
		int	i = 0;

        	Err 	err;
        	LocalID dbID;
        	UInt16 	cardNo;
        	DmSearchStateType state;

		g->state = BS_IDLE;

        	if(StrChr(str, ':') && !StrChr(str, '#')) {
            		if(StrChr(str, '.') && (StrLen(str) > 9))//(StrLen(str) >= 14) && (str[10] == '.'))   // for 'palm:crid.type'
            		{
				StrCopy(tmp, str + 5);
				tmp[4] = '\0';
				i = 14;

               			MemMove(&crid, tmp, sizeof(crid));
				StrCopy(tmp, str + 10);
				tmp[4] = '\0';

                		MemMove(&type, tmp, sizeof(type));
            		} else if(StrLen(str) >= 9)                        // for 'palm:crid'
            		{
				StrCopy(tmp, str + 5);
				tmp[4] = '\0';
				i = 9;

                		MemMove(&crid, tmp, sizeof(crid));
            		}
            		
			if (StrChr(str, '?')) {
				StrCopy(tmp, str + i + 1);
            			cmd = StrAToI(tmp); // launch code in query
			} else {
				cmd = 0;
			}
        	}            

        	err = DmGetNextDatabaseByTypeCreator(true, &state, type, crid, false, &cardNo, &dbID);
        	if(!err && dbID) {
            		SysUIAppSwitch(cardNo, dbID, cmd, NULL);
            		return;
        	} else {
            		MyErrorFunc("Could not run app", NULL);
		}

		return;
    	} else if (!StrNCaselessCompare(str, "PalmCall:", 9)) {
       	UInt16 	cmd = 0;
        	UInt32 	crid = 0;
        	UInt32 	type = 'appl';
		Char	tmp[31];
		int	i = 0;

        	Err 	err;
        	LocalID dbID;
        	UInt16 	cardNo;
        	DmSearchStateType state;

		g->state = BS_IDLE;

        	if(StrChr(str, ':') && !StrChr(str, '#')) {
            		if(StrChr(str, '.') && (StrLen(str) > 13))//(StrLen(str) >= 14) && (str[10] == '.'))   // for 'palm:crid.type'
            		{
				StrCopy(tmp, str + 9);
				tmp[4] = '\0';
				i = 17;

               			MemMove(&crid, tmp, sizeof(crid));
				StrCopy(tmp, str + 14);
				tmp[4] = '\0';

                		MemMove(&type, tmp, sizeof(type));
            		} else if(StrLen(str) >= 13)                        // for 'palmcall:crid'
            		{
				StrCopy(tmp, str + 9);
				tmp[4] = '\0';
				i = 12;

                		MemMove(&crid, tmp, sizeof(crid));
            		}
            		
			if (StrChr(str, '?')) {
				StrCopy(tmp, str + i + 1);
            			cmd = StrAToI(tmp); // launch code in query
			} else {
				cmd = 0;
			}
        	}            

        	err = DmGetNextDatabaseByTypeCreator(true, &state, type, crid, false, &cardNo, &dbID);
        	if(!err && dbID) {
            		SysUIAppSwitch(cardNo, dbID, cmd, NULL);
            		return;
        	} else {
            		MyErrorFunc("Could not run app", NULL);
		}

		return;
	} else if (!StrNCaselessCompare(str,"wtai://wp/mc;",13)) {
  		iDialNumberP = Malloc(StrLen(str)+1);
  		StrCopy(iDialNumberP, str + 13);
		iDialNumberP = UnEscapeCharacters(iDialNumberP);
  		DisplayDial(iDialNumberP,1);
		Free(iDialNumberP);
  		return;
  	} else if (!StrNCaselessCompare(str,"wtai://wp/sd;",13)){
  		iDialNumberP = Malloc(StrLen(str)+1);
  		StrCopy(iDialNumberP, str + 13);
		iDialNumberP = UnEscapeCharacters(iDialNumberP);
  		DisplayDial(iDialNumberP,2);
		Free(iDialNumberP);
  		return;
  	} else if (!StrNCaselessCompare(str,"wtai://wp/ap;",13)){
  		iDialNumberP = Malloc(StrLen(str)+1);
  		StrCopy(iDialNumberP, str + 13);
		iDialNumberP = UnEscapeCharacters(iDialNumberP);
  		DisplayDial(iDialNumberP,3);
		Free(iDialNumberP);
  		return;
  	} else if (!StrNCaselessCompare(str,"tel:",4)){
	   	if (StrNCaselessCompare(str,"tel://",6)==0){
	  		iDialNumberP = Malloc(StrLen(str)+1);
	  		StrCopy(iDialNumberP, str + 6);
			iDialNumberP = UnEscapeCharacters(iDialNumberP);
	  		DisplayDial(iDialNumberP,1);
	  		return;
	  	} else {
	  		iDialNumberP = Malloc(StrLen(str)+1);
	  		StrCopy(iDialNumberP, str + 4);
			iDialNumberP = UnEscapeCharacters(iDialNumberP);
	  		DisplayDial(iDialNumberP,1); 
	  		return;
	  	}
  	} else if (!StrNCaselessCompare(str,"internal:open",13)){
  		FormInputDeactivate(g);
  		FrmPopupForm(frmUrl);
	  	return;
  	} else if (!StrNCaselessCompare(str,"internal:goto-url",18)){
  		FormInputDeactivate(g);
  		FrmPopupForm(frmUrl);
	  	return;
  	} else if(!StrNCaselessCompare(str,"internal:",9) || !StrNCaselessCompare(str,"about:",6)){
		g->state = BS_IDLE;
		if(g->Url)
			Free(g->Url);
		g->Url=Malloc(MAX_URL_LEN+1);
		ErrFatalDisplayIf (!g->Url, "Malloc gUrl");
		StrCopy(g->Url, str);
		historyPush(g->Url,g);

		browserLoadInternalPage(g);
  		return;
  	} else if (!StrNCaselessCompare(str,"cache:",6)) {
		g->state = BS_CHECK_CACHE;
		if(g->Url)
			Free(g->Url);
		g->Url = Malloc(MAX_URL_LEN+1);
		ErrFatalDisplayIf(!g->Url, "Malloc gUrl");
		StrCopy(g->Url, str);
		historyPush(g->Url,g);

  		return;
  	} /*else if (!StrNCaselessCompare(str,"mtch:",5)) {
		//A bit redundant...
		urlStr = expandVar(str,g);
		if (urlStr[0]=='#'){
			g->state = BS_NEW_CARD;
			tmpCrd = g->wmldck.card;

			newUrl = Malloc(StrLen(g->Url) + StrLen(urlStr) + 1);
			ErrFatalDisplayIf (!newUrl, "Malloc failed");

			StrCopy(newUrl, g->Url);

			if ((posC = StrChr(newUrl, '#'))) {
				StrCopy(newUrl+(posC-newUrl), urlStr);
			} else {
				StrCopy(newUrl+StrLen(newUrl), urlStr);
			}

			if(g->Url)
				Free(g->Url);
			g->Url=Malloc(MAX_URL_LEN+1);
			ErrFatalDisplayIf (!g->Url, "Malloc gUrl");
			URLCreateURLStr(newUrl,g->Url,g);

			Free(newUrl);
			historyPush(g->Url,g);
		
		
			while(tmpCrd){
				if(StrCompare(tmpCrd->card_id, &urlStr[1])==0){
					g->CardOffset = tmpCrd->offset;
					Free(urlStr);
					return;
				}
				tmpCrd = tmpCrd->next;
	  		}

     	  		MyErrorFunc("Requested card not found.", NULL);
			g->state = BS_IDLE;
	  		Free(urlStr);
		} else {
			if(g->Url)
				Free(g->Url);
			g->Url=Malloc(MAX_URL_LEN+1);
			ErrFatalDisplayIf (!g->Url, "Malloc gUrl");
			URLCreateURLStr(urlStr,g->Url,g);
			historyPush(g->Url,g);
			Free(urlStr);
  		}
		g->state = BS_GET_OFFLINE;
  		return;
  	}*/ else if (!StrNCaselessCompare(str,"server:",7)) {
		urlStr = expandVar(str,g);
		if (!urlStr) {
			MyErrorFunc("Failed to post requested data for sync.", NULL);
			return;
		}
        	Application_PostURL(g->offline.channel, urlStr);
        	FrmAlert(AlertSubmitted);
		Free(urlStr);
		return;
	} else if (!StrNCaselessCompare(str,"javascript:history.back",23)) {
		FormInputDeactivate(g);
        	browserBackEvt(g);
  	} else if (!StrNCaselessCompare(str,"<PREV/>",7)) {
		FormInputDeactivate(g);
        	browserBackEvt(g);
  	} else if (!StrNCaselessCompare(str,"<REFRESH/>",10)) {
		FormInputDeactivate(g);
		//browserReloadEvt(g);
		//followLink(g->Url, g);
		if (g->pageContentType && 
		    !StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlc",24)) {
			palmResetState(g);
			g->state=BS_RENDER;
			g->refreshEvent = true;
		} else if (g->pageContentType && 
			   (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29) || 
			    !StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21) || 
			    !StrNCaselessCompare(g->pageContentType,"application/wml+xml",19) ||
			    !StrNCaselessCompare(g->pageContentType,"text/html",9) ||
			    !StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16))) {
				palmResetState(g);
            			g->state = BS_XHTML_PARSE_RENDER;
				g->refreshEvent = true;
            	} else if (g->pageContentType && !StrNCaselessCompare(g->pageContentType,"application/rss+xml",19)) {
				palmResetState(g);
            			g->state = BS_RSS_PARSE_RENDER;
		} else {
			return;
		}
  	} else if (!StrNCaselessCompare(g->Url,"device:home",11)) {
		if (!StrNCaselessCompare(g->CurrentConnection.home,"device:home",11)){
			if(g->Url)
				Free(g->Url);
			g->Url=Malloc(MAX_URL_LEN+1);
				ErrFatalDisplayIf (!g->Url, "Malloc gUrl");
			URLCreateURLStr("device:home",g->Url,g);
			historyPush(g->Url,g);
			//Free(urlStr);
			browserReloadEvt(g);
			return;
		} else {
			browserHomeEvt(g);
			return;
		}
  	} else {
		urlStr = expandVar(str,g);
		if (urlStr[0]=='#'){
			if (g->pageContentType &&
			    (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29) || 
	       		     !StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21) || 
	      		     !StrNCaselessCompare(g->pageContentType,"application/wml+xml",19) ||
	       	             !StrNCaselessCompare(g->pageContentType,"text/html",9) ||
			     !StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16) ||
			     !StrNCaselessCompare(g->pageContentType,"application/rss+xml",19))) {
				scrollToPageLocation(urlStr+1, g);
				return;
			}

			g->state = BS_NEW_CARD;
			tmpCrd = g->wmldck.card;

			/*newUrl = Malloc(StrLen(g->Url) + StrLen(urlStr) + 1);
			ErrFatalDisplayIf (!newUrl, "Malloc failed");

			StrCopy(newUrl, g->Url);

			if ((posC = StrChr(newUrl, '#'))) {
				StrCopy(newUrl+(posC-newUrl), urlStr);
			} else {
				StrCopy(newUrl+StrLen(newUrl), urlStr);
			}

			if(g->Url)
				Free(g->Url);
			g->Url=Malloc(MAX_URL_LEN+1);
			ErrFatalDisplayIf (!g->Url, "Malloc gUrl");
			URLCreateURLStr(newUrl,g->Url,g);

			Free(newUrl);*/

			if(g->Url)
				Free(g->Url);
			g->Url = Malloc(MAX_URL_LEN+1);
				ErrFatalDisplayIf (!g->Url, "Malloc gUrl");
			URLCreateURLStr(urlStr,g->Url,g);
			historyPush(g->Url,g);

		
			while (tmpCrd) {
				if (StrCompare(tmpCrd->card_id, &urlStr[1])==0) {
					g->CardOffset = tmpCrd->offset;
					Free(urlStr);
					return;
				}
				tmpCrd = tmpCrd->next;
	  		}

     	  		MyErrorFunc("Requested card not found.", NULL);
			g->state = BS_IDLE;
	  		Free(urlStr);
	  		return;
		} else {
			if (g->Url)
				Free(g->Url);
			g->Url=Malloc(MAX_URL_LEN+1);
			ErrFatalDisplayIf (!g->Url, "Malloc gUrl");
			URLCreateURLStr(urlStr,g->Url,g);
			historyPush(g->Url,g);
			Free(urlStr);

			browserReloadEvt(g);
			return;
  		}
  	}
}

void browserResizeDimensions(Int16 width, Int16 height, GlobalsType *g)
{
	UInt16		pageWidth = 0;
	UInt16		pageHeight = 0;

	if (height > g->imgH) {
		g->imgH = height;
	}

	if (((g->x + width) > g->DrawWidth) && (g_prefs.renderMode == RENDER_WIDE_SCREEN)) {
        	g->DrawWidth = g->x + width;

		if (g->DrawWidth > browserGetWidgetBounds(WIDGET_WIDTH))
			pageWidth = g->DrawWidth - browserGetWidgetBounds(WIDGET_WIDTH) + (11 * HiresFactor());

		SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal), g->ScrollX, 0, MAX(pageWidth, 0), browserGetWidgetBounds(WIDGET_WIDTH) - (11 * HiresFactor()));
	}

	//if ((g->x > g->DrawWidth) && (g_prefs.renderMode == RENDER_WIDE_SCREEN))
	//	g->DrawWidth = g->x;

	if ((g->y + height) > g->DrawHeight) {
        	g->DrawHeight = g->y + height;

		if (g->DrawHeight > browserGetWidgetBounds(WIDGET_HEIGHT))
			pageHeight = g->DrawHeight - browserGetWidgetBounds(WIDGET_HEIGHT) + (11 * HiresFactor());

		SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBar), g->ScrollY, 0, MAX(pageHeight, 0), browserGetWidgetBounds(WIDGET_HEIGHT) - (11 * HiresFactor()));
	}

	//if (g->y > g->DrawHeight)
	//	g->DrawHeight = g->y;
}

void palminsertbr(GlobalsType *g)
{
    /*UInt16	tempX = g->x, tempY = g->y;

    AdjAlignLine(g);

    if (g->imgH) {
	palmResizeWindow(g->imgH, g);
	g->y += g->imgH;
	g->imgH = 0;
    } else {
	palmResizeWindow(BrowserLineheightValue(), g);
    	g->y+=BrowserLineheightValue();
    }

    g->browserGlobals.imgYAdd = true;

    g->x = g->indent * BrowserIndentValue();

    if (g->DrawHeight <= browserGetWidgetBounds(WIDGET_HEIGHT))
	xhtml_ReSetWindow(g);


	if (g->HyperLinks && 
	   (g->HyperLinks->x1 == tempX) && ((g->HyperLinks->y1) == tempY)) { // Hyperlink will start at end of last line
		g->HyperLinks->x1 = g->x;
		g->HyperLinks->y1 = g->y;
	}*/

    palmprintln(g);
}

void palmprintln(GlobalsType *g)
{
	UInt16	tempX = g->x, tempY = g->y;
	AdjAlignLine(g);

	if (g->horizIndent && g->xindent && (g->horizIndent->h > g->fontH) && (g->horizIndent->align == Left)/* && (g->Align == Left)*/) {
		UInt32	imgStop = g->imgY + g->horizIndent->h;

		g->y += (g->fontH > g->imgH)?g->fontH:g->imgH;
		g->x = g->xindent + (g->indent * BrowserIndentValue()) + 1; // g->positionIndent;

		if ((g->y + g->fontH) > imgStop) {
			//g->imgH = 0;
			//g->xindent = 0;
			removeLastIndent(g);
		}

		if ((g->DrawHeight <= browserGetWidgetBounds(WIDGET_HEIGHT)) && (g->xhtml.background && (g->xhtml.background->noRefresh != true)))
			xhtml_ReSetWindow(g);

		g->imgH = 0;
	} else {
		if (g->imgH) {
			g->y += g->imgH;
			g->imgH = 0;
    		} else {
    			g->y += BrowserLineheightValue();
		}

		g->x = g->xindent + (g->indent * BrowserIndentValue()) + g->positionIndent;

		g->xindent = 0;

		if ((g->DrawHeight <= browserGetWidgetBounds(WIDGET_HEIGHT)) && (g->xhtml.background && (g->xhtml.background->noRefresh != true)))
			xhtml_ReSetWindow(g);
	}

    	/*if (g->imgH) {
		g->y += g->imgH;
		g->imgH = 0;
    	} else
    		g->y+=BrowserLineheightValue();*/

    	g->fontH = FontCharHeight();


	if (g->HyperLinks && 
	   (g->HyperLinks->x1 == tempX) && ((g->HyperLinks->y1) == tempY)) { // Hyperlink will start at end of last line
		g->HyperLinks->x1 = g->x;
		g->HyperLinks->y1 = g->y;
	}

    	g->browserGlobals.imgYAdd = true;
}

void scrollUpdate(GlobalsType *g)
{
	RectangleType	rect;

	if (g->y < browserGetWidgetBounds(WIDGET_HEIGHT)) {
		rect.topLeft.x = browserGetWidgetBounds(WIDGET_TOPLEFT_X) / HiresFactor();
		rect.topLeft.y = g->y + browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor();
		rect.extent.x = browserGetWidgetBounds(WIDGET_WIDTH) / HiresFactor();
		rect.extent.y = browserGetWidgetBounds(WIDGET_HEIGHT) / HiresFactor() - g->y;

		WinEraseRectangle(&rect, 0);
	}

	DOMRenderDOM(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget),
		     0 /*g->ScrollX*/, 0 /*g->ScrollY*/, g->ScrollX + browserGetWidgetBounds(WIDGET_WIDTH),
		     g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT),
		     /*((g->state != BS_IDLE) && (g->state != BS_ONTIMER) && (g->state != BS_XHTML_ONTIMER))?true:false*/ true, true, g);
	if (g->DrawHeight /*g->y*/ > browserGetWidgetBounds(WIDGET_HEIGHT))
		SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBar), g->ScrollY, 0, MAX(g->DrawHeight - browserGetWidgetBounds(WIDGET_HEIGHT) + (11 * HiresFactor()), 0), browserGetWidgetBounds(WIDGET_HEIGHT) - (11 * HiresFactor()));
	else
		SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBar),0,0,0,browserGetWidgetBounds(WIDGET_HEIGHT) - (11 * HiresFactor()));

	if (g->DrawWidth /*g->x*/ > browserGetWidgetBounds(WIDGET_WIDTH))
		SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal), g->ScrollX, 0, MAX(g->DrawWidth - browserGetWidgetBounds(WIDGET_WIDTH) + (11 * HiresFactor()), 0), browserGetWidgetBounds(WIDGET_WIDTH) - (11 * HiresFactor()));
	else
		SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal),0,0,0,browserGetWidgetBounds(WIDGET_WIDTH) - (11 * HiresFactor()));
}

Boolean scrollField(GlobalsType *g)
{
	if (g->InputFldActive)
		return true;

	return FldGetFocus(fldUrlBox);
}

void scrollUp(GlobalsType *g)
{
	if (scrollField(g))
		return;

	if (g_prefs.fiveWayMode == FIVEWAY_MODE_POINTER) {
		PointerScroll(&g->pointer, winUp, g);
	} else {
		BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), g->ScrollY - BrowserLineheightValue(), g);
	}
}

void scrollDown(GlobalsType *g)
{
	if (scrollField(g))
		return;

	if (g_prefs.fiveWayMode == FIVEWAY_MODE_POINTER) {
		PointerScroll(&g->pointer, winDown, g);
	} else {
		BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), g->ScrollY + BrowserLineheightValue(), g);
	}
}

void scrollLeft(GlobalsType *g)
{
	if (scrollField(g))
		return;

	if (g_prefs.fiveWayMode == FIVEWAY_MODE_POINTER) {
		PointerScroll(&g->pointer, winLeft, g);
	} else {
		BrowserScrollHorizontalTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), g->ScrollX - BrowserLineheightValue(), g);
	}
}

void scrollRight(GlobalsType *g)
{
	if (scrollField(g))
		return;

	if (g_prefs.fiveWayMode == FIVEWAY_MODE_POINTER) {
		PointerScroll(&g->pointer, winRight, g);
	} else {
		BrowserScrollHorizontalTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), g->ScrollX + BrowserLineheightValue(), g);
	}
}

static Boolean IsWhitespace(Char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '-';
}

Char *ConvertFromTo(CharEncodingType srcEncoding, Char *sourceStr, Char *destStr)
{
	Err 	err;
	UInt16 	ioDstBytes = StrLen(sourceStr);
	UInt16 	ioSrcBytes = StrLen(sourceStr);
	UInt32 	encoding;

    	FtrGet(sysFtrCreator, sysFtrNumEncoding, &encoding); // or charEncodingPalmLatin???

	err = TxtConvertEncoding (true, NULL, sourceStr, &ioSrcBytes, 
		srcEncoding, destStr, &ioDstBytes, (CharEncodingType)encoding, "?", 1);

	if (ioDstBytes < ioSrcBytes) destStr[ioDstBytes] = 0; // string was transformed and will be shorter...

	return (err == errNone) ? destStr : sourceStr; //if sourceStr returned than conversion failed
}

void palmprintf(char *instr, GlobalsType *g)
{
    Int16 length, width;
    Boolean fits;
    Char *str = instr;
    Boolean italic = (g->font)?g->font->italic:false;

    if ((g->webData.data != NULL) && (g->charSet != 0)) { // ((g->charSet != 0) || (g->wmldck.charSet != 0))) {
    	switch (g->charSet) { // switch ((g->wmldck.charSet) ? g->wmldck.charSet:g->charSet) {
		case UTF_8:
			str = ConvertFromTo(charEncodingUTF8, instr, str);
			break;
		/*case US_ASCII:
			str = ConvertFromTo(charEncodingAscii, instr, str);
			break;
		case ISO_8859_1:
			str = ConvertFromTo(charEncodingISO8859_1, instr, str);
			break;*/
		case USC_2:
			str = ConvertFromTo(charEncodingUCS2, instr, str);
			break;
		default:
			str = ConvertFromTo(charEncodingPalmLatin, instr, str);
			break;
    	}
    }


    if((g->x > 0) && g->needSpace) {
	g->x = g->x + (4 * HiresFactor());
    }
    g->needSpace = 0;

    if (g->x == 0)
    {
        while(IsWhitespace(*str)) str++;
        if(*str == '\0') return;
    }


    width = (browserGetWidgetBounds(DRAW_WIDTH) - 1) - g->x - (g->rightIndent * BrowserIndentValue());

    length = StrLen(str);

    FontCharsInWidth(str, &width, &length, &fits);
    while(!fits)
    {
        length = FontWordWrap(str, width);

        // check if there is a space-char so we we can wrap
        if(length > 0 && IsWhitespace(*(str + length - 1)))
        {
            // there is a space-char at position 'length'
	    if (italic == true) {
		TextAddText(str, length, g->x, g->y, italicGetWidth(str, length), FontCharHeight(), g);
		g->x += italicGetWidth(str, length);
	    } else {
		TextAddText(str, length, g->x, g->y, FontCharsWidth(str, length), FontCharHeight(), g);
	    	g->x += FontCharsWidth(str, length);
	    }
	    browserResizeDimensions(0, getFontHeight(g), g);
            palmprintln(g);

            str += length;
        }
        else
        {
            Char *ptr = str;
            while(IsWhitespace(*ptr)) ptr++;

            // no space-character in string, try if newline would help
            width  = browserGetWidgetBounds(DRAW_WIDTH) - 1;
            length = StrLen(ptr);

            FontCharsInWidth(ptr, &width, &length, &fits);
            if(!fits)
            {
                length = FontWordWrap(ptr, width);

                // the string still does not fit after a newline, check again for space-char
                if(length > 0 && IsWhitespace(*(ptr + length - 1)))
                {
                    // there is a space-char at position 'length'
                    palmprintln(g);
           	    //palmDrawChars(str, length, g->x, g->y, g);
		    if (italic == true) {
			TextAddText(str, length, g->x, g->y, FontCharsWidth(str, length), FontCharHeight(), g);
			g->x += italicGetWidth(str, length);
	    	    } else {
			TextAddText(str, length, g->x, g->y, FontCharsWidth(str, length), FontCharHeight(), g);
		    	g->x += FontCharsWidth(str, length);
		    }
		    browserResizeDimensions(0, getFontHeight(g), g);
                    palmprintln(g);
                    
                    str = ptr + length;
                }
                else
                {
                    // we'll need to cut ...
		    width = (browserGetWidgetBounds(DRAW_WIDTH)-1) - g->x - (g->rightIndent * BrowserIndentValue());
                    length = StrLen(str);

                    FontCharsInWidth(str, &width, &length, &fits);
                    
		    if (italic == true) {
			TextAddText(str, length, g->x, g->y, italicGetWidth(str, length), FontCharHeight(), g);
			g->x += italicGetWidth(str, length);
	    	    } else {
			TextAddText(str, length, g->x, g->y, FontCharsWidth(str, length), FontCharHeight(), g);
	    	    	g->x += FontCharsWidth(str, length);
		    }
		    browserResizeDimensions(0, getFontHeight(g), g);
                    palmprintln(g);

                    str += length;                        
                }
            }
            else
            {
                // the complete string fits after a newline. 
                palmprintln(g);
		if (italic == true) {
			TextAddText(str, length, g->x, g->y, italicGetWidth(str, length), FontCharHeight(), g);
			g->x += italicGetWidth(str, length);
	    	} else {
			TextAddText(str, length, g->x, g->y, FontCharsWidth(str, length), FontCharHeight(), g);
	       		 g->x += FontCharsWidth(str, length);
		}
                browserResizeDimensions(0, getFontHeight(g), g);
                return;
            }
        }
        
	width = (browserGetWidgetBounds(DRAW_WIDTH) - 1) - g->x - (g->rightIndent * BrowserIndentValue());
        length = StrLen(str);

        FontCharsInWidth(str, &width, &length, &fits);                
    }

    if (italic == true) {
	TextAddText(str, length, g->x, g->y, italicGetWidth(str, length), FontCharHeight(), g);
	g->x += italicGetWidth(str, length);
    } else {
	TextAddText(str, length, g->x, g->y, FontCharsWidth(str, length), FontCharHeight(), g);
    	g->x += FontCharsWidth(str, length);
    }
    browserResizeDimensions(0, getFontHeight(g), g);

    if(IsWhitespace(*(str + length))) g->x += FontCharWidth(' '); // laf fix
}

void palmsrcprintf(char *str, GlobalsType *g)
{
    Char * srcPtr;
    UInt32   sz;

    sz = MemHandleSize(g->srcHandle);
    srcPtr = MemHandleLock(g->srcHandle);
    if ( sz <= (StrLen(srcPtr)+StrLen(str))) { // We need to resize
        MemHandleUnlock(g->srcHandle);
        if (MemHandleResize(g->srcHandle,sz+StrLen(str)) == 0) {            
			sz+=StrLen(str);
            srcPtr = MemHandleLock(g->srcHandle);
        } else {
            DisplayError("Resize Failed");
            return; 
        }
    }
    StrCat(srcPtr,str);

    MemHandleUnlock(g->srcHandle);

}

void showProgress(GlobalsType *g)
{
    BitmapPtr resP = 0;

    if (g->progress < 0)
	g->progress = 0;

    // Slow things down a bit, and even it out...
    if (TimGetTicks() < g->browserGlobals.nextTick && g->progress != 0) return;

    resP = MemHandleLock(g->resBmpPrgrs[g->progress]);
    TabDrawProgress(resP, g);
    MemPtrUnlock(resP);
    g->progress++;
    if (g->progress > 12)
	g->progress = 1;//g->progress = ((g->progress)+1) % 14; // 4;

    g->browserGlobals.nextTick = TimGetTicks() + 2; // (SysTicksPerSecond() / 8);
}

void browserResetURL(GlobalsType *g)
{
	Char *tmpUrl;

	tmpUrl = historyPop(g);

	if (!tmpUrl)
		return;

	if (g->Url != NULL) Free(g->Url);

	g->Url = Malloc(MAX_URL_LEN+1);
	ErrFatalDisplayIf(!g->Url, "Malloc failed");

	StrCopy(g->Url, tmpUrl);
	StrCopy(g->WapLastUrl, tmpUrl);

	SetFieldFromStr(g->Url, fldUrlBox);
	FldReleaseFocus(GetObjectPtr(fldUrlBox));

	return;
}

void browserPushPageContent(GlobalsType *g)
{
	if (!g->webData.data)
		return;

	if (g->pushedPageContent)
		Free(g->pushedPageContent);

	g->pushedPageContent = Malloc(g->webData.length);
	if (!g->pushedPageContent)
		return;

	MemSet(g->pushedPageContent, g->webData.length, 0);
	MemMove(g->pushedPageContent, g->webData.data, g->webData.length);

	g->pushedPageLength = g->webData.length;

	if (g->pageContentType) {
		if (g->pushedPageContentType)
			Free(g->pushedPageContentType);

		g->pushedPageContentType = StrDup(g->pageContentType);
	}

	if (g->deckTitle) {
		if (g->pushedPageTitle)
			Free(g->pushedPageTitle);

		g->pushedPageTitle = StrDup(g->deckTitle);
	}

	g->pageStats.savedtimePageLoadStart	= g->pageStats.timePageLoadStart;
	g->pageStats.savedtimePageLoadFinish	= g->pageStats.timePageLoadFinish;
	g->pageStats.savedPageSize		= g->pageStats.pageSize;
}

void browserPopPageContent(GlobalsType *g)
{
	if (!g->pushedPageContent)
		return;

	if (g->webData.data)
		Free(g->webData.data);

	g->webData.data = Malloc(g->pushedPageLength);
	if (!g->webData.data)
		return;

	MemSet(g->webData.data, g->pushedPageLength, 0);
	MemMove(g->webData.data, g->pushedPageContent, g->pushedPageLength);

	g->webData.length = g->pushedPageLength;

	if (g->pushedPageContentType) {
		if (g->pageContentType)
			Free(g->pageContentType);

		g->pageContentType = StrDup(g->pushedPageContentType);
	}

	if (g->pushedPageTitle) {
		if (g->deckTitle)
			Free(g->deckTitle);
		
		g->deckTitle = StrDup(g->pushedPageTitle);
	}

	g->pageStats.timePageLoadStart	= g->pageStats.savedtimePageLoadStart;
	g->pageStats.timePageLoadFinish	= g->pageStats.savedtimePageLoadFinish;
	g->pageStats.pageSize		= g->pageStats.savedPageSize;
}

static void frmOpenEvent_OH(GlobalsType *g)
{
    	Err             error;
    	FormPtr         frm;
    

	g->resBmpPrgrs[0]=DmGetResource(bitmapRsc, bmpProgress0);
    	g->resBmpPrgrs[1]=DmGetResource(bitmapRsc, bmpProgress1);
    	g->resBmpPrgrs[2]=DmGetResource(bitmapRsc, bmpProgress2);
    	g->resBmpPrgrs[3]=DmGetResource(bitmapRsc, bmpProgress3);
    	g->resBmpPrgrs[4]=DmGetResource(bitmapRsc, bmpProgress4);
    	g->resBmpPrgrs[5]=DmGetResource(bitmapRsc, bmpProgress5);
    	g->resBmpPrgrs[6]=DmGetResource(bitmapRsc, bmpProgress6);
    	g->resBmpPrgrs[7]=DmGetResource(bitmapRsc, bmpProgress7);
    	g->resBmpPrgrs[8]=DmGetResource(bitmapRsc, bmpProgress8);
    	g->resBmpPrgrs[9]=DmGetResource(bitmapRsc, bmpProgress9);
    	g->resBmpPrgrs[10]=DmGetResource(bitmapRsc, bmpProgress10);
    	g->resBmpPrgrs[11]=DmGetResource(bitmapRsc, bmpProgress11);
    	g->resBmpPrgrs[12]=DmGetResource(bitmapRsc, bmpProgress12);
    	g->progress=0;
    	g->srcHandle=0;
	g->sock = -1;
    	g->inBuf = NULL;
    	g->HyperLinks = NULL;
    	g->Var = NULL;
    	g->segment = NULL;
    	g->Input = NULL;
    	g->Select = NULL;
	g->radioBtn = NULL;
	g->checkBox = NULL;
	g->formButton = NULL;
	g->Forms = NULL;
    	g->Url = NULL;
    	g->OpenUrl = NULL;
    	g->deckTitle = StrDup("Universe");
    	g->AuthUser = NULL;
    	g->AuthPass = NULL;
    	g->InputFldActive=NULL;

	FldSetFont(GetObjectPtr(fldUrlBox), stdFontFixed);

    	FrmDrawForm(FrmGetActiveForm());
	RSSShowIndicator(false);

	APIInitialiseBrowserControls(gadgetBrowserWidget, fldInput, listOption, scrollBarBrowser, scrollBarHorizontal, g);

	BrowserInitialiseGlobals();

	PointerInitialise(&g->pointer, g);

    	g->DrawHeight = 0; // browserGetWidgetBounds(WIDGET_HEIGHT);
	g->DrawWidth = browserGetWidgetBounds(WIDGET_WIDTH);
	g->BrowserAPI.lastWidgetWidth = browserGetWidgetBounds(WIDGET_WIDTH);
    	g->ScrollY = 0;
	g->ScrollX = 0;
    	SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBar),0,0,0,browserGetWidgetBounds(WIDGET_HEIGHT) - (11 * HiresFactor()));
	SclSetScrollBar(GetObjectPtr(g->BrowserAPI.BrowserScrollBarHorizontal),0,0,0,browserGetWidgetBounds(WIDGET_WIDTH) - (11 * HiresFactor()));
    	g->x=0;g->y=0;
    	g->state = BS_IDLE;
    	g->webData.data= NULL;
    	g->Align = Left;
	g->imgAlign = None;
    	g->onenterback = false;
    	g->refreshEvent = false;
    	g->hasEvent = false;
    	g->postBuf = NULL;
    	g->wtpState = WTP_Disconnected;
	g->offline.mtchDb = NULL;
	g->offline.record = 0;
	g->charSet = 0;
	g->contentLength = 0;
	g->inlineImage = NULL;
	g->redirects = 0;
	g->xhtml.background = NULL;
	g->browserGlobals.browserTitle = NULL;

    	g->isImage = false;
    	g->imgH = 0;
	g->imgY = 0;
	g->pageImages = NULL;
	
	g->indent = 0;
	g->rightIndent = 0;
	g->xindent = 0;
	g->fontH = FontCharHeight();
	g->font = NULL;

	g->partBound = NULL;

	g->rss = NULL;

	g->engineStop = false;
	g->userCancel = false;
	g->httpUseTempFile = false;

	g->cache.haveCacheFile = false;
	g->cache.cacheDoNotRead = false;
	g->cache.tryCache = true;

	g->horizIndent = NULL;
	g->position = NULL;
	g->positionIndent = 0;

	g->text = NULL;
	g->DOM = NULL;

	g->pushedPageContent = NULL;
	g->pushedPageLength = 0;
	g->pushedPageContentType = NULL;

	MemSet(&g->pageStats, sizeof(PageStatsType), 0);

	TabInitializeTabs(NULL, true, g);
	//TabDrawTabs(false, g);

	if (g_prefs.mode == true) browserSwitchMode(g_prefs.mode);

	PointerUpdatePointer(&g->pointer, g);
    
	if (g->launchURL != NULL) {
		dbConnGetConnection(g->ActiveConnection,&(g->conn),g);

    		g->Url=Malloc(StrLen(g->launchURL)+1);
    			ErrFatalDisplayIf (!g->Url, "Malloc gUrl");
		//StrCopy(g->Url,g->launchURL);
		URLCreateURLStr(g->launchURL, g->Url, g);

		StrCopy(g->DbUrlCurrentURL.name, "NULL");
		StrCopy(g->DbUrlCurrentURL.connection, g->ActiveConnection);
		StrCopy(g->DbUrlCurrentURL.urlstr, g->Url);

		Free(g->launchURL);
		g->launchURL = NULL;
	} else {
    		dbUrlReadCurrentUrl(g);

    		if (StrNCaselessCompare(g->DbUrlCurrentURL.connection, "- Use Active -", 14)==0)
			dbConnGetConnection(g->ActiveConnection,&(g->conn),g);
   		else
    			dbConnGetConnection(g->DbUrlCurrentURL.connection,&(g->conn),g);

    		g->Url=Malloc(StrLen(g->DbUrlCurrentURL.urlstr)+1);
    			ErrFatalDisplayIf (!g->Url, "Malloc gUrl");
		//StrCopy(g->Url,g->DbUrlCurrentURL.urlstr);
		URLCreateURLStr(g->DbUrlCurrentURL.urlstr, g->Url, g);
	}

	historyPush(g->Url,g);
	frm = FrmGetActiveForm();

	if (StrNCaselessCompare(g->Url,"cache:",6)==0) {
		g->state = BS_GET_OFFLINE;
	} else if (StrNCaselessCompare(g->Url,"mtch:",5)==0) {
		g->state = BS_GET_OFFLINE;
	} else if (StrNCaselessCompare(g->Url,"file:",5)==0) {
		g->state = BS_GET_LOCALFILE;
	} else if (StrNCaselessCompare(g->Url, "internal:",9)==0) {
		browserLoadInternalPage(g);
	} else if (StrNCaselessCompare(g->Url, "about:",6)==0) {
		browserLoadInternalPage(g);
	} else {
	    	browserSetTitle("Connecting"); // SetFieldFromStr("Connecting",fieldTitle);
	    	error = NetOpenNetwork(g);
	    	if (error == 0) {
		    	g->state = BS_GET_URL;
	    	} else {
			browserSetTitle((g->deckTitle)?g->deckTitle:"WAPUniverse");
		}
	}

}


Boolean browserReloadEvt(GlobalsType *g)
{
 	Err	error;

	ImageLoadStop(g);
  
	if (StrNCaselessCompare(g->Url,"internal:",9) &&
	    StrNCaselessCompare(g->Url,"about:",6)) {
		if (!StrNCaselessCompare(g->Url,"mtch:",5)) {
			g->state = BS_GET_OFFLINE;
		} else if (!StrNCaselessCompare(g->Url,"file:",5)) {
			g->state = BS_GET_LOCALFILE;
		} else {
			error = NetOpenNetwork(g);
			if (!error) {
				g->retransmission = false;
				g->state = BS_GET_URL;
			} else if (CacheFindCachedFile(g->Url, false, g)) {
				g->state = BS_CHECK_CACHE;
			} else {
				if (!g->onenterback)
					browserResetURL(g);
				g->state = BS_IDLE;
			}
		}
	} else {
		browserLoadInternalPage(g);
	}
	return(true);   
}

Boolean browserReloadButtonEvt(GlobalsType *g)//this is for the refresh button
{
	Err	error;
    
    	g->post = false; // if we don't do this things get messy

	g->cache.cacheDoNotRead = true;

	ImageLoadStop(g);
	
	if (StrNCaselessCompare(g->Url,"internal:",9) &&
	    StrNCaselessCompare(g->Url,"about:",6)) {
		if (!StrNCaselessCompare(g->Url,"mtch:",5)) {
			g->state = BS_GET_OFFLINE;
		} else if (!StrNCaselessCompare(g->Url,"file:",5)) {
			g->state = BS_GET_LOCALFILE;
		} else {
			error = NetOpenNetwork(g);
			if (error == 0) {
				g->retransmission = false;
				g->state = BS_GET_URL;
			} else {
				//browserResetURL(g);
				g->state = BS_IDLE;
			}
		}
	} else {
		browserLoadInternalPage(g);
	}
	return(true);   
}


Boolean browserSrcEvt(GlobalsType *g)
{
    WMLDeck         wmldck;

    if (!g->webData.data) {
        MyErrorFunc ("No page loaded", "View Source");
        return(true);
    }
    if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlscriptc",30)==0) {
	MyErrorFunc ("Not a valid web page.", "View Source");
	return(true);
    }
    if (g->webData.length > 64000) {
	MyErrorFunc("Unable to display source. The page exceeds the 64 kilobyte limit for text fields.","View Source");
	return true;
    }
    g->srcHandle = MemHandleNew(20);
    if (g->srcHandle<=0) {
        MyErrorFunc ("Failed to allocate memory.", "View Source");
        return(true);
    }
    MemSet(MemHandleLock(g->srcHandle),20,0);
    MemHandleUnlock(g->srcHandle);

    if (StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlc",24)==0) {
	wmldck.card=NULL;
    	wbxml_parse(&(g->webData), ACTION_SRC, &wmldck,0,g );
    	wbxml_free_wmldck(&wmldck);
	g->progress = 0;
	showProgress(g);
    } else if (((StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29)==0) || 
	       (StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21)==0) || 
	       (StrNCaselessCompare(g->pageContentType,"application/wml+xml",19)==0) ||
	       (StrNCaselessCompare(g->pageContentType,"text/html",9)==0) ||
	       (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)==0) ||
	       (StrNCaselessCompare(g->pageContentType,"application/rss+xml",19)==0))) {
			palmsrcprintf(g->webData.data, g);
    } else {
	goto src_bottom;
    }
    ScrollFrm("Page Source",g->srcHandle);
src_bottom:
    MemHandleFree(g->srcHandle);
    g->srcHandle=0;
    return(true);  
}

void browserStateError(GlobalsType *g)
{
	browserSetTitle((g->deckTitle)?g->deckTitle:"Universe"); // SetFieldFromStr("WAPUniverse",fieldTitle);
	browserResetURL(g);
	browserPopPageContent(g);
	g->state = BS_IDLE;
	g->progress = 0;
	showProgress(g);
	g->cache.cacheDoNotRead = false;
}

void browserWmlsStateError(GlobalsType *g)
{
	browserSetTitle((g->deckTitle)?g->deckTitle:"Universe"); // SetFieldFromStr("WAPUniverse",fieldTitle);
	//browserResetURL(g);
	//browserPopPageContent(g);
	g->state = BS_IDLE;
	g->progress = 0;
	showProgress(g);
	g->cache.cacheDoNotRead = false;
}


Boolean runBrowserState(GlobalsType *g)
{
    	int 			ret;
    	static UInt32		maxTimer;
    	static UInt32    	len;
    	//WinHandle       	prevDrawWin;
    	static RectangleType 	r;
    	Char			*tmpPtr;
    	FormPtr         	frm;
    	int 			j = 0, i = 0;
   	Err 			error;
    	int 			stop;//, stop2;
    	int 			wtpFail = 0;
	UInt16 			ctP, indexP;
	URLTarget 		*url;
	PostReq 		*postReq;
	int			http_hand = 0;
	HTTPErr			http_err = 0;
	int			xhtmlErr = 0;

	r.topLeft.x = browserGetWidgetBounds(WIDGET_TOPLEFT_X) / HiresFactor();
	r.topLeft.y = browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor();
	r.extent.x = browserGetWidgetBounds(WIDGET_WIDTH) / HiresFactor();
	r.extent.y = browserGetWidgetBounds(WIDGET_HEIGHT) / HiresFactor();
	
	frm = FrmGetActiveForm();

	browserSwitchSoftTwo(g);

    	if (g->state==BS_IDLE) {
		g->isImage = false;

		if (g->browserGlobals.tapandhold) {
			if ((TimGetTicks() >= (g->browserGlobals.taptick + (TAPTIMEOUT)))
			    && (g->browserGlobals.taptype == TAPIMAGE)) {
				checkPageImage(g->browserGlobals.tapx, g->browserGlobals.tapy, true, g);
				g->browserGlobals.tapandhold = false;
				g->browserGlobals.taptick = 0;
				g->browserGlobals.tapx = 0; g->browserGlobals.tapy = 0;
			} else if ((TimGetTicks() >= (g->browserGlobals.taptick + (TAPTIMEOUT)))
				   && (g->browserGlobals.taptype == TAPLINK)) {
				HyperlinkHandlePenHold(g->browserGlobals.tapx, g->browserGlobals.tapy, g);
				g->browserGlobals.tapandhold = false;
				g->browserGlobals.taptick = 0;
				g->browserGlobals.tapx = 0; g->browserGlobals.tapy = 0;
			}
		}

        	return(true);
    	}

    	// Update progres indicator
    	showProgress(g);
    	switch (g->state) {
    		case BS_GET_OFFLINE:
			g->isImage = false;

			browserPushPageContent(g);

			SetFieldFromStr(g->Url, fldUrlBox);
			FldReleaseFocus(GetObjectPtr(fldUrlBox));

			g->pageStats.timePageLoadStart = TimGetTicks();

			browserSetTitle("Decoding Data"); // SetFieldFromStr("Decoding Data",fieldTitle);
			error = ChannelGetIndex(g->Url, false, &indexP, &ctP, g);
			if (!error) {
       				ret = wsp_getOffline(indexP, &(g->webData), true, g);
				if (ret == 0) {
					if (!g->contentType) {
							browserStateError(g);
							
							break;
					}

					if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlscriptc",30)==0) {
						browserWmlsStateError(g);
						callWMLScriptFunc(g->Url);
						//browserStateError(g);
	    				} else if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlc",24)==0) {
            					g->state = BS_WBXML_PARSE;

						g->pageStats.pageSize = g->webData.length;
					} else if ((StrNCaselessCompare(g->contentType,"application/vnd.wap.xhtml+xml",29)==0) || 
					 	   (StrNCaselessCompare(g->contentType,"application/xhtml+xml",21)==0) || 
					 	   (StrNCaselessCompare(g->contentType,"application/wml+xml",19)==0) ||
					 	   (StrNCaselessCompare(g->contentType,"text/html",9)==0) ||
					 	   (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)==0)) {
            						g->state = BS_XHTML_PARSE_RENDER;

							g->pageStats.pageSize = g->webData.length;
            				} else if (StrNCaselessCompare(g->contentType,"application/rss+xml",19)==0) {
            					g->state = BS_RSS_PARSE_RENDER;
					} else {
						browserSetTitle((g->deckTitle)?g->deckTitle:"Universe"); // SetFieldFromStr("WAPUniverse",fieldTitle);

						for (i=0; i < StrLen(g->Url); i++) {
							if (g->Url[i] == '/'){
								j = i;
							}
    						}
    						j++;
						switch (DownloadAlert (g->Url+j, g->contentType)) {
							case 0: // Download to internal
								error = DownloadFile(g->Url+j, &g->webData, g);
								break;
							case 1: // Cancel
								break;
							case 2: // Download to card (coming soon)
								{
								Char *entryStringBuf = Malloc(255);
								if (!entryStringBuf) break;
								StrCopy(entryStringBuf, g->Url+j);
								FrmCustomResponseAlert(alVfsDownload, g_prefs.downloadTo, NULL, NULL, entryStringBuf, 255, FormHandleVfsDownload);
								if (entryStringBuf) Free(entryStringBuf);
								}
								break;
							default:
								break;
						}
						browserResetURL(g);
						browserPopPageContent(g);
	        				g->state = BS_IDLE;
	        				g->progress= 0;
	        				showProgress(g);
	   				}
				} else {
					ChannelClose(g);
					browserStateError(g);
				}
			} else {
				browserStateError(g);
				MyErrorFunc("The requested channel or page could not be found on this device. If you came from a bookmark, please make sure you entered the URL correctly.", NULL);
			}
        		break;
   		case BS_GET_LOCALFILE:
			g->isImage = false;

			browserPushPageContent(g);

			SetFieldFromStr(g->Url, fldUrlBox);
			FldReleaseFocus(GetObjectPtr(fldUrlBox));

			g->pageStats.timePageLoadStart = TimGetTicks();

			browserSetTitle("Decoding Data"); // SetFieldFromStr("Decoding Data",fieldTitle);
			if (FileURIExists(g->Url)) {
  				ret = FileURIOpen(g->Url, &g->webData, true, g);
				if (ret != 0) {
					MyErrorFunc("Could not open the requested file.", NULL);
					browserStateError(g);
				} else {
					if (!g->contentType) {
							browserStateError(g);
							
							break;
					}

					if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlscriptc",30)==0) {
						browserWmlsStateError(g);
						callWMLScriptFunc(g->Url);
						//browserStateError(g);
	    				} else if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlc",24)==0) {
            					g->state = BS_WBXML_PARSE;

						g->pageStats.pageSize = g->webData.length;
					} else if ((StrNCaselessCompare(g->contentType,"application/vnd.wap.xhtml+xml",29)==0) || 
					 	   (StrNCaselessCompare(g->contentType,"application/xhtml+xml",21)==0) || 
					 	   (StrNCaselessCompare(g->contentType,"application/wml+xml",19)==0) ||
					 	   (StrNCaselessCompare(g->contentType,"text/html",9)==0) ||
					 	   (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)==0)) {
            						g->state = BS_XHTML_PARSE_RENDER;

							g->pageStats.pageSize = g->webData.length;
            				} else if (StrNCaselessCompare(g->contentType,"application/rss+xml",19)==0) {
            					g->state = BS_RSS_PARSE_RENDER;
					} else {
						MyErrorFunc("Browser cannot handle the the requested local file.", NULL);

						browserResetURL(g);
						browserPopPageContent(g);
	        				g->state = BS_IDLE;
	        				g->progress= 0;
	        				showProgress(g);
	   				}
				}
			} else {
				browserStateError(g);
				MyErrorFunc("The requested file could not be found at the location specified or the location is a directory.", NULL);
			}
        		break;
    		case BS_WTP_CONNECT:
			browserSetTitle("Connecting"); // SetFieldFromStr("Connecting",fieldTitle);
			g->isImage = false;
			g->sock = wapConnect(&(g->conn),g->Url,true,wtpFail,g);
			if ((g->sock == -1) || (g->wtpState == WTP_Disconnected)) {
				NetCloseSocket(&g->sock);
				browserStateError(g);
				g->wtpState = WTP_Disconnected;
				break;
			}
			g->state = BS_WTP_GETCONNECT;
			break;
    		case BS_WTP_GETCONNECT:
			g->isImage = false;
			wtpFail = WTPHandleConnectResp(g->sock, g->conn.ipaddress, g);
			if (wtpFail == -1) {
				NetCloseSocket(&g->sock);
				g->retransmission = false;
				browserStateError(g);
				g->wtpState = WTP_Disconnected;
				break;
			} else if (wtpFail == 0) {
				g->retransmission = true;
				g->state = BS_WTP_CONNECT;
				break;
			} else {
				g->retransmission = false;
				g->state = BS_GET_URL;
			}
			break;
		case BS_CHECK_CACHE:
			g->isImage = false;

			SetFieldFromStr(g->Url, fldUrlBox);
			FldReleaseFocus(GetObjectPtr(fldUrlBox));

			//browserSetTitle("Decoding Data"); // SetFieldFromStr("Decoding Data",fieldTitle);
			if (CacheLoadFileFromCache(&(g->webData), g->Url, true, true, g) == true) {
				if (!g->contentType) {
					browserStateError(g);
					
					break;
				}

				if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlscriptc",30)==0) {
					browserWmlsStateError(g);
					callWMLScriptFunc(g->Url);
					browserStateError(g);
	    			} else if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlc",24)==0) {
            				g->state = BS_WBXML_PARSE;

					g->pageStats.pageSize = g->webData.length;
				} else if ((StrNCaselessCompare(g->contentType,"application/vnd.wap.xhtml+xml",29)==0) || 
				 	   (StrNCaselessCompare(g->contentType,"application/xhtml+xml",21)==0) || 
				 	   (StrNCaselessCompare(g->contentType,"application/wml+xml",19)==0) ||
				 	   (StrNCaselessCompare(g->contentType,"text/html",9)==0) ||
				 	   (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)==0)) {
            					g->state = BS_XHTML_PARSE_RENDER;

						g->pageStats.pageSize = g->webData.length;
            			} else if (StrNCaselessCompare(g->contentType,"application/rss+xml",19)==0) {
            				g->state = BS_RSS_PARSE_RENDER;
				} else {
					browserSetTitle((g->deckTitle)?g->deckTitle:"Universe"); // SetFieldFromStr("WAPUniverse",fieldTitle);

					for (i=0; i < StrLen(g->Url); i++) {
						if (g->Url[i] == '/'){
							j = i;
						}
    					}
    					j++;
					switch (DownloadAlert (g->Url+j, g->contentType)) {
						case 0: // Download to internal
							error = DownloadFile(g->Url+j, &g->webData, g);
							break;
						case 1: // Cancel
							break;
						case 2: // Download to card (coming soon)
							{
							Char *entryStringBuf = Malloc(255);
							if (!entryStringBuf) break;
							StrCopy(entryStringBuf, g->Url+j);
							FrmCustomResponseAlert(alVfsDownload, g_prefs.downloadTo, NULL, NULL, entryStringBuf, 255, FormHandleVfsDownload);
							if (entryStringBuf) Free(entryStringBuf);
							}
							break;
						default:
							break;
					}
					browserResetURL(g);
					browserPopPageContent(g);
	        			g->state = BS_IDLE;
	        			g->progress= 0;
	        			showProgress(g);
	   			}

				break;
			} else {
				//browserSetTitle((g->deckTitle)?g->deckTitle:"WAPUniverse"); // SetFieldFromStr("WAPUniverse",fieldTitle);
				g->state = BS_IDLE;
				g->progress = 0;
				showProgress(g);
			}
			g->state = BS_GET_URL;
			g->cache.tryCache = false;
			break;
    		case BS_GET_URL:
			g->isImage = false;
			g->userCancel = false;
			g->browserGlobals.retransCount = 0;

			browserPushPageContent(g);

			g->pageStats.timePageLoadStart = TimGetTicks();

			SetFieldFromStr(g->Url, fldUrlBox);
			FldReleaseFocus(GetObjectPtr(fldUrlBox));

			if (g->cache.tryCache) {
				g->state = BS_CHECK_CACHE;
				break;
			}

			g->cache.tryCache = true;

			dbConnGetConnection(g->DbUrlCurrentURL.connection,&(g->conn),g);
			if ((g->wtpState == WTP_Disconnected) && (g->conn.connectionType == 'O')) {
				g->state = BS_WTP_CONNECT;
				break;
			} else if ((g->conn.connectionType == 'h') || (g->conn.connectionType == 'd')) {
				g->state = BS_HTTP_GET;
				break;
			}
			g->retransmission = false;
			browserSetTitle("Requesting URL"); // SetFieldFromStr("Requesting URL",fieldTitle);
        		g->sock = wapGetUrl (&(g->conn),g->Url,true,&error,g);
			if (error) {
				NetCloseSocket(&g->sock);
				browserStateError(g);
				g->wtpState = WTP_Disconnected;
				break;
			}
			if ((g->wtpState == WTP_Disconnected) && (g->conn.connectionType == 'O')) {
				browserStateError(g);
				break;
			}
        		g->state = BS_GET_REPLY;
			browserSetTitle("Waiting For Reply"); // SetFieldFromStr("Waiting for reply",fieldTitle);
        		maxTimer = TimGetTicks() + (REPLY_TIMEOUT * SysTicksPerSecond());
        		break;
    		case BS_WTP_REGET_URL:
			g->isImage = false;
			g->retransmission = true;
			if (g->browserGlobals.retransCount >= 9) {
				WTPAbort(g->sock, g);
				browserStateError(g);
				MyErrorFunc("Could not receive data from server.", "No gateway response");
				break;
			}
			reWtpGet (g->sock,&(g->conn),g->Url,true,&error,g);
			if (error) {
				WTPAbort(g->sock, g);
				browserStateError(g);
				break;
			}
       			g->state = BS_GET_REPLY;
        		//maxTimer = TimGetTicks() + (REPLY_TIMEOUT * SysTicksPerSecond());
			break;
    		case BS_GET_REPLY:
			g->isImage = false;
			browserSetTitle("Receiving URL"); // SetFieldFromStr("Receiving URL",fieldTitle);

			if (!g->inBuf) {
				g->inBuf = Malloc(DEF_RECEIVE_BUF+1);   
				ErrFatalDisplayIf (!g->inBuf, "Malloc failed");
			}

        		len = WspGetReply(g->sock,g->inBuf,DEF_RECEIVE_BUF,maxTimer,g);
			if ((g->wtpState == WTP_Connected) && (g->conn.connectionType == 'O') && ((len == 0) || (len == -5))) {
				if (len == -5) g->browserGlobals.retransCount++;
				g->state = BS_WTP_REGET_URL;
			} else {
				if ((len <= 0) || (len == -10)) {
					browserStateError(g);
				} else {
        				g->state = BS_WSP_DECODE;
				}
				g->post = false;
    				if (g->postBuf != NULL){ Free(g->postBuf); g->postBuf = NULL; }
				if (g->AuthUser != NULL){ Free(g->AuthUser); g->AuthUser = NULL; }
      				if (g->AuthPass != NULL){ Free(g->AuthPass); g->AuthPass = NULL; }
				SARdeleteData(g);
			}
			g->retransmission = false;
			g->browserGlobals.retransCount = 0;
        		break;
    		case BS_WSP_DECODE:
			g->isImage = false;
			browserSetTitle("Decoding Data"); // SetFieldFromStr("Decoding Data",fieldTitle);
       			ret = WspDecode(g->inBuf,len, &(g->webData), g);
        		if (ret == 0) {

				if (!g->contentType) {
					browserStateError(g);
					
					break;
				}

            			if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlc",24)==0) {
					CacheAddFileToCache(&(g->webData), g->Url, false, g);
            				g->state = BS_WBXML_PARSE;

					g->pageStats.pageSize = g->webData.length;
            			} else if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlscriptc",30)==0) {
					CacheAddFileToCache(&(g->webData), g->Url, false, g);
					browserWmlsStateError(g);
					callWMLScriptFunc(g->Url);
					//browserStateError(g);
	    			} else if ((StrNCaselessCompare(g->contentType,"application/vnd.wap.xhtml+xml",29)==0) || 
					   (StrNCaselessCompare(g->contentType,"application/xhtml+xml",21)==0) || 
					   (StrNCaselessCompare(g->contentType,"application/wml+xml",19)==0) ||
					   (StrNCaselessCompare(g->contentType,"text/html",9)==0) ||
					   (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)==0)) {
					CacheAddFileToCache(&(g->webData), g->Url, false, g);
            				g->state = BS_XHTML_PARSE_RENDER;

					g->pageStats.pageSize = g->webData.length;
            			} else if (StrNCaselessCompare(g->contentType,"application/rss+xml",19)==0) {
            				g->state = BS_RSS_PARSE_RENDER;
				} else {
					browserSetTitle((g->deckTitle)?g->deckTitle:"WAPUniverse"); // SetFieldFromStr("WAPUniverse",fieldTitle);

					for (i=0; i < StrLen(g->Url); i++) {
						if (g->Url[i] == '/'){
							j = i;
						}
    					}
    					j++;
					switch (DownloadAlert (g->Url+j, g->contentType)) {
						case 0: // Download to internal
							error = DownloadFile(g->Url+j, &g->webData, g);
							break;
						case 1: // Cancel
							break;
						case 2: // Download to card (coming soon)
							{
							Char *entryStringBuf = Malloc(255);
							if (!entryStringBuf) break;
							StrCopy(entryStringBuf, g->Url+j);

							FrmCustomResponseAlert(alVfsDownload, g_prefs.downloadTo, NULL, NULL, entryStringBuf, 255, FormHandleVfsDownload);
							if (entryStringBuf) Free(entryStringBuf);
							}
							break;
						default:
							break;
					}
					browserResetURL(g);
					browserPopPageContent(g);
	        			g->state = BS_IDLE;
	        			g->progress= 0;
	        			showProgress(g);
	   			}
        		} else {
        			if (ret == WSP_ERR_REDIRECT) {
        				followLink(g->urlBuf,g);
        			} else if (ret == WSP_ERR_UNHANDELED) {
					browserStateError(g);
				} else if (ret == WSP_ERR_AUTHORIZE) {
        				followLink(g->Url, g);
        			}/* else if (ret == WSP_ERR_UNAUTH) {
        				browserStateError(g);
        			} */else {
					if (ret == WSP_ERR_MEM) MyErrorFunc("Not enough memory to complete request. Please free up some memory and try again", NULL);
					browserStateError(g);
    				}
        		}
			if (g->inBuf != NULL) {
        			Free(g->inBuf);
        			g->inBuf = NULL;
    			}
        		break;
		case BS_HTTP_GET:
    			g->charSet = 0;
			g->userCancel = false;
			g->browserGlobals.savedclength = 0;
			g->browserGlobals.savedrlength = 0;
			g->browserGlobals.updateMenu = true;
			g->browserGlobals.redrawMenuBar = true;

			browserSetTitle("Requesting URL"); // SetFieldFromStr("Requesting URL",fieldTitle);
			if (HttpLibRef == 0) {
				browserStateError(g);
				MyErrorFunc("Failed to retrieve requested URL", NULL);
				break;
			}

			DebugOutInternal("BrowserState", "BS_HTTP_GET", 0, __FILE__, __LINE__, g);
			
			url = ParseURL( g->Url );
			if ( url != NULL ) {
				if (g->post == true) {
                        		postReq = CreatePostReq();
                        		if ( postReq == NULL ) {
                            			browserStateError(g);
                                		MyErrorFunc("Failed to retrieve requested URL", NULL);
						DestroyURL( url );
						g->post = false;
						break;
                        		} else {
                           			SetReqContentType( postReq, "application/x-www-form-urlencoded" );
                            			SetReqContent( postReq, g->postBuf, StrLen( g->postBuf ), 0 );
                            			if ( (http_err = HTTPPost( url, g->Url, postReq, "Universe-HTTP-Cache", false, &g->httpParser )) != 0 ) {
							DebugOutInternal("BrowserState", "cleanup post", 0, __FILE__, __LINE__, g);
							HTTPCleanUp(&g->sock, &g->httpParser);
							g->state = BS_IDLE;
	        					g->progress= 0;
							g->post = false;
	        					showProgress(g);
							DebugOutInternal("BrowserState", "reset url", 0, __FILE__, __LINE__, g);
							browserSetTitle((g->deckTitle)?g->deckTitle:"WAPUniverse"); // SetFieldFromStr("WAPUniverse",fieldTitle);
							browserResetURL(g);
							DebugOutInternal("BrowserState", "reset content", 0, __FILE__, __LINE__, g);
							browserPopPageContent(g);
							DebugOutInternal("BrowserState", "report", 0, __FILE__, __LINE__, g);
							switch (http_err) {
								case HTTPErr_ConnectError:
									break;
								case HTTPErr_TempDBErr:
									break;
								case HTTPErr_NoMem:
									MyErrorFunc("Not enough memory to complete request. Please free up some memory and try again", NULL);
									break;
								case HTTPErr_SizeMismatch:
								default: {
									char httpString[50];
									StrPrintF(httpString, "PalmHTTP: %d", http_err);
                            						MyErrorFunc("Failed to retrieve requested URL", httpString);
									} break;
							}
							DebugOutInternal("BrowserState", "destroy post req", 0, __FILE__, __LINE__, g);
							DestroyPostReq( postReq );
							DebugOutInternal("BrowserState", "destroy url", 0, __FILE__, __LINE__, g);
							DestroyURL( url );
							DebugOutInternal("BrowserState", "done", 0, __FILE__, __LINE__, g);
							//HTTPLibStop();
							break;
                            			}
                            			DestroyPostReq( postReq );
						g->post = false;
					}
				} else {
					DebugOutInternal("BrowserState", "get", 0, __FILE__, __LINE__, g);
                        		if ( (http_err = HTTPGet( url, g->Url, "Universe-HTTP-Cache", false, &g->httpParser )) != 0 ) {
						DebugOutInternal("BrowserState", "get error", (Int32)http_err, __FILE__, __LINE__, g);
						HTTPCleanUp(&g->sock, &g->httpParser);
						browserStateError(g);
						/*{
							char httpString[50];
							StrPrintF(httpString, "PalmHTTP: %d", http_err);
                            				MyErrorFunc("Failed to retrieve requested URL", httpString);
						}*/
						switch (http_err) {
							case HTTPErr_ConnectError:
								break;
							case HTTPErr_TempDBErr:
								break;
							case HTTPErr_NoMem:
								MyErrorFunc("Not enough memory to complete request. Please free up some memory and try again", NULL);
								break;
							case HTTPErr_SizeMismatch:
							default: {
								char httpString[50];
								StrPrintF(httpString, "PalmHTTP: %d", http_err);
                            					MyErrorFunc("Failed to retrieve requested URL", httpString);
								} break;
						}
						DebugOutInternal("BrowserState", "destroy url", 0, __FILE__, __LINE__, g);
						DestroyURL( url );
						//HTTPLibStop();
						break;
                        		}
					DebugOutInternal("BrowserState", "Get sent", 0, __FILE__, __LINE__, g);
				}
                        	DestroyURL( url );
				DebugOutInternal("BrowserState", "url destroyed", 0, __FILE__, __LINE__, g);
                    	} else {
                            	browserStateError(g);
                                MyErrorFunc("Failed to retrieve requested URL", NULL);
				//HTTPLibStop();
				break;
			}

			if (g->AuthUser != NULL){ Free(g->AuthUser); g->AuthUser = NULL; }
      			if (g->AuthPass != NULL){ Free(g->AuthPass); g->AuthPass = NULL; }

			DebugOutInternal("BrowserState", "receiving", 0, __FILE__, __LINE__, g);

			CookieSetLastCookieUrl(NULL, g); // Clear out last cookie url

			g->state = BS_HTTP_RECEIVE;
			break;
		case BS_HTTP_RECEIVE:
			EvtResetAutoOffTimer(); // Make sure that the device does not power off during a long download...
						// Eventually add a progress meter too.

			if (g->httpParser.state == PS_Error) {
				HTTPCleanUp(&g->sock, &g->httpParser);
				browserStateError(g);
                                MyErrorFunc("Failed to retrieve requested URL", NULL);
				break;
			}

			DebugOutInternal("BrowserState", "get data", 0, __FILE__, __LINE__, g);

			HTTPParseEngine(g->sock, &g->httpParser);

			DebugOutInternal("BrowserState", "data got", 0, __FILE__, __LINE__, g);

			if (g->httpParser.state == PS_Done) {
				HTTPCleanUp(&g->sock, &g->httpParser);
				g->state = BS_HTTP_HANDLE;
				break;
			} else if (g->httpParser.state == PS_Error) {
				HTTPCleanUp(&g->sock, &g->httpParser);
				browserStateError(g);
                                MyErrorFunc("Failed to retrieve requested URL", NULL);
				break;
			} else {
				g->state = BS_HTTP_RECEIVE;
			}

			DebugOutInternal("BrowserState", "bottom receive", 0, __FILE__, __LINE__, g);
			break;
		case BS_HTTP_HANDLE:
			browserSetTitle("Handling Data"); // SetFieldFromStr("Handling Data",fieldTitle);

			http_hand = HTTPHandle(&(g->webData), g);

			DebugOutInternal("BrowserState", "handled", http_hand, __FILE__, __LINE__, g);

			if (http_hand == 1) { // redirection or another non-halting alert
				break;
			} else if (http_hand == 2) {
				browserStateError(g);
				MyErrorFunc("Not enough memory to complete request. Please free up some memory and try again", NULL);
				break;
			} else if (http_hand == 3) {
				browserStateError(g);
				break;
			}

			DebugOutInternal("BrowserState", "no err", 0, __FILE__, __LINE__, g);

			if (!g->contentType) {
				browserStateError(g);

				break;
			}

			DebugOutInternal("BrowserState", "content type", 0, __FILE__, __LINE__, g);

            		if ((StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlc",24)==0) &&
				(http_hand != 7)) {
				CacheAddFileToCache(&(g->webData), g->Url, false, g);
            			g->state = BS_WBXML_PARSE;

				g->pageStats.pageSize = g->webData.length;
            		} else if (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlscriptc",30)==0) {
				CacheAddFileToCache(&(g->webData), g->Url, false, g);
				browserWmlsStateError(g);
				callWMLScriptFunc(g->Url);
				browserStateError(g);
	    		} else if (((StrNCaselessCompare(g->contentType,"application/vnd.wap.xhtml+xml",29)==0) || 
				    (StrNCaselessCompare(g->contentType,"application/xhtml+xml",21)==0) || 
				    (StrNCaselessCompare(g->contentType,"application/wml+xml",19)==0) ||
				    (StrNCaselessCompare(g->contentType,"text/html",9)==0) ||
				    (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)==0)) && 
					(http_hand != 7)) {
					CacheAddFileToCache(&(g->webData), g->Url, false, g);
            				g->state = BS_XHTML_PARSE_RENDER;

					g->pageStats.pageSize = g->webData.length;
            		} else if ((StrNCaselessCompare(g->contentType,"multipart/",10)==0) && 
					(http_hand != 7)) {
            			if (HTTPMultipartExtract(&g->httpParser) != 0) {
	        			browserStateError(g);

					MyErrorFunc("Invalid data type.", NULL);
				} else {
					g->state = BS_HTTP_HANDLE;
				}
            		} else if ((StrNCaselessCompare(g->contentType,"application/rss+xml",19)==0) &&
					(http_hand != 7)) {
            			g->state = BS_RSS_PARSE_RENDER;
			} else {
				browserSetTitle((g->deckTitle)?g->deckTitle:"WAPUniverse"); // SetFieldFromStr("WAPUniverse",fieldTitle);

				for (i=0; i < StrLen(g->Url) && i < MAX_URL_LEN; i++) {
					if (g->Url[i] == '/'){
						j = i;
					}
    				}
    				j++;
				switch (DownloadAlert (g->Url+j, g->contentType)) {
					case 0: // Download to internal
						if (g->httpUseTempFile == true) {
							UInt16 			cardNoP = 0;
							LocalID 		dbIDP;
							DmSearchStateType 	stateInfoP;

							error = DmGetNextDatabaseByTypeCreator(true, &stateInfoP, 'DATA', 'Wapu', true, &cardNoP, &dbIDP);
							dbIDP = DmFindDatabase(cardNoP, "Universe-HTTP-Cache");
							error = DownloadFileDatabase(dbIDP, g->Url+j, g->Url+j, g);
							g->httpUseTempFile = false;
							HTTPFlushCache();
						} else {
							error = DownloadFile(g->Url+j, &g->webData, g);
						}
						break;
					case 1: // Cancel
						break;
					case 2: // Download to card
						{
						Char *entryStringBuf = Malloc(255);
						if (!entryStringBuf) break;
						StrCopy(entryStringBuf, g->Url+j);

						FrmCustomResponseAlert(alVfsDownload, g_prefs.downloadTo, NULL, NULL, entryStringBuf, 255, FormHandleVfsDownload);
						if (entryStringBuf) Free(entryStringBuf);
						}
						break;
					default:
						break;
				}
				HTTPFlushCache();
				browserResetURL(g);
				browserPopPageContent(g);
	        		g->state = BS_IDLE;
	        		g->progress= 0;
	        		showProgress(g);
	   		}

			DebugOutInternal("BrowserState", "bottom handle", 0, __FILE__, __LINE__, g);

			break;
    		case BS_WBXML_PARSE:
			palmResetState(g);
	
			g->wmldck.card=NULL;
			g->imgH = 0;
			g->hasEvent = false;
			g->onEvent = NOEVENT;
			g->indent = 0;
			g->rightIndent = 0;
			g->userCancel = false;
			palmRegular(true);
			WinSetTextColor(UIColorGetTableEntryIndex(UIFieldText));
    			WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));

			g->browserGlobals.imgYAdd = true;

			browserSetTitle("Parsing Data"); // SetFieldFromStr("Parsing Data",fieldTitle);
        		stop = wbxml_parse(&(g->webData), ACTION_PARSE, &(g->wmldck), 0,g);
			if (stop == (-15)) {
				WspFreePdu(&(g->webData));
				g->state= BS_IDLE;
			} else {
				tmpPtr = StrChr(g->Url,'#');
				if (tmpPtr != NULL) {
					g->CardOffset = getCard(tmpPtr+1, g);
				} else {
					if(g->wmldck.card)  // if there is a first card defined, show it
	        				g->CardOffset = g->wmldck.card->offset;
					else
	       		 			g->CardOffset = 0;
				}
				g->state= BS_RENDER;
			}
        		break;
		case BS_NEW_CARD:
			if (g->pageContentType && 
			    ((StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29)==0) || 
			     (StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21)==0) || 
			     (StrNCaselessCompare(g->pageContentType,"application/wml+xml",19)==0) ||
			     (StrNCaselessCompare(g->pageContentType,"text/html",9)==0) ||
			     (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)==0))) {

			} else {
				palmResetCardState(g);

	    			g->x=0;g->y=0;
	    			g->imgH = 0;
    	    			g->ScrollY = 0;
				g->ScrollX = 0;
				g->browserGlobals.imgYAdd = true;
	    			HyperlinkFreeLinks(g);
	    			//disposeVar(g);
	    			disposeInput(g);
	    			disposeSelect(g);
				destroyForms(g);
				TextFreeText(g);
				freePageLocation(g);
				freePageImages(g);
				AreaFreeAreas(g);
				palmRegular(true);
				WinSetTextColor(UIColorGetTableEntryIndex(UIFieldText));
    				WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
	    			//prevDrawWin = WinGetDrawWindow();
    	    			//WinSetDrawWindow(g->DrawWin);
   	    			//WinEraseWindow();
    	    			//WinSetDrawWindow(prevDrawWin);
    	    			WinEraseRectangle(&r,0);
    	    			scrollUpdate(g);

				SetFieldFromStr(g->Url, fldUrlBox);
				FldReleaseFocus(GetObjectPtr(fldUrlBox));

	    			g->state= BS_RENDER;
			}
	    		break;
		case BS_RENDER:
	    		browserSetTitle("Rendering Data"); // SetFieldFromStr("Rendering data",fieldTitle);
			g->wmldck.ontimerUrl=NULL;
			g->wmldck.timerVal=-1;
			g->onforward = NULL;
			g->indent = 0;
			g->rightIndent = 0;
			g->imgY = 0;
			g->Render = true; // viewing a WML page after an HTML page might turn this off
			palmRegular(true);
			WinSetTextColor(UIColorGetTableEntryIndex(UIFieldText));
    			WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
			freeAlignment(g);
			freeFont(g);
			freeBackground(g);
			freeIndent(g);

	    		wbxml_parse(&(g->webData), ACTION_RENDER, &(g->wmldck), g->CardOffset,g);

			if (g->engineStop || g->userCancel) {
				g->state = BS_COMPLETE_WML;
			} else if (g->pageImages && ImageIsDownloadableImages(g)) {
				g->downloadImage = g->pageImages;
				g->state = BS_DOWNLOAD_IMG;
			} else {
				g->state = BS_COMPLETE_WML;
			}
			break;
		case BS_COMPLETE_WML:
			if (g->hasEvent) {
				g->state= BS_IDLE;
				if (g->onforward && (g->onforward != NULL))
					followLink(g->onforward,g);
				g->hasEvent = false;
				if (g->onforward) Free(g->onforward);
			} else {
        			updateScreen(g);
				if(g->wmldck.timerVal > 0){
	        			g->state = BS_ONTIMER;
	        			maxTimer = TimGetTicks() + ( (g->wmldck.timerVal * SysTicksPerSecond() )/10);
				}else{
					g->state = BS_IDLE;
				}
			}

			EvtResetAutoOffTimer();

			BrowserRefreshScrollBars(g);
			DOMRenderDOM(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), 0 /*g->ScrollX*/, 0 /*g->ScrollY*/, g->ScrollX + browserGetWidgetBounds(WIDGET_WIDTH), g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT), true, true, g);

			if (g->engineStop)
				MyErrorFunc("The entire page could not be displayed because your device is low on dynamic memory.", NULL);


			HistoryAddPageToHistory(g->Url, g->deckTitle, g);

        		g->progress= 0;
        		showProgress(g);
			g->onenterback = false;
			g->refreshEvent = false;
			g->engineStop = false;
			g->userCancel = false;
			g->cache.cacheDoNotRead = false;
			g->BrowserAPI.lastWidgetWidth = browserGetWidgetBounds(WIDGET_WIDTH);
			g->downloadImage = NULL;

			g->pageStats.timePageLoadFinish = TimGetTicks();
			break;	
		case BS_XHTML_PARSE_RENDER:
			palmResetState(g);

			g->imgH = 0;
			g->imgY = 0;
			g->hasEvent = false;
			g->onEvent = NOEVENT;
			g->onforward = NULL;
			g->indent = 0;
			g->rightIndent = 0;
			g->xhtml.ontimerUrl = NULL;
			g->userCancel = false;
			freeAlignment(g);
			freeFont(g);
			freeBackground(g);
			freeIndent(g);

			browserSetTitle("Parsing Data"); // SetFieldFromStr("Parsing data",fieldTitle);

			xhtmlErr = xhtml_parse(&(g->xhtml), &(g->webData), g);
			if (xhtmlErr != 0) {
				browserStateError(g);
			}

			DebugOutInternal("BS_XHTML_PARSE_RENDER", "1", 0, __FILE__, __LINE__, g);

			if (g->engineStop || g->userCancel) {
				g->state = BS_COMPLETE_HTML;
			} else if (g->pageImages && ImageIsDownloadableImages(g)) {
				g->downloadImage = g->pageImages;
				g->state = BS_DOWNLOAD_IMG;
			} else {
				g->state = BS_COMPLETE_HTML;
			}


			if (StrChr(g->Url, '#'))
				scrollToPageLocation(StrChr(g->Url, '#') + 1, g);
			break;
		case BS_COMPLETE_HTML:
			//updateScreen(g);
			//g->state = BS_IDLE;
			DebugOutInternal("BS_COMPLETE_HTML", "top", 0, __FILE__, __LINE__, g);

			if (g->hasEvent) {
				DebugOutInternal("BS_COMPLETE_HTML", "has event", 0, __FILE__, __LINE__, g);
				g->state= BS_IDLE;
				if (g->onforward)
					followLink(g->onforward, g);
				g->hasEvent = false;
				if (g->onforward) Free(g->onforward);
			} else {
        			updateScreen(g);
				if(g->xhtml.timerVal > 0){
					DebugOutInternal("BS_COMPLETE_HTML", "has timer", 0, __FILE__, __LINE__, g);
	        			g->state = BS_XHTML_ONTIMER;
	        			maxTimer = TimGetTicks() + (g->xhtml.timerVal * SysTicksPerSecond());
				} else {
					DebugOutInternal("BS_COMPLETE_HTML", "idle", 0, __FILE__, __LINE__, g);
					g->state = BS_IDLE;
				}
			}
			
			DebugOutInternal("BS_COMPLETE_HTML", "1", 0, __FILE__, __LINE__, g);

			EvtResetAutoOffTimer();

			BrowserRefreshScrollBars(g);
			DOMRenderDOM(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), 0 /*g->ScrollX*/, 0 /*g->ScrollY*/, g->ScrollX + browserGetWidgetBounds(WIDGET_WIDTH), g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT), true, true, g);

			DebugOutInternal("BS_COMPLETE_HTML", "2", 0, __FILE__, __LINE__, g);

			if (g->engineStop)
				MyErrorFunc("The entire page could not be displayed because your device is low on dynamic memory.", NULL);

			DebugOutInternal("BS_COMPLETE_HTML", "3", 0, __FILE__, __LINE__, g);

			HistoryAddPageToHistory(g->Url, g->deckTitle, g);

			DebugOutInternal("BS_COMPLETE_HTML", "4", 0, __FILE__, __LINE__, g);

        		g->progress = 0;
        		showProgress(g);
			g->onenterback = false;
			g->refreshEvent = false;
			g->engineStop = false;
			g->userCancel = false;
			g->cache.cacheDoNotRead = false;
			g->BrowserAPI.lastWidgetWidth = browserGetWidgetBounds(WIDGET_WIDTH);
			g->downloadImage = NULL;
			
			DebugOutInternal("BS_COMPLETE_HTML", "5", 0, __FILE__, __LINE__, g);

			if (g->rssFeed != NULL)
				RSSShowIndicator(true);

			DebugOutInternal("BS_COMPLETE_HTML", "bottom", 0, __FILE__, __LINE__, g);

			g->pageStats.timePageLoadFinish = TimGetTicks();

			break;
		case BS_DOWNLOAD_IMG:
#ifdef ENABLE_IMG_THREAD
			ImageLoadThreadStart(g);
			//g->state = BS_DOWNLOADING_IMG;
#else
			if (g->downloadImage) {
				DebugOutInternal("Browser state", "loading image (pre)", 0, __FILE__, __LINE__, g);
				ImageLoadImages(g);
				DebugOutInternal("Browser State", "loading image (post)", 0, __FILE__, __LINE__, g);
				updateScreen(g);
			} else {
				if (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlc",24)) {
					g->state = BS_COMPLETE_WML;
            			} else if (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29) || 
					   !StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21) || 
					   !StrNCaselessCompare(g->pageContentType,"application/wml+xml",19) ||
					   !StrNCaselessCompare(g->pageContentType,"text/html",9) ||
					   !StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)) {
						g->state = BS_COMPLETE_HTML;
            			}
			}
#endif
			break;
		case BS_ALT_DOWNLOAD_IMG:
			if (g->downloadImage) {
				ImageLoadImages(g);
				updateScreen(g);
			} else {
				if (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlc",24)) {
					g->state = BS_COMPLETE_WML;
            			} else if (!StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29) || 
					   !StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21) || 
					   !StrNCaselessCompare(g->pageContentType,"application/wml+xml",19) ||
					   !StrNCaselessCompare(g->pageContentType,"text/html",9) ||
					   !StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)) {
						g->state = BS_COMPLETE_HTML;
            			}
			}
			break;
		case BS_DOWNLOADING_IMG:
			// This is an empty state to keep the engine running in the main thread while images download 
			// in the other thread. This is to allow the stop button to function. The ImageLoadThreadStop
			// function will handle changing states.
			break;
		case BS_UPDATE_IMG:
			updateScreen(g);
			g->state = BS_DOWNLOADING_IMG;
			break;
		case BS_DECODING_IMG:
			g->state = BS_DOWNLOADING_IMG;
			break;
		case BS_DOWNLOAD_STOP:
			ImageLoadThreadStop(g->imageThreadRef, g);
			break;
		case BS_RSS_PARSE_RENDER:
			palmResetState(g);

			g->imgH = 0;
			g->imgY = 0;
			g->hasEvent = false;
			g->onEvent = NOEVENT;
			g->onforward = NULL;
			g->indent = 0;
			g->rightIndent = 0;
			g->xhtml.ontimerUrl = NULL;
			g->userCancel = false;
			freeAlignment(g);
			freeFont(g);
			freeBackground(g);
			freeIndent(g);

			browserSetTitle("Parsing Data"); // SetFieldFromStr("Parsing data",fieldTitle);

			xhtmlErr = rss_parse(&(g->xhtml), &(g->webData), g);
			if (xhtmlErr != 0) {
				browserStateError(g);

        			g->progress= 0;
        			showProgress(g);
				g->onenterback = false;
				g->refreshEvent = false;

				break;
			}

			EvtResetAutoOffTimer();

			updateScreen(g);
			g->BrowserAPI.lastWidgetWidth = browserGetWidgetBounds(WIDGET_WIDTH);
			g->state = BS_IDLE;


			if (g->pageImages && !g->engineStop && !g->userCancel) {
				g->downloadImage = g->pageImages;
				while (g->downloadImage) {
					ImageLoadImages(g);
					updateScreen(g);
				}
			}

			if (g->engineStop == true) MyErrorFunc("The entire page could not be displayed because your device is low on dynamic memory.", NULL);

			HistoryAddPageToHistory(g->Url, g->deckTitle, g);

        		g->progress= 0;
        		showProgress(g);
			g->onenterback = false;
			g->refreshEvent = false;
			g->engineStop = false;
			g->userCancel = false;
			g->cache.cacheDoNotRead = false;
			g->downloadImage = NULL;

			g->pageStats.timePageLoadFinish = TimGetTicks();
			break;
		case BS_ONTIMER:
			if (!g->wmldck.ontimerUrl) {
				g->state = BS_IDLE;
				maxTimer = 0;
				g->wmldck.ontimerUrl = NULL;
				g->wmldck.timerVal = 0;
			} else if (maxTimer < TimGetTicks()){
				followLink(g->wmldck.ontimerUrl,g);
				Free(g->wmldck.ontimerUrl);
				g->wmldck.ontimerUrl = NULL;
				g->wmldck.timerVal = 0;
			} 
			break;
		case BS_XHTML_ONTIMER:
			if (!g->xhtml.ontimerUrl) {
				g->state = BS_IDLE;
				maxTimer = 0;
				g->xhtml.ontimerUrl = NULL;
				g->xhtml.timerVal = 0;
			} else if (maxTimer < TimGetTicks()){
				followLink(g->xhtml.ontimerUrl,g);
				Free(g->xhtml.ontimerUrl);
				g->xhtml.ontimerUrl = NULL;
				g->xhtml.timerVal = 0;
			}
			break;
		
    		default:
    	}
    	return true;
}



static void frmCloseEvent_OH(GlobalsType *g)
{

    FormInputDeactivate(g);

    ImageLoadStop(g);

    TabDeleteTabs(g);

    DOMDeleteTree(g);

    if (/*(g->conn.connectionType == 'O') && */(g->wtpState == WTP_Connected)) { //clean up our WTP session
	WSPDisconnect(g->sock, g);
    }
    // close socket if needed              
    NetCloseSocket(&g->sock);
    if(g->NetLibOpen == true)
	NetLibClose(AppNetRefnum, false);
   	g->NetLibOpen=false;
    if (g->inBuf != NULL) {
        Free(g->inBuf);
        g->inBuf = NULL;
    }
    if (g->postBuf != NULL) {
	Free(g->postBuf);
	g->postBuf = NULL;
    }
    if (g->Url != NULL) {
        Free(g->Url);
        g->Url = NULL;
    }
    if (g->contentType != NULL) {
        Free(g->contentType);
        g->contentType = NULL;
    }
    if (g->pageContentType != NULL) {
        Free(g->pageContentType);
        g->pageContentType = NULL;
    }
    if (g->rssFeed != NULL) {
        Free(g->rssFeed);
        g->rssFeed = NULL;
    }
    if (g->urlBuf != NULL) {
	Free(g->urlBuf);
	g->urlBuf = NULL;
    }
    if (g->browserGlobals.browserTitle != NULL) {
	Free(g->browserGlobals.browserTitle);
	g->browserGlobals.browserTitle = NULL;
    }
    //WinDeleteWindow(g->DrawWin,false);
    //g->DrawWin = NULL;
    WspFreePdu(&(g->webData));
    HyperlinkFreeLinks(g);
    disposeVar(g);
    disposeInput(g);
    disposeSelect(g);
    destroyForms(g);
    freePageLocation(g);
    freePageImages(g);
    SARdeleteData(g);
    ChannelClose(g);
    freeAlignment(g);
    freeFont(g);
    freeList(g);
    freeBackground(g);
    freeIndent(g);
    TextFreeText(g);
    AreaFreeAreas(g);
    wbxml_free_wmldck(&(g->wmldck));
    xhtml_free(&(g->xhtml), g);
    rss_free(&(g->xhtml), g);
    DmReleaseResource(g->resBmpPrgrs[0]);
    DmReleaseResource(g->resBmpPrgrs[1]);
    DmReleaseResource(g->resBmpPrgrs[2]);
    DmReleaseResource(g->resBmpPrgrs[3]);
    DmReleaseResource(g->resBmpPrgrs[4]);
    DmReleaseResource(g->resBmpPrgrs[5]);
    DmReleaseResource(g->resBmpPrgrs[6]);
    DmReleaseResource(g->resBmpPrgrs[7]);
    DmReleaseResource(g->resBmpPrgrs[8]);
    DmReleaseResource(g->resBmpPrgrs[9]);
    DmReleaseResource(g->resBmpPrgrs[10]);
    DmReleaseResource(g->resBmpPrgrs[11]);
    DmReleaseResource(g->resBmpPrgrs[12]);
    g->pushedPageLength = 0;
    if (g->pushedPageContent)
	Free(g->pushedPageContent);
    g->pushedPageContent = NULL;
    if (g->pushedPageContentType)
	Free(g->pushedPageContentType);
    g->pushedPageContentType = NULL;

    PointerCleanUp(&g->pointer, g);

    WinPopDrawState();
}

/**----------------------------------**/
/** Form Event Handler for FormBrowser**/
/**----------------------------------**/

Boolean browserHandleOptionsList(Int16 selection, GlobalsType *g)
{
	Boolean handled = false;

	if (selection >= 0) {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
		g->browserGlobals.optMenuOpen = false;
	}

	switch (selection) {
		case 0: // Home
			handled = browserHomeEvt(g);
			break;
		case 1: // Open Site
			FrmPopupForm(frmUrl);
			handled = true;
			break;
		case 2: // Add bookmark
			addBookmark(g);
			handled = true;
			break;
		case 3: // View Bookmarks
			handled = browserURLsEvt(g);
			break;
		case 4: // Reload
			handled = browserReloadButtonEvt(g);
			break;
		/*case 5: // Use Wallet
			{
			FieldPtr fld;

				fld = GetFocusObjectPtr ();
				if (fld) {
					InsertFromWallet();
				} else {
					MyErrorFunc("A field must be active to use the wallet", NULL);
				}
			}
			handled = true;
			break;*/
		case 5: // View News Feed
			FormInputDeactivate(g);
			if (g->rssFeed != NULL) followLink(g->rssFeed, g);
			handled = true;
			break;
		case 6: // History
			FrmPopupForm(frmHistory);
			handled = true;
			break;
		case 7: // Connection Settings
			g->prevFormID = frmBrowser;
			FrmPopupForm(frmConnectionList); //SwitchForm(frmConnectionList,g);
			handled = true;
			break;
		case 8: // Browser Settings
			FrmPopupForm(frmPrefs);
			handled = true;
			break;
		case 9: // Page Info
      	  		switch (ShowURL(g->Url, NULL, NULL)){
      	  			case 1:
					switch ( FrmCustomAlert (alSetHome, g->ActiveConnection, "", "") ) {
						case 1:
							setHome (g);
							break;
						default:
							break;
					}
      	  				break;

				case 2:
					addBookmark (g);
					break;
      	  	
      	  			default:
      	  				break;
      	  		}
			handled = true;
			break;
		case 10: // Exit Browser
			ExitApp();
			break;
		default:
			FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
			FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
			FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
			WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
			g->browserGlobals.optMenuOpen = false;
			handled = true;
			break;
	}

	return handled;
}

/****************************************************/
/* 
   Function: Boolean FormBrowser_EH(EventPtr event);
   Description: Main Form Event Handler for FormBrowser
 */
/****************************************************/



/* Form Event Handler */
Boolean ehFormBrowser(EventPtr event)
{
	Boolean		handled = false;
  	GlobalsType	*g;
  	FormPtr         frm = FrmGetActiveForm();
	RectangleType	screen;
	Err		saveErr = 0;
	BitmapPtr	resP = 0;
	UInt32		scrollTo;

#ifdef __GNUC__
    CALLBACK_PROLOGUE
#endif

	screen.topLeft.x = 0;
	screen.topLeft.y = browserGetScreenHeight()-100;
	screen.extent.x = 160;
	screen.extent.y = 86;

		  
	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    if (ResizeHandleEvent(event))
        return true;

    switch (event->eType) {
    
    case menuCmdBarOpenEvent:
        MenuCmdBarAddButton( menuCmdBarOnLeft, bmpBookmark2, 
                menuCmdBarResultMenuItem, miBookmarks, NULL );
        MenuCmdBarAddButton( menuCmdBarOnLeft, bmpSource, 
                menuCmdBarResultMenuItem, miViewSource, NULL );

        if (NetLibConnected() == true){
        	MenuCmdBarAddButton( menuCmdBarOnLeft, bmpDisconnect, 
                	menuCmdBarResultMenuItem, miDis, NULL );
		g->NetLibOpen = true;
	} else {
        	MenuCmdBarAddButton( menuCmdBarOnLeft, bmpConnect, 
                	menuCmdBarResultMenuItem, miConnect, NULL );
		g->NetLibOpen = false;
	}
               
    handled = false;
    break;
    
    case menuOpenEvent:
        if (NetLibConnected() == true){
        	MenuHideItem(miConnect);
        	MenuShowItem(miDis);
		g->NetLibOpen = true;
        } else {
        	MenuHideItem(miDis);
        	MenuShowItem(miConnect);
		g->NetLibOpen = false;
        }

	/*if (g_prefs.renderMode == RENDER_SMALL_SCREEN) {
		MenuHideItem(miRenderSmallScreen);
		MenuHideItem(miRenderWS_Sel);
		MenuShowItem(miRenderWideScreen);
		MenuShowItem(miRenderSS_Sel);
	} else if (g_prefs.renderMode == RENDER_WIDE_SCREEN) {
		MenuHideItem(miRenderWideScreen);
		MenuHideItem(miRenderSS_Sel);
		MenuShowItem(miRenderSmallScreen);
		MenuShowItem(miRenderWS_Sel);
	}*/
    handled = true;
    break;
    
    case ctlSelectEvent:
    	if (event->data.ctlSelect.controlID == buttonMenu) {
			//FormInputDeactivate(g);
            handled = EvtEnqueueKey( menuChr, 0, commandKeyMask );
        }
        if (event->data.ctlSelect.controlID == buttonBack) {
			FormInputDeactivate(g);
		handled = browserBackEvt(g);
        }
        if (event->data.ctlSelect.controlID == buttonHome) {
			FormInputDeactivate(g);
			handled = browserHomeEvt(g);
        }
        if (event->data.ctlSelect.controlID == buttonOpenURL) {
			FrmPopupForm(frmUrl);
        }
        if (event->data.ctlSelect.controlID == buttonReload) {
			FormInputDeactivate(g);
			handled = browserReloadButtonEvt(g);
        }
        if (event->data.ctlSelect.controlID == buttonStop) {
			FormInputDeactivate(g);
			handled = browserStopEvt(g);
        }
        if (event->data.ctlSelect.controlID == buttonBook) {
			FormInputDeactivate(g);
			handled = browserURLsEvt(g);
        }
	if (event->data.ctlSelect.controlID == buttonRss) {
			FormInputDeactivate(g);
			if (g->rssFeed != NULL) followLink(g->rssFeed, g);
			handled = true;
        }
	if (event->data.ctlSelect.controlID == buttonTabClose) {
		if (g->state != BS_IDLE) {
			browserStopEvt(g);
		}
		TabDeleteTab(g->tabList.currentTab, true, g);
	}
	if (event->data.ctlSelect.controlID == buttonGo) {
		FieldPtr	urlFldP = GetObjectPtr(fldUrlBox);
		Char		*urlP = FldGetTextPtr(urlFldP);
		Char		*goUrl = NULL;

		if (urlP && (FldGetTextLength(urlFldP) > 1) 
			 && (StrNCaselessCompare( urlP, "http://", 7 ))
			 && (StrNCaselessCompare( urlP, "https://", 8 ))
			 && (StrNCaselessCompare( urlP, "wtai://", 7))
			 && (StrNCaselessCompare( urlP, "tel:", 4))
			 && (StrNCaselessCompare( urlP, "mailto:", 7))
			 && (StrNCaselessCompare( urlP, "device:", 7))
			 && (StrNCaselessCompare( urlP, "internal:", 9))
			 && (StrNCaselessCompare( urlP, "about:", 6))
			 && (StrNCaselessCompare( urlP, "proxy:", 6))
			 && (StrNCaselessCompare( urlP, "mtch:", 5))
			 && (StrNCaselessCompare( urlP, "file:", 5)) ) {				    
				FldSetInsPtPosition( urlFldP, 0 );
				FldInsert( urlFldP, "http://", 7 );
				FldReleaseFocus( urlFldP );
		}

		urlP = FldGetTextPtr(urlFldP);

		if (urlP && (FldGetTextLength(urlFldP) > 1)) {
			goUrl = StrDup(urlP);

			if (goUrl) {
				followLink(goUrl, g);
				Free(goUrl);
			}
		}

		FldReleaseFocus(GetObjectPtr(fldUrlBox));
	}
        if (event->data.ctlSelect.controlID == buttonSoftOnea) {

		FormInputDeactivate(g);

		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));

		g->browserGlobals.saved = WinSaveBits(&screen, &saveErr);

		FrmSetObjectPosition(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions), 1, browserGetScreenHeight() - 82);
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));

		LstSetSelection((ListPtr)(FrmGetObjectPtr(frm, (FrmGetObjectIndex(frm, listOptions)))), 0);

		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpOptionsMenu));
    		WinDrawBitmap(resP, 0, browserGetScreenHeight()-100);
    		MemPtrUnlock(resP);

		g->browserGlobals.optMenuOpen = true;

		handled = true;
        }
        if (event->data.ctlSelect.controlID == buttonSoftOneb) {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
		g->browserGlobals.optMenuOpen = false;
		handled = true;
        }
        if (event->data.ctlSelect.controlID == buttonSoftTwoa) {
		FormInputDeactivate(g);
		handled = browserBackEvt(g);
        }
        if (event->data.ctlSelect.controlID == buttonSoftTwob) {
		FormInputDeactivate(g);
		handled = browserStopEvt(g);
        }
        /*if (event->data.ctlSelect.controlID == buttonHistory) {
			FormInputDeactivate(g);
			FrmPopupForm(frmHistory);
        }*/
        break;

    case lstSelectEvent:
	if (event->data.lstSelect.listID == listOptions) {
		handled = browserHandleOptionsList(event->data.lstSelect.selection,g);
	}
	break;

    case frmOpenEvent:
        frmOpenEvent_OH(g);
        break;
    
    case frmUpdateEvent:
	FrmDrawForm(FrmGetActiveForm());

	BrowserUpdateScreen(g);

        switch (event->data.frmUpdate.updateCode) {
        	case frmUpdateGotoUrl:
        		followLink(g->OpenUrl,g);
			handled = true;
        		break;
		case frmUpdateMode:
			browserSwitchMode(g_prefs.mode);
			handled = true;
			break;
		case frmUpdateNewTab:
			TabCreateTab(g->OpenUrl, /* TODO: Make preference*/ true, g);
			if (g->OpenUrl)
				Free(g->OpenUrl);
			g->OpenUrl = NULL;
			handled = true;
			break;
        }
	handled = true;
        break;

    case winDisplayChangedEvent:
	if (WinGetActiveWindow() == FrmGetWindowHandle(FrmGetFormPtr(frmBrowser))) {
		FormInputDeactivate(g);
		BrowserUpdateScreen(g);
		BrowserRefreshScrollBars(g);
	}
	break;

    case winEnterEvent:
	if ((event->data.winEnter.enterWindow == (WinHandle)FrmGetFormPtr(frmBrowser)) &&
	    !g->InputFldActive)
		updateScreen(g);
	break;

    case frmCloseEvent:
        frmCloseEvent_OH(g);
        break;

    case nilEvent:
        handled = runBrowserState(g);
        break;

    case sclRepeatEvent:
	if (g->browserGlobals.optMenuOpen == true) {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
		g->browserGlobals.optMenuOpen = false;
	}
	FormInputDeactivate(g);
        //g->ScrollY = event->data.sclRepeat.newValue*10;
        //scrollUpdate(g);

	scrollTo = event->data.sclRepeat.newValue;

	switch (event->data.sclRepeat.scrollBarID) {
       		case scrollBarBrowser:
			//scrollTo += (((scrollTo < g->ScrollY)?-1:1) * BrowserLineheightValue()) - 1;
			BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
					g->BrowserAPI.BrowserWidget), scrollTo, g);
			break;
		case scrollBarHorizontal:
			//scrollTo += (((scrollTo < g->ScrollX)?-1:1) * BrowserLineheightValue()) - 1;
			BrowserScrollHorizontalTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
						  g->BrowserAPI.BrowserWidget), scrollTo, g);
			break;
	}
        break;

    case keyDownEvent:
	if (event->data.keyDown.chr == chrLineFeed) {
		if (!g->InputFldActive && FldGetFocus(fldUrlBox)) {
			CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonGo)));
			handled = true;
		} else if (g->InputFldActive) {
			if (g->InputFldActive->type == inputTypeText) {
				if (FormGetAssociatedForm(g->InputFldActive, TEXT, g->Forms)) {
					InputPtr	active = g->InputFldActive;

					FormInputDeactivate(g);
					FormSetFirstSubmitButton(FormGetAssociatedForm(active, TEXT, g->Forms), g->formButton, g);
					FormSubmitForm(FormGetAssociatedForm(active, TEXT, g->Forms), g);

					handled = true;
				}
			}
		} else if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			handled = browserReloadButtonEvt(g);
		}
	} else if ((event->data.keyDown.chr == chrDelete) ||
		   (event->data.keyDown.chr == chrBackspace)) {
		if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			handled = browserBackEvt(g);
		}
	} else if (event->data.keyDown.chr == 'x') {
		if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			handled = browserStopEvt(g);
		}
	} else if (event->data.keyDown.chr == ' ') {
		if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			FrmSetFocus(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fldUrlBox));
			FldSetSelection(GetObjectPtr(fldUrlBox), 0, StrLen(FldGetTextPtr(GetObjectPtr(fldUrlBox))));
			
			handled = true;
		} /*else if (g->InputFldActive) {
			if (g->InputFldActive->type == inputTypeText)
				handled = true;
		}*/
	} else if (event->data.keyDown.chr == 'r') {
		if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
									      g->BrowserAPI.BrowserWidget),
					g->ScrollY - BrowserLineheightValue(), g);
			
			handled = true;
		}
	} else if (event->data.keyDown.chr == 'c') {
		if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
									      g->BrowserAPI.BrowserWidget),
					g->ScrollY + BrowserLineheightValue(), g);
			
			handled = true;
		}
	} else if (event->data.keyDown.chr == 'd') {
		if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			BrowserScrollHorizontalTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
											g->BrowserAPI.BrowserWidget),
						  g->ScrollX - BrowserLineheightValue(), g);
			
			handled = true;
		}
	} else if (event->data.keyDown.chr == 'g') {
		if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			BrowserScrollHorizontalTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
											g->BrowserAPI.BrowserWidget),
						  g->ScrollX + BrowserLineheightValue(), g);
			
			handled = true;
		}
	} else if ((event->data.keyDown.chr == '+') ||
		   (event->data.keyDown.chr == 'w')) {
		if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			// scroll up or down??
			BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
									      g->BrowserAPI.BrowserWidget),
					g->ScrollY - browserGetWidgetBounds(WIDGET_HEIGHT), g);
			
			handled = true;
		}
	} else if ((event->data.keyDown.chr == '-') ||
		   (event->data.keyDown.chr == 's')) {
		if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
			// scroll up or down??
			BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
									      g->BrowserAPI.BrowserWidget),
					g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT), g);
			
			handled = true;
		}
	}

        break;

	
	case menuEvent:
	    //FormInputDeactivate(g);
            handled = mhMenuView(event->data.menu.itemID,g);
  	    break;

	case mouseOverEvent:
		handled = browserPenOverEvt(event, g);
		break;
  	
	case penDownEvent:
		handled = browserPenDownEvt(event, g);
		break;

	case penUpEvent: // leave this one at the end of the switch !!
		handled = browserPenUpEvt(event, g);
		break;
		
	default:
		break;

    }
#ifdef __GNUC__
    CALLBACK_EPILOGUE
#endif
    return handled;
}


/* Item Handler Functions Here */

/**--------------------------------------**/
/** Form Object Handler Functions for FormBrowser **/
/**--------------------------------------**/


static Boolean browserPenOverEvt(EventPtr event, GlobalsType *g)
{
	Int16	xpos = event->screenX;
	Int16	ypos = event->screenY;

	xpos -= browserGetWidgetBounds(WIDGET_TOPLEFT_X) / HiresFactor();
	ypos -= browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor();

	return BrowserHandlePenOver(xpos, ypos, g);
}

static Boolean browserPenDownEvt(EventPtr event, GlobalsType *g)
{
	Int16 xpos = event->screenX;
	Int16 ypos = event->screenY;

	if (WinGetActiveWindow() != FrmGetWindowHandle(FrmGetFormPtr(frmBrowser)))
		return false;

	if ((g->browserGlobals.optMenuOpen == true) && (ypos > (browserGetScreenHeight()-100)) && (ypos < (browserGetScreenHeight() - 14))) {
		if ((xpos > (browserGetScreenWidth() - 2)) || ((ypos > (browserGetScreenHeight()-100)) && (ypos < (browserGetScreenHeight()-82))))
			return true;

		return false;
	}

	if ((g->browserGlobals.optMenuOpen == true) && !((ypos > (browserGetScreenHeight() - 14)) && (ypos < browserGetScreenHeight()) && (xpos < 65))) {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
		WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
		g->browserGlobals.optMenuOpen = false;
	}

	if((xpos > (browserGetWidgetBounds(WIDGET_WIDTH) / HiresFactor())) || (ypos > ((browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor()) + 
		(browserGetWidgetBounds(WIDGET_HEIGHT) / HiresFactor()))) || (ypos < ((browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor()) + 2)))
			return(false);

	if (g->InputFldActive) {
		RectangleType		bounds;
		
		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fldInput), &bounds);
		if (RctPtInRectangle(xpos, ypos, &bounds)) {
			//FldEmulateEnter(fldInput);
			return false; // FldHandleEvent(GetObjectPtr(fldInput), event);
		} else {
			FormInputDeactivate(g);
		}
	}

	PointerSetPosition(xpos * HiresFactor(), ypos * HiresFactor(), &g->pointer, true, g);

	xpos -= browserGetWidgetBounds(WIDGET_TOPLEFT_X) / HiresFactor();
	ypos -= browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor();

	//if (formIsInputActive(g->InputFldActive, xpos + g->ScrollX, ypos + g->ScrollY, g))
	//	return false;

	return BrowserHandlePenTap(xpos, ypos, true, g);
}

static Boolean browserPenUpEvt(EventPtr event, GlobalsType *g)
{
	Int16 	xpos,ypos;


	xpos = event->screenX;
	ypos = event->screenY;

	if (WinGetActiveWindow() != FrmGetWindowHandle(FrmGetFormPtr(frmBrowser)))
		return false;

	if (g->InputFldActive) {
		RectangleType		bounds;
		
		FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fldInput), &bounds);
		if (RctPtInRectangle(xpos, ypos, &bounds)) {
			return false; // FldHandleEvent(GetObjectPtr(fldInput), event);
		}
	}

	if (g->browserGlobals.tapandhold == true) {
		g->browserGlobals.tapandhold = false;
		g->browserGlobals.taptick = 0;
		g->browserGlobals.tapx = 0; g->browserGlobals.tapy = 0;
		g->browserGlobals.taptype = TAPNONE;
	}
	
	if (g->browserGlobals.optMenuOpen == true) {
		if (((ypos < browserGetScreenHeight()-100) || (ypos > (browserGetScreenHeight() - 14))) &&
			!(((ypos > (browserGetScreenHeight() - 14)) && (ypos < browserGetScreenHeight())) && ((xpos > 0) && (xpos < 65))) ) {
			FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
			FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
			FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
			WinRestoreBits(g->browserGlobals.saved, 0, browserGetScreenHeight()-100); g->browserGlobals.saved = 0;
			g->browserGlobals.optMenuOpen = false;
		}
		return(false);
	}

	if((xpos > (browserGetWidgetBounds(WIDGET_WIDTH) / HiresFactor())) || (ypos > ((browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor()) + 
		(browserGetWidgetBounds(WIDGET_HEIGHT) / HiresFactor()))) || (ypos < (browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor()))) {
			//if ((xpos > 14) && (xpos < 110) && (ypos < ((browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor()) - 14)) && (!MenuGetActiveMenu())) { // not a great way to do it...
			//	EvtEnqueueKey( menuChr, 0, commandKeyMask );
			//	return true;
			//}

			if (g->selectedLink) {
				HyperlinkDeSelectLink(g->selectedLink, g);
				g->selectedLink = NULL;
			}

			if (ypos < (browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor())) {
				return TabHandlePen(xpos, ypos, g);
			}
			
			return(false);
	}

	PointerSetPosition(xpos * HiresFactor(), ypos * HiresFactor(), &g->pointer, false, g);

	// user clicked in the browser area, check if he/she clicked a hyperlink

	// let's see if we have a link under this xpos/ypos position

	xpos -= browserGetWidgetBounds(WIDGET_TOPLEFT_X) / HiresFactor();
	ypos -= browserGetWidgetBounds(WIDGET_TOPLEFT_Y) / HiresFactor();

	//if (formIsInputActive(g->InputFldActive, xpos + g->ScrollX, ypos + g->ScrollY, g))
	//	return false;

	return BrowserHandlePenTap(xpos, ypos, false, g);
}

Boolean browserStopEvt(GlobalsType *g)
{
	FormPtr         frm;

    	g->progress= 0;
    	showProgress(g);

	ImageLoadStop(g);

    	dbConnGetConnection(g->DbUrlCurrentURL.connection,&(g->conn),g);
    	if ((g->conn.connectionType == 'O') && ((g->state == BS_GET_URL) || 
		(g->state == BS_GET_REPLY) || (g->state == BS_WTP_REGET_URL) || 
		(g->state == BS_WTP_CONNECT) || (g->state == BS_WTP_GETCONNECT))) { // abort request

		WTPAbort (g->sock, g); // A combo of Abort and Disconnect
		//WTPDisconnect (g->sock, g);
		/*if (g->sock >= 0)
        		close (g->sock);
    		g->sock = -1;*/
		if ((g->wtpState == WTP_Disconnected) && (g->sock != -1)) {
			NetCloseSocket(&g->sock);
		}
		SARdeleteData(g);
		//WSPCloseSocket(g->sock);
    	}

	if ((g->state == BS_HTTP_GET) || (g->state == BS_HTTP_RECEIVE)) {
		FileClose(g->httpParser.fd);
		HTTPCleanUp(&g->sock, &g->httpParser);
	}

	if ((g->state == BS_HTTP_GET) || (g->state == BS_HTTP_RECEIVE) ||
	    (g->state == BS_GET_URL) || (g->state == BS_GET_REPLY) ||
	    (g->state == BS_WTP_REGET_URL) || (g->state == BS_WTP_CONNECT) ||
	    (g->state == BS_WTP_GETCONNECT)) {
		browserResetURL(g);
		browserPopPageContent(g);
	}

	g->post = false;
    	if (g->postBuf != NULL){ Free(g->postBuf); g->postBuf = NULL; }
	if (g->AuthUser != NULL){ Free(g->AuthUser); g->AuthUser = NULL; }
      	if (g->AuthPass != NULL){ Free(g->AuthPass); g->AuthPass = NULL; }

	g->browserGlobals.retransCount = 0;

	if (g->state != BS_IDLE && g->state != BS_ONTIMER && g->state != BS_XHTML_ONTIMER)
		browserSetTitle((g->deckTitle)?g->deckTitle:"WAPUniverse"); // SetFieldFromStr("WAPUniverse",fieldTitle);

    	g->isImage = false;
    	g->state = BS_IDLE;
    	
    	frm = FrmGetActiveForm();

    	return(true);
}

			
/****************************************************/
/* 
   Function: Boolean ButtonURLs_OH(EventPtr event);
   Description: Button Object Handler for the FormBrowser Form
 */
/****************************************************/

Boolean browserURLsEvt(GlobalsType *g)
{
    g->prevFormID = frmBrowser;
    FrmPopupForm(frmUrlList); //SwitchForm(frmUrlList,g);
    return(true);   
}


/****************************************************/
/* 
   Function: Boolean ButtonBack_OH(EventPtr event);
   Description: Button Object Handler for the FormBrowser Form
 */
/****************************************************/

Boolean browserBackEvt(GlobalsType *g)
{
    Char	*tmpUrl;
    FormPtr	frm;
    //WinHandle	prevDrawWin;

	ImageLoadStop(g);

	g->post = false;//if we don't do this things get messy
	
    
	frm = FrmGetActiveForm ();
	tmpUrl = historyPop(g);
	if (!tmpUrl)
		return false; //return(browserURLsEvt(g));
	if(g->Url)
		Free(g->Url);
	g->Url=Malloc(MAX_URL_LEN+1);
	ErrFatalDisplayIf (!g->Url, "Malloc");
	URLCreateURLStr(tmpUrl,g->Url,g);
	Free(tmpUrl);

	g->onenterback = true;

	browserReloadEvt(g);
	return(true);   
}

Boolean browserHomeEvt(GlobalsType *g)
{	
	g->post = false;//if we don't do this things get messy

	g->DbConnCurrentRecord = g->ActiveConnStore;
	
	dbConnReadCurrentConnection (g);
	followLink(g->CurrentConnection.home, g);

	return true;
}


