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
#include	"dbUrl.h"
#include    	"formUrlList.h"
#include 	"PalmUtils.h"
#include 	"../res/WAPUniverse_res.h"
#include    	"MenuHnd.h"
#include	"dbConn.h"
#include	"WAP.h"
#include	"DIA.h"
#include	"resize.h"
#include	"URL.h"
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


//static Boolean ButtonOpen_OH(EventPtr event, GlobalsType *g) SEC_1; 	/* Button */
//static Boolean ButtonNew_OH(EventPtr event, GlobalsType *g) SEC_1; 	/* Button */
//static Boolean ButtonEdit_OH(EventPtr event, GlobalsType *g) SEC_1; 	/* Button */
static Boolean URLsCategoryTrigger_OH(EventPtr event, GlobalsType *g) SEC_1; 	/* Pop-up Trigger */
static Boolean ListURL_OH(EventPtr event) SEC_1; 	/* List */
static Boolean CategoriesList_OH(EventPtr event) SEC_1; 	/* List */
static void DrawOneURLInList (Int16 itemNumber, RectangleType *bounds, Char * * text) SEC_1;
static void InitViewURLs (GlobalsType *g) SEC_1;

void UpdateUserTitle(Char *title) {
	WinDrawOperation 	oldMode;
	RGBColorType 		rgb;
	IndexedColorType	index, origText = 0;
	Int16			titleX = 16;
	Int16			titleY = 0; // 1

	rgb.r = 255;
	rgb.g = 255;
	rgb.b = 255;
	
    	index = WinRGBToIndex( &rgb );
	origText = WinSetTextColor(index);

	FntSetFont(boldFont);

	oldMode = WinSetDrawMode(winOverlay);
	WinPaintChars(title,FntWordWrap(title,96),titleX,titleY);
	WinSetDrawMode(oldMode);

	FntSetFont(stdFont);

	WinSetTextColor(origText);
}

static void
DrawOneURLInList (Int16 itemNumber, RectangleType *bounds, Char * * text)
{
  MemHandle         h;
  void	            *p =NULL;
  Int16             seekAmount = itemNumber;
  UInt16            index = 0;
  GlobalsType		*g;
  
#ifdef __GNUC__
  CALLBACK_PROLOGUE
#endif

  FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

  // must do seek to skip over secret records
    DmSeekRecordInCategory (g->DbUrlDb, &index, seekAmount, dmSeekForward, g->DbUrlCurrentCategory);
  h = DmQueryRecord (g->DbUrlDb, index);
  if (h)
    {
      dbUrlPackedURL*      packedURL = MemHandleLock (h);
      dbUrlURL 	  url;
      dbUrlUnPackURL (&url, packedURL);
      p = (void *)packedURL;
      DrawCharsToFitWidth (url.name, bounds, ' ');
      if (p){
      	MemPtrUnlock (p);
      }
    }
#ifdef __GNUC__
  CALLBACK_EPILOGUE
#endif
}

static void
InitViewURLs (GlobalsType *g)
{

  FormPtr         frm = FrmGetActiveForm ();
  ListPtr         list = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, listUrlList));
  UInt16            numURLs = DmNumRecordsInCategory (g->DbUrlDb, g->DbUrlCurrentCategory);
  LstSetListChoices (list, NULL, numURLs);
  LstSetDrawFunction (list, DrawOneURLInList);

  enableUrlEditOpen(g);
  CategoryGetName (g->DbUrlDb, g->DbUrlCurrentCategory, g->DbUrlCategoryName);
  CategorySetTriggerLabel (GetObjectPtr (triggerUrlListCategory), g->DbUrlCategoryName);
  //TODO: Return to last viewed category
}

void enableUrlEditOpen(GlobalsType *g)
{
  FormPtr	frm;
  Boolean 	enable;
  
    if (dbUrlNumRecords(g) > 0)
		enable = true;
    else
		enable = false;
    frm = FrmGetFormPtr (frmUrlList);
    CtlSetEnabled(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, buttonOpen)),enable);
  	CtlSetEnabled(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, buttonEdit)),enable);
    CtlSetUsable(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, buttonOpen)),enable);
  	CtlSetUsable(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, buttonEdit)),enable);
	
}


/**----------------------------------**/
/** Form Event Handler for FormURLs**/
/**----------------------------------**/

/****************************************************/
/* 
   Function: Boolean FormURLs_EH(EventPtr event);
   Description: Main Form Event Handler for FormURLs
 */
/****************************************************/


/* Form Event Handler */
Boolean
ehFormUrlList(EventPtr event)
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
	    if (event->data.ctlSelect.controlID == buttonMenuU) {
		handled = EvtEnqueueKey( menuChr, 0, commandKeyMask ); }
            if (event->data.ctlSelect.controlID == buttonOpen) {
                ButtonOpen_OH(event,g); }
            if (event->data.ctlSelect.controlID == buttonNew) {
                ButtonNew_OH(event,g); }
            if (event->data.ctlSelect.controlID == buttonEdit) {
                ButtonEdit_OH(event,g); }
            if (event->data.ctlSelect.controlID == buttonViewU) {
                SwitchForm(frmConnectionList,g); }
            if (event->data.ctlSelect.controlID == buttonInfoU) {
                FrmPopupForm(frmInfo); }//DisplayAbout(); }
            if (event->data.ctlSelect.controlID == triggerUrlListCategory) {
                URLsCategoryTrigger_OH(event,g); }
	    if (event->data.ctlSelect.controlID == buttonUrlClose) {
		g->prevFormID = 0;
                FrmReturnToForm(frmBrowser); }
            break;

        case lstSelectEvent:
        case popSelectEvent:
            if (event->data.ctlSelect.controlID == triggerUrlListCategory) {
                URLsCategoryTrigger_OH(event,g); }
            if (event->data.ctlSelect.controlID == listUrlList) {
                ListURL_OH(event); }
            if (event->data.ctlSelect.controlID == listCategoriesList) {
                CategoriesList_OH(event); }
            break;
        
        case keyDownEvent:
        	if ((event->data.keyDown.chr == pageUpChr) && !SysCheck5Way()) {
            		frm = FrmGetActiveForm();
            		lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, listUrlList));
            		//LstScrollList(lst, winUp, LstGetVisibleItems(lst));
            		lstItem = LstGetSelection (lst);
			lstTotal = LstGetNumberOfItems (lst);
			if (lstItem == 0){
				LstSetSelection (lst, lstTotal-1);
			}
			else {
				LstSetSelection (lst, lstItem-1);
			}
        	} else if ((event->data.keyDown.chr == pageDownChr) && !SysCheck5Way()) {
            		frm = FrmGetActiveForm();
            		lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, listUrlList));
            		//LstScrollList(lst, winDown, LstGetVisibleItems(lst));
            		lstItem = LstGetSelection (lst);
			lstTotal = LstGetNumberOfItems (lst);
			if (lstItem == lstTotal-1){
				LstSetSelection (lst, 0);
			}
			else {
				LstSetSelection (lst, lstItem+1);
			}
        	}
#ifdef HAVE_PALMONE_SDK
		else if ((event->data.keyDown.chr == vchrNavChange) && !SysCheck5Way()) {
			if(keyCode & navBitRight){
				g->DbUrlCurrentCategory = CategoryGetNext (g->DbUrlDb, g->DbUrlCurrentCategory);
				InitViewURLs (g);
    				FrmDrawForm (FrmGetActiveForm ());
				{
					Char *titleTemp = Malloc(10);

					if (titleTemp != NULL) {
						StrCopy(titleTemp, "Bookmarks");
						UpdateUserTitle(titleTemp);
						Free(titleTemp);
					}
	   			}
			}				
			if(keyCode & navBitLeft){
				SwitchForm(frmConnectionList,g);
			}				
			if(keyCode & navBitSelect){
				ButtonOpen_OH(event,g);
			}
		} else if ((event->data.keyDown.chr == keyRockerUp) && !SysCheck5Way()) {
            		frm = FrmGetActiveForm();
            		lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, listUrlList));
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
            		lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, listUrlList));
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
			g->DbUrlCurrentCategory = CategoryGetNext (g->DbUrlDb, g->DbUrlCurrentCategory);
			InitViewURLs (g);
    			FrmDrawForm (FrmGetActiveForm ());
			{
				Char *titleTemp = Malloc(10);

				if (titleTemp != NULL) {
					StrCopy(titleTemp, "Bookmarks");
					UpdateUserTitle(titleTemp);
					Free(titleTemp);
				}
	   		}
		} else if ((event->data.keyDown.chr == keyRockerLeft) && !SysCheck5Way()) {
			SwitchForm(frmConnectionList,g);
		} else if ((event->data.keyDown.chr == keyRockerCenter) && !SysCheck5Way()) {
			ButtonOpen_OH(event,g);
		}
#endif
         break;

	case frmUpdateEvent:
	   FrmDrawForm(FrmGetActiveForm());

	   {
		Char *titleTemp = Malloc(15);

		if (titleTemp != NULL) {
			StrCopy(titleTemp, "Bookmarks");
			UpdateUserTitle(titleTemp);
			Free(titleTemp);
		}
	   }

	   handled = true;
	   break;

        case frmOpenEvent:
           InitViewURLs(g);
           FrmDrawForm(FrmGetActiveForm());

	   if (g->prevFormID == frmBrowser) {
		FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonUrlClose));
	   }

	   {
		Char *titleTemp = Malloc(10);

		if (titleTemp != NULL) {
			StrCopy(titleTemp, "Bookmarks");
			UpdateUserTitle(titleTemp);
			Free(titleTemp);
		}
	   }

           handled = true;
           break;

	case winDisplayChangedEvent:
		if (WinGetActiveWindow() == (WinHandle)FrmGetFormPtr(frmUrlList)) {
			Char *titleTemp = Malloc(15);

			if (titleTemp != NULL) {
				StrCopy(titleTemp, "Bookmarks");
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
/** Form Object Handler Functions for FormURLs **/
/**--------------------------------------**/

/****************************************************/
/* 
   Function: Boolean ButtonOpen_OH(EventPtr event);
   Description: Button Object Handler for the FormURLs Form
 */
/****************************************************/

/*static*/ Boolean ButtonOpen_OH(EventPtr event, GlobalsType *g)
{
    FormPtr	frm;
    //const	Char *iDialNumberP;
    Char	*iDialNumberP;
 	
	frm = FrmGetActiveForm ();
	g->DbUrlCurrentRecord = LstGetSelection (FrmGetObjectPtr (frm,
                                       FrmGetObjectIndex(frm, listUrlList)));

	dbUrlReadCurrentUrl(g);

	if (StrNCaselessCompare(g->DbUrlCurrentURL.urlstr,"wtai://wp/mc;",13)==0){
		iDialNumberP = malloc(StrLen(g->DbUrlCurrentURL.urlstr)+1);
		StrCopy(iDialNumberP, g->DbUrlCurrentURL.urlstr + 13);
		DisplayDial(iDialNumberP,1);
		return(true);
	}
	else if (StrNCaselessCompare(g->DbUrlCurrentURL.urlstr,"wtai://wp/sd;",13)==0){
		iDialNumberP = malloc(StrLen(g->DbUrlCurrentURL.urlstr)+1);
		StrCopy(iDialNumberP, g->DbUrlCurrentURL.urlstr + 13);
		DisplayDial(iDialNumberP,2);
		return(true);
	}
	else if (StrNCaselessCompare(g->DbUrlCurrentURL.urlstr,"wtai://wp/ap;",13)==0){
		iDialNumberP = malloc(StrLen(g->DbUrlCurrentURL.urlstr)+1);
		StrCopy(iDialNumberP, g->DbUrlCurrentURL.urlstr + 13);
		DisplayDial(iDialNumberP,3);
		return(true);
	}
	else if (StrNCaselessCompare(g->DbUrlCurrentURL.urlstr,"tel:",4)==0){
		if (StrNCaselessCompare(g->DbUrlCurrentURL.urlstr,"tel://",6)==0){
			iDialNumberP = malloc(StrLen(g->DbUrlCurrentURL.urlstr)+1);
			StrCopy(iDialNumberP, g->DbUrlCurrentURL.urlstr + 6);
			DisplayDial(iDialNumberP,1);
			return(true);
		} else {
			iDialNumberP = malloc(StrLen(g->DbUrlCurrentURL.urlstr)+1);
			StrCopy(iDialNumberP, g->DbUrlCurrentURL.urlstr + 4);
			DisplayDial(iDialNumberP,1); 
			return(true);
		}
	}
   
	//OpenNetwork(g);
	g->CurrentView = frmBrowser;
	//FrmGotoForm (g->CurrentView);

	if (g->prevFormID == frmBrowser) { // the user opened from the browser window, so do not reload browser window
		dbUrlReadCurrentUrl(g);

		if (StrNCaselessCompare(g->DbUrlCurrentURL.connection, "- Use Active -", 14)==0)
			dbConnGetConnection(g->ActiveConnection,&(g->conn),g);
   		else
    			dbConnGetConnection(g->DbUrlCurrentURL.connection,&(g->conn),g);

		g->OpenUrl=Malloc(StrLen(g->DbUrlCurrentURL.urlstr)+1);
    			ErrFatalDisplayIf (!g->OpenUrl, "Malloc gUrl");
		//StrCopy(g->OpenUrl,g->DbUrlCurrentURL.urlstr);
		URLCreateURLStr(g->DbUrlCurrentURL.urlstr, g->OpenUrl, g);

		FrmReturnToForm(frmBrowser);
		FrmUpdateForm(frmBrowser, frmUpdateGotoUrl);
	} else {
		g->prevFormID = 0;
		SwitchForm(frmBrowser, g);
	}

	return(true);
}


/****************************************************/
/* 
   Function: Boolean ButtonNew_OH(EventPtr event);
   Description: Button Object Handler for the FormURLs Form
 */
/****************************************************/

/*static*/ Boolean ButtonNew_OH(EventPtr event, GlobalsType *g)
{
	UInt16	index;
	Boolean	created;

	created = dbUrlCreateRecord(g, &index);

	if (created == true) {
    		g->CurrentView = frmEditURL;
    		SwitchForm(frmEditURL, g);//FrmPopupForm(g->CurrentView);
		enableUrlEditOpen(g);
		return(true);
	} else {
		return(false);
	}
}


/****************************************************/
/* 
   Function: Boolean ButtonEdit_OH(EventPtr event);
   Description: Button Object Handler for the FormURLs Form
 */
/****************************************************/

/*static*/ Boolean ButtonEdit_OH(EventPtr event, GlobalsType *g)
{
	FormPtr frm;

   	frm = FrmGetActiveForm ();
   	g->DbUrlCurrentRecord = LstGetSelection (FrmGetObjectPtr (frm,
                                        FrmGetObjectIndex(frm, listUrlList)));
   	g->CurrentView = frmEditURL;
   	SwitchForm(frmEditURL, g);//FrmPopupForm(g->CurrentView);
   	enableUrlEditOpen(g);
   	return(true);
}

/****************************************************/
/* 
   Function: Int16 URLsCategoryTrigger_OH(EventPtr event);
   Description: Pop-up Trigger Object Handler for the FormURLs Form
 */
/****************************************************/

static Boolean URLsCategoryTrigger_OH(EventPtr event, GlobalsType *g)
{
	if (CategorySelect(g->DbUrlDb, FrmGetActiveForm (),
		triggerUrlListCategory, listCategoriesList,
		true, &(g->DbUrlCurrentCategory), g->DbUrlCategoryName, 1, 0)) {
              		g->NewCategoryEdited = true;
      	}
   	InitViewURLs(g);
    	FrmDrawForm(FrmGetActiveForm ());

	{
		Char *titleTemp = Malloc(10);

		if (titleTemp != NULL) {
			StrCopy(titleTemp, "Bookmarks");
			UpdateUserTitle(titleTemp);
			Free(titleTemp);
		}
	}

	return(true);
}


/****************************************************/
/* 
   Function: Int16 ListURL_OH(EventPtr event);
   Description: List Object Handler for the FormURLs Form
 */
/****************************************************/

static Boolean ListURL_OH(EventPtr event)
{

	return(true);
}


/****************************************************/
/* 
   Function: Int16 CategoriesList_OH(EventPtr event);
   Description: List Object Handler for the FormURLs Form
 */
/****************************************************/

static Boolean CategoriesList_OH(EventPtr event)
{

	return(true);
}


