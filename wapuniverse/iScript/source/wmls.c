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
 * $Id: wmls.c,v 1.4 2004/05/13 10:19:24 york Exp $ 
 */

#include "wmls.h"
#include "wmlscode.h"
#include "MathLib.h"

static ScriptModule *ScriptLoadModule(ScriptLibGlobals *gP, const Char *uri, UInt16 len, Err *errP);
static Int16 ScriptFindFunc(ScriptModule *m, Char *str, UInt16 strlen);
static Err   ScriptFuncCall(ScriptLibGlobals *gP, ScriptModule *m, UInt8 findex);
static Err   ScriptLibFuncCall(ScriptLibGlobals *gP, UInt16 lindex, UInt8 findex);
static Err   ScriptUrlFuncCall(ScriptLibGlobals *gP, UInt16 uindex, UInt8 findex);
static Int32 ReadIntMB(UInt8 *buf, UInt16 *offsetP);


Err ScriptExecute(ScriptLibGlobals *gP, const Char *uri, ScriptSlot *rs)
{
    Int16 findex;
    UInt8 argc = 0;
    Char *str, *ptr;
    Err err = errNone;

    ScriptModule *m;
    ScriptSlot *stack;
    ScriptFunction *fp;


    str = StrChr(uri, '#');
    if(str == NULL) return scriptErrInvalidURI;

    m = ScriptLoadModule(gP, uri, str - uri, &err);
    if(m == NULL) return err;

    ptr = StrChr(++str, '(');
    if(ptr == NULL) return scriptErrInvalidURI;

    findex = ScriptFindFunc(m, str, ptr - str);
    if(findex < 0) return scriptErrFunctionNotFound;

    stack = MemPtrNew(sizeof(ScriptSlot) * gP->stackSize);
    if(stack == NULL) return scriptErrMemory;

    gP->sp = stack + gP->stackSize; // initialize stack pointer

    while(1)
    {
        // push function parameters into the stack
        ptr++;

        while(*ptr == ' ') ptr++; // skip spaces

        if(*ptr == ')') break;

        ptr = SlotParseConst(--gP->sp, ptr), argc++;
        if(ptr == NULL) { err = scriptErrSyntax; goto __cleanup; }

        while(*ptr == ' ') ptr++; // skip spaces

        if(*ptr == ')') break;
        if(*ptr != ',') { err = scriptErrSyntax; goto __cleanup; }
    }


    // check number of parameters
    fp = m->func_table + findex;
    if(argc != fp->arg_count)
    {
        err = scriptErrVerificationFailed;
        goto __cleanup;
    }

    // execute function
    err = ScriptFuncCall(gP, m, findex);
    if(err) goto __cleanup;

    // save result into result slot
    rs->type     = gP->sp->type;
    rs->value.a  = gP->sp->value.a;
    SLOT_FREE(gP->sp);

__cleanup:
    while(gP->sp < stack + gP->stackSize) { SLOT_POP(gP->sp); }
    MemPtrFree(stack);

    return err;
}

Err ScriptUnloadModules(ScriptLibGlobals *gP)
{
    ScriptModule *m = gP->modules, *p;
    UInt16 i;

    while(m)
    {
        if(m->uri)
            MemPtrFree(m->uri);

        if(m->const_pool)
        {
            for(i = 0; i < m->const_count; i++)
                SlotFree(m->const_pool + i);
            MemPtrFree(m->const_pool);
        }

        if(m->func_table)
            MemPtrFree(m->func_table);

        p = m, m = m->next;

        gP->hostP->UnloadModule(p->moduleP);
        MemPtrFree(p);
    }

    gP->modules = NULL;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static ScriptModule *ScriptLoadModule(ScriptLibGlobals *gP, const Char *uri, UInt16 len, Err *errP)
{
    ScriptModule *m;
    UInt16 offset, i;
    UInt8 *moduleP;

    *errP = 0;

    // find it among already loaded modules
    for(m = gP->modules; m; m = m->next)
    {
        if(StrNCompare(m->uri, uri, len) == 0 && m->uri[len] == 0)
            return m;
    }

    // load module from the host
    *errP = gP->hostP->LoadModule(uri, len, (void**)&moduleP, &offset);
    if(*errP) return NULL;

    // parse module
    m = MemPtrNew(sizeof(ScriptModule));
    if(m == NULL) { *errP = scriptErrMemory; return NULL; }

    offset++; // skip version info

    MemSet(m, sizeof(ScriptModule), 0);
    m->uri           = ScriptStrCopyEx(uri, len);
    m->moduleP       = moduleP;
    m->code_size     = ReadIntMB(moduleP, &offset);
    m->const_count   = ReadIntMB(moduleP, &offset);
    m->const_charset = ReadIntMB(moduleP, &offset);

    if(m->const_count)
    {
        m->const_pool = MemPtrNew(sizeof(ScriptSlot) * m->const_count);
        if(m->const_pool == NULL) { *errP = scriptErrMemory; return NULL; }

        for(i = 0; i < m->const_count; i++)
            SlotLoadConst(m->const_pool + i, moduleP, &offset);
    }

    // pragma pool
    m->pragma_count = ReadIntMB(moduleP, &offset);
    for(i = 0; i < m->pragma_count; i++)
        switch(moduleP[offset++])
        {
        case 0: // Access Domain
        case 1: // Access Path
            ReadIntMB(moduleP, &offset);
            break;

        case 2: // User Agent Property
            ReadIntMB(moduleP, &offset);
            ReadIntMB(moduleP, &offset);
            break;

        case 3: // User Agent Property and Scheme
            ReadIntMB(moduleP, &offset);
            ReadIntMB(moduleP, &offset);
            ReadIntMB(moduleP, &offset);
            break;
        }

    // function pool
    m->func_count   = moduleP[offset++];
    m->export_count = moduleP[offset++];
    m->export_table = moduleP + offset;

    // skip exported function names
    for(i = 0; i < m->export_count; i++)
        offset += moduleP[offset + 1] + 2;

    m->func_table = MemPtrNew(sizeof(ScriptFunction) * m->func_count);
    if(m->func_table == NULL) { *errP = scriptErrMemory; return NULL; }

    for(i = 0; i < m->func_count; i++)
    {
        ScriptFunction *fp = m->func_table + i;

        fp->arg_count = moduleP[offset++];
        fp->var_count = moduleP[offset++];
        fp->code_size = ReadIntMB(moduleP, &offset);
        fp->code      = moduleP + offset;

        offset += fp->code_size;
    }

    return m->next = gP->modules, gP->modules = m;
}

static Int16 ScriptFindFunc(ScriptModule *m, Char *str, UInt16 strlen)
{
    UInt16 i;

    UInt8 *ptr = m->export_table;
    UInt8 findex, length;

    for(i = 0; i < m->export_count; i++)
    {
        findex = *ptr++; // exported function index
        length = *ptr++; // exported function name length

        if(length == strlen && StrNCompare(ptr, str, strlen) == 0)
            return findex;

        ptr += length;   // go to next name entry
    }

    return -1;
}

static Err ScriptFuncCall(ScriptLibGlobals *gP, ScriptModule *m, UInt8 findex)
{
    ScriptFunction *fp;
    ScriptSlot *bp, *xp, *tp;
    UInt8 *ip, *code_end;
    UInt16 i;
    Err err;
    FlpCompDouble flpArg1;
    FlpCompDouble flpArg2;
    FlpCompDouble theCompFloat;

    if(findex >= m->func_count)
        goto __verification_failed; 

    fp = m->func_table + findex;
    bp = gP->sp + fp->arg_count - 1; // TODO: check for stack underflow!!!
    ip = fp->code;

    for(i = 0; i < fp->var_count; i++)
    {
        SLOT_PUSH_STRING(gP->sp, NULL);
    }

    code_end = ip + fp->code_size;
    while(ip < code_end)
    {
        switch(*ip)
        {
        case RESERVED_00:
            goto __illegal_instruction;

        case JUMP_FW:
            ip += *(++ip);
            break;

        case JUMP_FW_W:
            ip += *(UInt16*)(++ip) + 1;
            break;

        case JUMP_BW:
            ip -= *(ip + 1);
            continue;

        case JUMP_BW_W:
            ip -= *(UInt16*)(ip + 1);
            continue;

        case TJUMP_FW:
            if(SLOT_IS_FALSE(gP->sp)) ip += *(++ip);
            SLOT_POP(gP->sp);
            break;
            
        case TJUMP_FW_W:
            if(SLOT_IS_FALSE(gP->sp)) ip += *(UInt16*)(++ip) + 1;
            SLOT_POP(gP->sp);
            break;
            
        case TJUMP_BW:
            if(SLOT_IS_FALSE(gP->sp)) ip -= *(ip + 1);
            SLOT_POP(gP->sp);
            continue;

        case TJUMP_BW_W:
            if(SLOT_IS_FALSE(gP->sp)) ip -= *(UInt16*)(ip + 1);
            SLOT_POP(gP->sp);
            break;

        case CALL:
            err = ScriptFuncCall(gP, m, *(++ip));
            if(err) return err;
            break;

        case CALL_LIB:
            err = ScriptLibFuncCall(gP, *(++ip), *(++ip));
            if(err) return err;

            break;

        case CALL_LIB_W:
            err = ScriptLibFuncCall(gP, *(UInt16*)(++ip), *(++ip));
            if(err) return err;

            ++ip;
            break;

        case CALL_URL:
            err = ScriptUrlFuncCall(gP, *(++ip), *(++ip));
            if(err) return err;

            break;

        case CALL_URL_W:
            err = ScriptUrlFuncCall(gP, *(UInt16*)(++ip), *(++ip));
            if(err) return err;

            ++ip;
            break;

        case LOAD_VAR:
            SLOT_PUSH_VAR(gP->sp, bp - *(++ip));
            break;

        case STORE_VAR:
            SLOT_SAVE_VAR(gP->sp, bp - *(++ip));
            break;

        case INCR_VAR:
	    tp = bp - *(++ip);
	    if (tp->type == scriptTypeInteger) {
            	SLOT_TO_INTEGER(tp);
            	xp->value.n++;
	    }
	    else {
            	SLOT_TO_FLOAT(tp);
	    	//xp->value.f++;
		flpArg1.d = xp->value.f;
		flpArg2.d = 1.0;
		FlpBufferCorrectedAdd(&theCompFloat.fd, flpArg1.fd, flpArg2.fd, kDefAccuracy); 
		xp->value.f = theCompFloat.d;
	    }
            break;

        case DECR_VAR:
	    tp = bp - *(++ip);
	    if (tp->type == scriptTypeInteger) {
            	SLOT_TO_INTEGER(tp);
            	xp->value.n--;
	    }
	    else {
	    	SLOT_TO_FLOAT(tp);
            	//xp->value.f--;
		flpArg1.d = xp->value.f;
		flpArg2.d = 1.0;
		FlpBufferCorrectedSub(&theCompFloat.fd, flpArg1.fd, flpArg2.fd, kDefAccuracy); 
		xp->value.f = theCompFloat.d;
	    }
            break;

        case LOAD_CONST:
            SLOT_PUSH_VAR(gP->sp, m->const_pool + *(++ip));
            break;

        case LOAD_CONST_W:
            SLOT_PUSH_VAR(gP->sp, m->const_pool + *(UInt16*)(++ip));
            ++ip;
            break;

        case CONST_0:
            SLOT_PUSH_INTEGER(gP->sp, 0);
            break;

        case CONST_1:
            SLOT_PUSH_INTEGER(gP->sp, 1);
            break;

        case CONST_M1:
            SLOT_PUSH_INTEGER(gP->sp, -1);
            break;

        case CONST_ES:
            SLOT_PUSH_STRING(gP->sp, NULL);
            break;

        case CONST_INVALID:
            SLOT_PUSH_INVALID(gP->sp);
            break;

        case CONST_TRUE:
            SLOT_PUSH_TRUE(gP->sp);
            break;

        case CONST_FALSE:
            SLOT_PUSH_FALSE(gP->sp);
            break;

        case INCR:
	    if (gP->sp->type == scriptTypeInteger) {
            	SLOT_TO_INTEGER(gP->sp);
            	xp->value.n++;
	    }
	    else {
		SLOT_TO_FLOAT(gP->sp);
		//xp->value.f++;
		flpArg1.d = xp->value.f;
		flpArg2.d = 1.0;
		FlpBufferCorrectedAdd(&theCompFloat.fd, flpArg1.fd, flpArg2.fd, kDefAccuracy); 
		xp->value.f = theCompFloat.d;
	    }
            break;

        case DECR:
	    if (gP->sp->type == scriptTypeInteger) {
            	SLOT_TO_INTEGER(gP->sp);
            	xp->value.n--;
	    }
	    else {
		SLOT_TO_FLOAT(gP->sp);
		//xp->value.f--;
		flpArg1.d = xp->value.f;
		flpArg2.d = 1.0;
		FlpBufferCorrectedSub(&theCompFloat.fd, flpArg1.fd, flpArg2.fd, kDefAccuracy); 
		xp->value.f = theCompFloat.d;
	    }
            break;

        case ADD_ASG:
            switch(SlotMixedOp(xp = bp - *(++ip), gP->sp))
            {
            case scriptTypeString:
                SlotStrAppend(xp, gP->sp->value.s, StrLen(gP->sp->value.s));
                break;

            case scriptTypeInteger:
                xp->value.n += gP->sp->value.n;
                break;

            case scriptTypeFloat:
                //xp->value.f += gP->sp->value.f;
		flpArg1.d = xp->value.f;
		flpArg2.d = gP->sp->value.f;
		FlpBufferCorrectedAdd(&theCompFloat.fd, flpArg1.fd, flpArg2.fd, kDefAccuracy); 
		xp->value.f = theCompFloat.d;
                break;
            }

            SLOT_POP(gP->sp);
            break;

        case SUB_ASG:
	    tp = bp - *(++ip);
	    if ((gP->sp->type == scriptTypeInteger) && (tp->type == scriptTypeInteger)) {
            	SLOT_TO_INTEGER(gP->sp);
            	SLOT_TO_INTEGER(tp);
            	xp->value.n -= gP->sp->value.n;
	    } else {
            	SLOT_TO_FLOAT(gP->sp);
            	SLOT_TO_FLOAT(tp);
            	//xp->value.f -= gP->sp->value.f;
		flpArg1.d = xp->value.f;
		flpArg2.d = gP->sp->value.f;
		FlpBufferCorrectedSub(&theCompFloat.fd, flpArg1.fd, flpArg2.fd, kDefAccuracy); 
		xp->value.f = theCompFloat.d;
	    }
            SLOT_FREE(gP->sp);
            break;

        case UMINUS:
	    if (gP->sp->type == scriptTypeInteger) {
            	SLOT_TO_INTEGER(gP->sp);
            	gP->sp->value.n = -gP->sp->value.n;
	    } else {
            	SLOT_TO_FLOAT(gP->sp);
            	gP->sp->value.f = -gP->sp->value.f;
	    }
            break;

        case ADD:
            switch(SlotMixedOp(xp = gP->sp + 1, gP->sp))
            {
            case scriptTypeString:
                SlotStrAppend(xp, gP->sp->value.s, StrLen(gP->sp->value.s));
                break;

            case scriptTypeInteger:
                xp->value.n += gP->sp->value.n;
                break;

            case scriptTypeFloat:
                //xp->value.f += gP->sp->value.f;
		flpArg1.d = xp->value.f;
		flpArg2.d = gP->sp->value.f;
		FlpBufferCorrectedAdd(&theCompFloat.fd, flpArg1.fd, flpArg2.fd, kDefAccuracy); 
		xp->value.f = theCompFloat.d;
                break;
            }

            SLOT_POP(gP->sp);
            break;

        case SUB:
	    tp = gP->sp + 1;
	    if ((gP->sp->type == scriptTypeInteger) && (tp->type == scriptTypeInteger)) {
            	SLOT_TO_INTEGER_OP(gP->sp);
            	xp->value.n -= gP->sp->value.n;
	    } else {
            	SLOT_TO_FLOAT_OP(gP->sp);
            	//xp->value.f -= gP->sp->value.f;
		flpArg1.d = xp->value.f;
		flpArg2.d = gP->sp->value.f;
		FlpBufferCorrectedSub(&theCompFloat.fd, flpArg1.fd, flpArg2.fd, kDefAccuracy); 
		xp->value.f = theCompFloat.d;
	    }
            SLOT_FREE(gP->sp);
            break;

        case MUL:
	    tp = gP->sp + 1;
	    if ((gP->sp->type == scriptTypeInteger) && (tp->type == scriptTypeInteger)) {
            	SLOT_TO_INTEGER_OP(gP->sp);
            	xp->value.n *= gP->sp->value.n;
	    } else {
            	SLOT_TO_FLOAT_OP(gP->sp);
            	xp->value.f = gP->sp->value.f * xp->value.f;
	    }
            SLOT_FREE(gP->sp);
            break;

        case DIV:
	    tp = gP->sp + 1;
	    if ((gP->sp->type == scriptTypeInteger) && (tp->type == scriptTypeInteger)) {
            	SLOT_TO_INTEGER_OP(gP->sp);
            	xp->value.n /= gP->sp->value.n;
	    } else {
            	SLOT_TO_FLOAT_OP(gP->sp);
            	xp->value.f = gP->sp->value.f / xp->value.f;
	    }
            SLOT_FREE(gP->sp);
            break;

        case IDIV:
            SLOT_TO_INTEGER_OP(gP->sp);
            xp->value.n /= gP->sp->value.n;
	    break;

        case REM:
            SLOT_TO_INTEGER_OP(gP->sp);
            xp->value.n %= gP->sp->value.n;
            SLOT_FREE(gP->sp);
            break;

        case B_AND:
            SLOT_TO_INTEGER_OP(gP->sp);
            xp->value.n &= gP->sp->value.n;
            SLOT_FREE(gP->sp);
            break;

        case B_OR:
            SLOT_TO_INTEGER_OP(gP->sp);
            xp->value.n |= gP->sp->value.n;
            SLOT_FREE(gP->sp);
            break;

        case B_XOR:
            SLOT_TO_INTEGER_OP(gP->sp);
            xp->value.n ^= gP->sp->value.n;
            SLOT_FREE(gP->sp);
            break;

        case B_NOT:
            SLOT_TO_INTEGER(gP->sp);
            xp->value.n = ~xp->value.n;
            break;

        case B_LSHIFT:
            SLOT_TO_INTEGER_OP(gP->sp);
            xp->value.n <<= gP->sp->value.n;
            SLOT_FREE(gP->sp);
            break;

        case B_RSSHIFT:
            SLOT_TO_INTEGER_OP(gP->sp);
            xp->value.n >>= gP->sp->value.n;
            SLOT_FREE(gP->sp);
            break;

        case B_RSZSHIFT:
            SLOT_TO_INTEGER_OP(gP->sp);
            xp->value.n >>= gP->sp->value.n;
            xp->value.n &= 0x7FFFFFFF;
            SLOT_FREE(gP->sp);
            break;

        case EQ:
            switch(SlotMixedOp(xp = gP->sp + 1, gP->sp))
            {
            case scriptTypeString:
                SlotReplace(xp, scriptTypeBoolean, ScriptStrCompare(xp->value.s, gP->sp->value.s) == 0);
                break;

            case scriptTypeInteger:
                SlotReplace(xp, scriptTypeBoolean, xp->value.n == gP->sp->value.n);
                break;

            case scriptTypeFloat:
                SlotReplace(xp, scriptTypeBoolean, xp->value.f == gP->sp->value.f);
                break;
            }

            SLOT_POP(gP->sp);
            break;

        case LE:
            switch(SlotMixedOp(xp = gP->sp + 1, gP->sp))
            {
            case scriptTypeString:
                SlotReplace(xp, scriptTypeBoolean, ScriptStrCompare(xp->value.s, gP->sp->value.s) <= 0);
                break;

            case scriptTypeInteger:
                SlotReplace(xp, scriptTypeBoolean, xp->value.n <= gP->sp->value.n);
                break;

            case scriptTypeFloat:
                SlotReplace(xp, scriptTypeBoolean, xp->value.f <= gP->sp->value.f);
                break;
            }

            SLOT_POP(gP->sp);
            break;

        case LT:
            switch(SlotMixedOp(xp = gP->sp + 1, gP->sp))
            {
            case scriptTypeString:
                SlotReplace(xp, scriptTypeBoolean, ScriptStrCompare(xp->value.s, gP->sp->value.s) < 0);
                break;

            case scriptTypeInteger:
                SlotReplace(xp, scriptTypeBoolean, xp->value.n < gP->sp->value.n);
                break;

            case scriptTypeFloat:
                SlotReplace(xp, scriptTypeBoolean, xp->value.f < gP->sp->value.f);
                break;
            }

            SLOT_POP(gP->sp);
            break;

        case GE:
            switch(SlotMixedOp(xp = gP->sp + 1, gP->sp))
            {
            case scriptTypeString:
                SlotReplace(xp, scriptTypeBoolean, ScriptStrCompare(xp->value.s, gP->sp->value.s) >= 0);
                break;

            case scriptTypeInteger:
                SlotReplace(xp, scriptTypeBoolean, xp->value.n >= gP->sp->value.n);
                break;

            case scriptTypeFloat:
                SlotReplace(xp, scriptTypeBoolean, xp->value.f >= gP->sp->value.f);
                break;
            }

            SLOT_POP(gP->sp);
            break;

        case GT:
            switch(SlotMixedOp(xp = gP->sp + 1, gP->sp))
            {
            case scriptTypeString:
                SlotReplace(xp, scriptTypeBoolean, ScriptStrCompare(xp->value.s, gP->sp->value.s) > 0);
                break;

            case scriptTypeInteger:
                SlotReplace(xp, scriptTypeBoolean, xp->value.n > gP->sp->value.n);
                break;

            case scriptTypeFloat:
                SlotReplace(xp, scriptTypeBoolean, xp->value.f > gP->sp->value.f);
                break;
            }

            SLOT_POP(gP->sp);
            break;

        case NE:
            switch(SlotMixedOp(xp = gP->sp + 1, gP->sp))
            {
            case scriptTypeString:
                SlotReplace(xp, scriptTypeBoolean, ScriptStrCompare(xp->value.s, gP->sp->value.s) != 0);
                break;

            case scriptTypeInteger:
                SlotReplace(xp, scriptTypeBoolean, xp->value.n != gP->sp->value.n);
                break;

            case scriptTypeFloat:
                SlotReplace(xp, scriptTypeBoolean, xp->value.f != gP->sp->value.f);
                break;
            }

            SLOT_POP(gP->sp);
            break;

        case NOT:
            SLOT_TO_BOOLEAN(gP->sp);
            gP->sp->value.n = !gP->sp->value.n;
            break;

        case SCAND:
            if(SLOT_IS_FALSE(gP->sp))
            {
                SLOT_PUSH_FALSE(gP->sp);
            }
            break;

        case SCOR:
            SLOT_TO_BOOLEAN(gP->sp);
            if(gP->sp->type != scriptTypeBoolean || gP->sp->value.n)
            {
                SLOT_PUSH_FALSE(gP->sp);
            }
            else
                gP->sp->value.n = WMLS_TRUE;
            break;

        case TOBOOL:
            SLOT_TO_BOOLEAN(gP->sp);
            break;

        case POP:
            SLOT_POP(gP->sp);
            break;

        case TYPEOF:
	    SlotReplace(gP->sp, scriptTypeInteger, gP->sp->type);
            break;

        case ISVALID:
            SlotFree(gP->sp);
            gP->sp->value.n = (gP->sp->type == scriptTypeInvalid) ? WMLS_TRUE : WMLS_FALSE;
            gP->sp->type = scriptTypeBoolean;
            break;

        case RETURN:
            goto __cleanup_vars;

        case RETURN_ES:
            SLOT_PUSH_STRING(gP->sp, NULL);
            goto __cleanup_vars;

        case DBG:
        case RESERVED_3D:
        case RESERVED_3E:
        case RESERVED_3F:
            goto __illegal_instruction;

        case STORE_VAR_0:       case STORE_VAR_1:       case STORE_VAR_2:       case STORE_VAR_3:
        case STORE_VAR_4:       case STORE_VAR_5:       case STORE_VAR_6:       case STORE_VAR_7:
        case STORE_VAR_8:       case STORE_VAR_9:       case STORE_VAR_10:      case STORE_VAR_11:
        case STORE_VAR_12:      case STORE_VAR_13:      case STORE_VAR_14:      case STORE_VAR_15:
            SLOT_SAVE_VAR(gP->sp, bp - (0x0F & *ip));
            break;

        case LOAD_CONST_0:      case LOAD_CONST_1:      case LOAD_CONST_2:      case LOAD_CONST_3:
        case LOAD_CONST_4:      case LOAD_CONST_5:      case LOAD_CONST_6:      case LOAD_CONST_7:
        case LOAD_CONST_8:      case LOAD_CONST_9:      case LOAD_CONST_10:     case LOAD_CONST_11:
        case LOAD_CONST_12:     case LOAD_CONST_13:     case LOAD_CONST_14:     case LOAD_CONST_15:
            SLOT_PUSH_VAR(gP->sp, m->const_pool + (0x0F & *ip));
            break;

        case CALL_0:            case CALL_1:            case CALL_2:            case CALL_3:
        case CALL_4:            case CALL_5:            case CALL_6:            case CALL_7:
            err = ScriptFuncCall(gP, m, 0x07 & *ip);
            if(err) return err;
            break;

        case CALL_LIB_0:        case CALL_LIB_1:        case CALL_LIB_2:        case CALL_LIB_3:
        case CALL_LIB_4:        case CALL_LIB_5:        case CALL_LIB_6:        case CALL_LIB_7:
            err = ScriptLibFuncCall(gP, *(++ip), 0x07 & *ip);
            if(err) return err;

            break;

        case INCR_VAR_0:        case INCR_VAR_1:        case INCR_VAR_2:        case INCR_VAR_3:
        case INCR_VAR_4:        case INCR_VAR_5:        case INCR_VAR_6:        case INCR_VAR_7:
            //SLOT_TO_INTEGER(bp - (0x07 & *ip));
            //xp->value.n++;
	    tp = bp - (0x07 & *ip);
	    if (tp->type == scriptTypeInteger) {
            	SLOT_TO_INTEGER(tp);
            	xp->value.n++;
	    }
	    else {
            	SLOT_TO_FLOAT(tp);
	    	//xp->value.f++;
		flpArg1.d = xp->value.f;
		flpArg2.d = 1.0;
		FlpBufferCorrectedAdd(&theCompFloat.fd, flpArg1.fd, flpArg2.fd, kDefAccuracy); 
		xp->value.f = theCompFloat.d;
	    }
            break;

        case RESERVED_78:       case RESERVED_79:       case RESERVED_7A:       case RESERVED_7B:
        case RESERVED_7C:       case RESERVED_7D:       case RESERVED_7E:       case RESERVED_7F:
            goto __illegal_instruction;

        case JUMP_FW_0:         case JUMP_FW_1:         case JUMP_FW_2:         case JUMP_FW_3:
        case JUMP_FW_4:         case JUMP_FW_5:         case JUMP_FW_6:         case JUMP_FW_7:
        case JUMP_FW_8:         case JUMP_FW_9:         case JUMP_FW_10:        case JUMP_FW_11:
        case JUMP_FW_12:        case JUMP_FW_13:        case JUMP_FW_14:        case JUMP_FW_15:
        case JUMP_FW_16:        case JUMP_FW_17:        case JUMP_FW_18:        case JUMP_FW_19:
        case JUMP_FW_20:        case JUMP_FW_21:        case JUMP_FW_22:        case JUMP_FW_23:
        case JUMP_FW_24:        case JUMP_FW_25:        case JUMP_FW_26:        case JUMP_FW_27:
        case JUMP_FW_28:        case JUMP_FW_29:        case JUMP_FW_30:        case JUMP_FW_31:
            ip += (0x1F & *ip);
            break;

        case JUMP_BW_0:         case JUMP_BW_1:         case JUMP_BW_2:         case JUMP_BW_3:
        case JUMP_BW_4:         case JUMP_BW_5:         case JUMP_BW_6:         case JUMP_BW_7:
        case JUMP_BW_8:         case JUMP_BW_9:         case JUMP_BW_10:        case JUMP_BW_11:
        case JUMP_BW_12:        case JUMP_BW_13:        case JUMP_BW_14:        case JUMP_BW_15:
        case JUMP_BW_16:        case JUMP_BW_17:        case JUMP_BW_18:        case JUMP_BW_19:
        case JUMP_BW_20:        case JUMP_BW_21:        case JUMP_BW_22:        case JUMP_BW_23:
        case JUMP_BW_24:        case JUMP_BW_25:        case JUMP_BW_26:        case JUMP_BW_27:
        case JUMP_BW_28:        case JUMP_BW_29:        case JUMP_BW_30:        case JUMP_BW_31:
            ip -= (0x1F & *ip);
            continue;

        case TJUMP_FW_0:        case TJUMP_FW_1:        case TJUMP_FW_2:        case TJUMP_FW_3:
        case TJUMP_FW_4:        case TJUMP_FW_5:        case TJUMP_FW_6:        case TJUMP_FW_7:
        case TJUMP_FW_8:        case TJUMP_FW_9:        case TJUMP_FW_10:       case TJUMP_FW_11:
        case TJUMP_FW_12:       case TJUMP_FW_13:       case TJUMP_FW_14:       case TJUMP_FW_15:
        case TJUMP_FW_16:       case TJUMP_FW_17:       case TJUMP_FW_18:       case TJUMP_FW_19:
        case TJUMP_FW_20:       case TJUMP_FW_21:       case TJUMP_FW_22:       case TJUMP_FW_23:
        case TJUMP_FW_24:       case TJUMP_FW_25:       case TJUMP_FW_26:       case TJUMP_FW_27:
        case TJUMP_FW_28:       case TJUMP_FW_29:       case TJUMP_FW_30:       case TJUMP_FW_31:
            if(SLOT_IS_FALSE(gP->sp)) ip += (0x1F & *ip);
            SLOT_POP(gP->sp);
            break;

        case LOAD_VAR_0:        case LOAD_VAR_1:        case LOAD_VAR_2:        case LOAD_VAR_3:
        case LOAD_VAR_4:        case LOAD_VAR_5:        case LOAD_VAR_6:        case LOAD_VAR_7:
        case LOAD_VAR_8:        case LOAD_VAR_9:        case LOAD_VAR_10:       case LOAD_VAR_11:
        case LOAD_VAR_12:       case LOAD_VAR_13:       case LOAD_VAR_14:       case LOAD_VAR_15:
        case LOAD_VAR_16:       case LOAD_VAR_17:       case LOAD_VAR_18:       case LOAD_VAR_19:
        case LOAD_VAR_20:       case LOAD_VAR_21:       case LOAD_VAR_22:       case LOAD_VAR_23:
        case LOAD_VAR_24:       case LOAD_VAR_25:       case LOAD_VAR_26:       case LOAD_VAR_27:
        case LOAD_VAR_28:       case LOAD_VAR_29:       case LOAD_VAR_30:       case LOAD_VAR_31:
            SLOT_PUSH_VAR(gP->sp, bp - (0x1F & *ip));
            break;
        }
        
        ip++;
    }

    SLOT_PUSH_STRING(gP->sp, NULL);

__cleanup_vars:
    xp = gP->sp++; // save stack top value
    while(gP->sp <= bp) { SLOT_POP(gP->sp); }
    
    *bp = *xp, gP->sp = bp;
    return errNone;

__illegal_instruction:
    return scriptErrIllegalInstruction;

__verification_failed:
    return scriptErrVerificationFailed;
}

static Err ScriptLibFuncCall(ScriptLibGlobals *gP, UInt16 lindex, UInt8 findex)
{
    Err err;
    UInt16 argc = 0;
    ScriptSlot result;

    // prepare result slot
    result.type = scriptTypeString;
    result.value.s = NULL;

    // invoke library function
    if(lindex >= STDLIB_COUNT)
        err = gP->hostP->LibraryCall(lindex, findex, &argc, gP->sp, &result);
    else
        err = ScriptStdFuncCall(gP, lindex, findex, &argc, gP->sp, &result);

    // clean up parameters in the stack
    while(argc-- > 0) { SLOT_POP(gP->sp); }

    // place result on the top of the stack
    SLOT_ALLOC(gP->sp);
    gP->sp->type    = result.type;
    gP->sp->value.a = result.value.a;

    return err;
}

static Err ScriptUrlFuncCall(ScriptLibGlobals *gP, UInt16 uindex, UInt8 findex)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

/////////////////////////////////////////////////////////////////////////////////////////

Char *ScriptStrCopy(const Char *str)
{
    return ScriptStrCopyEx(str, str ? StrLen(str) : 0);
}

Char *ScriptStrCopyEx(const Char *str, UInt16 len)
{
    Char *res;

    if(!str) return NULL;

    res = MemPtrNew(len + 1);
    if(!res) return NULL;

    StrNCopy(res, str, len);
    *(res + len) = 0;
    return res;
}

void ScriptStrFree(Char *str)
{
    if(str) MemPtrFree(str);
}

Int16 ScriptStrCompare(Char *s1, Char *s2)
{
    if(s1)
        return s2 ? StrCompare(s1, s2) : 1;
    else
        return s2 ? -1 : 0;
}

///////////////////////////////////////////////////////////////////////////////

ScriptSlot *SlotToBoolean(ScriptSlot *ptr)
{
    switch(ptr->type)
    {
    case scriptTypeInteger:
        ptr->type = scriptTypeBoolean;
        break;

    case scriptTypeFloat:
        ptr->type = scriptTypeBoolean;
	ptr->value.n = ptr->value.f ? WMLS_TRUE : WMLS_FALSE;
        break;

    case scriptTypeString:
        SlotReplace(ptr, scriptTypeBoolean, ptr->value.s && StrLen(ptr->value.s));
        break;

    case scriptTypeBoolean:
        break;

    default:
        ptr->type = scriptTypeInvalid;
        break;
    }

    return ptr;
}

ScriptSlot *SlotToInteger(ScriptSlot *ptr)
{
    switch(ptr->type)
    {
    case scriptTypeInteger:
        break;

    case scriptTypeFloat:
        ptr->type = scriptTypeInvalid;
        break;

    case scriptTypeString:
        if(ptr->value.s && StrLen(ptr->value.s))
            SlotReplace(ptr, scriptTypeInteger, StrAToI(ptr->value.s));
        else
            SlotReplace(ptr, scriptTypeInvalid, 0);
        break;

    case scriptTypeBoolean:
        ptr->type = scriptTypeInteger;
        ptr->value.n = ptr->value.n ? 1 : 0;
        break;

    default:
        ptr->type = scriptTypeInvalid;
        break;
    }

    return ptr;
}

ScriptSlot *SlotToString(ScriptSlot *ptr)
{
    Char buf[12], *tmp, flp[22];

    switch(ptr->type)
    {
    case scriptTypeInteger:
        StrIToA(buf, ptr->value.n);
        
        ptr->type = scriptTypeString;
        ptr->value.s = NULL;
        SlotStrAppend(ptr, buf, StrLen(buf));
        break;

    case scriptTypeFloat:
	DoubleToString (ptr->value.f, flp);
        
        ptr->type = scriptTypeString;
        ptr->value.s = NULL;
        SlotStrAppend(ptr, flp, StrLen(flp));
        break;

    case scriptTypeString:
        break;

    case scriptTypeBoolean:
        tmp = ptr->value.n ? "true" : "false";

        ptr->type = scriptTypeString;
        ptr->value.s = NULL;
        SlotStrAppend(ptr, tmp, StrLen(tmp));
        break;

    default:
        ptr->type = scriptTypeInvalid;
        break;
    }

    return ptr;
}

ScriptSlot *SlotToFloat(ScriptSlot *ptr)
{
    switch(ptr->type)
    {
    case scriptTypeInteger:
        ptr->type = scriptTypeFloat;
        ptr->value.f = ptr->value.n;
        break;

    case scriptTypeFloat:
        break;

    case scriptTypeString:
        if(ptr->value.s && StrLen(ptr->value.s)) {
        	ptr->type = scriptTypeFloat;
        	ptr->value.f = StringToDouble(ptr->value.s);
	} else
            ptr->type = scriptTypeInvalid;
        break;

    case scriptTypeBoolean:
        ptr->type = scriptTypeFloat;
        ptr->value.f = ptr->value.n;
        break;

    default:
        ptr->type = scriptTypeInvalid;
        break;
    }

    return ptr;
}

UInt16 SlotMixedOp(ScriptSlot *arg1, ScriptSlot *arg2)
{
    if(arg1->type == scriptTypeString)
    {
        if(arg2->type == scriptTypeString || SlotToString(arg2)->type == scriptTypeString)
            return scriptTypeString;
         
    }
    else if(arg1->type == scriptTypeInteger)
    {
        if(arg2->type == scriptTypeString)
        {
            SlotToString(arg1);
            return scriptTypeString;
        }

        if(arg2->type == scriptTypeInteger)
            return scriptTypeInteger;

        if(arg2->type == scriptTypeBoolean)
        {
            SlotToInteger(arg2);
            return scriptTypeInteger;
        }

        if(arg2->type == scriptTypeFloat)
        {
            SlotToFloat(arg1);
            return scriptTypeFloat;
        }
    }
    else if(arg1->type == scriptTypeBoolean)
    {
        if(arg2->type == scriptTypeString)
        {
            SlotToString(arg1);
            return scriptTypeString;
        }

        SlotToInteger(arg1);

        if(arg2->type == scriptTypeInteger)
            return scriptTypeInteger;

        if(arg2->type == scriptTypeBoolean)
        {
            SlotToInteger(arg2);
            return scriptTypeInteger;
        }

        if(arg2->type == scriptTypeFloat)
        {
            SlotToFloat(arg1);
            return scriptTypeFloat;
        }
    }

    else if(arg1->type == scriptTypeFloat)
    {
        if(arg2->type == scriptTypeString)
        {
            SlotToString(arg1);
            return scriptTypeString;
        }

        if(arg2->type == scriptTypeInteger) {
	    SlotToFloat(arg2);
            return scriptTypeFloat;
	}

        if(arg2->type == scriptTypeBoolean)
        {
            SlotToFloat(arg1);
            return scriptTypeFloat;
        }

        if(arg2->type == scriptTypeFloat)
            return scriptTypeFloat;
    }

    SlotFree(arg1);
    SlotFree(arg2);
        
    return arg1->type = arg2->type = scriptTypeInvalid;
}

///////////////////////////////////////////////////////////////////////////////

void SlotCopy(ScriptSlot *dst, ScriptSlot *src)
{
    if((dst->type = src->type) == scriptTypeString)
        dst->value.s = ScriptStrCopy(src->value.s);
    else
        dst->value.a = src->value.a;
}

void SlotMove(ScriptSlot *dst, ScriptSlot *src)
{
    SlotFree(dst);

    dst->value.a = src->value.a;
    dst->type    = src->type;
    src->type    = scriptTypeInvalid;
}

void SlotFree(ScriptSlot *ptr)
{
    if(ptr->type == scriptTypeString)
    {
        ScriptStrFree(ptr->value.s);
        ptr->value.s = NULL;
    }
}

void SlotReplace(ScriptSlot *ptr, UInt8 type, Int32 value)
{
    SlotFree(ptr);

    ptr->type = type;
    ptr->value.n = value;
}

void SlotStrAppend(ScriptSlot *ptr, Char *str, UInt32 len1)
{
    Char *tmp;

    if(len1)
    {
        if(ptr->value.s)
        {
            UInt32 len2 = StrLen(ptr->value.s);
            UInt32 len3 = MemPtrSize(ptr->value.s);

            if(len1 + len2 >= len3)
            {
                tmp = MemPtrNew((len1 + len2 + 63) & 0xFFFFFFC0);
                ErrFatalDisplayIf(!tmp, "Out of memory [SlotStrAppend]");

                StrCopy(tmp, ptr->value.s);
                MemPtrFree(ptr->value.s);

                ptr->value.s = tmp;
            }
            
            tmp = ptr->value.s + len2;
        }
        else
        {
            ptr->value.s = tmp = MemPtrNew((len1 + 63) & 0xFFFFFFC0);
            ErrFatalDisplayIf(!tmp, "Out of memory [SlotStrAppend]");
        }

        StrNCopy(tmp, str, len1);
        tmp[len1] = 0;
    }
}

void SlotLoadConst(ScriptSlot *slot, UInt8 *src, UInt16 *offsetP)
{
    Int32 size;
    
    switch(src[(*offsetP)++])
    {
    case 0: // 8 bit signed integer
        slot->type = scriptTypeInteger;
        slot->value.n = *(Int8*)(src + *offsetP);
        *offsetP += 1;
        break;

    case 1: // 16 bit signed integer
        slot->type = scriptTypeInteger;
        slot->value.n = *(Int16*)(src + *offsetP);
        *offsetP += 2;
        break;

    case 2: // 32 bit signed integer
        slot->type = scriptTypeInteger;
        slot->value.n = *(Int32*)(src + *offsetP);
        *offsetP += 4;
        break;

    case 3: // 32 floating point
        slot->type = scriptTypeFloat;
        slot->value.f = *(double*)(src + *offsetP);
        *offsetP += 4;
        break;

    case 4: // UTF-8 string
        size = ReadIntMB(src, offsetP);

        slot->type = scriptTypeString;
        slot->value.s = MemPtrNew(size + 1);
        ErrFatalDisplayIf(slot->value.s == NULL, "MemPtrNew failed [SlotLoadConst]");
        
        MemMove(slot->value.s, src + *offsetP, size);
        slot->value.s[size] = 0;

        *offsetP += size;
        break;
        
    case 5: // empty string
        slot->type = scriptTypeString;
        slot->value.s = NULL;
        break;

    case 6: // string with external encoding
        size = ReadIntMB(src, offsetP);

        slot->type = scriptTypeString;
        slot->value.s = MemPtrNew(size + 1);
        ErrFatalDisplayIf(slot->value.s == NULL, "MemPtrNew failed [SlotLoadConst]");
        
        MemMove(slot->value.s, src + *offsetP, size);
        slot->value.s[size] = 0;

        *offsetP += size;
        break;

    default:
        ErrFatalDisplay("Unknown constant type [SlotLoadConst]");
    }
}

Char *SlotParseConst(ScriptSlot *ptr, Char *str)
{
    Int32 sign = 1, value = 0;

    if(*str == '\"' || *str == '\'')
    {
        Char *tmp = str;
        
        do {
            tmp = StrChr(tmp + 1, *str);
            if(tmp == NULL) return NULL;
        } while(*(tmp - 1) == '\\');

        ptr->type = scriptTypeString;
        ptr->value.s = NULL;

        str++;
        SlotStrAppend(ptr, str, tmp - str);
        return tmp + 1;
    }

    if(StrNCompare(str, "false", 5) == 0)
    {
        ptr->type = scriptTypeBoolean;
        ptr->value.n = WMLS_FALSE;
        return str + 5;
    }

    if(StrNCompare(str, "true", 4) == 0)
    {
        ptr->type = scriptTypeBoolean;
        ptr->value.n = WMLS_TRUE;
        return str + 4;
    }

    if(*str == '-')
    {
        while(*(++str) == ' ') ;
        sign = -1;
    }
    
    if ((StrChr(str, '.')) || (StrChr(str, 'e')) || (StrChr(str, 'E'))) { // Floating point

    	Char *tmp;
	Char *pos;

	StrCopy(tmp, str);

	if((pos = StrChr(tmp, ','))) {
		*pos = 0;
	}
	else {
		if((pos = StrChr(tmp, ')'))) {
			*pos = 0;
		}
		else {
			return NULL;
		}
	}

        ptr->type = scriptTypeFloat;
        ptr->value.f = StringToDouble(tmp);

        return str + StrLen(tmp);
    }
    else { // Integer
    	if(*str == '-')
    	{
    		while(*(++str) == ' ') ;
    		sign = -1;
    	}
    	if(*str == '+')
    	{
    		while(*(++str) == ' ') ;
    		sign = +1;
    	}

    	if(*str >= '0' && *str <= '9')
    	{
        	while(*str >= '0' && *str <= '9')
            		value = 10 * value + *str++ - '0';

		if (!value) value = 0;

        	ptr->type = scriptTypeInteger;
        	ptr->value.n = sign * value;
        	return str;
    	}
    }  
 
    return NULL;
}

static Int32 ReadIntMB(UInt8 *buf, UInt16 *offsetP)
{
    Int32 v = 0;
    UInt8 c;
    
    do {
        c = buf[(*offsetP)++];
        v = (v << 7) | (c & 0x7F);
    }
    while((c & 0x80) != 0);

    return v;
}

double StringToDouble(char *str)
{
    FlpCompDouble theCompFloat;
    double theFloat;

	// Convert a string to a float
	//theCompFloat.fd = FlpAToF(str);
	FlpBufferAToF(&theCompFloat.fd, str);
	theFloat = theCompFloat.d;
	return theFloat;
}

Err DoubleToString(double dbl, char *str)
{
    FlpCompDouble theFloat;

	theFloat.d = dbl;
	return FlpFToA(theFloat.fd, str);
}