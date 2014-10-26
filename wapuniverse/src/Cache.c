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
#include 	"cache.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"build.h"
#include	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"

#define		CACHE_MAX_RECORD_SIZE	63	// maximum size for database records, limit: 64kb
#define		BLOCK_SIZE		1024	// size of 1 byte
#define		MAX_SIZE		64512


static Boolean CacheUpdateCache(GlobalsType *g)
{
	/*UInt16          nrRecs, i, pos;
	UInt32		j = 0, k = 0;
	MemHandle       cacheRec;
	Char		*pcache;
	Char		*s;
	Char		size[maxStrIToALen];
	Err		error = 0;
	Boolean		private = false;
	UInt32		offset = 0;
	Char		*contentType = NULL;
	Char		*url = NULL;
	Char		*data = NULL;
	UInt32		length = 0;
	UInt32		recordLength;*/
	UInt16          cardNo;       // card containing the application database
	LocalID         dbID;         // handle for application database

	CacheClearCache(g);

	/*nrRecs = DmNumRecordsInCategory(g->cache.DbCache, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
      		cacheRec = DmQueryNextInCategory(g->cache.DbCache, &pos, dmAllCategories);
      		pcache = MemHandleLock(cacheRec);

		//private = (Boolean)pcache[0];
		//pcache++;

  		url = StrDup(pcache);
		pcache += StrLen(pcache) + 1;

		contentType = StrDup(pcache);
		pcache += StrLen(pcache) + 1;

		StrCopy(size, pcache);
  		pcache += StrLen(pcache) + 1;

		length = StrAToI(size);

		data = Malloc(length+1);
		for (j=0,k=0;j<(length);j++)
    			data[j] = pcache[k++];

		
     		MemHandleUnlock(cacheRec);

		//StrIToA(size, length);
		recordLength = StrLen(url) + length + StrLen(contentType) + StrLen(size) + 4;

		if (MemHandleResize(cacheRec, recordLength) == 0) {
      			s = MemHandleLock(cacheRec);
      			offset = 0;
			DmWrite (s, offset, (Char *) &private, 1);
			offset++;

      			error = DmStrCopy(s, offset, (Char *)url);

      			offset += StrLen(url) + 1;

      			error = DmStrCopy(s, offset, (Char *)contentType);

      			offset += StrLen(contentType) + 1;

			StrIToA(size, length);

      			error = DmStrCopy(s, offset, (Char *)size);

      			offset += StrLen(size) + 1;

      			//error = DmStrCopy(s, offset, (Char *)data);
			error = DmWrite(s, offset, data, length);

			MemHandleUnlock(cacheRec);
		} else {
			ErrDisplay ("***MemHandleResize Failed");
		}

		DmReleaseRecord(g->cache.DbCache, pos, true);

		Free(url);
		Free(contentType);
		Free(data);
		private = false;
		length = 0;

      		pos++;
    	}*/

	DmOpenDatabaseInfo(g->cache.DbCache, &dbID, NULL, NULL, &cardNo, NULL);

	g->cache.version = dbCacheVersion;

	DmSetDatabaseInfo(0, dbID, NULL, NULL, &g->cache.version, NULL, NULL,
                              	NULL, NULL, NULL, NULL, NULL, NULL);

	return true;
}

void CacheCreateCacheFile(UInt16 mode, GlobalsType *g)
{
	Int16           error;        // error code
	UInt16          cardNo;       // card containing the application database
	LocalID         dbID;         // handle for application database
	UInt16          dbAttrs;      // database attributes


 	g->cache.DbCache = DmOpenDatabaseByTypeCreator(dbCacheDBType, wuAppType, mode);
  	if (!g->cache.DbCache) {
      		// The database doesn't exist, create it now.
      		error = DmCreateDatabase (0, dbCacheDBName, wuAppType,
                                dbCacheDBType, false);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error,
                         "Create new CacheDB failed");

      		// Find the application's database.
      		g->cache.DbCache = DmOpenDatabaseByTypeCreator(dbCacheDBType,
                                                wuAppType, mode);

      		// Get info about the database
      		error = DmOpenDatabaseInfo(g->cache.DbCache, &dbID, NULL, NULL, &cardNo, NULL);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error,
                         "Get CacheDB info failed");

      		// Get attributes for the database
      		error = DmDatabaseInfo (0, dbID, NULL, &dbAttrs, &g->cache.version, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error,
                         "Get CacheDB info failed");

		g->cache.version = dbCacheVersion;
      		// Set the new attributes in the database
      		error = DmSetDatabaseInfo(0, dbID, NULL, &dbAttrs, &g->cache.version, NULL,
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error,
                         "Set CacheDB info failed");

    	} else {
		error = DmOpenDatabaseInfo(g->cache.DbCache, &dbID, NULL, NULL, &cardNo, NULL);
		if (!error) {
      			error = DmDatabaseInfo (0, dbID, NULL, NULL, &g->cache.version, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL);
			if (!error && (g->cache.version < dbCacheVersion)) {
				CacheUpdateCache(g);
			} /* If the cache version is greater then decaliared, complain */
		}
	}
}

static UInt16 CacheUrlLen(Char *url)
{
	/*if (StrChr(url, '?')) {
		return (UInt16)(StrChr(url, '?') - url);
	} else*/ if (StrChr(url, '#')) {
		return (UInt16)(StrChr(url, '#') - url);
	} else {
		return StrLen(url);
	}

	return 0;
}

Boolean CacheCreateRecord(GlobalsType *g, UInt16 *recIndex)
{
	MemPtr          p;
	MemHandle       cacheRec;
	UInt16          index = 0;
	Err             error;
	UInt16          attr;

	// Create a new first record in the database
	cacheRec = DmNewRecord(g->cache.DbCache, &index, dbCacheSize);

	// Lock down the block containing the new record.
	p = MemHandleLock(cacheRec);

	// Write a zero to the first byte of the record to null terminate the new
	// URL string.
	error = DmSet(p, 0, dbCacheSize, (UInt8) 0);

	// Check for fatal error.
	//ErrFatalDisplayIf(error, "Could not write to new record.");
	if (error) {
		MemHandleUnlock(cacheRec);
		return(false);
	}

	// Unlock the block of the new record.
	MemPtrUnlock(p);

  	// P11. Set the category of the new record to the current category.
  	DmRecordInfo(g->cache.DbCache, index, &attr, NULL, NULL);
  	attr &= ~dmRecAttrCategoryMask; // Remove all category bits
  	// no category support for connection settings (yet?), place
  	// the new record in the unfiled category.
  	attr |= dmUnfiledCategory;
  	DmSetRecordInfo(g->cache.DbCache, index, &attr, NULL);


	// Release the record to the database manager.  The true value indicates
	// that
	// the record contains "dirty" data.  Release Record will set the record's 
	// dirty flag and update the database modification count.
	DmReleaseRecord(g->cache.DbCache, index, true);

	*recIndex = index;

	return (true);
}

Boolean CacheFindCachedFile(Char *url, Boolean isImage, GlobalsType *g)
{
	UInt16            	nrRecs, i, pos;
	MemHandle        	cacheRec;
	Char			*pcache;
	Boolean			found = false;
	UInt16			urlLen = CacheUrlLen(url);

	if (g->cache.cacheDoNotRead == true)
		return false;

	nrRecs = DmNumRecordsInCategory(g->cache.DbCache, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
      		cacheRec = DmQueryNextInCategory(g->cache.DbCache, &pos, dmAllCategories);
      		pcache = MemHandleLock(cacheRec);

      		if (((Boolean)pcache[0] == g->privateBrowsing) &&
		    !StrNCompare(url, pcache + 1, urlLen) && (CacheUrlLen(pcache + 1) == urlLen)) {
			found = true;
        	}
     		MemHandleUnlock(cacheRec);

      		pos++;
    	}

	return found;
}

static Boolean private_CacheFindCachedFile(Char *url, Boolean isImage, GlobalsType *g)
{
	UInt16            	nrRecs, i, pos;
	MemHandle        	cacheRec;
	Char			*pcache;
	Boolean			found = false;
	UInt16			urlLen = StrLen(url);

	urlLen = CacheUrlLen(url);

	nrRecs = DmNumRecordsInCategory(g->cache.DbCache, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
      		cacheRec = DmQueryNextInCategory(g->cache.DbCache, &pos, dmAllCategories);
      		pcache = MemHandleLock(cacheRec);

      		if (((Boolean)pcache[0] == g->privateBrowsing) &&
		    !StrNCompare(url, pcache + 1, urlLen) && (CacheUrlLen(pcache + 1) == urlLen)) {
			found = true;
        	}
     		MemHandleUnlock(cacheRec);

      		pos++;
    	}

	return found;
}

Boolean CacheDeleteEntry(Char *url, Boolean isImage, GlobalsType *g)
{
	UInt16            	nrRecs, i, pos;
	MemHandle        	cacheRec;
	Err			error = errNone;
	Char			*pcache;
	UInt16			urlLen = StrLen(url);

	urlLen = CacheUrlLen(url);

	nrRecs = DmNumRecordsInCategory(g->cache.DbCache, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
      		cacheRec = DmQueryNextInCategory(g->cache.DbCache, &pos, dmAllCategories);
      		pcache = MemHandleLock(cacheRec);

      		if (!StrNCompare(url, pcache + 1, urlLen) && (CacheUrlLen(pcache + 1) == urlLen)) {
			MemHandleUnlock(cacheRec);
			error = DmDeleteRecord(g->cache.DbCache, pos);
			error = DmRemoveRecord(g->cache.DbCache, pos);
			return false;
        	}
     		MemHandleUnlock(cacheRec);

      		pos++;
    	}

	return false;
}

UInt32 CacheGetStoreSize(GlobalsType *g)
{
	Err		error;
	UInt16		cardNo = 0;
	LocalID		dbID;
	UInt32		dataBytes = 0;

	error = DmOpenDatabaseInfo(g->cache.DbCache, &dbID, NULL, NULL, &cardNo, NULL);

	error = DmDatabaseSize(cardNo, dbID, NULL, NULL, &dataBytes);

	return dataBytes;
}

Boolean CacheIsUncacheable(Char *url)
{
	Char	*uncacheable[] = { "mtch", "file", NULL };
	int	i = 0;

	while (uncacheable[i]) {
		if (!StrNCaselessCompare(url, uncacheable[i], StrLen(uncacheable[i]))) {
			return true;
		}
		i++;
	}

	return false;
}

Boolean CacheAddFileToCache(WebDataPtr webData, Char *url, Boolean isImage, GlobalsType *g)
{
	UInt32		recordLength;
	UInt32		offset = 0;
	UInt16		recIndex = 0;
	Err		error;
	MemHandle	currec;
	Char		*s;
	Char		size[maxStrIToALen];
	Boolean		handled = false;

	/*
	if (g->privateBrowsing == true)
		return false;
	*/

	if (CacheIsUncacheable(url))
		return false;

	StrIToA(size, webData->length);

	recordLength = StrLen(url) + webData->length + StrLen(g->contentType) + StrLen(size) + 4;


	if (recordLength > MAX_SIZE)
		goto cache_add_cleanup;

	if (((CacheGetStoreSize(g) + recordLength) > g_prefs.maxCache) ||
        (recordLength > NVFSGetNVFSFreeSize())) { // uh oh, NVFS dbcache out of room...
		Boolean	cleared = true;
		// need to add code to cleanup the cache
		while (((CacheGetStoreSize(g) + recordLength) > g_prefs.maxCache) && (cleared == true)) {
			cleared = CacheClearLastFile(g);
		}
	}
	
	if (private_CacheFindCachedFile(url, isImage, g) == true) {
		if (g->cache.cacheDoNotRead == true)
			CacheDeleteEntry(url, isImage, g); // update with newer
		else
			goto cache_add_cleanup; // already exists in the cache
	}


	if (!CacheCreateRecord(g, &recIndex))
		goto cache_add_cleanup;

	currec = DmGetRecord(g->cache.DbCache, 0);

	if (MemHandleResize(currec, recordLength) == 0) {
      		s = MemHandleLock(currec);
      		offset = 0;

		/*StrIToA(size, (Int32)g->privateBrowsing);

      		error = DmStrCopy(s, offset, (Char *)size);

      		offset += StrLen(size) + 1;*/
		
		DmWrite (s, offset, (Char *) &g->privateBrowsing, 1);
		offset++;

      		error = DmStrCopy(s, offset, (Char *)url);

      		offset += StrLen(url) + 1;

      		error = DmStrCopy(s, offset, (Char *)g->contentType);

      		offset += StrLen(g->contentType) + 1;

		StrIToA(size, webData->length);

      		error = DmStrCopy(s, offset, (Char *)size);

      		offset += StrLen(size) + 1;

      		//error = DmStrCopy(s, offset, (Char *)webData->data);
		error = DmWrite(s, offset, webData->data, webData->length);


      		MemHandleUnlock(currec);

		handled = true;
    	} else {
	        ErrDisplay ("***MemHandleResize Failed");
	}

	error = DmReleaseRecord(g->cache.DbCache, 0, true);
	
cache_add_cleanup:

	g->cache.cacheDoNotRead = false;

	return handled;
}

Boolean CacheLoadFileFromCache(WebDataPtr webData, Char *url, Boolean remember, Boolean isImage, GlobalsType *g)
{
	UInt16            	nrRecs, i, pos;
	//UInt32			j = 0, k = 0;
	MemHandle        	cacheRec;
	Char			*pcache;
	Char			size[200];
	Boolean			loaded = false;
	Boolean			private = false;
	UInt16			urlLen = StrLen(url);

	/*
	if (g->privateBrowsing == true)
		return false;
	*/
	if (g->cache.cacheDoNotRead == true)
		return false;

	urlLen = CacheUrlLen(url);

	WspFreePdu(webData);

	nrRecs = DmNumRecordsInCategory(g->cache.DbCache, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
      		cacheRec = DmQueryNextInCategory(g->cache.DbCache, &pos, dmAllCategories);
      		pcache = MemHandleLock(cacheRec);

      		if (!StrNCompare(url, pcache + 1, urlLen) && (CacheUrlLen(pcache + 1) == urlLen)) {
			webData->transaction_id = 0x01;
  			webData->pdu_type = WSP_PDU_REPLY;

			private = (Boolean)pcache[0];
			pcache++;

			if (private != g->privateBrowsing)
				goto cache_read_bottom;

  			pcache += StrLen(pcache) + 1; // passover url


			if (g->contentType)
				Free(g->contentType);
			g->contentType = Malloc(StrLen(pcache)+1);
			if (!g->contentType)
				goto cache_read_cleanup;

  			StrCopy(g->contentType, pcache);
  			pcache += StrLen(pcache) + 1;

			if (g->contentType && (g->isImage != true)) {
				if (g->pageContentType) Free(g->pageContentType);
				g->pageContentType = Malloc(StrLen(g->contentType)+1);
					if (!g->pageContentType) return false; // ErrFatalDisplayIf (!g->pageContentType, "WspDecode: Malloc Failed");
	    			StrCopy(g->pageContentType, g->contentType);
			}

			StrCopy(size, pcache);
  			pcache += StrLen(pcache) + 1;

			webData->length = StrAToI(size);
			if ( g->isImage != true ) g->contentLength = webData->length;

			webData->data = Malloc(webData->length+1);
			MemMove(webData->data, pcache, webData->length);
			//for (j=0,k=0;j<(webData->length);j++)
    			//		wsppdu->data[j] = pcache[k++];

			if (((StrNCaselessCompare(g->contentType,"text/xml",8)==0) ||
	    	     	     (StrNCaselessCompare(g->contentType,"text/plain",10)==0)) &&
	     	      	      StrStr(webData->data, "<rss") && (g->isImage != true)) {
				Free(g->contentType);
				g->contentType = Malloc(StrLen("application/rss+xml")+1);
					if (!g->contentType) return false;
				StrCopy(g->contentType, "application/rss+xml");
	
				if (g->pageContentType) Free(g->pageContentType);
				g->pageContentType = Malloc(StrLen(g->contentType)+1);
					if (!g->pageContentType) return false;
	    			StrCopy(g->pageContentType, g->contentType); 
			}

			loaded = true;

			if(remember)
				StrCopy(g->WapLastUrl,url);
  			
			MemHandleUnlock(cacheRec);

			break;
        	}

cache_read_bottom:
     		MemHandleUnlock(cacheRec);

      		pos++;
    	}

	return loaded;

cache_read_cleanup:
	MemHandleUnlock(cacheRec);

	return false;
}

/*Boolean CacheClearCache(GlobalsType *g)
{
	UInt16          cardNo;       // card containing the application database
	LocalID         dbID;         // handle for application database
	Err		error = errNone;
	Boolean		cleared = false;

	error = DmOpenDatabaseInfo(g->cache.DbCache, &dbID, NULL, NULL, &cardNo, NULL);
	if (error != errNone)
		return false;

	error = DmDeleteDatabase(cardNo, dbID);

	if (error == errNone)
		cleared = true;

	return cleared;
}*/

Boolean CacheClearCache(GlobalsType *g)
{
	UInt16          nrRecs, i, pos;
	MemHandle       cacheRec;
	Boolean		cleared = false;
	Err		error = errNone;


	nrRecs = DmNumRecordsInCategory(g->cache.DbCache, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
		nrRecs = DmNumRecordsInCategory(g->cache.DbCache, dmAllCategories);
		pos = 0; i = 0;
      		cacheRec = DmQueryNextInCategory(g->cache.DbCache, &pos, dmAllCategories);

		error = DmDeleteRecord(g->cache.DbCache, pos);
		error = DmRemoveRecord(g->cache.DbCache, pos);

      		pos++;
    	}

	if (error == errNone)
		cleared = true;

	return cleared;
}

Boolean CacheClearLastFile(GlobalsType *g)
{
	UInt16          nrRecs, pos;
	MemHandle       cacheRec;
	Boolean		cleared = false;
	Err		error = errNone;


	nrRecs = DmNumRecordsInCategory(g->cache.DbCache, dmAllCategories);
	pos = nrRecs - 1;
	
      	cacheRec = DmQueryNextInCategory(g->cache.DbCache, &pos, dmAllCategories);

	error = DmDeleteRecord(g->cache.DbCache, pos);
	error = DmRemoveRecord(g->cache.DbCache, pos);


	if (error == errNone)
		cleared = true;

	return cleared;
}

Boolean CacheClearPrivateFiles(GlobalsType *g)
{
	UInt16          nrRecs, pos;
	MemHandle       cacheRec;
	Boolean		cleared = false;
	Err		error = errNone;
	Char		*pcache;
	int		i = 0;

	nrRecs = DmNumRecordsInCategory(g->cache.DbCache, dmAllCategories);
	pos = 0;
	for (i = 0; i < nrRecs; i++) {
		cacheRec = DmQueryNextInCategory(g->cache.DbCache, &pos, dmAllCategories);
		if (!cacheRec)
			break;
		pcache = MemHandleLock(cacheRec);
		if (!pcache) {
			MemHandleUnlock(cacheRec);
			break;
		}

		if (pcache[0] == 1) {
			MemHandleUnlock(cacheRec);

			error = DmDeleteRecord(g->cache.DbCache, pos);
			error = DmRemoveRecord(g->cache.DbCache, pos);

			if (error == errNone)
				cleared = true;
		} else {
			MemHandleUnlock(cacheRec);
			pos++;
		}
	}

	return cleared;
}



