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
#include 	<PalmOS.h>
#include 	"wmls.h"
#include 	"formBrowser.h"
#include 	"iscript.h"
#include 	"WAPUniverse.h"
#include 	"../res/WAPUniverse_res.h"
#include	"variable.h"
#include	"Form.h"
#include	"Url.h"

static void SlotSetString(ScriptSlot *slot, const Char *str);
static void SlotSetStringEx(ScriptSlot *slot, const Char *str, Int16 len);
/*static Err SlotTypeCast(ScriptSlot *slot, ScriptType type);*/

static void WMLSResetBrowserContext(GlobalsType *g)
{
	browserResetURL(g);
	//browserPopPageContent(g);
}

Err WMLSLoadModule(const Char *uri, UInt16 len, void **modulePP, UInt16 *offsetP)
{
    GlobalsType *g;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    g->wmlsBrowserContextHandled = false;

    g->wmlsData = Malloc(g->webData.length + 1);
    if (!g->wmlsData) {
	WMLSResetBrowserContext(g);
	return 1;
    }
    MemMove(g->wmlsData, g->webData.data, g->webData.length);

    browserPopPageContent(g);

    *modulePP = g->wmlsData; // g->webData.data;
    *offsetP  = 0;

    return errNone;
}

Err WMLSUnloadModule(void *moduleP)
{
    GlobalsType *g;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    //MemHandleUnlock(moduleH);
    if (!g->wmlsBrowserContextHandled)
	WMLSResetBrowserContext(g);

    g->wmlsBrowserContextHandled = false;

    return errNone;
}

static void SlotSetString(ScriptSlot *slot, const Char *str)
{
    SlotSetStringEx(slot, str, StrLen(str));
}

static void SlotSetStringEx(ScriptSlot *slot, const Char *str, Int16 len)
{
    slot->type = scriptTypeString;
    slot->value.s = MemPtrNew(len + 1);

    if(slot->value.s)
    {
        StrNCopy(slot->value.s, str, len);
        slot->value.s[len] = '\0';
    }
}

/*static Err SlotTypeCast(ScriptSlot *slot, ScriptType type)
{

    	return ScriptLibTypeCast(gScriptRef, slot, type);
}*/

/*
    Basically we want to "go back" to the previous real page and start from there.
    This should be called before any link functions.
*/

Err WMLSLibraryCall(UInt16 lindex, UInt8 findex, UInt16 *argcP, ScriptSlot *sp, ScriptSlot *rp)
{
    Err 	err = errNone;
    UInt16 	res;
    GlobalsType *g;
    //Char 	*newUrl;
    //Char 	*posC;
    Char 	*tempStr;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    switch(lindex)
    {
    case 3: /* URL library */
        switch(findex)
        {
        case  0: *argcP = 1; // isValid
        case  1: *argcP = 1; // getScheme
        case  2: *argcP = 1; // getHost
        case  3: *argcP = 1; // getPort
        case  4: *argcP = 1; // getPath
        case  5: *argcP = 1; // getParameters
        case  6: *argcP = 1; // getQuery
        case  7: *argcP = 1; // getFragment
        case  8: *argcP = 0; // getBase
            err = scriptErrNotImplemented; // TODO: implement me
            break;

        case  9: *argcP = 0; // getReferer
            err = scriptErrNotImplemented; // TODO: implement me
            break;

        case 10: *argcP = 2; // resolve
            err = scriptErrNotImplemented; // TODO: implement me
            break;

        case 11: *argcP = 1; // escapeString
            err = scriptErrNotImplemented; // TODO: implement me
            break;

        case 12: *argcP = 1; // unescapeString
            err = scriptErrNotImplemented; // TODO: implement me
            break;

        case 13: *argcP = 2; // loadString
            err = scriptErrNotImplemented; // TODO: implement me
            break;

        default: err = scriptErrFunctionNotFound;
        }
        break;

    case 4: /* WMLBrowser library */
        switch(findex)
        {
        case  0: *argcP = 1; // getVar
            ScriptLibTypeCast(gScriptRef, sp, scriptTypeString);
            if(sp->type == scriptTypeString && sp->value.s)
            {
		addVar(sp->value.s,g);
		SlotSetString(rp, getVar(sp->value.s, g));
            }
            break;

        case  1: *argcP = 2; // setVar
            ScriptLibTypeCast(gScriptRef, sp + 0, scriptTypeString);
            ScriptLibTypeCast(gScriptRef, sp + 1, scriptTypeString);
            
            if(sp[0].type == scriptTypeString && sp[1].type == scriptTypeString && sp[1].value.s)
            {
		addVar(sp[1].value.s, g);
                setVar(sp[1].value.s, sp[0].value.s, g);

                rp->type = scriptTypeBoolean;
                rp->value.n = 1;
            }
            else
                rp->type = scriptTypeInvalid;
            break;
        
        case  2: *argcP = 1; // go
            ScriptLibTypeCast(gScriptRef, sp, scriptTypeString);

	    //browserResetURL(g);
	    WMLSResetBrowserContext(g);
	    g->wmlsBrowserContextHandled = true;

	    /*if (sp->value.s[0] == '#') {
		newUrl = MemPtrNew(StrLen(sp->value.s) + StrLen(g->History->hist[g->History->index].url) + 1);
		StrCopy(newUrl, g->History->hist[g->History->index].url);

		if ((posC = StrChr(g->History->hist[g->History->index].url, '#'))) {
			StrCopy(newUrl+(posC-g->History->hist[g->History->index].url), sp->value.s);
		} else {
			StrCopy(newUrl+StrLen(newUrl), sp->value.s);
		}

		followLink(newUrl, g);

		MemPtrFree(newUrl);
	    } else {
            	followLink(sp->value.s, g);
	    }*/

	    followLink(sp->value.s, g);
            break;

        case  3: *argcP = 0; // prev
	    //browserResetURL(g);
	    WMLSResetBrowserContext(g);
	    g->wmlsBrowserContextHandled = true;
            browserBackEvt(g);
            break;

        case  4: *argcP = 0; // newContext
            err = scriptErrNotImplemented; // TODO: implement me
            break;

        case  5: *argcP = 0; // getCurrentCard
            err = scriptErrNotImplemented; // TODO: implement me
            break;

        case  6: *argcP = 0; // refresh
	    //browserResetURL(g);
	    WMLSResetBrowserContext(g);
	    g->wmlsBrowserContextHandled = true;
            FormInputDeactivate(g);
	    g->state=BS_RENDER;
            break;

        case  7: *argcP = 1; // status
            //ScriptLibTypeCast(gScriptRef, sp, scriptTypeString);
            //gB.browserShowStatusText(sp->value.s ? sp->value.s : "");
	    err = scriptErrNotImplemented; // TODO: implement me, How?
            break;

        default:
	    err = scriptErrFunctionNotFound;
        }
        break;

    case 5: /* Dialogs library */
        switch(findex)
        {
        case  0: *argcP = 2; // prompt
            ScriptLibTypeCast(gScriptRef, sp + 1, scriptTypeString);
            ScriptLibTypeCast(gScriptRef, sp + 0, scriptTypeString);

            SlotSetStringEx(rp, "", 80);

            res = FrmCustomResponseAlert(AlertPrompt,
                sp[1].value.s ? sp[1].value.s : "",
                sp[0].value.s ? "\nDefault: " : "",
                sp[0].value.s ? sp[0].value.s : "",
                rp->value.s, 80, NULL);

            if(res != 0)
            {
                Char *tmp;

                tmp = rp->value.s, rp->value.s = sp->value.s, sp->value.s = tmp;
            }
            break;

        case  1: *argcP = 3; // confirm
            ScriptLibTypeCast(gScriptRef, sp + 2, scriptTypeString);
//          ScriptLibTypeCast(gScriptRef, sp + 1, scriptTypeString);
//          ScriptLibTypeCast(gScriptRef, sp + 0, scriptTypeString);

            res = FrmCustomAlert(AlertPrompt, sp[2].value.s ? sp[2].value.s : "", "", "");

            rp->type    = scriptTypeBoolean;
            rp->value.n = (res == 0);
            break;

        case  2: *argcP = 1; // alert
            ScriptLibTypeCast(gScriptRef, sp, scriptTypeString);
            FrmCustomAlert(AlertCustom, sp->value.s ? sp->value.s : "", "", "");
            break;

        default: err = scriptErrFunctionNotFound;
        }
        break;

    case 512: /* WTAPublic library */
	switch(findex)
	{
	case 0: *argcP = 1; // makeCall
	    ScriptLibTypeCast(gScriptRef, sp, scriptTypeString);
	    if (PrvDialListCanBePhoneNumber(sp->value.s, StrLen(sp->value.s)) == false) {
		rp->type    = scriptTypeInvalid;
		break;
	    }
	    DisplayDial(sp->value.s,1);

	    SlotSetString(rp, "");
	    break;

	case 1: *argcP = 1; // sendDTMF
	    ScriptLibTypeCast(gScriptRef, sp, scriptTypeString);
	    if (PrvDialListCanBePhoneNumber(sp->value.s, StrLen(sp->value.s)) == false) {
		rp->type    = scriptTypeInvalid;
		break;
	    }
	    DisplayDial(sp->value.s,2);

	    SlotSetString(rp, "");
	    break;

	case 2: *argcP = 2; // addPBEntry
            ScriptLibTypeCast(gScriptRef, sp + 1, scriptTypeString);
            ScriptLibTypeCast(gScriptRef, sp + 0, scriptTypeString);
	    if (PrvDialListCanBePhoneNumber(sp[0].value.s, StrLen(sp[0].value.s)) == false) {
		rp->type    = scriptTypeInvalid;
		break;
	    }
	    tempStr = MemPtrNew(StrLen(sp[0].value.s) + StrLen(sp[1].value.s) + 1);
	    if (!tempStr) {
            	rp->type    = scriptTypeInteger;
            	rp->value.n = -103;
		break;
	    }
	    StrCopy(tempStr, sp[0].value.s); StrCopy(tempStr + StrLen(tempStr), ";"); StrCopy(tempStr + StrLen(tempStr), sp[1].value.s);
	    DisplayDial(tempStr,3);
	    MemPtrFree(tempStr);

	    SlotSetString(rp, "");
	    break;

	default:
	    err = scriptErrFunctionNotFound;
	}
	break;

    case 513: /* WTAVoiceCall */
	switch(findex)
	{
	default: err = scriptErrNotImplemented;
	}
	break;

    case 514: /* WTANetText */
	switch(findex)
	{
	default: err = scriptErrNotImplemented;
	}
	break;

    case 515: /* WTAPhoneBook */
	switch(findex)
	{
	default: err = scriptErrNotImplemented;
	}
	break;

    case 516: /* WTAMisc */
	switch(findex)
	{
	default: err = scriptErrNotImplemented;
	}
	break;

    case 519: /* WTACallLog */
	switch(findex)
	{
	default: err = scriptErrNotImplemented;
	}
	break;

    default:
        //err = LibraryCall(lindex, findex, argcP, sp, rp, SlotTypeCast);
	err = scriptErrLibraryNotFound;
        break;
    }

    return err;
}

Boolean callWMLScriptFunc(const Char *uri)
{
  Err err;
  ScriptSlot result = { scriptTypeInvalid };
  UInt16 numApps;

  static ScriptHost host = {
        WMLSLoadModule,
        WMLSUnloadModule,
        WMLSLibraryCall,
  };


	err = ScriptLibOpen(gScriptRef, &host, 64);
    	if (err)
		return true;

	// execute script URI
	err = ScriptLibExecute(gScriptRef, uri, &result);
	//if(err) FrmCustomAlert(alError, "Invalid script URI or script error.", "WMLS", ""); // We should display the error

	// cleanup result slot
	ScriptLibTypeCast(gScriptRef, &result, scriptTypeInvalid);

    	err = ScriptLibClose(gScriptRef, &numApps);
    	ErrFatalDisplayIf(err, "Could not close WMLSLib");

        return true;
}
