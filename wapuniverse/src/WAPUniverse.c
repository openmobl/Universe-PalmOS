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
#include	<PalmOS.h>
#include	<SysEvtMgr.h>
#include 	<DLServer.h>
#include 	<PalmCompatibility.h>
#include	<TelephonyMgr.h>
#include 	<TelephonyMgrTypes.h>
#include	<Window.h>
#include	<Helper.h>
#include	<HelperServiceClass.h>
#include	<PalmUtils.h>

#ifdef HAVE_PALMONE_SDK
#include	<common/system/palmOneNavigator.h>
#include	<common/system/palmOneChars.h>
#include 	<KeyMgr.h>
#include 	<68K/Hs.h>
#include 	<68K/Libraries/PmKeyLib/PmKeyLib.h>
#endif

#include 	"WAPUniverse.h"
#include	"build.h"
#include 	"../res/WAPUniverse_res.h"
#include	"formUrlList.h"
#include	"formConnectionList.h"
#include	"formEditConnection.h"
#include	"formEditUrl.h"
#include	"formBrowser.h"
#include	"frmScroll.h"
#include	"frmUrl.h"
#include	"wml.h"
#include	"about.h"
#include	"pnoJpeg.h"
#include	"frmPrefs.h"
#include	"http.h"
#include	"history.h"
#include	"image.h"
#include	"Cache.h"
#include	"DIA.h"
#include	"resize.h"
#include	"frmHistory.h"
#include	"PalmUtils.h"
#include	"BrowserAPI.h"
#include	"Cookie.h"
#include	"Form.h"
#include	"Layout.h"
#include	"Font.h"
#include	"palmpng.h"
#include	"debug.h"
#include	"FiveWay.h"
#include	"GIFLib.h"

static UInt16 RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags);
static Boolean ApplicationHandleEvent ( EventPtr event );
static int StartApplication(UInt16 cmd, void *cmdPBP); // StartApplication(void);
static void EventLoop(void);
static void StopApplication(void);


// Following globals are needed if you do not like crashes in the network lib
Int32            		AppNetTimeout;
UInt16           		AppNetRefnum; // used by SysLibFind
Err 			 	errno; 	   // Berkely socketinterface 
WAPPreferenceType 		g_prefs;
UInt16 				jpegLibRef;
UInt16 				gScriptRef;
UInt16				gGifRef;
UInt16 				HttpLibRef;
ScriptHost  			scriptHost;    // scripting engine host API
Boolean     			scriptWarned;

// globals for internal use
//DmOpenRef    		gDbUrlDb;          // handle for URL database (bookmarks)
//DmOpenRef    		gDbConnDb;          // handle for Connection database 
//UInt16       		gCurrentView;    // id of current form


char *ErrMsg(Err errno)
{
    static char msgstr[32];

    if (errno != 0)
        SysErrString(errno, msgstr, 32);
    else
        msgstr[0] = '\0';

    return(msgstr);
}

static Boolean SplashFormHandleEvent(EventType * eventP)
{
    Boolean handled = false;
    FormType * frmP;
    GlobalsType *g;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    if ( ResizeHandleEvent( eventP ) )
        return true;

    switch (eventP->eType) 
        {
       
        case frmOpenEvent:
            frmP = FrmGetActiveForm();
            FrmDrawForm(frmP);

/*#ifdef IS_RELEASE_CANDIDATE
	    palmSmall(true);
	    WinDrawChars("Release Candidate "RCVERSIONSTR,StrLen("Release Candidate "RCVERSIONSTR),5,70);
	    WinDrawChars(BUILDSTR,StrLen(BUILDSTR),5,83);
	    palmSmall(false);
#endif*/

            SysTaskDelay(/*150*/ 80);
	    if ((g->launchURL != NULL) && (dbConnNumRecords(g) > 0)) {
		g->CurrentView = frmBrowser;
	    } else if (dbConnNumRecords(g) > 0) {
		if (g->nextView)
			g->CurrentView = g->nextView;
		else
	   		g->CurrentView = frmUrlList;
	    } else {
	   	g->CurrentView = frmConnectionList;
	    }
	    g->nextView = 0;
     	    FrmGotoForm (g->CurrentView);
            handled = true;
            break;
            
        case frmUpdateEvent:
        
        default:
            break;
        }

    return handled;
}

UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
	int error;
	//UInt32 romVersion;

	error = RomVersionCompatible (version50, launchFlags);
    	if (error)
       		return error;

	switch (cmd) {
		case sysAppLaunchCmdNormalLaunch:
			error = StartApplication(cmd, cmdPBP);	// Application start code
			if (error)
				return error;
			EventLoop();				// Event loop
			StopApplication();			// Application stop code
			break;
		case sysAppLaunchCmdGoToURL:
		case sysAppLaunchCmdGotoHomepage:
		case sysAppLaunchCmdGotoBlank:
		case sysAppLaunchCmdViewBookmarks:
		case sysAppLaunchCmdViewConnSet:
			error = StartApplication(cmd, cmdPBP);	// Application start code
			if (error)
				return error;
			EventLoop();				// Event loop
			StopApplication();			// Application stop code
			break;
        case sysAppLaunchCmdSyncNotify:
        case sysAppLaunchCmdSystemReset:
            // TODO: Add provisioning support
            /*ExgRegisterDatatype(wuAppType, exgRegTypeID,
                        "application/x-wap-prov.browser-bookmarks\tapplication/x-wap-prov.browser-settings",
                        "WAP Browser bookmark\tWAP Browser connection setting", 0);*/
            /*ExgSetDefaultApplication(wuAppType, exgRegTypeID,
                        "application/x-wap-prov.browser-bookmarks\tapplication/x-wap-prov.browser-settings");*/
            ExgRegisterDatatype(wuAppType, exgRegSchemeID, "http\thttps\tfile",
                                    "Hypertext Transfer Protocol\tHypertext Transfer Protocol Secure\tInternal file", 0);
            break;
		case sysAppLaunchCmdNotify:
			HandleResizeNotification(((SysNotifyParamType *)cmdPBP )->notifyType);
			break;
		default:
			break;
	}

	return 0;
}		

gWAPAppInfoType gWAPAppInfo =
{
  // Renamed categories
  0x0008,                   // set the 2nd and 3rd bits indicating those
                                // categories as changed.
  {
    "Unfiled",
    "Business",
    "Personal",
    "Search", 
    "Communications",
    "Finance",
    "News",
    "Portals",
    "Shopping",
    "Travel",
    "Weather",
    "", "", "", ""
  },
    // Category Uniq IDs
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    // Last Uniq ID
  15,
    // Reserved
  0, 0
};


void SwitchForm(UInt16 formID, GlobalsType *g)
{
	FrmGotoForm(formID);
	g->CurrentView = formID;
}

/*void DisplayLicense()
{
MemHandle gplStr;
	
	gplStr = DmGetResource (strRsc, strGPL);
	ScrollFrm ("WAPUniverse Copyright", gplStr);
	
}

void DisplayPushHelp()
{
MemHandle pushStr;
	
	pushStr = DmGetResource (strRsc, strPush);
	ScrollFrm ("Push Inbox Help", pushStr);
	
}*/

Boolean PrvDialListCanBePhoneNumber( Char* text, Int16 textLen )
{
	UInt16 offset = 0;

	while (offset < textLen)
	{
		WChar curChar;
		offset += TxtGetNextChar(text, offset, &curChar);
		if ( StrChr(gPhoneChars, curChar) != NULL )
			return true;
	}

	return false;
}

void PrvDialListPhoneNumberFilter( Char* outString, Int16* outLen, const Char* inString, Int16 inLen )
{
	UInt16 inOffset;
	UInt16 outOffset;
	Boolean fLastWasSpace;

	inOffset = 0;
	outOffset = 0;
	fLastWasSpace = false;

	while ( (inOffset < inLen) && (outOffset < *outLen) )
	{
		WChar curChar;

		inOffset += TxtGetNextChar(inString, inOffset, &curChar);
		if (StrChr(gPhoneChars, curChar))
		{
			// Only + at the beginning
			if ( (curChar == chrPlusSign) && (outOffset > 0) )
			{
				outOffset += TxtSetNextChar(outString, outOffset, chrSpace);
				fLastWasSpace = true;
			}
			else
			{
				outOffset += TxtSetNextChar(outString, outOffset, curChar);
				fLastWasSpace = false;
			}
		}
		else if ( !fLastWasSpace && (outOffset > 0) )	// No space at the beginning
		{
			outOffset += TxtSetNextChar(outString, outOffset, chrSpace);
			fLastWasSpace = true;
		}
	}

	// No space at the end
	if (fLastWasSpace)
		outOffset--;

	TxtSetNextChar(outString, outOffset, chrNull);

	*outLen = outOffset;
}

/***********************************************************************
 *
 * FUNCTION:    PrvBroadcastHelperAction
 *
 * DESCRIPTION: Send the Helper Enumeration code.
 *
 * PARAMETERS:  details - structure containing the body of the
 *							broadcast.
 *
 * RETURNED:    Boolean
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean PrvBroadcastHelperAction ( void * details )
{
	SysNotifyParamType note;
	Err err = 0;

	MemSet(&note, sizeof(note), 0);

	note.notifyType = sysNotifyHelperEvent;
	note.notifyDetailsP = details;
	note.handled = false;

	err = SysNotifyBroadcast(&note);
	if (err) {
		Char buffer[32];
		StrIToH(buffer, err);
		FrmCustomAlert(alError, buffer, NULL, NULL);
	}

	return note.handled;
}



/***********************************************************************
 *
 * FUNCTION:    PrvInvokeHelperService
 *
 * DESCRIPTION: Invoke the helper service.
 *
 * PARAMETERS:  serviceClassID - the helper service class ID 
 *				dataP - pointer to data for the dataP field in the
 *							HelperNotifyEventType structure.
 *				displayedNameP - a pointer to the name.
 *				detailsP - a pointer to 'details' data for the
 *							'detailsP' field in the HelperNotifyEventType
 *							structure.
 *
 * RETURNED:    Err
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err
PrvInvokeHelperService( UInt32 serviceClassID, Char *dataP, Char *displayedNameP, void *detailsP )
{
	Err err = 0;
	HelperNotifyExecuteType execute;
	HelperNotifyEventType event;
	Boolean handled = false;

	{
		MemSet(&event, sizeof(event), 0);
		MemSet(&execute, sizeof(execute), 0);
	
		execute.dataP = dataP;
		execute.detailsP = detailsP;
		execute.displayedName = displayedNameP;
		execute.helperAppID = 0;					// All helpers.
		execute.serviceClassID = serviceClassID;
		execute.err = 0;

		event.version = kHelperNotifyCurrentVersion;
		event.actionCode = kHelperNotifyActionCodeExecute;
		event.data.executeP = &execute;
	
		handled = PrvBroadcastHelperAction(&event);
		if (!handled) {
			FrmCustomAlert(alError, "Helper service did not handle the notification.", "Execute", NULL);
		}
		/*else if (event.data.executeP->err) {
			Char buffer[32];
			StrIToH(buffer, err);
			FrmCustomAlert(alError, "\nError in executing helper service.\n", buffer, NULL);
			//errno = err;
			//FrmCustomAlert(alError, "\nError in executing helper service.", ErrMsg(err), NULL);
		}*/
	}

	return err;
}

/***********************************************************************
 *
 * FUNCTION:    PrvValidateHelperService
 *
 * DESCRIPTION: Validate the helper service.
 *
 * PARAMETERS:  serviceClassID - the helper service class ID 
 *
 * RETURNED:    handled (true: helper service exists, false otherwise)
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err
PrvValidateHelperService(UInt32 serviceClassID)
{
    HelperNotifyValidateType	validate;
    HelperNotifyEventType	event;
    Boolean			handled = false;

	{
		MemSet(&event, sizeof(event), 0);
		MemSet(&validate, sizeof(validate), 0);
	
		validate.helperAppID = 0;					// All helpers.
		validate.serviceClassID = serviceClassID;

		event.version = kHelperNotifyCurrentVersion;
		event.actionCode = kHelperNotifyActionCodeValidate;
		event.data.validateP = &validate;
	
		handled = PrvBroadcastHelperAction(&event);
	}

return handled;
}


void DisplayDial(const Char *iDialNumberP, int callType)
{
	Err err;
	UInt16 gTelRefNum;
	TelAppID gTelAppID;
	FormType *form;
	UInt16 buttonHit;
	Boolean open = false;
	UInt16 ioTransIdP;
	ControlPtr      ctl;
	MemHandle	numberH;
	Char*		numberP;
	Int16		len;

	

	form = FrmInitForm(frmDial);

	/*if (PrvDialListCanBePhoneNumber(iDialNumberP, StrLen(iDialNumberP)) != true) {
		//return  MyErrorFunc("\nCould not dial number.\nReason:", "Phone number invalid type");
		FrmHideObject(form,FrmGetObjectIndex(form,buttonDialDial));
	}
	else {*/
		//len = min(len, StrLen(iDialNumberP));
		len = StrLen(iDialNumberP);
		
		numberH = MemHandleNew(len + 1);
		if (!numberH)
			return;

		numberP = MemHandleLock(numberH);
		if (callType != 3) {
			PrvDialListPhoneNumberFilter(numberP, &len, iDialNumberP, StrLen(iDialNumberP));
		}
		if (callType == 3){
			StrCopy(numberP, iDialNumberP);
		}
		numberP[len] = chrNull;
		MemHandleUnlock(numberH);
		MemHandleResize(numberH, len + 1);
		//MemHandleLock(numberH);
	//}

	ctl = (ControlPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,lblPhoneNumber2))));
          CtlSetLabel(ctl,/*iDialNumberP*/numberP);
  	
  	if (callType == 1) {
  		FrmShowObject(form,FrmGetObjectIndex(form,buttonDialMsg));
  	}
  	else if (callType == 2) {
  		ctl = (ControlPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,lblPhoneNumber))));
          CtlSetLabel(ctl,"  DTMF:");
        ctl = (ControlPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,buttonDialDial))));
          CtlSetLabel(ctl,"Send");
  	}
  	else if (callType == 3) {
  		ctl = (ControlPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,lblPhoneNumber))));
          CtlSetLabel(ctl,"Address:");
        ctl = (ControlPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,buttonDialDial))));
          CtlSetLabel(ctl,"Add");
  	}
  
  	buttonHit = FrmDoDialog(form);
  	
  	FrmDeleteForm(form);
  	
  	if (buttonHit == buttonDialDial)
  	{			  

		if (callType == 1) {//User Requested voice call

			if (PrvValidateHelperService(kHelperServiceClassIDVoiceDial)) {
				PrvInvokeHelperService(kHelperServiceClassIDVoiceDial, /*iDialNumberP*/numberP, "Dialing Number....", NULL);
			}
			else {
				//start
				err = SysLibFind(kTelMgrLibName, &gTelRefNum);
				if (err !=0)
				{
				   // The library is not loaded, so load it. 
				   err = SysLibLoad(kTelMgrDatabaseType,
				      kTelMgrDatabaseCreator, &gTelRefNum);
				   if (err) {
				      MyErrorFunc("Could not load the phone library", "load Phone.lib");
				   return;
				   }
				}

				err = TelOpen(gTelRefNum, kTelMgrVersion, &gTelAppID);
				if (err){
					  MyErrorFunc("Could not open the phone library", "open Phone.lib");
				return;
				}
				open = true;
				
				//MyErrorFunc("Could not dial number.\nReason:","Function not implemented");
				err = TelIsSpcServiceAvailable(gTelRefNum, gTelAppID, &ioTransIdP);
				if (err){
					MyErrorFunc("This phone does not support the speech service.", "Check Speech Availability");
				return;
				}
				err = TelIsSpcCallNumberSupported(gTelRefNum, gTelAppID, &ioTransIdP);
				if (err){
					MyErrorFunc("This phone does not support speech calls.", "Check Speech Call Availability");
				return;
				}
					
				err = TelSpcCallNumber(gTelRefNum, gTelAppID, numberP, &ioTransIdP);
				if (err){
					MyErrorFunc("Could not dial the phone number.", "Dial phone number.");
				return;
				}
			}
		}
		else if (callType == 2) {//User requested to send a DTMF
			err = SysLibFind(kTelMgrLibName, &gTelRefNum);
			if (err !=0)
			{
				// The library is not loaded, so load it. 
				err = SysLibLoad(kTelMgrDatabaseType,
				   kTelMgrDatabaseCreator, &gTelRefNum);
				if (err) {
				   MyErrorFunc("Could not load the phone library", "load Phone.lib");
				return;
				}
			}

			err = TelOpen(gTelRefNum, kTelMgrVersion, &gTelAppID);
			if (err){
				MyErrorFunc("Could not open the phone library", "open Phone.lib");
			return;
			}
			open = true;
			
			TelSpcSendBurstDTMF(gTelRefNum, gTelAppID, numberP, &ioTransIdP);
			//MyErrorFunc("Could not send DTMF.\nReason:","Function not implemented");
		}
		else if (callType == 3) {//User Requested to add a Address/Phone Book entry
			MyErrorFunc("Could not add Address/Phone Book entry.\nReason:","Function not implemented");
		}
	}
	
	else if (buttonHit == buttonDialMsg){
		if (PrvValidateHelperService(kHelperServiceClassIDSMS)) {
			PrvInvokeHelperService(kHelperServiceClassIDSMS, /*iDialNumberP*/numberP, NULL, NULL);
		}
		else {
			MyErrorFunc("Could not send SMS.\nReason:","Function not implemented");
		}
	}
	
	else if (buttonHit == buttonDialCancel)
		return;

	if (open == true){
		if (TelClose(gTelRefNum, gTelAppID) != telErrLibStillInUse)
		   SysLibRemove(gTelRefNum);
	}
//end
}


/*void DisplayAbout()
{
	GlobalsType *g;
          ControlPtr      ctl;
          FormPtr         frm;
          
          MenuEraseStatus (0);
          // Load the info form, then display it.
          frm = FrmInitForm (frmInfo);
          ctl = (ControlPtr)(FrmGetObjectPtr(frm,(FrmGetObjectIndex(frm,lblVersion))));
          CtlSetLabel(ctl,VERSIONSTR);
}*/

static void SetDBBackupBit(DmOpenRef dbP)
{
  DmOpenRef localDBP;
  LocalID dbID;
  UInt16 cardNo;
  UInt16 attributes;

  localDBP = dbP;
 
  // now set the backup bit on localDBP
  DmOpenDatabaseInfo(localDBP, &dbID, NULL, NULL, &cardNo, NULL);
  DmDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  attributes |= dmHdrAttrBackup;
  DmSetDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

// This may or may not be neccessary. I am trying to see if it will fix the bug
// that causes a crash when exiting, that sometimes occurs. Untill I solve the
// crash, this will remain as it isn't hurting anything.
/*static Boolean FreeResources(GlobalsType *g)
{
	g->needSpace = 0;
	g->onEvent = 0;
	g->Align = 0;
	g->timeElapse = 0;
	if (g->OpenUrl != NULL) Free(g->OpenUrl);
		g->OpenUrl = NULL;
	if (g->deckTitle != NULL) Free(g->deckTitle);
		g->deckTitle = NULL;
	if (g->postBuf != NULL) Free(g->postBuf);
		g->postBuf = NULL;
	g->post = false;
	if (g->urlBuf != NULL) Free(g->urlBuf);
		g->urlBuf = NULL;
	if (g->contentType != NULL) Free(g->contentType);
		g->contentType = NULL;
	if (g->pageContentType != NULL) Free(g->pageContentType);
		g->pageContentType = NULL;
	if (g->launchURL != NULL) Free(g->launchURL);
		g->launchURL = NULL;
        g->tid = 0;
        g->old_tid = 0;
	g->tid_new = 0;
	g->sid = 0;
	if (g->AuthUser != NULL) Free(g->AuthUser);
		g->AuthUser = NULL;
    	if (g->AuthPass != NULL) Free(g->AuthPass);
		g->AuthPass = NULL;
	if (g->onforward != NULL) Free(g->onforward);
		g->onforward = NULL;
	g->hasEvent = false;
	g->wtpState = 0;
	g->retransmission = false;
	g->wasPost = false;
	g->isImage = false;
	g->onenterback = false;
	g->refreshEvent = false;
	g->imgH = 0;
	g->charSet = 0;
	g->indent = 0;
	g->rightIndent = 0;
	g->contentLength = 0;

	return true;
}*/

static void setUpFiveWay(void)
{
#ifdef HAVE_PALMONE_SDK
	Err error = errNone;
	UInt16 gLibRefNum;

	error = SysLibFind(kPmKeyLibName, &gLibRefNum);
	if (error) {
		error = SysLibLoad(sysFileTLibrary, kPmKeyLibCreator, &gLibRefNum);
		
		if (error) {
			error = errNone;
		}
	}
#endif
}

static void setDisplayDepth(Boolean close) // set close to true on AppStop, false on AppStart
{
	Err 		error;
	UInt32 		depthP;

	if (close == false) { 		// AppStart
		error = WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &depthP, NULL);
		if ((error == errNone) && (depthP & 0x8000)) {
			depthP = 16;
			WinScreenMode(winScreenModeSet, NULL, NULL, &depthP, NULL);
		}
	} else if (close == true) { 	// AppStop
		WinScreenMode(winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);
	}

	return;
}

// This is a somewhat bad hack to get the field select working again. TODO: Fix it
Boolean HandleBrowserControls(EventType *event)
{
	Boolean		handled = false;
	UInt16		activeForm = FrmGetActiveFormID();
	GlobalsType	*g;
	Int16		x;
	Int16		y;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	switch (activeForm) {
		case frmBrowser:
			if (((event->eType == penDownEvent) || (event->eType == penUpEvent) || (event->eType == penMoveEvent)) &&
			    g->InputFldActive) {
				x = event->screenX - browserGetWidgetBounds(WIDGET_TOPLEFT_X);
				y = event->screenY - browserGetWidgetBounds(WIDGET_TOPLEFT_Y) + g->ScrollY;

				if ((x >= g->InputFldActive->x1) && (x <= g->InputFldActive->x2) &&
				    (y >= g->InputFldActive->y1) && (y <= g->InputFldActive->y2)) {
					handled = FldHandleEvent((FieldPtr)GetObjectPtr(fldInput), event);
				} /*else if (!((x >= 0) && (x <= 12) && (y >= 0) && (y <= 12))) {
					deactivateInputFld(g);
				} else {
					deactivateInputFld(g);
				}*/
			}
			break;
	}

	return handled;
}

static Boolean HandleVChrs(EventType* event)
{
    Boolean handled;
    GlobalsType *g;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    handled = false;

    if ((event->data.keyDown.modifiers & poweredOnKeyMask) != 0 &&
        (event->data.keyDown.chr == vchrHard1 ||
         event->data.keyDown.chr == vchrHard2 ||
         event->data.keyDown.chr == vchrHard3 ||
         event->data.keyDown.chr == vchrHard4 ||
	 event->data.keyDown.chr == vchrMenu))
        return handled;

    if (FrmGetActiveFormID() == frmBrowser) {
	switch (event->data.keyDown.chr) {
		case vchrMenu:
			// This will catch the menu key to prevent users from tapping it and having the status bar over-write the first item.
			if ((g->state != BS_NULL) && (g->state != BS_IDLE) && (g->state != BS_ONTIMER) && (g->state != BS_XHTML_ONTIMER))
				handled = true;

			return handled;
			break;
		default:
			handled = false;
			break;
	}
    }

    if ((FrmGetActiveFormID() == frmBrowser) && (g_prefs.mode == MODE_ONEHAND) && (event->eType == keyDownEvent)) {
        switch ( event->data.keyDown.chr ) {
            case vchrHard1:
		if (g->browserGlobals.optMenuOpen == false)
			CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea)));
		else if (g->browserGlobals.optMenuOpen == true)
			CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb)));
		handled = true;
                break;

            case vchrHard2:
		if (g->browserGlobals.optMenuOpen == true)
			CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb)));
		handled = false;
                break;

            case vchrHard3:
		if (g->browserGlobals.optMenuOpen == true)
			CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb)));
		handled = false;
                break;

            case vchrHard4:
		if (g->browserGlobals.optMenuOpen == true) { handled = true; break; }
		if (g->state == BS_IDLE)
			CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwoa)));
		else
			CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwob)));
		handled = true;
                break;

            default:
		//if (g->browserGlobals.optMenuOpen == true)
		//	CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb)));
                handled = false;
                break;
        }
    }
 
    return handled;
}


static int StartApplication(UInt16 cmd, void *cmdPBP)
{
	GlobalsType 	*g;
	FormPtr         frm;
	UInt16          mode;
	UInt16          prefsSize;
	Err             error, err;

	// A little integrity check
	if (sizeof(GlobalsType) != sizeof(GlobalsType_dummy))
		MyErrorFunc("sizeof(GlobalsType) != sizeof(GlobalsType_dummy). Fix it!!", NULL);

  	// Check if secret record should be shown.
  	if (PrefGetPreference(prefHidePrivateRecordsV33))
    		mode = dmModeReadWrite;
  	else
    		mode = dmModeReadWrite | dmModeShowSecret;

	// Read the saved preferences / saved-state information.
	prefsSize = sizeof(WAPPreferenceType);
	if (PrefGetAppPreferences(appFileCreator, appPrefID, &g_prefs, 
		&prefsSize, true) == noPreferenceFound) {
			//g_prefs.imgBrdr       = 1;
			g_prefs.timeout         = 60;
			g_prefs.timeoutlist     = 2;
			g_prefs.altAgent        = 0;
			g_prefs.images          = true; 		// display images
			g_prefs.conn            = 0;
			g_prefs.mode            = MODE_BROWSER;
			//g_prefs.imgResize 	= false;
			g_prefs.agentNum        = 0;
			g_prefs.maxCache        = 262144;		// (256 * ONE_KILOBYTE)
			g_prefs.fontSize        = 0;
			g_prefs.cookie          = COOKIE_PREF_ASK;
			g_prefs.renderMode      = RENDER_SMALL_SCREEN;
			g_prefs.startDest       = START_HOME;
			g_prefs.private         = false;
			g_prefs.debug           = false;
            g_prefs.debugMask       = debugOffMask;
			g_prefs.fiveWayMode     = FIVEWAY_MODE_POINTER;
			g_prefs.fiveWaySpeed	= 8 * HiresFactor();
			g_prefs.fiveWayAccel	= 2;
			g_prefs.disableUAProf	= false;
			StrCopy(g_prefs.downloadTo, DEFAULT_DOWNLOAD);
            StrCopy(g_prefs.lastURL, BROWSER_ABOUT_BLANK);

            // save them
            PrefSetAppPreferences(appFileCreator, appPrefID,
                appPrefVersionNum, &g_prefs,
                sizeof(WAPPreferenceType), true);
	}

	g = (GlobalsType *)Malloc(sizeof(GlobalsType));
	if (!g) {
		MyErrorFunc("Could not load Universe. Not enough dynamic memory.", NULL);
		return -1;
	}

 	MemSet(g, sizeof(GlobalsType), 0);
 	FtrSet(wuAppType, ftrGlobals, (UInt32)g);

	debugStart(g);

	debugOut("AppStart", "set depth", 0, __FILE__, __LINE__, g);
	setDisplayDepth(false);

	debugOut("AppStart", "set resize", 0, __FILE__, __LINE__, g);
	InitializeResizeSupport( resizeIndex );

	g->state = BS_IDLE;
	g->Render = true;
	g->nextView = 0;

	debugOut("AppStart", "open connections", 0, __FILE__, __LINE__, g);
	dbConnOpenOrCreateDB(mode,g);
	debugOut("AppStart", "open bookmarks", 0, __FILE__, __LINE__, g);
	dbUrlOpenOrCreateDB(mode,g);
	debugOut("AppStart", "open cache", 0, __FILE__, __LINE__, g);
	CacheCreateCacheFile(mode,g);
	debugOut("AppStart", "open history", 0, __FILE__, __LINE__, g);
	HistoryCreateHistoryFile(mode, g);
	debugOut("AppStart", "open cookies", 0, __FILE__, __LINE__, g);
	CookieOpenJar(g->CookieJar, mode, g);

	debugOut("AppStart", "read current connection", 0, __FILE__, __LINE__, g);
	g->DbConnCurrentRecord = g_prefs.conn;
    dbConnReadCurrentConnection (g);
    StrCopy(g->ActiveConnection, g->CurrentConnection.name);
    g->ActiveConnStore = g_prefs.conn;

	g->privateBrowsing = g_prefs.private;
	   
	//Load the Jpeg Library
	/*error = SysLibFind(jpegLibName, &jpegLibRef);
	if (error)
		error = SysLibLoad(sysResTLibrary, jpegLibCreator, &jpegLibRef);
	if (!error)
		error = JpegLibOpen(jpegLibRef, jpegLibCurrentVersion);
	else
		jpegLibRef = 0;*/

	debugOut("AppStart", "open pnoJpegLib", 0, __FILE__, __LINE__, g);
	jpegLibRef = NULL;
	error = pnoJpeg_OpenLibrary(&jpegLibRef);
	debugOut("AppStart", "pnoJpegError (blank = opened)", error, __FILE__, __LINE__, g);

	debugOut("AppStart", "open script lib", 0, __FILE__, __LINE__, g);
	if (SysLibFind(scriptLibName, &gScriptRef))
    	{
        	err = SysLibLoad(scriptLibType, scriptLibCreator, &gScriptRef);
		if (err)
			gScriptRef = 0;

		debugOut("AppStart", "scriptlib error (0 = none) may not be loaded", err, __FILE__, __LINE__, g);
    	}

	debugOut("AppStart", "open gif lib", 0, __FILE__, __LINE__, g);
	GIFLib_OpenLibrary(&gGifRef, &g->gifContext);

	debugOut("AppStart", "load png", 0, __FILE__, __LINE__, g);
	g->pngLibrary = Png_Init('armc',1000);

	debugOut("AppStart", "load http", 0, __FILE__, __LINE__, g);
	if (HTTPLibStart( 'Wapu' ) == 0) HttpLibRef = 1;
	else HttpLibRef = 0;

	setUpFiveWay();

	g->post = false;
	g->wasPost = false;

	g->sid = 0;
	g->tid = 0;
	g->tid_new = 0;
	g->old_tid = 0;

	g->isImage = false;
	g->onenterback = false;

	g->launchURL = NULL;

	g->wtpState = WTP_Disconnected;

	APIInitialiseBrowserControls(gadgetBrowserWidget, fldInput, listOption, scrollBarBrowser, scrollBarHorizontal, g);

	debugOut("AppStart", "load fonts", 0, __FILE__, __LINE__, g);
	FontLoadFont(narrowFont, narrowFontID);
	FontLoadFont(italicFont, italicFontID);
	FontLoadFont(emphasisFont, emphasisFontID);
	FontLoadFont(preFont, preFontID);
	FontLoadFont(supFont, supFontID);
	FontLoadFont(smallNormalFont, smallNormalFontID);
	FontLoadFont(smallBoldFont, smallBoldFontID);
	FontLoadFont(smallSmallFont, smallSmallFontID);
	FontLoadFont(smallPreformattedFont, smallPreformattedFontID);
	//FontLoadFont(smallLargeFont, smallLargeFontID);
	FontLoadFont(passwordFont, passwordFontID);
	FontLoadFont(stdFontFixed, stdFontFixedID);

	/*if ((dbConnNumRecords(g) > 0) && (cmd == sysAppLaunchCmdGoToURL)) {
		g->launchURL = Malloc(StrLen(cmdPBP)+1);
		if (!g->launchURL) goto bottom;
		StrCopy(g->launchURL, cmdPBP);
bottom:
	} else */if (g->launchURL != NULL) {
		Free(g->launchURL);
	}

	switch (cmd) {
		case sysAppLaunchCmdGoToURL:
			if (dbConnNumRecords(g) > 0) {
				g->launchURL = StrDup(cmdPBP);
			}
			break;
		case sysAppLaunchCmdGotoHomepage:
			if (dbConnNumRecords(g) > 0) {
				g->DbConnCurrentRecord = g->ActiveConnStore;
	
				dbConnReadCurrentConnection (g);

				g->launchURL = StrDup(g->CurrentConnection.home);
			}
			break;
		case sysAppLaunchCmdGotoBlank:
			if (dbConnNumRecords(g) > 0) {
				g->launchURL = StrDup(BROWSER_ABOUT_BLANK);
			}
			break;
		case sysAppLaunchCmdViewBookmarks:
			g->nextView = frmUrlList;
			break;
		case sysAppLaunchCmdViewConnSet:
			g->nextView = frmConnectionList;
			break;
		case sysAppLaunchCmdNormalLaunch: // select view from user preferences
			switch (g_prefs.startDest) {
				case START_BLANK:
					g->launchURL = StrDup(BROWSER_ABOUT_BLANK);
					break;
				case START_HOME:
					if (dbConnNumRecords(g) > 0) {
						g->DbConnCurrentRecord = g->ActiveConnStore;
						dbConnReadCurrentConnection(g);
						g->launchURL = StrDup(g->CurrentConnection.home);
					}
					break;
                case START_LASTURL:
                    break;
				case START_BOOK:
				default:
					break;
			}
			break;
		default:
			g->nextView = frmUrlList;
			break;
	}

	debugOut("AppStart", "go!", 0, __FILE__, __LINE__, g);
	g->CurrentView = frmSplash;
	FrmGotoForm (g->CurrentView);
	frm = FrmGetActiveForm();

	return 0;
}

static Boolean ApplicationHandleEvent(EventPtr event)
{
    FormPtr frm;
    Int16   formId;
    Boolean handled = false;

    if (event->eType == frmLoadEvent) {
        formId = event->data.frmLoad.formID;
        frm = FrmInitForm(formId);
        FrmSetActiveForm(frm);
        
        switch(formId) {

         case frmUrlList:
		SetResizePolicy(frmUrlList);
             	FrmSetEventHandler(frm, ehFormUrlList);
             	break;
         case frmEditURL:
             	FrmSetEventHandler(frm, ehFormEditURL);
             	break;
         case frmBrowser:
		SetResizePolicy(frmBrowser);
             	FrmSetEventHandler(frm, ehFormBrowser);
             	break;
         case frmEditConnection:
             	FrmSetEventHandler(frm, ehFormEditConnection);
             	break;
         case frmConnectionList:
		SetResizePolicy(frmConnectionList);
             	FrmSetEventHandler(frm, ehFormConnectionList);
             	break;
         case frmSplash:
		SetResizePolicy(frmSplash);
         	FrmSetEventHandler(frm, SplashFormHandleEvent);
         	break;
         case frmInfo:
         	FrmSetEventHandler(frm, ehFormInfo);
         	break;
         case frmUrl:
         	FrmSetEventHandler(frm, ehFormUrl);
         	break;
         case frmPrefs:
         	FrmSetEventHandler(frm, PrefsFormHandleEvent);
         	break;
         case frmHistory:
		SetResizePolicy(frmHistory);
         	FrmSetEventHandler(frm, ehFrmHistory);
         	break;
         /*case frmPushInbox:
         	FrmSetEventHandler(frm, ehFormPushList);
         	break;*/
	 case frmAddBookmark:
		FrmSetEventHandler(frm, AddBookmarkFormEventHandler);
		break;
         case frmBrowserPrivacy:
         	FrmSetEventHandler(frm, PrivacyFormHandleEvent);
         	break;
         
       
         default:
	     break;
            }
        handled = true;
        }
    return handled;
}


void EventLoop(void)
{
    EventType 	event;
    UInt16 	error;

    do {
        EvtGetEvent(&event, /*SysTicksPerSecond() / 10*/ 5 );

	if (!HandleVChrs(&event)) {
		if (!FiveWayHandleEvent(&event)) {
        		if (!SysHandleEvent(&event)) {
            			if (!MenuHandleEvent(0, &event, &error)) {
                			if (!ApplicationHandleEvent(&event)) {
						FrmDispatchEvent(&event);
						/*if (!HandleBrowserControls(&event)) {
                	   				FrmDispatchEvent(&event);
						}*/
					}
				}
			}
		}
	}
            
    } while (event.eType != appStopEvent);
    

}

static void StopApplication(void)        
{
	GlobalsType	*g;	
	RectangleType	r = {{20, 40}, {120, 40}};
	UInt16		cornerDiam = 0;
	//UInt16		usecount;
	Err		err; // , error;
	//UInt16		numApps;
	//void		*pageFilePtr = NULL;

	// get globals reference
	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	WinEraseRectangle(&r, cornerDiam);
	WinDrawRectangleFrame(popupFrame, &r);
	FntSetFont(largeBoldFont);
	WinDrawChars("Exiting the Browser....", StrLen("Exiting the Browser...."), 25, 53);

	g_prefs.conn = g->ActiveConnStore;
	g_prefs.private = g->privateBrowsing;

	FrmCloseAllForms();

	if ((g->conn.connectionType == 'O') && (g->wtpState == WTP_Connected)) { //clean up our WTP session
		WSPDisconnect(g->sock, g);
	}
	if (g->sock >= 0)
        	close (g->sock);
 
	//Close the Jpeg Library
	/*if (jpegLibRef) {
 		error = JpegLibClose(jpegLibRef, &usecount);
		if (error == errNone && usecount == 0)
			SysLibRemove(jpegLibRef);
	}*/

	err = pnoJpegClose(jpegLibRef);

	if (g->pngLibrary)
		Png_Close(g->pngLibrary);

	if (gScriptRef) {
		//err = ScriptLibClose(gScriptRef, &numApps);
		//if (err == errNone && numApps == 0)
		SysLibRemove(gScriptRef);
	}

	if (gGifRef) {
		GIFLib_CloseLibrary(gGifRef, g->gifContext);
	}

	if (HttpLibRef == 1) HTTPLibStop();

	PrefSetAppPreferences(
        	appFileCreator, appPrefID, appPrefVersionNum, 
        	&g_prefs, sizeof(WAPPreferenceType), true);

	historyClear(g);

	SysTaskDelay(/*50*/ 20);
	TerminateResizeSupport();
	//FrmCloseAllForms();

	debugOut("AppStop", "close bookmarks", 0, __FILE__, __LINE__, g);
	SetDBBackupBit(g->DbUrlDb);
	DmCloseDatabase(g->DbUrlDb);
	debugOut("AppStop", "close connections", 0, __FILE__, __LINE__, g);
	SetDBBackupBit(g->DbConnDb);
	DmCloseDatabase(g->DbConnDb);
	//SetDBBackupBit(g->DbPushDb);
	//DmCloseDatabase(g->DbPushDb);
	debugOut("AppStop", "close cache", 0, __FILE__, __LINE__, g);
	DmCloseDatabase(g->cache.DbCache);
	debugOut("AppStop", "close history", 0, __FILE__, __LINE__, g);
	SetDBBackupBit(g->DbHistory);
	DmCloseDatabase(g->DbHistory);
	debugOut("AppStop", "close cookies", 0, __FILE__, __LINE__, g);
	SetDBBackupBit(g->DbCookieJar);
	CookieCloseJar(g->CookieJar, g);

	debugEnd(g);

	//FreeResources(g);

	setDisplayDepth(true);

	FtrUnregister(wuAppType, ftrGlobals);
	Free(g);
}


static UInt16 RomVersionCompatible (UInt32 requiredVersion, UInt16 launchFlags)
{
  UInt32           romVersion;
  
  FtrGet (sysFtrCreator, sysFtrNumROMVersion, &romVersion);
  if (romVersion < requiredVersion)
    {
      if ((launchFlags & (sysAppLaunchFlagNewGlobals |
          sysAppLaunchFlagUIApp)) ==
          (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
          FrmAlert (alRomIncompatible);
          if (romVersion < 0x02000000){
            AppLaunchWithCommand (sysFileCDefaultApp,
                                  sysAppLaunchCmdNormalLaunch, NULL);
	  }
        }
      return (sysErrRomIncompatible);
    }
  return 0;
}

