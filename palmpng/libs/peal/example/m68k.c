/**********
 * Copyright (c) 2004 Greg Parker.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GREG PARKER ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********/

#include <PalmOS.h>
#include <stdint.h>
#include <stdarg.h>
#include "peal.h"

#define swap32(n) ( ((((unsigned long) (n)) << 24) & 0xFF000000) | \
                    ((((unsigned long) (n)) <<  8) & 0x00FF0000) | \
                    ((((unsigned long) (n)) >>  8) & 0x0000FF00) | \
                    ((((unsigned long) (n)) >> 24) & 0x000000FF) )


void print(char *format, ...)
{
    RectangleType r, empty;
    char buf[200];

    va_list args;
    va_start(args, format);
    StrVPrintF(buf, format, args);
    va_end(args);

    r.topLeft.x = 0;
    r.topLeft.y = 0;
    r.extent.x = 160;
    r.extent.y = 160;
    WinScrollRectangle(&r, winDown, 12, &empty);
    WinEraseRectangle(&empty, 0);

    WinDrawChars(buf, StrLen(buf), 1, 1);
}

static MemHandle armH = NULL;

PealModule *load(void)
{
    /*
    armH = DmGetResource('armc', 1000);
    if (!armH) {
        return 0;
    }
    return PealLoad(MemHandleLock(armH));
    */
    return PealLoadFromResources('armc', 1000);
}


void unload(PealModule *m)
{
    PealUnload(m);
    /*
    MemHandleUnlock(armH);
    DmReleaseResource(armH);
    */
}


uint32_t callback(void)
{
    return 42;
}


void test(PealModule *m)
{
    void *GetCallCount_arm;
    void *SetFunctionPointer_arm;
    void *CallFunctionPointer_arm;
    void *Call68K_arm;
    
    uint32_t *CallCount_arm;
    void **Callback_arm;
    uint32_t result;
    char *str;

    // Call an ARM function that uses global data
    GetCallCount_arm = PealLookupSymbol(m, "GetCallCount");

    result = PealCall(m, GetCallCount_arm, NULL);
    print("CallCount(): %lu", result);
    result = PealCall(m, GetCallCount_arm, NULL);
    print("CallCount(): %lu", result);
    result = PealCall(m, GetCallCount_arm, NULL);
    print("CallCount(): %lu", result);

    // Read an ARM variable directly
    CallCount_arm = (uint32_t *)PealLookupSymbol(m, "CallCount");
    print("CallCount variable is %lu", swap32(*CallCount_arm));

    // Call an ARM function that uses function pointers and global data
    SetFunctionPointer_arm = PealLookupSymbol(m, "SetFunctionPointer");
    CallFunctionPointer_arm = PealLookupSymbol(m, "CallFunctionPointer");
    
    PealCall(m, SetFunctionPointer_arm, (void *)1);

    str = (char *)PealCall(m, CallFunctionPointer_arm, NULL);
    print("CallFunctionPointer (fn 1): '%s'", str);

    PealCall(m, SetFunctionPointer_arm, (void *)0);
    
    str = (char *)PealCall(m, CallFunctionPointer_arm, NULL);
    print("CallFunctionPointer (fn 0): '%s'", str);
    str = (char *)PealCall(m, CallFunctionPointer_arm, NULL);
    print("CallFunctionPointer (fn 0): '%s'", str);

    // Set an ARM variable
    Callback_arm = (void **)PealLookupSymbol(m, "Callback_m68k");
    *Callback_arm = (void *)swap32(&callback);

    // Call an ARM function that calls back into m68k
    Call68K_arm = PealLookupSymbol(m, "Call68K");
    result = PealCall(m, Call68K_arm, NULL);
    print("m68k callback return %lu", result);

    // Read an ARM variable directly
    CallCount_arm = PealLookupSymbol(m, "CallCount");
    print("CallCount variable is %lu", swap32(*CallCount_arm));
}


UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
    PealModule *m;

    // we don't handle search et al.
    if (cmd != sysAppLaunchCmdNormalLaunch) return 0;
    
    m = load();
    test(m);
    unload(m);

    print("Quitting in 10 seconds.");
    SysTaskDelay(10*SysTicksPerSecond());
    return 0;
}
