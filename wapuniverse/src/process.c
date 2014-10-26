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

// TODO: MERGE FIVEWAY CODE WITH FIVEWAY.C

#include 	<PalmOS.h>

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

#include 	"WAPUniverse.h"
#include	"build.h"
#include 	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"
#include	"formBrowser.h"
#include	"history.h"
#include 	"wml.h"
#include	"wmls.h"
#include	"wtp.h"
#include	"http.h"
#include	"Form.h"
#include	"FiveWay.h"
#include	"String.h"
#include	"Layout.h"
#include	"Browser.h"

#ifndef ENABLE_IMG_THREAD
static Boolean ProcessHardKeys(EventType* event);
static Boolean ProcessHandleEvent(EventPtr event);
#endif

Boolean ProcessEventLoopTimeout(UInt32 timeout)
{
#ifndef ENABLE_IMG_THREAD
    EventType event;

	EvtGetEvent(&event, 0);

        if (event.eType == nilEvent && timeout > 0) EvtGetEvent(&event, timeout); 
		
	if (!ProcessHardKeys(&event)) {
        	if (ProcessHandleEvent(&event)) {
			return true;
		} else {
			return false;
		}
	} else {
		return true;
	}

	return false;
#else
	return false;
#endif
}

// The way these two event processes work is different. Returning true means the current process should stop what it is doing.

#ifndef ENABLE_IMG_THREAD

static Boolean ProcessHardKeys(EventType* event)
{
	Boolean handled;
	GlobalsType *g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	handled = false;

	if ((event->data.keyDown.modifiers & poweredOnKeyMask) != 0 &&
	    (event->data.keyDown.chr == vchrHard1 ||
		event->data.keyDown.chr == vchrHard2 ||
		event->data.keyDown.chr == vchrHard3 ||
		event->data.keyDown.chr == vchrHard4))
			return handled;

	if ((FrmGetActiveFormID() == frmBrowser) && (g_prefs.mode == MODE_ONEHAND)) {
		switch ( event->data.keyDown.chr ) {
			case vchrHard1:
				handled = false;
				break;

			case vchrHard2:
				handled = false;
				break;

			case vchrHard3:
				handled = false;
				break;

			case vchrHard4:
				//CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftTwob)));
				handled = true;
				break;

			default:
				//if (optMenuOpen == true)
				//	CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb)));
				handled = false;
				break;
		}
	}
 
	return handled;
}

static Boolean ProcessHandleEvent(EventPtr event)
{
	Boolean		handled = false;
  	GlobalsType	*g;
  	//FormPtr         frm = FrmGetActiveForm();
	UInt16 		keyCode = event->data.keyDown.keyCode;
	//ListType 	*lst;
	//Int16 		lstItem, lstTotal;
	RectangleType 	rect;

		    	
	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	switch (event->eType) {
		case ctlSelectEvent:
			if (event->data.ctlSelect.controlID == buttonStop) {
				FormInputDeactivate(g);
				//handled = browserStopEvt(g);
				handled = true;
        		} else if (event->data.ctlSelect.controlID == buttonSoftTwob) {
				FormInputDeactivate(g);
				//handled = browserStopEvt(g);
				handled = true;
        		}
        		break;

		case sclRepeatEvent:
			/*if (optMenuOpen == true) {
				FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),listOptions));
				FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb));
				FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOnea));
				WinRestoreBits(saved, 0, 60); saved = 0;
				optMenuOpen = false;
			}*/
			FormInputDeactivate(g);
        		g->ScrollY = event->data.sclRepeat.newValue*10;
        		scrollUpdate(g);
        		break;

		case keyDownEvent:
			if (event->data.keyDown.chr == pageUpChr
#ifdef HAVE_PALMONE_SDK
			    || event->data.keyDown.chr == keyRockerUp
#endif
#ifdef HAVE_SONY_SDK
			    || event->data.keyDown.chr == vchrJogUp
#endif
			   ) {
				if (StrNCaselessCompare(g->Url,"internal:open",13)!=0){
					FormInputDeactivate(g);
					//fiveWayUp(g);
					scrollUp(g);
				}
			} else if (event->data.keyDown.chr == pageDownChr
#ifdef HAVE_PALMONE_SDK
				   || event->data.keyDown.chr == keyRockerDown
#endif
#ifdef HAVE_SONY_SDK
				   || event->data.keyDown.chr == vchrJogDown
#endif
				  ) {
					if (StrNCaselessCompare(g->Url,"internal:open",13)!=0){
						FormInputDeactivate(g);
						//fiveWayDown(g);
						scrollDown(g);
					}
			}
#ifdef HAVE_PALMONE_SDK
			else if (event->data.keyDown.chr == vchrNavChange) {
				if (keyCode & navBitRight) {
					FormInputDeactivate(g);
					////scrollDown(g);
					//fiveWayDown(g);
					scrollRight(g);
				} else if (keyCode & navBitLeft) {
					FormInputDeactivate(g);
	            			////scrollUp(g);
					//fiveWayUp(g);
					scrollLeft(g);
				}
				
				/*if(keyCode & navBitSelect){
					fiveWaySelect(g);
				}*/
			} else if (event->data.keyDown.chr == keyRockerRight) {
				FormInputDeactivate(g);
				////scrollDown(g);
				//fiveWayDown(g);
				scrollRight(g);
			} else if (event->data.keyDown.chr == keyRockerLeft) {
				FormInputDeactivate(g);
				////scrollUp(g);
				//fiveWayUp(g);
				scrollLeft(g);
			} else if (event->data.keyDown.chr == keyRockerCenter) {
				//fiveWaySelect(g);
			}
#endif
			else if (event->data.keyDown.chr == 'x') {
				if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
					handled = true;
				}
			} else if (event->data.keyDown.chr == 'r') {
				if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
					BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
											      g->BrowserAPI.BrowserWidget),
							g->ScrollY - BrowserLineheightValue(), g);
				}
			} else if (event->data.keyDown.chr == 'c') {
				if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
					BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
											      g->BrowserAPI.BrowserWidget),
							g->ScrollY + BrowserLineheightValue(), g);
				}
			} else if (event->data.keyDown.chr == 'd') {
				if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
					BrowserScrollHorizontalTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
													g->BrowserAPI.BrowserWidget),
								  g->ScrollX - BrowserLineheightValue(), g);
				}
			} else if (event->data.keyDown.chr == 'g') {
				if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
					BrowserScrollHorizontalTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
													g->BrowserAPI.BrowserWidget),
								  g->ScrollX + BrowserLineheightValue(), g);
				}
			} else if ((event->data.keyDown.chr == '+') ||
				   (event->data.keyDown.chr == 'w')) {
				if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
					// scroll up or down??
					BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
											      g->BrowserAPI.BrowserWidget),
							g->ScrollY - browserGetWidgetBounds(WIDGET_HEIGHT), g);
				}
			} else if ((event->data.keyDown.chr == '-') ||
				   (event->data.keyDown.chr == 's')) {
				if (!FldGetFocus(fldUrlBox) && !FldGetFocus(fldInput) && !g->InputFldActive) {
					// scroll up or down??
					BrowserScrollTo(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),
											      g->BrowserAPI.BrowserWidget),
							g->ScrollY + browserGetWidgetBounds(WIDGET_HEIGHT), g);
				}
			}
			break;
  	
		case penDownEvent:
			//handled = browserPenDownEvt(event,g);
			if (g_prefs.mode == MODE_ONEHAND) {
				FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), buttonSoftTwob), &rect);
				if (RctPtInRectangle(event->screenX, event->screenY, &rect))
					handled = true;
			}
			/*if ((event->screenX >= 30 && event->screenY >= (browserGetScreenHeight() - 14)) &&
			      (event->screenX <= 44 && event->screenY <= browserGetScreenHeight())) {
				handled = true;
			}*/
			break;

		case penUpEvent: // leave this one at the end of the switch !!
			//handled = browserPenUpEvt(event,g);
			if (g_prefs.mode == MODE_ONEHAND) {
				FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), buttonSoftTwob), &rect);
				if (RctPtInRectangle(event->screenX, event->screenY, &rect))
					handled = true;
			}
		
			FrmGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), buttonStop), &rect);
			if (RctPtInRectangle(event->screenX, event->screenY, &rect))
				handled = true;
			break;

		case nilEvent:
			handled = false;
			break;
		
		default:
			break;
	}

	return handled;
}

#endif
