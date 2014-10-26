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
#include 	"http.h"
#include 	"build.h"
#include 	"WAPUniverse.h"
#include 	"wtp.h"
#include 	"wsp.h"
#include 	"WAP.h"
#include 	"../res/WAPUniverse_res.h"
#include 	"dbConn.h"
#include 	"dbUrl.h"
#include 	"formBrowser.h"
#include 	"PalmUtils.h"
#include 	"wml.h"
#include	"base64.h"
#include	"process.h"
#include	"Cookie.h"
#include	"debug.h"
#include	"Net.h"
#include	"Error.h"
#include	"Pointer.h"
#include	"Browser.h"
#include	"BrowserAPI.h"
#include	"Layout.h"
#include	"palmpng.h"
#include	"DOM.h"


UInt16 DownloadAlert (char* file, char* contentType)
{
UInt16 tapped;

    if (!file || (file == NULL) || (StrLen(file) < 1))
	tapped = FrmCustomAlert (alDownload, "unknown", contentType, NULL);
    else
    	tapped = FrmCustomAlert (alDownload, file, contentType, NULL);

    return tapped;
}

static Err PrvSendDB(const void *pData, UInt32 *pSize, void *pSocket)
{
	Err	 err;
	
	*pSize = ExgSend((ExgSocketType *)pSocket, (void *)pData, *pSize, &err);
	
	return err;
}


Err DownloadFileDatabase(LocalID dbID, Char *pName, Char *pDescription, GlobalsType *g)
{
	ExgSocketType	sock;
	Err		err = 0;
	Char 		errStr[5];
	
	// Clear the exchange socket. All unused fields MUST be set to zero.
	MemSet(&sock, sizeof(ExgSocketType), 0);
	
	// Set some of the fields (the minimum)
	sock.name = pName;				// file-name.pdb
	sock.description = pDescription;		// user description
	sock.type = g->contentType;			// content-type
	
	// Send the data. ExgDBWrite() calls the callback routine until all is sent.
	err = ExgPut(&sock);
	err = ExgDBWrite(PrvSendDB, (void *)&sock, NULL, dbID, 0);
	err = ExgDisconnect(&sock, err);

	if (err) {
		StrPrintF(errStr, "%4x", err);
		MyErrorFunc("\nCould not download file\nReason:\n", errStr);
       	}
	
	return err;
}

Err DownloadFile(Char *filename, WebDataPtr webData, GlobalsType *g)
{
    ExgSocketType exgSocket;
    Err err = 0;
    Char errStr[5];

    

    // Construct the exchange socket
    // It's important to initialize the structure to zeros
    MemSet(&exgSocket, sizeof(exgSocket), 0);


    // Set the user-friendly description of the data
    exgSocket.description = filename;

    // Specify local mode to exchange only with the local device
    exgSocket.localMode = 1;

    //Specify the MIME type
    exgSocket.type = g->contentType;

    // Use the filename as the socket name. The Exchange manager
    // determines the type of data from the file extension.
    if (!filename || (filename == NULL) || (StrLen(filename) < 1))
	exgSocket.name = "unknown";
    else
    	exgSocket.name = filename;

    //Do not let the receiving application launch and
    //go to the received data.
    exgSocket.noGoTo = 1;

    // Initiate the data transfer
    err = ExgPut(&exgSocket);

    // Send the data
    if (!err) {

            // Send all the bytes that were read
            ExgSend(&exgSocket, webData->data, webData->length, &err);
            if (err) {
                StrPrintF(errStr, "%4x", err);
                MyErrorFunc("\nCould not download file\nReason:\n", errStr);
                return err;
            }

        // Terminate the exchange manager transfer
        ExgDisconnect(&exgSocket, err);
    }


    // Report any errors
       if (err) {
           StrPrintF(errStr, "%4x", err);
           MyErrorFunc("\nCould not download file\nReason:\n", errStr);
       }
    return err;
}

