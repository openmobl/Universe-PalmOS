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
#include 	<VFSMgr.h>
#include	<FileStream.h>
#include	"WAPUniverse.h"
#include	"../res/WAPUniverse_res.h"
#include	"http.h"

int DownloadToVfs(Char *filename, char *data, UInt32 size);
int DownloadDBToVfs(Char *filename, /*LocalID dbIDP, */UInt32 size);

#define	VFS_WRITE_BLOCK	4096

Boolean FormHandleVfsDownload(Int16 button, Char *attempt)
{
	int		ret = 0;
	GlobalsType	*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	switch (button) {
		case frmResponseCreate:
			break;
		case 0:
			if (g->httpUseTempFile == true) {
				/*UInt16 			cardNoP = 0;
				LocalID 		dbIDP;
				DmSearchStateType 	stateInfoP;
				Err			error;

				error = DmGetNextDatabaseByTypeCreator(true, &stateInfoP, 'DATA', 'Wapu', true, &cardNoP, &dbIDP); // should check for errors
				dbIDP = DmFindDatabase(cardNoP, "Universe-HTTP-Cache");

				ret = DownloadDBToVfs(attempt, dbIDP, g->webData.length);*/
				
				ret = DownloadDBToVfs(attempt, g->webData.length);

				g->httpUseTempFile = false;
				//HTTPFlushCache();

				if (ret == 0)
					return true;
				else if (ret == 1)
					return false;
				else
					return true;
			} else {
				ret = DownloadToVfs(attempt, g->webData.data, g->webData.length);
				if (ret == 0)
					return true;
				else if (ret == 1)
					return false;
				else
					return true;
			}
			break;
		case 1:
			return true;
			break;
		case frmResponseQuit:
			break;
		default:
			break;
	}

	return false;
}

void VfsError(Err err)
{
	switch (err) {
		case vfsErrFileNotFound:
		case expErrNotOpen:
			MyErrorFunc("Generic vfs error.", NULL);
			break;
		case vfsErrBadName:
			MyErrorFunc("The file name you entered contains invalid characters. Please enter a new name.", NULL);
			break;
		/*case vfsErrFileAlreadyExists:
			MyErrorFunc("The file already exists. Enter a new name.", NULL); // add overwrite option...
			break;*/
		case vfsErrNoFileSystem:
		case vfsErrVolumeBadRef:
			MyErrorFunc("No expansion card found.", NULL);
			break;
		case vfsErrVolumeFull:
			MyErrorFunc("Not enough free space on the memory card.", NULL);
			break;
		case expErrCardReadOnly:
			MyErrorFunc("Could not save to card. The memory card is read only.", NULL);
			break;
		case vfsErrFilePermissionDenied:
			MyErrorFunc("You do not have permission to save to the card..", NULL);
			break;
		case vfsErrIsADirectory:
			MyErrorFunc("The file name you entered is a directory.", NULL);
			break;
		case errNone:
		default:
			break;

	}
}

void cacheError(Err err)
{
	switch (err) {
		case fileErrMemError:
			MyErrorFunc("Could not open or work with file, out of memory.", NULL);
			break;
		case fileErrInvalidParam:
			MyErrorFunc("Could not open or work with file, invalid.", NULL);
			break;
		case fileErrCorruptFile:
			MyErrorFunc("Could not open or work with file, file is corrupt.", NULL);
			break;
		case fileErrNotFound:
			MyErrorFunc("Could not find the cache.", NULL);
			break;
		case fileErrTypeCreatorMismatch:
			MyErrorFunc("Could not open or work with file, the provided settings do not match.", NULL);
			break;
		case fileErrReplaceError:
			MyErrorFunc("Could not replace the file.", NULL);
			break;
		case fileErrCreateError:
			MyErrorFunc("Could not create the file.", NULL);
			break;
		case fileErrOpenError:
			MyErrorFunc("Could not open or work with file.", NULL);
			break;
		case fileErrInUse:
			MyErrorFunc("The file is already in use.", NULL);
			break;
		case fileErrReadOnly:
			MyErrorFunc("The file is read only.", NULL);
			break;
		case fileErrInvalidDescriptor:
			MyErrorFunc("Invalid file descriptor.", NULL);
			break;
		case fileErrCloseError:
			MyErrorFunc("Generic close error.", NULL);
			break;
		case fileErrOutOfBounds:
			MyErrorFunc("Error out of bounds.", NULL);
			break;
		case fileErrPermissionDenied:
			MyErrorFunc("Could not access the file file. Permission Denied.", NULL);
			break;
		case fileErrIOError:
			MyErrorFunc("File IO Error.", NULL);
			break;
		case fileErrEOF:
			MyErrorFunc("The end of the file has been reached.", NULL);
			break;
		case fileErrNotStream:
			MyErrorFunc("Invalid file type. The requested file is not a stream file.", NULL);
			break;
		case errNone:
		default:
			break;
	}
}

int DownloadToVfs(Char *filename, char *data, UInt32 size)
{
	UInt16 		volRefFirst = 0;
	UInt16 		volRefNum = 0; 
	UInt32 		volIterator = vfsIteratorStart;
	FileRef 	fileRefP;
	Err		err;
	Char		filepath[255];
	UInt32 		vfsMgrVersion;
	UInt32 		numBytesWrittenP = 0;
	UInt32 		bufferoffset = 0;

	// check for a VFS palm
	err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &vfsMgrVersion); 
	if(err){	   
    		MyErrorFunc("This device does not support memory cards.", NULL);
		return 2;
	}

 
	while (volIterator != vfsIteratorStop) { 
		err = VFSVolumeEnumerate(&volRefNum, &volIterator); 
		if (err == errNone) { 
			// Do something with the volRefNum
			volRefFirst = volRefNum;
		} else { 
			// handle error...?
			MyErrorFunc("No expansion card found.", NULL);
		} 
	}
	volRefNum = volRefFirst;

	err = VFSDirCreate(volRefNum, g_prefs.downloadTo);
	if (err != errNone && err != vfsErrFileAlreadyExists) {
		VfsError(err);
		return 2;
	}

	StrCopy(filepath, g_prefs.downloadTo);
	StrCopy(filepath+StrLen(filepath), filename);

top:
	err = VFSFileCreate(volRefNum, filepath);
	if (err == vfsErrFileAlreadyExists) {
		switch ( FrmCustomAlert(alVfsFileExists, filename, "", "") ) {
			case 0:
				return 1;
				break;
			case 1:
				err = VFSFileDelete(volRefNum, filepath);
				if (err != errNone) {
					VfsError(err);
					return 1;
				}
				goto top;
				break;
			default:
				return 1;
				break;
		}
	} else if (err != errNone) {
		VfsError(err);
		return 2;
	}

	err = VFSFileOpen(volRefNum, filepath, vfsModeReadWrite, &fileRefP);
	if (err != errNone) {
		VfsError(err);
		return 2;
	}

	/*err = VFSFileWrite(fileRefP, size, data, NULL);
	if (err != errNone) {
		VfsError(err);
		return 2;
	}*/

	do {
         	err = VFSFileWrite(fileRefP, size - bufferoffset, data + bufferoffset,
    					&numBytesWrittenP);
         	if (err != errNone) {
			VfsError(err);
			return 2;
		}

         	bufferoffset += numBytesWrittenP;
     	} while (bufferoffset < size);

	err = VFSFileClose(fileRefP);

	FrmCustomAlert(alVfsDownloadComplete, filename, "", "");

	return 0;
}

static Boolean progressVFSCallbackFunction(PrgCallbackDataPtr cbP)
{
	StrCopy(cbP->textP, cbP->message);
	cbP->bitmapId = NULL;
	cbP->textChanged = true;

	return true;
} 

int DownloadDBToVfs(Char *filename, /*LocalID dbIDP, */UInt32 size)
{
	UInt16 		volRefFirst = 0;
	UInt16 		volRefNum = 0; 
	UInt32 		volIterator = vfsIteratorStart;
	FileRef 	fileRefP;
	Err		err, fileError;
	Char		filepath[255];
	UInt32 		vfsMgrVersion; 
	Char		*tempData;
	UInt32		copied = 0;
	UInt32 		numBytesWrittenP = 0;
	FileHand	cache = NULL;
	UInt32		readBytes = 0;
	Boolean		loopError = false;
	UInt32 		bufferoffset = 0;
	ProgressPtr 	prg = NULL;
	EventType 	event;

	// check for a VFS palm
	err = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &vfsMgrVersion); 
	if(err){	   
    		MyErrorFunc("This device does not support memory cards.", NULL);
		return 2;
	}

 
	while (volIterator != vfsIteratorStop) { 
		err = VFSVolumeEnumerate(&volRefNum, &volIterator); 
		if (err == errNone) { 
			// Do something with the volRefNum
			volRefFirst = volRefNum;
		} else { 
			// handle error...?
			MyErrorFunc("No expansion card found.", NULL);
		} 
	}
	volRefNum = volRefFirst;

	err = VFSDirCreate(volRefNum, g_prefs.downloadTo);
	if (err != errNone && err != vfsErrFileAlreadyExists) {
		VfsError(err);
		return 2;
	}

	StrCopy(filepath, g_prefs.downloadTo);
	StrCopy(filepath+StrLen(filepath), filename);

top:
	err = VFSFileCreate(volRefNum, filepath);
	if (err == vfsErrFileAlreadyExists) {
		switch ( FrmCustomAlert(alVfsFileExists, filename, "", "") ) {
			case 0:
				return 1;
				break;
			case 1:
				err = VFSFileDelete(volRefNum, filepath);
				if (err != errNone) {
					VfsError(err);
					return 1;
				}
				goto top;
				break;
			default:
				return 1;
				break;
		}
	} else if (err != errNone) {
		VfsError(err);
		return 2;
	}

	err = VFSFileOpen(volRefNum, filepath, vfsModeReadWrite, &fileRefP);
	if (err != errNone) {
		VfsError(err);
		return 2;
	}

    	cache = FileOpen( 0, "Universe-HTTP-Cache", 'DATA', 'Wapu', fileModeReadOnly, &fileError);
	if (fileError != errNone) {
		cacheError(fileError);
		VFSFileClose(fileRefP);
		VFSFileDelete(fileRefP, filepath); // delete file if we encounter an error
		return 2;
	}

	/*for (copied = 0; copied < size; copied += numBytesWrittenP) {
		tempData = Malloc(VFS_WRITE_BLOCK);

		if (!tempData) {
			cacheError(fileErrMemError);
			return 2;
		}

		readBytes = FileRead( cache, tempData, VFS_WRITE_BLOCK, 1, &fileError);
		if (fileError != errNone) {
			cacheError(fileError);
			if (tempData != NULL) Free(tempData);
			tempData = NULL;
			return 2;
		}

		err = VFSFileWrite(fileRefP, readBytes, tempData, &numBytesWrittenP);
		if (err != errNone) {
			VfsError(err);
			if (tempData != NULL) Free(tempData);
			tempData = NULL;
			return 2;
		}


		Free(tempData);
		tempData = NULL;
	}*/

	tempData = Malloc(VFS_WRITE_BLOCK);

	if (!tempData) {
		MyErrorFunc("Could not save to card. Out of memory.", NULL);

		FileFlush(cache); // clean up the cache
		FileClose(cache);

		err = VFSFileClose(fileRefP);
		VFSFileDelete(fileRefP, filepath); // delete file if we encounter an error

		return 2;
	}

	prg = PrgStartDialog("Saving to Card",progressVFSCallbackFunction, NULL);

	for (copied = 0; copied < size; copied += numBytesWrittenP) {

		PrgUpdateDialog(prg, 0, 1, "Saving the file to the memory card.", true);
 		EvtGetEvent(&event, 0);
 		if (!PrgHandleEvent(prg, &event)) {
  			if (PrgUserCancel(prg)) {
				loopError = true;
				break;
			}
		}

		readBytes = FileRead( cache, tempData, 1, VFS_WRITE_BLOCK, &fileError);
		if (fileError != errNone) {
			if (fileError == fileErrEOF) break;

			cacheError(fileError);
			loopError = true;
			break;
		}

		do {
         		err = VFSFileWrite(fileRefP, readBytes - bufferoffset, tempData + bufferoffset,
    					&numBytesWrittenP);
         		if (err != errNone) {
				VfsError(err);
				loopError = true;
				break;
			}

         		bufferoffset += numBytesWrittenP;
     		} while (bufferoffset < readBytes);

		bufferoffset = 0;

		if (loopError == true)
			break;
	}

	if (tempData != NULL) Free(tempData);
	tempData = NULL;


	FileFlush(cache); // clean up the cache
	FileClose(cache);

	err = VFSFileClose(fileRefP);

	if (prg) PrgStopDialog(prg, true);

	if (loopError == true) {
		VFSFileDelete(fileRefP, filepath); // delete file if we encounter an error
		return 2;
	}

	FrmCustomAlert(alVfsDownloadComplete, filename, "", "");

	return 0;
}

