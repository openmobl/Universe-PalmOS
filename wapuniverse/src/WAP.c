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
#include 	<SystemMgr.h>
#include 	"build.h"
#include 	"WAPUniverse.h"
#include 	"wsp.h"
#include 	"dbConn.h"
#include 	"dbUrl.h"
#include 	"wtp.h"
#include	"variable.h"
#include	"Form.h"
#include	"String.h"
#include	"formBrowser.h"


/*Char *AgentString[] = {
	"Universe/3.0.6 (PalmOS; N; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)", 			// Generic User Agent
	"Universe/3.0.6 (PalmOS; N; m68K; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)", 		// Motorola 68K agent
	"Universe/3.0.6 (PalmOS; N; ARM; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)", 		// ARM CPU agent
	"Universe/3.0.6 (PalmOS; N; X-Scale; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)", 		// X-Scale CPU agent
	"Universe/3.0.6 (PalmOS; N; POSE; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)", 		// PalmOS Emulator agent
	"Universe/3.0.6 (PalmOS; N; x86; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)", 		// PalmOS Simulator agent
	"Universe/3.0.6 (PalmOS; N; unknown; en) MobileToken/20070113 (WAP 1.3/2.0, XHTML; OpenMobl)",
	"UP.Browser/4.0 (compatable; Universe/3.0.5; PalmOS; N; en)", 						// Unwired Planet agent
	"Nokia6600/1.0 (4.09.1) SymbianOS/7.0s (compatible; Universe/3.0.6)",					// Nokia agent: Nokia6600/1.0 (4.09.1) SymbianOS/7.0s Series60/2.0 Profile/MIDP-2.0 Configuration/CLDC-1.0
	//"Mozilla/5.0 (compatible; Universe/3.0.4; PalmOS; N; en) MobileToken/20070113 (OpenMobl)"		// Mozilla Compatible user agent
	"Mozilla/5.0 (PalmOS; N; ARM; en-us) MobileToken/20061221 (OpenMobl) Universe/3.0",			// Mozilla Compatible user agent
	"Mozilla/4.0 (compatible; MSIE6, Windows NT 5.0)"							// IE UA
};*/

/*Char *AgentString[] = {
	// Mozilla Compatible user agent
	"Mozilla/5.0 (PalmOS; U; ARM; en-us) MobileToken/" ## BUILDFORMATSTR ## " (OpenMobl) Universe/3.0",

	// Mozilla Firefox user agent
	"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-us; rv:1.8.1.1) Gecko/" ## BUILDFORMATSTR ## " Firefox/2.0.0.1",

	// IE UA
	"Mozilla/4.0 (compatible; MSIE6, Windows NT 5.0; SV1)",

	// Nokia S60
	"Mozilla/5.0 (SymbianOS/9.2; U; Series60/3.1 NokiaE70/1.0; Profile/MIDP-2.0 Configuration/CLDC-1.1) AppleWebKit/413 (KHTML, like Gecko) Safari/413",

	// Nokia agent: Nokia6600/1.0 (4.09.1) SymbianOS/7.0s Series60/2.0 Profile/MIDP-2.0 Configuration/CLDC-1.0
	"Nokia6600/1.0 (4.09.1) SymbianOS/7.0s (compatible; Universe/" ## VERSIONSTR2 ## ")"
};*/

Char *AgentString[] = {
	// Mozilla Compatible user agent
	AGENT_STRING_DEFAULT,

	// Mozilla Firefox user agent
	AGENT_STRING_MOZ,

	// MS IE 6 UA
	AGENT_STRING_MSIE6,

	// Nokia S60
	AGENT_STRING_S60,

	// Nokia agent: Nokia6600/1.0 (4.09.1) SymbianOS/7.0s Series60/2.0 Profile/MIDP-2.0 Configuration/CLDC-1.0
	AGENT_STIRING_NOK
};


/*int StringToIPAddr (char *str, unsigned long *addr)
{
	struct hostent 		*hp = NULL;
    	Err 			h_errno;
    	Int32 			AppNetTimeout;
    	NetHostInfoBufType 	AppHostInfo;    // DO NOT DELETE !!
						// without this AppHostInfo, gethostbyname() corrupts memory!!!!!!!


	AppNetTimeout = REPLY_TIMEOUT * SysTicksPerSecond();
	//if ((hp = gethostbyname (str)) == 0) {
	//	if ((long)(*addr = inet_addr (str)) == -1)
	//		return 0;
	//} else
	//	bcopy ((char *)(hp->h_addr), (char *)addr, hp->h_length);


	if ((long)(*addr = inet_addr (str)) == -1) {
		if ((hp = gethostbyname (str)) != NULL)
			bcopy ((char *)(hp->h_addr), (char *)addr, hp->h_length);
		else
			return 1;
	}

	return 1;
}*/

int StringToIPAddr (char *str, unsigned long *addr)
{
	GlobalsType 		*g;
	NetHostInfoBufPtr	AppHostInfo = NULL;
	NetHostInfoPtr		HostInfo = NULL;
	UInt32 			AppNetTimeout = 4 * SysTicksPerSecond();
	Err 			errP;
	NetIPAddr 		address;
	NetIPAddr 		*addressPtr = NULL;
	UInt32			time = 0;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	showProgress(g);

	address = NetLibAddrAToIN(AppNetRefnum, str);
	if (address != -1) {
		*addr = address;
		return(1);
	}

	AppHostInfo = (NetHostInfoBufPtr) Malloc(sizeof(NetHostInfoBufType));
	if (AppHostInfo == NULL) {
		//if (g->isImage != true) MyErrorFunc("Not enought dynamic heap for network communications.", NULL);
		return(0);
	}

	time = 0;
	do {
		HostInfo = NetLibGetHostByName(AppNetRefnum, str, AppHostInfo, AppNetTimeout, &errP);
		time += AppNetTimeout;
	} while ((HostInfo == NULL) && (errP == netErrTimeout) && (time < (g_prefs.timeout * SysTicksPerSecond())));

	if (HostInfo == NULL || errP != errNone) {
		//if (g->isImage != true) MyErrorFunc("Host lookup failed. Try again later.", NULL);
		if (AppHostInfo) Free(AppHostInfo);
		return(0);
	}

	showProgress(g);

	addressPtr = ( NetIPAddr*)HostInfo->addrListP[0];
	address = NetNToHL(*addressPtr);

	showProgress(g);

	if (AppHostInfo) Free(AppHostInfo);

	*addr = address;

	return(1);
}


char *gstrcopy(char *dst, const char *src) {
    // Code removed for performance reasons
    /*char *tmp=dst;

    if ( dst==NULL || src==NULL)
        return NULL;
    while (*src)
        *tmp++ = *src++;
    *tmp=0; 
    return(dst);*/

    return StrCopy(dst, src);
}


UInt16 gstrlen(char *str) {
    // Code removed for performance reasons
    /*UInt16 i=0;

    if ( str == NULL)
        return(0);

    while (*str++)
        i++;
    return(i);*/
    return StrLen(str);
}

char *GetErrStr(Err errno)
{
    static char msgstr[150];

    if (errno != 0)
        SysErrString(errno, msgstr, 50);
    else
        msgstr[0] = '\0';

    return(msgstr);
}

void enc_uintvar(char ostr[], UInt16 *j, unsigned long value)
{
    /* A uintvar is defined to be up to 32 bits large, so it will
     * fit in 5 octets. */
    unsigned char octets[5];
    UInt16 i, o;
    UInt16 start;

    /* Handle last byte separately; it has no continuation bit,
     * and must be encoded even if value is 0. */
    octets[4] = value & 0x7f;
    value >>= 7;

    for (i = 3; value > 0 && i >= 0; i--) {
        octets[i] = 0x80 | (value & 0x7f);
        value >>= 7;
    }
    start = i + 1;

    for (o = 0; o < (5 - start); o++) {
       ostr[(*j)++] = octets[start + o];//data[o];
    }
}

Boolean GetPort(Char *text, Int32 *valueP)
{

   *valueP = StrAToI(text);
   return true;
}

// connect2server: open the connection to the 'hostname' server
// and return the socket descriptor
int Connect2Server(char *gwip, char *urlStr, GlobalsType *g )
{
    unsigned long	host;
    int			sock;
    Err			error;
    Int32		port;

	if ((g->conn.connectionType == 'O') && ((g->wtpState == WTP_Connected) || (g->wtpState == WTP_Connecting))) {
		return(g->sock);
	} else if (g->conn.connectionType != 'O' && g->sock != -1) {
		return(g->sock);
	} else {
		if (StringToIPAddr(gwip,&host) != 1) {
			//DisplayError("Invalid WAP Gateway IP. You cannot use a domain name. You need an IP address");
			DisplayError("Connection host lookup failed. Try again later.");
			return -1;
		}


		memset((char *) &(g->RmtAddress), 0, sizeof(g->RmtAddress));
		g->RmtAddress.sin_family = AF_INET;

		GetPort(g->conn.port, &port);
		g->RmtAddress.sin_port = htons((UInt16)port);
		g->RmtAddress.sin_addr.s_addr = host;

		sock = NetLibSocketOpen (AppNetRefnum, netSocketAddrINET, netSocketTypeDatagram, netSocketProtoIPUDP, 20*SysTicksPerSecond(), &error);//socket(AF_INET, SOCK_DGRAM, 0);
			//sock = NetLibSocketOpen (AppNetRefnum, AF_INET, SOCK_DGRAM, 0, 20*SysTicksPerSecond(), &error);//socket(AF_INET, SOCK_DGRAM, 0);
		if (error != 0) {
			MyErrorFunc("\nCould not create communications socket.\nReason:\n",GetErrStr(error));//WspHostErrStr());
			return -2;
		}            


		g->LclAddress.sin_family = AF_INET;
		if (g->conn.connectionType == 'L') {
			g->LclAddress.sin_port   = htons(WAP_WSP_CL_LOCAL); }
		if (g->conn.connectionType == 'O') {
			g->LclAddress.sin_port   = htons(WAP_WSP_CO_LOCAL); }
		if (g->conn.connectionType == '1') {
			g->LclAddress.sin_port   = htons(WAP_WSP_WTLS_CL_LOCAL); }
		if (g->conn.connectionType == '2') {
			g->LclAddress.sin_port   = htons(WAP_WSP_WTLS_CO_LOCAL); }
		g->LclAddress.sin_addr.s_addr = INADDR_ANY;

#ifdef BIND
		if (bind(sock, (struct sockaddr *) &g->LclAddress, sizeof(g->LclAddress)) < 0) {
			MyErrorFunc("\nCould not bind to port.\nReason:\n",WspHostErrStr());
			return -2;
		}
#endif

		return(sock);
	}

	return(-1); //should never reach here, but just in case
}

int wsp_get(int sock, char *gwip, char *urlStr, Boolean remember, Err *error, GlobalsType *g )
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


    	// See WSP.PDF for more info
    	outbuf[i++]=0x01; // Transaction ID
    	outbuf[i++]=WSP_PDU_GET;
    	//escapeUrl(urlStr); 
    	enc_uintvar(outbuf,&i,gstrlen(urlStr)); // URIlen , uintvar !!
    	gstrcopy((outbuf+i), urlStr);
	if(remember)
		gstrcopy(g->WapLastUrl,urlStr);
    	i+= gstrlen(urlStr);

    	// Start of headers
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
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x94; // 14 application/vnd.wap.wmlc
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xA9; // 29 application/vnd.wap.wbxml
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x95; // 15 application/vnd.wap.wmlscriptc
	outbuf[i++]=0x80; // 00 Accept
	gstrcopy(&(outbuf[i]),"image/bmp");
       			i+= gstrlen("image/bmp");
       			outbuf[i++]=0x00;
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x9d; // 1d image/gif
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xA0; // 20 image/png
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x9E; // 1E image/jpeg
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xA1; // 21 image/vnd.wap.wbmp
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xBB; // 3B application/xhtml+xml
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xBC; // 3C application/wml+xml
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xC5; // 45 application/vnd.wap.xhtml+xml
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x82; // 02 text/html
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x80; // 00 all
    	if (g->conn.proxyuser && gstrlen(g->conn.proxyuser) && g->conn.proxypass && gstrlen(g->conn.proxypass)) {
      		outbuf[i++]=0xA1; // 21 Proxy-Authorization
      		if ( (gstrlen(g->conn.proxyuser) + gstrlen(g->conn.proxypass) + 3) >= 31 ) {
	        	outbuf[i++]=0x1F;
      		}
      		enc_uintvar(outbuf,&i,gstrlen(g->conn.proxyuser) + gstrlen(g->conn.proxypass) + 3);
      		outbuf[i++]=0x80;
      		gstrcopy(&(outbuf[i]),g->conn.proxyuser);
       			i+= gstrlen(g->conn.proxyuser);
       			outbuf[i++]=0x00;
      		gstrcopy(&(outbuf[i]),g->conn.proxypass);
       			i+= gstrlen(g->conn.proxypass);
       			outbuf[i++]=0x00;
    	}

	// TODO: what if user does not want a password, i.e. horrible security??
    	if (g->AuthUser && g->AuthPass && gstrlen(g->AuthUser) && gstrlen(g->AuthPass)) {
      		outbuf[i++]=0x87; // 07 Authorization
      		if ( (gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3) >= 31 ) {
	        	outbuf[i++]=0x1F;
      		}
      		enc_uintvar(outbuf,&i,gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3);
      		outbuf[i++]=0x80;
      		gstrcopy(&(outbuf[i]),g->AuthUser);
       			i+= gstrlen(g->AuthUser);
       			outbuf[i++]=0x00;
      		gstrcopy(&(outbuf[i]),g->AuthPass);
       			i+= gstrlen(g->AuthPass);
       			outbuf[i++]=0x00;

      		//Free(g->AuthUser);
      		//Free(g->AuthPass);
      		//g->AuthUser = NULL;
      		//g->AuthPass = NULL;
    	}
    	outbuf[i++]=0xA9; // 29 User Agent
    
	if (g_prefs.altAgent == true) {
		gstrcopy(&(outbuf[i]),AgentString[g_prefs.agentNum + 1]);
	    	i+= gstrlen(AgentString[g_prefs.agentNum + 1]);
	} else {
		gstrcopy(&(outbuf[i]),AgentString[0]);
	    	i+= gstrlen(AgentString[0]);
	}
    	outbuf[i++]=0x00;
    
	if (!g_prefs.disableUAProf) {
    		outbuf[i++]=0xB5; // 35 Profile
    		gstrcopy(&(outbuf[i]), UNIVERSE_UA_PROF);
    		i+= gstrlen(UNIVERSE_UA_PROF);
    			outbuf[i++]=0x00;
	}

    	outbuf[i++]=0x89; // 09 Connection
    
    	gstrcopy(&(outbuf[i]),"Encoding-Version");
		i+= gstrlen("Encoding-Version");
		outbuf[i++]=0x00;
    	gstrcopy(&(outbuf[i]),"Encoding-Version");
		i+= gstrlen("Encoding-Version");
		outbuf[i++]=0x00;
    	gstrcopy(&(outbuf[i]),"1.3");
		i+= gstrlen("1.3");
		outbuf[i++]=0x00;
    
    	AppNetTimeout = 120 * SysTicksPerSecond();//120 seconds
    
    	if (sendto(sock,&outbuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0) {
        	MyErrorFunc("\nCould not send data to the server.\nReason:\n", WspHostErrStr());
		*error = errno;
	} else {
		*error = errNone;
	}

	Free(outbuf);

    	return sock;
}

int wsp_post(int sock, char *gwip, char *urlStr, Boolean remember, Err *error, GlobalsType *g )
// Boolean remember: true if we are loading a WML page (and not an image), so we set gLastURL to current URL, needed for relative links later on
{
	Char		*outbuf = NULL;
    	UInt16		i = 0;
    	Int32		AppNetTimeout;
    	UInt16		agent = 0, profLen = 0;

	*error = errNone;

	outbuf = Malloc(WAP_MAX_PACKET);
	if (!outbuf) {
		*error = errWapNoMemory;
		MyErrorFunc("Failed to request data from the server.", "Reason: Out of memory");
		return sock;
	}

	MemSet(outbuf, WAP_MAX_PACKET, 0);
    

	if (g_prefs.altAgent)
		agent = g_prefs.agentNum + 1;


	// See WSP.PDF for more info
	outbuf[i++]=0x01; // Transaction ID
	outbuf[i++]=WSP_PDU_POST; //POST
	//escapeUrl(urlStr); //convert ' ' in string to '+'
	enc_uintvar(outbuf,&i,gstrlen(urlStr)); // URIlen , uintvar !!

	if (!g_prefs.disableUAProf) {
    		profLen = gstrlen(UNIVERSE_UA_PROF) + 2;
	} else {
		profLen = 0;
	}

	if ( gstrlen(g->conn.proxyuser) != 0 ) {
		if ( (gstrlen(g->conn.proxyuser) + gstrlen(g->conn.proxypass) + 3) >= 31 ) {
			if ( (gstrlen(g->AuthUser) != 0) && (g->AuthUser != NULL) && (gstrlen(g->AuthPass) != 0) && (g->AuthPass != NULL) ) {
				if ( (gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3) >= 31 ) {
					enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->conn.proxyuser)+gstrlen(g->conn.proxypass)+gstrlen(g->AuthUser)+gstrlen(g->AuthPass)+38+4+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+1+gstrlen("image/bmp")+14+profLen); // Header & ContentType , uintvar !!
				} else {
					enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->conn.proxyuser)+gstrlen(g->conn.proxypass)+gstrlen(g->AuthUser)+gstrlen(g->AuthPass)+37+4+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+1+gstrlen("image/bmp")+14+profLen); // Header & ContentType , uintvar !!
				}
			} else {
				enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->conn.proxyuser)+gstrlen(g->conn.proxypass)+33+4+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+1+gstrlen("image/bmp")+14+profLen); // Header & ContentType , uintvar !!
			}
		} else {
			if ( (gstrlen(g->AuthUser) != 0) && (g->AuthUser != NULL) && (gstrlen(g->AuthPass) != 0) && (g->AuthPass != NULL) ) {
				if ( (gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3) >= 31 ) {
					enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->conn.proxyuser)+gstrlen(g->conn.proxypass)+gstrlen(g->AuthUser)+gstrlen(g->AuthPass)+37+4+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+1+gstrlen("image/bmp")+14+profLen); // Header & ContentType , uintvar !!
				} else {
					enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->conn.proxyuser)+gstrlen(g->conn.proxypass)+gstrlen(g->AuthUser)+gstrlen(g->AuthPass)+36+4+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+1+gstrlen("image/bmp")+14+profLen); // Header & ContentType , uintvar !!
				}
			} else {
				enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->conn.proxyuser)+gstrlen(g->conn.proxypass)+32+4+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+1+gstrlen("image/bmp")+14+profLen); // Header & ContentType , uintvar !!
			}
		}
	} else {
		if ( (gstrlen(g->AuthUser) != 0) && (g->AuthUser != NULL) && (gstrlen(g->AuthPass) != 0) && (g->AuthPass != NULL) ) {
			if ( (gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3) >= 31 ) {
				enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->AuthUser)+gstrlen(g->AuthPass)+33+4+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+gstrlen("image/bmp")+14+profLen); // Header & ContentType , uintvar !!
			} else {
				enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+gstrlen(g->AuthUser)+gstrlen(g->AuthPass)+32+4+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+gstrlen("image/bmp")+14+profLen); // Header & ContentType , uintvar !!
			}
		} else {
			enc_uintvar(outbuf,&i,gstrlen(AgentString[agent])+28+4+(2*(gstrlen("Encoding-Version")))+gstrlen("1.3")+gstrlen("image/bmp")+14+profLen); // Header & ContentType , uintvar !!
		}
	}

	gstrcopy((outbuf+i), urlStr);
	if(remember)
		gstrcopy(g->WapLastUrl,urlStr);
	i+= gstrlen(urlStr);
	// Start of headers

	outbuf[i++]=0x92; // 12 x-www-form-urlencoded

	outbuf[i++]=0x83; // 03 Accept Language
	outbuf[i++]=0x99; // 19 = en
	outbuf[i++]=0x81; // 01 Accept Charset
	outbuf[i++]=0xEA; // 6A = UTF-8
	outbuf[i++]=0x81; // 01 Accept Charset
	outbuf[i++]=0x83; // 03 us-ascii
	outbuf[i++]=0x81; // 01 Accept Charset
	outbuf[i++]=0x84; // 04 iso-8859-1 
	outbuf[i++]=0xbb; // 01 Accept Charset
	outbuf[i++]=0x03; // 03 us-ascii
	outbuf[i++]=0x02; // 02 Accept Encoding
	outbuf[i++]=0x03; // 03 Token Text ???????
	outbuf[i++]=0xE8; // 68 F6
	outbuf[i++]=0x80; // 00 Accept
	outbuf[i++]=0x94; // 14 application/vnd.wap.wmlc
	outbuf[i++]=0x80; // 00 Accept
	outbuf[i++]=0xA9; // 29 application/vnd.wap.wbxml
	outbuf[i++]=0x80; // 00 Accept
	outbuf[i++]=0x95; // 15 application/vnd.wap.wmlscriptc
	outbuf[i++]=0x80; // 00 Accept
	gstrcopy(&(outbuf[i]),"image/bmp");
       			i+= gstrlen("image/bmp");
       			outbuf[i++]=0x00;
	outbuf[i++]=0x80; // 00 Accept
	outbuf[i++]=0x9d; // 1d image/gif
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xA0; // 20 image/png
	outbuf[i++]=0x80; // 00 Accept
	outbuf[i++]=0x9E; // 1E image/jpeg
	outbuf[i++]=0x80; // 00 Accept
	outbuf[i++]=0xA1; // 21 image/vnd.wap.wbmp
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xBB; // 3B application/xhtml+xml
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xBC; // 3C application/wml+xml
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0xC5; // 45 application/vnd.wap.xhtml+xml
    	outbuf[i++]=0x80; // 00 Accept
    	outbuf[i++]=0x82; // 02 text/html
	outbuf[i++]=0x80; // 00 Accept
	outbuf[i++]=0x80; // 00 all
	if (g->conn.proxyuser && gstrlen(g->conn.proxyuser) && g->conn.proxypass && gstrlen(g->conn.proxypass)) {
		outbuf[i++]=0xA1; // 21 Proxy-Authorization
		if ( (gstrlen(g->conn.proxyuser) + gstrlen(g->conn.proxypass) + 3) >= 31 ) {
	        	outbuf[i++]=0x1F;
		}
		enc_uintvar(outbuf,&i,gstrlen(g->conn.proxyuser) + gstrlen(g->conn.proxypass) + 3);
		outbuf[i++]=0x80;
			gstrcopy(&(outbuf[i]),g->conn.proxyuser);
			i+= gstrlen(g->conn.proxyuser);
		outbuf[i++]=0x00;
			gstrcopy(&(outbuf[i]),g->conn.proxypass);
			i+= gstrlen(g->conn.proxypass);
		outbuf[i++]=0x00;
	}

	// TODO: what if user does not want a password, i.e. horrible security??
		if (g->AuthUser && g->AuthPass && gstrlen(g->AuthUser) && gstrlen(g->AuthPass)) {
		outbuf[i++]=0x87; // 07 Authorization
		if ( (gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3) >= 31 ) {
			outbuf[i++]=0x1F;
		}
		enc_uintvar(outbuf,&i,gstrlen(g->AuthUser) + gstrlen(g->AuthPass) + 3);
		outbuf[i++]=0x80;
			gstrcopy(&(outbuf[i]),g->AuthUser);
			i+= gstrlen(g->AuthUser);
		outbuf[i++]=0x00;
			gstrcopy(&(outbuf[i]),g->AuthPass);
			i+= gstrlen(g->AuthPass);
		outbuf[i++]=0x00;

		//Free(g->AuthUser);
		//Free(g->AuthPass);
		//g->AuthUser = NULL;
		//g->AuthPass = NULL;
	}
	outbuf[i++]=0xA9; // 29 User Agent
    
	gstrcopy(&(outbuf[i]),AgentString[agent]);
		i+= gstrlen(AgentString[agent]);
		outbuf[i++]=0x00;

	if (!g_prefs.disableUAProf) {
    		outbuf[i++]=0xB5; // 35 Profile
    		gstrcopy(&(outbuf[i]), UNIVERSE_UA_PROF);
    			i+= gstrlen(UNIVERSE_UA_PROF);
    			outbuf[i++]=0x00;
	}

	outbuf[i++]=0x89; // 09 Connection
    
	gstrcopy(&(outbuf[i]),"Encoding-Version");
		i+= gstrlen("Encoding-Version");
		outbuf[i++]=0x00;
	gstrcopy(&(outbuf[i]),"Encoding-Version");
		i+= gstrlen("Encoding-Version");
		outbuf[i++]=0x00;
	gstrcopy(&(outbuf[i]),"1.3");
		i+= gstrlen("1.3");
		outbuf[i++]=0x00;

	
	gstrcopy(&(outbuf[i]),g->postBuf);
		i+= gstrlen(g->postBuf);
	
	g->post = false;
	if (g->postBuf)
		Free(g->postBuf);
    
	AppNetTimeout = 120 * SysTicksPerSecond();//120 seconds
    
	if (sendto(sock,&outbuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0) {
		MyErrorFunc("\nCould not send data to the server.\nReason:\n", WspHostErrStr());
		*error = errno;
	} else {
		*error = errNone;
	}

	Free(outbuf);

	return sock;
}


// returns socket
int wapGetUrl(dbConnConnection *conn, char *urlstr, Boolean remember, Err *error, GlobalsType *g )
// Boolean remember: true if we are loading a WML page (and not an image), so we set gLastURL to current URL, needed for relative links later on
{
    int         sock = -1;
    int		tmpSock;


    sock = Connect2Server(conn->ipaddress,urlstr,g);       
    if (sock == -1) {
	*error = errWapNoConnection;
        return(-1);
    }

    if ((conn->connectionType == 'O') && (g->wtpState != WTP_Connected)) { 
	WTPConnect(sock,conn->ipaddress,urlstr,remember,g);
	if (g->wtpState == WTP_Disconnected) {
		g->wtpState = WTP_Disconnected;
		*error = errWapNoConnection;
		return (sock);
	}

	//TODO: retreive conntect
    }

    if (g->conn.connectionType == 'L') {
    	if (g->post == true) {
    		tmpSock = wsp_post(sock,conn->ipaddress,urlstr,remember,error,g);
    	} else {
    		tmpSock = wsp_get(sock,conn->ipaddress,urlstr,remember,error,g);
    	}
    } else {
	if (g->post == true) {
    		tmpSock = wtp_post(sock,conn->ipaddress,urlstr,remember,error,g);
    	} else {
		tmpSock = wtp_get(sock,conn->ipaddress,urlstr,remember,error,g);
	}
    }


    return(sock);
}

// returns socket
int wapConnect(dbConnConnection *conn, char *urlstr, Boolean remember, int wtpFail, GlobalsType *g )
// Boolean remember: true if we are loading a WML page (and not an image), so we set gLastURL to current URL, needed for relative links later on
{
    int         sock = -1;

    sock = Connect2Server(conn->ipaddress,urlstr,g);       
    if (sock == -1) {
    	return(-1);
    }

    if ((conn->connectionType == 'O') && (g->wtpState != WTP_Connected)) { 
	wtpFail = WTPConnect(sock,conn->ipaddress,urlstr,remember,g);
	if (g->wtpState == WTP_Disconnected) {
		g->retransmission = true;
		g->wtpState = WTP_Disconnected;
		return (sock);
	}
    }

    return(sock);
}

void reWtpGet (int sock, dbConnConnection *conn, char *urlstr, Boolean remember, Err *error, GlobalsType *g )
{
	//wtp_get(sock,conn->ipaddress,urlstr,remember,g);
	if (g->post == true) {
    		wtp_post(sock,conn->ipaddress,urlstr,remember,error,g);
    	} else {
		wtp_get(sock,conn->ipaddress,urlstr,remember,error,g);
	}
}


int WSPDisconnect (Int16 sock, GlobalsType *g)
{   unsigned char obuf[500];
    UInt16 i = 0;


	if (g->sid != 0) {

    		g->old_tid = g->tid;
    		g->tid = next_tid(g->old_tid);
    		g->tid_new = (g->tid < g->old_tid);  /* Did we wrap? */

		// Send Disconnect
		obuf[i++] = 0x0E;
 		obuf[i++] = 0x00;
		obuf[i++] = g->tid;
 		obuf[i++] = 0x10;
 		obuf[i++] = WSP_Disconnect;
        	enc_uintvar(obuf,&i,g->sid);

		if (sendto(sock,&obuf,i,0, (struct sockaddr *) &g->RmtAddress,sizeof(g->RmtAddress))<0)
        	  MyErrorFunc("\nCould not send data to the server.\nReason:\n", WspHostErrStr());
	}

	g->wtpState = WTP_Disconnected;

	return sock;
}

