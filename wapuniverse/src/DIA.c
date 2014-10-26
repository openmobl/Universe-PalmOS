/* $Id: DIA.c,v 1.12 2004/12/15 20:02:47 arpruss Exp $ */
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


#include "resize.h"

#include "DIA.h"

#ifdef SUPPORT_DIA_HANDERA
#include <Vga.h>
#include <Silk.h>
#endif

#ifdef SUPPORT_DIA_SONY
#include <SonyCLIE.h>

#ifndef vskResizeVertically
#  define vskResizeVertically     1
#endif
#ifndef vskResizeHorizontally
#  define vskResizeHorizontally   2
#endif
#endif

#define COORDSYS_STACK_LEN 10


static DIAHardwareType  hardware = DIA_HARDWARE_NONE;
static UInt32           displayChangeNotification = 0;
static Boolean          haveNotification;
static UInt16           card;
static LocalID          db;
static Boolean          palmHiRes;
static Boolean          sony1AllowResize = false;
static UInt16           coordSysStack[ 10 ];
static UInt16           coordSysStackPtr = 0;
#ifdef SUPPORT_DIA_SONY
static UInt16           sonyRefNum;
#endif
#ifdef SUPPORT_DIA_HANDERA
static VgaRotateModeType savedHanderaRotation;
static VgaScreenModeType savedHanderaScreenMode;
#endif



#ifdef SUPPORT_DIA_HANDERA
static DIAHardwareType InitializeHandera( void )
{
    UInt32 version;
    if ( _TRGSilkFeaturePresent( &version ) ) {
        VgaRotateModeType rotation;
        VgaScreenModeType screenMode;

        VgaGetScreenMode( &screenMode, &rotation );

        savedHanderaScreenMode = screenMode;
        savedHanderaRotation   = rotation;

        if ( screenMode != screenMode1To1 )
            VgaSetScreenMode( screenMode1To1, rotation );

        return DIA_HARDWARE_HANDERA;
    }
    else {
        return DIA_HARDWARE_NONE;
    }
}
#else
# define InitializeHandera() DIA_HARDWARE_NONE
#endif



#ifdef SUPPORT_DIA_SONY
static DIAHardwareType InitializeSony( void )
{
    Err    err;
    UInt32 version;

    err = SysLibFind( sonySysLibNameSilk, &sonyRefNum );
    if ( err == sysErrLibNotFound ) {
        err = SysLibLoad( 'libr', sonySysFileCSilkLib, &sonyRefNum );
    }
    if ( err != errNone )
        return DIA_HARDWARE_NONE;
    if ( errNone == FtrGet( sonySysFtrCreator, sonySysFtrNumVskVersion, 
                        &version ) ) {
        /* Version 2 and up */
        err = VskOpen( sonyRefNum );
        if ( errNone == err )
            return DIA_HARDWARE_SONY2;
    }
    else {
        /* Version 1 and up */
        err = SilkLibOpen( sonyRefNum );
        if ( errNone == err ) {
            /* Make sure we are in a Hi-Res mode */
            UInt32 width;
            UInt32 height;
            UInt16 sonyHiResRefNum;

            err = SysLibFind( sonySysLibNameHR, &sonyHiResRefNum );
            if ( err == sysErrLibNotFound )
                err = SysLibLoad( 'libr', sonySysFileCHRLib, &sonyHiResRefNum );

            if ( err == errNone ) {

                err = HROpen( sonyHiResRefNum );
                HRWinScreenMode( sonyHiResRefNum, winScreenModeGet, &width,
                     &height, NULL, NULL );

                if ( width < hrWidth ) {

                    width  = hrWidth;
                    height = hrHeight;
                    if ( errNone != HRWinScreenMode( sonyHiResRefNum,
                                        winScreenModeSet,
                                        &width, &height, NULL, NULL ) ) {
                        HRClose( sonyHiResRefNum );
                        return DIA_HARDWARE_NONE;
                    }
                }
                HRClose( sonyHiResRefNum );
            }
            else
                return DIA_HARDWARE_NONE;
        }

        SilkLibDisableResize( sonyRefNum );

        sony1AllowResize = false;

        return DIA_HARDWARE_SONY1;
    }
    return DIA_HARDWARE_NONE;
}
#else
# define InitializeSony() DIA_HARDWARE_NONE
#endif



#ifdef HAVE_PALM_DIA_SDK
static DIAHardwareType InitializePalm( void )
{
    UInt32 version;
    Err    err;
    err = FtrGet( pinCreator, pinFtrAPIVersion, &version );
    if ( err != errNone )
        return DIA_HARDWARE_NONE;
    if ( pinAPIVersion1_1 <= version )
        return DIA_HARDWARE_PALM11;
    else if ( pinAPIVersion1_0 <= version )
        return DIA_HARDWARE_PALM10;
    else
        return DIA_HARDWARE_NONE;
}
#else
# define InitializePalm() DIA_HARDWARE_NONE
#endif



/* Check if this is an unknown device with non-standard screen size
   so we can make proper use of the screen size. */
static DIAHardwareType InitializeUnknown( void )
{
    Coord extentX;
    Coord extentY;
    PushCoordinateSystemToStandard();
    WinGetDisplayExtent( &extentX, &extentY );
    PopCoordinateSystem();
    if ( extentX != STD_EXTENT_X || extentY != STD_EXTENT_Y )
        return DIA_HARDWARE_UNKNOWN;
    else
        return DIA_HARDWARE_NONE;
}



static void RegisterNotification( void )
{
    Err      err;

    haveNotification = false;
    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY1:
        case DIA_HARDWARE_SONY2:
            displayChangeNotification = sysNotifyDisplayChangeEvent;
            break;
#endif
#ifdef SUPPORT_DIA_HANDERA
        case DIA_HARDWARE_HANDERA:
            displayChangeNotification = trgNotifySilkEvent;
            break;
#endif
#ifdef HAVE_PALM_DIA_SDK
        case DIA_HARDWARE_PALM10:
        case DIA_HARDWARE_PALM11:
            displayChangeNotification = sysNotifyDisplayResizedEvent;
            break;
#endif
        default:
            return;
    }
    err = SysCurAppDatabase( &card, &db );
    if ( err != errNone )
        return;
    err = SysNotifyRegister( card, db, displayChangeNotification, NULL,
        sysNotifyNormalPriority, 0 );
    haveNotification = ( err == errNone );
}



static void UnregisterNotification( void )
{
    if ( haveNotification ) {
        SysNotifyUnregister( card, db, displayChangeNotification,
            sysNotifyNormalPriority );
        haveNotification = false;
    }
}



DIAHardwareType InitializeDIA( void )
{
    Err    err;
    UInt32 version;

    err       = FtrGet( sysFtrCreator, sysFtrNumWinVersion, &version );

    palmHiRes = ( err == errNone && 4 <= version );

    hardware = InitializeHandera();
    if ( hardware == DIA_HARDWARE_NONE )
        hardware = InitializePalm();
    if ( hardware == DIA_HARDWARE_NONE )
        hardware = InitializeSony();
    if ( hardware == DIA_HARDWARE_NONE )
        hardware = InitializeUnknown();
    RegisterNotification();

    return hardware;
}



void TerminateDIA( void )
{
    UnregisterNotification();
    if ( GetDIAState() == DIA_STATE_NO_STATUS_BAR )
        SetDIAState( DIA_STATE_MIN );
    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY1:
            SilkLibClose( sonyRefNum );
            break;
        case DIA_HARDWARE_SONY2:
            VskClose( sonyRefNum );
            break;
#endif
#ifdef SUPPORT_DIA_HANDERA
        case DIA_HARDWARE_HANDERA: {
            VgaRotateModeType rotation;
            VgaScreenModeType screenMode;

            VgaGetScreenMode( &screenMode, &rotation );

            if ( screenMode != savedHanderaScreenMode ||
                 savedHanderaRotation != rotation ) {
                VgaSetScreenMode( savedHanderaScreenMode,
                    savedHanderaRotation );
            }
        }
#endif
        default:
            break;
    }
    hardware = DIA_HARDWARE_NONE;
}




void SetDIAState( DIAStateType state )
{
    if ( DIA_HARDWARE_HANDERA == hardware && GetDIAState() == state )
        return;

    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY1: {
            Boolean oldAllow;

            oldAllow = sony1AllowResize;

            if ( ! oldAllow )
                SetDIAAllowResize( true );

            switch ( state ) {
                case DIA_STATE_MAX:
                    SilkLibResizeDispWin( sonyRefNum, silkResizeNormal );
                    break;
                case DIA_STATE_MIN:
                    SilkLibResizeDispWin( sonyRefNum, silkResizeToStatus );
                    break;
                case DIA_STATE_NO_STATUS_BAR:
                    SilkLibResizeDispWin( sonyRefNum, silkResizeMax );
                    break;
                default:
                    break;
            }
            break;

            if ( ! oldAllow )
                SetDIAAllowResize( false );
        }

        case DIA_HARDWARE_SONY2: {
            Boolean oldAllow;
            UInt16  allowState;

            if ( errNone == VskGetState( sonyRefNum, vskStateEnable,
                               &allowState ) )
                oldAllow = ( allowState != 0 );
            else
                oldAllow = false;

            if ( ! oldAllow ) {
                SetDIAAllowResize( true );
            }

            switch ( state ) {
                case DIA_STATE_MAX:
                    VskSetState( sonyRefNum, vskStateResize, vskResizeMax );
                    break;
                case DIA_STATE_MIN:
                    VskSetState( sonyRefNum, vskStateResize, vskResizeMin );
                    break;
                case DIA_STATE_NO_STATUS_BAR:
                    VskSetState( sonyRefNum, vskStateResize, vskResizeNone );
                    break;
                default:
                    break;
            }
            break;

            if ( ! oldAllow )
                SetDIAAllowResize( false );
        }
#endif
#ifdef SUPPORT_DIA_HANDERA
        case DIA_HARDWARE_HANDERA:
            switch ( state ) {
                case DIA_STATE_MAX:
                    SilkMaximizeWindow();
                    break;
                case DIA_STATE_MIN:
                case DIA_STATE_NO_STATUS_BAR:
                    SilkMinimizeWindow();
                    break;
                default:
                    break;
            }
            break;
#endif
#ifdef HAVE_PALM_DIA_SDK
        case DIA_HARDWARE_PALM10:
        case DIA_HARDWARE_PALM11:
            switch ( state ) {
                case DIA_STATE_MAX:
                    PINSetInputAreaState( pinInputAreaOpen );
                    break;
                case DIA_STATE_MIN:
                    PINSetInputAreaState( pinInputAreaClosed );
                    StatShow();
                    break;
                case DIA_STATE_NO_STATUS_BAR:
                    PINSetInputAreaState( pinInputAreaClosed );
                    StatHide();
                    break;
                default:
                    break;
            }
            break;
#endif
        default:
            break;
    }
}



DIAStateType GetDIAState( void )
{
    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY1: {
            Coord extentY;
            PushCoordinateSystemToStandard();
            WinGetDisplayExtent( NULL, &extentY );
            PopCoordinateSystem();
            if ( 240 <= extentY )
                return DIA_STATE_NO_STATUS_BAR;
            else if ( 225 <= extentY )
                return DIA_STATE_MIN;
            else
                return DIA_STATE_MAX;
            break;
        }
        case DIA_HARDWARE_SONY2: {
            UInt16 state;
            Err    err;
            err = VskGetState( sonyRefNum, vskStateResize, &state );
            if ( err != errNone )
                return DIA_STATE_UNDEFINED;
            switch ( state ) {
                case vskResizeMax:
                    return DIA_STATE_MAX;
                case vskResizeMin:
                    return DIA_STATE_MIN;
                case vskResizeNone:
                    return DIA_STATE_NO_STATUS_BAR;
                default:
                    return DIA_STATE_UNDEFINED;
            }
        }
#endif
#ifdef SUPPORT_DIA_HANDERA
        case DIA_HARDWARE_HANDERA:
            if ( SilkWindowMaximized() )
                return DIA_STATE_MAX;
            else
                return DIA_STATE_MIN;
#endif
#ifdef HAVE_PALM_DIA_SDK
        case DIA_HARDWARE_PALM10:
        case DIA_HARDWARE_PALM11:
            switch ( PINGetInputAreaState() ) {
                case pinInputAreaOpen:
                    return DIA_STATE_MAX;
                case pinInputAreaClosed:
                case pinInputAreaNone: {
                    Err    err;
                    UInt32 value;

                    err = StatGetAttribute( statAttrBarVisible, &value );

                    if ( err != errNone || value != 0 )
                        return DIA_STATE_MIN;
                    else
                        return DIA_STATE_NO_STATUS_BAR;
                }
                default:
                    return DIA_STATE_UNDEFINED;
            }
#endif
        default:
            return DIA_STATE_MAX;
    }
}



void SetDIAAllowResize( Boolean allow )
{
    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY2:
            if ( allow ) {
                /* If available, enable horizontal resize */
                if ( 0x03 <= VskGetAPIVersion( sonyRefNum ) )
                    VskSetState( sonyRefNum, vskStateEnable,
                        vskResizeHorizontally );
                /* Enable vertical resize */
                VskSetState( sonyRefNum, vskStateEnable,
                    vskResizeVertically );
            }
            else {
                VskSetState( sonyRefNum, vskStateEnable, 0 );
            }
            break;
        case DIA_HARDWARE_SONY1:
            if ( allow )
                SilkLibEnableResize( sonyRefNum );
            else
                SilkLibDisableResize( sonyRefNum );

            sony1AllowResize = allow;

            break;
#endif
#ifdef HAVE_PALM_DIA_SDK
        case DIA_HARDWARE_PALM11:
        case DIA_HARDWARE_PALM10:
            PINSetInputTriggerState( allow ? pinInputTriggerEnabled :
                                                 pinInputTriggerDisabled );
            SysSetOrientationTriggerState( allow ?
                sysOrientationTriggerEnabled : sysOrientationTriggerDisabled );
            break;
#endif
        /* Note: On Handera, resizing is always enabled */
        default:
            break;
    }
}




static Boolean MatchLastExtents( void )
{
    static Coord lastX;
    static Coord lastY;
    Coord        extentX;
    Coord        extentY;

    extentX = lastX;
    extentY = lastY;

    PushCoordinateSystemToStandard();

    WinGetDisplayExtent( &lastX, &lastY );

    PopCoordinateSystem();

    return extentX == lastX && extentY == lastY;
}




Boolean HandleResizeNotification( UInt32 notificationType )
{
    switch ( notificationType ) {
#ifdef SUPPORT_DIA_SONY
        case sysNotifyDisplayChangeEvent:
#endif
#ifdef HAVE_PALM_DIA_SDK
        case sysNotifyDisplayResizedEvent:
#endif
#ifdef SUPPORT_DIA_HANDERA
        case trgNotifySilkEvent:
#endif
        {
            EventType e;

            if ( ! MatchLastExtents() ) {
                SetHaveWinDisplayChangedEvent( true );
                if ( hardware != DIA_HARDWARE_PALM11 ) {
                    MemSet( &e, sizeof(EventType), 0 );
                    e.eType = winDisplayChangedEvent;
                    EvtAddUniqueEventToQueue( &e, 0, true );
                }
            }
            return true;
        }    
        default:
            return false;
    }
}



void SetCustomDIAPolicy( UInt16 formID )
{
#ifdef HAVE_PALM_DIA_SDK
	if ( hardware == DIA_HARDWARE_PALM10 || hardware == DIA_HARDWARE_PALM11 ) {
	    FormType* formPtr;
        formPtr = FrmGetFormPtr( formID );
        if ( formPtr != NULL ) {
            FrmSetDIAPolicyAttr( formPtr, frmDIAPolicyCustom );
        }
    }
#endif
}



DIAHardwareType GetDIAHardware( void )
{
    return hardware;
}



void SetDIAConstraints( WinHandle winH, Boolean big, Boolean allowBig )
{
#ifdef HAVE_PALM_DIA_SDK
	if ( hardware == DIA_HARDWARE_PALM10 || hardware == DIA_HARDWARE_PALM11 ) {
	    PushCoordinateSystemToStandard();

        WinSetConstraintsSize( winH,
            STD_EXTENT_Y,
            big ? MAX_EXTENT_Y : STD_EXTENT_Y,
            allowBig ? MAX_EXTENT_Y : STD_EXTENT_Y,
            STD_EXTENT_X,
            big ? MAX_EXTENT_X : STD_EXTENT_X,
            allowBig ? MAX_EXTENT_X : STD_EXTENT_X );

        PopCoordinateSystem();
    }
#endif
}



/* Check which DIA state covers more screen space */
Int16 CompareDIAState( DIAStateType x, DIAStateType y )
{
    if ( y == DIA_STATE_UNDEFINED )
        y = DIA_STATE_MAX;
    if ( x == DIA_STATE_UNDEFINED )
        x = DIA_STATE_MAX;
    if ( x == y )
        return 0;
    switch ( x ) {
        case DIA_STATE_MIN:
            return y == DIA_STATE_NO_STATUS_BAR ? 1 : -1;
        case DIA_STATE_NO_STATUS_BAR:
            return -1;
        case DIA_STATE_MAX:
        default:
            return 1;
    }
}



static UInt16 SafeWinSetCoordinateSystem( UInt16 coordSys )
{
    if ( ! palmHiRes || NULL == WinGetDrawWindow() )
        return kCoordinatesStandard;
    else
        return WinSetCoordinateSystem( coordSys );
}



void PushCoordinateSystemToStandard( void )
{
    coordSysStack[ coordSysStackPtr++ ] =
        SafeWinSetCoordinateSystem( kCoordinatesStandard );
}




void PopCoordinateSystem( void )
{
    SafeWinSetCoordinateSystem( coordSysStack[ --coordSysStackPtr ] );
}



void GetHiddenStatusBarArea( RectangleType* area )
{
    MemSet( area, sizeof( RectangleType ), 0 );

    if ( DIA_STATE_NO_STATUS_BAR != GetDIAState() ) {
        return;
    }

    switch ( hardware ) {
        case DIA_HARDWARE_SONY2:
        case DIA_HARDWARE_SONY1:
        case DIA_HARDWARE_PALM11:
        case DIA_HARDWARE_PALM10: {
            Coord extentX;
            Coord extentY;

            WinGetDisplayExtent( &extentX, &extentY );
            switch ( extentX ) {
                case 160:  /* 160 x 240 */
                    area->topLeft.x = 0;
                    area->topLeft.y = 225;
                    area->extent.x  = 160;
                    area->extent.y  = 240 - 225;
                    break;
                case 240: /* 240 x 160 */
                    area->topLeft.x = 225;
                    area->topLeft.y = 0;
                    area->extent.x  = 240 - 225;
                    area->extent.y  = 160;
                    break;
                case 320: /* 320 x 480 */
                    area->topLeft.x = 0;
                    area->topLeft.y = 450;
                    area->extent.x  = 320;
                    area->extent.y  = 480 - 450;
                    break;
                case 480: /* 480 x 320 */
                    area->topLeft.x = 450;
                    area->topLeft.y = 0;
                    area->extent.x  = 480 - 450;
                    area->extent.y  = 320;
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}



FontID HanderaAdjustFont
       (
       FontID font
       )
{
#ifdef SUPPORT_DIA_HANDERA
    if ( hardware == DIA_HARDWARE_HANDERA &&
         0 == ( font & fntAppFontCustomBase ) )
        return VgaBaseToVgaFont( font );
    else
#endif
        return font;
}



Coord HanderaCoord( Coord x )
{
#ifdef SUPPORT_DIA_HANDERA
    return hardware == DIA_HARDWARE_HANDERA ? x * 3 / 2 : x;
#else
    return x;
#endif
}
