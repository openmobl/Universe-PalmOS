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


#include	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"build.h"
#include 	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"
#include	"Form.h"
#include	"Layout.h"
#include	"Browser.h"
#include	"Pointer.h"
#include	"image.h"
#include	"Hyperlinks.h"
#include	"FileURI.h"
#include	"debug.h"



void FileURIParse(Char *url, Char *card, Char *path)
{
	UInt32	len = 0;
	Char	*slash = NULL;

	if (StrNCaselessCompare(url, "file://", StrLen("file://")))
		return;

	len += StrLen("file://");

    if (url[len] == '/') {
        len++;
    }

	StrCopy(card, url + len);
	if ((slash = StrChr(card, '/'))) {
		*slash = '\0';
	}

	slash = StrChr(url + len, '/');
	len = (UInt32)(slash - url);

	StrCopy(path, url + len);
}

Boolean FileURIIsInternal(Char *card)
{
	if (!StrNCaselessCompare(card, FILE_URI_INTERNAL, StrLen(FILE_URI_INTERNAL))) {
		return true;
	}

	return false;
}

UInt16 FileURIGetVFSRefNum(Char *card)
{
	UInt16 		volRefNum = 0; 
	UInt32 		volIterator = vfsIteratorStart;
	Err		err;
	Char		volName[MAX_FILE_URI_LEN + 1];

	while (volIterator != vfsIteratorStop) { 
		err = VFSVolumeEnumerate(&volRefNum, &volIterator); 
		if (err == errNone) { 
			MemSet(volName, MAX_FILE_URI_LEN, 0);
			VFSVolumeGetLabel(volRefNum, volName, MAX_FILE_URI_LEN);

			if (!StrNCaselessCompare(volName, card, StrLen(card))) {
				return volRefNum;
			}
		}
	}

	return 0;
}

Boolean FileURIExists(Char *url)
{
	Char		card[MAX_FILE_URI_LEN + 1];
	Char		path[MAX_FILE_URI_LEN + 1];
	UInt32 		vfsMgrVersion;
	UInt16		volRefNum 	= 0;
	UInt32		attributes 	= 0;
	FileRef		fileRef;
	Boolean		exists 		= false;
	FileHand	fileHand 	= 0;
	Err		err 		= errNone;

	if (FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &vfsMgrVersion)) {
		return false;
	}

	FileURIParse(url, card, path);

	if (FileURIIsInternal(card)) {
		// Ok, so, if the file opens, we assume that it exists
		// Also, path with be something like /filename, so skip past the '/'
		fileHand = FileOpen(0, path + 1, 0, 0, fileModeReadOnly, &err);
		if (!fileHand && (err != errNone)) {
			return false;
		}

		FileClose(fileHand);

		// Assume that the file exists
		return true;
	} else {
		if ((volRefNum = FileURIGetVFSRefNum(card))) {
			exists = true;

			VFSFileOpen(volRefNum, path, FILE_URI_OPEN_MODE, &fileRef);
			VFSFileGetAttributes(fileRef, &attributes);
			if (attributes & vfsFileAttrDirectory) {
				exists = false;
			}
			VFSFileClose(fileRef);
		}
	}

	return exists;
}

Boolean FileURIGetExtension(Char *path, Char *ext)
{
	int	i = StrLen(path);

	while (i && (path[i] != '.')) {
		i--;
	}

	if (!i) {
		return false;
	}

	StrCopy(ext, path + i + 1);

	return true;
}

// return of 1 indicates error
UInt32 FileURIOpen(Char *url, WebDataPtr webData, Boolean remember, GlobalsType *g)
{
	Char		card[MAX_FILE_URI_LEN + 1];
	Char		path[MAX_FILE_URI_LEN + 1];
	Char		ext[FILE_URI_EXT_LEN];
	UInt32 		vfsMgrVersion;
	UInt16		volRefNum;
	FileRef 	fileRef;
	Err		err 		= errNone;
	UInt32		length 		= 0;
	UInt32		bytesRead 	= 0;
	UInt32		totalBytesRead 	= 0;
	FileHand	fileHand 	= 0;

	DebugOutFileURI("FileURIOpen", "top", 0, __FILE__, __LINE__, g);

	FileURIParse(url, card, path);

	if (FileURIIsInternal(card)) {
		DebugOutFileURI("FileURIOpen", "open file", 0, __FILE__, __LINE__, g);

		fileHand = FileOpen(0, path + 1, 0, 0, fileModeReadOnly, &err);
		if (!fileHand && (err != errNone)) {
			return 1;
		}

		DebugOutFileURI("FileURIOpen", "opened, free data", 0, __FILE__, __LINE__, g);

		WspFreePdu(webData);
		webData->data	= NULL;
		webData->length	= 0;

		DebugOutFileURI("FileURIOpen", "data freed, check size", 0, __FILE__, __LINE__, g);

		FileTell(fileHand, &length, &err);

		DebugOutFileURI("FileURIOpen", "size gotten", 0, __FILE__, __LINE__, g);

		webData->length = length;
		webData->data	= Malloc(webData->length);
		if (!webData->data) {
			DebugOutFileURI("FileURIOpen", "failed to allocate memory", 0, __FILE__, __LINE__, g);

			FileClose(fileHand);
			return 1;
		}

		DebugOutFileURI("FileURIOpen", "copy data", 0, __FILE__, __LINE__, g);

		do {
			bytesRead = FileRead(fileHand, webData->data + totalBytesRead,
						1, webData->length, &err);
			totalBytesRead += bytesRead;
		} while ((totalBytesRead < webData->length) && (err == errNone) && bytesRead);

		DebugOutFileURI("FileURIOpen", "data copied, close", 0, __FILE__, __LINE__, g);

		FileClose(fileHand);

		DebugOutFileURI("FileURIOpen", "closed", 0, __FILE__, __LINE__, g);
	} else {
		DebugOutFileURI("FileURIOpen", "check compatibility", 0, __FILE__, __LINE__, g);

		if (FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &vfsMgrVersion)) {
			DebugOutFileURI("FileURIOpen", "is not vfs??!!", 0, __FILE__, __LINE__, g);
			return 1;
		}

		DebugOutFileURI("FileURIOpen", "obtain volrefnum", 0, __FILE__, __LINE__, g);

		volRefNum = FileURIGetVFSRefNum(card);
		if (!volRefNum) {
			return 1;
		}

		DebugOutFileURI("FileURIOpen", "open file", 0, __FILE__, __LINE__, g);
		DebugOutFileURI("FileURIOpen", path, 0, __FILE__, __LINE__, g);

		err = VFSFileOpen(volRefNum, path, FILE_URI_OPEN_MODE, &fileRef);
		if (err) {
			return 1;
		}

		DebugOutFileURI("FileURIOpen", "opened, free data", 0, __FILE__, __LINE__, g);

		WspFreePdu(webData);
		webData->data	= NULL;
		webData->length	= 0;

		DebugOutFileURI("FileURIOpen", "data freed, check size", 0, __FILE__, __LINE__, g);

		VFSFileSize(fileRef, &length);

		DebugOutFileURI("FileURIOpen", "size gotten", 0, __FILE__, __LINE__, g);

		webData->length = length;
		webData->data	= Malloc(webData->length);
		if (!webData->data) {
			DebugOutFileURI("FileURIOpen", "failed to allocate memory", 0, __FILE__, __LINE__, g);

			VFSFileClose(fileRef);
			return 1;
		}

		DebugOutFileURI("FileURIOpen", "copy data", 0, __FILE__, __LINE__, g);

		do {
			err = VFSFileRead(fileRef, length - totalBytesRead,
					webData->data + totalBytesRead, &bytesRead);
			totalBytesRead += bytesRead;
		} while ((totalBytesRead < length) && (err == errNone) && bytesRead);

		DebugOutFileURI("FileURIOpen", "data copied, close", 0, __FILE__, __LINE__, g);

		VFSFileClose(fileRef);

		DebugOutFileURI("FileURIOpen", "closed", 0, __FILE__, __LINE__, g);

	}

	DebugOutFileURI("FileURIOpen", "get ext", 0, __FILE__, __LINE__, g);

	if (FileURIGetExtension(path, ext)) {
		DebugOutFileURI("FileURIOpen", ext, 0, __FILE__, __LINE__, g);

		if (g->contentType)
			Free(g->contentType);
		g->contentType		= NULL;

		if (!StrNCaselessCompare(ext, "htm", 3)) {
			if (g->pageContentType)
				Free(g->pageContentType);
			g->pageContentType	= NULL;

			g->contentType		= StrDup("text/html");
			g->pageContentType 	= StrDup(g->contentType);
		} else if (!StrNCaselessCompare(ext, "wmlc", 4)) {
			if (g->pageContentType)
				Free(g->pageContentType);
			g->pageContentType	= NULL;

			g->contentType		= StrDup("application/vnd.wap.wmlc");
			g->pageContentType 	= StrDup(g->contentType);
		} else if (!StrNCaselessCompare(ext, "wmlsc", 5)) {
			if (g->pageContentType)
				Free(g->pageContentType);
			g->pageContentType	= NULL;

			g->contentType		= StrDup("application/vnd.wap.wmlscriptc");
			g->pageContentType 	= StrDup(g->contentType);
		} else if (!StrNCaselessCompare(ext, "wml", 3)) {
			if (g->pageContentType)
				Free(g->pageContentType);
			g->pageContentType	= NULL;

			g->contentType		= StrDup("text/vnd.wap.wml");
			g->pageContentType 	= StrDup(g->contentType);
		} else if (!StrNCaselessCompare(ext, "rss", 3)) {
			if (g->pageContentType)
				Free(g->pageContentType);
			g->pageContentType	= NULL;

			g->contentType		= StrDup("application/rss+xml");
			g->pageContentType 	= StrDup(g->contentType);
		} else if (!StrNCaselessCompare(ext, "gif", 3)) {
			g->contentType		= StrDup("image/gif");
		} else if (!StrNCaselessCompare(ext, "jpg", 3) ||
			   !StrNCaselessCompare(ext, "jpeg", 4)) {
			g->contentType		= StrDup("image/jpeg");
		} else if (!StrNCaselessCompare(ext, "png", 3)) {
			g->contentType		= StrDup("image/png");
		} else if (!StrNCaselessCompare(ext, "wbmp", 4)) {
			g->contentType		= StrDup("image/vnd.wap.wbmp");
		} else if (!StrNCaselessCompare(ext, "bmp", 3)) {
			g->contentType		= StrDup("image/bmp");
		} else if (!StrNCaselessCompare(ext, "pbmp", 4)) {
			g->contentType		= StrDup("image/vnd.palm.pbmp");
		}

		DebugOutFileURI("FileURIOpen", g->contentType, 0, __FILE__, __LINE__, g);
		DebugOutFileURI("FileURIOpen", g->pageContentType, 0, __FILE__, __LINE__, g);
	}

	DebugOutFileURI("FileURIOpen", "got extentsion. finish up", 0, __FILE__, __LINE__, g);

	if (remember) {
		StrCopy(g->WapLastUrl, g->Url);
	}

	DebugOutFileURI("FileURIOpen", "done!", 0, __FILE__, __LINE__, g);

	DebugOutFileURI("FileURIOpen", "returning", 0, __FILE__, __LINE__, g);

	return 0;
}
