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
#include 	<sys_socket.h>
#include	<netinet_in.h>
#include	"Tab.h"
#include 	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"../res/WAPUniverse_res.h"
#include	"Browser.h"
#include	"Layout.h"
#include	"Form.h"
#include	"xhtml.h"
#include	"wml.h"
#include	"variable.h"
#include	"rss.h"
#include	"history.h"
#include	"Area.h"
#include	"image.h"
#include	"Text.h"
#include	"DOM.h"
#include	"wsp.h"
#include	"PalmUtils.h"
#include	"dbOffline.h"
#include	"FiveWay.h"
#include	"Font.h"
#include	"Hyperlinks.h"


static UInt32 tabIDList[MAX_TABS + 1] = {
	NULL,	// Tab 0 does not exist
	BrowserTab1,
	BrowserTab2,
	BrowserTab3
};

Boolean TabInitializeTabs(Char *url, Boolean active, GlobalsType *g)
{
	TabStackType	*stack;
	TabStackType	*activeStack;

	if ((g->tabList.tabCount + 1) > MAX_TABS) {
		MyErrorFunc("You have reached the maximum allowed tabs. Please close a tab before opening a new one.", NULL);
		return false;
	}

	stack = Malloc(sizeof(TabStackType));
	if (!stack)
		return false;
	MemSet(stack, sizeof(TabStackType), 0);

	stack->g = (GlobalsType_dummy *)g;

	activeStack = g->tabList.tabStack;
	if (activeStack) {
		while (activeStack->next) {
			activeStack = activeStack->next;
		}
		activeStack->next = stack;
	} else {
		g->tabList.tabStack = stack;
	}

	g->tabList.tabCount++;

	if (!g->tabList.currentTab)
		g->tabList.currentTab = 1;

	if (active)
		TabSwitchTab(g->tabList.tabCount, g);

	FtrGet(wuAppType, ftrGlobals, (UInt32*)&g);

	if (url)
		followLink(url, g);

	return true;
}

void TabSetupGlobals(GlobalsType *g)
{
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
    	g->deckTitle = StrDup("Universe"); // TODO: Don't do this inline
    	g->AuthUser = NULL;
    	g->AuthPass = NULL;
    	g->InputFldActive=NULL;

	g->browserGlobals.retransCount = 0;
	g->browserGlobals.nextTick = 0;
	g->browserGlobals.saved = 0;
	g->browserGlobals.savedclength = 0;
	g->browserGlobals.savedrlength = 0;
	g->browserGlobals.updateMenu = true;
	g->browserGlobals.redrawMenuBar = true;
	g->browserGlobals.tapandhold = false;
	g->browserGlobals.taptick = 0;
	g->browserGlobals.tapx = 0;
	g->browserGlobals.tapy = 0;
	g->browserGlobals.imgYAdd = true;
	g->browserGlobals.optMenuOpen = false;

    	g->DrawHeight = 0; // browserGetWidgetBounds(WIDGET_HEIGHT);
	g->DrawWidth = browserGetWidgetBounds(WIDGET_WIDTH);
	g->BrowserAPI.lastWidgetWidth = browserGetWidgetBounds(WIDGET_WIDTH);
    	g->ScrollY = 0;
	g->ScrollX = 0;
   	g->x=0;
	g->y=0;
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
}

Boolean TabCreateTab(Char *url, Boolean active, GlobalsType *g)
{
	TabStackType	*stack;
	TabStackType	*activeStack;

	if ((g->tabList.tabCount + 1) > MAX_TABS) {
		MyErrorFunc("You have reached the maximum allowed tabs. Please close a tab before opening a new one.", NULL);
		return false;
	}

	stack = Malloc(sizeof(TabStackType));
	if (!stack)
		return false;
	MemSet(stack, sizeof(TabStackType), 0);

	stack->g = Malloc(sizeof(GlobalsType));
	if (!stack->g) {
		Free(stack);
		MyErrorFunc("Could not create a new tab. No memory.", NULL);
		return false;
	}
	MemSet(stack->g, sizeof(GlobalsType), 0);

	// TODO: Need to initialize the globals that do not change across tabs??
	TabSetupGlobals((GlobalsType *)stack->g);

	activeStack = g->tabList.tabStack;
	if (activeStack) {
		while (activeStack->next) {
			activeStack = activeStack->next;
		}
		activeStack->next = stack;
	} else {
		g->tabList.tabStack = stack;
	}

	g->tabList.tabCount++;

	if (!g->tabList.currentTab)
		g->tabList.currentTab = 1;

	if (active)
		TabSwitchTab(g->tabList.tabCount, g);

	FtrGet(wuAppType, ftrGlobals, (UInt32*)&g);

	if (url)
		followLink(url, g);

	return true;
}

Boolean TabIsValidTab(UInt32 tabNumber, GlobalsType *g)
{
	TabStackType	*stack;
	UInt32		count = 1;

	// If MAX_TABS is 255 that means unlimited tab support.
	// Make sure that we don't create more tabs then allowed.
	if ((tabNumber > MAX_TABS) && (MAX_TABS != 255))
		return false;

	stack = g->tabList.tabStack;
	while (stack->next) {
		stack = stack->next;
		count++;
	}

	// Check to see that we are asking for a valid tab
	if ((tabNumber > count) || (!tabNumber))
		return false;
	else
		return true;

	return true;
}

TabStackType *TabGetTabStack(UInt32 tabNumber, GlobalsType *g)
{
	TabStackType	*activeStack;
	UInt32		count = 1;

	if (!TabIsValidTab(tabNumber, g))
		return NULL;

	if (tabNumber == 1) {
		return g->tabList.tabStack;
	} else {
		activeStack = g->tabList.tabStack;

		while ((count < (tabNumber - 1)) && activeStack->next) {
			activeStack = activeStack->next;
			count++;
		}
		
		return activeStack->next;
	}

	return NULL;
}

void TabSwapResources(UInt32 tabNumber, GlobalsType *g1, GlobalsType *g2)
{
	if (g2) { // If we have the last tab, then we want to copy the last tab to new tab and switch
		g1->EditURLConnection	= g2->EditURLConnection;
		g1->pngLibrary		= g2->pngLibrary;
		g1->CookieJar		= g2->CookieJar;
		g1->CurrentView		= g2->CurrentView;
		g1->nextView		= g2->nextView;
		g1->CurrentCategory	= g2->CurrentCategory;
		g1->RecordCategory	= g2->RecordCategory;
		g1->NewCategory		= g2->NewCategory;
		g1->prevFormID		= g2->prevFormID;
		g1->DbUrlCurrentCategory= g2->DbUrlCurrentCategory;
		g1->DbConnCurrentRecord	= g2->DbConnCurrentRecord;
		g1->ActiveConnStore	= g2->ActiveConnStore;
		MemMove(&g1->DbUrlCategoryName, &g2->DbUrlCategoryName, sizeof(g1->DbUrlCategoryName));
		MemMove(&g1->ActiveConnection, &g2->ActiveConnection, sizeof(g1->ActiveConnection));
		g1->DbHistory		= g2->DbHistory;
		g1->DbCookieJar		= g2->DbCookieJar;
		g1->DbUrlDb		= g2->DbUrlDb;
		g1->DbConnDb		= g2->DbConnDb;
		g1->xhtmlOutputFileRef	= g2->xhtmlOutputFileRef;
		g1->debugOutputFileRef	= g2->debugOutputFileRef;
		g1->srcHandle		= g2->srcHandle;
		MemMove(&g1->resBmpPrgrs, &g2->resBmpPrgrs, sizeof(g1->resBmpPrgrs));
		g1->ConnectionsHandle	= g2->ConnectionsHandle;
		g1->ConnectionsListArrayH = g2->ConnectionsListArrayH;
		//MemMove(&g1->BrowserAPI, &g2->BrowserAPI, sizeof(BrowserAPIType));
		g1->BrowserAPI.BrowserWidget		= g2->BrowserAPI.BrowserWidget;
		g1->BrowserAPI.BrowserInputField	= g2->BrowserAPI.BrowserInputField;
		g1->BrowserAPI.BrowserOptionsList	= g2->BrowserAPI.BrowserOptionsList;
		g1->BrowserAPI.BrowserScrollBar		= g2->BrowserAPI.BrowserScrollBar;
		g1->BrowserAPI.BrowserScrollBarHorizontal = g2->BrowserAPI.BrowserScrollBarHorizontal;
		//g1->cache.DbCache	= g2->cache.DbCache;
		MemMove(&g1->cache, &g2->cache, sizeof(CacheType));
		MemMove(&g1->CurrentConnection, &g2->CurrentConnection, sizeof(dbConnConnection));
		MemMove(&g1->conn, &g2->conn, sizeof(dbConnConnection));
		MemMove(&g1->tabList, &g2->tabList, sizeof(TabArrayType));
		MemMove(&g1->WAPAppInfo, &g2->WAPAppInfo, sizeof(gWAPAppInfoType));
		//MemMove(&g1->pageStats, &g2->pageStats, sizeof(PageStatsType));
        g1->pageStats.totalMemoryUsed = g2->pageStats.totalMemoryUsed;
		MemMove(&g1->pointer, &g2->pointer, sizeof(WindowPointerType));
		g1->imageThreadRef	= g2->imageThreadRef;
		g1->imageThreadLoadCount= g2->imageThreadLoadCount;
		g1->NetLibOpen		= g2->NetLibOpen;
		g1->privateBrowsing	= g2->privateBrowsing;
		g1->NewCategoryEdited	= g2->NewCategoryEdited;
		g1->Var			= g2->Var;
		g1->gifContext		= g2->gifContext;
	}

	FtrSet(wuAppType, ftrGlobals, (UInt32)g1);
}

Boolean TabSwitchTab(UInt32 tabNumber, GlobalsType *g)
{
	UInt32		currentTab;

	if (!TabIsValidTab(tabNumber, g))
		return false;

	currentTab = g->tabList.currentTab;
	g->tabList.currentTab = tabNumber;


	TabSwapResources(tabNumber, (GlobalsType*)(TabGetTabStack(tabNumber, g))->g,
				    (GlobalsType*)(TabGetTabStack(currentTab, g))->g);

	FtrGet(wuAppType, ftrGlobals, (UInt32*)&g);

	TabDrawTabs(false, g);

	updateScreen(g);
	if (g->Url)
		SetFieldFromStr(g->Url, fldUrlBox);
	RSSShowIndicator((g->rssFeed)?true:false);

	return true;
}



Boolean TabFreeResources(TabStackType *stack, GlobalsType *g)
{
	if (!stack)
		return false;

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
    	//disposeVar(g);
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
    	g->pushedPageLength = 0;
    	if (g->pushedPageContent)
		Free(g->pushedPageContent);
    	g->pushedPageContent = NULL;
    	if (g->pushedPageContentType)
		Free(g->pushedPageContentType);
    	g->pushedPageContentType = NULL;

	Free(stack->g);
	Free(stack);

	return true;
}

Boolean TabDeleteTabs(GlobalsType *g)
{
	UInt32	count = 1;
	UInt32	tabCount = g->tabList.tabCount;

	while (count < tabCount) {
		TabDeleteTab(1, false, g);
		count++;
	}

	return true;
}

Boolean TabDeleteTab(UInt32 tabNumber, Boolean refresh, GlobalsType *g)
{
	TabStackType	*stack;
	TabStackType	*activeStack;
	TabStackType	*tempStack;
	TabStackType	*nextStack;
	
	if (!TabIsValidTab(tabNumber, g))
		return false;

	if (g->tabList.tabCount == 1) {
		// assuming that we a re deleting the 1st tab, with only 1 tab

		return false;
	}

	activeStack = g->tabList.tabStack;
	if ((tabNumber > 1) && (tabNumber < g->tabList.tabCount)) {
		stack = TabGetTabStack(tabNumber, g);
		if (!stack)
			return false;
		tempStack = TabGetTabStack(tabNumber - 1, g);
		if (!tempStack)
			return false;
		nextStack = TabGetTabStack(tabNumber + 1, g);
		if (!nextStack)
			return false;

		TabSwapResources(tabNumber + 1, (GlobalsType*)(TabGetTabStack(tabNumber + 1, g))->g,
						(GlobalsType*)stack->g); // update the new tabs resources
		TabSwapResources(tabNumber, (GlobalsType*)stack->g,
					    NULL); // Make ours active

		tempStack->next = nextStack;

		TabFreeResources(stack, (GlobalsType *)stack->g);

		TabSwapResources(tabNumber, (GlobalsType *)nextStack->g, NULL); // Make next active

		FtrGet(wuAppType, ftrGlobals, (UInt32*)&g);
		g->tabList.tabCount--;

		TabDrawTabs(false, g);
	} else if (tabNumber > 1) {
		stack = TabGetTabStack(tabNumber, g);
		if (!stack)
			return false;
		tempStack = TabGetTabStack(tabNumber - 1, g);
		if (!tempStack)
			return false;
		
		TabSwapResources(tabNumber - 1, (GlobalsType*)tempStack->g,
						(GlobalsType*)stack->g); // update the new tabs resources
		TabSwapResources(tabNumber, (GlobalsType*)stack->g,
					    NULL); // Make ours active

		tempStack->next = NULL;

		TabFreeResources(stack, (GlobalsType *)stack->g);

		TabSwapResources(tabNumber - 1, (GlobalsType*)tempStack->g,
					    NULL); // Make next active

		FtrGet(wuAppType, ftrGlobals, (UInt32*)&g);

		g->tabList.currentTab = tabNumber - 1;
		g->tabList.tabCount--;

		TabDrawTabs(false, g);
	} else {
		stack = TabGetTabStack(tabNumber, g);
		if (!stack)
			return false;
		tempStack = TabGetTabStack(tabNumber + 1, g);
		if (!tempStack)
			return false;

		TabSwapResources(tabNumber + 1, (GlobalsType*)tempStack->g,
						(GlobalsType*)stack->g); // update the new tabs resources
		TabSwapResources(tabNumber, (GlobalsType*)stack->g,
					    NULL); // Make ours active

		TabFreeResources(stack, (GlobalsType *)stack->g);

		TabSwapResources(tabNumber, (GlobalsType *)tempStack->g,
					    NULL); // Make next active

		FtrGet(wuAppType, ftrGlobals, (UInt32*)&g);
		g->tabList.tabStack = tempStack;
		g->tabList.tabCount--;

		TabDrawTabs(false, g);
	}

	if (refresh) {
		updateScreen(g);
		if (g->Url)
			SetFieldFromStr(g->Url, fldUrlBox);
	}

	return true;
}

Boolean TabDrawTitle(TabStackType *stack, Boolean isActiveTab, RectangleType *bounds)
{
	WinDrawOperation 	oldMode;
	RGBColorType		rgb;
	IndexedColorType	index, origText = 0;

	if (!stack)
		return false;

	rgb.r = 255;
	rgb.g = 255;
	rgb.b = 255;

	if (stack->g->deckTitle && isActiveTab) {
		browserSetTitle(stack->g->deckTitle);
	} else if (stack->g->deckTitle) {
    		index = WinRGBToIndex( &rgb );
		origText = WinSetTextColor(index);

		setTitleFont(true);
		oldMode = WinSetDrawMode(winOverlay);
		WinPaintChars(stack->g->deckTitle, FntWordWrap(stack->g->deckTitle, bounds->extent.x - 4), 
			      bounds->topLeft.x + 2, bounds->topLeft.y + 3);
		WinSetDrawMode(oldMode);
		setTitleFont(false);

		WinSetTextColor(origText);
	} else {
		return false;
	}

	return true;
}

void TabDrawProgress(BitmapPtr resP, GlobalsType *g)
{
    RectangleType   bounds;
    
    FrmGetObjectBounds(FrmGetActiveForm(), 
                FrmGetObjectIndex(FrmGetActiveForm(), tabIDList[g->tabList.currentTab]),
                &bounds);
    
    WinDrawBitmap(resP, bounds.topLeft.x + bounds.extent.x - 9,
                    bounds.topLeft.y + 4);//124, 148);
}

void TabDrawTabs(Boolean blankActive, GlobalsType *g)
{
	TabStackType	*stack;
	UInt32		count = 1;
	RectangleType	bounds;
	BitmapPtr	bmp, resP;

	if (WinGetActiveWindow() != FrmGetWindowHandle(FrmGetFormPtr(frmBrowser)))
		return;

	resP = MemHandleLock(DmGetResource(bitmapRsc, bmpMenu));
    	WinDrawBitmap (resP, 0, 0);
    	MemPtrUnlock(resP);
	resP = MemHandleLock(DmGetResource(bitmapRsc, bmpTopLeft));
    	WinDrawBitmap (resP, browserGetScreenWidth() - 4, 0);
    	MemPtrUnlock(resP);

	while (count <= g->tabList.tabCount) {
		FrmGetObjectBounds(FrmGetActiveForm(), 
				   FrmGetObjectIndex(FrmGetActiveForm(), tabIDList[count]),
				   &bounds);

		stack = TabGetTabStack(count, g);

		if (count == g->tabList.currentTab) {
			bmp = MemHandleLock(DmGetResource(bitmapRsc, bmpTabSelected));
			WinDrawBitmap(bmp, bounds.topLeft.x, bounds.topLeft.y);
			MemPtrUnlock(bmp);

			FrmSetObjectPosition(FrmGetActiveForm(),
					     FrmGetObjectIndex(FrmGetActiveForm(), BrowserTitle),
					     bounds.topLeft.x + 2, bounds.topLeft.y + 4);

            if (g->progress) {
				FrmHideObject(FrmGetActiveForm(),
					      FrmGetObjectIndex(FrmGetActiveForm(), buttonTabClose));
            }
            
            if (g->tabList.tabCount > 1) {
                FrmSetObjectPosition(FrmGetActiveForm(),
					     FrmGetObjectIndex(FrmGetActiveForm(), buttonTabClose),
					     bounds.topLeft.x + bounds.extent.x - 8, bounds.topLeft.y + 4);
                         
				FrmShowObject(FrmGetActiveForm(),
					      FrmGetObjectIndex(FrmGetActiveForm(), buttonTabClose));
            }

			if (g->tabList.tabCount == 1) {
				FrmHideObject(FrmGetActiveForm(),
					      FrmGetObjectIndex(FrmGetActiveForm(), buttonTabClose));

				bmp = MemHandleLock(DmGetResource(bitmapRsc, bmpTabSelected));
				WinDrawBitmap(bmp, bounds.topLeft.x, bounds.topLeft.y);
				MemPtrUnlock(bmp);
			} /*else {
				FrmShowObject(FrmGetActiveForm(),
					      FrmGetObjectIndex(FrmGetActiveForm(), buttonTabClose));
			}*/

			if (!blankActive)
				TabDrawTitle(stack, true, &bounds);
		} else if (count < g->tabList.currentTab) {
			bmp = MemHandleLock(DmGetResource(bitmapRsc, bmpTabDeselectedL));
			WinDrawBitmap(bmp, bounds.topLeft.x, bounds.topLeft.y);
			MemPtrUnlock(bmp);

			TabDrawTitle(stack, false, &bounds);
		} else {
			bmp = MemHandleLock(DmGetResource(bitmapRsc, bmpTabDeselected));
			WinDrawBitmap(bmp, bounds.topLeft.x, bounds.topLeft.y);
			MemPtrUnlock(bmp);

			TabDrawTitle(stack, false, &bounds);
		}

		count++;
	}
}

Boolean TabHandlePen(Coord x, Coord y, GlobalsType *g)
{
	RectangleType	bounds; //, close;
	UInt32		count = 1;

	/*FrmGetObjectBounds(FrmGetActiveForm(), 
			   FrmGetObjectIndex(FrmGetActiveForm(), buttonTabClose),
			   &close);

	if ((x >= close.topLeft.x) && (x <= (close.topLeft.x + close.extent.x)) &&
	    (y >= close.topLeft.y) && (y <= (close.topLeft.y + close.extent.y))) {
		if (g->state != BS_IDLE) {
			browserStopEvt(g);
			TabDeleteTab(g->tabList.currentTab, g);
		}

		return true;
	}*/

	while (count <= g->tabList.tabCount) {
		FrmGetObjectBounds(FrmGetActiveForm(), 
				   FrmGetObjectIndex(FrmGetActiveForm(), tabIDList[count]),
				   &bounds);
		
		if ((x >= bounds.topLeft.x) && (x <= (bounds.topLeft.x + bounds.extent.x)) &&
		    (y >= bounds.topLeft.y) && (y <= (bounds.topLeft.y + bounds.extent.y))) {
			if (count != g->tabList.currentTab) {
				TabSwitchTab(count, g);

				return true;
			} else {
				return false;
			}
		}
		
		count++;
	}

	return false;
}






/*void TabFreeText(TextPtr text)
{
	GlobalsType	g;

	g.text = text;

	TextFreeText(&g);
}

void TabFreeArea(AreaPtr area)
{
	GlobalsType	g;

	g.areaPtr = area;

	AreaFreeAreas(&g);
}

void TabFreeXHTML(XHTMLPtr xhtml, WebFontPtr font, AlignPtr align,
		  OLPtr list, HorizIndentPtr indent, PositionType *position)
{
	GlobalsType	g;

	g.font = font;
	g.AlignList = align;
	g.organizedList = list;
	g.horizIndent = indent;
	g.position = position;
	
	xhtml_free(xhtml, &g);
}

void TabFreeSAR(SARPtr segment)
{
	GlobalsType	g;

	g.segment = segment;

	SARdeleteData(&g);
}

void TabFreeLocation(CardPtr location)
{
	GlobalsType	g;

	g.htmlPageLocID = location;

	freePageLocation(&g);
}

void TabFreeImages(ImagePtr images, MapPtr maps)
{
	GlobalsType	g; // stupid way to do this

	g.pageImages = images;
	g.ImageMaps = maps;

	freePageImages(&g);
}

void TabFreeHyperLinks(HyperLinkPtr links)
{
	GlobalsType	g; // stupid way to do this

	g.HyperLinks = links;

	HyperlinkFreeLinks(&g);
}

void TabFreeForms(RadioInputPtr radio, CheckInputPtr check, ButtonInputPtr button,
		  HTMLFormPtr form)
{
	GlobalsType	g; // stupid way of doing this

	g.radioBtn = radio;
	g.checkBox = check;
	g.formButton = button;
	g.Forms = form;

	destroyForms(&g);
}

void TabFreeSelect(SelectPtr select)
{
	GlobalsType	g; // stupid way of doing this

	g.Select = select;

	disposeSelect(&g);
}

void TabFreeVariable(VarPtr var)
{
	GlobalsType	g; // stupid way of doing this

	g.Var = var;

	disposeVar(&g);
}

void TabFreeInput(InputPtr input)
{
	GlobalsType	g; // stupid way of doing this...

	g.Input = input;

	disposeInput(&g);
}

void TabFreeDOM(DOMPtr dom)
{
	GlobalsType	g;

	g.DOM = dom;

	DOMDeleteTree(&g);
}

void TabFreeRSS(RSSPtr rss, RSSPtr temp)
{
	GlobalsType	g;

	g.xhtml.buffer = NULL;

	g.rss = rss;
	g.tempRss = temp;

	rss_free(&g.xhtml, &g);
}

void TabFreeHistory(historyPtr history)
{
	GlobalsType	g;

	g.History = history;

	historyClear(&g);
}*/

/*void TabFreeText(TextPtr text, GlobalsType *g)
{
	TextPtr		t = g->text;

	g->text = text;

	TextFreeText(g);

	g->text = t;
}

void TabFreeArea(AreaPtr area, GlobalsType *g)
{
	AreaPtr		a = g->areaPtr;

	g->areaPtr = area;

	AreaFreeAreas(g);

	g->areaPtr = a;
}

void TabFreeXHTML(XHTMLPtr xhtml, WebFontPtr font, AlignPtr align,
		  OLPtr list, HorizIndentPtr indent, PositionType *position,
		  GlobalsType *g)
{
	WebFontPtr	f = g->font;
	AlignPtr	a = g->AlignList;
	OLPtr		l = g->organizedList;
	HorizIndentPtr	i = g->horizIndent;
	PositionType	*p = g->position;

	g->font = font;
	g->AlignList = align;
	g->organizedList = list;
	g->horizIndent = indent;
	g->position = position;
	
	xhtml_free(xhtml, g);

	g->font = f;
	g->AlignList = a;
	g->organizedList = l;
	g->horizIndent = i;
	g->position = p;
}

void TabFreeSAR(SARPtr segment, GlobalsType *g)
{
	SARPtr		s = g->segment;

	g->segment = segment;

	SARdeleteData(g);

	g->segment = s;
}

void TabFreeLocation(CardPtr location, GlobalsType *g)
{
	CardPtr		l = g->htmlPageLocID;

	g->htmlPageLocID = location;

	freePageLocation(g);

	g->htmlPageLocID = l;
}

void TabFreeImages(ImagePtr images, MapPtr maps, GlobalsType *g)
{
	ImagePtr	i = g->pageImages;
	MapPtr		m = g->ImageMaps;

	g->pageImages = images;
	g->ImageMaps = maps;

	freePageImages(g);

	g->pageImages = i;
	g->ImageMaps = m;
}

void TabFreeHyperLinks(HyperLinkPtr links, GlobalsType *g)
{
	HyperLinkPtr	l = g->HyperLinks;

	g->HyperLinks = links;

	HyperlinkFreeLinks(g);

	g->HyperLinks = l;
}

void TabFreeForms(RadioInputPtr radio, CheckInputPtr check, ButtonInputPtr button,
		  HTMLFormPtr form, GlobalsType *g)
{
	RadioInputPtr	r = g->radioBtn;
	CheckInputPtr	c = g->checkBox;
	ButtonInputPtr	b = g->formButton;
	HTMLFormPtr	f = g->Forms;

	g->radioBtn = radio;
	g->checkBox = check;
	g->formButton = button;
	g->Forms = form;

	destroyForms(g);

	g->radioBtn = r;
	g->checkBox = c;
	g->formButton = b;
	g->Forms = f;
}

void TabFreeSelect(SelectPtr select, GlobalsType *g)
{
	SelectPtr	s = g->Select;

	g->Select = select;

	disposeSelect(g);

	g->Select = s;
}

void TabFreeVariable(VarPtr var, GlobalsType *g)
{
	VarPtr		v = g->Var;

	g->Var = var;

	disposeVar(g);

	g->Var = v;
}

void TabFreeInput(InputPtr input, GlobalsType *g)
{
	InputPtr	i = g->Input;

	g->Input = input;

	disposeInput(g);

	g->Input = i;
}

void TabFreeDOM(DOMPtr dom, GlobalsType *g)
{
	DOMPtr		d = g->DOM;

	g->DOM = dom;

	DOMDeleteTree(g);

	g->DOM = d;
}

void TabFreeRSS(XHTMLPtr xhtml, RSSPtr rss, RSSPtr temp, GlobalsType *g)
{
	RSSPtr		r = g->rss;
	RSSPtr		t = g->tempRss;

	g->xhtml.buffer = NULL;

	g->rss = rss;
	g->tempRss = temp;

	rss_free(xhtml, g);

	g->rss = r;
	g->tempRss = t;
}

void TabFreeHistory(historyPtr history, GlobalsType *g)
{
	historyPtr	h = g->History;

	g->History = history;

	historyClear(g);

	g->History = h;
}*/

/*Boolean TabSwapResources(UInt32 tabNumber, Boolean stackToGlobals, GlobalsType *g)
{
	TabStackType *stack;

	stack = TabGetTabStack(tabNumber, g);
	if (!stack)
		return false;

	if (stackToGlobals) {
		g->CardOffset 		= stack->CardOffset;
		g->indent 		= stack->indent;
		g->xindent 		= stack->xindent;
		g->fontH 		= stack->fontH;
		g->rightIndent 		= stack->rightIndent;
		g->contentLength 	= stack->contentLength;
		g->timeElapse		= stack->timeElapse;
		g->sid			= stack->sid;
		g->imgH			= stack->imgH;
		g->imgY			= stack->imgY;
		g->pushedPageLength	= stack->pushedPageLength;
		if (g->urlBuf)
			Free(g->urlBuf);
		g->urlBuf		= stack->urlBuf;
		if (g->contentType)
			Free(g->contentType);
		g->contentType		= stack->contentType;
		if (g->pageContentType)
			Free(g->pageContentType);
		g->pageContentType	= stack->pageContentType;
		if (g->launchURL)
			Free(g->launchURL);
		g->launchURL		= stack->launchURL;
		if (g->inBuf)
			Free(g->inBuf);
		g->inBuf		= stack->inBuf;
		if (g->Url)
			Free(g->Url);
		g->Url			= stack->Url;
		if (g->EditURLConnection)
			Free(g->EditURLConnection);
		g->EditURLConnection	= stack->EditURLConnection;
		if (g->OpenUrl)
			Free(g->OpenUrl);
		g->OpenUrl		= stack->OpenUrl;
		if (g->deckTitle)
			Free(g->deckTitle);
		g->deckTitle		= stack->deckTitle;
		if (g->postBuf)
			Free(g->postBuf);
		g->postBuf		= stack->postBuf;
		if (g->AuthUser)
			Free(g->AuthUser);
		g->AuthUser		= stack->AuthUser;
    		if (g->AuthPass)
			Free(g->AuthPass);
		g->AuthPass		= stack->AuthPass;
		if (g->onforward)
			Free(g->onforward);
		g->onforward		= stack->onforward;
		if (g->partBound)
			Free(g->partBound);
		g->partBound		= stack->partBound;
		if (g->rssFeed)
			Free(g->rssFeed);
		g->rssFeed		= stack->rssFeed;
		if (g->pushedPageContent)
			Free(g->pushedPageContent);
		g->pushedPageContent	= stack->pushedPageContent;
		if (g->pushedPageContentType)
			Free(g->pushedPageContentType);
		g->pushedPageContentType = stack->pushedPageContentType;
		if (g->wmlsData)
			Free(g->wmlsData);
		g->wmlsData		= stack->wmlsData;
		g->InputFldActive	= stack->InputFldActive;
		//TabFreeInput(g->Input);
		disposeInput(g);
		g->Input		= stack->Input;
		//TabFreeVariable(g->Var);
		disposeVar(g);
		g->Var			= stack->Var;
		//TabFreeSelect(g->Select);
		disposeSelect(g);
		g->Select		= stack->Select;
		//TabFreeForms(g->radioBtn, g->checkBox, g->formButton, g->Forms);
		destroyForms(g);
		g->radioBtn		= stack->radioBtn;
		g->checkBox		= stack->checkBox;
		g->formButton		= stack->formButton;
		g->Forms		= stack->Forms;
		g->ActiveForm		= stack->ActiveForm;
		//TabFreeXHTML(&g->xhtml, g->font, g->AlignList, g->organizedList, g->horizIndent, g->position);
		xhtml_free(&g->xhtml, g);
		g->font			= stack->font;
		g->AlignList		= stack->AlignList;
		g->organizedList	= stack->organizedList;
		g->horizIndent		= stack->horizIndent;
		g->position		= stack->position;
		g->inlineImage		= stack->inlineImage; // is this proper?
		//TabFreeRSS(g->rss, g->tempRss);
		rss_free(&g->xhtml, g);
		g->rss			= stack->rss;
		g->tempRss		= stack->tempRss;
		//TabFreeLocation(g->htmlPageLocID);
		freePageLocation(g);
		g->htmlPageLocID	= stack->htmlPageLocID;
		//TabFreeImages(g->pageImages, g->ImageMaps);
		freePageImages(g);
		g->pageImages		= stack->pageImages;
		g->ImageMaps		= stack->ImageMaps;
		g->downloadImage	= stack->downloadImage;
		g->fontH		= stack->fontH;
		//TabFreeSAR(g->segment);
		SARdeleteData(g);
		g->segment		= stack->segment;
		//TabFreeHyperLinks(g->HyperLinks);
		HyperlinkFreeLinks(g);
		g->HyperLinks		= stack->HyperLinks;
		g->currentLink		= stack->currentLink;
		//TabFreeText(g->text);
		TextFreeText(g);
		g->text			= stack->text;
		//TabFreeDOM(g->DOM);
		DOMDeleteTree(g);
		g->DOM			= stack->DOM;
MyErrorFunc("area",NULL);
		//TabFreeArea(g->areaPtr);
		AreaFreeAreas(g);
		g->areaPtr		= stack->areaPtr;
		g->selectedLink		= stack->selectedLink;
		//TabFreeHistory(g->History);
MyErrorFunc("history",NULL);
		historyClear(g);
		g->History		= stack->History;
		g->charSet		= stack->charSet;
		g->positionIndent	= stack->positionIndent;
		g->redirects		= stack->redirects;
		g->progress		= stack->progress;
		g->DrawHeight		= stack->DrawHeight;
		g->DrawWidth		= stack->DrawWidth;
		g->ScrollY		= stack->ScrollY;
		g->ScrollX		= stack->ScrollX;
		g->x			= stack->x;
		g->y			= stack->y;
		g->state		= stack->state;
		g->sock			= stack->sock;
		g->DbUrlCurrentRecord	= stack->DbUrlCurrentRecord;
		g->needSpace		= stack->needSpace;
		g->onEvent		= stack->onEvent;
		g->wtpState		= stack->wtpState;
        	g->tid			= stack->tid;
        	g->old_tid		= stack->old_tid;
		//MemMove(&g->WapLastUrl, &stack->WapLastUrl, MAX_URL_LEN + 1);
		MemMove(&g->WapLastUrl, &stack->WapLastUrl, sizeof(g->WapLastUrl));
		MemMove(&g->CookieLastUrl, &stack->CookieLastUrl, sizeof(g->CookieLastUrl));
		g->xhtmlOutputFileRef	= stack->xhtmlOutputFileRef;
		g->srcHandle		= stack->srcHandle;

		//
		//MemHandle       	ConnectionsHandle;
		//MemHandle       	ConnectionsListArrayH;
		//
MyErrorFunc("some memmove",NULL);
		MemMove(&g->RmtAddress, &stack->RmtAddress, sizeof(struct sockaddr_in));
		MemMove(&g->LclAddress, &stack->LclAddress, sizeof(struct sockaddr_in));
		WspFreePdu(&g->webData);
		MemMove(&g->webData, &stack->webData, sizeof(WebDataType));

		//
		//BrowserGlobalsType	browserGlobals;
		//BrowserAPIType		BrowserAPI;
		//
MyErrorFunc("more memmove",NULL);
		MemMove(&g->offline, &stack->offline, sizeof(OfflineType));
		MemMove(&g->xhtml, &stack->xhtml, sizeof(XHTML));
		if (g->httpParser.saveFileName)
			Free(g->httpParser.saveFileName);
    		if (g->httpParser.url)
			Free(g->httpParser.url);
		MemMove(&g->httpParser, &stack->httpParser, sizeof(HTTPParse));
		MemMove(&g->cache, &stack->cache, sizeof(CacheType));
		g->Align		= stack->Align;
		g->imgAlign		= stack->imgAlign;
MyErrorFunc("more more memmove",NULL);
		MemMove(&g->DbUrlCurrentURL, &stack->DbUrlCurrentURL, sizeof(dbUrlURL));
MyErrorFunc("wml free",NULL);
		wbxml_free_wmldck(&g->wmldck);
		MemMove(&g->wmldck, &stack->wmldck, sizeof(WMLDeck));
		g->imageThreadRef	= stack->imageThreadRef;
		g->imageThreadLoadCount	= stack->imageThreadLoadCount;
		g->tid_new		= stack->tid_new;
		g->Render		= stack->Render;
		g->engineStop		= stack->engineStop;
		g->userCancel		= stack->userCancel;
		g->httpUseTempFile	= stack->httpUseTempFile;
		g->hasEvent		= stack->hasEvent;
		g->retransmission	= stack->retransmission;
		g->wasPost		= stack->wasPost;
		g->isImage		= stack->isImage;
		g->onenterback		= stack->onenterback;
		g->refreshEvent		= stack->refreshEvent;
		g->post			= stack->post;
		g->wmlsBrowserContextHandled = stack->wmlsBrowserContextHandled;
MyErrorFunc("bottom",NULL);
	} else {
		stack->CardOffset 		= g->CardOffset;
		stack->indent 			= g->indent;
		stack->xindent 			= g->xindent;
		stack->fontH 			= g->fontH;
		stack->rightIndent 		= g->rightIndent;
		stack->contentLength 		= g->contentLength;
		stack->timeElapse		= g->timeElapse;
		stack->sid			= g->sid;
		stack->imgH			= g->imgH;
		stack->imgY			= g->imgY;
		stack->pushedPageLength		= g->pushedPageLength;
		if (stack->urlBuf)
			Free(stack->urlBuf);
		stack->urlBuf			= g->urlBuf;
		if (stack->contentType)
			Free(stack->contentType);
		stack->contentType		= g->contentType;
		if (stack->pageContentType)
			Free(stack->pageContentType);
		stack->pageContentType		= g->pageContentType;
		if (stack->launchURL)
			Free(stack->launchURL);
		stack->launchURL		= g->launchURL;
		if (stack->inBuf)
			Free(stack->inBuf);
		stack->inBuf			= g->inBuf;
		if (stack->Url)
			Free(stack->Url);
		stack->Url			= g->Url;
		if (stack->EditURLConnection)
			Free(stack->EditURLConnection);
		stack->EditURLConnection	= g->EditURLConnection;
		if (stack->OpenUrl)
			Free(stack->OpenUrl);
		stack->OpenUrl			= g->OpenUrl;
		if (stack->deckTitle)
			Free(stack->deckTitle);
		stack->deckTitle		= g->deckTitle;
		if (stack->postBuf)
			Free(stack->postBuf);
		stack->postBuf			= g->postBuf;
		if (stack->AuthUser)
			Free(stack->AuthUser);
		stack->AuthUser			= g->AuthUser;
    		if (stack->AuthPass)
			Free(stack->AuthPass);
		stack->AuthPass			= g->AuthPass;
		if (stack->onforward)
			Free(stack->onforward);
		stack->onforward		= g->onforward;
		if (stack->partBound)
			Free(stack->partBound);
		stack->partBound		= g->partBound;
		if (stack->rssFeed)
			Free(stack->rssFeed);
		stack->rssFeed			= g->rssFeed;
		if (stack->pushedPageContent)
			Free(stack->pushedPageContent);
		stack->pushedPageContent	= g->pushedPageContent;
		if (stack->pushedPageContentType)
			Free(stack->pushedPageContentType);
		stack->pushedPageContentType 	= g->pushedPageContentType;
		if (stack->wmlsData)
			Free(stack->wmlsData);
		stack->wmlsData			= g->wmlsData;
		stack->InputFldActive		= g->InputFldActive;
		TabFreeInput(stack->Input, g);
		stack->Input			= g->Input;
		TabFreeVariable(stack->Var, g);
		stack->Var			= g->Var;
		TabFreeSelect(stack->Select, g);
		stack->Select			= g->Select;
		TabFreeForms(stack->radioBtn, stack->checkBox, stack->formButton, stack->Forms, g);
		stack->radioBtn			= g->radioBtn;
		stack->checkBox			= g->checkBox;
		stack->formButton		= g->formButton;
		stack->Forms			= g->Forms;
		stack->ActiveForm		= g->ActiveForm;
		TabFreeXHTML(&stack->xhtml, stack->font, stack->AlignList, stack->organizedList, stack->horizIndent, stack->position, g);
		stack->font			= g->font;
		stack->AlignList		= g->AlignList;
		stack->organizedList		= g->organizedList;
		stack->horizIndent		= g->horizIndent;
		stack->position			= g->position;
		stack->inlineImage		= g->inlineImage; // is this proper?
		TabFreeRSS(&stack->xhtml, stack->rss, stack->tempRss, g);
		stack->rss			= g->rss;
		stack->tempRss			= g->tempRss;
		TabFreeLocation(stack->htmlPageLocID, g);
		stack->htmlPageLocID		= g->htmlPageLocID;
		TabFreeImages(stack->pageImages, stack->ImageMaps, g);
		stack->pageImages		= g->pageImages;
		stack->ImageMaps		= g->ImageMaps;
		stack->downloadImage		= g->downloadImage;
		stack->fontH			= g->fontH;
		TabFreeSAR(stack->segment, g);
		stack->segment			= g->segment;
		TabFreeHyperLinks(stack->HyperLinks, g);
		stack->HyperLinks		= g->HyperLinks;
		stack->currentLink		= g->currentLink;
		TabFreeText(stack->text, g);
		stack->text			= g->text;
		TabFreeDOM(stack->DOM, g);
		stack->DOM			= g->DOM;
		TabFreeArea(stack->areaPtr, g);
		stack->areaPtr			= g->areaPtr;
		stack->selectedLink		= g->selectedLink;
		TabFreeHistory(stack->History, g);
		stack->History			= g->History;
		stack->charSet			= g->charSet;
		stack->positionIndent		= g->positionIndent;
		stack->redirects		= g->redirects;
		stack->progress			= g->progress;
		stack->DrawHeight		= g->DrawHeight;
		stack->DrawWidth		= g->DrawWidth;
		stack->ScrollY			= g->ScrollY;
		stack->ScrollX			= g->ScrollX;
		stack->x			= g->x;
		stack->y			= g->y;
		stack->state			= g->state;
		stack->sock			= g->sock;
		stack->DbUrlCurrentRecord	= g->DbUrlCurrentRecord;
		stack->needSpace		= g->needSpace;
		stack->onEvent			= g->onEvent;
		stack->wtpState			= g->wtpState;
        	stack->tid			= g->tid;
        	stack->old_tid			= g->old_tid;
		//MemMove(&stack->WapLastUrl, &g->WapLastUrl, MAX_URL_LEN + 1);
		MemMove(&stack->WapLastUrl, &g->WapLastUrl, sizeof(g->WapLastUrl));
		MemMove(&stack->CookieLastUrl, &g->CookieLastUrl, sizeof(g->CookieLastUrl));
		stack->xhtmlOutputFileRef	= g->xhtmlOutputFileRef;
		stack->srcHandle		= g->srcHandle;

		//
		//MemHandle       	ConnectionsHandle;
		//MemHandle       	ConnectionsListArrayH;
		//

		MemMove(&stack->RmtAddress, &g->RmtAddress, sizeof(struct sockaddr_in));
		MemMove(&stack->LclAddress, &g->LclAddress, sizeof(struct sockaddr_in));
		WspFreePdu(&stack->webData);
		MemMove(&stack->webData, &g->webData, sizeof(WebDataType));

		//
		//BrowserGlobalsType	browserGlobals;
		//BrowserAPIType		BrowserAPI;
		//

		MemMove(&stack->offline, &g->offline, sizeof(OfflineType));
		MemMove(&stack->xhtml, &g->xhtml, sizeof(XHTML));
		if (stack->httpParser.saveFileName)
			Free(stack->httpParser.saveFileName);
    		if (stack->httpParser.url)
			Free(stack->httpParser.url);
		MemMove(&stack->httpParser, &g->httpParser, sizeof(HTTPParse));
		MemMove(&stack->cache, &g->cache, sizeof(CacheType));
		stack->Align			= g->Align;
		stack->imgAlign			= g->imgAlign;
		MemMove(&stack->DbUrlCurrentURL, &g->DbUrlCurrentURL, sizeof(dbUrlURL));
		wbxml_free_wmldck(&stack->wmldck);
		MemMove(&stack->wmldck, &g->wmldck, sizeof(WMLDeck));
		stack->imageThreadRef		= g->imageThreadRef;
		stack->imageThreadLoadCount	= g->imageThreadLoadCount;
		stack->tid_new			= g->tid_new;
		stack->Render			= g->Render;
		stack->engineStop		= g->engineStop;
		stack->userCancel		= g->userCancel;
		stack->httpUseTempFile		= g->httpUseTempFile;
		stack->hasEvent			= g->hasEvent;
		stack->retransmission		= g->retransmission;
		stack->wasPost			= g->wasPost;
		stack->isImage			= g->isImage;
		stack->onenterback		= g->onenterback;
		stack->refreshEvent		= g->refreshEvent;
		stack->post			= g->post;
		stack->wmlsBrowserContextHandled = g->wmlsBrowserContextHandled;

MyErrorFunc("bottom",NULL);
	}
	
	return true;
}*/

