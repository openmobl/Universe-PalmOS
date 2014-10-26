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
#include    	"../res/WAPUniverse_res.h"
#include 	"WAPUniverse.h"

static void ScrollFrmScroll (FormPtr frm, WinDirectionType direction) SEC_1;
void ScrollFrm (Char * title, MemHandle bodyHandle) SEC_1;

static void ScrollFrmScroll (FormPtr frm, WinDirectionType direction)
{
	UInt16 upIndex;
	UInt16 downIndex;
	UInt16 linesToScroll;
	Boolean enableUp;
	Boolean enableDown;
	FieldPtr fld;

	fld = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, scrllField));
	linesToScroll = FldGetVisibleLines (fld) - 1;
	FldScrollField (fld, linesToScroll, direction);

	enableUp = FldScrollable (fld, winUp);
	enableDown = FldScrollable (fld, winDown);
	upIndex = FrmGetObjectIndex (frm, buttonScrollUp);
	downIndex = FrmGetObjectIndex (frm, buttonScrollDown);

	FrmUpdateScrollers (frm, upIndex, downIndex, enableUp, enableDown);
}


void ScrollFrm (Char * title, MemHandle bodyHandle)
{
	UInt16 upIndex;
	UInt16 downIndex;
	FormPtr frm;
	FieldPtr fld;
	Boolean enableUp;
	Boolean enableDown;
	Boolean done = false;
	EventType event;
	FormPtr curForm;
	
	curForm = FrmGetActiveForm();		// save active form/window state

	InsPtEnable (false);

	frm = FrmInitForm (frmScroll);
	FrmSetTitle(frm,title);

	fld = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, scrllField));
	FldSetTextHandle (fld, (MemHandle)bodyHandle);

//	FrmEraseForm (curForm);
	FrmDrawForm (frm);
	FrmSetActiveForm (frm);
	
	upIndex = FrmGetObjectIndex (frm, buttonScrollUp);
	downIndex = FrmGetObjectIndex (frm, buttonScrollDown);

	enableUp = FldScrollable (fld, winUp);
	enableDown = FldScrollable (fld, winDown);
	FrmUpdateScrollers (frm, upIndex, downIndex, enableUp, enableDown);
	
	while (! done)
		{
		EvtGetEvent (&event, evtWaitForever);
		if (! SysHandleEvent (&event))
			FrmHandleEvent (frm, &event);

		if (event.eType == ctlSelectEvent)
			{
			switch (event.data.ctlSelect.controlID)
				{
				case buttonScrollUp:
					ScrollFrmScroll (frm, winUp);
					break;
					
				case buttonScrollDown:
					ScrollFrmScroll (frm, winDown);
					break;
				case buttonScrollDone:
					done = true;
				}
			if (event.data.ctlSelect.controlID == buttonScrollDone)
				done = true;
			}
					
		else if (event.eType == ctlRepeatEvent)
			{
			switch (event.data.ctlSelect.controlID)
				{
				case buttonScrollUp:
					ScrollFrmScroll (frm, winUp);
					break;
					
				case buttonScrollDown:
					ScrollFrmScroll (frm, winDown);
					break;
				}
			}


		else if (event.eType == keyDownEvent)
			{
			switch (event.data.keyDown.chr)
				{
				case pageUpChr:
					ScrollFrmScroll(frm, winUp);
					break;
					
				case pageDownChr:
					ScrollFrmScroll (frm, winDown);
					break;
				}
			}


		else if (event.eType == appStopEvent)
			{
			EvtAddEventToQueue (&event);
			break;
			}
		}

	FldSetTextHandle (fld, 0);
	FrmEraseForm (frm);
	FrmDeleteForm (frm);
	
//	FrmDrawForm(curForm);
	FrmSetActiveForm(curForm);		

}

