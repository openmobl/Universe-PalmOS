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
#include    	<SysEvtMgr.h>
#include	<SystemResources.h>
#include 	"WAPUniverse.h"
#include    	"PalmUtils.h"
#include	"build.h"
#include 	"../res/WAPUniverse_res.h"
#include    	"formUrlList.h"
#include    	"formConnectionList.h"
#include    	"formEditConnection.h"
#include    	"formEditUrl.h"
#include	"formBrowser.h"
#include    	"dbConn.h"
#include    	"frmScroll.h"
#include 	<NetMgr.h>
#include 	"wml.h"
#include    	"history.h"
#include    	"frmUrl.h"
#include    	"MenuHnd.h"

#ifdef __GNUC__
#include "callbacks.h"
#endif

#define appFileCreator            'Wapu'
#define appVersionNum              0x01
#define appPrefID                  0x00
#define appPrefVersionNum          0x01

#define			UrlDefaultPrefix	     "http://"
#define			UrlDefaultPrefixS	     "https://"
static	char*	UrlDefaultPrefixP      = UrlDefaultPrefix;
//#define			UrlDefaultPrefixCheckL	 4
#define			UrlDefaultPrefixCheckL	 7
static	char*	UrlDefaultPrefixSP      = UrlDefaultPrefixS;
#define			UrlDefaultPrefixSCheckL	 8
#define			UrlDefaultPrefixL	     sizeof(UrlDefaultPrefix) - 1	// -1 for zero ending
#define			UrlDefaultPrefix2	     "www."
static	char*	UrlDefaultPrefix2P      = UrlDefaultPrefix2;
#define			UrlDefaultPrefix2L	     sizeof(UrlDefaultPrefix2) - 1	// -1 for zero ending
#define			UrlDefaultPrefix3	     ".com"
static	char*	UrlDefaultPrefix3P      = UrlDefaultPrefix3;
#define			UrlDefaultPrefix3L	     sizeof(UrlDefaultPrefix3) - 1	// -1 for zero ending
#define			UrlDefaultPrefix4	     ".org"
static	char*	UrlDefaultPrefix4P      = UrlDefaultPrefix4;
#define			UrlDefaultPrefix4L	     sizeof(UrlDefaultPrefix4) - 1	// -1 for zero ending
#define			UrlDefaultPrefix5	     "wap"
static	char*	UrlDefaultPrefix5P      = UrlDefaultPrefix5;
#define			UrlDefaultPrefix5L	     sizeof(UrlDefaultPrefix5) - 1	// -1 for zero ending
#define			UrlDefaultPrefix6	     "/"
static	char*	UrlDefaultPrefix6P      = UrlDefaultPrefix6;
#define			UrlDefaultPrefix6L	     sizeof(UrlDefaultPrefix6) - 1	// -1 for zero ending
#define			UrlDefaultPrefix7	     "index"
static	char*	UrlDefaultPrefix7P      = UrlDefaultPrefix7;
#define			UrlDefaultPrefix7L	     sizeof(UrlDefaultPrefix7) - 1	// -1 for zero ending
#define			UrlDefaultPrefix8	     ".wml"
static	char*	UrlDefaultPrefix8P      = UrlDefaultPrefix8;
#define			UrlDefaultPrefix8L	     sizeof(UrlDefaultPrefix8) - 1	// -1 for zero ending
#define			UrlDefaultPrefix9	     ".net"
static	char*	UrlDefaultPrefix9P      = UrlDefaultPrefix9;
#define			UrlDefaultPrefix9L	     sizeof(UrlDefaultPrefix9) - 1	// -1 for zero ending
#define			UrlDefaultPrefix10	     ".edu"
static	char*	UrlDefaultPrefix10P      = UrlDefaultPrefix10;
#define			UrlDefaultPrefix10L	     sizeof(UrlDefaultPrefix10) - 1	// -1 for zero ending
#define			UrlDefaultPrefix11	     ".co.jp"
static	char*	UrlDefaultPrefix11P      = UrlDefaultPrefix11;
#define			UrlDefaultPrefix11L	     sizeof(UrlDefaultPrefix11) - 1	// -1 for zero ending
#define			UrlDefaultPrefix12	     ".uk"
static	char*	UrlDefaultPrefix12P      = UrlDefaultPrefix12;
#define			UrlDefaultPrefix12L	     sizeof(UrlDefaultPrefix12) - 1	// -1 for zero ending
#define			UrlDefaultPrefix13	     ".de"
static	char*	UrlDefaultPrefix13P      = UrlDefaultPrefix13;
#define			UrlDefaultPrefix13L	     sizeof(UrlDefaultPrefix13) - 1	// -1 for zero ending
#define			UrlDefaultPrefix14	     "http://"
static	char*	UrlDefaultPrefix14P      = UrlDefaultPrefix14;
#define			UrlDefaultPrefix14L	     sizeof(UrlDefaultPrefix14) - 1	// -1 for zero ending
#define			UrlDefaultPrefix15	     "https://"
static	char*	UrlDefaultPrefix15P      = UrlDefaultPrefix15;
#define			UrlDefaultPrefix15L	     sizeof(UrlDefaultPrefix15) - 1	// -1 for zero ending


Boolean ehFormUrl(EventPtr event)
{
    Boolean 		handled = false;
    FormPtr         	frm;
    GlobalsType 	*g;
    Char*           	urlP;
    FieldPtr		urlFldP;
    Int16 		selection;	
    
    
	urlFldP = GetObjectPtr(fldUrl);
	urlP    = (Char*) FldGetTextPtr(urlFldP);

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
    	switch (event->eType) {
        case frmOpenEvent:
            frm = FrmGetActiveForm();
            FrmDrawForm(frm);
            FrmSetFocus(frm, FrmGetObjectIndex(frm, fldUrl));
            //WinDrawLine (116, 115, 153, 115);
            //WinDrawLine (153, 116, 153, 128);
            //WinDrawLine (116, 128, 153, 128);
            handled = true;
            break;
            
	case frmUpdateEvent:
	    FrmDrawForm(FrmGetActiveForm());
	    handled = true;
	    break;

        case frmCloseEvent:
			handled = true;
			break;
		case menuEvent:
           mhMenuEdit(event->data.menu.itemID);
    	   break;
	case keyDownEvent:
	   switch (event->data.keyDown.chr) {
		case chrLineFeed:
			CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonUrlOpen)));
			handled = true;
			break;
	   }
	   break;
        case ctlSelectEvent:
		switch (event->data.ctlSelect.controlID)
		{
			case triggerMore:
				selection = LstPopupList(GetObjectPtr (lstMore));
				if ( selection == 0 )
                			FldInsert( urlFldP, UrlDefaultPrefix14P, UrlDefaultPrefix14L );
            			else if ( selection == 1 )
                			FldInsert( urlFldP, UrlDefaultPrefix15P, UrlDefaultPrefix15L );
				else if ( selection == 2 )
                			FldInsert( urlFldP, UrlDefaultPrefix7P, UrlDefaultPrefix7L );
            			else if ( selection == 3 )
                			FldInsert( urlFldP, UrlDefaultPrefix8P, UrlDefaultPrefix8L );
                		else if ( selection == 4 )
                			FldInsert( urlFldP, UrlDefaultPrefix9P, UrlDefaultPrefix9L );
                		else if ( selection == 5 )
                			FldInsert( urlFldP, UrlDefaultPrefix10P, UrlDefaultPrefix10L );
                		else if ( selection == 6 )
                			FldInsert( urlFldP, UrlDefaultPrefix11P, UrlDefaultPrefix11L );
                		else if ( selection == 7 )
                			FldInsert( urlFldP, UrlDefaultPrefix12P, UrlDefaultPrefix12L );
                		else if ( selection == 8 )
                			FldInsert( urlFldP, UrlDefaultPrefix13P, UrlDefaultPrefix13L );
				handled = true;
				break;
			case pushWww:
				FldInsert( urlFldP, UrlDefaultPrefix2P, UrlDefaultPrefix2L );
				CtlSetValue (GetObjectPtr(pushWww), 0);
				handled = true;
				break;
			case pushCom:
				FldInsert( urlFldP, UrlDefaultPrefix3P, UrlDefaultPrefix3L );
				CtlSetValue (GetObjectPtr(pushCom), 0);
				handled = true;
				break;
			case pushOrg:
				FldInsert( urlFldP, UrlDefaultPrefix4P, UrlDefaultPrefix4L );
				CtlSetValue (GetObjectPtr(pushOrg), 0);
				handled = true;
				break;
			case pushWap:
				FldInsert( urlFldP, UrlDefaultPrefix5P, UrlDefaultPrefix5L );
				CtlSetValue (GetObjectPtr(pushWap), 0);
				handled = true;
				break;
			case pushSlash:
				FldInsert( urlFldP, UrlDefaultPrefix6P, UrlDefaultPrefix6L );
				CtlSetValue (GetObjectPtr(pushSlash), 0);
				handled = true;
				break;
			
			case buttonUrlOpen:
				urlFldP = GetObjectPtr(fldUrl);
		    		urlP    = FldGetTextPtr(urlFldP);
				// Should it be a caseless compare??
		    		if (urlP && (FldGetTextLength(urlFldP) > 1) 
					 && (StrNCaselessCompare( urlP, UrlDefaultPrefixP, UrlDefaultPrefixCheckL ) != 0)
					 && (StrNCaselessCompare( urlP, UrlDefaultPrefixSP, UrlDefaultPrefixSCheckL ) != 0)
					 && (StrNCaselessCompare( urlP, "wtai://", 7) != 0)
					 && (StrNCaselessCompare( urlP, "tel:", 4) != 0)
					 && (StrNCaselessCompare( urlP, "mailto:", 7) != 0)
					 && (StrNCaselessCompare( urlP, "device:", 7) != 0)
					 && (StrNCaselessCompare( urlP, "internal:", 9) != 0)
					 && (StrNCaselessCompare( urlP, "about:", 6) != 0)
					 && (StrNCaselessCompare( urlP, "proxy:", 6) != 0)
					 && (StrNCaselessCompare( urlP, "mtch:", 5) != 0)
					 && (StrNCaselessCompare( urlP, "file:", 5) != 0) ) {
										    
						FldSetInsPtPosition( urlFldP, 0 );
						FldInsert( urlFldP, UrlDefaultPrefixP, UrlDefaultPrefixL );
						FldReleaseFocus( urlFldP );
				}
				if (urlP && (FldGetTextLength(urlFldP) > 1) &&
					(  (StrNCompare( urlP, "wtai://", 7) == 0)
					|| (StrNCompare( urlP, "tel:", 4) == 0)
					|| (StrNCompare( urlP, "mailto:", 7) == 0) ) ) {
						followLink(urlP, g);
						handled = true;
						break;
				}
				urlP    = FldGetTextPtr(urlFldP);
				if(FldGetTextLength(urlFldP) > 1) {
					if (g->OpenUrl && g->OpenUrl != NULL)
			        		Free(g->OpenUrl);
				        g->OpenUrl = Malloc(StrLen(urlP)+1);
				        ErrFatalDisplayIf (!g->OpenUrl, "Malloc Failed");
				    	if (!urlP) {
						MyErrorFunc ("Could not open the URL. No URL!\n", "Open URL"); 
						return true;
					} else {
					    	StrCopy(g->OpenUrl, urlP);
						FrmReturnToForm(frmBrowser);
						FrmUpdateForm(frmBrowser, frmUpdateGotoUrl);
					}
				} else {
			  		MyErrorFunc ("No URL. If you want to close the URL dialog without loading a URL please click the Cancel button.\n", NULL);
				}
				handled = true;
				break;
			case buttonUrlCancel:
				FrmReturnToForm(frmBrowser);
				handled = true;
				break;
		}
		break;
		
        
        default:
            break;
        }
       
    return handled;
}