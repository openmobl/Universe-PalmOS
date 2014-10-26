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
#include "about.h"
#include "WAPUniverse.h"
#include "../res/WAPUniverse_res.h"
#include "build.h"
#include "PalmUtils.h"

#ifdef __GNUC__
#include "callbacks.h"
#endif


/***********************************************************************
 *
 *	Updates the scrollers for the help text
 *
 *	Paramaters:		None
 *
 *	Returns:			Nothing
 *
 ***********************************************************************/
static void UpdateScrollers()
{
	UInt16		scrollPos		= 0;
	UInt16		textHeight	= 0;
	UInt16		fieldHeight	= 0;
	UInt16		maxpos			= 0;	

	//get field vals
	FldGetScrollValues (GetObjectPtr(fldInfo), &scrollPos, &textHeight, &fieldHeight);

	//set scroll vals
	if(textHeight > fieldHeight)
	{
		maxpos = textHeight - fieldHeight - 1;
	}
	else
	{
		maxpos = 0;
	}

	//set scrollbar
	SclSetScrollBar (GetObjectPtr(sclInfo), scrollPos, 0, maxpos, fieldHeight);
}

/***********************************************************************
 *
 *	Help form event handler
 *
 *	Paramaters:		eventP : EventPtr - ptr to the event
 *
 *	Returns:			Boolean	- true if handled, false otherwise
 *
 ***********************************************************************/
Boolean ehFormInfo(EventPtr event)
{
    Boolean		handled = false;
    FormPtr		frm;
    UInt16		lines = 0;
    WinDirectionType	dir = winDown;
    UInt16		scrollPos = 0;
    UInt16		textHeight = 0;
    UInt16		fieldHeight = 0;
    ControlPtr		ctl;
    MemHandle		textH = NULL;

#ifdef __GNUC__
    CALLBACK_PROLOGUE
#endif

	//get field ptr
	frm = FrmGetActiveForm();


	switch (event->eType) 
		{

	case frmUpdateEvent:
		FrmDrawForm(FrmGetActiveForm());
		handled = true;
		break;

    case ctlSelectEvent:
		if (event->data.ctlSelect.controlID == buttonInfoOK){
			FldSetTextHandle(GetObjectPtr(fldInfo), NULL);
			FrmReturnToForm(0);//return to calling form
	        }
	  break;

		//form opened			
		case frmOpenEvent:
			textH = DmGetResource('tSTR', strGPL);
			if(textH)
			{
				FldSetTextHandle(GetObjectPtr(fldInfo), textH);
			}
			UpdateScrollers();
            ctl = (ControlPtr)(FrmGetObjectPtr(frm,(FrmGetObjectIndex(frm,lblVersion))));
            CtlSetLabel(ctl,VERSIONSTR2);
			FrmDrawForm (frm);
			break;

		//form closed
		case frmCloseEvent:
			//release MemHandle
			FldSetTextHandle(GetObjectPtr(fldInfo), NULL);
			break;

    // scroll bar used
    case sclRepeatEvent:
    	//scroll as appropriate
    	if(event->data.sclExit.value > event->data.sclRepeat.newValue)
    	{
    		lines = event->data.sclExit.value - event->data.sclRepeat.newValue;
    		dir = winUp;
    	}
			else
			{
    		lines = event->data.sclExit.newValue - event->data.sclRepeat.value;
    		dir = winDown;
			}
			 	
			FldScrollField(GetObjectPtr(fldInfo), lines, dir);
			UpdateScrollers();
			break;

		case fldChangedEvent:
			UpdateScrollers();
			break;

		case keyDownEvent:
			switch (event->data.keyDown.chr)
			{
				case pageUpChr:
					FldGetScrollValues (GetObjectPtr(fldInfo), &scrollPos, &textHeight, &fieldHeight);	
					FldScrollField(GetObjectPtr(fldInfo), fieldHeight - 1, winUp);
					UpdateScrollers();
					handled = true;
					break;

				case pageDownChr:
					FldGetScrollValues (GetObjectPtr(fldInfo), &scrollPos, &textHeight, &fieldHeight);					
					FldScrollField(GetObjectPtr(fldInfo), fieldHeight - 1, winDown);
					UpdateScrollers();
					handled = true;
					break;
			}
			break;

		default:
			break;
		
		}
#ifdef __GNUC__
    CALLBACK_EPILOGUE
#endif
	return handled;
}