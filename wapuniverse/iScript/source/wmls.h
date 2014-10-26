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
 * $Id: wmls.h,v 1.2 2004/04/21 16:28:36 york Exp $ 
 */

#ifndef __WMLS_H__
#define __WMLS_H__

#define BUILDING_SCRIPT_LIB

#include <PalmOS.h>
#include "../iscript.h"

#define WMLS_FALSE         0
#define WMLS_TRUE          1

#define STDLIB_COUNT       3

#define kDefAccuracy 	   0

typedef struct
{
    UInt8   arg_count;             // argument count
    UInt8   var_count;             // variable count
    UInt32  code_size;             // code size in bytes
    UInt8  *code;                  // code pointer
} ScriptFunction;


typedef struct ScriptModuleTag
{
    Char           *uri;           // module URI
    void           *moduleP;       // pointer to module's memory chunk
    UInt32          code_size;     // code size in bytes

    UInt16          const_count;   // constants count
    UInt16          const_charset; // charset of string constants
    ScriptSlot     *const_pool;    // pointer to constants array

    UInt16          pragma_count;  // pragma count (pragmas aren't handled for now)

    UInt8           export_count;  // exported function count
    UInt8          *export_table;  // pointer to name table

    UInt8           func_count;    // function count
    ScriptFunction *func_table;    // function table

    struct ScriptModuleTag *next;
} ScriptModule;


typedef struct
{
    UInt16        refCount;        // library reference counter
    UInt16        stackSize;       // stack size
    ScriptHost   *hostP;           // pointer to host interface implementation
    ScriptModule *modules;         // pointer to the chain of loaded modules
    ScriptSlot   *sp;              // machine stack pointer
    UInt16	  MathLibRef;
} ScriptLibGlobals;


Err ScriptExecute(ScriptLibGlobals *gP, const Char *uri, ScriptSlot *rs);
Err ScriptUnloadModules(ScriptLibGlobals *gP);
Err ScriptTypeCast(ScriptSlot *slot, ScriptType type);
Err ScriptStdFuncCall(ScriptLibGlobals *gP, UInt16 lindex, UInt8 findex, UInt16 *argcP, ScriptSlot *sp, ScriptSlot *rp);


// utilities
Char *ScriptStrCopy(const Char *str);
Char *ScriptStrCopyEx(const Char *str, UInt16 len);
void  ScriptStrFree(Char *str);
Int16 ScriptStrCompare(Char *s1, Char *s2);


// slot management functions
ScriptSlot *SlotToBoolean(ScriptSlot *ptr);
ScriptSlot *SlotToInteger(ScriptSlot *ptr);
ScriptSlot *SlotToString(ScriptSlot *ptr);
ScriptSlot *SlotToFloat(ScriptSlot *ptr);
UInt16      SlotMixedOp(ScriptSlot *arg1, ScriptSlot *arg2);

void  SlotCopy(ScriptSlot *dst, ScriptSlot *src);
void  SlotMove(ScriptSlot *dst, ScriptSlot *src);
void  SlotFree(ScriptSlot *ptr);
void  SlotReplace(ScriptSlot *ptr, UInt8 type, Int32 value);
void  SlotStrAppend(ScriptSlot *ptr, Char *str, UInt32 length);
void  SlotLoadConst(ScriptSlot *ptr, UInt8 *src, UInt16 *offsetP);
Char *SlotParseConst(ScriptSlot *ptr, Char *str);

double StringToDouble(char *str);
Err DoubleToString(double dbl, char *str);

// slot management macros

#define SLOT_ALLOC(sp) \
    sp--                     // TODO: check for stack overflow

#define SLOT_FREE(sp) \
    sp++

#define SLOT_PUSH_INTEGER(sp, v) \
    SLOT_ALLOC(sp); \
    sp->type = scriptTypeInteger; \
    sp->value.n = v

#define SLOT_PUSH_FLOAT(sp, v) \
    SLOT_ALLOC(sp); \
    sp->type = scriptTypeFloat; \
    sp->value.f = v

#define SLOT_PUSH_STRING(sp, v) \
    SLOT_ALLOC(sp); \
    sp->type = scriptTypeString; \
    sp->value.s = ScriptStrCopy(v)

#define SLOT_PUSH_STRING2(sp, v, l) \
    SLOT_ALLOC(sp); \
    sp->type = scriptTypeString; \
    sp->value.s = ScriptStrCopyEx(v, l)

#define SLOT_PUSH_BOOLEAN(sp, v) \
    SLOT_ALLOC(sp); \
    sp->type = scriptTypeBoolean; \
    sp->value.n = v;

#define SLOT_PUSH_FALSE(sp) \
    SLOT_PUSH_BOOLEAN(sp, WMLS_FALSE)
    
#define SLOT_PUSH_TRUE(sp) \
    SLOT_PUSH_BOOLEAN(sp, WMLS_TRUE)

#define SLOT_PUSH_INVALID(sp) \
    SLOT_ALLOC(sp); \
    sp->type = scriptTypeInvalid;

#define SLOT_PUSH_VAR(sp, bp) \
    SLOT_ALLOC(sp); \
    SlotCopy(sp, bp);

#define SLOT_SAVE_VAR(sp, bp) \
    SlotMove(bp, sp); \
    SLOT_FREE(sp)

#define SLOT_POP(sp) \
    SlotFree(sp); \
    SLOT_FREE(sp)

#define SLOT_TO_BOOLEAN(bp) \
    xp = bp; \
    if(xp->type != scriptTypeBoolean) SlotToBoolean(xp)
 
#define SLOT_TO_INTEGER(bp) \
    xp = bp; \
    if(xp->type != scriptTypeInteger) SlotToInteger(xp)

#define SLOT_TO_FLOAT(bp) \
    xp = bp; \
    if(xp->type != scriptTypeFloat) SlotToFloat(xp)
 
#define SLOT_TO_STRING(bp) \
    xp = bp; \
    if(xp->type != scriptTypeString) SlotToString(xp)
 
#define SLOT_IS_FALSE(xp) \
    (xp->type == scriptTypeInvalid || !xp->value.n)

#define SLOT_TO_INTEGER_OP(sp) \
    if(sp->type != scriptTypeInteger) SlotToInteger(sp); \
    xp = sp + 1; \
    if(xp->type != scriptTypeInteger) SlotToInteger(xp); \
    if(xp->type == scriptTypeInteger) xp->type = sp->type

#define SLOT_TO_FLOAT_OP(sp) \
    if(sp->type != scriptTypeFloat) SlotToFloat(sp); \
    xp = sp + 1; \
    if(xp->type != scriptTypeFloat) SlotToFloat(xp); \
    if(xp->type == scriptTypeFloat) xp->type = sp->type
    

#endif // __WMLS_H__
