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
#include 	"WAPUniverse.h"
#include    	"formConnectionList.h"
#include    	"formUrlList.h"
#include    	"dbConn.h"
#include 	"PalmUtils.h"
#include    	"formEditConnection.h"
#include 	"../res/WAPUniverse_res.h"
#include	"MenuHnd.h"
#ifdef __GNUC__
#include "callbacks.h"
#endif

static Boolean ButtonConnectionSettingsDone_OH(EventPtr event, GlobalsType *g) SEC_1;
static Boolean ButtonConnectionSettingsDelete_OH(EventPtr event, GlobalsType *g) SEC_1;
static void ConnectionDetailsInit (GlobalsType *g) SEC_1;
static UInt16 ConnectionDetailsApply (GlobalsType *g) SEC_1;

static void ConnectionDetailsInit (GlobalsType *g)
{
  UInt16            		attr;
  dbConnPackedConnection*	pconn;
  MemHandle        			rechand;
  Int16             		seekAmount = g->DbConnCurrentRecord;
  UInt16            		index = 0;
  ControlType *controlP;
  FormPtr         	frm;

  // must do seek to skip over secret records
  DmSeekRecordInCategory (g->DbConnDb, &index, seekAmount, dmSeekForward, dmAllCategories);

  // Get the secret and category attributes of the current record.
  DmRecordInfo (g->DbConnDb, index, &attr, NULL, NULL);
//  gRecordCategory = attr & dmRecAttrCategoryMask;

  rechand = DmQueryRecord (g->DbConnDb, index);
  pconn = MemHandleLock (rechand);
  dbConnUnPackConnection (&(g->CurrentConnection), pconn);
  frm = FrmGetFormPtr (frmEditConnection);

  SetFieldFromStr(g->CurrentConnection.name , fldConnectionName);
  SetFieldFromStr(g->CurrentConnection.ipaddress , fldIPAddress);
  SetFieldFromStr(g->CurrentConnection.home , fldHome);
  SetFieldFromStr(g->CurrentConnection.proxyuser , fldConnectionUser);
  SetFieldFromStr(g->CurrentConnection.proxypass , fldConnectionPass);
  if (g->CurrentConnection.bearer == 'C'){
	LstSetSelection (GetObjectPtr(listBearer), 0);
	controlP = GetObjectPtr(triggerBearer);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listBearer), 0) ); }
  if (g->CurrentConnection.bearer == 'S'){
	LstSetSelection (GetObjectPtr(listBearer), 1);
	controlP = GetObjectPtr(triggerBearer);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listBearer), 1) ); }
	
  if (g->CurrentConnection.connectionType == 'L') {
  	LstSetSelection (GetObjectPtr(listConnType), 0);
	controlP = GetObjectPtr(triggerConnType);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listConnType), 0) ); }
  else if (g->CurrentConnection.connectionType == 'O') {
  	LstSetSelection (GetObjectPtr(listConnType), 1);
	controlP = GetObjectPtr(triggerConnType);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listConnType), 1) ); }
  /*if (g->CurrentConnection.connectionType == '1') {
  	LstSetSelection (GetObjectPtr(listConnType), 2);
	controlP = GetObjectPtr(triggerConnType);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listConnType), 2) ); }
  else if (g->CurrentConnection.connectionType == '2') {
  	LstSetSelection (GetObjectPtr(listConnType), 3);
	controlP = GetObjectPtr(triggerConnType);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listConnType), 3) ); }*/
  else if (g->CurrentConnection.connectionType == 'h') {
  	LstSetSelection (GetObjectPtr(listConnType), 2);
	controlP = GetObjectPtr(triggerConnType);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listConnType), 2) ); }
  else if (g->CurrentConnection.connectionType == 'd') {
  	LstSetSelection (GetObjectPtr(listConnType), 3);
	controlP = GetObjectPtr(triggerConnType);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listConnType), 3) ); }
  else {
  	LstSetSelection (GetObjectPtr(listConnType), 0);
	controlP = GetObjectPtr(triggerConnType);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listConnType), 0) ); }
  if (StrLen(g->CurrentConnection.home) == 0)
  	StrCopy(g->CurrentConnection.home,"http://");
  SetFieldFromStr(g->CurrentConnection.home , fldHome);

  if (StrLen(g->CurrentConnection.port) == 0) {
	if (g->CurrentConnection.connectionType == 'L')
  		StrCopy(g->CurrentConnection.port,"9200");
	else if (g->CurrentConnection.connectionType == 'O')
  		StrCopy(g->CurrentConnection.port,"9201");
	else if (g->CurrentConnection.connectionType == 'h')
  		StrCopy(g->CurrentConnection.port,"8080");
	else if (g->CurrentConnection.connectionType == 'd')
  		StrCopy(g->CurrentConnection.port,"8080");
	else
  		StrCopy(g->CurrentConnection.port,"9200");
  }
  SetFieldFromStr(g->CurrentConnection.port , fldPort);
  
  MemHandleUnlock (rechand);

}

static UInt16 ConnectionDetailsApply (GlobalsType *g)
{
	UInt16            	attr;
  	UInt16            	updateCode = 0;
  	dbConnConnection 	tConn;
  	MemHandle        	currec;
  	FormPtr         	frm;
  	Int16             	seekAmount = g->DbConnCurrentRecord;
  	UInt16            	index = 0;
  	Char 			*lock;
  	UInt16 		pushed;
  	FieldPtr        	nameFldPtr, ipFldPtr, homeFldPtr, portFldPtr, usrFldPtr, passFldPtr;
  	Char 			*nameTxtPtr, *ipTxtPtr, *homeTxtPtr, *portTxtPtr, *usrTxtPtr, *passTxtPtr;
  	Int16			connTypeSelection = 0;


	// Field values

	// Name Field
  	frm = FrmGetFormPtr (frmEditConnection);
  	nameFldPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldConnectionName))));
  	nameTxtPtr = FldGetTextPtr(nameFldPtr);

	// IP Field
	ipFldPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldIPAddress))));
	ipTxtPtr = FldGetTextPtr(ipFldPtr);

	// Homepage Field
	homeFldPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldHome))));
	homeTxtPtr = FldGetTextPtr(homeFldPtr);

	// Port Field
	portFldPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldPort))));
	portTxtPtr = FldGetTextPtr(portFldPtr);

	// Username Field
	usrFldPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldConnectionUser))));
	usrTxtPtr = FldGetTextPtr(usrFldPtr);

	// Password field
	passFldPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldConnectionPass))));
	passTxtPtr = FldGetTextPtr(passFldPtr);

	connTypeSelection = LstGetSelection(GetObjectPtr(listConnType));


	if (StrLen(nameTxtPtr) <= 0) {
		MyErrorFunc("This connection setting cannot be saved with an empty name.", NULL);
		updateCode = 1;
		return updateCode;
	} 
	if (StrLen(ipTxtPtr) <= 0) {
		MyErrorFunc("A valid IP address or hostname is required.", NULL);
		updateCode = 1;
		return updateCode;
	}
	if (((connTypeSelection == 0) || (connTypeSelection == 1) || (connTypeSelection == 2)) &&
	    (StrNCaselessCompare(ipTxtPtr, "0.0.0.0", 7) == 0)) {
		MyErrorFunc("An IP address of \"0.0.0.0\" cannot be used on a non-Direct connection.", NULL);
		updateCode = 1;
		return updateCode;
	}
	if (StrLen(portTxtPtr) <= 0) { // shouldn't freak out if the type is direct
		MyErrorFunc("A valid port is required.", NULL);
		updateCode = 1;
		return updateCode;
	}
	if ((StrAToI(portTxtPtr) <= 0) && (StrAToI(portTxtPtr) > 65535)) {
		MyErrorFunc("A valid port number greater then 0 and less than 65535 is required.", NULL);
		updateCode = 1;
		return updateCode;
	}


  	// must do seek to skip over secret records
  	DmSeekRecordInCategory (g->DbConnDb, &index, seekAmount, dmSeekForward, dmAllCategories);

  	// Get the secret and category attributes of the current record.
  	DmRecordInfo (g->DbConnDb, index, &attr, NULL, NULL);
	

  	if (nameTxtPtr != NULL) {
  		lock = StrStr(nameTxtPtr, ";L");
  		if (lock) {
      			*lock = 0;
      			pushed = FrmCustomAlert(alLock, nameTxtPtr, NULL, NULL);
      			switch (pushed) {
          		case 0:
             			tConn.lock = true;
              			break;
          		case 1:
              			tConn.lock = false;
              			break;
     			}
      			SetFieldFromStr(nameTxtPtr, fldConnectionName);
  		} else {
  			tConn.lock = false;
  		}

    		StrCopy (tConn.name, nameTxtPtr);
  	}
    
  
  	if (StrLen(ipTxtPtr) > 0)
    		StrCopy(tConn.ipaddress, ipTxtPtr);
    

  	if (StrLen(homeTxtPtr) > 0)
   		StrCopy(tConn.home, homeTxtPtr);
  	else
    		StrCopy(tConn.home, "device:home");//impersonate an UP.Browser home URL
  

  	if (StrLen(portTxtPtr) > 0)
    		StrCopy (tConn.port, portTxtPtr);

  
  	if (StrLen(usrTxtPtr) > 0)
    		StrCopy (tConn.proxyuser, usrTxtPtr);
  	else
    		StrCopy (tConn.proxyuser, "");


  	if (StrLen(passTxtPtr) > 0)
    		StrCopy (tConn.proxypass, passTxtPtr);
  	else
   		StrCopy (tConn.proxypass, "");


  	//at the moment no security is installed and only conectionless connection mode is supported.
  	//TODO: Add security and support for connection oriented mode (port 9201)
  	if (connTypeSelection == 0) {		// WSP, Temporary
  		tConn.connectionType = 'L';
  		tConn.security = 'N'; 
	} else if (connTypeSelection == 1) {	// WTP, Permanent
  		tConn.connectionType = 'O';
  		tConn.security = 'N'; 
	}
  	/*else if (connTypeSelection == 2) {	// WSP, Secured Temporary
  		tConn.connectionType = '1';
  		tConn.security = 'Y'; }
  	else if (connTypeSelection == 3) {	// WSP, Secured Permanent
  		tConn.connectionType = '2';
  		tConn.security = 'Y'; 
	}*/
  	else if (connTypeSelection == 2) {	// HTTP, Proxy or WAP 2.0
  		tConn.connectionType = 'h';
  		tConn.security = 'N'; 
	} else if (connTypeSelection == 3) {	// HTTP Direct
  		tConn.connectionType = 'd';
  		tConn.security = 'N'; 
	} else {
  		tConn.connectionType = 'd';
  		tConn.security = 'N'; 
	}

  	//tConn.connectionType = 'L';
  	//tConn.security = 'N';
  	tConn.bearer = 'C';
  	// \/ \/ Removed because we do not want to cause problems. Not untill we have support for SMS
  	/*if (LstGetSelection (GetObjectPtr(listBearer)) == 0) {
  		tConn.bearer = 'C'; 
	} else if (LstGetSelection (GetObjectPtr(listBearer)) == 1){
  		tConn.bearer = 'S'; 
	} else {
  		tConn.bearer = 'C'; 
	}*/

  	currec = DmGetRecord (g->DbConnDb, index);
  	dbConnPackConnection (&tConn, currec);
  	DmReleaseRecord (g->DbConnDb, index, true);
  	dbConnSortRecord (g->DbConnDb, &index);
    
    NVFSSyncDatabase(g->DbConnDb);

  	return updateCode;
}

void ConnFrmScroll (FormPtr frm, WinDirectionType direction)
{
	UInt16 upIndex;
	UInt16 downIndex;
	Boolean enableUp = false;
	Boolean enableDown = true;

	if (direction == winUp) {
		enableUp = false;
		enableDown = true;
	} else if (direction == winDown) {
		enableUp = true;
		enableDown = false;
	}
	upIndex = FrmGetObjectIndex (frm, buttonScrollConnUp);
	downIndex = FrmGetObjectIndex (frm, buttonScrollConnDwn);

	if (direction == winUp) {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblProxyAuth));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblConnectionUser));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldConnectionUser));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblConnectionPass));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldConnectionPass));

		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblConnectionTitle));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldConnectionName));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblIPAddress));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldIPAddress));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblPort));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldPort));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblConnectionType));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,triggerConnType));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblBearer));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,triggerBearer));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblHome));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldHome));
	} else if (direction == winDown) {
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblConnectionTitle));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldConnectionName));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblIPAddress));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldIPAddress));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblPort));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldPort));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblConnectionType));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,triggerConnType));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblBearer));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,triggerBearer));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblHome));
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldHome));

		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblProxyAuth));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblConnectionUser));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldConnectionUser));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,lblConnectionPass));
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldConnectionPass));
	}

	FrmUpdateScrollers (frm, upIndex, downIndex, enableUp, enableDown);
}

/**----------------------------------**/
/** Form Event Handler for FormEditSettings**/
/**----------------------------------**/

/****************************************************/
/* 
   Function: Boolean FormEditSettings_EH(EventPtr event);
   Description: Main Form Event Handler for FormEditSettings
 */
/****************************************************/


/* Form Event Handler */
Boolean ehFormEditConnection(EventPtr event)
{
    Boolean handled = false;
	GlobalsType *g;
	FormPtr         	frm;
	
#ifdef __GNUC__
    CALLBACK_PROLOGUE
#endif

	frm = FrmGetFormPtr (frmEditConnection);
	
	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
    switch(event->eType) {

        case ctlSelectEvent:
            if (event->data.ctlSelect.controlID == buttonConnectionSettingsDone) {
                handled = ButtonConnectionSettingsDone_OH(event,g); }
            if (event->data.ctlSelect.controlID == buttonConnectionSettingsDelete) {
                handled = ButtonConnectionSettingsDelete_OH(event,g); }
            if (event->data.ctlSelect.controlID == buttonScrollConnUp) {
		ConnFrmScroll (frm, winUp);
                handled = true; }
            if (event->data.ctlSelect.controlID == buttonScrollConnDwn) {
		ConnFrmScroll (frm, winDown);
                handled = true; }
	    if (event->data.ctlSelect.controlID == triggerBearer) {
		FrmAlert (alDataOnly);
		handled = true;}
            break;

        case ctlRepeatEvent:
            if (event->data.ctlSelect.controlID == buttonScrollConnUp) {
		ConnFrmScroll (frm, winUp);
                handled = true; }
            if (event->data.ctlSelect.controlID == buttonScrollConnDwn) {
		ConnFrmScroll (frm, winDown);
                handled = true; }
            break;

        case tblSelectEvent:
            break;

        case lstSelectEvent:
        case popSelectEvent:
	    if (event->data.popSelect.listID == listConnType) {
		if (event->data.popSelect.selection == 0)
			  SetFieldFromStr("9200", fldPort);
		if (event->data.popSelect.selection == 1)
			  SetFieldFromStr("9201", fldPort);
		if (event->data.popSelect.selection == 2)
			  SetFieldFromStr("8080", fldPort);
		if (event->data.popSelect.selection == 3) // A port and an IP address is not needed, should hide them
			  SetFieldFromStr("80", fldPort);
            }
            break;

        case sclEnterEvent:
            break;

	case frmUpdateEvent:
	   FrmDrawForm(FrmGetActiveForm());
	   handled = true;
	   break;

        case frmOpenEvent:
           ConnectionDetailsInit(g);
           FrmDrawForm(FrmGetActiveForm());
           handled = true;
           FrmSetFocus (FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fldConnectionName));
	   ConnFrmScroll (frm, winUp);
           break;

        case menuEvent:
           mhMenuEdit(event->data.menu.itemID);
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
/** Form Object Handler Functions for FormEditSettings **/
/**--------------------------------------**/

/****************************************************/
/* 
   Function: Int16 ButtonConnectionSettingsDone_OH(EventPtr event);
   Description: Button Object Handler for the FormEditSettings Form
 */
/****************************************************/

static Boolean ButtonConnectionSettingsDone_OH(EventPtr event, GlobalsType *g)
{
	UInt16		settingError = 0;

    	settingError = ConnectionDetailsApply(g);

	if (settingError == 0) {
    		//FrmCloseAllForms();
    		g->CurrentView = frmConnectionList;

    		SwitchForm(frmConnectionList, g);//FrmReturnToForm(0);
		return(true);
	} else {
		return(false);
	}
}


/****************************************************/
/* 
   Function: Int16 ButtonConnectionSettingsDelete_OH(EventPtr event);
   Description: Button Object Handler for the FormEditSettings Form
 */
/****************************************************/

static Boolean ButtonConnectionSettingsDelete_OH(EventPtr event, GlobalsType *g)
{
    FieldPtr	ipPtr;
    FormPtr	frm;

	frm = FrmGetFormPtr (frmEditConnection);
	ipPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldConnectionName))));

	switch (FrmCustomAlert (alDelete, "connection setting", FldGetTextPtr (ipPtr), "")) {
		case 1:
			// The delete button was pressed.  Delete the current record.
			if ( dbConnDeleteCurrentRecord (g))
			{
				// Delete was completed.  Close the details and the edit forms.
				//FrmCloseAllForms ();

       				// Go to the main form.
	  			g->CurrentView = frmConnectionList;
       				SwitchForm(frmConnectionList, g);//FrmReturnToForm(0);
       				return(true);
    			}
			break;
		default:
			return(false);
			break;
    	}

	return true;
}


