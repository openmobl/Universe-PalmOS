/* $Id: resize.h,v 1.11 2004/09/28 03:33:17 arpruss Exp $ */
/*******************************************************************************
    Copyright (c) 2004, Alexander R. Pruss
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice, this
        list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright notice, this
        list of conditions and the following disclaimer in the documentation and/or
        other materials provided with the distribution.

        Neither the name of the PalmResize Project nor the names of its
        contributors may be used to endorse or promote products derived from this
        software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef PLUCKER_RESIZE_H
#define PLUCKER_RESIZE_H

//#include "viewer.h"
#include <PalmOS.h>
#include "config.h"
#include "sects.h"

#if ! defined( SUPPORT_DIA ) && defined( HAVE_SILKSCREEN )
# define SUPPORT_DIA
#endif

#if ! defined( SUPPORT_DIA_SONY ) && defined( HAVE_SONY_SDK )
# define SUPPORT_DIA_SONY
#endif

#if ! defined( SUPPORT_DIA_HANDERA ) && defined( HAVE_HANDERA_SDK )
# define SUPPORT_DIA_HANDERA
#endif

#ifndef winDisplayChangedEvent
# define winDisplayChangedEvent 0x4101
#else
# define HAVE_PALM_DIA_SDK
#endif

#include "resizeconsts.h"

#define WORD_LIST_TYPE 'wrdl'

typedef struct {
    UInt16 objectID;
    UInt16 flags;
    UInt16 reserved;
} DIAConfigEntryType;

typedef UInt8 DIAStateType;

typedef struct {
    UInt16             sizeInWords;
    UInt16             flags;
    UInt16             bin;
    UInt16             preferredState;
    DIAConfigEntryType objectList[0];
} DIAConfigType;

typedef struct {
    UInt16 from;
    UInt16 to;
} DIAIndexEntryType;

typedef struct {
    UInt16 count;
    DIAIndexEntryType mapList[ 0 ];
} DIAIndexType;

typedef struct {
    UInt16         formID;
    UInt16         numObjects;
    Boolean        open;
    MemHandle      configHandle;
    Coord          lastExtentX;
    Coord          lastExtentY;
    RectangleType  lastBounds;
    Boolean        forceRedraw;
    DIAConfigType* config;
    RectangleType* originalBounds;
} DIAFormEntryType;

#ifdef SUPPORT_DIA
extern void InitializeResizeSupport( UInt16 formMapId ) RESIZE_SECTION;
extern void TerminateResizeSupport( void ) RESIZE_SECTION;
extern void SetResizePolicy( UInt16 formID ) RESIZE_SECTION;
extern Boolean ResizeHandleFrmOpenEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleFrmCloseEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleWinDisplayChangedEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleWinExitEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleWinEnterEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleFrmRedrawUpdateCode( void ) RESIZE_SECTION;
extern void LoadResizePrefs( UInt32 appID, UInt16 prefID ) RESIZE_SECTION;
extern void SaveResizePrefs( UInt32 appID, UInt16 prefID, Int16 version )
    RESIZE_SECTION;
extern void ResizeRefreshCurrentForm( void ) RESIZE_SECTION;
extern void SetHaveWinDisplayChangedEvent( Boolean value ) RESIZE_SECTION;
extern Boolean ResizeHandleEvent( EventType* event ) RESIZE_SECTION;
#else

#define HandleResizeNotification( x )
#define InitializeResizeSupport( x )
#define TerminateResizeSupport()
#define SetResizePolicy( x )
#define ResizeHandleFrmOpenEvent()  true
#define ResizeHandleFrmCloseEvent() true
#define ResizeHandleWinDisplayChangedEvent() true
#define ResizeHandleWinExitEvent()  true
#define ResizeHandleWinEnterEvent() true
#define LoadResizePrefs( a, b )
#define SaveResizePrefs( a, b, c )
#define ResizeRefreshCurrentForm()
#define SetHaveWinDisplayChangedEvent( x )
#define ResizeHandleFrmRedrawUpdateCode() true
#define ResizeHandleEvent( e ) false

#endif

#endif /* _ARP_RESIZE_H */
