/*
 * GIFLib.h
 *
 * Universe GIF Decompressor.
 *
 * Copyright (c) 2007 OpenMobl Systems. All Rights Reserved.
 *
 * Portions Copyright (c) 1994-1999 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */

#ifndef GIFLIB_H_
#define GIFLIB_H_

/* Palm OS common definitions */
#include <SystemMgr.h>

/* If we're actually compiling the library code, then we need to
 * eliminate the trap glue that would otherwise be generated from
 * this header file in order to prevent compiler errors in CW Pro 2. */
#ifdef BUILDING_GIFLIB
	#define GIFLIB_LIB_TRAP(trapNum)
#else
	#define GIFLIB_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif

/*********************************************************************
 * Type and creator of Sample Library database
 *********************************************************************/

#define		GIFLibCreatorID	'GIFD'
#define		GIFLibTypeID		sysFileTLibrary

/*********************************************************************
 * Internal library name which can be passed to SysLibFind()
 *********************************************************************/

#define		GIFLibName		"GIFLib"

/*********************************************************************
 * GIFLib result codes
 * (appErrorClass is reserved for 3rd party apps/libraries.
 * It is defined in SystemMgr.h)
 *********************************************************************/

/* invalid parameter */
#define GIFLibErrParam		(appErrorClass | 1)		

/* library is not open */
#define GIFLibErrNotOpen		(appErrorClass | 2)		

/* returned from GIFLibClose() if the library is still open */
#define GIFLibErrStillOpen	(appErrorClass | 3)		



typedef struct GIFImageDescType {
	struct GIFImageDescType	*next;
	BitmapPtr		bitmap;
	Int32			transparent;
	Int16			x;
	Int16			y;
	Int16			width;
	Int16			height;
} GIFImageDescType;

typedef struct GIFImageType {
	GIFImageDescType	*images;
	Int32			background;
	Int16			width;
	Int16			height;
} GIFImageType;

/*********************************************************************
 * API Prototypes
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Standard library open, close, sleep and wake functions */

extern Err GIFLibOpen(UInt16 refNum, UInt32 * clientContextP)
	GIFLIB_LIB_TRAP(sysLibTrapOpen);
				
extern Err GIFLibClose(UInt16 refNum, UInt32 clientContext)
	GIFLIB_LIB_TRAP(sysLibTrapClose);

extern Err GIFLibSleep(UInt16 refNum)
	GIFLIB_LIB_TRAP(sysLibTrapSleep);

extern Err GIFLibWake(UInt16 refNum)
	GIFLIB_LIB_TRAP(sysLibTrapWake);

/* Custom library API functions */

extern Err GIFLibGetVersion(UInt16 refNum, UInt32 *versionP)
	GIFLIB_LIB_TRAP(sysLibTrapBase + 5);

extern Err GIFLibDecompressImage(UInt16 refNum, Char *data, Int32 length,
						  Int16 *width, Int16 *height, BitmapPtr *image)
	GIFLIB_LIB_TRAP(sysLibTrapBase + 6);

#ifdef __cplusplus
}
#endif


#endif /* GIFLIB_H_ */
