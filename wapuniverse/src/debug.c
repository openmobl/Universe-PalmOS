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

#include 	<PalmOS.h>
#include	<VFSMgr.h>
#include	"debug.h"
#include 	"BrowserAPI.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"build.h"
#include	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"
#include	"Layout.h"
#include	"Form.h"
#include	"variable.h"
#include	"Browser.h"
#include	"libhtmlparse.h"
#include	"http.h"
#include	"frmScroll.h"

void xhtml_debugStart(GlobalsType *g)
{
#ifdef DEBUG_HTML
	Err		err = errNone;
	UInt16 		volRefFirst = 0;
	UInt16 		volRefNum = 0; 
	UInt32 		volIterator = vfsIteratorStart;
	FileRef 	fileRefP;
	Char		filepath[255];
	UInt32 		vfsMgrVersion;
	Boolean		exists = false;

	if (!g_prefs.debug)
		return;

	g->xhtmlOutputFileRef = NULL;

	err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &vfsMgrVersion);
	if (err)
		return;

	while (volIterator != vfsIteratorStop) { 
		err = VFSVolumeEnumerate(&volRefNum, &volIterator); 
		if (!err) { 
			// Do something with the volRefNum
			volRefFirst = volRefNum;
		} else { 
			return;
		} 
	}
	volRefNum = volRefFirst;

	err = VFSDirCreate(volRefNum, "/PALM/Programs/Universe_3.0/");
	if (err != errNone && err != vfsErrFileAlreadyExists) {
		return;
	}

	StrCopy(filepath, "/PALM/Programs/Universe_3.0/");
	StrCopy(filepath+StrLen(filepath), "xhtml_debug_output.txt");

//create_debug_top:
	err = VFSFileCreate(volRefNum, filepath);
	if (err == vfsErrFileAlreadyExists) {
		/*err = VFSFileDelete(volRefNum, filepath);
		if (err != errNone) {
			return;
		}
		goto create_debug_top;*/
		exists = true;
	} else if (err != errNone) {
		return;
	}

	err = VFSFileOpen(volRefNum, filepath, vfsModeReadWrite, &fileRefP);
	if (err != errNone) {
		return;
	}

	g->xhtmlOutputFileRef = fileRefP;

	if (exists) {
		VFSFileSeek(fileRefP, vfsOriginEnd, 0);
		xhtml_debugOut("-- --", NULL, 0, false, "----------new", g);
	}
#endif
}

void xhtml_debugOut(Char *tag, struct ArgvTable *attributes, int numAttr, Boolean end, Char *comment, GlobalsType *g)
{
#ifdef DEBUG_HTML
	Err		err = errNone;
	UInt32 		numBytesWrittenP = 0;
	UInt32 		bufferoffset = 0;
	Char		*tempData;
	UInt32		bufferLength = 0;
	int		i = 0;

	if (!g_prefs.debug)
		return;

	if (!g->xhtmlOutputFileRef)
		return;

	if (end) {
		bufferLength += StrLen("end/");
		bufferLength += StrLen(tag);
		bufferLength += StrLen(" ");
		if (comment) {
			bufferLength += StrLen("/comm: ");
			bufferLength += StrLen(comment);
			bufferLength += StrLen(" ");
		}
	} else {
		bufferLength += StrLen("Tag: ");
		bufferLength += StrLen(tag);
		bufferLength += StrLen(" Attr: ");
		for(i=0; i < numAttr; i++) {
			bufferLength += StrLen(attributes[i].arg);
			bufferLength += 2; // for ", "
		}
		if (comment) {
			bufferLength += StrLen("Comm: ");
			bufferLength += StrLen(comment);
			bufferLength += StrLen(" ");
		}
	}
	bufferLength += 2; // "\r\n"

	tempData = Malloc(bufferLength + 1);

	if (!tempData) {
		return;
	}

	i = 0;

	if (end) {
		StrCopy(tempData, "end/");
		StrCopy(tempData+StrLen(tempData), tag);
		StrCopy(tempData+StrLen(tempData), " ");
		if (comment) {
			StrCopy(tempData+StrLen(tempData), "/comm: ");
			StrCopy(tempData+StrLen(tempData), comment);
			StrCopy(tempData+StrLen(tempData), " ");
		}
	} else {
		StrCopy(tempData, "Tag: ");
		StrCopy(tempData+StrLen(tempData), tag);
		StrCopy(tempData+StrLen(tempData), " Attr: ");
		for(i=0; i < numAttr; i++) {
			StrCopy(tempData+StrLen(tempData), attributes[i].arg);
			StrCopy(tempData+StrLen(tempData), ", ");
		}
		if (comment) {
			StrCopy(tempData+StrLen(tempData), "Comm: ");
			StrCopy(tempData+StrLen(tempData), comment);
			StrCopy(tempData+StrLen(tempData), " ");
		}
	}
	StrCopy(tempData+StrLen(tempData), "\r\n");

	do {
         	err = VFSFileWrite(g->xhtmlOutputFileRef, bufferLength - bufferoffset, tempData + bufferoffset,
    				&numBytesWrittenP);
         	if (err != errNone) {
			goto xhtml_cleanup;
		}

         	bufferoffset += numBytesWrittenP;
     	} while (bufferoffset < bufferLength);

	bufferoffset = 0;

xhtml_cleanup:

	Free(tempData);

	return;
#endif
}

void xhtml_debugEnd(GlobalsType *g)
{
#ifdef DEBUG_HTML
	Err	err = errNone;

	if (!g_prefs.debug)
		return;

	if (!g->xhtmlOutputFileRef)
		return;

	err = VFSFileClose(g->xhtmlOutputFileRef);
#endif
}



void debugStart(GlobalsType *g)
{
#ifdef DEBUG_GENERAL
	Err		err = errNone;
	UInt16 		volRefFirst = 0;
	UInt16 		volRefNum = 0; 
	UInt32 		volIterator = vfsIteratorStart;
	FileRef 	fileRefP;
	Char		filepath[255];
	UInt32 		vfsMgrVersion;
	Boolean		exists = false;

	if (!g_prefs.debug)
		return;

	g->debugOutputFileRef = NULL;

	err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &vfsMgrVersion);
	if (err)
		return;

	while (volIterator != vfsIteratorStop) { 
		err = VFSVolumeEnumerate(&volRefNum, &volIterator); 
		if (!err) { 
			// Do something with the volRefNum
			volRefFirst = volRefNum;
		} else { 
			return;
		} 
	}
	volRefNum = volRefFirst;

	err = VFSDirCreate(volRefNum, "/PALM/Programs/Universe_3.0/");
	if (err != errNone && err != vfsErrFileAlreadyExists) {
		return;
	}

	StrCopy(filepath, "/PALM/Programs/Universe_3.0/");
	StrCopy(filepath+StrLen(filepath), "debug_output.txt");

//create__top:
	err = VFSFileCreate(volRefNum, filepath);
	if (err == vfsErrFileAlreadyExists) {
		/*err = VFSFileDelete(volRefNum, filepath);
		if (err != errNone) {
			return;
		}
		goto create__top;*/
		exists = true;
	} else if (err != errNone) {
		return;
	}

	err = VFSFileOpen(volRefNum, filepath, vfsModeReadWrite, &fileRefP);
	if (err != errNone) {
		return;
	}

	g->debugOutputFileRef = fileRefP;

	if (exists) {
		VFSFileSeek(fileRefP, vfsOriginEnd, 0);
		debugOut("-------------------", "start", 0, __FILE__, __LINE__, g);
	}
#endif
}

void debugOut(Char *str1, Char *str2, Int32 num1, Char *file, Int16 line, GlobalsType *g)
{
#ifdef DEBUG_GENERAL
	Err		err = errNone;
	UInt32 		numBytesWrittenP = 0;
	UInt32 		bufferoffset = 0;
	Char		*tempData;
	UInt32		bufferLength = 0;
	char		num[maxStrIToALen];
	char		lineNum[maxStrIToALen];

//{Char temp[50]; StrPrintF(temp, "%s:%d", file, line); WinDrawChars(temp, StrLen(temp), 0, 0);}

	if (!g_prefs.debug)
		return;

	if (!g->debugOutputFileRef)
		return;

	if (num1)
		StrIToA(num, num1);
	StrIToA(lineNum, line);

	bufferLength += StrLen(str1);
	if (str2)
		bufferLength += StrLen(str2);
	if (num1)
		bufferLength += StrLen(num);
	bufferLength += StrLen(file);
	bufferLength += StrLen(lineNum);
	bufferLength += 2; // "\r\n"
	bufferLength += 5; // " " x 4 & ":"

	tempData = Malloc(bufferLength + 1);

	if (!tempData) {
		return;
	}

	StrCopy(tempData, str1);
	if (str2) {
		StrCopy(tempData+StrLen(tempData), " ");
		StrCopy(tempData+StrLen(tempData), str2);
	}
	if (num1) {
		StrCopy(tempData+StrLen(tempData), " ");
		StrCopy(tempData+StrLen(tempData), num);
	}
	StrCopy(tempData+StrLen(tempData), " ");
	StrCopy(tempData+StrLen(tempData), file);
	StrCopy(tempData+StrLen(tempData), ": ");
	StrCopy(tempData+StrLen(tempData), lineNum);

	bufferLength = StrLen(tempData);

	StrCopy(tempData+StrLen(tempData), "\r\n");

	bufferLength += 2;

	do {
         	err = VFSFileWrite(g->debugOutputFileRef, bufferLength - bufferoffset, tempData + bufferoffset,
    				&numBytesWrittenP);
         	if (err != errNone) {
			goto debug_cleanup;
		}

         	bufferoffset += numBytesWrittenP;
     	} while (bufferoffset < bufferLength);

	bufferoffset = 0;

debug_cleanup:

	Free(tempData);

	return;
#endif
}

void debugEnd(GlobalsType *g)
{
#ifdef DEBUG_GENERAL
	Err	err = errNone;

	if (!g_prefs.debug)
		return;

	if (!g->debugOutputFileRef)
		return;

	err = VFSFileClose(g->debugOutputFileRef);
#endif
}

void DebugShowData(Char *title, Char *data)
{
	MemHandle	handle;

	ErrFatalDisplayIf(!title, "No title!!");

	if (!data) {
		MyErrorFunc("No Data", title);
		return;
	}

	handle = MemHandleNew(StrLen(data) + 1);
	if (handle<=0) {
		MyErrorFunc("Failed to allocate memory.", title);
		return;
	}

	MemSet(MemHandleLock(handle),StrLen(data) + 1,0);
	MemHandleUnlock(handle);

	StrCopy(MemHandleLock(handle), data);
	MemHandleUnlock(handle);

	ScrollFrm(title, handle);

	MemHandleFree(handle);
}

void DebugShowLoadInfo(GlobalsType *g)
{
	Char	*data;

	data = Malloc(StrLen(g->Url) + (50000) + 1);
	if (!data)
		return;

	StrPrintF(data, "Stats for %s:\n\nURL: %s\nSize: %ld kb\nLoad: %ld secs",
		  g->deckTitle, g->Url, g->pageStats.pageSize / 1024,
		  (g->pageStats.timePageLoadFinish - g->pageStats.timePageLoadStart) / SysTicksPerSecond());

	DebugShowData("Page Load Info", data);

	Free(data);
}

/*void DebugShowMemUsage(GlobalsType *g)
{
	Char	*data;
    UInt32  freeMem     = 0;
    UInt32  totalMem    = 0;
    UInt32  largest     = 0;
WinDrawChar('0',0,0);
SysTaskDelay(SysTicksPerSecond());
	data = Malloc(50000);
	if (!data)
		return;
WinDrawChar('1',0,0);
SysTaskDelay(SysTicksPerSecond());
    freeMem = GetFreeMemory(NULL, &totalMem, false);
WinDrawChar('2',0,0);
SysTaskDelay(SysTicksPerSecond());
    largest = GetFreeMemory(NULL, NULL, true);
WinDrawChar('3',0,0);
SysTaskDelay(SysTicksPerSecond());
	StrPrintF(data, "Memory usage:\n\nDevice:\n     Total: %ld\n Available: %ld\n   Largest: %ld\n\nBrowser:\n      Used: %ld",
		  totalMem, freeMem, largest, g->pageStats.totalMemoryUsed);
WinDrawChar('4',0,0);
SysTaskDelay(SysTicksPerSecond());
	DebugShowData("Memory Usage", data);
WinDrawChar('5',0,0);
SysTaskDelay(SysTicksPerSecond());
	Free(data);
WinDrawChar('6',0,0);
SysTaskDelay(SysTicksPerSecond());
}*/

void DebugShowMemUsage(GlobalsType *g)
{
	Char	*data;
    UInt32  freeMem     = 0;
    UInt32  totalMem    = 0;
    UInt32  largest     = 0;

	data = Malloc(50000);
	if (!data)
		return;

    freeMem = GetFreeMemory(NULL, &totalMem, false);

    largest = GetFreeMemory(NULL, NULL, true);

	StrPrintF(data, "Memory usage:\n\nDevice:\n     Total: %ld\n Available: %ld\n   Largest: %ld\n\nBrowser:\n      Used: %ld",
		  totalMem, freeMem, largest, g->pageStats.totalMemoryUsed);

	DebugShowData("Memory Usage", data);

	Free(data);
}

void DebugShowGlobals(GlobalsType *g)
{
	Char	*data;

	data = Malloc(50000);
	if (!data)
		return;

	StrPrintF(data, "Globals for: %s\n\n" \
                    " Tab # %d (of %d, max %d)\n" \
                    "  DrawWidth: %d\n"   \
                    " DrawHeight: %d\n"   \
                    "            ScrollX: %d\n"   \
                    "            ScrollY: %d\n"   \
                    "                         x: %d\n"   \
                    "                         y: %d\n",
                    g->Url, g->tabList.currentTab, g->tabList.tabCount,
                    MAX_TABS, g->DrawWidth, g->DrawHeight,
                    g->ScrollX, g->ScrollY, g->x, g->y);

	DebugShowData("Page Globals", data);

	Free(data);
}

