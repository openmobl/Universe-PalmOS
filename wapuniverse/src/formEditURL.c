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
#include    	"dbUrl.h"
#include    	"dbConn.h"
#include    	"MenuHnd.h"
#include    	"PalmUtils.h"
#include    	"formEditUrl.h"
#include 	"../res/WAPUniverse_res.h"
#include	"formBrowser.h"
#include	"Form.h"
#include	"debug.h"
#ifdef __GNUC__
#include 	"callbacks.h"
#endif



static Boolean EditURLDoneButton_OH(EventPtr event, GlobalsType *g) SEC_1;
static Boolean TriggerURLConnection_OH(EventPtr event, GlobalsType *g) SEC_1;
static Boolean EditURLCategoryPoptrigger_OH(EventPtr event, GlobalsType *g) SEC_1;
static Boolean EditURLPrivateCheckBox_OH(EventPtr event) SEC_1;
static Boolean EditURLDeleteButton_OH(EventPtr event, GlobalsType *g) SEC_1;

static void URLDetailsInit(GlobalsType *g)
{
  ControlPtr      ctl;
  UInt16          attr;
  Boolean         secret;
  dbUrlPackedURL* purl;
  MemHandle       rechand;
  Int16           seekAmount = g->DbUrlCurrentRecord;
  UInt16          index = 0;

  // must do seek to skip over secret records
  DmSeekRecordInCategory (g->DbUrlDb, &index, seekAmount, dmSeekForward, g->DbUrlCurrentCategory);

  // Get the secret and category attributes of the current record.
  DmRecordInfo (g->DbUrlDb, index, &attr, NULL, NULL);
  secret = attr & dmRecAttrSecret;
  g->RecordCategory = attr & dmRecAttrCategoryMask;

  // If the record is marked secret, turn on the secret checkbox.
  CtlSetValue (GetObjectPtr (chkbxEditURLPrivate), secret);

  rechand = DmQueryRecord (g->DbUrlDb, index);
  purl = MemHandleLock (rechand);
  dbUrlUnPackURL (&g->DbUrlCurrentURL, purl);

  // Set the label of the category trigger.
  CategoryGetName (g->DbUrlDb, g->RecordCategory, g->DbUrlCategoryName);
  ctl = GetObjectPtr (poptriggerEditURLCategory);
  CategorySetTriggerLabel (ctl, g->DbUrlCategoryName);

  // NewCategory indicates which category wants to changed the 
  // current record to.  Initialize it to the current record's current
  // category.
  g->NewCategory = g->RecordCategory;
  g->NewCategoryEdited = false;

  SetFieldFromStr(g->DbUrlCurrentURL.name , fldURLTitle);
  if (StrLen(g->DbUrlCurrentURL.urlstr) == 0)
  	StrCopy(g->DbUrlCurrentURL.urlstr,"http://");
  SetFieldFromStr(g->DbUrlCurrentURL.urlstr , fldURL);

  MemHandleUnlock (rechand);
  dbConnGetConnectionsList(g);

}

static UInt16 URLDetailsApply (GlobalsType *g)
{
  	UInt16        	attr;
  	Boolean       	secret;
  	UInt16		updateCode = 0;
  	Boolean		dirty = false;    // P11. If an attribute changed
  	dbUrlURL	tURL;
  	MemHandle     	currec;
  	FormPtr       	frm;
  	FieldPtr      	nameFldPtr, urlFldPtr;
	Char		*nameTxtPtr, *urlTxtPtr;
  	Int16         	seekAmount = g->DbUrlCurrentRecord;
  	UInt16        	index = 0;
  	Char 		*c;

	// Get Field Ptr
	frm = FrmGetFormPtr (frmEditURL);
	// Name field
  	nameFldPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldURLTitle))));
	nameTxtPtr = FldGetTextPtr(nameFldPtr);
	
	// URL field
  	urlFldPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldURL))));
	urlTxtPtr = FldGetTextPtr(urlFldPtr);

	if (StrLen(nameTxtPtr) <= 0) {
		MyErrorFunc("A name for this bookmark is required", NULL);
		updateCode |= updateGeneralError;
		return updateCode;
	}

	if (StrLen(urlTxtPtr) <= 0) {
		MyErrorFunc("A URL for this bookmark is required", NULL);
		updateCode |= updateGeneralError;
		return updateCode;
	}
	

  	// must do seek to skip over secret records
  	DmSeekRecordInCategory (g->DbUrlDb, &index, seekAmount, dmSeekForward, g->DbUrlCurrentCategory);

  	// Get the secret and category attributes of the current record.
  	DmRecordInfo (g->DbUrlDb, index, &attr, NULL, NULL);

  	
  	if (StrLen(nameTxtPtr) > 0)
    		StrCopy (tURL.name, nameTxtPtr);

  	if (StrLen(urlTxtPtr) > 0){
    		StrCopy (tURL.urlstr, urlTxtPtr);
		while((c=StrChr(tURL.urlstr,0x0A))){  // remove CR/LF
			StrCopy(c,c+1);
		}
  	}
  	StrCopy (tURL.connection,g->EditURLConnection);

  	currec = DmGetRecord (g->DbUrlDb, index);
  	dbUrlPackURL (&tURL, currec);
  	DmReleaseRecord (g->DbUrlDb, index, true);

  	// Get the current setting of the secret checkbox.
  	secret = (CtlGetValue (GetObjectPtr (chkbxEditURLPrivate)) != 0);

  	// Has the secret attribute been changed?
  	if (((attr & dmRecAttrSecret) == dmRecAttrSecret) != secret) {
      		// Yes, it was changed.  What is the new setting?
      		if (secret)
        		// Now it should be secret.
        		attr |= dmRecAttrSecret;
      		else
        		// Now it should not be secret.
        		attr &= ~dmRecAttrSecret;

      		dirty = true;
    	}


  	// P11. Has the category been changed?
  	if (g->RecordCategory != g->NewCategory) {
      		attr &= ~dmRecAttrCategoryMask; // Remove all category bits.
      		attr |= g->NewCategory;      // Set the new category
      		dirty = true;             // mark this record dirty later
      		g->RecordCategory = g->NewCategory;     // remember which category the record 
                                        // is in
      		updateCode |= updateCategoryChanged;  // send this update code
    	}


  	// P11. If the current category was deleted, renamed, or merged with
  	// another category, then the list view needs to be redrawn.
  	if (g->NewCategoryEdited) {
      		g->RecordCategory = g->NewCategory;
      		updateCode |= updateCategoryChanged;
    	}


  	if (dirty) {
      		// Set the dirty flag for the record.
      		attr |= dmRecAttrDirty;
      		DmSetRecordInfo (g->DbUrlDb, index, &attr, NULL);
    	}

  	dbUrlSortRecord(g->DbUrlDb, &index);
    
    NVFSSyncDatabase(g->DbUrlDb);

  return updateCode;
}


/**----------------------------------**/
/** Form Event Handler for FormEditURL**/
/**----------------------------------**/

/****************************************************/
/* 
   Function: Boolean FormEditURL_EH(EventPtr event);
   Description: Main Form Event Handler for FormEditURL
 */
/****************************************************/


/* Form Event Handler */
Boolean ehFormEditURL(EventPtr event)
{
    Boolean handled = false;
	GlobalsType *g;
	
#ifdef __GNUC__
    CALLBACK_PROLOGUE
#endif

  	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
    switch(event->eType) {

        case ctlSelectEvent:
            if (event->data.ctlSelect.controlID == buttonEditURLDone) {	
                handled = EditURLDoneButton_OH(event,g); }
            if (event->data.ctlSelect.controlID == buttonEditURLDelete) {
                handled = EditURLDeleteButton_OH(event,g); }
            if (event->data.ctlSelect.controlID == chkbxEditURLPrivate) {
                handled = EditURLPrivateCheckBox_OH(event); }
            if (event->data.ctlSelect.controlID == poptriggerEditURLCategory) {
                handled = EditURLCategoryPoptrigger_OH(event,g); }
			break;

        case lstSelectEvent:
        case popSelectEvent:
            if (event->data.popSelect.controlID == poptriggerEditURLCategory) {
                handled = EditURLCategoryPoptrigger_OH(event,g); }
            if (event->data.popSelect.controlID == triggerURLConnection) {
                handled = TriggerURLConnection_OH(event,g); }
            break;

	case frmUpdateEvent:
	   FrmDrawForm(FrmGetActiveForm());
	   handled = true;
	   break;

        case frmOpenEvent:
           URLDetailsInit(g);
           FrmDrawForm(FrmGetActiveForm());
	   FrmSetFocus (FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fldURLTitle));
           handled = true;
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

/* Form Event Handler */
Boolean AddBookmarkFormEventHandler(EventPtr event)
{
	Boolean 	handled = false;
	GlobalsType 	*g;
	MemHandle       currec;
	Char		*name = NULL, *url = NULL;
	FieldPtr	fld;
	FormType 	*form = FrmGetActiveForm();
	Int16		nameLen = 0, urlLen = 0;
	Char		*tempName = NULL;
	UInt16		index = 0;
	Boolean		created = false;
	
#ifdef __GNUC__
    CALLBACK_PROLOGUE
#endif

  	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
	switch(event->eType) {

		case ctlSelectEvent:
	    		switch (event->data.ctlSelect.controlID) {
				case buttonBookmarkAdd:
					fld = (FieldPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,fieldBookmarkName))));
						name = FldGetTextPtr(fld);
						nameLen = FldGetTextLength(fld);
					fld = (FieldPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,fieldBookmarkURL))));
						url = FldGetTextPtr(fld);
						urlLen = FldGetTextLength(fld);
					if ((!name || !url) || (nameLen == 0 || urlLen == 0)) {
						FrmCustomAlert(alHTTPError, "A name or address must be entered. Please go back and enter one or hit cancel.", NULL, NULL);
						break;
					}

					{
						g->DbUrlCurrentCategory = dmUnfiledCategory;
						
						debugOut("AddBkmrk", "create database", 0, __FILE__, __LINE__, g);

						created = dbUrlCreateRecord(g, &index);
						if (created != true) {
							MyErrorFunc("Could not create bookmark entry. Database error.", NULL);
							debugOut("AddBkmrk", "create database err", 0, __FILE__, __LINE__, g);
							FrmReturnToForm(0);
						}

						if (name && (StrLen(name) < 30)) {
							StrCopy(g->DbUrlCurrentURL.name, name);
						} else if (name && (StrLen(name) > 30)) {
							StrNCopy(g->DbUrlCurrentURL.name, name, 29); 
							g->DbUrlCurrentURL.name[29] = chrNull; 
						} 

						//else
						//	StrCopy(g->DbUrlCurrentURL.name, "Untitled");
						StrCopy (g->DbUrlCurrentURL.urlstr, (url)?url:g->Url);
						StrCopy (g->DbUrlCurrentURL.connection, g->ActiveConnection);
						debugOut("AddBkmrk", "get record", 0, __FILE__, __LINE__, g);
						currec = DmGetRecord (g->DbUrlDb, 0);
						debugOut("AddBkmrk", "got record, pack url", 0, __FILE__, __LINE__, g);
						dbUrlPackURL (&g->DbUrlCurrentURL, currec);
						debugOut("AddBkmrk", "packed url, release", 0, __FILE__, __LINE__, g);
						DmReleaseRecord (g->DbUrlDb, 0, true);
						debugOut("AddBkmrk", "release, now sort", 0, __FILE__, __LINE__, g);

						dbUrlSortRecord(g->DbUrlDb, &index);
						debugOut("AddBkmrk", "sorted", 0, __FILE__, __LINE__, g);
					}
					FrmReturnToForm(0);
					break;
				case buttonBookmarkCancel:
					FrmReturnToForm(0);
					break;
			}
            		break;

		case frmUpdateEvent:
	   		FrmDrawForm(FrmGetActiveForm());
	   		break;

        	case frmOpenEvent:
           		FrmDrawForm(FrmGetActiveForm());
			tempName = Malloc(9);
			if (tempName) StrCopy(tempName, "Untitled");

			SetFieldFromStr((g->deckTitle) ? g->deckTitle:tempName, fieldBookmarkName);
			SetFieldFromStr(g->Url, fieldBookmarkURL);

	   		FrmSetFocus (FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fieldBookmarkName));

			if (tempName) Free(tempName);
           		handled = true;
           		break;

        	case menuEvent:
    	   		break;

		default:
	   		break;


	}
#ifdef __GNUC__
    CALLBACK_EPILOGUE
#endif
    	return handled;
}

/*static Int16 PopupAddBookmark(Char *name, Char *url, GlobalsType *g)
{
	//Err 		err;
	FormType 	*form;
	UInt16 		buttonHit;
	FieldPtr      	fld;

	

	form = FrmInitForm(frmAddBookmark);

	//fld = (FieldPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,fieldBookmarkName))));
	//SetFieldTextFromStr(fld, (g->deckTitle) ? g->deckTitle:"Untitled", false);
	//fld = (FieldPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,fieldBookmarkURL))));
	//SetFieldTextFromStr(fld, g->Url, false);

	//SetFieldFromStr((g->deckTitle) ? g->deckTitle:"Untitled", fieldBookmarkName);
	//SetFieldFromStr(g->Url, fieldBookmarkURL);
  
  	buttonHit = FrmDoDialog(form);
  	
  	FrmDeleteForm(form);
  	
  	if (buttonHit == buttonBookmarkAdd)
  	{			  
		fld = (FieldPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,fieldBookmarkName))));
			name = Malloc(StrLen(FldGetTextPtr(fld))+1);
			if (!name) { name = NULL; return buttonHit; }
			StrCopy(name, FldGetTextPtr(fld));
		fld = (FieldPtr)(FrmGetObjectPtr(form,(FrmGetObjectIndex(form,fieldBookmarkURL))));
			url = Malloc(StrLen(FldGetTextPtr(fld))+1);
			if (!url) { url = NULL; return buttonHit; }
			StrCopy(url, FldGetTextPtr(fld));
	}

	return buttonHit;
}

void addBookmark (GlobalsType *g)
{
  MemHandle	currec;
  Char		*name = NULL, *url = NULL;

	if (PopupAddBookmark(name, url, g) == buttonBookmarkAdd) {
		g->DbUrlCurrentCategory = dmUnfiledCategory;
		dbUrlCreateRecord (g);
		// must do seek to skip over secret records
		StrCopy(g->DbUrlCurrentURL.name, (name)?name:((g->deckTitle)?g->deckTitle:"Untitled"));

		StrCopy (g->DbUrlCurrentURL.urlstr, (url)?url:g->Url);
		StrCopy (g->DbUrlCurrentURL.connection, g->ActiveConnection);
		currec = DmGetRecord (g->DbUrlDb, 0);
		dbUrlPackURL (&g->DbUrlCurrentURL, currec);
		DmReleaseRecord (g->DbUrlDb, 0, true);

		if (name) Free(name);
		if (url) Free(url);
	}
	return;
}*/

void addBookmark (GlobalsType *g)
{
  /*MemHandle        	currec;

	g->DbUrlCurrentCategory = dmUnfiledCategory;
	dbUrlCreateRecord (g);
	// must do seek to skip over secret records
	if (g->deckTitle)
		StrCopy(g->DbUrlCurrentURL.name, g->deckTitle);
	else
		StrCopy(g->DbUrlCurrentURL.name, "Untitled");
	StrCopy (g->DbUrlCurrentURL.urlstr, g->Url);
	StrCopy (g->DbUrlCurrentURL.connection, g->ActiveConnection);
	currec = DmGetRecord (g->DbUrlDb, 0);
	dbUrlPackURL (&g->DbUrlCurrentURL, currec);
	DmReleaseRecord (g->DbUrlDb, 0, true);*/
	FrmPopupForm(frmAddBookmark);
	return;
}

void setHome (GlobalsType *g)
{
  UInt16            		attr;
  dbConnPackedConnection*	pconn;
  MemHandle        			rechand;
  Int16             		seekAmount = g->ActiveConnStore;
  UInt16            		index = 0;
  dbConnConnection 	tConn;
  MemHandle        	currec;

	  FormInputDeactivate(g);
	  dbConnReadCurrentConnection (g);
	//Open Conn DB
	// must do seek to skip over secret records
	  DmSeekRecordInCategory (g->DbConnDb, &index, seekAmount, dmSeekForward, dmAllCategories);

	// Get the secret and category attributes of the current record.
	  DmRecordInfo (g->DbConnDb, index, &attr, NULL, NULL);
	//  gRecordCategory = attr & dmRecAttrCategoryMask;

	  rechand = DmQueryRecord (g->DbConnDb, index);
	  pconn = MemHandleLock (rechand);
	  dbConnUnPackConnection (&(g->CurrentConnection), pconn);
	//Close Conn DB
	  //DmSeekRecordInCategory (g->DbConnDb, &index, seekAmount, dmSeekForward, dmAllCategories);

	// Get the secret and category attributes of the current record.
	  DmRecordInfo (g->DbConnDb, index, &attr, NULL, NULL);
  	 
	  StrCopy (tConn.name, g->CurrentConnection.name);
  	  StrCopy (tConn.ipaddress, g->CurrentConnection.ipaddress);
	  StrCopy (tConn.home, g->Url); 
	  StrCopy (tConn.port, g->CurrentConnection.port);
	  StrCopy (tConn.proxyuser, g->CurrentConnection.proxyuser);
      	  StrCopy (tConn.proxypass, g->CurrentConnection.proxypass);
  	  //tConn.security = 'N';
  	  tConn.connectionType = g->CurrentConnection.connectionType;
  	  tConn.bearer = 'C';
  	  
  	  currec = DmGetRecord (g->DbConnDb, index);
	  dbConnPackConnection (&tConn, currec);
	  DmReleaseRecord (g->DbConnDb, index, true);
	  //dbConnSortRecord (g->DbConnDb, &index);
	  //MyErrorFunc("internal:home-edit not supported."," ");
	  dbConnReadCurrentConnection (g);
	  followLink(g->CurrentConnection.home, g);
  	  return;
}


/* Item Handler Functions Here */

/**--------------------------------------**/
/** Form Object Handler Functions for FormEditURL **/
/**--------------------------------------**/

/****************************************************/
/* 
   Function: Int16 EditURLDoneButton_OH(EventPtr event);
   Description: Button Object Handler for the FormEditURL Form
 */
/****************************************************/

static Boolean EditURLDoneButton_OH(EventPtr event, GlobalsType *g)
{
	UInt16		bookmarkError = 0;

    	bookmarkError = URLDetailsApply(g);
	if (!(bookmarkError & updateGeneralError)) {
   		dbConnDisposeConnectionsList(g);
    		//FrmCloseAllForms ();
    		SwitchForm(frmUrlList, g); //FrmReturnToForm(0);//FrmGotoForm (frmUrlList);
		return(true);
	} else {
		return(false);
	}
}


/****************************************************/
/* 
   Function: Int16 EditURLDeleteButton_OH(EventPtr event);
   Description: Button Object Handler for the FormEditURL Form
 */
/****************************************************/

static Boolean EditURLDeleteButton_OH(EventPtr event, GlobalsType *g)
{
FieldPtr ipPtr;
FormPtr frm;

    frm = FrmGetFormPtr (frmEditURL);
    ipPtr = (FieldPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex (frm, fldURLTitle))));


    switch (FrmCustomAlert (alDelete, "bookmark", FldGetTextPtr (ipPtr), "")) {
	case 1:
   		if (dbUrlDeleteCurrentRecord (g))
   		{
       			// Delete was completed.  Close the details and the edit forms.
       			dbConnDisposeConnectionsList(g);
       			//FrmCloseAllForms ();

       			// Go to the main form.
       			SwitchForm(frmUrlList, g); //FrmReturnToForm(0); //FrmGotoForm(frmUrlList);
   			return(true);
   		}
	default:
		return(false);
		break;
    }
}


/****************************************************/
/* 
   Function: Int16 EditURLPrivateCheckBox_OH(EventPtr event);
   Description: CheckBox Object Handler for the FormEditURL Form
 */
/****************************************************/

static Boolean EditURLPrivateCheckBox_OH(EventPtr event)
{

	return(0);
}


/****************************************************/
/* 
   Function: Int16 EditURLCategoryPoptrigger_OH(EventPtr event);
   Description: Pop-up Trigger Object Handler for the FormEditURL Form
 */
/****************************************************/

static Boolean EditURLCategoryPoptrigger_OH(EventPtr event, GlobalsType *g)
{
          if (CategorySelect (g->DbUrlDb, FrmGetActiveForm (),
                              poptriggerEditURLCategory, listEditURLCategoriesList,
                              true, &(g->NewCategory), g->DbUrlCategoryName, 1, 0))
            {
              g->NewCategoryEdited = true;
            }
  
	return(true);
}


/****************************************************/
/* 
   Function: Int16 TriggerURLConnection_OH(EventPtr event);
   Description: Pop-up Trigger Object Handler for the FormEditURL Form
 */
/****************************************************/

static Boolean TriggerURLConnection_OH(EventPtr event, GlobalsType *g)
{
    ControlPtr      ctlPtr;
    FormPtr         frm;

    g->EditURLConnection = LstGetSelectionText(event->data.popSelect.listP,
                            event->data.popSelect.selection); 
    frm = FrmGetFormPtr (frmEditURL);
    ctlPtr =(ControlPtr)(FrmGetObjectPtr(frm,(FrmGetObjectIndex(frm,triggerURLConnection))));
    CtlSetLabel(ctlPtr,g->EditURLConnection);
	return(true);
}


