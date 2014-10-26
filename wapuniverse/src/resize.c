/* $Id: resize.c,v 1.33 2004/12/09 20:23:08 arpruss Exp $ */
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

#include <PalmOS.h>
#include <LstGlue.h>
#include "util.h"
#include "DIA.h"
#ifdef SUPPORT_DIA_HANDERA
#include <Vga.h>
#endif

#define DebugInfo(s) /* { char _tempstring[500];StrPrintF(_tempstring,"%s(%d) ",s,active.formID);\
        ClipboardAppendItem(clipboardText,_tempstring,StrLen(_tempstring)); } */
        
typedef struct {
    UInt16    formID;
    FormType* formPtr;
    UInt16    index;
    UInt16    flags;
    DIAFormEntryType* entry;
} ResizeTemporaryData;


static DIAFormEntryType* formList = NULL;
static DIAStateType*     bins;
static UInt16            numBins;
static UInt16            formCount = 0;
static ResizeTemporaryData active;
static Boolean           haveWinDisplayChangedEvent = false;



RESIZE_SECTION static UInt16 StandardExtent( void )
{
#ifdef SUPPORT_DIA_HANDERA
    if ( GetDIAHardware() == DIA_HARDWARE_HANDERA )
        return 240;
    else
#endif
        return 160;
}



/* Update form, but only once */
void UniqueUpdateForm( UInt16 formID, UInt16 code )
{
    EventType e;
    MemSet( &e, sizeof(EventType), 0 );
    e.eType                     = frmUpdateEvent;
    e.data.frmUpdate.formID     = formID;
    e.data.frmUpdate.updateCode = code;
    EvtAddUniqueEventToQueue( &e, 0, true );
    DebugInfo("Queuing frmUpdateForm");
}



/* Same as FrmGetObjectIndex() but works with GSI_OBJECT_ID, too */
RESIZE_SECTION static UInt16 MyFrmGetObjectIndex( FormPtr formPtr, UInt16 id )
{
    UInt16 i;
    if ( id != GSI_OBJECT_ID )
        return FrmGetObjectIndex( formPtr, id );
    for ( i = FrmGetNumberOfObjects( formPtr ) - 1 ; 0 < i ; i -- ) {
        if ( FrmGetObjectType( formPtr, i ) == frmGraffitiStateObj ) {
            return i;
        }
    }
    return 0;
}



RESIZE_SECTION static void WinGlueGetBounds( WinHandle winH, RectangleType* rect )
{
    Err     err;
    UInt32  version;
    err = FtrGet( sysFtrCreator, sysFtrNumROMVersion, &version );
    if ( err == errNone && 0x04003000 <= version ) {
        WinGetBounds( winH, rect );
    }
    else {
        WinHandle oldH;
        oldH = WinSetDrawWindow( winH );
        WinGetDrawWindowBounds( rect );
        WinSetDrawWindow( oldH );
    }
}



RESIZE_SECTION static void SafeFrmGetObjectBounds
    (
    const FormType* formPtr,
    UInt16 objectIndex,
    RectangleType* rect
    )
{
    WinHandle oldWin;

    oldWin = WinSetDrawWindow( FrmGetWindowHandle( formPtr ) );

    FrmGetObjectBounds( formPtr, objectIndex, rect );

    if ( NULL != oldWin )
        WinSetDrawWindow( oldWin );
}



RESIZE_SECTION static void SafeFrmSetObjectBounds
    (
    FormType* formPtr,
    UInt16 objectIndex,
    RectangleType* rect
    )
{
    WinHandle oldWin;

    oldWin = WinSetDrawWindow( FrmGetWindowHandle( formPtr ) );

    FrmSetObjectBounds( formPtr, objectIndex, rect );

    if ( NULL != oldWin )
        WinSetDrawWindow( oldWin );
}



RESIZE_SECTION static void AddForm( UInt16 indexInList, MemHandle handle, UInt16 formID )
{
    UInt16         numObjects;
    DIAConfigType* config;

    formList[ indexInList ].configHandle = handle;
    if ( handle == NULL ) {
        return;
    }

    config          = ( DIAConfigType* )MemHandleLock( handle );
    numObjects      = ( ( config->sizeInWords + 1 ) * sizeof( UInt16 ) -
                        sizeof( DIAConfigType ) ) /
                          sizeof( DIAConfigEntryType );

    formList[ indexInList ].formID         = formID;
    formList[ indexInList ].config         = config;
    formList[ indexInList ].numObjects     = numObjects;
    formList[ indexInList ].open           = false;

    if ( config->flags & DIA_FORM_USE_BIN &&
         numBins < 1 + config->bin )
        numBins = 1 + config->bin;
}



/* This sets up the original bounds array as needed */
RESIZE_SECTION static void InitialSetupForm( UInt16 indexInList ) 
{
    RectangleType* originalBounds;
    FormType*      formPtr;
    UInt16         numObjects;
    UInt16         i;

    numObjects = formList[ indexInList ].numObjects;
    if ( numObjects == 0 )
        return;
    if ( formList[ indexInList ].originalBounds != NULL )
        return;
    formPtr = FrmGetFormPtr( formList[ indexInList ].formID );
    if ( formPtr == NULL )
        return;

    originalBounds = ( RectangleType* )SafeMemPtrNew( numObjects * 
                                           sizeof( RectangleType ) );
    formList[ indexInList ].originalBounds = originalBounds;

    /* First object is the form itself */
    WinGlueGetBounds( FrmGetWindowHandle( formPtr ), &( originalBounds[ 0 ] ) );

    /* Now on to the other objects */
    for ( i = 1 ; i < numObjects ; i++ ) {
         UInt16 objectIndex;
         objectIndex = MyFrmGetObjectIndex( formPtr,
                     formList[ indexInList ].config->objectList[ i ].objectID );
         SafeFrmGetObjectBounds( formPtr, objectIndex, &( originalBounds[ i ] ) );
    }
}



/* Adjust one coordinate */
RESIZE_SECTION static Coord Adjust
    (
    Coord value,            /* old value to adjust */
    Coord oldObjectExtent,  /* old extent if relevant */
    Coord newObjectExtent,  /* already adjusted extent if relevant */
    Coord oldExtent,        /* old form/screen extent */
    Coord newExtent,        /* new form/screen extent */
    UInt16 shiftedFlag      /* adjustment flag, shifted down as needed */
    )
{
    Coord delta;
    
    delta = newExtent - oldExtent;

    switch ( shiftedFlag ) {
        case DIA_X_FIX >> DIA_SHIFT_X:
            return value;
        case DIA_X_RATIO >> DIA_SHIFT_X:
            return value * ( Int32 )newExtent / oldExtent;
        case DIA_X_RIGHT >> DIA_SHIFT_X:
            return newExtent - ( oldExtent - value - oldObjectExtent ) - newObjectExtent;
        case DIA_X_PLUS_DW >> DIA_SHIFT_X:
            return value + delta;
        case DIA_X_PLUS_1_2_DW >> DIA_SHIFT_X:
            return value + delta / 2;
        case DIA_X_PLUS_1_3_DW >> DIA_SHIFT_X:
            return value + delta / 3;
        case DIA_X_PLUS_2_3_DW >> DIA_SHIFT_X:
            return value + 2 * delta / 3;
        case DIA_X_PLUS_1_4_DW >> DIA_SHIFT_X:
            return value + delta / 4;
        case DIA_X_PLUS_3_4_DW >> DIA_SHIFT_X:
            return value + 3 * delta / 4;
        default: /* should not happen */
            return value;
    }
}



/* Get new bounds for an object, adjusted for DIA */
RESIZE_SECTION static void GetNewBounds( UInt16 displayExtentX, UInt16 displayExtentY,
    UInt16 stdFormExtentX, UInt16 stdFormExtentY,
    RectangleType* oldBounds, RectangleType* newBounds,
    DIAConfigEntryType* configEntry )
{
    UInt16 flags;
    flags = configEntry->flags;

    if ( flags & DIA_MASK_W ) {
        newBounds->extent.x  = Adjust( oldBounds->extent.x, 0, 0, 
                                   stdFormExtentX,
                                   displayExtentX,
                                   ( flags & DIA_MASK_W ) >> DIA_SHIFT_W );
    }
    if ( flags & DIA_MASK_H ) {
        newBounds->extent.y  = Adjust( oldBounds->extent.y, 0, 0,
                                   stdFormExtentY,
                                   displayExtentY,
                                   ( flags & DIA_MASK_H ) >> DIA_SHIFT_H );
    }
    newBounds->topLeft.x = Adjust( oldBounds->topLeft.x, oldBounds->extent.x,
                               newBounds->extent.x,
                               stdFormExtentX, displayExtentX,
                               ( flags & DIA_MASK_X ) >> DIA_SHIFT_X );
    newBounds->topLeft.y = Adjust( oldBounds->topLeft.y, oldBounds->extent.y,
                               newBounds->extent.y,
                               stdFormExtentY, displayExtentY,
                               ( flags & DIA_MASK_Y ) >> DIA_SHIFT_Y );
}



/* This compares two rectangles.  If sonyDoubleSecond is set and the first
   bounds are exactly double the size of the first and DIA hardware is
   DIA_HARDWARE_SONY1, then they also match. */
RESIZE_SECTION static Boolean SameBounds( RectangleType* a, RectangleType* b,
    Boolean sonyDoubleSecond )
{
    Boolean compare;
    compare = ( a->extent.y == b->extent.y && a->extent.x == b->extent.x &&
                a->topLeft.y == b->topLeft.y && a->topLeft.x == b->topLeft.x );
#ifdef SUPPORT_DIA_SONY
    if ( ! compare && GetDIAHardware() == DIA_HARDWARE_SONY1 ) {
        return a->extent.y == 2 * b->extent.y &&
               a->extent.x == 2 * b->extent.x &&
               a->topLeft.y == 2 * b->topLeft.y &&
               a->topLeft.x == 2 * b->topLeft.x;
    }
#endif
    return compare;
}



/* Returns true if there is moving done */
RESIZE_SECTION static Boolean MoveObjectsInForm( UInt16 indexInList ) 
{
    Coord     extentX;
    Coord     extentY;
    UInt16    i;
    UInt16    focus;
    FormType* formPtr;
    DIAFormEntryType* entry;

    RectangleType  newBounds;
    RectangleType  curBounds;
    
    entry = &( formList[ indexInList ] );

    formPtr = FrmGetFormPtr( entry->formID );
    if ( formPtr == NULL )
        return false;
    InitialSetupForm( indexInList );
    WinGetDisplayExtent( &extentX, &extentY );

    /* First object is the form itself */
    WinGlueGetBounds( FrmGetWindowHandle( formPtr ), &curBounds );
    newBounds = curBounds;
    GetNewBounds( extentX, extentY, StandardExtent(), StandardExtent(),
         &( formList[ indexInList ].originalBounds[ 0 ] ),
         &newBounds, &( entry->config->objectList[ 0 ] ) );

/* { Char ss[55]; StrPrintF(ss, " %d x %d ", extentX, extentY ); DebugInfo(ss); DebugInfo("abc"); } DebugInfo("def"); */

    if ( ( SameBounds( &curBounds, &newBounds, true ) ||
           ( formList[ indexInList ].open &&
           SameBounds( &newBounds, &( formList[ indexInList ].lastBounds ), false ) ) ) &&
           extentX == entry->lastExtentX && extentY == entry->lastExtentY ) {
        DebugInfo( "skipping resize" );
        return false;
    }

    if ( GetDIAHardware() == DIA_HARDWARE_SONY1 || 
         GetDIAHardware() == DIA_HARDWARE_HANDERA ) {
         /* Sony OS4 and Handera work differently from everybody else */
        FrmEraseForm( formPtr );
    }

    entry->lastBounds  = newBounds;
    entry->lastExtentX = extentX;
    entry->lastExtentY = extentY;
    WinSetBounds( FrmGetWindowHandle(formPtr), &newBounds );

    /* The rest is resized relative to the form itself */
    extentX = newBounds.extent.x;
    extentY = newBounds.extent.y;
    focus   = FrmGetFocus( formPtr );

    for ( i = 1 ; i < entry->numObjects ; i++ ) {
         UInt16  objectIndex;

         objectIndex = MyFrmGetObjectIndex( formPtr,
                     entry->config->objectList[ i ].objectID );
         SafeFrmGetObjectBounds( formPtr, objectIndex, &curBounds );
         newBounds = curBounds;
         GetNewBounds( extentX, extentY,
             entry->originalBounds[ 0 ].extent.x,
             entry->originalBounds[ 0 ].extent.y,
             &( entry->originalBounds[ i ] ),
             &newBounds, &( entry->config->objectList[ i ] ) );

         if ( ! SameBounds( &curBounds, &newBounds, false ) ) {
             SafeFrmSetObjectBounds( formPtr, objectIndex, &newBounds );
             
             switch ( FrmGetObjectType( formPtr, objectIndex ) ) {
                 case frmFieldObj:
    /*             case frmScrollBarObj: */
                 {
                     FieldType* field;
                     UInt16     insPt;

                     field   = ( FieldType* ) FrmGetObjectPtr( formPtr, objectIndex );

                     if ( entry->config->objectList[ i ].flags & DIA_H_GRANULAR ) {
                         FontID oldFont;
                         Coord  newHeight;

                         oldFont   = FntSetFont( FldGetFont( field ) );

                         newHeight = newBounds.extent.y / FntLineHeight() *
                                         FntLineHeight();
                                         
                         FntSetFont( oldFont );

                         if ( 0 < newHeight ) {
                             newBounds.extent.y = newHeight;
                             SafeFrmSetObjectBounds( formPtr, objectIndex, &newBounds );
                         }
                     }
                     insPt   = FldGetInsPtPosition( field );
                     FldRecalculateField( field, true );
                     FldSetInsPtPosition( field, insPt );
                     FldSendChangeNotification( field );
                     break;
                 }
                 case frmListObj:
                     if ( entry->config->objectList[ i ].flags & DIA_H_GRANULAR ) {
                         ListType* list;
                         FontID oldFont;
                         Coord  newHeight;

                         list   = ( ListType* ) FrmGetObjectPtr( formPtr, objectIndex );

                         oldFont   = FntSetFont( LstGlueGetFont( list ) );

                         newHeight = newBounds.extent.y / FntLineHeight() *
                                         FntLineHeight();

                         if ( 0 < newHeight ) {
                             newBounds.extent.y = newHeight;
                             SafeFrmSetObjectBounds( formPtr, objectIndex, &newBounds );
                         }
                     }
                     break;

                 default:
                     break;
             }
         }
    }

    if ( FrmGetFocus( formPtr ) != focus )
        FrmSetFocus( formPtr, focus );

    return true;
}



void InitializeResizeSupport( UInt16 formMapId )
{
    MemHandle     indexHandle;
    DIAIndexType* index;
    UInt16        i;

    if ( formList != NULL )
        return;
    indexHandle = DmGetResource( WORD_LIST_TYPE, formMapId );
    if ( indexHandle == NULL )
        return;

    if ( DIA_HARDWARE_NONE == InitializeDIA() )
        return;

    index     = ( DIAIndexType* )MemHandleLock( indexHandle );
    numBins   = 0;
    formCount = index->count * sizeof( UInt16 ) /
                    sizeof( DIAIndexEntryType );

    if ( formCount == 0 ) {
        numBins = 0;
        return;
    }

    formList  = ( DIAFormEntryType* ) SafeMemPtrNew( formCount * 
                                          sizeof( DIAFormEntryType ) );
    MemSet( formList, formCount * sizeof( DIAFormEntryType ), 0 );

    for ( i = 0 ; i < formCount ; i++ ) {
         AddForm( i, DmGetResource( WORD_LIST_TYPE, index->mapList[ i ].to ),
             index->mapList[ i ].from );
    }

    MemHandleUnlock( indexHandle );
    DmReleaseResource( indexHandle );

    if ( 0 < numBins ) {
        bins = ( DIAStateType* ) SafeMemPtrNew( numBins * 
                                     sizeof( DIAStateType ) );
        for ( i = 0 ; i < numBins ; i++ )
             bins[ i ] = DIA_STATE_UNDEFINED;
    }
}




void TerminateResizeSupport( void )
{
     UInt16  i;

     if ( formList == NULL )
         return;
     TerminateDIA();
     for ( i = 0 ; i < formCount ; i++ ) {
          if ( NULL != formList[ i ].configHandle ) {
              MemHandleUnlock( formList[ i ].configHandle );
              DmReleaseResource( formList[ i ].configHandle );
              if ( formList[ i ].originalBounds != NULL )
                  SafeMemPtrFree( formList[ i ].originalBounds );
          }
     }
     SafeMemPtrFree( formList );
     formCount = 0;
     if ( 0 < numBins ) {
         SafeMemPtrFree( bins );
     }
}




void LoadResizePrefs( UInt32 appID, UInt16 prefID )
{
    UInt16 size;

    size = 0;

    if ( formList == NULL || numBins == 0 ||
         noPreferenceFound ==
             PrefGetAppPreferences( appID, prefID, NULL, &size, true ) ) {
        return;
    }

    if ( numBins * sizeof( DIAStateType ) < size ) {
        size = numBins * sizeof( DIAStateType );
    }

    PrefGetAppPreferences( appID, prefID, bins, &size, true );
}



void SaveResizePrefs( UInt32 appID, UInt16 prefID, Int16 version )
{
    if ( formList == NULL || numBins == 0 )
        return;

    PrefSetAppPreferences( appID, prefID, version, bins,
        numBins * sizeof( DIAStateType ), true );
}



RESIZE_SECTION static UInt16 GetFormIndex( UInt16 formID ) 
{
    UInt16 i;

    if ( formList == NULL )
        return 0;

    for ( i = 0 ; i < formCount ; i++ )
        if ( formList[ i ].formID == formID )
            break;

    return i;
}



void SetResizePolicy( UInt16 formID )
{
    if ( GetFormIndex( formID ) < formCount )
        SetCustomDIAPolicy( formID );
}



RESIZE_SECTION static Boolean GetFormInfo( void ) 
{
    active.formID  = FrmGetActiveFormID();
    active.formPtr = FrmGetActiveForm();
    active.index   = GetFormIndex( active.formID );

    if ( active.index < formCount ) {
        active.entry = &( formList[ active.index ] );
        active.flags = active.entry->config->flags;
        return true;
    }
    else {
        return false;
    }
}





RESIZE_SECTION static Boolean PrepareForm( void ) 
{
    if ( active.index == formCount ) {
        SetDIAState( DIA_STATE_MAX );
        SetDIAAllowResize( false );
        return false;
    }

    return MoveObjectsInForm( active.index );
}



RESIZE_SECTION static void SaveDIAState( void ) 
{
    if ( active.index == formCount )
        return;

    if ( active.entry->config->flags & DIA_FORM_USE_BIN ) {
        bins[ active.entry->config->bin ] = GetDIAState();
    }
}




RESIZE_SECTION static Boolean FormStartup( void )
{
    DIAStateType newState;
    Boolean   big;
    Boolean   allowBig;
    Boolean   changed;
    Boolean   allowResize;

    if ( active.index == formCount ) {
        SetDIAState( DIA_STATE_MAX );
        SetDIAAllowResize( false );
        SetDIAConstraints( FrmGetWindowHandle( active.formPtr ), false, false );
        return false;
    }

    newState = GetDIAState();

    if ( ( active.flags & DIA_FORM_USE_BIN ) &&
         ( bins[ active.entry->config->bin ] != DIA_STATE_UNDEFINED ) ) {
        newState = bins[ active.entry->config->bin ];
/*        if ( newState == DIA_STATE_MIN ) DebugInfo("bin = min");
        if ( newState == DIA_STATE_MAX ) DebugInfo("bin = max"); */
    }
    else if ( ! ( active.flags & DIA_FORM_KEEP_LAST ) ) {
        newState = active.entry->config->preferredState;
    }

    if ( active.flags & DIA_FORM_NO_RESIZE ) {
        allowResize = false;
        allowBig = false;
    }
    else {
        allowResize = true;
        allowBig = true;
    }

    big      = ( newState == DIA_STATE_MIN || newState == DIA_STATE_NO_STATUS_BAR );
    allowBig = allowBig || big;

    SetDIAConstraints( FrmGetWindowHandle( active.formPtr ), allowBig, allowBig );

    SetDIAState( newState );

    SetDIAAllowResize( allowResize );

    changed = PrepareForm();

/*    if ( changed ) DebugInfo("Changed in FormStartup"); */

    SaveDIAState();

    return changed;
}


RESIZE_SECTION static void ForceRedrawAll( void )
{
    UInt16 i;
    for ( i = 0 ; i < formCount ; i++ ) {
        if ( formList[ i ].open )
            formList[ i ].forceRedraw = true;
    }
}



Boolean ResizeHandleFrmOpenEvent( void )
{
    GetFormInfo();
#ifdef SUPPORT_DIA_HANDERA
    if ( GetDIAHardware() == DIA_HARDWARE_HANDERA &&
         ( active.index == formCount || ! ( active.flags & DIA_FORM_HANDERA ) )
       ) {
        VgaFormModify( active.formPtr, vgaFormModify160To240 );
    }
#endif
    if ( active.index < formCount ) {
        MemSet( &( active.entry->lastBounds ), sizeof( RectangleType ), 0 );
        active.entry->lastExtentX = -1;
        active.entry->lastExtentY = -1;
        active.entry->forceRedraw = false;
    }
    DebugInfo("FrmOpen");
    PushCoordinateSystemToStandard();
    FormStartup();
    if ( active.index < formCount ) {
        active.entry->open = true;
    }
    PopCoordinateSystem();
    haveWinDisplayChangedEvent = false;
    return true;
}



Boolean ResizeHandleFrmCloseEvent( void )
{
    if ( GetFormInfo() ) {
        DebugInfo("FrmClose");
        active.entry->open        = false;
        active.entry->forceRedraw = false;
        SaveDIAState();
    }
    return true;
}



Boolean ResizeHandleWinDisplayChangedEvent( void )
{
    haveWinDisplayChangedEvent = false;
    if ( ! GetFormInfo() )
        return true;

    PushCoordinateSystemToStandard();

    DebugInfo("WinDisplayChanged");
    if ( PrepareForm() || active.entry->forceRedraw ) {
        UniqueUpdateForm( active.formID, frmRedrawUpdateCode );
        active.entry->forceRedraw = false;
    }
    SaveDIAState();

    PopCoordinateSystem();

    return true;
}



Boolean ResizeHandleWinExitEvent( void )
{
    DebugInfo("WinExit");

    return true;
}



Boolean ResizeHandleFrmRedrawUpdateCode( void )
{
    GetFormInfo();
    DebugInfo("FrmUpdate");
    if ( active.index < formCount )
        active.entry->forceRedraw = false;
    return true;
}



Boolean ResizeHandleWinEnterEvent( void )
{
    GetFormInfo();
    DebugInfo("WinEnter");
    if ( active.index == formCount ) {
        return true;
    }

    PushCoordinateSystemToStandard();

    /* If the last form we have record of being in is this one, then
       we may have missed what happened on a form. */

    if ( active.entry->open && ( FormStartup() ||
         active.entry->forceRedraw ) )
       {
        EventType e;
        DebugInfo("WinEnter: Possibly missed winDisplayChangedEvent");
        MemSet( &e, sizeof( EventType ), 0 );
        e.eType = winDisplayChangedEvent;
        EvtAddUniqueEventToQueue( &e, 0, true );
        DebugInfo("Queuing display change");
        active.entry->forceRedraw = true;
    }

    PopCoordinateSystem();

    return true;
}




void SetHaveWinDisplayChangedEvent( Boolean value )
{
    DebugInfo("SetHaveWinDisplayChangedEvent()");
    haveWinDisplayChangedEvent = value;
    ForceRedrawAll();
}




void ResizeRefreshCurrentForm( void )
{
    GetFormInfo();
    if ( active.index == formCount )
        return;

    PushCoordinateSystemToStandard();

    FormStartup();
    SaveDIAState();
    UniqueUpdateForm( active.formID, frmRedrawUpdateCode );

    PopCoordinateSystem();
}




Boolean ResizeHandleEvent( EventType* event )
{
    Boolean handled = false;
    ResizeTemporaryData old;

    switch ( event->eType ) {
        case nilEvent:
            return false;
        case frmOpenEvent: {
            old = active;
            ResizeHandleFrmOpenEvent();
            active = old;
            break;
        }
        case frmCloseEvent: {
            old = active;
            ResizeHandleFrmCloseEvent();
            active = old;
            break;
        }
        case winEnterEvent: {
            old = active;
            ResizeHandleWinEnterEvent();
            active = old;
            break;
        }
        case winExitEvent: {
            old = active;
            ResizeHandleWinExitEvent();
            active = old;
            break;
        }
        case winDisplayChangedEvent: {
            old = active;
            ResizeHandleWinDisplayChangedEvent();
            active = old;
            break;
        }
        case frmUpdateEvent:
            if ( event->data.frmUpdate.updateCode == frmRedrawUpdateCode ) {
                old = active;
                ResizeHandleFrmRedrawUpdateCode();
                active = old;
            }
            break;
        default:
            break;
    }

    return handled;
}

