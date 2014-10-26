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
#include 	<PalmCompatibility.h>
#include	<VFSMgr.h>
#include 	"wsp.h"
#include 	"WAPUniverse.h"
#include 	"../res/WAPUniverse_res.h"
#include	"formBrowser.h"
#include 	"wtp.h"
#include	"variable.h"
#include	"wml.h"


void HTTPError (char* error)
{

    FrmCustomAlert (alHTTPError, error, NULL, NULL);
}

/*void SetFieldFromStr2(const Char *srcP , UInt16 fieldIndex)
{
  FieldPtr        fldP;
  FormPtr         frm = FrmGetActiveForm ();
  MemHandle       txtH;
  MemHandle       oldTxtH;

  fldP = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, fieldIndex));
  ErrNonFatalDisplayIf (!fldP, "missing field");
  oldTxtH = (MemHandle) FldGetTextHandle (fldP);
  // get some space in which to stash the string
  txtH = MemHandleNew (StrLen (srcP) + 1);
  ErrFatalDisplayIf (!txtH, "MemHandleNew Failed");
  StrCopy (MemHandleLock (txtH), srcP);
  // unlock the string MemHandle 
  MemHandleUnlock (txtH);
  FldSetTextHandle (fldP, (MemHandle) txtH);
  FldDrawField (fldP);
  // Free the MemHandle AFTER we call FldSetTextHandle.
  if (oldTxtH)
    MemHandleFree (oldTxtH);

}*/

static char *wspStatusCodeStr[] = { 
	"300: Multiple choices", // 0x30
	"301: Moved permanently", // 0x31
	"302: Moved temporarily", // 0x32
        "303: See other", // 0x33
        "304: Not modified", // 0x34
        "305: Use proxy", // 0x35
        "-", // 0x36
        "307: Temporary Redirect", // 0x37
        "-", // 0x38
        "-", // 0x39
        "-", // 0x3A
        "-", // 0x3B
        "-", // 0x3C
        "-", // 0x3D
        "-", // 0x3E
        "-", // 0x3F
        "400: Bad request", // 0x40
        "401: Unauthorized", // 0x41
        "402: Payment required", // 0x42
        "403: Forbidden", // 0x43
        "404: Not found", //0x44
        "405: Method not Allowed", //0x45
        "406: Not acceptable", //0x46
        "407: Proxy authentication required", //0x47
        "408: Request timeout", //0x48
        "409: Conflict", //0x49
        "410: Gone", //0x4A
        "411: Length required", //0x4B
        "412: Precondition failed", //0x4C
        "413: Request entity too large", //0x4D
        "414: Request-URI too large", // 0X4E
        "415: Unsupported media type", // 0x4F
        "416: Request Range Not Satisfiable", // 0x50
        "417: Expectation Failed", // 0x51
        "-", // 0x52
        "-", // 0x53
        "-", // 0x54
        "-", // 0x55
        "-", // 0x56
        "-", // 0x37
        "-", // 0x58
        "-", // 0x59
        "-", // 0x5A
        "-", // 0x5B
        "-", // 0x5C
        "-", // 0x5D
        "-", // 0x5E
        "-", // 0x5F
        "500: Internal server error", // 0x60
        "501: Not implemented", // 0x61
        "502: Bad Gateway", // 0x62
        "503: Service unavailable", // 0x63
        "504: Gateway Timeout", // 0x64
        "505: HTTP version not supported" // 0x65
};  


static char *wspContentTypeStr[] = {
	"*/*", 						//  0x00
	"text/*", 					//  0x01
	"text/html", 					//  0x02
	"text/plain", 					//  0x03
	"text/x-hdml", 					//  0x04
	"text/x-ttml", 					//  0x05
	"text/x-vCalendar", 				//  0x06
	"text/x-vCard", 				//  0x07
	"text/vnd.wap.wml", 				//  0x08
	"text/vnd.wap.wmlscript", 			//  0x09
	"application/vnd.wap.catc", 			//  0x0A
	"Multipart/*", 					//  0x0B
	"Multipart/mixed", 				//  0x0C
	"Multipart/form-data", 				//  0x0D
	"Multipart/byteranges", 			//  0x0E
	"multipart/alternative", 			//  0x0F
	"application/*", 				//  0x10
	"application/java-vm", 				//  0x11
	"application/x-www-form-urlencoded", 		//  0x12
	"application/x-hdmlc", 				//  0x13
	"application/vnd.wap.wmlc", 			//  0x14
	"application/vnd.wap.wmlscriptc", 		//  0x15
	"application/vnd.wap.wsic", 			//  0x16
	"application/vnd.wap.uaprof", 			//  0x17
	"application/vnd.wap.wtls-ca-certificate", 	//  0x18
	"application/vnd.wap.wtls-user-certificate", 	//  0x19
	"application/x-x509-ca-cert", 			//  0x1A
	"application/x-x509-user-cert", 		//  0x1B
	"image/*", 					//  0x1C
	"image/gif", 					//  0x1D
	"image/jpeg", 					//  0x1E
	"image/tiff", 					//  0x1F
	"image/png",					//  0x20
	"image/vnd.wap.wbmp", 				//  0x21
	"application/vnd.wap.multipart.*", 		//  0x22
	"application/vnd.wap.multipart.mixed", 		//  0x23
	"application/vnd.wap.multipart.form-data", 	//  0x24
	"application/vnd.wap.multipart.byteranges", 	//  0x25
	"application/vnd.wap.multipart.alternative", 	//  0x26
	"application/xml", 				//  0x27
	"text/xml", 					//  0x28
	"application/vnd.wap.wbxml", 			//  0x29
	"application/x-x968-cross-cert",		//  0x2A
	"application/x-x968-ca-cert",			//  0x2B
	"application/x-x968-user-cert",			//  0x2C
	"text/vnd.wap.si",				//  0x2D
	"application/vnd.wap.sic",			//  0x2E
	"text/vnd.wap.sl",				//  0x2F
	"application/vnd.wap.slc",			//  0x30
	"text/vnd.wap.co",				//  0x31
	"application/vnd.wap.coc",			//  0x32
	"application/vnd.wap.multipart.related",	//  0x33
	"application/vnd.wap.sia",			//  0x34
	"text/vnd.wap.connectivity-xml",		//  0x35
	"application/vnd.wap.connectivity-wbxml",	//  0x36
	"application/pkcs7-mime",			//  0x37
	"application/vnd.wap.hashed-certificate",	//  0x38
	"application/vnd.wap.signed-certificate",	//  0x39
	"application/vnd.wap.cert-response",		//  0x3A
	"application/xhtml+xml",			//  0x3B
	"application/wml+xml",				//  0x3C
	"text/css",					//  0x3D
	"application/vnd.wap.mms-message",		//  0x3E
	"application/vnd.wap.rollover-certificate",	//  0x3F
	"application/vnd.wap.locc+wbxml",		//  0x40
	"application/vnd.wap.loc+xml",			//  0x41
	"application/vnd.syncml.dm+wbxml",		//  0x42
	"application/vnd.syncml.dm+xml",		//  0x43
	"application/vnd.syncml.notification",		//  0x44
	"application/vnd.wap.xhtml+xml",		//  0x45
	"application/vnd.wv.csp.cir",			//  0x46
	"application/vnd.oma.dd+xml",			//  0x47
	"application/vnd.oma.drm.message",		//  0x48
	"application/vnd.oma.drm.content",		//  0x49
	"application/vnd.oma.drm.rights+xml",		//  0x4A
	"application/vnd.oma.drm.rights+wbxml",		//  0x4B
	"application/vnd.wv.csp+xml",			//  0x4C
	"application/vnd.wv.csp+wbxml"			//  0x4D
}; 

char *WspHostErrStr()
{
    static char msgstr[100];

    if (errno != 0)
        SysErrString(errno, msgstr, 50);
    else
        msgstr[0] = '\0';

    return(msgstr);
}
                                    
UInt32 WspReadInt(char *instr, Int32 *i)
{
	UInt32 v = 0;
	unsigned char c;
	
	do {
		c = instr[(*i)++];
		v = (v << 7)|(c & 0x7f);
	}while (( c & 0x80) !=0);
	
	return (v);
	
}

Boolean WSPGetRealm(unsigned char *realm, UInt32 i, unsigned char buf[], int sz)
{
UInt32 AuthLen, s, j=0;

	s = i;
	while ((buf[i] != 0xAD) && (i < (s + sz)))
		i++;
	if (i >= sz)
		return false;
	i++;
	AuthLen = WspReadInt(buf, &i);
	i++;
	for (j=0; j < (AuthLen - 1); j++) {
		realm[j] = buf[i++]; 
	}
	return true;
}

Boolean Authenticate (Char *realm, Char *user, Char *pass)
{
    UInt16	buttonHit;
    FormType	*form;

   
	// Display an alert to comfirm the operation.
	form = FrmInitForm(AuthForm);
 
	FldSetTextPtr (FrmGetObjectPtr (form, FrmGetObjectIndex (form, AuthRealmField)), realm);
 
	buttonHit = FrmDoDialog(form);

	if (buttonHit == AuthOkButton) {
		// Should check NULL and not length
		if ((FldGetTextPtr(FrmGetObjectPtr (form, FrmGetObjectIndex (form, AuthUserField))) == NULL) || 
			(FldGetTextPtr(FrmGetObjectPtr (form, FrmGetObjectIndex (form, AuthPasswordField))) == NULL)) {
			MyErrorFunc("User ID or Password cannot be left empty!\n","WWW-Authenticate");
		} else {
			StrCopy(user, FldGetTextPtr (FrmGetObjectPtr (form, FrmGetObjectIndex (form, AuthUserField))));
			StrCopy(pass, FldGetTextPtr (FrmGetObjectPtr (form, FrmGetObjectIndex (form, AuthPasswordField))));
		}
	}

	FrmDeleteForm(form);
	if (buttonHit == AuthCancelButton)
		return (false);
 
	return (true);
}


int WspDecode (char *instr, UInt32 inlen, WebDataPtr webData, GlobalsType *g)
{
 	UInt32 		/*j,*/i,l,c,n,e,o,s,t;
	UInt32		error = 0;
 	UInt32 		sz, hsz;
 	Boolean 	authreq;
 	unsigned char 	*realm;
 	Boolean 	wantAuth;
	Boolean		haveCharset = false;

  	i=0;
  	if (g->conn.connectionType == 'O') {
    		i=2; //need to move forward
  	}

	WspFreePdu(webData);

  	webData->data = NULL;
  	webData->transaction_id = instr[i++];
  	webData->pdu_type = instr[i++];

  	switch(webData->pdu_type) {
		case  WSP_PDU_REPLY : // reply

			webData->status_code = instr[i++];
			hsz = WspReadInt(instr,&i);

			switch (webData->status_code) {
				case WSP_STATUS_OK:
					error = 0;
					break;
				case WSP_STATUS_AUTHREQ:
					{
	  					//browserSetTitle("Authorizing"); // SetFieldFromStr2("Authorizing",fieldTitle);
	  					realm = Malloc(MAX_URL_LEN + 1);
							ErrFatalDisplayIf (!realm, "WspDecode: Malloc Failed");
	  					authreq = WSPGetRealm(realm, i, instr, hsz);
	  					if (authreq == true) {
							if (g->AuthUser != NULL){ Free(g->AuthUser); g->AuthUser = NULL; }
							if (g->AuthPass != NULL){ Free(g->AuthPass); g->AuthPass = NULL; }

							g->AuthUser = Malloc(255);
								ErrFatalDisplayIf (!g->AuthUser, "WspDecode: Malloc Failed");
							g->AuthPass = Malloc(255);
								ErrFatalDisplayIf (!g->AuthPass, "WspDecode: Malloc Failed");

							wantAuth = Authenticate(realm, g->AuthUser, g->AuthPass);

							if (realm) Free(realm);

							if (wantAuth == true) {
								return WSP_ERR_AUTHORIZE;
							} else {
								if (g->AuthUser && (g->AuthUser != NULL)) Free(g->AuthUser);
								if (g->AuthPass && (g->AuthPass != NULL)) Free(g->AuthPass);
		  						g->AuthUser = NULL;
	  							g->AuthPass = NULL;
								HTTPError("401: Unauthorized");
								return WSP_ERR_UNAUTH;
							}
	  					}
	  					if (realm) Free(realm);
	  					g->AuthUser = NULL;
	  					g->AuthPass = NULL;
	  					HTTPError("401: Unauthorized");
          					return WSP_ERR_UNAUTH;
					}
					break;
				case WSP_STATUS_ACCEPTED:
             				HTTPError("Accepted");
             				return(-webData->status_code);
					break;
				case WSP_STATUS_MOVED_TEMP:
				case WSP_STATUS_MOVED_PERM:
				case WSP_STATUS_TEMP_REDIR:
				case WSP_STATUS_SEE_OTHER:
					{
		    				if (g->isImage != true) browserSetTitle("Redirecting"); // SetFieldFromStr2("Redirecting",fieldTitle);

	            				l = i + hsz;
	            				while ((instr[i] != 0xffffff9C) && (i < l)) {
	            	   				i++;
	            				}
	            				if (i == l) {
	         	   				HTTPError("Received 302 Moved Temporarily but found no URL!\n");
	         	   				return WSP_ERR_UNHANDELED;
	            				} else {
				  			i++;
				   			n = i;
				   			while ((instr[i] != 0x00) && (i < l)) {
		         	   				i++;
		           				}
		           				if (i == l) return -1;
		           				else {
			           				e = i;
			           				l = (e+1)-(n)+1;
			           				FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
			          				if (g->urlBuf != NULL)
    					   				Free(g->urlBuf);
    				   				g->urlBuf = Malloc(/*(e+2)-(n)*/MAX_URL_LEN);
			           				if (!g->urlBuf) return WSP_ERR_MEM; // ErrFatalDisplayIf (!g->urlBuf, "WspDecode: Malloc Failed");
			           				i = n;
			           				for (c=0;c<l;c++)
    					   				g->urlBuf[c] = instr[i++];
		           				}
	            				}
	  		 			return WSP_ERR_REDIRECT;
					}
					break;
				default:
					error = webData->status_code;
					break;
			}

			// get content type
  			l = i + hsz;
  			o = l;
			n = i;
			if (instr[i] == 0x03) { i++; haveCharset = true; }
			if (instr[i] == 0x07) i++; //This denotes type, Openwave's servers send it, we don't need it so skip it.
			if (((instr[i]-0xFFFFFF00) >= 0x80) && ((instr[i]-0xFFFFFF00) <= 0xCD/*0xA9*/)){
				FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
				if (g->contentType) Free(g->contentType);
	    			g->contentType = Malloc(StrLen(wspContentTypeStr[(instr[i]-0xFFFFFF00)-0x80])+1);
					if (!g->contentType) return WSP_ERR_MEM; // ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
	    			StrCopy(g->contentType, wspContentTypeStr[(instr[i]-0xFFFFFF00)-0x80]);
			} else if ((instr[n] == 0x00) || (instr[n] >= 0x20)) {
				while ((instr[i] != 0x00) && (i < l)) {
					i++;
				}
				if (i == l) return -1;
				else {
					e = i;
					l = (e+1)-(n)+1;
					FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
					if (g->contentType) Free(g->contentType);
	    				g->contentType = Malloc(MAX_URL_LEN);
						if (!g->contentType) return WSP_ERR_MEM; // ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
					i = n;

					for (c=0;c<l;c++)
	    					g->contentType[c] = instr[i++];
			 	}
			} else {
				if (instr[n] == 0x1F) {
					i++;
					s = WspReadInt(instr,&i);
					t = i;
				} else {
					s = instr[n];
					i++;
					t = i;
				}

				e = i;
				l = (e+1)-(n)+1;
				FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);
				if (g->contentType) Free(g->contentType);
	    			g->contentType = Malloc(s + 1);
					if (!g->contentType) return WSP_ERR_MEM; // ErrFatalDisplayIf (!g->contentType, "WspDecode: Malloc Failed");
				i = t; // n;

				for (c=0;c<s;c++)
	    				g->contentType[c] = instr[i++];

				if (c < s && !haveCharset) haveCharset = true;
			}

			// TODO: check for cache control and add support

			if (haveCharset == true) {
				if (g->isImage != true) g->charSet = WspReadInt(instr,&i);
			} else {
				if (g->isImage != true) g->charSet = UTF_8;
			}

	    		i = o;

			if (g->contentType && (g->isImage != true)) {
				if (g->pageContentType) Free(g->pageContentType);
				g->pageContentType = Malloc(StrLen(g->contentType)+1);
					if (!g->pageContentType) return WSP_ERR_MEM; // ErrFatalDisplayIf (!g->pageContentType, "WspDecode: Malloc Failed");
	    			StrCopy(g->pageContentType, g->contentType);
			}

  			// Data
  			sz = inlen-i;
			if (sz > 0) {

				if ((webData->status_code != WSP_STATUS_OK) && 
                                    ((StrNCaselessCompare(g->contentType,"application/vnd.wap.xhtml+xml",29)!=0) && 
				     (StrNCaselessCompare(g->contentType,"application/xhtml+xml",21)!=0) && 
				     (StrNCaselessCompare(g->contentType,"application/wml+xml",19)!=0) &&
				     (StrNCaselessCompare(g->contentType,"text/html",9)!=0) &&
				     (StrNCaselessCompare(g->contentType,"text/vnd.wap.wml",16)!=0) &&
				     (StrNCaselessCompare(g->contentType,"application/vnd.wap.wmlc",24)!=0)))
					if (g->isImage != true) MyErrorFunc(wspStatusCodeStr[(webData->status_code)-0x30], "Invalid error document format!");

				webData->data = Malloc(sz+1); 
   					if (!webData->data) return WSP_ERR_MEM; // ErrFatalDisplayIf (!webData->data, "WspDecode: Malloc Failed");
  				//for (j=0;j<(sz);j++)
    				//	webData->data[j] = instr[i++];
				MemMove(webData->data, instr + i, sz);
  				webData->length = sz;
				if (g->isImage != true) g->contentLength = sz;
			} else {
				webData->data = NULL;
				webData->length = 0;

				if ((webData->status_code >= 0x30) && (webData->status_code <= 0x65)) {
	                  	 	if (g->isImage != true) {
	                   			HTTPError(wspStatusCodeStr[(webData->status_code)-0x30]);
	               			} else {
	                   			if (g->isImage != true)
	                   				HTTPError("Unknown server response code");
	               			}
	               			return(-webData->status_code);
	            		} else if (webData->status_code == WSP_STATUS_OK) {
					if (g->isImage != true) MyErrorFunc("No Content!", NULL);
				}
				return WSP_ERR_UNHANDELED;
			}

			if (((StrNCaselessCompare(g->contentType,"text/xml",8)==0) ||
	    	     	     (StrNCaselessCompare(g->contentType,"text/plain",10)==0)) &&
	     	      	      StrStr(webData->data, "<rss") && (g->isImage != true)) {
				Free(g->contentType);
				g->contentType = Malloc(StrLen("application/rss+xml")+1);
					if (!g->contentType) return 3;
				StrCopy(g->contentType, "application/rss+xml");
	
				if (g->pageContentType) Free(g->pageContentType);
				g->pageContentType = Malloc(StrLen(g->contentType)+1);
					if (!g->pageContentType) return 3;
	    			StrCopy(g->pageContentType, g->contentType); 
			}

			break;
		default:
			if (g->isImage != true) MyErrorFunc("Unhandled server response !!", NULL);
			return WSP_ERR_UNHANDELED;
			break;
  	}
  	return(0);
}

int WspFreePdu(WebDataPtr webData)
{
	if (webData->data != NULL)
		Free(webData->data);
	webData->data = NULL;
    return(0);
}

int WspInitPdu(WebDataPtr webData)
{
	webData->data = NULL;

	return(0);
}

void showProgress2(GlobalsType *g)
{
    BitmapPtr resP=0;

    resP=MemHandleLock(g->resBmpPrgrs[g->progress]);
    WinDrawBitmap (resP, 135, 147);//124, 148);
    MemPtrUnlock(resP);
    g->progress= ((g->progress)+1)%4;
}

UInt32 WspGetReply(Int16 sock, char *buf, UInt32 buflen, UInt32 maxTimer, GlobalsType *g)
{
    	UInt32		ret; 
    	UInt16 		t = 0, off = 0;
	Boolean		haveTPI = false;
	UInt32		msgSize = 0;
	Boolean		haveStart = false;

	AppNetTimeout = 4 * SysTicksPerSecond();

	// please don't turn me off now
	EvtResetAutoOffTimer();


	start:
for(;;) {

	//start:

	ret = recv(sock,buf,buflen,0);
	showProgress(g);

	if ((errno==netErrTimeout) && ( TimGetTicks() < maxTimer ) ) {
		return(0);
	}

	if (ret <= 0) {
    		g->progress= 0;
    		showProgress2(g);
        	g->state = BS_IDLE;
		browserSetTitle("WAPUniverse"); // SetFieldFromStr2("WAPUniverse",fieldTitle);
        	if (ret == 0) {
			if (g->isImage != true)
        			MyErrorFunc("Could not receive data from server. Connection closed by remote host.","Receive");  
			return(-1);
			break;
        	}
		if (g->isImage != true)
        		MyErrorFunc("\nCould not receive data from the server.\nReason:\n",WspHostErrStr());
		if (g->conn.connectionType == 'O') {
			WTPAbort (sock, g);
		}
        	if (errno == ENOMEM) {
        		MyErrorFunc("ENOMEM","");
        	}
		ret = -10;
		break;
	} else {
		if (g->conn.connectionType == 'O') {
			switch ((buf[0] & 0x78) >> 3) {
				case WTP_Result:
					switch ((buf[0] & 0x07) >> 1) { 
						case 0x03: // SAR Not supported
						case 0x01: // End of transmission
							if (buf[2] != g->tid) //need better tid handeling
								return(0);
							else
								WTPAck(sock, g);
							break;
						case 0x00: // Not Last
						{
							if (buf[0] & 0x80) {
								if (buf[4] == 0x02) {
									unsigned char intgr[3];

									intgr[0] = buf[5];
									intgr[1] = buf[6];
									msgSize = (UInt32)intgr;
								}

								off = 4;
								haveTPI = true;
							}
							SARaddSegment(0, false, buf, ret, SAR_IS_FIRST, off, g);
							haveStart = true;
							goto start;
							break;
						}
					}
					break;
				case WTP_SAR_Result:
					// for missing packets, we should try to wait for them
					if (buf[3] != (t + 1)) {
						SARWTPNegAck (sock, buf[3], t, g);
						goto start;
						break;
					} else if (!haveStart) {
						SARWTPNegAck (sock, 0, 0, g);
						goto start;
						break;
					}
					if (!(buf[0] & 0x01)) t++;

					switch ((buf[0] & 0x07) >> 1) { 
						case 0x02: // Last packet of group
							SARaddSegment(buf[3], false, buf, ret, SAR_NOT_FIRST, 4, g);
							SARWTPAck(sock, t, true, haveTPI, g);
							goto start;
							break;
						case 0x01: // End of transmission
							SARaddSegment(buf[3], true, buf, ret, SAR_NOT_FIRST, 4, g);
							SARWTPAck(sock, t, false, false, g);
							ret = SARreassemble(buf, buflen, g);
							SARdeleteData(g);
							break;
						case 0x00: // Not Last
							SARaddSegment(buf[3], false, buf, ret, SAR_NOT_FIRST, 4, g);
							goto start;
							break;
					}
					break;
				case WTP_Ack:
					if (buf[0] == WTP_TidVe) {
						WTPTidVe(sock, g);
					}
					ret = WspGetReply(sock,buf,buflen,maxTimer,g); // should we loop a little, like while(!ret) ?
					if (ret != -5) return(ret);
					else return(-5);
					break;
				case WTP_Abort:
					if (g->isImage != true) MyErrorFunc("Could not receive data from server. Remote host sent Abort.","Receive");
    					g->progress= 0;
    					showProgress2(g);
        				g->state = BS_IDLE;
					if (g->isImage != true) browserSetTitle("WAPUniverse"); // SetFieldFromStr2("WAPUniverse",fieldTitle);
					ret = -10;
					break;
				default:
					if (buf[2] != g->tid) // need better tid handeling?
						return(0);
					else
						WTPAck(sock, g);
    					break;
			}
			break;
    		}
        	//g->state = BS_WSP_DECODE;

   	}
	break;

}

    return(ret);
}

void WSPCloseSocket(Int16 *sock)
{
    if (*sock != -1)
        close (*sock);
    *sock = -1;
}


Boolean SARaddSegment(UInt32 pos, Boolean last, char *data, UInt32 size, int type, int off, GlobalsType *g)
{
  SARPtr tmp, sar;
  UInt16 i = 0;
	
	sar = g->segment;
	while(sar != NULL)
	{
		
		if (pos == sar->pos)
		{
			return(false); // We already added this segment, I hope...
		}
		sar = sar->next;
	}

	tmp = Malloc(sizeof(SARType));
		ErrFatalDisplayIf (!tmp, "Malloc Failed");
	MemSet(tmp,sizeof(SARType),0);
	tmp->pos = pos;
	tmp->last = last;
	//if (type == SAR_NOT_FIRST) {
		tmp->size = size-off;
		tmp->data = Malloc(tmp->size+1);
		ErrFatalDisplayIf (!tmp->data, "Malloc Failed");

		for(i=0; i<(tmp->size); i++) {
			tmp->data[i] = data[i+off];
		}
	/*} else {
		tmp->size = size;
		tmp->data = Malloc(tmp->size+1);
		ErrFatalDisplayIf (!tmp->data, "Malloc Failed");

		for(i=0; i<(tmp->size); i++) {
			tmp->data[i] = data[i];
		}
	}*/

	tmp->next = g->segment;
    	g->segment = tmp;
			
return(true);
}

UInt32 SARreassemble(char *buf, Int16 buflen, GlobalsType *g)
{
  SARPtr tmp;
  UInt32 o = 0, offset = 0;
  UInt16 i = 0;
  Boolean finished = false;
  UInt32 lastpkt = 0;


	tmp = g->segment;

	while (tmp != NULL) {
		if (tmp->last) {
			lastpkt = tmp->pos;
			break;
		}

		tmp = tmp->next;
	}

	tmp = g->segment;
	while(!finished)
  	{
	  	while(tmp != NULL){
			if (tmp->pos == o)
				break;
			else
				tmp = tmp->next;	      		
	  	}

		//if (MemPtrSize(buf) < (offset+tmp->size+1))
		//	MemPtrResize(buf, offset+tmp->size+1);

		for(i=0; i<(tmp->size); i++) {
			buf[offset++] = tmp->data[i];
		}

		//offset += tmp->size;

		if (o == lastpkt)
			finished = true;

		o++; // increase offeset

		tmp = g->segment; //start over	
  	}

	return offset;
}

Boolean SARdeleteData(GlobalsType *g)
{
  SARPtr tmp;

	while(g->segment != NULL){
		tmp = g->segment;
		g->segment = g->segment->next;
		tmp->pos = 0;
		tmp->size = 0;
		tmp->last = false;
		if(tmp->data)
			Free(tmp->data);
		Free(tmp);
	}		
  	return(true);
}
