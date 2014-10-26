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
#include	"../res/WAPUniverse_res.h"
#include	"callbacks.h"
#include 	"WAPUniverse.h"
#include 	"PalmUtils.h"
#include	"history.h"

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

static void HistoryFormInit(FormPtr frmP, GlobalsType *g);


/***********************************************************************
 *
 *	History form event handler
 *
 *	Paramaters:	event : EventPtr- pointer to the event
 *
 *	Returns:	Boolean		- true if vent was handled, false otherwise
 *
 ***********************************************************************/
Boolean ehFrmHistory(EventPtr event)
{
	Boolean 		handled = false;
  	FormPtr 		frmP = FrmGetActiveForm();
	ListPtr			list = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, listHistory));
	Int16 			lstItem, lstTotal;
	GlobalsType		*g;
	UInt16			selection;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	frmP = FrmGetActiveForm();

	if ( ResizeHandleEvent( event ) )
        	return true;

	switch (event->eType) {
		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID) {
				case buttonHistoryDone:
					FrmReturnToForm(0);
					handled = true;
	        			break;

				case buttonHistoryGo:
					if (LstGetNumberOfItems(list) <= 0)
						break;
					selection = (LstGetSelection(list) - 1) / 2;
					if (((LstGetSelection(list) - 1) & 1) && (LstGetSelection(list) != 0)) {
						selection += 1;
					} else {
						selection += 0;
					}
					HistoryGoToURL(NULL, selection, true, g);
					handled = true;
					break;

				case buttonClearHistory:
					HistoryClearHistory(g);
					FrmUpdateForm(frmHistory, frmRedrawUpdateCode);
					handled = true;
					break;
	    		}
			break;
			
		case frmOpenEvent:
			frmP = FrmGetActiveForm();
			HistoryFormInit(frmP, g);
			FrmDrawForm(frmP);
			handled = true;
			break;

		case frmUpdateEvent:
			frmP = FrmGetActiveForm();
			HistoryFormInit(frmP, g);
			FrmDrawForm(frmP);
			handled = true;
			break;

		case frmCloseEvent:
			handled = true;
			break;

		case keyDownEvent:
        		if ((event->data.keyDown.chr == pageUpChr) && !SysCheck5Way()) {
            			lstItem = LstGetSelection (list);
				lstTotal = LstGetNumberOfItems (list);
				if (lstItem == 0){
					LstSetSelection(list, lstTotal-1);
				} else {
					LstSetSelection(list, lstItem-1);
				}
        		} else if ((event->data.keyDown.chr == pageDownChr) && !SysCheck5Way()) {
            			lstItem = LstGetSelection (list);
				lstTotal = LstGetNumberOfItems (list);
				if (lstItem == lstTotal-1){
					LstSetSelection(list, 0);
				} else {
					LstSetSelection(list, lstItem+1);
				}
        		}
#ifdef HAVE_PALMONE_SDK
			else if ((event->data.keyDown.chr == vchrNavChange) && !SysCheck5Way()) {				
				if(event->data.keyDown.keyCode & navBitSelect){
					if (LstGetNumberOfItems(list) <= 0)
						break;
					selection = (LstGetSelection(list) - 1) / 2;
					if (((LstGetSelection(list) - 1) & 1) && (LstGetSelection(list) != 0)) {
						selection += 1;
					} else {
						selection += 0;
					}
					HistoryGoToURL(NULL, selection, true, g);
				}
			} else if ((event->data.keyDown.chr == keyRockerCenter) && !SysCheck5Way()) {
				if (LstGetNumberOfItems(list) <= 0)
					break;
				selection = (LstGetSelection(list) - 1) / 2;
				if (((LstGetSelection(list) - 1) & 1) && (LstGetSelection(list) != 0)) {
					selection += 1;
				} else {
					selection += 0;
				}
				HistoryGoToURL(NULL, selection, true, g);
			}
#endif
#ifdef HAVE_SONY_SDK
			else if ((event->data.keyDown.chr == vchrJogPush) && !SysCheck5Way()) {
				if (LstGetNumberOfItems(list) <= 0)
					break;
				selection = (LstGetSelection(list) - 1) / 2;
				if (((LstGetSelection(list) - 1) & 1) && (LstGetSelection(list) != 0)) {
					selection += 1;
				} else {
					selection += 0;
				}
				HistoryGoToURL(NULL, selection, true, g);
			}
#endif
			break;

		default:
			break;
		
		}
	
	return handled;
}

void DrawListChars(char* s, RectanglePtr r)
{

    Int16	stringLength = StrLen (s) + 1;
    Int16	pixelWidth = r->extent.x - 10;
    Boolean	truncate;

	// FntCharsInWidth will update stringLength to the 
	// maximum without exceeding the width
	FntCharsInWidth (s, &pixelWidth, &stringLength, &truncate);

	WinDrawChars (s, stringLength, r->topLeft.x+8, r->topLeft.y);
}

static void DrawOneEntryInList(Int16 itemNumber, RectangleType *bounds, Char * * text)
{
	MemHandle	h;
	Int16 		seekAmount = (itemNumber - 1) / 2; // itemNumber;
	UInt16		index = 0;
	GlobalsType	*g;
	Char		*phistory;
	FontPtr		fntPtr;
	FontID		fntID;


#ifdef __GNUC__
  CALLBACK_PROLOGUE
#endif

	if (((itemNumber - 1) & 1) && (itemNumber != 0)) {
		seekAmount += 1;
	} else {
		seekAmount += 0;
	}

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	// must do seek to skip over secret records
	DmSeekRecordInCategory (g->DbHistory, &index, seekAmount, dmSeekForward, dmAllCategories);
	h = DmQueryRecord(g->DbHistory, index);
	if (h) {
      		phistory = MemHandleLock(h);
		
		/*if (StrLen(phistory+StrLen(phistory)+1) > 0)
			DrawListChars(phistory+StrLen(phistory)+1,bounds);
		else
			DrawListChars(phistory,bounds);*/

		if (itemNumber & 1) { // URL
			fntPtr = MemHandleLock(DmGetResource('nfnt', smallNormalFontID));
			FntDefineFont(smallNormalFont, fntPtr);
			fntID = FntSetFont(smallNormalFont);
			MemPtrUnlock(fntPtr);

			DrawListChars(phistory,bounds);

			FntSetFont(fntID);
		} else { // Name
			if (StrLen(phistory+StrLen(phistory)+1) > 0)
				DrawListChars(phistory+StrLen(phistory)+1,bounds);
		}

      		MemHandleUnlock (h);
    	}
#ifdef __GNUC__
  CALLBACK_EPILOGUE
#endif
}

/***********************************************************************
 *
 *	Inits the History form
 *
 *	Paramaters:		frmP : FormPtr	- ptr to history form
 *
 *	Returns:			Nothing
 *
 ***********************************************************************/
static void HistoryFormInit(FormPtr frmP, GlobalsType *g)
{
	FormPtr	frm = FrmGetActiveForm ();
	ListPtr	list = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, listHistory));
	UInt16	numEntries = DmNumRecordsInCategory(g->DbHistory, dmAllCategories);

	LstSetListChoices (list, NULL, (numEntries * 2));
	LstSetDrawFunction (list, DrawOneEntryInList);
}

