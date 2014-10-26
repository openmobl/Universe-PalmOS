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
 * $Id: iscript.h,v 1.5 2004/05/06 14:41:48 york Exp $ 
 */

#ifndef __SCRIPT_LIB_H__
#define __SCRIPT_LIB_H__

#include <LibTraps.h>

#ifndef BUILDING_SCRIPT_LIB
#define SCRIPT_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#else
#define SCRIPT_LIB_TRAP(trapNum)
#endif

/****************************************************************************************
 * Internal library name which can be passed to SysLibFind()
 ***************************************************************************************/
#define scriptLibName      "iScriptLib"
#define scriptLibCreator   'iSct'
#define scriptLibType      'libr'

#ifdef __GNUC__

#define scriptLibTrapGetVersion     (sysLibTrapCustom + 0)
#define scriptLibTrapExecute        (sysLibTrapCustom + 1)
#define scriptLibTrapUnloadModules  (sysLibTrapCustom + 2)
#define scriptLibTrapTypeCast       (sysLibTrapCustom + 3)

#else

typedef enum
{
    scriptLibTrapGetVersion = sysLibTrapCustom,
    scriptLibTrapExecute,
    scriptLibTrapUnloadModules,
    scriptLibTrapTypeCast
} ScriptLibTrapNumberEnum;

#endif

/***************************************************************************************
 * Library result codes
 ***************************************************************************************/
#define scriptErrParam              (appErrorClass |  1) // invalid parameter
#define scriptErrNotOpen            (appErrorClass |  2) // library is not open
#define scriptErrUnexpected         (appErrorClass |  3) // unexpected call
#define scriptErrMemory             (appErrorClass |  4) // memory error occurred
#define scriptErrInvalidURI         (appErrorClass |  5) // URI format is invalid
#define scriptErrFunctionNotFound   (appErrorClass |  6) // function not found
#define scriptErrSyntax             (appErrorClass |  7) // syntax error
#define scriptErrIllegalInstruction (appErrorClass |  8) // illegal instruction encountered
#define scriptErrVerificationFailed (appErrorClass |  9) // code verification failed
#define scriptErrNotImplemented     (appErrorClass | 10) // not implemented
#define scriptErrExited             (appErrorClass | 11) // exit code
#define scriptErrAborted            (appErrorClass | 12) // abort code
#define scriptErrLibraryNotFound    (appErrorClass | 13) // script library not found
#define scriptErrPageNotFound       (appErrorClass | 14) // page not found

typedef enum {
    scriptTypeInteger = 0,
    scriptTypeFloat,
    scriptTypeString,
    scriptTypeBoolean,
    scriptTypeInvalid,
    scriptTypeArray,
    scriptTypeObject
} ScriptType;


typedef struct
{
    ScriptType type;

    union
    {
        Int32 n;             // integer or boolean value
        Char *s;             // string value
        void *a;             // any value
	double f;	     // float value
    } value;
} ScriptSlot;



typedef struct
{
    /**
     * Invoked by script engine to load specified by URI module. If module
     * is succeessfully loaded, its pointer is stored in moduleP. The module
     * may contain arbitrary information. However, it must include WMLScript
     * encoded stream. An offset to the beginning of the WMLScript encoded
     * stream is passed through offsetP.
     *
     * @param uri requested module URI
     * @param len the URI length
     * @param modulePP pointer to variable to store module pointer
     * @param offsetP pointer to variable to store offset to WMLScript encoded stream
     */
    Err (*LoadModule)(const Char *uri, UInt16 len, void **modulePP, UInt16 *offsetP);

    /**
     * Invoked by script engine to unload a specified module.
     *
     * @param module the module pointer to unload
     */
    Err (*UnloadModule)(void *moduleP);

    /**
     * Invoked by script engine to perform external library call.
     *
     * @param lindex library index
     * @param findex function index
     * @param argcP pointer to a variable to store number of parameters function accepts
     * @param sp stack pointer
     * @param rp result slot pointer
     */
    Err (*LibraryCall)(UInt16 lindex, UInt8 findex, UInt16 *argcP, ScriptSlot *sp, ScriptSlot *rp);
} ScriptHost;


/********************************************************************
 * API Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes script engine and passes a pointer to host interface implementation.
 *
 * @param ref shared library reference
 * @param hostP pointer to host interface implementation
 * @param stackSize size of the stack
 * @return error code
 */
extern Err ScriptLibOpen(UInt16 ref, ScriptHost *hostP, UInt16 stackSize)
    SCRIPT_LIB_TRAP(sysLibTrapOpen);

/**
 * Closes the script engine.
 *
 * @param ref shared library reference
 * @param numAppsP pointer to a variable to store reference count
 * @return error code
 */
extern Err ScriptLibClose(UInt16 ref, UInt16 *numAppsP)
    SCRIPT_LIB_TRAP(sysLibTrapClose);

/**
 * Moves the script engine into sleep mode.
 *
 * @param ref shared library reference
 * @return error code
 */
extern Err ScriptLibSleep(UInt16 ref)
    SCRIPT_LIB_TRAP(sysLibTrapSleep);

/**
 * Wakes up the script engine from sleep.
 *
 * @param ref shared library reference
 * @return error code
 */
extern Err ScriptLibWake(UInt16 ref)
    SCRIPT_LIB_TRAP(sysLibTrapWake);

/**
 * Gets script engine version.
 *
 * @param ref shared library reference
 * @param versionP pointer to a variable to hold version
 * @return error code
 */
extern Err ScriptLibGetVersion(UInt16 ref, UInt32 *versionP)
    SCRIPT_LIB_TRAP(scriptLibTrapGetVersion);

/**
 * Executes script URI.
 *
 * @param ref shared library reference
 * @param uri script URI
 * @return error code
 */
extern Err ScriptLibExecute(UInt16 ref, const Char *uri, ScriptSlot *rs)
    SCRIPT_LIB_TRAP(scriptLibTrapExecute);

/**
 * Unloads all loaded modules and set script engine into initial state.
 *
 * @param ref shared library reference
 * @return error code
 */
extern Err ScriptLibUnloadModules(UInt16 ref)
    SCRIPT_LIB_TRAP(scriptLibTrapUnloadModules);

/**
 * Performs type casting to a specified type.
 *
 * @param ref shared library reference
 * @param slotP slot pointer to be casted
 * @param type type to cast to
 * @return error code
 */
extern Err ScriptLibTypeCast(UInt16 ref, ScriptSlot *slotP, ScriptType type)
    SCRIPT_LIB_TRAP(scriptLibTrapTypeCast);


#ifdef __cplusplus 
}
#endif


#endif  // __SCRIPT_LIB_H__
