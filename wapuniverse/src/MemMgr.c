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
#include    <MemGlue.h>
#include	"WAPUniverse.h"
#include	"../res/WAPUniverse_res.h"
#include	"http.h"

void MemMgrIncramentSizeCount(Int32 size)
{
    GlobalsType     *g;
    
    if (!size)
        return;
    
    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
    
    if (g) {
        g->pageStats.totalMemoryUsed += size;
    }
}

MemPtr MemMgrChunkNew(UInt32 size)
{
    MemPtr          newPtr = NULL;
    //GlobalsType     *g;
    
    if (!size)
        return NULL;
    
    //FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
    
    newPtr = MemGluePtrNew(size);
    
    if (newPtr /*&& g*/) {
        //g->pageStats.totalMemoryUsed += size;
        MemMgrIncramentSizeCount(size);
    }
    
    return newPtr;
}

Err MemMgrChunkFree(MemPtr ptr)
{
    UInt32          ptrSize = 0;
    Err             result  = errNone;
    //GlobalsType     *g;
    
    if (!ptr)
        return memErrInvalidParam;
        
    //FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
    
    ptrSize = MemPtrSize(ptr);
    
    result = MemPtrFree(ptr);
    
    if ((result == errNone) /*&& g*/) {
        //g->pageStats.totalMemoryUsed -= ptrSize;
        MemMgrIncramentSizeCount(-ptrSize);
    }
    
    return result;
}

Err MemMgrChunkMove(MemPtr dest, MemPtr src, UInt32 size)
{
    Err     result  = errNone;
    
    if (!dest || !src || !size)
        return memErrInvalidParam;
    
    result = MemMove(dest, src, size);
    
    return result;
}

Err MemMgrChunkSet(MemPtr ptr, UInt32 size, UInt8 val)
{
    Err     result  = errNone;
    
    if (!ptr || !size)
        return memErrInvalidParam;
    
    result = MemSet(ptr, size, val);
    
    return result;
}

Err MemMgrChunkResize(MemPtr ptr, UInt32 size)
{
    Err             result  = errNone;
    UInt32          ptrSize = 0;
    //GlobalsType     *g;
    
    if (!ptr || !size)
        return memErrInvalidParam;
        
    //FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
    
    ptrSize = MemPtrSize(ptr);
        
    result = MemPtrResize(ptr, size);
    
    if ((result == errNone) /*&& g*/) {
        //g->pageStats.totalMemoryUsed -= ptrSize;
        MemMgrIncramentSizeCount(-ptrSize);
        //g->pageStats.totalMemoryUsed += abs(ptrSize - size);
        MemMgrIncramentSizeCount(abs(ptrSize - size));
    }
    
    return result;
}

MemPtr MemMgrChunkRealloc(MemPtr ptr, UInt32 size)
{
    MemPtr  local = NULL;

    if (!size) {
        return NULL;
    }

    if (!ptr) {
        return MemMgrChunkNew(size);
    }

    if (MemMgrChunkResize((MemPtr)ptr, size)) {
        local = MemMgrChunkNew(size);
        if (!local) {
            return NULL;
        }

        MemMgrChunkMove(local, ptr, size);

        MemMgrChunkFree(ptr);
        
        return local;
    }

    return ptr;
}

MemPtr MemMgrChunkCombine(MemPtr ptr1, UInt32 size1, MemPtr ptr2, UInt32 size2)
{
    MemPtr  out = NULL;

    if (!ptr1 || !size1 || !ptr2 || !size2) {
        return NULL;
    }

    out = MemMgrChunkRealloc(ptr1, size1 + size2);
    if (!out) {
        return NULL;
    }

    MemMgrChunkMove(out + size1, ptr2, size2);

    return out;
}
