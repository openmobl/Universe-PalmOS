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
#include    "dbConn.h"
#include    "PalmUtils.h"

Boolean dbConnCreateRecord (GlobalsType *g)
{
  MemPtr            p;
  MemHandle        	connRec;
  UInt16            index = 0;
  Err            	error;
  UInt16            attr;         // record attributes (category)

  // Create a new first record in the database
  connRec = DmNewRecord (g->DbConnDb, &index, dbConnNewConnSize);
  // Lock down the block containing the new record.
  p = MemHandleLock (connRec);
  // Write a zero to the first byte of the record to null terminate the new
  // memo string.
  error = DmSet (p, 0, dbConnNewConnSize, (UInt8) 0);

  // Check for fatal error.
  ErrFatalDisplayIf (error, "Could not write to new record.");

  // Unlock the block of the new record.
  MemPtrUnlock (p);


  // P11. Set the category of the new record to the current category.
  DmRecordInfo (g->DbConnDb, index, &attr, NULL, NULL);
  attr &= ~dmRecAttrCategoryMask; // Remove all category bits
  // no category support for connection settings (yet?), place
  // the new record in the unfiled category.
  attr |= dmUnfiledCategory;
  DmSetRecordInfo (g->DbConnDb, index, &attr, NULL);


  // Release the record to the database manager.  The true value indicates
  // that
  // the record contains "dirty" data.  Release Record will set the record's 
  // dirty flag and update the database modification count.
  DmReleaseRecord (g->DbConnDb, index, true);

  // Remember the index of the current record.
  g->DbConnCurrentRecord = 0;
  return (true);
}

/************************************************************
 *  FUNCTION: PackConnection
 *  DESCRIPTION: Packs a dbConnConnection to consume less space in the DB
 *  PARAMETERS: the unpacked dbConnConnection (conn), and the destination DB handle
 *  RETURNS: no nothing, but should probably 
 *************************************************************/
void dbConnPackConnection (dbConnConnection * conn, MemHandle connDBEntry)
{
  // figure out necessary size
  UInt16            length = 0;
  Char *         s;
  UInt16            offset = 0;

  length = StrLen (conn->name) +
    StrLen (conn->ipaddress) +
    StrLen (conn->home) +
    StrLen (conn->port) +
    StrLen (conn->proxyuser) +
    StrLen (conn->proxypass) +
    3 +                         // 1 for bearer, 1 for connection type, 1 for lock
    6;                          // 3 for 3 string terminators

  // resize the MemHandle
  if (MemHandleResize (connDBEntry, length) == 0)
    {
      // copy the fields
      s = MemHandleLock (connDBEntry);
      offset = 0;
      DmStrCopy (s, offset, (Char *) conn->name);
      offset += StrLen (conn->name) + 1;
      DmStrCopy (s, offset, (Char *) conn->ipaddress);
      offset += StrLen (conn->ipaddress) + 1;
      DmStrCopy (s, offset, (Char *) conn->home);
      offset += StrLen (conn->home) + 1;
      DmStrCopy (s, offset, (Char *) conn->port);
      offset += StrLen (conn->port) + 1;
      DmStrCopy (s, offset, (Char *) conn->proxyuser);
      offset += StrLen (conn->proxyuser) + 1;
      DmStrCopy (s, offset, (Char *) conn->proxypass);
      offset += StrLen (conn->proxypass) + 1;
      DmWrite (s, offset, (Char *) &conn->bearer,1);
      offset++;
      DmWrite (s, offset, (Char *) &conn->connectionType,1);
      offset++;
      DmWrite (s, offset, (Char *) &conn->lock,1);
      MemHandleUnlock (connDBEntry);
    }
	else
	{
        ErrDisplay ("***MemHandleResize failed");	
	}
}

/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
void dbConnUnPackConnection (dbConnConnection * conn, dbConnPackedConnection * packedConnection)
{
  char*           s = packedConnection->name;

  StrCopy (conn->name, s);
  s += StrLen (s) + 1;
  StrCopy (conn->ipaddress, s);
  s += StrLen (s) + 1;
  StrCopy (conn->home, s);
  s += StrLen (s) + 1;
  StrCopy (conn->port, s);
  s += StrLen (s) + 1;
  StrCopy (conn->proxyuser, s);
  s += StrLen (s) + 1;
  StrCopy (conn->proxypass, s);
  s += StrLen (s) + 1;
  conn->bearer = *s;
  s++;
  conn->connectionType = *s;
  s++;
  conn->lock = *s;

}

/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
void dbConnDisposeConnectionsList(GlobalsType *g)
{
    if (g->ConnectionsListArrayH) {
        MemHandleFree(g->ConnectionsListArrayH);
        g->ConnectionsListArrayH=NULL;
    }
    if (g->ConnectionsHandle) {
      MemHandleUnlock(g->ConnectionsHandle);
      MemHandleFree(g->ConnectionsHandle);
      g->ConnectionsHandle=NULL;
    }
}

/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
void dbConnGetConnection(char *connstr, dbConnConnection *conn, GlobalsType *g)
{
  UInt16            nrRecs, i, pos;
  MemHandle        connRec;
  dbConnPackedConnection* pconn;

  nrRecs = DmNumRecordsInCategory (g->DbConnDb, dmAllCategories);
  pos = 0;
  for (i = 0; i < nrRecs; i++)
    {
      connRec = DmQueryNextInCategory (g->DbConnDb, &pos, dmAllCategories);
      pconn = MemHandleLock (connRec);
      if (StrCompare((Char *)pconn,connstr) == 0)
        { 
            dbConnUnPackConnection (conn, pconn);
        }
      MemHandleUnlock (connRec);
      pos++;
    }
}

/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
void dbConnGetConnectionsList (GlobalsType *g)
{
  ListPtr         connectionslstPtr;
  // dbConnConnectionPtr connRecPtr;
  ControlPtr      ctlPtr;
  MemHandle        connRec;
  UInt16            nrRecs, nrConnections, i, pos, titlelen,
  connectionsOffset = 0;
  UInt16            j = 0;
  FormPtr         frm;
  Char*           cptr;
  Char *         connections;
  Err             error;

  frm = FrmGetFormPtr (frmEditURL);
  nrRecs = DmNumRecordsInCategory (g->DbConnDb, dmAllCategories);
  pos = 0;
  nrConnections = 0;
  g->ConnectionsHandle = MemHandleNew (sizeof (char));
  connections = MemHandleLock (g->ConnectionsHandle);
  *connections = 0;

  if (FrmGetActiveFormID() == frmEditURL) {
	MemHandleUnlock (g->ConnectionsHandle);
	error = MemHandleResize (g->ConnectionsHandle, StrLen("- Use Active -") +
                               connectionsOffset + sizeof ('\0'));
	connections = MemHandleLock (g->ConnectionsHandle);
	StrCopy(connections+connectionsOffset,"- Use Active -");
	connectionsOffset += (StrLen("- Use Active -")+1);
	nrConnections++;
  }

  for (i = 0; i < nrRecs; i++)
    {
      connRec = DmQueryNextInCategory (g->DbConnDb, &pos, dmAllCategories);
      cptr = MemHandleLock (connRec);
      titlelen = StrLen (cptr);
      MemHandleUnlock (g->ConnectionsHandle);
      error = MemHandleResize (g->ConnectionsHandle, titlelen +
                               connectionsOffset + sizeof ('\0'));
      connections = MemHandleLock (g->ConnectionsHandle);
      StrCopy(connections+connectionsOffset,cptr);
      connectionsOffset += (titlelen+1);
      nrConnections++;
      MemHandleUnlock (connRec);
      pos++;
    }
  if (nrConnections == 0)
    {
      MemHandleUnlock (g->ConnectionsHandle);
      return;
    }
  g->ConnectionsListArrayH = SysFormPointerArrayToStrings (connections, nrConnections);
  connectionslstPtr = (ListPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex
                                                        (frm, listURLConnection))));
  LstSetListChoices (connectionslstPtr, MemHandleLock
                     (g->ConnectionsListArrayH), nrConnections);
  LstSetHeight (connectionslstPtr, nrConnections);
  // find current connection index
  for (i = 0; i < nrConnections; i++)
  {
      if (StrCompare(LstGetSelectionText(connectionslstPtr,i),g->DbUrlCurrentURL.connection )==0)
         j= i;
  }
  LstSetSelection(connectionslstPtr,j); // current connection 
  ctlPtr =(ControlPtr)(FrmGetObjectPtr(frm,(FrmGetObjectIndex(frm,triggerURLConnection))));
  g->EditURLConnection = LstGetSelectionText(connectionslstPtr,
                          LstGetSelection(connectionslstPtr));
  CtlSetLabel(ctlPtr,g->EditURLConnection);
   
}


/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
void dbConnOpenOrCreateDB(UInt16 mode, GlobalsType *g)
{
  Int16             error;        // error code
  UInt16            cardNo;       // card containing the application database
  LocalID         	dbID;         // handle for application database
  UInt16            dbAttrs;      // database attributes
  MemHandle        	currec;


  g->DbConnDb = DmOpenDatabaseByTypeCreator (dbConnDBType, dbConnDBType/*wuAppType*/, mode);
  if (!g->DbConnDb)
    {
      // The database doesn't exist, create it now.
      error = DmCreateDatabase (0, dbConnDBName, dbConnDBType/*wuAppType*/,
                                dbConnDBType, false);

      // Check for fatal error.
      ErrFatalDisplayIf (error,
                         "Create new ConnDB failed");

      // Find the application's database.
      g->DbConnDb = DmOpenDatabaseByTypeCreator (dbConnDBType,
                                                dbConnDBType/*wuAppType*/, mode);

      // Get info about the database
      error = DmOpenDatabaseInfo (g->DbConnDb, &dbID, NULL, NULL, &cardNo, NULL);

      // Check for fatal error.
      ErrFatalDisplayIf (error,
                         "Get ConnDB info failed");

      // Get attributes for the database
      error = DmDatabaseInfo (0, dbID, NULL, &dbAttrs, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL);

      // Check for fatal error.
      ErrFatalDisplayIf (error,
                         "Get ConnDB info failed");

      // Set the new attributes in the database
      error = DmSetDatabaseInfo (0, dbID, NULL, &dbAttrs, NULL, NULL,
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

      // Check for fatal error.
      ErrFatalDisplayIf (error,
                         "Set ConnDB info failed");

      // Store category info in the application's information block.
      dbUrlInitAppInfo (g->DbConnDb,g);

      // Create a default record for "Direct"
      dbConnCreateRecord (g);
      StrCopy (g->CurrentConnection.name, "-Direct-");
      StrCopy (g->CurrentConnection.ipaddress, "0.0.0.0");
      StrCopy (g->CurrentConnection.home, "http://wap2.wapuniverse.com/");
      StrCopy (g->CurrentConnection.port, "80");
      StrCopy (g->CurrentConnection.proxyuser, "");
      StrCopy (g->CurrentConnection.proxypass, "");
      g->CurrentConnection.bearer = 'C'; // 'C'SD
      g->CurrentConnection.connectionType = 'd'; // C'L' or C'O' or 'd' for direct
      g->CurrentConnection.lock = false;
      currec = DmGetRecord (g->DbConnDb, 0);
      dbConnPackConnection (&g->CurrentConnection, currec);
      DmReleaseRecord (g->DbConnDb, 0, true);

      // Create a default record for "Openwave"
      /*dbConnCreateRecord (g);
      StrCopy (g->CurrentConnection.name, "Openwave");
      StrCopy (g->CurrentConnection.ipaddress, "12.25.203.11");
      StrCopy (g->CurrentConnection.home, "http://wap.wapuniverse.com/");
      StrCopy (g->CurrentConnection.port, "49201");
      StrCopy (g->CurrentConnection.proxyuser, "");
      StrCopy (g->CurrentConnection.proxypass, "");
      g->CurrentConnection.bearer = 'C'; // 'C'SD
      g->CurrentConnection.connectionType = 'O'; // C'L' or C'O'
      g->CurrentConnection.lock = false;
      currec = DmGetRecord (g->DbConnDb, 0);
      dbConnPackConnection (&g->CurrentConnection, currec);
      DmReleaseRecord (g->DbConnDb, 0, true);*/
      
      NVFSSyncDatabase(g->DbConnDb);

    }

  // Set current database record to no current record.
  g->DbConnCurrentRecord = noRecordSelected;
}


/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
static Int16 dbConnCompareRecords(void *c1, void *c2) //dbConnConnectionPtr c1, dbConnConnectionPtr c2)
{
	Int16			result = 0;
	dbConnConnection	rec1;
	dbConnConnection	rec2;

	dbConnUnPackConnection(&rec1, c1);
	dbConnUnPackConnection(&rec2, c2);

	result = StrCompare(rec1.name, rec2.name);

	return result;
}

void dbConnReadCurrentConnection (GlobalsType *g)
{
  dbConnPackedConnection* pconn;
  MemHandle        rechand;
  Int16             seekAmount = g->DbConnCurrentRecord;
  UInt16            index = 0;

  // must do seek to skip over secret records
  DmSeekRecordInCategory (g->DbConnDb, &index, seekAmount, dmSeekForward, dmAllCategories);
  rechand = DmQueryRecord (g->DbConnDb, index);
  if (rechand)
    {
      pconn = MemHandleLock (rechand);
      dbConnUnPackConnection (&(g->CurrentConnection), pconn);
      MemHandleUnlock (rechand);
    }

}

/************************************************************
 *  FUNCTION: 
 *  DESCRIPTION: 
 *  PARAMETERS: 
 *  RETURNS: 
 *************************************************************/
Err dbConnSortRecord (DmOpenRef dbP, UInt16 *indexP)
{
  Err             		err;
  UInt16            	index;
  UInt16            	attributes;
  UInt32           		uniqueID;
  MemHandle          	recordH;
  MemHandle        		h;
  Boolean         		dontMove;
  dbConnConnectionPtr	cmp;
  dbConnConnectionPtr   recordP;

  // Check if the record is already in the correct position.
  recordP = MemHandleLock (DmQueryRecord (dbP, *indexP));
  if (*indexP > 0)
    {
      // This record wasn't deleted and deleted records are at the end of the
      // database so the prior record may not be deleted!
      h = DmQueryRecord (dbP, *indexP - 1);
      if (!h)
        dontMove = false;
      else
        {
          cmp = MemHandleLock (h);
          dontMove = (dbConnCompareRecords (cmp, recordP) == -1);
          MemPtrUnlock (cmp);
        }
    }
  else
    dontMove = true;

  if (dontMove && (*indexP + 1 < DmNumRecords (dbP)))
    {
      DmRecordInfo (dbP, *indexP + 1, &attributes, NULL, NULL);
      if (!(attributes & dmRecAttrDelete))
        {
          cmp = MemHandleLock (DmQueryRecord (dbP, *indexP + 1));
          dontMove &= (dbConnCompareRecords (recordP, cmp) == -1);
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
                              dbConnCompareRecords, 0);
  MemPtrUnlock (recordP);

  err = DmAttachRecord (dbP, &index, recordH, 0);
  if (err)
    return (err);

  DmSetRecordInfo (dbP, index, &attributes, &uniqueID);

  *indexP = index;

  return (err);
}

Boolean dbConnDeleteCurrentRecord (GlobalsType *g)
{
  Int16             seekAmount = g->DbConnCurrentRecord;
  UInt16            index = 0;

  DmSeekRecordInCategory (g->DbConnDb, &index, seekAmount, dmSeekForward, dmAllCategories);
  DmRemoveRecord (g->DbConnDb, index);
  return (true);
}

UInt16 dbConnNumRecords(GlobalsType *g)
{
  return(DmNumRecordsInCategory (g->DbConnDb, dmAllCategories));
}
