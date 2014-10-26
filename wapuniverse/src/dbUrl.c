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
#include    <PalmOS.h>
#include    <SysEvtMgr.h>
#include    "WAPUniverse.h"
#include    "../res/WAPUniverse_res.h"
#include    "dbUrl.h"
#include    "PalmUtils.h"

Boolean dbUrlCreateRecord (GlobalsType *g, UInt16 *recIndex)
{
  MemPtr          p;
  MemHandle       urlRec;
  UInt16          index = 0;
  Err             error;
  UInt16          attr;         // record attributes (category)

  // Create a new first record in the database
  urlRec = DmNewRecord (g->DbUrlDb, &index, dbUrlNewURLSize);

  // Lock down the block containing the new record.
  p = MemHandleLock (urlRec);

  // Write a zero to the first byte of the record to null terminate the new
  // URL string.
  error = DmSet (p, 0, dbUrlNewURLSize, (UInt8) 0);

  // Check for fatal error.
  //ErrFatalDisplayIf (error, "Could not write to new record.");
  if (error) {
	MemHandleUnlock(urlRec);
	return (false);
  }

  // Unlock the block of the new record.
  MemPtrUnlock (p);


  // P11. Set the category of the new record to the current category.
  DmRecordInfo (g->DbUrlDb, index, &attr, NULL, NULL);
  attr &= ~dmRecAttrCategoryMask; // Remove all category bits
  if (g->DbUrlCurrentCategory == dmAllCategories) {
      // Since the user isn't looking in any particular category place
      // the new record in the unfiled category.
      attr |= dmUnfiledCategory;
  } else {
      // Set the attribute bits to indicate the current category.
      attr |= g->DbUrlCurrentCategory;
  }
  DmSetRecordInfo (g->DbUrlDb, index, &attr, NULL);


  // Release the record to the database manager.  The true value indicates
  // that
  // the record contains "dirty" data.  Release Record will set the record's 
  // dirty flag and update the database modification count.
  DmReleaseRecord (g->DbUrlDb, index, true);

  *recIndex = index;

  // Remember the index of the current record.
  g->DbUrlCurrentRecord = 0;
  return (true);
}

/************************************************************
 *  FUNCTION: PackURL
 *  DESCRIPTION: Packs a URL to consume less space in the DB
 *  PARAMETERS: the unpacked URL (url), and the destination DB handle
 *  RETURNS: no nothing, but it wouldn't hurt todo so ... 
 *************************************************************/
void dbUrlPackURL (dbUrlURL * url, MemHandle urlDBEntry)
{
  // figure out necessary size
  UInt16            length = 0;
  Char              *s;
  UInt16            offset = 0;

  length = StrLen (url->name) + StrLen (url->urlstr)
            + StrLen (url->connection) + 3; // 3 for 3 string terminators

  // resize the MemHandle
  if (MemHandleResize (urlDBEntry, length) == 0)
    {
      // copy the fields
      s = MemHandleLock (urlDBEntry);
      offset = 0;
      DmStrCopy (s, offset, (Char *) url->name);
      offset += StrLen (url->name) + 1;
      DmStrCopy (s, offset, (Char *) url->urlstr);
      offset += StrLen (url->urlstr) + 1;
      DmStrCopy (s, offset, (Char *) url->connection);
      MemHandleUnlock (urlDBEntry);
    }
	else
	{
	        ErrDisplay ("***MemHandleResize Failed");
	}
}

/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
void dbUrlUnPackURL (dbUrlURL * url, dbUrlPackedURL * packedURL)
{
  char*           s = packedURL->name;

  StrCopy (url->name, s);
  s += StrLen (s) + 1;
  StrCopy (url->urlstr, s);
  s += StrLen (s) + 1;
  StrCopy (url->connection, s);

}


/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
void dbUrlOpenOrCreateDB(UInt16 mode, GlobalsType *g)
{
  	Int16		error;        // error code
  	UInt16		cardNo;       // card containing the application database
  	LocalID		dbID;         // handle for application database
  	UInt16		dbAttrs;      // database attributes
  	MemHandle	currec;
  	UInt16		recIndex = 0;
	Boolean		created = false;

	// Find the application's URL database.
  	g->DbUrlDb = DmOpenDatabaseByTypeCreator (dbUrlDBType, dbUrlDBType/*wuAppType*/, mode);
  	if (!g->DbUrlDb) {
      		// The database doesn't exist, create it now.
      		error = DmCreateDatabase (0, dbUrlDBName, dbUrlDBType/*wuAppType*/,
                                dbUrlDBType, false);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error, "Could not create new DB.");

      		// Find the application's database.
      		g->DbUrlDb = DmOpenDatabaseByTypeCreator (dbUrlDBType, dbUrlDBType/*wuAppType*/, mode);

      		// Get info about the database
      		error = DmOpenDatabaseInfo (g->DbUrlDb, &dbID, NULL, NULL, &cardNo, NULL);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error, "Could not get DB info.");

      		// Get attributes for the database
      		error = DmDatabaseInfo (0, dbID, NULL, &dbAttrs, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error, "Could not get DB info.");

      		// Set the new attributes in the database
      		error = DmSetDatabaseInfo (0, dbID, NULL, &dbAttrs, NULL, NULL,
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

      		// Check for fatal error.
      		ErrFatalDisplayIf (error, "Could not set DB info.");

      		// Store category info in the application's information block.
      		dbUrlInitAppInfo (g->DbUrlDb,g);

      		// Create a default record for "Channel: WAPUniverse"
      		/*created = dbUrlCreateRecord(g, &recIndex);
		if (created != true) {
			goto dburl_bottom;
		}
      		StrCopy (g->DbUrlCurrentURL.name, "Channel: WAPUniverse");
      		StrCopy (g->DbUrlCurrentURL.urlstr, "mtch:WAPUniverse/index.html");
      		StrCopy (g->DbUrlCurrentURL.connection, "- Use Active -");
      		currec = DmGetRecord (g->DbUrlDb, 0);
      		dbUrlPackURL (&g->DbUrlCurrentURL, currec);
      		DmReleaseRecord (g->DbUrlDb, 0, true);

      		dbUrlSortRecord(g->DbUrlDb, &recIndex);

      		// Create a default record for "Universe Home"
      		created = dbUrlCreateRecord(g, &recIndex);
		if (created != true) {
			goto dburl_bottom;
		}
      		StrCopy (g->DbUrlCurrentURL.name, "Universe Home");
      		StrCopy (g->DbUrlCurrentURL.urlstr, "http://wap2.wapuniverse.com/");
      		StrCopy (g->DbUrlCurrentURL.connection, "- Use Active -");
      		currec = DmGetRecord (g->DbUrlDb, 0);
      		dbUrlPackURL (&g->DbUrlCurrentURL, currec);
      		DmReleaseRecord (g->DbUrlDb, 0, true);

      		dbUrlSortRecord(g->DbUrlDb, &recIndex);*/
      
      		// Create a default record for "Google"

      		created = dbUrlCreateRecord(g, &recIndex);
            if (created != true) {
                goto dburl_bottom;
            }

      		StrCopy (g->DbUrlCurrentURL.name, "Google");
      		StrCopy (g->DbUrlCurrentURL.urlstr, "http://www.google.com/xhtml");
      		StrCopy (g->DbUrlCurrentURL.connection, "- Use Active -");

      		currec = DmGetRecord (g->DbUrlDb, 0);

      		dbUrlPackURL (&g->DbUrlCurrentURL, currec);
      		DmReleaseRecord (g->DbUrlDb, 0, true);

      		dbUrlSortRecord(g->DbUrlDb, &recIndex);
            
            NVFSSyncDatabase(g->DbUrlDb);

dburl_bottom:
    	}

  	// Set current database record to no current record.
  	g->DbUrlCurrentRecord = noRecordSelected;

  	// Get the name of the current category from the app info block.
  	// The app info block was created and initialized by MemoPadInitAppInfo.
  	// CategoryCreateList(g->DbUrlDb,CategoryName,g->DbUrlCurrentCategory,true,true,1,0,true);

    // first check to make sure that our db is not corrupted!!!
    if (!dbUrlCheckAppInfo(g->DbUrlDb, g)) {
        MyErrorFunc(urlDbCorruptWarning, NULL);
        dbUrlInitAppInfo(g->DbUrlDb, g); // corrupted!!!!
    }
    
  	CategoryGetName (g->DbUrlDb, g->DbUrlCurrentCategory, g->DbUrlCategoryName);

}


/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
static Int16 dbUrlCompareRecords(void *u1, void *u2) //dbUrlURLPtr u1, dbUrlURLPtr u2)
{
	Int16		result = 0;
	dbUrlURL	rec1;
	dbUrlURL	rec2;

	dbUrlUnPackURL(&rec1, u1);
	dbUrlUnPackURL(&rec2, u2);

	result = StrCompare(rec1.name, rec2.name);

	return result;
}

void dbUrlReadCurrentUrl (GlobalsType *g)
{
  dbUrlPackedURL*	purl;
  MemHandle        	rechand;
  Int16             seekAmount = g->DbUrlCurrentRecord;
  UInt16            index = 0;

  // must do seek to skip over secret records
  DmSeekRecordInCategory (g->DbUrlDb, &index, seekAmount, dmSeekForward, g->DbUrlCurrentCategory);
  rechand = DmQueryRecord (g->DbUrlDb, index);
  if (rechand)
    {
      purl = MemHandleLock (rechand);
      dbUrlUnPackURL (&(g->DbUrlCurrentURL), purl);
      MemHandleUnlock (rechand);
    }

}

/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
Err dbUrlSortRecord (DmOpenRef dbP, UInt16 *indexP)
{
	Err		err;
	UInt16		index;
	UInt16		attributes;
	UInt32		uniqueID;
	MemHandle	recordH;
 	MemHandle	h;
	Boolean		dontMove;
	dbUrlURLPtr	cmp;
	dbUrlURLPtr	recordP;

	// Check if the record is already in the correct position.
	recordP = MemHandleLock(DmQueryRecord (dbP, *indexP));
	if (*indexP > 0) {
		// This record wasn't deleted and deleted records are at the end of the
		// database so the prior record may not be deleted!
		h = DmQueryRecord (dbP, *indexP - 1);
		if (!h) {
			dontMove = false;
		} else {
			cmp = MemHandleLock (h);
			dontMove = (dbUrlCompareRecords(cmp, recordP) == -1);
			MemPtrUnlock(cmp);
		}
	} else {
		dontMove = true;
	}

	if (dontMove && (*indexP + 1 < DmNumRecords (dbP))) {
		DmRecordInfo (dbP, *indexP + 1, &attributes, NULL, NULL);
		if (!(attributes & dmRecAttrDelete)) {
			cmp = MemHandleLock (DmQueryRecord (dbP, *indexP + 1));
			dontMove &= (dbUrlCompareRecords (recordP, cmp) == -1);
			MemPtrUnlock (cmp);
		}
	}
	MemPtrUnlock (recordP);

	if (dontMove)
		return (0);


	// Since the routine that determines the records sort position uses a 
	// binary search algorythm we need to remove the record from the database 
	// before we can determine its new position.  We will also save and restore 
	// the 
	// record's attributes and unique ID.
	DmRecordInfo (dbP, *indexP, &attributes, &uniqueID, NULL);

	err = DmDetachRecord (dbP, *indexP, &recordH);
	if (err)
		return (err);

	recordP = MemHandleLock ((MemHandle) recordH);
	index = DmFindSortPosition (dbP, recordP, NULL, (DmComparF *)
                              dbUrlCompareRecords, 0);
	MemPtrUnlock (recordP);

	err = DmAttachRecord (dbP, &index, recordH, 0);
	if (err)
		return (err);

	DmSetRecordInfo (dbP, index, &attributes, &uniqueID);

	*indexP = index;

	return (err);
}

UInt16 dbUrlNumRecords(GlobalsType *g)
{
  return(DmNumRecordsInCategory (g->DbUrlDb, g->DbUrlCurrentCategory));
}

Boolean dbUrlDeleteCurrentRecord (GlobalsType *g)
{
  Int16             seekAmount = g->DbUrlCurrentRecord;
  UInt16            index = 0;

  DmSeekRecordInCategory (g->DbUrlDb, &index, seekAmount, dmSeekForward, g->DbUrlCurrentCategory);
  DmRemoveRecord (g->DbUrlDb, index);
  return (true);
}


Boolean dbUrlCheckAppInfo(DmOpenRef dbP, GlobalsType *g)
{
	UInt16		cardNo;
	LocalID		dbID;
	LocalID		appInfoID;
    
    if (DmOpenDatabaseInfo (dbP, &dbID, NULL, NULL, &cardNo, NULL))
        return false; // bad?
    if (DmDatabaseInfo (cardNo, dbID, NULL, NULL, NULL, NULL, NULL,
                      NULL, NULL, &appInfoID, NULL, NULL, NULL))
        return false; // bad?
        
    return (appInfoID != NULL);
}

/************************************************************
 *  FUNCTION: gWAPInitAppInfo
 *  DESCRIPTION: Create an app info chunk if missing.  Set
 *      the category strings to a default.
 *  PARAMETERS: database poInt16er
 *  RETURNS: 0 if successful, errorcode if not
 *************************************************************/
Err dbUrlInitAppInfo (DmOpenRef dbP, GlobalsType *g)
{
	UInt16		cardNo;
	MemHandle	h;
	LocalID		dbID;
	LocalID		appInfoID;
	gWAPAppInfoPtr	appInfoP;
	gWAPAppInfoPtr	nilP = 0;


  // We have a DmOpenRef and we want the database's app info block ID.
  // Get the database's dbID and cardNo and then use them to get the
  // appInfoID.
  if (DmOpenDatabaseInfo (dbP, &dbID, NULL, NULL, &cardNo, NULL))
    return dmErrInvalidParam;
  if (DmDatabaseInfo (cardNo, dbID, NULL, NULL, NULL, NULL, NULL,
                      NULL, NULL, &appInfoID, NULL, NULL, NULL))
    return dmErrInvalidParam;


  // If no appInfoID exists then we must create a new one.    
  if (appInfoID == NULL)
    {
      h = DmNewHandle (dbP, sizeof (gWAPAppInfoType));
      if (!h)
        return dmErrMemError;

      appInfoID = MemHandleToLocalID (h);
      DmSetDatabaseInfo (cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL,
                         NULL, &appInfoID, NULL, NULL, NULL);
    }


  // Lock the appInfoID and copy in defaults from our default structure.  
  appInfoP = MemLocalIDToLockedPtr (appInfoID, cardNo);

  // Clear the app info block.
  // DmSet (appInfoP, 0, sizeof (gWAPAppInfoType), 0);
  DmWrite (appInfoP, 0, &(g->WAPAppInfo), sizeof (gWAPAppInfoType));

  // Initialize the categories.
  CategoryInitialize ((AppInfoPtr) appInfoP, localizedAppInfoStr);

  // Initialize reserved2.  This is actually redundant since the DmSet
  // call above has already set reserved2 to zero.  But this shows how
  // to set variables in addition to the categories.
  DmSet (appInfoP, (UInt32) & nilP->reserved2, sizeof
         (appInfoP->reserved2), 0);

  MemPtrUnlock (appInfoP);
  return 0;
}


