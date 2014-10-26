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
#include 	"WAPUniverse.h"
#include    	"dbConn.h"
#include 	"../res/WAPUniverse_res.h"
#include    	"formBrowser.h"
#include    	"PalmUtils.h"
#include	"build.h"
#include	"wml.h"
#include	"Form.h"
#include	"formEditUrl.h"
#include	"Tab.h"
#include	"Cookie.h"
#include	"Cache.h"
#include	"Net.h"
#include	"debug.h"
#include    "history.h"

/**************************************/
/** Menu Handler Functions           **/
/**************************************/

void PageInfo (char* error, char* additional)
{

    FrmCustomAlert (infPageInf, error, additional ? additional : "", NULL);
}

int ShowURL (char* error, char* additional, char* last)
{

    return (FrmCustomAlert (infURL, error, additional ? additional : "", last ? last : ""));
}

int PrivateBrowsingState(char* state)
{

    return (FrmCustomAlert(alPrivateBrowsing, state, "", ""));
}

static void initRedraw(GlobalsType *g)
{
		FormInputDeactivate(g);
		if (g->pageContentType && 
		    (StrNCaselessCompare(g->pageContentType,"application/vnd.wap.wmlc",24)==0)) {
			palmResetState(g);
			g->state=BS_RENDER;
			g->refreshEvent = true;
		} else if (g->pageContentType && 
			   ((StrNCaselessCompare(g->pageContentType,"application/vnd.wap.xhtml+xml",29)==0) || 
			    (StrNCaselessCompare(g->pageContentType,"application/xhtml+xml",21)==0) || 
			    (StrNCaselessCompare(g->pageContentType,"application/wml+xml",19)==0) ||
			    (StrNCaselessCompare(g->pageContentType,"text/html",9)==0) ||
			    (StrNCaselessCompare(g->pageContentType,"text/vnd.wap.wml",16)==0))) {
				palmResetState(g);
            			g->state = BS_XHTML_PARSE_RENDER;
				g->refreshEvent = true;
            	} else if (g->pageContentType && (StrNCaselessCompare(g->pageContentType,"application/rss+xml",19)==0)) {
				palmResetState(g);
            			g->state = BS_RSS_PARSE_RENDER;
		}
}

static void LoadNetworkPanel()
{
    // from    http://oasis.palm.com/devzone/knowledgebasearticle.cfm?article_id=1204

	/*
    static UInt32 gPanelCreators[8] = {
    sysFileCButtons,
    sysFileCDigitizer,
    sysFileCFormats,
    sysFileCGeneral,
    sysFileCModemPanel,
    sysFileCNetworkPanel,
    sysFileCOwner,
    sysFileCShortCuts,
    };
    */
	
    LocalID theDBID;
    UInt16 theCardNo;
    DmSearchStateType theSearchState;

    // Grab the id of the panel we want to launch
    DmGetNextDatabaseByTypeCreator(true, &theSearchState, sysFileTPanel, sysFileCNetworkPanel,
            true, &theCardNo, &theDBID);

    // Launch the panel
    SysUIAppSwitch(theCardNo, theDBID, sysAppLaunchCmdPanelCalledFromApp, NULL );

}


/****************************************************/
/* 
   Function: Int16 MenuOptions_MH(UInt16 menuID);
   Description: Menu Handler for MenuOptions 
 */
/****************************************************/

Int16 mhMenuOptions(UInt16 menuID, GlobalsType *g)
{
FormType *frmP;

    switch (menuID) {

      case OptionsGetInfo:
          //DisplayAbout();
          FrmPopupForm(frmInfo);
          break;

      //case OptionLicense:
      //    DisplayLicense();
      //    break;

      case OptionConnectionSettings:
          SwitchForm(frmConnectionList,g);
          break;

      case OptionBookmarks:
	 if(dbConnNumRecords(g)>0){
                SwitchForm(frmUrlList,g); 					
	 } else{
		FrmCustomAlert(alError, "You need to define at least one connection\n", "", NULL);
         }
         break;
		  
    case OptionNetwork:
        LoadNetworkPanel();
        break;
        
     case OptionsHelp:
     	frmP = FrmInitForm (frmHelp);
        FrmDoDialog (frmP);                    
        FrmDeleteForm (frmP);
        break;
     
     case OptionsBuild:
      	  FrmCustomAlert (infBuild, BUILDSTR, VERSIONSTR, NULL);
      	  break;
      
      case OptionsPrefs:
      	FrmPopupForm(frmPrefs);
      	break;

      case OptionsDonate:
	FrmHelp(strDonate);
	break;

      default:
	  break;
     }
     return(0);
}

/****************************************************/
/* 
   Function: Int16 MenuOptions_MH(UInt16 menuID);
   Description: Menu Handler for MenuOptions 
 */
/****************************************************/

Int16 mhMenuPush(UInt16 menuID, GlobalsType *g)
{

	switch (menuID) {
		case PushPushHelp:
		//	DisplayPushHelp();
			break;

		default:
			break;
	}
	return(0);
}


/****************************************************/
/* 
   Function: Int16 MenuView_MH(UInt16 menuID);
   Description: Menu Handler for MenuView 
 */
/****************************************************/

Boolean mhMenuView(UInt16 menuID, GlobalsType *g)
{
	Boolean 	status = false;
	FormType 	*frmP;
	UInt16		count = 0;
	Err		err = errNone;
	
    	switch (menuID) {

      		case miViewSource:
			FormInputDeactivate(g);
      	  		MenuEraseStatus( NULL );
	      		status = browserSrcEvt(g);
          		break;

	  	case miReload:
			FormInputDeactivate(g);
	  	  	status = browserReloadEvt(g);
          		break;
          
      		case miDis:
			FormInputDeactivate(g);
    			if (/*(g->conn.connectionType == 'O') && */
				(g->wtpState == WTP_Connected)) { //clean up our WTP session
				WSPDisconnect(g->sock, g);
    			}
		  	NetCloseSocket(&g->sock);
		  	//if(NetLibConnected() == true) {
				do {
	    				err = NetLibClose(AppNetRefnum, true);
					NetLibOpenCount(AppNetRefnum, &count);
				} while ((count > 0) && (err == netErrStillOpen));
   	      			if (!NetLibConnected())
					g->NetLibOpen = false; 
			//}
          		break;

	  	case miBack:
			FormInputDeactivate(g);
	      		status = browserBackEvt(g);
          		break;

      		case miStop:
			FormInputDeactivate(g);
	      		status = browserStopEvt(g);
	  	  	break;
		  
      		case miConnectionSettings:
			FormInputDeactivate(g);
			g->prevFormID = frmBrowser;
          		FrmPopupForm(frmConnectionList); //SwitchForm(frmConnectionList,g);
          		break;

      		case miBookmarks:
			FormInputDeactivate(g);
	      		status = browserURLsEvt(g);
          		break;
		  
      		case miNetwork:
			FormInputDeactivate(g);
        		LoadNetworkPanel();
        		break;

      		case miGetInfo:
			FormInputDeactivate(g);
		  	//DisplayAbout();
		  	FrmPopupForm(frmInfo);
          		status = true;
          		break;

      		//case miLicense:
			//FormInputDeactivate(g);
          	//	DisplayLicense();
          	//	status = true;
          	//	break;
      	  
      		case miBuild:
			FormInputDeactivate(g);
      	  		FrmCustomAlert (infBuild, BUILDSTR, VERSIONSTR, NULL);
      	  		break;
      	  
      		case miURL:
			FormInputDeactivate(g);
      	  		switch (ShowURL (g->Url, NULL, NULL)){
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
      	 		break;
      	  
      		case miConnect:
			FormInputDeactivate(g);
      	  		//if(NetLibConnected() == false) {
	    			NetOpenNetwork(g);
   	      			if(NetLibConnected() == true) g->NetLibOpen = true; 
			//}
      	  		break;
      	  
      		case miExit:
			FormInputDeactivate(g);
          		//SysAppLauncherDialog();
          		ExitApp();
      	  		break;
      	  
      		case miHistory:
      			FrmPopupForm(frmHistory);
      			break;
      	
     		case miPush:
			FormInputDeactivate(g);
      			FrmPopupForm(frmPushInbox);
      			break;
      
      		case miPrefs:
			FormInputDeactivate(g);
      			FrmPopupForm(frmPrefs);
      			break;
      	
      		case miMarkSite:
			FormInputDeactivate(g);
      			addBookmark (g);
      			break;

      		case miHelp:
			FormInputDeactivate(g);
			frmP = FrmInitForm (frmHelp);
        		FrmDoDialog (frmP);                    
        		FrmDeleteForm (frmP);
			break;

      		case miDonate:
			FormInputDeactivate(g);
			FrmHelp(strDonate);
			break;

		case miPrivate:
			FrmPopupForm(frmBrowserPrivacy);
			break;

		case miPrivateToggle:
			switch (PrivateBrowsingState((g->privateBrowsing == true) ? "Enabled":"Disabled")) {
				case 0: // Enable
					g->privateBrowsing = true;
					break;
				case 1: // Disable
                    if (g->privateBrowsing) { // do not accidentally clear
                        historyClear(g);
                    }
					CookieCleanPrivateCookies(&g->CookieJar, g);
					CacheClearPrivateFiles(g);
					g->privateBrowsing = false;
					break;
				default:
					break;
			}
			break;

		case miRenderSmallScreen:
			g_prefs.renderMode = RENDER_SMALL_SCREEN;
			initRedraw(g);
			break;

		case miRenderWideScreen:
			g_prefs.renderMode = RENDER_WIDE_SCREEN;
			initRedraw(g);
			break;

		case miOpenUrl:
			FrmPopupForm(frmUrl);
			break;

		case miNewTab:
			TabCreateTab("about:blank", /* TODO: Make preference*/ true, g);
			break;

		case miCloseTab:
			if (g->state != BS_IDLE) {
				browserStopEvt(g);
			}
			TabDeleteTab(g->tabList.currentTab, true, g);
			break;

		case miGotoHome:
			g->post = false;//if we don't do this things get messy

			g->DbConnCurrentRecord = g->ActiveConnStore;
	
			dbConnReadCurrentConnection(g);
			followLink(g->CurrentConnection.home, g);
			break;

		case miLoadInfo:
			DebugShowLoadInfo(g);
			break;

		case miPostData:
			DebugShowData("Post Data", g->postBuf);
			break;

        case miMemUsage:
            DebugShowMemUsage(g);
            break;
            
        case miPageGlobals:
            DebugShowGlobals(g);
            break;
      	  
        default:
	  		break;
     	}
     	return(status);
}



/****************************************************/
/* 
   Function: Int16 MenuEdit_MH(UInt16 menuID);
   Description: Menu Handler for MenuEdit 
 */
/****************************************************/

Int16 mhMenuEdit(UInt16 menuID)
{
  FieldPtr        fld;
 
    switch (menuID) {

    case EditEditCut:       // Do a cut to the clipboard if text is
                                // highlighted.
      fld = GetFocusObjectPtr ();
      if (fld)
        FldCut (fld);
      break;

    case EditEditCopy:          // Copy the highlighted text to the
                                // clipboard.
      fld = GetFocusObjectPtr ();
      if (fld)
        FldCopy (fld);
      break;

    case EditEditPaste:     // Do a paste from the clipboard.
      fld = GetFocusObjectPtr ();
      if (fld)
        FldPaste (fld);
      break;

    case EditEditUndo:          // undo the last text change.
      fld = GetFocusObjectPtr ();
      if (fld)
        FldUndo (fld);
      break;

    case EditEditKeyboard:      // display the on screen keyboard
      SysKeyboardDialog (kbdAlpha);
      break;

    case EditEditGraffiti:      // Display the graffiti reference screen
      SysGraffitiReferenceDialog (referenceDefault);
      break;

    default:
	  break;
   }
     return(0);
}



