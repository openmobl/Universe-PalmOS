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
#include	"PalmUtils.h"
#include	"frmPrefs.h"
#include 	"WAPUniverse.h"
#include 	"../res/WAPUniverse_res.h"
#include    	"MenuHnd.h"
#include	"Cache.h"
#include	"history.h"
#include	"http.h"
#include	"variable.h"
#ifdef __GNUC__
#include 	"callbacks.h"
#endif
#include	"Cookie.h"


//SetFieldTextFromHandle
extern FieldPtr SetFieldTextFromHandle(UInt16 fieldID, MemHandle txtH)
{
	MemHandle oldTxtH;
	FormPtr frm = FrmGetActiveForm();
	FieldPtr fldP;
	
	fldP = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldID));
	ErrNonFatalDisplayIf(!fldP, "Missing field");
	oldTxtH = FldGetTextHandle(fldP);

	// set the text	
	FldSetTextHandle(fldP, txtH);

	// the following line has been commented out to prevent drawing the
	// field before the form has actually been drawn. Lets see how it goes! 
	//	FldDrawField(fldP);
	
	if (oldTxtH)
	{
		MemHandleFree(oldTxtH);
	}
		
	return fldP;
}

void DrawSampleFont(UInt16 x, UInt16 y, UInt16 font)
{
	Char		sample[25];
	UInt16		len = 0;
	RectangleType	rect = {{x,y},{0,0}};
	FontID		currFont = FntGetFont();
	FontPtr 	fntPtr;
	FontID		fontid = 0;
	UInt16		resid = 0;


	StrCopy(sample, "This is a sample");
	sample[16] = '\0';

	len = StrLen(sample);

	rect.extent.x = 20;
	rect.extent.y = 13;

	if (font == 1) {
		fontid = smallSmallFont;
		resid = smallSmallFontID;

		fntPtr = MemHandleLock(DmGetResource('nfnt', resid));
		FntDefineFont(fontid, fntPtr);
		currFont = FntSetFont(fontid);
		MemPtrUnlock(fntPtr);
	} else if (font == 2) {
		currFont = FntSetFont(boldFont);
	}
	

	WinEraseRectangle(&rect, 0);
	WinDrawChars(sample, len, x, y);

	FntSetFont(currFont);
}

//PrefsFormInit
static void PrefsFormInit(FormType * frmP)
{
//MemHandle txtH;
ControlType *controlP;

	
	/*//set history option
	if (g_prefs.history == true)
		CtlSetValue (GetObjectPtr(checkPrefHistory), 1);
	else
		CtlSetValue (GetObjectPtr(checkPrefHistory), 0);
	
	//set disconnect on exit
	if (g_prefs.disconnect == true)
		CtlSetValue (GetObjectPtr(checkPrefDis), 1);
	else
		CtlSetValue (GetObjectPtr(checkPrefDis), 0);
	
	//set Prompt at start
	if (g_prefs.open == true)
		CtlSetValue (GetObjectPtr(checkPrefPromptOpen), 1);
	else
		CtlSetValue (GetObjectPtr(checkPrefPromptOpen), 0);
	
	//set Prompt at end
	if (g_prefs.close == true)
		CtlSetValue (GetObjectPtr(checkPrefPromptClose), 1);
	else
		CtlSetValue (GetObjectPtr(checkPrefPromptClose), 0);

	//set Start with
	if (g_prefs.start == 0)
		LstSetSelection (GetObjectPtr(listPrefStart), 0);
	else
		LstSetSelection (GetObjectPtr(listPrefStart), 1);
	controlP = GetObjectPtr(triggerPrefStart);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listPrefStart), g_prefs.start) );
	
	//set Agent
	//CtlSetValue (GetObjectPtr(checkPrefAltAgent), g_prefs.agent);
	if (g_prefs.agent == 0)
		LstSetSelection (GetObjectPtr(listPrefAltAgent), 0);
	else if (g_prefs.agent == 1)
		LstSetSelection (GetObjectPtr(listPrefAltAgent), 1);
	else if (g_prefs.agent == 2)
		LstSetSelection (GetObjectPtr(listPrefAltAgent), 2);
	else if (g_prefs.agent == 3) {
		LstSetSelection (GetObjectPtr(listPrefAltAgent), 3);
		FrmShowObject (FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fieldPrefAgent));
	} else
		LstSetSelection (GetObjectPtr(listPrefAltAgent), 0);
	controlP = GetObjectPtr(triggerPrefAgent);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listPrefAltAgent), g_prefs.agent) );
	

		txtH = MemHandleNew(StrLen(g_prefs.CustomAgent) + 1);
		if (txtH)
		{
			StrCopy(MemHandleLock(txtH), g_prefs.CustomAgent);
		
			MemHandleUnlock(txtH);

			SetFieldTextFromHandle(fieldPrefAgent, txtH);
		}*/

	if (g_prefs.timeout == 10)
		LstSetSelection (GetObjectPtr(listPrefTimeout), 0);
	else if (g_prefs.timeout == 30)
		LstSetSelection (GetObjectPtr(listPrefTimeout), 1);
	else if (g_prefs.timeout == 60)
		LstSetSelection (GetObjectPtr(listPrefTimeout), 2);
	else if (g_prefs.timeout == 90)
		LstSetSelection (GetObjectPtr(listPrefTimeout), 3);
	else if (g_prefs.timeout == 120)
		LstSetSelection (GetObjectPtr(listPrefTimeout), 4);
	else if (g_prefs.timeout == 240)
		LstSetSelection (GetObjectPtr(listPrefTimeout), 5);
	else
		LstSetSelection (GetObjectPtr(listPrefTimeout), 2);
	controlP = GetObjectPtr(triggerPrefTimeout);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listPrefTimeout), g_prefs.timeoutlist) );

	/*//set Image border
	if (g_prefs.imgBrdr == true)
		CtlSetValue (GetObjectPtr(checkImgBrdr), 1);
	else
		CtlSetValue (GetObjectPtr(checkImgBrdr), 0);

	//set Image Resize
	if (g_prefs.imgResize == true)
		CtlSetValue (GetObjectPtr(checkImgResize), 1);
	else
		CtlSetValue (GetObjectPtr(checkImgResize), 0);*/

	//Alt Agent
	if (g_prefs.altAgent == true) {
		CtlSetValue (GetObjectPtr(checkAltAgent), 1);
		//FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),triggerPrefUA));
		LstSetSelection(GetObjectPtr(listPrefUA), g_prefs.agentNum);

		controlP = GetObjectPtr(triggerPrefUA);
		CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listPrefUA), g_prefs.agentNum) );
	} else {
		CtlSetValue (GetObjectPtr(checkAltAgent), 0);
		//FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),triggerPrefUA));
		LstSetSelection(GetObjectPtr(listPrefUA), g_prefs.agentNum);

		controlP = GetObjectPtr(triggerPrefUA);
		CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listPrefUA), g_prefs.agentNum) );
	}

	//display images
	if (g_prefs.images == true)
		CtlSetValue (GetObjectPtr(checkImages), 1);
	else
		CtlSetValue (GetObjectPtr(checkImages), 0);

	//mode
	if (g_prefs.mode == true)
		CtlSetValue (GetObjectPtr(checkMode), 1);
	else
		CtlSetValue (GetObjectPtr(checkMode), 0);

	{
		Char	size[maxStrIToALen];
	
		StrIToA(size, g_prefs.maxCache / 1024);

		SetFieldFromStr(size, fieldCacheSize);
	}

	controlP = GetObjectPtr(triggerPrefFont);
	if (g_prefs.fontSize == 0) {
		LstSetSelection (GetObjectPtr(listPrefFont), 1);
		CtlSetLabel( controlP, LstGetSelectionText(GetObjectPtr(listPrefFont), 1) );
	} else if (g_prefs.fontSize == 1) {
		LstSetSelection (GetObjectPtr(listPrefFont), 0);
		CtlSetLabel( controlP, LstGetSelectionText(GetObjectPtr(listPrefFont), 0) );
	} else if (g_prefs.fontSize == 2) {
		LstSetSelection (GetObjectPtr(listPrefFont), 2);
		CtlSetLabel( controlP, LstGetSelectionText(GetObjectPtr(listPrefFont), 2) );
	} else {
		LstSetSelection (GetObjectPtr(listPrefFont), 1);
		CtlSetLabel( controlP, LstGetSelectionText(GetObjectPtr(listPrefFont), 1) );
	}

	controlP = GetObjectPtr(triggerPrefCookie);
	if (g_prefs.cookie == COOKIE_PREF_ACCEPT) {
		LstSetSelection (GetObjectPtr(listPrefCookie), 0);
		CtlSetLabel( controlP, LstGetSelectionText(GetObjectPtr(listPrefCookie), 0) );
	} else if (g_prefs.cookie == COOKIE_PREF_ASK) {
		LstSetSelection (GetObjectPtr(listPrefCookie), 1);
		CtlSetLabel( controlP, LstGetSelectionText(GetObjectPtr(listPrefCookie), 1) );
	} else if (g_prefs.cookie == COOKIE_PREF_DECLINE) {
		LstSetSelection (GetObjectPtr(listPrefCookie), 2);
		CtlSetLabel( controlP, LstGetSelectionText(GetObjectPtr(listPrefCookie), 2) );
	} else {
		LstSetSelection (GetObjectPtr(listPrefCookie), 1);
		CtlSetLabel( controlP, LstGetSelectionText(GetObjectPtr(listPrefCookie), 1) );
	}

	LstSetSelection (GetObjectPtr(listPrefStartLoc), g_prefs.startDest);
	controlP = GetObjectPtr(triggerPrefStartLoc);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listPrefStartLoc), g_prefs.startDest) );

	//set Debug
	if (g_prefs.debug == true)
		CtlSetValue(GetObjectPtr(checkDebug), 1);
	else
		CtlSetValue(GetObjectPtr(checkDebug), 0);

	if (g_prefs.fiveWayMode == FIVEWAY_MODE_POINTER)
		CtlSetValue(GetObjectPtr(checkPointer), 1);
	else
		CtlSetValue(GetObjectPtr(checkPointer), 0);

	LstSetSelection (GetObjectPtr(listFWSpeed), g_prefs.fiveWaySpeed - 1);
	controlP = GetObjectPtr(triggerFWSpeed);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listFWSpeed), g_prefs.fiveWaySpeed - 1) );
	
	LstSetSelection (GetObjectPtr(listFWAccel), g_prefs.fiveWayAccel);
	controlP = GetObjectPtr(triggerFWAccel);
	CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listFWAccel), g_prefs.fiveWayAccel) );

	if (g_prefs.fiveWayMode != FIVEWAY_MODE_POINTER) {
		FrmHideObject(frmP, FrmGetObjectIndex(frmP, labelFWSpeed));
		FrmHideObject(frmP, FrmGetObjectIndex(frmP, triggerFWSpeed));
		FrmHideObject(frmP, FrmGetObjectIndex(frmP, labelFWAccel));
		FrmHideObject(frmP, FrmGetObjectIndex(frmP, triggerFWAccel));
	}

	if (g_prefs.disableUAProf == true)
		CtlSetValue(GetObjectPtr(checkDisableUAProf), 1);
	else
		CtlSetValue(GetObjectPtr(checkDisableUAProf), 0);

	PrefsSwitchView(tabGeneral);
}

void PrefsSwitchView(Int16 view)
{
	FormPtr	frm = FrmGetActiveForm();

	FrmHideObject(frm, FrmGetObjectIndex(frm, labelPrefTime));
	FrmHideObject(frm, FrmGetObjectIndex(frm, labelPrefSec));
	FrmHideObject(frm, FrmGetObjectIndex(frm, triggerPrefTimeout));
	FrmHideObject(frm, FrmGetObjectIndex(frm, labelPrefStartLoc));
	FrmHideObject(frm, FrmGetObjectIndex(frm, triggerPrefStartLoc));
	FrmHideObject(frm, FrmGetObjectIndex(frm, checkImages));
	FrmHideObject(frm, FrmGetObjectIndex(frm, checkMode));
	FrmHideObject(frm, FrmGetObjectIndex(frm, checkPointer));
	FrmHideObject(frm, FrmGetObjectIndex(frm, labelFWSpeed));
	FrmHideObject(frm, FrmGetObjectIndex(frm, triggerFWSpeed));
	FrmHideObject(frm, FrmGetObjectIndex(frm, labelFWAccel));
	FrmHideObject(frm, FrmGetObjectIndex(frm, triggerFWAccel));

	FrmHideObject(frm, FrmGetObjectIndex(frm, labelCacheSize));
	FrmHideObject(frm, FrmGetObjectIndex(frm, fieldCacheSize));
	FrmHideObject(frm, FrmGetObjectIndex(frm, labelCacheKB));
	FrmHideObject(frm, FrmGetObjectIndex(frm, buttonCacheClear));
	FrmHideObject(frm, FrmGetObjectIndex(frm, labelPrefCookie));
	FrmHideObject(frm, FrmGetObjectIndex(frm, triggerPrefCookie));
	FrmHideObject(frm, FrmGetObjectIndex(frm, labelPrefFont));
	FrmHideObject(frm, FrmGetObjectIndex(frm, triggerPrefFont));


	FrmHideObject(frm, FrmGetObjectIndex(frm, checkAltAgent));
	FrmHideObject(frm, FrmGetObjectIndex(frm, triggerPrefUA));
	FrmHideObject(frm, FrmGetObjectIndex(frm, checkDisableUAProf));
	FrmHideObject(frm, FrmGetObjectIndex(frm, checkDebug));
	switch (view) {
		case tabPage:
			CtlSetValue(GetObjectPtr(tabGeneral), 0);
			CtlSetValue(GetObjectPtr(tabPage), 1);
			CtlSetValue(GetObjectPtr(tabAdvanced), 0);

			FrmShowObject(frm, FrmGetObjectIndex(frm, labelCacheSize));
			FrmShowObject(frm, FrmGetObjectIndex(frm, fieldCacheSize));
			FrmShowObject(frm, FrmGetObjectIndex(frm, labelCacheKB));
			FrmShowObject(frm, FrmGetObjectIndex(frm, buttonCacheClear));
			FrmShowObject(frm, FrmGetObjectIndex(frm, labelPrefCookie));
			FrmShowObject(frm, FrmGetObjectIndex(frm, triggerPrefCookie));
			FrmShowObject(frm, FrmGetObjectIndex(frm, labelPrefFont));
			FrmShowObject(frm, FrmGetObjectIndex(frm, triggerPrefFont));
			break;
		case tabAdvanced:
			CtlSetValue(GetObjectPtr(tabGeneral), 0);
			CtlSetValue(GetObjectPtr(tabPage), 0);
			CtlSetValue(GetObjectPtr(tabAdvanced), 1);
			
			FrmShowObject(frm, FrmGetObjectIndex(frm, checkAltAgent));
			if (g_prefs.altAgent)
				FrmShowObject(frm, FrmGetObjectIndex(frm, triggerPrefUA));
			FrmShowObject(frm, FrmGetObjectIndex(frm, checkDisableUAProf));
			FrmShowObject(frm, FrmGetObjectIndex(frm, checkDebug));
			break;
		case tabGeneral:
		default:
			CtlSetValue(GetObjectPtr(tabGeneral), 1);
			CtlSetValue(GetObjectPtr(tabPage), 0);
			CtlSetValue(GetObjectPtr(tabAdvanced), 0);
			
			FrmShowObject(frm, FrmGetObjectIndex(frm, labelPrefTime));
			FrmShowObject(frm, FrmGetObjectIndex(frm, labelPrefSec));
			FrmShowObject(frm, FrmGetObjectIndex(frm, triggerPrefTimeout));
			FrmShowObject(frm, FrmGetObjectIndex(frm, labelPrefStartLoc));
			FrmShowObject(frm, FrmGetObjectIndex(frm, triggerPrefStartLoc));
			FrmShowObject(frm, FrmGetObjectIndex(frm, checkImages));
			FrmShowObject(frm, FrmGetObjectIndex(frm, checkMode));
			FrmShowObject(frm, FrmGetObjectIndex(frm, checkPointer));

			if (g_prefs.fiveWayMode == FIVEWAY_MODE_POINTER) {
				FrmShowObject(frm, FrmGetObjectIndex(frm, labelFWSpeed));
				FrmShowObject(frm, FrmGetObjectIndex(frm, triggerFWSpeed));
				FrmShowObject(frm, FrmGetObjectIndex(frm, labelFWAccel));
				FrmShowObject(frm, FrmGetObjectIndex(frm, triggerFWAccel));
			}
			break;
	}
}

Boolean PrefsFormHandleEvent(EventType * eventP)
{
    Boolean handled = false;
    FormType * frmP;
    //Char* text = NULL;
    //Int16 selection;
    //ControlType *controlP;
    GlobalsType	*g;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    switch (eventP->eType) 
        {
        case frmOpenEvent:
            frmP = FrmGetActiveForm();
            PrefsFormInit(frmP);
            FrmDrawForm(frmP);
	    //DrawSampleFont(4, 129, g_prefs.fontSize);
            handled = true;
            break;
            
	case frmUpdateEvent:
		FrmDrawForm(FrmGetActiveForm());
		handled = true;
		break;

        case frmCloseEvent:					
		//SetFieldTextFromHandle(fieldPrefTime, NULL);
		//SetFieldTextFromHandle(fieldPrefAgent, NULL);
		handled = true;
		break;

        case ctlSelectEvent:
		switch (eventP->data.ctlSelect.controlID)
		{
			case tabGeneral:
			case tabPage:
			case tabAdvanced:
				PrefsSwitchView(eventP->data.ctlSelect.controlID);
				break;

			case buttonPrefSave:

					/*//g_prefs.history = CtlGetValue (GetObjectPtr(checkPrefHistory));
					if (CtlGetValue (GetObjectPtr(checkPrefHistory)) == 1)
						g_prefs.history = true;
					else
						g_prefs.history = false;
					//g_prefs.disconnect = CtlGetValue (GetObjectPtr(checkPrefDis));
					if (CtlGetValue (GetObjectPtr(checkPrefDis)) == 1)
						g_prefs.disconnect = true;
					else
						g_prefs.disconnect = false;
					//g_prefs.open = CtlGetValue (GetObjectPtr(checkPrefPromptOpen));
					if (CtlGetValue (GetObjectPtr(checkPrefPromptOpen)) == 1)
						g_prefs.open = true;
					else
						g_prefs.open = false;
					//g_prefs.close = CtlGetValue (GetObjectPtr(checkPrefPromptClose));
					if (CtlGetValue (GetObjectPtr(checkPrefPromptClose)) == 1)
						g_prefs.close = true;
					else
						g_prefs.close = false;
					//g_prefs.agent = LstGetSelection (GetObjectPtr(listPrefAltAgent));//CtlGetValue (GetObjectPtr(checkPrefAltAgent));
					if (LstGetSelection (GetObjectPtr(listPrefAltAgent)) == 0)
						g_prefs.agent = 0;
					else if (LstGetSelection (GetObjectPtr(listPrefAltAgent)) == 1)
						g_prefs.agent = 1;
					else if (LstGetSelection (GetObjectPtr(listPrefAltAgent)) == 2)
						g_prefs.agent = 2;
					else if (LstGetSelection (GetObjectPtr(listPrefAltAgent)) == 3)
						g_prefs.agent = 3;
					else
						g_prefs.agent = 0;

						text = FldGetTextPtr(GetObjectPtr(fieldPrefAgent));
						if(text)
						{
							StrCopy(g_prefs.CustomAgent, text);
						}

					//g_prefs.start = LstGetSelection (GetObjectPtr(listPrefStart));
					if (LstGetSelection (GetObjectPtr(listPrefStart)) == 0)
						g_prefs.start = 0;
					else if (LstGetSelection (GetObjectPtr(listPrefStart)) == 1)
						g_prefs.start = 1;
					else
						g_prefs.start = 0;*/
						
					if (LstGetSelection (GetObjectPtr(listPrefTimeout)) == 0)
						g_prefs.timeout = 10;
					else if (LstGetSelection (GetObjectPtr(listPrefTimeout)) == 1)
						g_prefs.timeout = 30;
					else if (LstGetSelection (GetObjectPtr(listPrefTimeout)) == 2)
						g_prefs.timeout = 60;
					else if (LstGetSelection (GetObjectPtr(listPrefTimeout)) == 3)
						g_prefs.timeout = 90;
					else if (LstGetSelection (GetObjectPtr(listPrefTimeout)) == 4)
						g_prefs.timeout = 120;
					else if (LstGetSelection (GetObjectPtr(listPrefTimeout)) == 5)
						g_prefs.timeout = 240;
					else
						g_prefs.timeout = 60;	
					g_prefs.timeoutlist	= LstGetSelection (GetObjectPtr(listPrefTimeout));
					//SetFieldTextFromHandle(fieldPrefTime, NULL);
					//SetFieldTextFromHandle(fieldPrefAgent, NULL);

					/*if (CtlGetValue (GetObjectPtr(checkImgBrdr)) == 1)
						g_prefs.imgBrdr = true;
					else
						g_prefs.imgBrdr = false;*/


					if (CtlGetValue (GetObjectPtr(checkAltAgent)) == 1) {
						g_prefs.altAgent = true;
						g_prefs.agentNum = LstGetSelection(GetObjectPtr(listPrefUA));
					} else {
						g_prefs.altAgent = false;
					}

					if (CtlGetValue (GetObjectPtr(checkImages)) == 1)
						g_prefs.images = true;
					else
						g_prefs.images = false;

					/*if (CtlGetValue (GetObjectPtr(checkImgResize)) == 1)
						g_prefs.imgResize = true;
					else
						g_prefs.imgResize = false;*/


					if (CtlGetValue (GetObjectPtr(checkMode)) == 1)
						g_prefs.mode = true;
					else
						g_prefs.mode = false;
					{
						UInt32	cacheSize = StrAToI(FldGetTextPtr((FieldPtr)(FrmGetObjectPtr(FrmGetActiveForm(), (FrmGetObjectIndex(FrmGetActiveForm(), fieldCacheSize))))));
						
						if (cacheSize < 256) {
							MyErrorFunc("The cache cannot be smaller than 256 Kb. Please change it.", NULL);
							handled = true;
							break;
						}

                        if (NVFSIsNVFS() && (cacheSize > 9216)) { // NVFS devices cannot have more cache than dbcache
                            MyErrorFunc("The cache size cannot be greater than 9MB on NVFS devices. Please change it.", NULL);
                            handled = true;
                            break;
                        }

						g_prefs.maxCache = cacheSize * 1024;
					}

					if (LstGetSelection (GetObjectPtr(listPrefFont)) == 0)
						g_prefs.fontSize = 1;
					else if (LstGetSelection (GetObjectPtr(listPrefFont)) == 1)
						g_prefs.fontSize = 0;
					else if (LstGetSelection (GetObjectPtr(listPrefFont)) == 2)
						g_prefs.fontSize = 2;

					if (LstGetSelection (GetObjectPtr(listPrefCookie)) == 0)
						g_prefs.cookie = COOKIE_PREF_ACCEPT;
					else if (LstGetSelection (GetObjectPtr(listPrefCookie)) == 1)
						g_prefs.cookie = COOKIE_PREF_ASK;
					else if (LstGetSelection (GetObjectPtr(listPrefCookie)) == 2)
						g_prefs.cookie = COOKIE_PREF_DECLINE;

					g_prefs.startDest = LstGetSelection (GetObjectPtr(listPrefStartLoc));

					if (CtlGetValue (GetObjectPtr(checkDebug)) == 1)
						g_prefs.debug = true;
					else
						g_prefs.debug = false;

					if (CtlGetValue(GetObjectPtr(checkPointer)) == 1)
						g_prefs.fiveWayMode = FIVEWAY_MODE_POINTER;
					else
						g_prefs.fiveWayMode = FIVEWAY_MODE_REGULAR;

					g_prefs.fiveWaySpeed	= LstGetSelection (GetObjectPtr(listFWSpeed)) + 1;
					g_prefs.fiveWayAccel	= LstGetSelection (GetObjectPtr(listFWAccel));

					if (CtlGetValue(GetObjectPtr(checkDisableUAProf)) == 1)
						g_prefs.disableUAProf = true;
					else
						g_prefs.disableUAProf = false;

					PrefSetAppPreferences( appFileCreator, appPrefID, appPrefVersionNum, &g_prefs, sizeof(WAPPreferenceType), true);
				FrmReturnToForm(0);
				if (FrmGetActiveFormID() == frmBrowser) FrmUpdateForm(frmBrowser, frmUpdateMode);
				handled = true;
				break;
			case buttonPrefCancel:
				//SetFieldTextFromHandle(fieldPrefTime, NULL);
				//SetFieldTextFromHandle(fieldPrefAgent, NULL);
				FrmReturnToForm(0);
				if (FrmGetActiveFormID() == frmBrowser) FrmUpdateForm(frmBrowser, frmUpdateMode);
				handled = true;
				break;
			case buttonCacheClear:
				CacheClearCache(g);
				break;
			/*case triggerPrefAgent:
				selection = LstPopupList(GetObjectPtr (listPrefAltAgent));
				if (selection == 3) {
					FrmShowObject (FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fieldPrefAgent));
				} else {
					FrmHideObject (FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fieldPrefAgent));
				}
				controlP = GetObjectPtr(triggerPrefAgent);
				CtlSetLabel( controlP, LstGetSelectionText (GetObjectPtr(listPrefAltAgent), selection) );
				handled = true;
				break;*/
			case checkAltAgent:
				if (CtlGetValue(GetObjectPtr(checkAltAgent)) == 1) {
					FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),triggerPrefUA));
				} else {
					FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),triggerPrefUA));
				}
				break;
		}
		break;

		/*case popSelectEvent:
			switch(eventP->data.popSelect.listID) {
				case listPrefFont:
					if (LstGetSelection (GetObjectPtr(listPrefFont)) == 0)
						DrawSampleFont(20, 81, 1);
					else if (LstGetSelection (GetObjectPtr(listPrefFont)) == 1)
						DrawSampleFont(20, 81, 0);
					else if (LstGetSelection (GetObjectPtr(listPrefFont)) == 2)
						DrawSampleFont(20, 81, 2);
					break;
			}
			break;*/
		
        
        default:
            break;
        }
       
    return handled;
}

Boolean PrivacyFormHandleEvent(EventType * eventP)
{
    Boolean handled = false;
    FormType * frmP;
    //Char* text = NULL;
    //Int16 selection;
    //ControlType *controlP;
    GlobalsType	*g;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    switch (eventP->eType) 
        {
        case frmOpenEvent:
            frmP = FrmGetActiveForm();
	    if (g->privateBrowsing == true)
		CtlSetValue (GetObjectPtr(checkPrivacyEnable), 1);
	    else
		CtlSetValue (GetObjectPtr(checkPrivacyEnable), 0);
            FrmDrawForm(frmP);
            handled = true;
            break;
            
	case frmUpdateEvent:
		FrmDrawForm(FrmGetActiveForm());
		handled = true;
		break;

        case frmCloseEvent:					
		//SetFieldTextFromHandle(fieldPrefTime, NULL);
		//SetFieldTextFromHandle(fieldPrefAgent, NULL);
		handled = true;
		break;

        case ctlSelectEvent:
		switch (eventP->data.ctlSelect.controlID)
		{
			case buttonPrivacyCancel:
				FrmReturnToForm(0);
				handled = true;
				break;
			case buttonPrivacyClearCache:
				CacheClearCache(g);
				HTTPFlushCache();
				break;
			case buttonPrivacyClearHistory:
				historyClear(g);
				HistoryClearHistory(g);
				break;
			case buttonPrivacyClearCookies:
				CookieCleanCookieJar(g->CookieJar, g);
				break;
			case buttonPrivacyClearAll:
				CacheClearCache(g);
				HTTPFlushCache();
				historyClear(g);
				HistoryClearHistory(g);
				CookieCleanCookieJar(g->CookieJar, g);
				disposeVar(g);
				break;
			case checkPrivacyEnable:
				if (CtlGetValue (GetObjectPtr(checkPrivacyEnable)) == 1) {
					g->privateBrowsing = true;
				} else {
                    historyClear(g);
					CookieCleanPrivateCookies(&g->CookieJar, g);
					CacheClearPrivateFiles(g);
					g->privateBrowsing = false;
				}
				break;
		}
		break;
		
        
        default:
            break;
        }
       
    return handled;
}

