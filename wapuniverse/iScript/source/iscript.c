/* 
 * Copyright (c) 2004, TapTarget. All rights reserved.
 * Copyright (c) 2002-2004, Yuri Plaksyuk (york@noir.crocodile.org).
 *
 * http://www.taptarget.com/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *      - Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 * 
 *      - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 * 
 *      - All advertising materials mentioning features or use of this
 *      software must display the following acknowledgement: This
 *      product includes software developed by TapTarget.
 * 
 *      - The name of TapTarget may not be used to endorse or
 *      promote products derived from this software without specific
 *      prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY TAPTARGET "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL TAPTARGET BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: iscript.c,v 1.3 2004/05/13 10:19:24 york Exp $ 
 */

#include "wmls.h"
#include "MathLib.h"

#if 0
#define TRACE_CALLS
#endif


#define DECLARE_GLOBALS(g)  SysLibTblEntryPtr entryP = SysLibTblEntry(ref); \
                            ScriptLibGlobals *g = entryP->globalsP; \
                            if(!g) return scriptErrNotOpen

#ifdef TRACE_CALLS
#   define TRACE(msg) ErrDisplay("TRACE: " msg)
#else
#   define TRACE(msg)
#endif


Err start(UInt16 ref, SysLibTblEntryPtr entryP)
{
    extern void *jmptable();

    entryP->dispatchTblP = (void*)jmptable;
    entryP->globalsP     = 0;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Err ScriptLibOpen(UInt16 ref, ScriptHost *hostP, UInt16 stackSize)
{
    SysLibTblEntryPtr entryP = SysLibTblEntry(ref);
    ScriptLibGlobals *gP = entryP->globalsP; 
    Err error;

    TRACE("ScriptLibOpen");

    if(!gP)
    {
        // allocate space for the globals
        gP = entryP->globalsP = MemPtrNew(sizeof(ScriptLibGlobals));
        if(!gP) return scriptErrMemory;

        MemPtrSetOwner(gP, 0);
        MemSet(gP, sizeof(ScriptLibGlobals), 0);
    }

    gP->stackSize = stackSize;
    gP->hostP     = hostP;
    gP->refCount++;

    error = SysLibFind(MathLibName, &gP->MathLibRef);
    if (error)
        error = SysLibLoad(LibType, MathLibCreator, &gP->MathLibRef);
    ErrFatalDisplayIf(error, "Can't find MathLib"); // Just an example; handle it gracefully
    error = MathLibOpen(gP->MathLibRef, MathLibVersion);
    ErrFatalDisplayIf(error, "Can't open MathLib"); 

    return 0;
}

Err ScriptLibClose(UInt16 ref, UInt16 *numAppsP)
{
    UInt16 usecount;
    Err error;

    DECLARE_GLOBALS(gP);
  
    TRACE("ScriptLibClose");

    error = MathLibClose(gP->MathLibRef, &usecount);
    ErrFatalDisplayIf(error, "Can't close MathLib");
    if (usecount == 0)
        SysLibRemove(gP->MathLibRef);

    ScriptUnloadModules(gP);

    *numAppsP = --gP->refCount;
    if(*numAppsP == 0)
    {
        // free lib globals
        MemChunkFree(entryP->globalsP);
        entryP->globalsP = NULL;
    }

    return errNone;
}

Err ScriptLibSleep(UInt16 ref)
{
    return errNone;
}

Err ScriptLibWake(UInt16 ref)
{
    return errNone;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Err ScriptLibGetVersion(UInt16 ref, UInt32 *versionP)
{
    TRACE("ScriptLibGetVersion");

    *versionP = (VERSION_MAJOR << 8) | VERSION_MINOR;
    return 0;
}

Err ScriptLibExecute(UInt16 ref, const Char *uri, ScriptSlot *rs)
{
    DECLARE_GLOBALS(gP);

    TRACE("ScriptLibExecute");

    return ScriptExecute(gP, uri, rs);
}

Err ScriptLibUnloadModules(UInt16 ref)
{
    DECLARE_GLOBALS(gP);

    TRACE("ScriptLibUnloadModules");

    return ScriptUnloadModules(gP);
}

Err ScriptLibTypeCast(UInt16 ref, ScriptSlot *slotP, ScriptType type)
{
    TRACE("ScriptLibTypeCast");

    switch(type)
    {
    case scriptTypeInteger:
        SlotToInteger(slotP);
        break;

    case scriptTypeFloat:
        SlotToFloat(slotP);
	break;

    case scriptTypeString:
        SlotToString(slotP);
        break;
    
    case scriptTypeBoolean:
        SlotToBoolean(slotP);
        break;

    default:
        return scriptErrNotImplemented;
    }

    return 0;
}
