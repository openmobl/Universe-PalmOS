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
#include	<SystemMgr.h>
#include 	"WAPUniverse.h"
#include 	"../res/WAPUniverse_res.h"
#include    	"formConnectionList.h"
#include    	"MenuHnd.h"
#include    	"dbConn.h"
#include 	"WAP.h"
#include 	"PalmUtils.h"
#include	"build.h"
#include	"formUrlList.h"
#include	"DIA.h"
#include	"resize.h"
#ifdef HAVE_PALMONE_SDK
#include	<common/system/palmOneNavigator.h>
#include	<common/system/palmOneChars.h>
#include	<common/system/HSKeyTypes.h>
#include 	<KeyMgr.h>
#include 	<68K/Hs.h>
#include 	<68K/Libraries/PmKeyLib/PmKeyLib.h>
#endif
#ifdef __GNUC__
#include 	"callbacks.h"
#endif


void MyInfoFunc (char* error, char* additional, char* more)
{

    FrmCustomAlert (infAgent, error, additional, more);
}
		

// function prototypes
//static Boolean ButtonConnectionActivate_OH(EventPtr event, GlobalsType *g);
//static Boolean ButtonConnectionNew_OH(EventPtr event, GlobalsType *g);
static Boolean ListConnectionSettings_OH(EventPtr event);
//static Boolean ButtonConnectionEdit_OH(EventPtr event, GlobalsType *g);

void enableConnectionEditOpen(GlobalsType *g)
{
  FormPtr	frm;
  Boolean 	enable;
  UInt16    nr;
  
    nr =  dbConnNumRecords(g); 
    if (nr >0)
		enable = true;
	else
		enable = false;
    frm = FrmGetFormPtr (frmConnectionList);
    CtlSetEnabled(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, buttonConnectionActivate)),enable);
  	CtlSetEnabled(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, buttonConnectionEdit)),enable);
    CtlSetUsable(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, buttonConnectionActivate)),enable);
  	CtlSetUsable(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, buttonConnectionEdit)),enable);
	
}

// draw strings at top of rectangle r, but don't overwrite
// right-edge of r
void DrawPadlockandChars (char* s, RectanglePtr r, char c, Boolean padlock)
{
    //char            t[40];
    Int16          stringLength = StrLen (s) + 1;
    Int16          pixelWidth = r->extent.x;
    Boolean         truncate;
    BitmapPtr	   bitmap;

	//t[0] = c;

	//StrCopy (&t[1], s);

	switch (c) {
		case '\273':
			bitmap = MemHandleLock(DmGetResource(bitmapRsc, bmpSelected));
			WinDrawBitmap(bitmap, r->topLeft.x, r->topLeft.y+1);
			MemPtrUnlock(bitmap);
			break;
		case ' ':
		default:
			bitmap = MemHandleLock(DmGetResource(bitmapRsc, bmpSpace));
			WinDrawBitmap(bitmap, r->topLeft.x, r->topLeft.y+1);
			MemPtrUnlock(bitmap);
			break;
	}

	// FntCharsInWidth will update stringLength to the 
	// maximum without exceeding the width
	FntCharsInWidth (s, &pixelWidth, &stringLength, &truncate);

	WinDrawChars (s, stringLength, r->topLeft.x+8, r->topLeft.y);

	if (padlock) {
		bitmap = MemHandleLock(DmGetResource(bitmapRsc, bmpLock));
		WinDrawBitmap(bitmap, r->extent.x-3, r->topLeft.y+2);
		MemPtrUnlock(bitmap);
	}
}

static void
DrawOneConnectionInList (Int16 itemNumber, RectangleType *bounds, Char * * text)
{
  MemHandle        	h;
  Int16             seekAmount = itemNumber;
  UInt16            index = 0;
  GlobalsType 		*g;


#ifdef __GNUC__
  CALLBACK_PROLOGUE
#endif

 FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

  // must do seek to skip over secret records
  DmSeekRecordInCategory (g->DbConnDb, &index, seekAmount, dmSeekForward, dmAllCategories);
  h = DmQueryRecord (g->DbConnDb, index);
  if (h)
    {
      dbConnPackedConnection* packedConnection = MemHandleLock (h);

      g->DbConnCurrentRecord = seekAmount;
      dbConnReadCurrentConnection (g);
      if (g->CurrentConnection.lock == true) {
	if (StrCompare (packedConnection->name, g->ActiveConnection) == 0)
        	DrawPadlockandChars (packedConnection->name, bounds, '\273', true);
      	else
        	DrawPadlockandChars (packedConnection->name, bounds, ' ', true);
      } else {
      	if (StrCompare (packedConnection->name, g->ActiveConnection) == 0)
        	DrawPadlockandChars (packedConnection->name, bounds, '\273', false);
      	else
        	DrawPadlockandChars (packedConnection->name, bounds, ' ', false);
      }

      MemHandleUnlock (h);
    }
#ifdef __GNUC__
  CALLBACK_EPILOGUE
#endif
}

static void
InitViewConnectionList(GlobalsType *g)
{

  FormPtr         frm = FrmGetActiveForm ();
  ListPtr         list = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, lstConnectionSettings));
  UInt16            numConnections = DmNumRecordsInCategory (g->DbConnDb, dmAllCategories);
  LstSetListChoices (list, NULL, numConnections);
  LstSetDrawFunction (list, DrawOneConnectionInList);

}


/**----------------------------------**/
/** Form Event Handler for FormConnectionList**/
/**----------------------------------**/

/****************************************************/
/* 
   Function: Boolean FormConnectionList_EH(EventPtr event);
   Description: Main Form Event Handler for FormConnectionList
 */
/****************************************************/


/* Form Event Handler */
Boolean ehFormConnectionList(EventPtr event)
{
    Boolean handled = false;
	GlobalsType *g;
	ListType *lst;
	FormPtr  frm = FrmGetActiveForm();
	Int16 lstItem, lstTotal;
	UInt16 		keyCode		= event->data.keyDown.keyCode;
	
#ifdef __GNUC__
    CALLBACK_PROLOGUE
#endif

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if ( ResizeHandleEvent( event ) )
        	return true;

    switch(event->eType) {

        case ctlSelectEvent:
	    if (event->data.ctlSelect.controlID == buttonMenuC) {
		handled = EvtEnqueueKey( menuChr, 0, commandKeyMask ); }
            if (event->data.ctlSelect.controlID == buttonConnectionActivate) {
                handled = ButtonConnectionActivate_OH(event, g); }
            if (event->data.ctlSelect.controlID == buttonConnectionNew) {
                handled = ButtonConnectionNew_OH(event, g); }
            if (event->data.ctlSelect.controlID == buttonConnectionEdit) {
                handled = ButtonConnectionEdit_OH(event, g); }
            if (event->data.ctlSelect.controlID == buttonViewC) {
		if(dbConnNumRecords(g)>0){
			SwitchForm(frmUrlList,g); 					
		} else{
			FrmCustomAlert (alError, "You need to define at least one connection\n", "Conn. Settings", NULL);
		}
	    }
            if (event->data.ctlSelect.controlID == buttonInfoC) {
                FrmPopupForm(frmInfo);
	    }
            if (event->data.ctlSelect.controlID == buttonAgent) {
		UInt16	agent = 0;

		if (g_prefs.altAgent)
			agent = g_prefs.agentNum + 1;

		MyInfoFunc(AgentString[agent], CODENAME, VERSIONSTR);
            }
	    if (event->data.ctlSelect.controlID == buttonConnClose) {
		g->prevFormID = 0;
		FrmReturnToForm(frmBrowser);
		handled = true;}
            break;
        
        case keyDownEvent:
        	if ((event->data.keyDown.chr == pageUpChr) && !SysCheck5Way()) {
            		frm = FrmGetActiveForm();
            		lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, lstConnectionSettings));
            		//LstScrollList(lst, winUp, LstGetVisibleItems(lst));
            		lstItem = LstGetSelection (lst);
			lstTotal = LstGetNumberOfItems (lst);
			if (lstItem == 0){
				LstSetSelection (lst, lstTotal-1);
			} else {
				LstSetSelection (lst, lstItem-1);
			}
        	} else if ((event->data.keyDown.chr == pageDownChr) && !SysCheck5Way()) {
            		frm = FrmGetActiveForm();
            		lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, lstConnectionSettings));
            		//LstScrollList(lst, winDown, LstGetVisibleItems(lst));
            		lstItem = LstGetSelection (lst);
			lstTotal = LstGetNumberOfItems (lst);
			if (lstItem == lstTotal-1){
				LstSetSelection (lst, 0);
			} else {
				LstSetSelection (lst, lstItem+1);
			}
        	}
#ifdef HAVE_PALMONE_SDK
		else if ((event->data.keyDown.chr == vchrNavChange) && !SysCheck5Way()) {
			if (keyCode & navBitRight){
			}				
			if (keyCode & navBitLeft){
				if(dbConnNumRecords(g)>0){
					SwitchForm(frmUrlList,g); 					
				} else{
					FrmCustomAlert (alError, "You need to define at least one connection\n", "Conn. Settings", NULL);
				}
			}			
			if(keyCode & navBitSelect){
				ButtonConnectionActivate_OH(event,g);
			}
		} else if ((event->data.keyDown.chr == keyRockerUp) && !SysCheck5Way()) {
            		frm = FrmGetActiveForm();
            		lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, lstConnectionSettings));
            		//LstScrollList(lst, winUp, LstGetVisibleItems(lst));
            		lstItem = LstGetSelection (lst);
			lstTotal = LstGetNumberOfItems (lst);
			if (lstItem == 0){
				LstSetSelection (lst, lstTotal-1);
			}
			else {
				LstSetSelection (lst, lstItem-1);
			}
        	} else if ((event->data.keyDown.chr == keyRockerDown) && !SysCheck5Way()) {
            		frm = FrmGetActiveForm();
            		lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, lstConnectionSettings));
            		//LstScrollList(lst, winDown, LstGetVisibleItems(lst));
            		lstItem = LstGetSelection (lst);
			lstTotal = LstGetNumberOfItems (lst);
			if (lstItem == lstTotal-1){
				LstSetSelection (lst, 0);
			}
			else {
				LstSetSelection (lst, lstItem+1);
			}
        	} else if ((event->data.keyDown.chr == keyRockerRight) && !SysCheck5Way()) {
		
		} else if ((event->data.keyDown.chr == keyRockerLeft) && !SysCheck5Way()) {
			if(dbConnNumRecords(g)>0){
				SwitchForm(frmUrlList,g); 					
			} else{
				FrmCustomAlert (alError, "You need to define at least one connection\n", "Conn. Settings", NULL);
			}
		} else if ((event->data.keyDown.chr == keyRockerCenter) && !SysCheck5Way()) {
			ButtonConnectionActivate_OH(event,g);
		}
#endif
         break;

        case ctlRepeatEvent:
            break;

        case tblSelectEvent:
            break;

        case lstSelectEvent:
        case popSelectEvent:
            if (event->data.ctlSelect.controlID == lstConnectionSettings) {
                ListConnectionSettings_OH(event); }
            break;

        case sclEnterEvent:
            break;

	case frmUpdateEvent:
	   FrmDrawForm(FrmGetActiveForm());

	   {
		Char *titleTemp = Malloc(15);

		if (titleTemp != NULL) {
			StrCopy(titleTemp, "Conn. Settings");
			UpdateUserTitle(titleTemp);
			Free(titleTemp);
		}
	   }

	   handled = true;
	   break;

        case frmOpenEvent:
           InitViewConnectionList(g);
	       enableConnectionEditOpen(g);
           FrmDrawForm(FrmGetActiveForm());

	   if (g->prevFormID == frmBrowser) {
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonConnClose));
	   }

	   {
		Char *titleTemp = Malloc(15);

		if (titleTemp != NULL) {
			StrCopy(titleTemp, "Conn. Settings");
			UpdateUserTitle(titleTemp);
			Free(titleTemp);
		}
	   }

           handled = true;
           break;

	case winDisplayChangedEvent:
		if (WinGetActiveWindow() == (WinHandle)FrmGetFormPtr(frmConnectionList)) {
			Char *titleTemp = Malloc(15);

			if (titleTemp != NULL) {
				StrCopy(titleTemp, "Conn. Settings");
				UpdateUserTitle(titleTemp);
				Free(titleTemp);
			}
		}
		break;

         case menuEvent:
             mhMenuOptions(event->data.menu.itemID,g);
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
/** Form Object Handler Functions for FormConnectionList **/
/**--------------------------------------**/

/****************************************************/
/* 
   Function: Int16 ButtonConnectionActivate_OH(EventPtr event);
   Description: Button Object Handler for the FormConnectionList Form
 */
/****************************************************/

/*static*/ Boolean ButtonConnectionActivate_OH(EventPtr event, GlobalsType *g)
{
	FormPtr frm = FrmGetActiveForm ();

	if (/*(g->conn.connectionType == 'O') && */(g->wtpState == WTP_Connected)) { //clean up our WTP session
		WSPDisconnect(g->sock, g);
	}
	// close socket if needed              
	if (g->sock >= 0)
        	close (g->sock);

	g->DbConnCurrentRecord = LstGetSelection(FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, lstConnectionSettings)));
	g->ActiveConnStore = g->DbConnCurrentRecord;
	dbConnReadCurrentConnection (g);
	StrCopy (g->ActiveConnection, g->CurrentConnection.name);
	g->CurrentView = frmConnectionList;
	SwitchForm(frmConnectionList, g);//FrmGotoForm (g->CurrentView);
	return(true);
}


/****************************************************/
/* 
   Function: Int16 ButtonConnectionNew_OH(EventPtr event);
   Description: Button Object Handler for the FormConnectionList Form
 */
/****************************************************/
/*static*/ Boolean ButtonConnectionNew_OH(EventPtr event, GlobalsType *g)
{

   dbConnCreateRecord (g);
   g->CurrentView = frmEditConnection;
   SwitchForm(frmEditConnection, g);//FrmPopupForm(frmEditConnection);//FrmGotoForm (g->CurrentView);
   enableConnectionEditOpen(g);
   return(true);
}


/****************************************************/
/* 
   Function: Int16 ButtonConnectionEdit_OH(EventPtr event);
   Description: Button Object Handler for the FormConnectionList Form
 */
/****************************************************/
/*static*/ Boolean ButtonConnectionEdit_OH(EventPtr event, GlobalsType *g)
{
  FormPtr frm;
  
    frm = FrmGetActiveForm ();
    g->DbConnCurrentRecord = LstGetSelection (FrmGetObjectPtr (frm, 
                           FrmGetObjectIndex(frm, lstConnectionSettings)));
    dbConnReadCurrentConnection (g);

    if (g->CurrentConnection.lock == true) {
	FrmCustomAlert (alLockEdit, NULL, NULL, NULL);
	return(false);
    }

    g->CurrentView = frmEditConnection;
    SwitchForm(frmEditConnection, g);//FrmPopupForm(frmEditConnection);//FrmGotoForm (g->CurrentView);
	return(true);
}


/****************************************************/
/* 
   Function: Int16 ListConnectionSettings_OH(EventPtr event);
   Description: List Object Handler for the FormConnectionList Form
 */
/****************************************************/

static Boolean ListConnectionSettings_OH(EventPtr event)
{

	return(0);
}


