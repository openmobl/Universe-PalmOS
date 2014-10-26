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
 * $Id: wmlslib.c,v 1.2 2004/04/21 16:28:36 york Exp $ 
 */

#include "wmls.h"
#include "MathLib.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

static Char* SubstituteStr( Char* str, const Char* token, Char* sub, UInt16 subLen )
{
	int 			charsToMove;
	UInt16 		tokenLen;
	UInt16 		strLen;
	UInt16 		blockSize;
	Char* 		ptr;
	MemHandle 	strH;

	// Find the start of the token string, if it doesn't exist, exit.
	ptr = StrStr( str, token );
	if( ptr == NULL ) return str;
	
	tokenLen = StrLen( token );
	charsToMove = subLen - tokenLen;
		
	// Resize the string if necessary.
	strH = MemPtrRecoverHandle( str );
	strLen = StrLen( str );
	blockSize = MemHandleSize( strH );
	if( strLen + charsToMove + 1 >= blockSize )
		{
		MemHandleUnlock( strH );
		MemHandleResize( strH, strLen + charsToMove + 1 );
		str = (Char*)MemHandleLock( strH );
		ptr = StrStr( str, token );
		ErrNonFatalDisplayIf( ptr == NULL, "Msg missing token" );
		}
	
	// Make room for the substitute string.
	if( charsToMove )
		MemMove( ptr + subLen, ptr + tokenLen, StrLen( ptr + tokenLen )+1 );
		
	// Replace the token with the substitute string.
	MemMove( ptr, sub, subLen );
	
	return str;
}

//////////////////////////////////////////////////////////////////////////////////
// Lang

// <number> Lang.abs(<number>)
//
static Err Lang_abs(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToInteger(sp);

    rs->type = sp->type;
    rs->value.n = (sp->value.n >= 0) ? sp->value.n : -sp->value.n;
    return 0;
}

// <number> Lang.min(<number>, <number>)
//
static Err Lang_min(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToInteger(sp);
    SlotToInteger(sp + 1);

    if(sp->type == scriptTypeInteger && (sp + 1)->type == scriptTypeInteger)
    {
        rs->type = scriptTypeInteger;
        rs->value.n = (sp->value.n < (sp + 1)->value.n) ? sp->value.n : (sp + 1)->value.n;
    }
    else
        rs->type = scriptTypeInvalid;

    return 0;
}

// <number> Lang.max(<number>, <number>)
//
static Err Lang_max(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToInteger(sp);
    SlotToInteger(sp + 1);

    if(sp->type == scriptTypeInteger && (sp + 1)->type == scriptTypeInteger)
    {
        rs->type = scriptTypeInteger;
        rs->value.n = (sp->value.n > (sp + 1)->value.n) ? sp->value.n : (sp + 1)->value.n;
    }
    else
        rs->type = scriptTypeInvalid;

    return 0;
}

// <number> Lang.parseInt(<string>)
//
static Err Lang_parseInt(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToInteger(sp);
    SlotMove(rs, sp);

    return 0;
}

// <number> Lang.parseFloat(<string>)
//
static Err Lang_parseFloat(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToFloat(sp);
    SlotMove(rs, sp);
    return 0;
}

// <boolean> Lang_isInt(<any>)
//
static Err Lang_isInt(ScriptSlot *sp, ScriptSlot *rs)
{
    if(sp->type != scriptTypeInvalid)
    {
        rs->type = scriptTypeBoolean;
        rs->value.n = (sp->type == scriptTypeInteger) ? WMLS_TRUE : WMLS_FALSE;
    }
    else
    {
        rs->type = scriptTypeInvalid;
    }

    return 0;
}

// <boolean> Lang_isFloat(<any>)
//
static Err Lang_isFloat(ScriptSlot *sp, ScriptSlot *rs)
{
    if(sp->type != scriptTypeInvalid)
    {
        rs->type = scriptTypeBoolean;
        rs->value.n = (sp->type == scriptTypeFloat) ? WMLS_TRUE : WMLS_FALSE;
    }
    else
    {
        rs->type = scriptTypeInvalid;
    }
    return 0;
}

// <number> Lang.maxInt()
//
static Err Lang_maxInt(ScriptSlot *sp, ScriptSlot *rs)
{
    rs->type = scriptTypeInteger;
    rs->value.n = 0x7FFFFFFFL;
    return 0;
}

// <number> Lang.minInt()
//
static Err Lang_minInt(ScriptSlot *sp, ScriptSlot *rs)
{
    rs->type = scriptTypeInteger;
    rs->value.n = 0x80000000L;
    return 0;
}

// <boolean> Lang.float()
//
static Err Lang_float(ScriptSlot *sp, ScriptSlot *rs)
{
    rs->type = scriptTypeBoolean;
    rs->value.n = WMLS_TRUE;//WMLS_FALSE;
    return 0;
}

// <none> Lang.exit(<any>)
//
static Err Lang_exit(ScriptSlot *sp, ScriptSlot *rs)
{
    return scriptErrExited;
}

// <none> Lang.abort(<any>)
//
static Err Lang_abort(ScriptSlot *sp, ScriptSlot *rs)
{
    return scriptErrAborted;
}

// <number> Lang.random(<number>)
//
static Err Lang_random(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToInteger(sp);

    if(sp->type == scriptTypeInteger)
    {
        if(sp->value.n > 0)
        {
            Int32 v1 = SysRandom(0);
            Int32 v2 = SysRandom(0);

            rs->type = scriptTypeInteger;
            rs->value.n = ((v1 << 15) ^ v2) % (sp->value.n + 1);
            return 0;
        }
    }

    rs->type = scriptTypeInvalid;
    return 0;
}

// <string> Lang.seed(<number>)
//
static Err Lang_seed(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToInteger(sp);

    if(sp->type == scriptTypeInteger)
    {
        SysRandom((sp->value.n >= 0) ? sp->value.n : TimGetSeconds());

        rs->type = scriptTypeString;
        rs->value.s = NULL;
    }
    else
        rs->type = scriptTypeInvalid;

    return 0;
}

// <number> Lang.characterSet()
//
static Err Lang_characterSet(ScriptSlot *sp, ScriptSlot *rs)
{
    rs->type = scriptTypeInteger;
    rs->value.n = 4; // latin1
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////
// Float

// <number> Float.int(<number>)
//
static Err Float_int(ScriptLibGlobals *gP, ScriptSlot *sp, ScriptSlot *rs)
{
    ScriptSlot *p = SlotToInteger(sp);

    rs->type = scriptTypeInteger;
    rs->value.n = p->value.n;

    //rs->type = scriptTypeInvalid;
    return 0;
}

// <number> Float.floor(<number>)
//
static Err Float_floor(ScriptLibGlobals *gP, ScriptSlot *sp, ScriptSlot *rs)
{
    ScriptSlot *p = SlotToFloat(sp);

    //rs->type = scriptTypeFloat;
    //rs->value.f = floor(p->value.f);
    rs->type = scriptTypeFloat;
    MathLibFloor(gP->MathLibRef, p->value.f, &rs->value.f);//rs->value.n = floor(p->value.f);
    return 0;
}

// <number> Float.ceil(<number>)
//
static Err Float_ceil(ScriptLibGlobals *gP, ScriptSlot *sp, ScriptSlot *rs)
{
    ScriptSlot *p = SlotToFloat(sp);

    //rs->type = scriptTypeFloat;
    //rs->value.f = ceil(p->value.f);
    rs->type = scriptTypeFloat;
    MathLibCeil(gP->MathLibRef, p->value.f, &rs->value.f);//rs->value.n = ceil(p->value.f);
    return 0;
}

// <number> Float.pow(<number>, <number>)
//
static Err Float_pow(ScriptLibGlobals *gP, ScriptSlot *sp, ScriptSlot *rs)
{
    ScriptSlot *p1 = SlotToFloat(sp + 1);
    ScriptSlot *p2 = SlotToFloat(sp);

    rs->type = scriptTypeFloat;
    //rs->value.f = pow(p1->value.f, p2->value.f);
    MathLibPow(gP->MathLibRef, p1->value.f, p2->value.f, &rs->value.f);
    return 0;
}

// <number> Float.round(<number>)
//
static Err Float_round(ScriptLibGlobals *gP, ScriptSlot *sp, ScriptSlot *rs)
{
    rs->type = scriptTypeInvalid;
    return 0;
}

// <number> Float.sqrt(<number>)
//
static Err Float_sqrt(ScriptLibGlobals *gP, ScriptSlot *sp, ScriptSlot *rs)
{
    ScriptSlot *p = SlotToFloat(sp);

    rs->type = scriptTypeFloat;
    MathLibSqrt(gP->MathLibRef, p->value.f, &rs->value.f);//rs->value.f = sqrt(p->value.f);
    return 0;
}

// <number> Float.maxFloat()
//
static Err Float_maxFloat(ScriptLibGlobals *gP, ScriptSlot *sp, ScriptSlot *rs)
{
    //rs->type = scriptTypeInteger;//scriptTypeInvalid;
    //rs->value.n = FLP_MAX;
    rs->type = scriptTypeFloat;//scriptTypeInvalid;
    rs->value.f = 3.40282347E+38;//FLP_MAX;
    return 0;
}

// <number> Float.minFloat()
//
static Err Float_minFloat(ScriptLibGlobals *gP, ScriptSlot *sp, ScriptSlot *rs)
{
    //rs->type = scriptTypeInteger;//scriptTypeInvalid;
    //rs->value.n = FLP_MIN;
    rs->type = scriptTypeFloat;
    rs->value.f = 1.17549435E-38;//FLP_MIN;
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////
// String

// <number> String.length(<string>)
//
static Err String_length(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToString(sp);

    if(sp->type == scriptTypeString)
    {
        rs->type = scriptTypeInteger;
        rs->value.n = sp->value.s ? StrLen(sp->value.s) : 0;
    }
    else
        rs->type = scriptTypeInvalid;

    return 0;
}

// <number> String.isEmpty(<string>)
//
static Err String_isEmpty(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToString(sp);

    if(sp->type == scriptTypeString)
    {
        rs->type = scriptTypeBoolean;
        rs->value.n = (sp->value.s && StrLen(sp->value.s)) ? WMLS_TRUE : WMLS_FALSE;
    }
    else
        rs->type = scriptTypeInvalid;

    return 0;
}

// <string> String.charAt(<string>, <number>)
//
static Err String_charAt(ScriptSlot *sp, ScriptSlot *rs)
{
    ScriptSlot *p1 = SlotToString(sp + 1);
    ScriptSlot *p2 = SlotToInteger(sp);

    if(p1->type == scriptTypeString && p2->type == scriptTypeInteger)
    {
        rs->type = scriptTypeString;
        rs->value.s = (p1->value.s && p2->value.n >= 0 && p2->value.n < StrLen(p1->value.s)) ?
            ScriptStrCopyEx(p1->value.s + p2->value.n, 1) : NULL;
    }
    else
        rs->type = scriptTypeInvalid;

    return 0;
}

// <string> String.subString(<string>, <number>, <number>)
//
static Err String_subString(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <number> String.find(<string>, <string>)
//
static Err String_find(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <string> String.replace(<string>, <string>, <string>)
//
static Err String_replace(ScriptSlot *sp, ScriptSlot *rs)
{
    ScriptSlot *p1 = SlotToString(sp + 2);
    ScriptSlot *p2 = SlotToString(sp + 1);
    ScriptSlot *p3 = SlotToString(sp);
    Char *temp;

    temp = SubstituteStr( p1->value.s, p2->value.s, p3->value.3, StrLen(p3->value.s) );
    SlotSetString(rp, temp);
    MemPtrFree(temp);

    return 0;
    // TODO: implement me!!!
    //return scriptErrNotImplemented;
}

// <number> String.elements(<string>, <string>)
//
static Err String_elements(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <string> String.elementAt(<string>, <number>, <string>)
//
static Err String_elementAt(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <string> String.removeAt(<string>, <number>, <string>)
//
static Err String_removeAt(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <string> String.replaceAt(<string>, <string>, <number>, <string>)
//
static Err String_replaceAt(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <string> String.insertAt(<string>, <string>, <number>, <string>)
//
static Err String_insertAt(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <string> String.squeeze(<string>)
//
static Err String_squeeze(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <string> String.trim(<string>)
//
static Err String_trim(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <number> String.compare(<string>, <string>)
//
static Err String_compare(ScriptSlot *sp, ScriptSlot *rs)
{
    ScriptSlot *p1 = SlotToString(sp + 1);
    ScriptSlot *p2 = SlotToString(sp);

    if(p1->type == scriptTypeString && p2->type == scriptTypeString)
    {
        rs->type = scriptTypeInteger;
        rs->value.n = StrCompare(p1->value.s ? p1->value.s : "", p2->value.s ? p2->value.s : "");
    }
    else
        rs->type = scriptTypeInvalid;

    return 0;
}

// <string> String.toString(<any>)
//
static Err String_toString(ScriptSlot *sp, ScriptSlot *rs)
{
    SlotToString(sp);

    rs->value.a = sp->value.a;
    rs->type    = sp->type;
    sp->type    = scriptTypeInvalid;
    return 0;
}

// <string> String.format(<string>, <any>)
//
static Err String_format(ScriptSlot *sp, ScriptSlot *rs)
{
    // TODO: implement me!!!
    return scriptErrNotImplemented;
}

// <string> String.replaceChars(<string>, <string>, <number>)
//
static Err String_replaceChars(ScriptSlot *sp, ScriptSlot *rs)
{
    ScriptSlot *p1 = SlotToString(sp + 2);
    ScriptSlot *p2 = SlotToString(sp + 1);
    ScriptSlot *p3 = SlotToInteger(sp);

    if(p1->type == scriptTypeString &&
       p2->type == scriptTypeString &&
       p3->type == scriptTypeInteger)
    {
        Int32 l1, l2;

        if(p1->value.s && p2->value.s && (l1 = (Int32)StrLen(p1->value.s) - p3->value.n - 1) > 0)
        {
            l2 = StrLen(p2->value.s);

            StrNCopy(p1->value.s + p3->value.n, p2->value.s, MIN(l1, l2));
        }

        // move value to result
        rs->value.a = p1->value.a;
        rs->type    = p1->type;
        p1->type    = scriptTypeInvalid;
    }
    else
        rs->type = scriptTypeInvalid;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////
// Standard built-in library API table initialization

Err ScriptStdFuncCall(ScriptLibGlobals *gP, UInt16 lindex, UInt8 findex, UInt16 *argcP, ScriptSlot *sp, ScriptSlot *rp)
{
    Err err;

    switch(lindex)
    {
    case 0:
        switch(findex)
        {
        case  0: *argcP = 1; err = Lang_abs(sp, rp); break;
        case  1: *argcP = 2; err = Lang_min(sp, rp); break;
        case  2: *argcP = 2; err = Lang_max(sp, rp); break;
        case  3: *argcP = 1; err = Lang_parseInt(sp, rp); break;
        case  4: *argcP = 1; err = Lang_parseFloat(sp, rp); break;
        case  5: *argcP = 1; err = Lang_isInt(sp, rp); break;
        case  6: *argcP = 1; err = Lang_isFloat(sp, rp); break;
        case  7: *argcP = 0; err = Lang_maxInt(sp, rp); break;
        case  8: *argcP = 0; err = Lang_minInt(sp, rp); break;
        case  9: *argcP = 0; err = Lang_float(sp, rp); break;
        case 10: *argcP = 1; err = Lang_exit(sp, rp); break;
        case 11: *argcP = 1; err = Lang_abort(sp, rp); break;
        case 12: *argcP = 1; err = Lang_random(sp, rp); break;
        case 13: *argcP = 1; err = Lang_seed(sp, rp); break;
        case 14: *argcP = 0; err = Lang_characterSet(sp, rp); break;
        default: return scriptErrFunctionNotFound;
        }
        break;

    case 1:
        switch(findex)
        {
        case  0: *argcP = 1; err = Float_int(gP, sp, rp); break;
        case  1: *argcP = 1; err = Float_floor(gP, sp, rp); break;
        case  2: *argcP = 1; err = Float_ceil(gP, sp, rp); break;
        case  3: *argcP = 2; err = Float_pow(gP, sp, rp); break;
        case  4: *argcP = 1; err = Float_round(gP, sp, rp); break;
        case  5: *argcP = 1; err = Float_sqrt(gP, sp, rp); break;
        case  6: *argcP = 0; err = Float_maxFloat(gP, sp, rp); break;
        case  7: *argcP = 0; err = Float_minFloat(gP, sp, rp); break;
        default: return scriptErrFunctionNotFound;
        }
        break;

    case 2:
        switch(findex)
        {
        case  0: *argcP = 1; err = String_length(sp, rp); break;
        case  1: *argcP = 1; err = String_isEmpty(sp, rp); break;
        case  2: *argcP = 2; err = String_charAt(sp, rp); break;
        case  3: *argcP = 3; err = String_subString(sp, rp); break;
        case  4: *argcP = 2; err = String_find(sp, rp); break;
        case  5: *argcP = 3; err = String_replace(sp, rp); break;
        case  6: *argcP = 2; err = String_elements(sp, rp); break;
        case  7: *argcP = 3; err = String_elementAt(sp, rp); break;
        case  8: *argcP = 3; err = String_removeAt(sp, rp); break;
        case  9: *argcP = 4; err = String_replaceAt(sp, rp); break;
        case 10: *argcP = 4; err = String_insertAt(sp, rp); break;
        case 11: *argcP = 1; err = String_squeeze(sp, rp); break;
        case 12: *argcP = 1; err = String_trim(sp, rp); break;
        case 13: *argcP = 2; err = String_compare(sp, rp); break;
        case 14: *argcP = 1; err = String_toString(sp, rp); break;
        case 15: *argcP = 2; err = String_format(sp, rp); break;
        case 16: *argcP = 3; err = String_replaceChars(sp, rp); break;
        default: return scriptErrFunctionNotFound;
        }
        break;

    default:
        return scriptErrLibraryNotFound;
    }

    return err;
};
