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

#include 	"history.h"
#include 	"WAP.h"
#include 	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"PalmUtils.h"
#include	"debug.h"


Int16 HistoryAskRepost(void)
{
	return FrmCustomAlert(alHistoryPost, "", "", NULL);
}

void historyClear(GlobalsType *g)
{
	if (!g->History)
		return;

	g->History->index = g->History->count;

	while (g->History->count >= 0) {
		if (g->History->hist[g->History->index].url) {
			Free(g->History->hist[g->History->index].url);
			g->History->hist[g->History->index].url = NULL;
		}
		if (g->History->hist[g->History->index].postData) {
			Free(g->History->hist[g->History->index].postData);
			g->History->hist[g->History->index].postData = NULL;
		}
		g->History->count--;
		g->History->index--;
	}

	if (g->History != NULL) {
		Free(g->History);
		g->History = NULL;
	}
}

int historyPush(Char *url, GlobalsType *g )
{
	if ( !g->History ) { // no history yet, please create one
		g->History = Malloc ( sizeof(historyType) );
		MemSet(g->History,sizeof(historyType),0);
		g->History->index = -1;
		g->History->count = 0;
	}

	if ((g->History->index) >= 0) {
		if (g->History->hist[g->History->index].url && 
		    !StrNCompare(g->History->hist[g->History->index].url, url, StrLen(url)) &&
		    (g->History->hist[g->History->index].method != POST))
				return 0; // Do not add if last URL is same
	}

	if (g->History->index < (MAX_HISTORY - 1))
		g->History->index++;
	else
		g->History->index = 0;

	if(g->History->count < MAX_HISTORY)			
		g->History->count++;
	if(g->History->hist[g->History->index].url)
		Free(g->History->hist[g->History->index].url);
	g->History->hist[g->History->index].url = Malloc(StrLen(url)+1);
	ErrFatalDisplayIf (!g->History->hist[g->History->index].url, "wml.c/Malloc(history2)");
	StrCopy(g->History->hist[g->History->index].url,url);

	if (g->post && g->postBuf) {
		if(g->History->hist[g->History->index].postData)
			Free(g->History->hist[g->History->index].postData);
		g->History->hist[g->History->index].postData = StrDup(g->postBuf);
		if (g->History->hist[g->History->index].postData) {
			g->History->hist[g->History->index].method = POST;
		} else {
			g->History->hist[g->History->index].method = GET;
		}
	} else {
		g->History->hist[g->History->index].method = GET;
		g->History->hist[g->History->index].postData = NULL;	
	}

	return 0;			
}

Char *historyPop(GlobalsType *g)
{
Char *tmp, *ret;

	
	if ( !g->History )
		return(NULL);
	if (g->History->count== 1)
		return(NULL);		
    	if (g->History->index > 0)
		g->History->index--;
	else
		g->History->index = (MAX_HISTORY - 1);	
	g->History->count--;
	tmp = g->History->hist[g->History->index].url;
	if(!tmp)
		return(NULL);		
	ret = Malloc(StrLen(tmp)+1);
   	ErrFatalDisplayIf (!ret, "wml.c/Malloc(history3)");
	StrCopy(ret,tmp);

	if (g->History->hist[g->History->index].method == POST) {
		if (HistoryAskRepost()) {
			g->post = false;
			if (g->postBuf)
				Free(g->postBuf);
			g->postBuf = NULL;
		} else {
			g->post = true;

			if (g->postBuf)
				Free(g->postBuf);
			g->postBuf = NULL;

			g->postBuf = StrDup(g->History->hist[g->History->index].postData);
			if (!g->postBuf) {
				g->post = false;
			}
		}
	} else {
		g->post = false;
		if (g->postBuf)
			Free(g->postBuf);
		g->postBuf = NULL;
	}

	return(ret);
}

void HistoryCreateHistoryFile(UInt16 mode, GlobalsType *g)
{
	Int16           error;        // error code
	UInt16          cardNo;       // card containing the application database
	LocalID         dbID;         // handle for application database
	UInt16          dbAttrs;      // database attributes


 	g->DbHistory = DmOpenDatabaseByTypeCreator(dbHistoryDBType, wuAppType, mode);
  	if (!g->DbHistory) {
      		// The database doesn't exist, create it now.
      		error = DmCreateDatabase (0, dbHistoryDBName, wuAppType,
                                dbHistoryDBType, false);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error,
                         "Create new HistoryDB failed");

      		// Find the application's database.
      		g->DbHistory = DmOpenDatabaseByTypeCreator(dbHistoryDBType,
                                                wuAppType, mode);

      		// Get info about the database
      		error = DmOpenDatabaseInfo(g->DbHistory, &dbID, NULL, NULL, &cardNo, NULL);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error,
                         "Get HistoryDB info failed");

      		// Get attributes for the database
      		error = DmDatabaseInfo (0, dbID, NULL, &dbAttrs, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error,
                         "Get HistoryDB info failed");

      		// Set the new attributes in the database
      		error = DmSetDatabaseInfo(0, dbID, NULL, &dbAttrs, NULL, NULL,
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error,
                         "Set HistoryDB info failed");

    	}
}

Boolean HistoryCreateRecord(UInt16 *recIndex, GlobalsType *g)
{
	MemPtr          p;
	MemHandle       historyRec;
	UInt16          index = 0;
	Err             error;
	UInt16          attr;

	// Create a new first record in the database
	historyRec = DmNewRecord(g->DbHistory, &index, dbHistorySize);

	// Lock down the block containing the new record.
	p = MemHandleLock(historyRec);

	// Write a zero to the first byte of the record to null terminate the new
	// URL string.
	error = DmSet(p, 0, dbHistorySize, (UInt8) 0);

	// Check for fatal error.
	//ErrFatalDisplayIf(error, "Could not write to new record.");
	if (error) {
		MemHandleUnlock(historyRec);
		return(false);
	}

	// Unlock the block of the new record.
	MemPtrUnlock(p);

  	// P11. Set the category of the new record to the current category.
  	DmRecordInfo(g->DbHistory, index, &attr, NULL, NULL);
  	attr &= ~dmRecAttrCategoryMask; // Remove all category bits
  	// no category support for connection settings (yet?), place
  	// the new record in the unfiled category.
  	attr |= dmUnfiledCategory;
  	DmSetRecordInfo(g->DbHistory, index, &attr, NULL);


	// Release the record to the database manager.  The true value indicates
	// that
	// the record contains "dirty" data.  Release Record will set the record's 
	// dirty flag and update the database modification count.
	DmReleaseRecord(g->DbHistory, index, true);

	*recIndex = index;

	return (true);
}

Boolean HistoryDeleteEntry(Char *url, UInt16 index, GlobalsType *g)
{
	UInt16            	nrRecs, pos;
	MemHandle        	historyRec;
	Err			error = errNone;
	Char			*phistory;

	nrRecs = DmNumRecordsInCategory(g->DbHistory, dmAllCategories);
	if (index < nrRecs) {
		pos = index;

		historyRec = DmQueryNextInCategory(g->DbHistory, &pos, dmAllCategories);
      		phistory = MemHandleLock(historyRec);

		MemHandleUnlock(historyRec);
		error = DmDeleteRecord(g->DbHistory, pos);
		error = DmRemoveRecord(g->DbHistory, pos);

		return true;
	} else {
		return false;
	}

	/*pos = 0;
	for (i = 0; i < nrRecs; i++) {
      		historyRec = DmQueryNextInCategory(g->DbHistory, &pos, dmAllCategories);
      		phistory = MemHandleLock(historyRec);

      		if (StrCompare(url,phistory) == 0) {
			MemHandleUnlock(historyRec);
			error = DmDeleteRecord(g->DbHistory, pos);
			error = DmRemoveRecord(g->DbHistory, pos);
			return false;
        	}
     		MemHandleUnlock(historyRec);

      		pos++;
    	}*/

	return false;
}

Boolean HistoryFindEntry(Char *url, GlobalsType *g)
{
	UInt16            	nrRecs, i, pos;
	MemHandle        	historyRec;
	Char			*phistory;
	Boolean			found = false;


	nrRecs = DmNumRecordsInCategory(g->DbHistory, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
      		historyRec = DmQueryNextInCategory(g->DbHistory, &pos, dmAllCategories);
      		phistory = MemHandleLock(historyRec);

      		if (StrCompare(url,phistory) == 0) {
			found = true;
        	}
     		MemHandleUnlock(historyRec);

      		pos++;
    	}

	return found;
}

Boolean HistoryAddPageToHistory(Char *url, Char *title, GlobalsType *g)
{
	UInt32		offset = 0;
	UInt16		recIndex = 0;
	Err		error;
	MemHandle	currec;
	Char		*s;
	Boolean		handled = false;
	UInt16		recordLength = 0;

	recordLength = StrLen(url) + StrLen(title) + 2;

	if (g->privateBrowsing == true)
		return false;

	if (HistoryFindEntry(url, g) == true) {
		goto history_add_cleanup; // already exists in the history
	}


	if (HistoryCreateRecord(&recIndex, g) != true)
		goto history_add_cleanup;

	currec = DmGetRecord(g->DbHistory, 0);

	if (MemHandleResize(currec, recordLength) == 0) {
      		s = MemHandleLock(currec);
      		offset = 0;

      		error = DmStrCopy(s, offset, (Char *)url);

      		offset += StrLen(url) + 1;

      		error = DmStrCopy(s, offset, (Char *)title);

      		offset += StrLen(title) + 1;


      		MemHandleUnlock(currec);

		handled = true;
    	} else {
	        ErrDisplay ("***MemHandleResize Failed");
	}

	error = DmReleaseRecord(g->DbHistory, 0, true);
	
history_add_cleanup:


	return handled;
}

Boolean HistoryGoToURL(Char *url, UInt16 index, Boolean closeForm, GlobalsType *g)
{
	UInt16		nrRecs, pos;
	MemHandle	historyRec;
	Char		*phistory;
	Boolean		loaded = false;
	Char		*link = NULL;


	nrRecs = DmNumRecordsInCategory(g->DbHistory, dmAllCategories);
	if (index < nrRecs) {
		pos = index;

		historyRec = DmQueryNextInCategory(g->DbHistory, &pos, dmAllCategories);
      		phistory = MemHandleLock(historyRec);
		
		debugOut("HistoryGo", "dup string", 0, __FILE__, __LINE__, g);

		if (!closeForm) {
			link = StrDup(phistory);
			if (!link)
				goto history_go_cleanup;

			debugOut("HistoryGo", "unlock", 0, __FILE__, __LINE__, g);

			MemHandleUnlock(historyRec);

			debugOut("HistoryGo", "followLink", 0, __FILE__, __LINE__, g);

			followLink(link, g);
			Free(link);
		} else {
			g->OpenUrl = StrDup(phistory);
			if (!g->OpenUrl) {
				MemHandleUnlock(historyRec);
				FrmReturnToForm(0);
				return false;
			}

			//TODO: Handle for launching from another form?
			FrmReturnToForm(frmBrowser);
			FrmUpdateForm(frmBrowser, frmUpdateGotoUrl);
		}

		debugOut("HistoryGo", "loaded", 0, __FILE__, __LINE__, g);

		loaded = true;
	} else {
		loaded = false;
	}

	return loaded;

history_go_cleanup:
	MemHandleUnlock(historyRec);

	return false;
}

Boolean HistoryClearHistory(GlobalsType *g)
{
	UInt16          nrRecs, i, pos;
	MemHandle       historyRec;
	Boolean		cleared = false;
	Err		error = errNone;


	nrRecs = DmNumRecordsInCategory(g->DbHistory, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
		nrRecs = DmNumRecordsInCategory(g->DbHistory, dmAllCategories);
		pos = 0; i = 0;
      		historyRec = DmQueryNextInCategory(g->DbHistory, &pos, dmAllCategories);

		error = DmDeleteRecord(g->DbHistory, pos);
		error = DmRemoveRecord(g->DbHistory, pos);

      		pos++;
    	}

	if (error == errNone)
		cleared = true;

	return cleared;
}

