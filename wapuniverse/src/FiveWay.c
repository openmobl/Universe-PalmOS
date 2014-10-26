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

#include	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"build.h"
#include 	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"
#include	"Form.h"
#include	"Layout.h"
#include	"Browser.h"
#include	"Pointer.h"
#include	"image.h"
#include	"Hyperlinks.h"
#include	"FiveWay.h"


static void RedrawOptionsMenuBitmap(void)
{
	BitmapPtr	resP = 0;

	resP = MemHandleLock(DmGetResource(bitmapRsc, bmpOptionsMenu));
    	WinDrawBitmap(resP, 0, browserGetScreenHeight()-100);
    	MemPtrUnlock(resP);
}

UInt16 FiveWayKeyType(UInt16 chr, UInt16 keyCode, UInt16 modifiers)
{
	Int16	result = fiveWayDirNone;

	switch (chr) {
		case pageUpChr:
		case vchrRockerUp:
#ifdef HAVE_SONY_SDK
		case vchrJogUp:
#endif
			result = fiveWayDirUp;
			break;

        	case pageDownChr:
		case vchrRockerDown:
#ifdef HAVE_SONY_SDK
		case vchrJogDown:
#endif
			result = fiveWayDirDown;
			break;

#ifdef HAVE_PALMONE_SDK
        	case vchrNavChange:
			if(keyCode & navBitRight) {
				result = fiveWayDirRight;
			} else if (keyCode & navBitLeft){
				result = fiveWayDirLeft;
			} else if (keyCode & navBitSelect){
				result = fiveWayDirCenter;
			}
			break;
#endif

#ifdef HAVE_SONY_SDK
		case vchrJogRight:
#endif
		case vchrRockerRight:
			result = fiveWayDirRight;
			break;

#ifdef HAVE_SONY_SDK
		case vchrJogLeft:
#endif
		case vchrRockerLeft:
			result = fiveWayDirLeft;
			break;

#ifdef HAVE_SONY_SDK
		case vchrJogPush:
#endif
#ifdef HAVE_PALMONE_SDK
		case vchrHardRockerCenter:
#endif
		case vchrRockerCenter:
			result = fiveWayDirCenter;
			break;

		default:
			result = fiveWayDirNone;
			break;
	}

	return result;
}

Boolean FiveWayHandleEvent(EventType *event)
{
	FormPtr		frm		= FrmGetActiveForm();
	UInt16 		keyCode;
	UInt16		modifiers;
	UInt16		chr;
	Boolean		handled		= false;
	ListType 	*lst;
	Int16 		lstItem, lstTotal;
	Int16		flags = 0;
  	GlobalsType	*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (WinGetActiveWindow() == FrmGetWindowHandle(FrmGetFormPtr(frmBrowser))) {
		switch (event->eType) {
			case keyDownEvent:
				chr		= event->data.keyDown.chr;
				keyCode		= event->data.keyDown.keyCode;
				modifiers	= event->data.keyDown.modifiers;

#ifdef HAVE_PALMONE_SDK
				if ((chr == hsChrModifierKey) && (modifiers & willSendUpKeyMask)) {
					g->browserGlobals.haveAlt = true;
					break;
				}
#endif

				switch (FiveWayKeyType(chr, keyCode, modifiers)) {
					case fiveWayDirUp:
						if (g->pointer.holding)
							PointerSpeedAccelerate(&g->pointer, g_prefs.fiveWayAccel, g);
						if (scrollField(g))
							return false;

			            		if (StrNCaselessCompare(g->Url,"internal:open",13)!=0){
							if (g->browserGlobals.optMenuOpen == true && g_prefs.mode == MODE_ONEHAND) {
								frm = FrmGetActiveForm();
			            				lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, listOptions));
			            				lstItem = LstGetSelection (lst);
								lstTotal = LstGetNumberOfItems (lst);
								if (lstItem == 0){
									LstSetSelection (lst, lstTotal-1);
									RedrawOptionsMenuBitmap();
								} else {
									LstSetSelection (lst, lstItem-1);
									RedrawOptionsMenuBitmap();
								}

								handled = true;
							} else {
								scrollUp(g);
								handled = true;
							}
						}
						break;
					case fiveWayDirDown:
						if (g->pointer.holding)
							PointerSpeedAccelerate(&g->pointer, g_prefs.fiveWayAccel, g);
						if (scrollField(g))
							return false;

            					if (StrNCaselessCompare(g->Url,"internal:open",13)!=0){
							if (g->browserGlobals.optMenuOpen == true && g_prefs.mode == MODE_ONEHAND) {
            							frm = FrmGetActiveForm();
            							lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, listOptions));
            							lstItem = LstGetSelection (lst);
								lstTotal = LstGetNumberOfItems (lst);
								if (lstItem == lstTotal-1){
									LstSetSelection (lst, 0);
									RedrawOptionsMenuBitmap();
								} else {
									LstSetSelection (lst, lstItem+1);
									RedrawOptionsMenuBitmap();
								}

								handled = true;
							} else {
								scrollDown(g);
								handled = true;
							}
						}
						break;
					case fiveWayDirLeft:
						if (g->pointer.holding)
							PointerSpeedAccelerate(&g->pointer, g_prefs.fiveWayAccel, g);
						if (scrollField(g))
							return false;

						if (g->browserGlobals.optMenuOpen == true && g_prefs.mode == MODE_ONEHAND) {
							CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb)));
							handled = true;
						} else {
							if (g->InputFldActive) {
								UInt16	pos = 0;

								pos = FldGetInsPtPosition((FieldPtr)GetObjectPtr(g->BrowserAPI.BrowserInputField)) + 1;

								if (pos > 0)
									FldSetInsPtPosition((FieldPtr)GetObjectPtr(g->BrowserAPI.BrowserInputField), pos - 1);

								handled = true;
							} else {
								scrollLeft(g);
								handled = true;
							}
						}
						break;
					case fiveWayDirRight:
						if (g->pointer.holding)
							PointerSpeedAccelerate(&g->pointer, g_prefs.fiveWayAccel, g);
						if (scrollField(g))
							return false;

						if (g->browserGlobals.optMenuOpen == true && g_prefs.mode == MODE_ONEHAND) {
							CtlHitControl((ControlPtr)FrmGetObjectPtr(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),buttonSoftOneb)));
							handled = true;
						} else {
							if (g->InputFldActive) {
								UInt16	pos = 0;
								UInt16	len = 0;

								pos = FldGetInsPtPosition((FieldPtr)GetObjectPtr(g->BrowserAPI.BrowserInputField)) - 1;
								len = StrLen(FldGetTextPtr((FieldPtr)GetObjectPtr(g->BrowserAPI.BrowserInputField)));

								if (pos < len)
									FldSetInsPtPosition((FieldPtr)GetObjectPtr(g->BrowserAPI.BrowserInputField), pos + 1);

								handled = true;
							} else {
								scrollRight(g);
								handled = true;
							}
						}
						break;
					case fiveWayDirCenter:
						if (g->browserGlobals.optMenuOpen == true && g_prefs.mode == MODE_ONEHAND) {
							handled = browserHandleOptionsList(LstGetSelection(FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, listOptions))),g);
						} else {
							if (FldGetFocus(fldUrlBox)) {
								FrmSetFocus(FrmGetActiveForm(), noFocus);
								handled = true;
							} else if (g->InputFldActive) {
								FormInputDeactivate(g);
								handled = true;
							} else {
								if (g->browserGlobals.haveAlt)
									break;

#ifdef HAVE_PALMONE_SDK
								if (modifiers & willSendUpKeyMask)
									flags |= pointerGetsPenUp;
#endif

								flags |= pointerPenDown;

								PointerSelectLocation(&g->pointer, flags, g);
								handled = true;
							}
						}
						break;
					case fiveWayDirNone:
					default:
						handled = false;
						break;
				}
				break;
			case keyHoldEvent:
				chr		= event->data.keyHold.chr;
				keyCode		= event->data.keyHold.keyCode;
				modifiers	= event->data.keyHold.modifiers;

				switch (FiveWayKeyType(chr, keyCode, modifiers)) {
					case fiveWayDirUp:
					case fiveWayDirDown:
					case fiveWayDirLeft:
					case fiveWayDirRight:
						// accelerate pointer
						PointerSetHolding(&g->pointer, true, g);
						PointerSpeedAccelerate(&g->pointer, g_prefs.fiveWayAccel, g);
						handled = false;
						break;
					case fiveWayDirCenter:
						//g->isImage = false;

						if (g->browserGlobals.tapandhold) {
							if ((TimGetTicks() >= (g->browserGlobals.taptick + (TAPTIMEOUT)))
							    && (g->browserGlobals.taptype == TAPIMAGE)) {
								checkPageImage(g->browserGlobals.tapx, g->browserGlobals.tapy, true, g);
								g->browserGlobals.tapandhold = false;
								g->browserGlobals.taptick = 0;
								g->browserGlobals.tapx = 0; g->browserGlobals.tapy = 0;
							} else if ((TimGetTicks() >= (g->browserGlobals.taptick + (TAPTIMEOUT)))
								   && (g->browserGlobals.taptype == TAPLINK)) {
								//checkPageImage(g->browserGlobals.tapx, g->browserGlobals.tapy, true, g);
								g->browserGlobals.tapandhold = false;
								g->browserGlobals.taptick = 0;
								g->browserGlobals.tapx = 0; g->browserGlobals.tapy = 0;
							}

							handled = true;
						}
						break;
					case fiveWayDirNone:
					default:
						handled = false;
						break;
				}
				break;
			case keyUpEvent:
				chr		= event->data.keyUp.chr;
				keyCode		= event->data.keyUp.keyCode;
				modifiers	= event->data.keyUp.modifiers;

#ifdef HAVE_PALMONE_SDK
				if (chr == hsChrModifierKey) {
					g->browserGlobals.haveAlt = false;
					break;
				}
#endif

				switch (FiveWayKeyType(chr, keyCode, modifiers)) {
					case fiveWayDirUp:
					case fiveWayDirDown:
					case fiveWayDirLeft:
					case fiveWayDirRight:
						// reset pointer speed
						PointerSetHolding(&g->pointer, false, g);
						PointerSpeedReset(&g->pointer, g_prefs.fiveWaySpeed, g);
						handled = false;
						break;
					case fiveWayDirCenter:
						if (!(g->browserGlobals.optMenuOpen == true && g_prefs.mode == MODE_ONEHAND)) {
							if (!FldGetFocus(fldUrlBox) && !g->InputFldActive) {
								if (g->browserGlobals.haveAlt) {
									UInt16	x = g->pointer.x;
									UInt16	y = g->pointer.y;

									x -= browserGetWidgetBounds(WIDGET_TOPLEFT_X);
									y -= browserGetWidgetBounds(WIDGET_TOPLEFT_Y);

									handled = HyperlinkHandlePenHold(x + g->ScrollX, y + g->ScrollY, g);
									if (!handled) {
										handled = checkPageImage(x + g->ScrollX, y + g->ScrollY, true, g);
									}
								
									break;
								}

								flags |= pointerPenUp;

								PointerSelectLocation(&g->pointer, flags, g);
								handled = true;
							} else {
								handled = true;
							}
						}
						break;
					case fiveWayDirNone:
					default:
						handled = false;
						break;
				}
				break;
			default:
				break;
		}
	}

	return handled;
}