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
#include <PalmOS.h>
#include <SystemMgr.h>
#include "build.h"
#include "WAPUniverse.h"
#include "wtp.h"
#include "wsp.h"
#include "WAP.h"
#include "../res/WAPUniverse_res.h"
#ifdef __palmos__
    #include "dbConn.h"
    #include "dbUrl.h"
    #include "formBrowser.h"
#endif

void MyErrorFunc3 (char* error, char* additional)
{

    FrmCustomAlert (alError, error, additional ? additional : "", NULL);
}

/*void SetFieldFromStr3(const Char *srcP , UInt16 fieldIndex)
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

//Some local vars
unsigned short tid_addition = 1;


/*static void set_tid(unsigned char hdr[], int tid) {
	int c;
	
	c = hdr[1];
	c |= 0x7f & (tid >> 8);
	hdr[1] = c;
	hdr[2] = (unsigned char) tid;
}

//Use this only on Invoke packets, the others have no tid_new field
static void set_tid_new(unsigned char hdr[]) {
	int c;

	c = hdr[3];
	c |= 0x40;
	hdr[3] = c;
}*/

/* returns next tid, given current tid.  Every thread has its own
 * port, so has its own tid space. */
unsigned short next_tid(unsigned short old_tid) { 
    return (old_tid + tid_addition) % (1 << 15);
}


void addOctets( unsigned char in[], unsigned char out[], int inoff, int outoff)
{
int i;
	for (i = 0; i < (StrLen(in) - inoff); i++) {
		out[outoff+i] = in[inoff+i];
	}
}

UInt32 getUintVar(char *instr, int i)
{
	UInt32 v = 0;
	unsigned char c;
	int count;
	
	for (count = 0; count < 5; count++) {
		c = instr[i+count];
		v = (v << 7)|(c & 0x7f);
		if (!(c & 0x80)) {
            		return(v);
            	}
	}	

	return(v);
}

int WTPConnect(int sock, char *gwip, char *urlStr, Boolean remember, GlobalsType *g )
{
	Char		*outbuf = NULL;
	UInt16 		i = 0;
	Int32 		AppNetTimeout;
	UInt16		agent = 0;
	UInt16 		profLen = 0;
	static 		UInt32 maxTimer;

	outbuf = Malloc(WAP_MAX_PACKET);
	if (!outbuf) {
		g->wtpState = WTP_Disconnected;
		MyErrorFunc("Failed to connect to the server.", "Reason: Out of memory");
		return sock;
	}

	MemSet(outbuf, WAP_MAX_PACKET, 0);

	g->wtpState = WTP_Connecting;

    	AppNetTimeout = 3 * SysTicksPerSecond();
   	maxTimer = TimGetTicks() + (REPLY_TIMEOUT * SysTicksPerSecond());


	if (g_prefs.altAgent)
		agent = g_prefs.agentNum + 1;


	if (g->retransmission == false) {
    		g->old_tid = g->tid;
    		g->tid = next_tid(g->old_tid);
    		g->tid_new = (g->tid < g->old_tid);  /* Did we wrap? */
	}


    	// Send Connect
	if (g->retransmission == true)
#ifdef USE_SAR_SUPPORT
		outbuf[i++] = 0x0b;
#else
		outbuf[i++] = 0x0f; 
#endif
	else
#ifdef USE_SAR_SUPPORT
		outbuf[i++] = 0x0a;
#else
		outbuf[i++] = 0x0e; 
#endif
    	outbuf[i++] = 0x00;
    	outbuf[i++] = g->tid; // We sould use a better way to incorporate the TID
    	outbuf[i++] = 0x12;
    	outbuf[i++] = WSP_Connect;
    	outbuf[i++] = 0x10;
    	outbuf[i++] = 19;//8; //0x00; // Capabilities Length

	if (!g_prefs.disableUAProf) {
    		profLen = gstrlen(UNIVERSE_UA_PROF);//gstrlen("http://www.wapuniverse.com/devices/palmos_generic/wapuniverse.rdf");
	} else {
		profLen = 0;
	}

    	if ( gstrlen(g->conn.proxyuser) != 0 ) {
		if ( (gstrlen(g->conn.proxyuser) + gstrlen(g->conn.proxypass) + 3) >= 31 ) {
			enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->conn.proxyuser)+gstrlen(g->conn.proxypass)+profLen+35+8+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+gstrlen("image/bmp")+14+gstrlen("image/jpg")+2+gstrlen("application/xhtml+xml")+gstrlen("application/wml+xml")+gstrlen("application/vnd.wap.xhtml+xml"));//outbuf[i++] = StrLen(&AgentString[agent])+30; // Headers Length
		} else {
			enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->conn.proxyuser)+gstrlen(g->conn.proxypass)+profLen+34+8+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+gstrlen("image/bmp")+14+gstrlen("image/jpg")+2+gstrlen("application/xhtml+xml")+gstrlen("application/wml+xml")+gstrlen("application/vnd.wap.xhtml+xml"));//outbuf[i++] = StrLen(&AgentString[agent])+29; // Headers Length
		}
    	} else {
		enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+profLen+29+8+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+gstrlen("image/bmp")+14+gstrlen("image/jpg")+2+gstrlen("application/xhtml+xml")+gstrlen("application/wml+xml")+gstrlen("application/vnd.wap.xhtml+xml"));
    	}

    	//Capabilities
    	//Client SDU size
    	outbuf[i++] = 0x04;
    	outbuf[i++] = 0x80;
    	outbuf[i++] = 0x83;
    	outbuf[i++] = 0xf8;
    	outbuf[i++] = 0x00;
    	//Server SDU size
	/* NOTE: We should change this to 4k, as we have no   *
	 * facilities to disassemble large data at the moment */
    	outbuf[i++] = 0x04;
    	outbuf[i++] = 0x81;
    	outbuf[i++] = 0x83;
    	outbuf[i++] = 0xf8;
    	outbuf[i++] = 0x00;
    	//Protocol Options
    	outbuf[i++] = 0x02;
    	outbuf[i++] = 0x82;
    	outbuf[i++] = 0x00;
    	//Method MOR
    	outbuf[i++] = 0x02;
    	outbuf[i++] = 0x83;
    	outbuf[i++] = 0x01;
    	//Push MOR
    	outbuf[i++] = 0x02;
    	outbuf[i++] = 0x84;
    	outbuf[i++] = 0x00;

    	// Headers
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x94; // 14 application/vnd.wap.wmlc
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xA9; // 29 application/vnd.wap.wbxml
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x95; // 15 application/vnd.wap.wmlscriptc
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xA1; // 21 image/vnd.wap.wbmp
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x9E; // 1E image/jpeg
	outbuf[i++]=0x80; // 00 Accept
	gstrcopy(outbuf + i,"image/jpg");
       			i+= gstrlen("image/jpg");
       			outbuf[i++]=0x00;
	outbuf[i++]=0x80; // 00 Accept
	gstrcopy(outbuf + i,"image/bmp");
       			i+= gstrlen("image/bmp");
       			outbuf[i++]=0x00;
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x9d; // 1d image/gif
    	outbuf[i++]=0x80; // 00 Accept
    	gstrcopy(outbuf + i,"application/xhtml+xml");
       			i+= gstrlen("application/xhtml+xml");
       			outbuf[i++]=0x00; //outbuf[i++]=0xBB; // 3B application/xhtml+xml
    	outbuf[i++]=0x80; // 00 Accept
    	gstrcopy(outbuf + i,"application/wml+xml");
       			i+= gstrlen("application/wml+xml");
       			outbuf[i++]=0x00; //outbuf[i++]=0xBC; // 3C application/wml+xml
    	outbuf[i++]=0x80; // 00 Accept
    	gstrcopy(outbuf + i,"application/vnd.wap.xhtml+xml");
       			i+= gstrlen("application/vnd.wap.xhtml+xml");
       			outbuf[i++]=0x00; //outbuf[i++]=0xC5; // 45 application/vnd.wap.xhtml+xml
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x82; // 02 text/html
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xA0; // 20 image/png
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x80; // 00 all
    	outbuf[i++]=0x83; // 03 Accept Language
    	outbuf[i++]=0x99; // 19 = en
    	outbuf[i++]=0x81; // 01 Accept Charset
    	outbuf[i++]=0xEA; // 6A = UTF-8
    	outbuf[i++]=0x81; // 01 Accept Charset
    	outbuf[i++]=0x83; // 03 us-ascii
    	outbuf[i++]=0x81; // 01 Accept Charset
    	outbuf[i++]=0x84; // 04 iso-8859-1 
    	outbuf[i++]=0x81; // 01 Accept Charset
    	outbuf[i++]=0x03; // 03 us-ascii
    	outbuf[i++]=0x02; // 02 Accept Encoding
    	outbuf[i++]=0x03; // 03 Token Text ???????
    	outbuf[i++]=0xE8; // 68
    	if (g->conn.proxyuser && gstrlen(g->conn.proxyuser) && g->conn.proxypass && gstrlen(g->conn.proxypass)) {
      		outbuf[i++]=0xA1; // 21 Proxy-Authorization
      		if ( (gstrlen(g->conn.proxyuser) + gstrlen(g->conn.proxypass) + 3) >= 31 ) {
	        	outbuf[i++]=0x1F;
      		}
      		enc_uintvar(outbuf,&i,gstrlen(g->conn.proxyuser) + gstrlen(g->conn.proxypass) + 3);
      		outbuf[i++]=0x80;
      		gstrcopy(outbuf + i,g->conn.proxyuser);
       			i+= gstrlen(g->conn.proxyuser);
       			outbuf[i++]=0x00;
      		gstrcopy(outbuf + i,g->conn.proxypass);
       			i+= gstrlen(g->conn.proxypass);
       			outbuf[i++]=0x00;
    	}
    	outbuf[i++]=0xA9; // 29 User Agent
    	gstrcopy(outbuf + i,AgentString[agent]);
		i+= gstrlen(AgentString[agent]);
		outbuf[i++]=0x00;

	if (!g_prefs.disableUAProf) {
    		outbuf[i++]=0xB5; // 35 Profile
    		gstrcopy(outbuf + i, UNIVERSE_UA_PROF);
    			i+= gstrlen(UNIVERSE_UA_PROF);
    			outbuf[i++]=0x00;
	}

    	outbuf[i++]=0xb3; // 33 Bearer Indication
    	outbuf[i++]=0x80; // 00 IPv4
    	outbuf[i++]=0xb2; // 32 Accept-Application
    	outbuf[i++]=0x82; // 02 x-wap-application:wml.ua
    	outbuf[i++]=0x89; // 09 Connection
    
    	gstrcopy(outbuf + i,"Encoding-Version");
		i+= gstrlen("Encoding-Version");
		outbuf[i++]=0x00;
    	gstrcopy(outbuf + i,"Encoding-Version");
		i+= gstrlen("Encoding-Version");
		outbuf[i++]=0x00;
    	gstrcopy(outbuf + i,"1.3");
		i+= gstrlen("1.3");
		outbuf[i++]=0x00;

	if (sendto(sock,outbuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0) {
		MyErrorFunc3("\nCould not send data to the server.\nReason:\n", WspHostErrStr());
		g->wtpState = WTP_Disconnected;
	}

	Free(outbuf);

    	return sock;
}

int WTPHandleConnectResp(int sock, char *gwip, GlobalsType *g )
{
	unsigned char 	rep[500];
	UInt16 		j = 0, k = 0, n = 0;
	Int32 		AppNetTimeout;
	Int16 		reterr;
	char 		*buf;
	UInt32 		maxTimer;


	AppNetTimeout = 3 * SysTicksPerSecond();
	maxTimer = TimGetTicks() + (REPLY_TIMEOUT * SysTicksPerSecond());

for (;;) {    

restart:
	buf = Malloc(4097);
	if (!buf) {
		MyErrorFunc("Could not receive response. Memory error.", NULL);
		return -1;
	}

	reterr = recv(sock,buf,4096,0);
	showProgress(g);

    	if ((errno==netErrTimeout) && ( TimGetTicks() < maxTimer ) ) {
	    	g->retransmission = true;
    	    	return 0;
    	}

	if (reterr <= 0) {
		Free(buf);
		g->progress= 0;
		browserSetTitle("WAPUniverse"); // SetFieldFromStr3("WAPUniverse",fieldTitle);
		MyErrorFunc("\nCould not connect to the server.\nReason:\n", WspHostErrStr());
		g->wtpState = WTP_Disconnected;
    		return -1;
    	}
    	else {
		if (buf[0] == WTP_TidVe) {
			n = 0;
			Free(buf);
			buf = Malloc(4097);
retrans2:
			if (n > 0)
				rep[j++] = WTP_TidVe + 1; // Re-Transmission
			else
				rep[j++] = WTP_TidVe;
			rep[j++] = 0x00;
			rep[j++] = g->tid;
                	if (sendto(sock,&rep,j,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0) {
      				MyErrorFunc3("\nCould not send data to the server.\nReason:\n", WspHostErrStr());
				Free(buf);
				return -1;
			}


			n++;
			reterr = recv(sock,buf,4096,0);

			if ((errno == netErrTimeout) && ( TimGetTicks() < maxTimer ) ) {
	    			goto retrans2;
    			}

    			if (reterr <= 0) {
				Free(buf);
				if (reterr == -1) WTPAbort(sock, g);
				g->progress= 0;
				browserSetTitle("WAPUniverse"); // SetFieldFromStr3("WAPUniverse",fieldTitle);
				g->wtpState = WTP_Disconnected;
    				return -1;
    			}
		}

//TODO: Some of the stuff below needs to be redone for better accuracy...

		// Some gateways are too concerned and send Ack after just about anything we send, so
		// we must learn to ignore unwanted Ack's
		if (((buf[0] & 0x78) >> 3) == WTP_Ack) {
			Free(buf);
			goto restart;
		} else if (buf[3] == WSP_Redirect) {
			Free(buf);
			WTPAbort(sock, g);
			g->state = BS_IDLE;
	       	 	g->progress= 0;
			browserSetTitle("WAPUniverse"); // SetFieldFromStr3("WAPUniverse",fieldTitle);
			MyErrorFunc("\nReceived Redirect PDU.\nTry changing the port to 49200 or 49201 and try again.\n","WSP Redirect");
			g->wtpState = WTP_Disconnected;
			return -1;
		} else if ((buf[0] == WTP_Abort) || (buf[0] == WTP_Abort1) || (buf[0] == WTP_User_Abort)) {
			Free(buf);
			g->state = BS_IDLE;
	        	g->progress= 0;
			browserSetTitle("WAPUniverse"); // SetFieldFromStr3("WAPUniverse",fieldTitle);
			g->wtpState = WTP_Disconnected;
			MyErrorFunc("Failed to connect to the remote host. Server sent abort.","WAP Connect");
                	return -1;
    		} else if (buf[3] != WSP_ConnectReply) { // TODO:Need a better way to check...
			Free(buf);
			return 0;
		}

		while ( buf[k] != 0x02 ) k++;
		k++;
		g->sid = getUintVar(buf, k);
		Free(buf);
		break;
    	}
}
	WTPAck(sock, g);

    	g->wtpState = WTP_Connected;

    	return sock;
}

int wtp_get(int sock, char *gwip, char *urlStr, Boolean remember, Err *error, GlobalsType *g )
// Boolean remember: true if we are loading a WML page (and not an image), so we set gLastURL to current URL, needed for relative links later on
{
    Char		*outbuf = NULL;
    UInt16		i = 0;
    Int32		AppNetTimeout;

	*error = errNone;

	outbuf = Malloc(WAP_MAX_PACKET);
	if (!outbuf) {
		*error = errWapNoMemory;
		MyErrorFunc("Failed to request data from the server.", "Reason: Out of memory");
		return sock;
	}

	MemSet(outbuf, WAP_MAX_PACKET, 0);

	AppNetTimeout = 3 * SysTicksPerSecond();

	if (g->retransmission != true) {
		g->old_tid = g->tid;
		g->tid = next_tid(g->old_tid);
		g->tid_new = (g->tid < g->old_tid);  /* Did we wrap? */
	}

	// See WSP.PDF for more info
	if (g->retransmission == true)
#ifdef USE_SAR_SUPPORT
		outbuf[i++]= 0x0b;
#else
		outbuf[i++]= 0x0f; 
#endif
	else
#ifdef USE_SAR_SUPPORT
		outbuf[i++]= 0x0a;
#else
		outbuf[i++]= 0x0e; 
#endif

	outbuf[i++]= 0x00;
	outbuf[i++] = g->tid;// Transaction ID
	outbuf[i++]=0x12;
	outbuf[i++]=WSP_Get;
	//escapeUrl(urlStr); 
	enc_uintvar(outbuf,&i,gstrlen(urlStr)); // URIlen , uintvar !!
	gstrcopy((outbuf+i), urlStr);
	if(remember)
		gstrcopy(g->WapLastUrl,urlStr);
	i+= gstrlen(urlStr); 

	// TODO: what if user does not want a password, i.e. horrible security??
	if (g->AuthUser && g->AuthPass && gstrlen(g->AuthUser) && gstrlen(g->AuthPass)) {
		outbuf[i++]=0x87; // 07 Authorization
		if ( (gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3) >= 31 ) {
	        	outbuf[i++]=0x1F;
      		}
		enc_uintvar(outbuf,&i,gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3);
		outbuf[i++]=0x80;
			gstrcopy(outbuf + i,g->AuthUser);
			i+= gstrlen(g->AuthUser);
		outbuf[i++]=0x00;
			gstrcopy(outbuf + i,g->AuthPass);
			i+= gstrlen(g->AuthPass);
		outbuf[i++]=0x00;

		//Free(g->AuthUser);
		//Free(g->AuthPass);
		//g->AuthUser = NULL;
		//g->AuthPass = NULL;
	} 

	if (g->isImage == true) {
		outbuf[i++]=0x80; // 00 Accept
		outbuf[i++]=0x9E; // 1E image/jpeg
		outbuf[i++]=0x80; // 00 Accept
		gstrcopy(outbuf + i,"image/jpg");
       			i+= gstrlen("image/jpg");
       			outbuf[i++]=0x00;
		outbuf[i++]=0x80; // 00 Accept
		outbuf[i++]=0xA1; // 21 image/vnd.wap.wbmp
		outbuf[i++]=0x80; // 00 Accept
		gstrcopy(outbuf + i,"image/bmp");
       			i+= gstrlen("image/bmp");
       			outbuf[i++]=0x00;
    		outbuf[i++]=0x80; // 00 Accept
    		outbuf[i++]=0x9d; // 1d image/gif
    		outbuf[i++]=0x80; // 00 Accept
    		outbuf[i++]=0xA0; // 20 image/png
	}

	if (sendto(sock,outbuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0) {
		MyErrorFunc3("\nCould not send data to the server.\nReason:\n", WspHostErrStr());
		*error = errno;
	} else {
		*error = errNone;
	}

	g->retransmission = false;
	Free(outbuf);
	return sock;
}

int wtp_post(int sock, char *gwip, char *urlStr, Boolean remember, Err *error, GlobalsType *g )
// Boolean remember: true if we are loading a WML page (and not an image), so we set gLastURL to current URL, needed for relative links later on
{
	Char 		*outbuf = NULL;
    	UInt16 		i = 0;
    	Int32 		AppNetTimeout;

	*error = errNone;

	outbuf = Malloc(WAP_MAX_PACKET);
	if (!outbuf) {
		*error = errWapNoMemory;
		MyErrorFunc("Failed to request data from the server.", "Reason: Out of memory");
		return sock;
	}

	MemSet(outbuf, WAP_MAX_PACKET, 0);
    
    	AppNetTimeout = 3 * SysTicksPerSecond();


	if (g->retransmission != true) {
    		g->old_tid = g->tid;
    		g->tid = next_tid(g->old_tid);
    		g->tid_new = (g->tid < g->old_tid);  // Did we wrap?
	}

    	// Send POST Data
	if (g->retransmission == true)
#ifdef USE_SAR_SUPPORT
		outbuf[i++]= 0x0b;
#else
		outbuf[i++]= 0x0f; 
#endif
	else
#ifdef USE_SAR_SUPPORT
		outbuf[i++]= 0x0a;
#else
		outbuf[i++]= 0x0e; 
#endif
    	outbuf[i++] = 0x00;
    	outbuf[i++] = g->tid;// Transaction ID
    	outbuf[i++] = 0x12;
    	outbuf[i++] = WSP_PDU_POST;
    	enc_uintvar(outbuf,&i,gstrlen(urlStr)); // URIlen , uintvar !!
    
    	if ( (gstrlen(g->AuthUser) != 0) && (g->AuthUser != NULL) && (gstrlen(g->AuthPass) != 0) && (g->AuthPass != NULL) ) {
    		if ( (gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3) >= 31 ) {
			enc_uintvar(outbuf,&i,gstrlen(g->AuthUser)+gstrlen(g->AuthPass)+4); // Header & ContentType , uintvar !!
    		} else {
			enc_uintvar(outbuf,&i,gstrlen(g->AuthUser)+gstrlen(g->AuthPass)+3); // Header & ContentType , uintvar !!
    		}
    	} else {
		enc_uintvar(outbuf,&i,1); // Header & ContentType , uintvar !!
    	}

    	gstrcopy((outbuf+i), urlStr);
	if(remember)
		gstrcopy(g->WapLastUrl,urlStr);
    	i+= gstrlen(urlStr);

    	outbuf[i++]=0x92; // 12 x-www-form-urlencoded

	// TODO: what if user does not want a password, i.e. horrible security??
	if (g->AuthUser && g->AuthPass && gstrlen(g->AuthUser) && gstrlen(g->AuthPass)) {
      		outbuf[i++]=0x87; // 07 Authorization
      		if ( (gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3) >= 31 ) {
	        	outbuf[i++]=0x1F;
      		}
      		enc_uintvar(outbuf,&i,gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3);
      		outbuf[i++]=0x80;
      		gstrcopy(outbuf + i,g->AuthUser);
       			i+= gstrlen(g->AuthUser);
       			outbuf[i++]=0x00;
      		gstrcopy(outbuf + i,g->AuthPass);
       			i+= gstrlen(g->AuthPass);
       			outbuf[i++]=0x00;

     		//Free(g->AuthUser);
      		//Free(g->AuthPass);
      		//g->AuthUser = NULL;
      		//g->AuthPass = NULL;
    	}
  
    	gstrcopy(outbuf + i,g->postBuf);
     		i+= gstrlen(g->postBuf);
    
    
    	if (sendto(sock,outbuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0) {
          	MyErrorFunc3("\nCould not send data to the server.\nReason:\n", WspHostErrStr());
		*error = errno;
	}

	Free(outbuf);

    	return sock;
}

int WTPAbort (Int16 sock, GlobalsType *g)
{
	unsigned char 	obuf[500];
    	UInt16 		i = 0;

	AppNetTimeout = 3 * SysTicksPerSecond();

	// Send Ack
	obuf[i++] = WTP_User_Abort;
	obuf[i++] = 0x00;
        obuf[i++] = g->tid;
        obuf[i++] = 0xe8; // User Aborted, probably should add variable to pass

	if (sendto(sock,&obuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0)
          	MyErrorFunc("\nCould not send data to the server.\nReason:\n", WspHostErrStr());

	return sock;
}

int WTPAck (Int16 sock, GlobalsType *g)
{   
	unsigned char 	obuf[500];
    	UInt16 		i = 0;

	AppNetTimeout = 3 * SysTicksPerSecond();

	// Send Ack
	obuf[i++] = WTP_Ack1;
	obuf[i++] = 0x00;
        obuf[i++] = g->tid;

	if (sendto(sock,&obuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0)
          	MyErrorFunc("\nCould not send data to the server.\nReason:\n", WspHostErrStr());

	return sock;
}

int WTPTidVe (Int16 sock, GlobalsType *g)
{
	Char		*buf;
	UInt16		n = 0, j = 0;
	unsigned char	rep[500];
	Int16 		reterr = 0;
	UInt32 		maxTimer;


	AppNetTimeout = 3 * SysTicksPerSecond();
	maxTimer = TimGetTicks() + (REPLY_TIMEOUT * SysTicksPerSecond());

	n = 0;
	buf = Malloc(4097);
	if (!buf)
		return -1;
retrans2:
	if (n > 0)
		rep[j++] = WTP_TidVe + 1; // Re-Transmission
	else
		rep[j++] = WTP_TidVe;
	rep[j++] = 0x00;
	rep[j++] = g->tid;
        if (sendto(sock,&rep,j,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0)
      		MyErrorFunc3("\nCould not send data to the server.\nReason:\n", WspHostErrStr());


	n++;
	reterr = recv(sock,buf,4096,0);

	if ((errno == netErrTimeout) && ( TimGetTicks() < maxTimer ) ) {
	    	goto retrans2;
    	}

    	if (reterr <= 0) {
		Free(buf);
		if (reterr == -1) WTPAbort(sock, g);
		g->progress= 0;
		browserSetTitle("WAPUniverse"); // SetFieldFromStr3("WAPUniverse",fieldTitle);
		g->wtpState = WTP_Disconnected;
    		return -1;
    	}

	return sock;
}

int SARWTPAck (Int16 sock, int t, Boolean endGroup, Boolean haveTPI, GlobalsType *g)
{   
	unsigned char 	obuf[500];
    	UInt16 		i = 0;

	AppNetTimeout = 3 * SysTicksPerSecond();

	// Send Ack
	obuf[i++] = 0x98;
	obuf[i++] = 0x00;
        obuf[i++] = g->tid;

	obuf[i++] = ((endGroup == true && haveTPI == true) ? 0x99 : 0x19);// 0x19; //((endGroup == false && haveTPI == false) ? 0x19 : 0x99);// 0x19;
	obuf[i++] = t;

	if (endGroup == true && haveTPI == true) {
		obuf[i++] = 0x13;
		obuf[i++] = 0x04;
		obuf[i++] = 0x04;
		obuf[i++] = 0x00;
	} /*else if (endGroup == true && haveTPI == true) {
		obuf[i++] = 0x13;
		obuf[i++] = 0x04;
		obuf[i++] = 0x0b;
		obuf[i++] = 0xa8;
	}*/

	if (sendto(sock,&obuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0)
          	MyErrorFunc("\nCould not send data to the server.\nReason:\n", WspHostErrStr());

	return sock;
}

int SARWTPNegAck (Int16 sock, Int16 curPkt, Int16 pktCount, GlobalsType *g)
{   
	unsigned char 	obuf[500];
    	UInt16 		i = 0, j = 0;

	AppNetTimeout = 3 * SysTicksPerSecond();

	// Send Ack
	obuf[i++] = WTP_NegAck;
	obuf[i++] = 0x00;
        obuf[i++] = g->tid;
	obuf[i++] = curPkt - pktCount;

	for (j = pktCount; j < (curPkt + 1); j++) {
		obuf[i++] = j;
	}

	if (sendto(sock,&obuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0)
          	MyErrorFunc("\nCould not send data to the server.\nReason:\n", WspHostErrStr());

	return sock;
}


/*int WTPDisconnect (Int16 sock, GlobalsType *g)
{   
	unsigned char 	obuf[500];
    	UInt16 		i = 0, j = 0;

	AppNetTimeout = 3 * SysTicksPerSecond();

	// Send Ack
	obuf[i++] = WTP_Ack1;
	obuf[i++] = 0x00;
        obuf[i++] = g->tid;

	if (sendto(sock,&obuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0)
          MyErrorFunc3("\nCould not send data to the server.\nReason:\n", WspHostErrStr());


        i = 0;

	// Send Disconnect
#ifdef USE_SAR_SUPPORT
	outbuf[i++]= 0x0a;
#else
	outbuf[i++]= 0x0e; 
#endif
 	obuf[i++] = 0x00;
	obuf[i++] = g->tid;
 	obuf[i++] = 0x10;
 	obuf[i++] = WSP_Disconnect;
        enc_uintvar(obuf,&i,g->sid);

	if (sendto(sock,&obuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0)
          MyErrorFunc3("\nCould not send data to the server.\nReason:\n", WspHostErrStr());

	return sock;
}*/