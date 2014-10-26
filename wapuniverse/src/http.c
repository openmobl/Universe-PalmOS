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
/* tag: http protocol implementation file for PalmHTTP
 *
 * PalmHTTP - an HTTP library for Palm devices
 * 
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is PalmHTTP.
 *
 * The Initial Developer of the Original Code is
 * Mike Rowehl "Reverend" <miker@bitsplitter.net>.
 * Portions created by the Initial Developer are Copyright (C) 2003, 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Reverend <miker@bitsplitter.net>
 *
 * ***** END LICENSE BLOCK *****
 */

#include 	<PalmOS.h>
#include 	<Unix/sys_socket.h>

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

static Err errno;
static Boolean needLength = true;

#define HTTP_POST_METH 		"POST "
#define HTTP_GET_METH 		"GET "
#define HTTP_HEAD_METH 		"HEAD "

#ifdef USE_HTTP_1_1
#define HTTP_VERSION 		" HTTP/1.1"
#else
#define HTTP_VERSION 		" HTTP/1.0"
#endif

#define HTTP_HOST_HDR 		"Host: "
#define HTTP_CONTENTLENGTH_HDR 	"Content-Length: "
#define HTTP_LINE_ENDING 	"\r\n"
#define HTTP_CONTENTTYPE_HDR 	"Content-Type: "
#define HTTP_LOCATION_HDR	"Location: "
#define HTTP_CONTENT_LOCATION_HDR	"Content-Location: "
#define HTTP_CACHE_CONTROL_HDR	"Cache-Control: "
#define HTTP_USERAGENT_LINE 	"User-Agent: "
#define HTTP_CONNECTION		"Connection: close"
//#define HTTP_ACCEPT_ENC		"Accept-Encoding: "
#define HTTP_ACCEPT_LANG	"Accept-Language: en;g=0.8"
#define HTTP_ACCEPT_CHARSET	"Accept-Charset: utf-8;q=1.0,us-ascii,iso-8859-1;g=0.5"
#define HTTP_X_WAP_PROFILE	"X-Wap-Profile: \"" ## UNIVERSE_UA_PROF ## "\""
#define HTTP_ACCEPT		"Accept: application/vnd.wap.wmlc, application/vnd.wap.wbxml, application/vnd.wap.wmlscriptc, " ## \
				"application/xhtml+xml, application/wml+xml, application/vnd.wap.xhtml+xml, application/rss+xml, text/html, text/x-html, " ## \
		 		"text/vnd.wap.wml, text/plain, text/xml, image/vnd.wap.wbmp, image/jpg, image/jpeg, image/bmp, " ## \
				"image/gif, image/pbmp, image/png, */*"
#define HTTP_PROXY_AUTH		"Proxy-Authorization: Basic "
#define HTTP_WEB_AUTH		"Authorization: Basic "
#define HTTP_REALM_HDR		"WWW-Authenticate: "
#define HTTP_COOKIE_HDR		"Cookie: "
#define HTTP_SET_COOKIE_HDR	"Set-Cookie: "

#define HTTPLIB_TYPE 		'TEMP'
#define HTTPDATA_STORE		'Wapu'
#define HTTPDATA_FILE		"Universe-HTTP-Cache"

#define DEFAULT_CONTENT_TYPE 	"text/plain"

/*
 * The creator ID will be appended to this name in order to create the full
 * name of the temp database.  That means the length needs to be 5 longer than
 * the string here, 4 chars for the creator id and 1 for the null terminator
 */
#define HTTPLIB_NAME "HTTPLib_SA_"
#define HTTPLIB_NAME_LEN (16)

/* Size of strings used to hold ascii conversion of numbers */
#define CLS_LENGTH (11)


/*
 * There are still some servers that behave poorly on certain combinations of
 * valid network operations (if you don't feed them enough data for them to 
 * get what they want in a single network read).  Since it's also slightly more
 * efficient to form the image of the headers and send them in larger network
 * writes, we use a scratch record in a database to accumulate the request line
 * and headers and then send off the whole thing.  The struct has an error flag
 * field that's used to short circuit requests, so that we can just push data 
 * in without checking to see what the status is.  And then at the end we 
 * check once to see if the whole batch was successful.
 */

typedef struct ScratchRec_struct {
    MemHandle handle;
    UInt16 index;
    UInt16 size;
    UInt8 errFlag;
} ScratchRec;


/*
 * Local prototypes (private to this file)
 */

/* General utility */
//static char *StrDup( char *string );

/* Temp scratch area */
static int StartScratchArea( ScratchRec *scratch );
static void AddToScratch( ScratchRec *scratch, char *text, int length );
#define AddScratchLine( scratch, text ) \
          AddToScratch( scratch, text, StrLen( text ) )

/* Parse read buffer handling */
static UInt32 BufSizeRemaining( HTTPParse *parse );
static void BufConsumeToPointer( HTTPParse *parse, char *newFirst );
static char *NextBufByte( HTTPParse *parse );
static Int16 FillReadBuff( NetSocketRef sock, HTTPParse *parse, Int32 maxTimer );

/* Parse functions */
static int MarkEOL( HTTPParse *parse );
static char *ParseResponseCode( char *start );
static void ParseResponseLine( HTTPParse *parse );
static void ParseHeaders( HTTPParse *parse );
static void ParseBody( HTTPParse *parse );

static void MultipartParseEngine( HTTPParse *parse, Char *data );
static Int32 dataUsed = 0;


/* Network cover */
static int SendAll( NetSocketRef sock, char *data, UInt32 length, HTTPParse *parse );


/*
 * Private globals
 */

static DmOpenRef gHttpLib = NULL;

//static Boolean isRedir = false;
Int16 http_response = 200;
Char http_realm[200];
UInt32 http_clength = 0;
UInt32 http_received = 0;

typedef struct
{
  Int16		code;
  unsigned char wspVal;
  char 		*error;
} HTTPResponseCode;

static HTTPResponseCode code_list[] =
{
    { 300 , 0x30 , "300: Multiple choices"              },
	{ 301 , 0x31 , "301: Moved permanently"             },
	{ 302 , 0x32 , "302: Moved temporarily"             },
    { 303 , 0x33 , "303: See other"                     },
    { 304 , 0x34 , "304: Not modified"                  },
    { 305 , 0x35 , "305: Use proxy"                     },
    { 307 , 0x37 , "307: Temporary Redirect"            },
    { 400 , 0x40 , "400: Bad request"                   },
    { 401 , 0x41 , "401: Unauthorized"                  },
    { 402 , 0x42 , "402: Payment required"              },
    { 403 , 0x43 , "403: Forbidden"                     },
    { 404 , 0x44 , "404: Not found"                     },
    { 405 , 0x45 , "405: Method not Allowed"            },
    { 406 , 0x46 , "406: Not acceptable"                },
    { 407 , 0x47 , "407: Proxy authentication required" },
    { 408 , 0x48 , "408: Request timeout"               },
    { 409 , 0x49 , "409: Conflict"                      },
    { 410 , 0x4A , "410: Gone"                          },
    { 411 , 0x4B , "411: Length required"               },
    { 412 , 0x4C , "412: Precondition failed"           },
    { 413 , 0x4D , "413: Request entity too large" 		},
    { 414 , 0x4E , "414: Request-URI too large" 		},
    { 415 , 0x4F , "415: Unsupported media type" 		},
    { 416 , 0x50 , "416: Request Range Not Satisfiable" },
    { 417 , 0x51 , "417: Expectation Failed"            },
    { 500 , 0x60 , "500: Internal server error" 		},
    { 501 , 0x61 , "501: Not implemented"               },
    { 502 , 0x62 , "502: Bad Gateway"                   },
    { 503 , 0x63 , "503: Service unavailable"           },
    { 504 , 0x64 , "504: Gateway Timeout"               },
    { 505 , 0x65 , "505: HTTP version not supported" 	},
	{ NULL,	0x00 , "000: Unknown!!"                     }
};

#define HTTP_CODE_LIST_MAX	31

char *HostErrStr(Err err)
{
    static char msgstr[100];

    if (errno != 0)
        SysErrString(err, msgstr, 50);
    else
        msgstr[0] = '\0';

    return(msgstr);
}

/*
 * Name:   HTTPLibStart()
 * Args:   creator - creator ID to use for the database
 * Return: 0 on success, -1 on error
 * Desc:   Creates the temp database used by the library.  I haven't registered
 *         a creator ID for the library itself, so the calling app should pass
 *         it's own creator ID and this function will create a database under
 *         that ID.
 */

int HTTPLibStart( UInt32 creator )
{
    Err error;
    int recs;
    int i;
    char dbName[HTTPLIB_NAME_LEN];
    char *tmpPtr;

    gHttpLib = DmOpenDatabaseByTypeCreator( HTTPLIB_TYPE, creator,
                                            dmModeReadWrite ); 
    if ( !gHttpLib ) { 
        StrCopy( dbName, HTTPLIB_NAME );
        tmpPtr = dbName + StrLen( dbName );
        *tmpPtr++ = (char)((creator & 0xff000000) >> 24);
        *tmpPtr++ = (char)((creator & 0x00ff0000) >> 16);
        *tmpPtr++ = (char)((creator & 0x0000ff00) >> 8);
        *tmpPtr++ = (char)((creator & 0x000000ff));
        *tmpPtr = '\0';

        error = DmCreateDatabase( 0, dbName, creator, HTTPLIB_TYPE,
                                  false ); 
        if ( error ) {
            return -1;
        }

        gHttpLib = DmOpenDatabaseByTypeCreator( HTTPLIB_TYPE, creator,
                                                dmModeReadWrite ); 
        if ( !gHttpLib ) {
            return -2;
        }
    } else {
        recs = DmNumRecords( gHttpLib );
        for ( i = 0; i < recs; i++ ) {
            DmRemoveRecord( gHttpLib, 0 );
        }
    }

    return 0;
}


/*
 * Name:   HTTPLibStop()
 * Args:   none
 * Return: none
 * Desc:   Attempts to free up the temporary storage database records and
 *         shut the open database.
 */

void HTTPLibStop( void )
{
    int recs;
    int i;

    if ( gHttpLib != NULL ) {
        recs = DmNumRecords( gHttpLib );
        for ( i = 0; i < recs; i++ ) {
            DmRemoveRecord( gHttpLib, 0 );
        }
        DmCloseDatabase( gHttpLib );
        gHttpLib = NULL;
    }
}


/*
 * Name:   StrDup()
 * Args:   string - string to duplicate
 * Return: Newly allocated string on success
 *         NULL on error
 * Desc:   Just like the *nix version strdup(), duplicates a string into heap
 *         storage.
 */

/*static char *StrDup( char *string )
{
    Int16 strSize;
    char *returnStr;

    strSize = StrLen( string ) + 1;
    returnStr = Malloc( strSize );
    if ( returnStr == NULL ) {
        return NULL;
    }

    StrCopy( returnStr, string );
    return returnStr;
}*/

/*NetSocketRef NetUTCP2Open (Char* hostName, Char* serviceName, Int16 port)
{
	int				s;
	Err 				h_errno;
    	UInt32 				AppNetTimeout;
#ifdef __palmos__
    	NetHostInfoBufType 		AppHostInfo;     // DO NOT DELETE !!
	// without this AppHostInfo, gethostbyname() corrupts memory!!!!!!!
#endif
	struct hostent 			*hp = NULL;
	struct NetSocketAddrINType	server;

	if ((hp = gethostbyname(hostName))) {
    		memset((char *) &server,0, sizeof(server));

    		MemMove((char *) &server.addr, hp->h_addr, hp->h_length);
    		server.family = hp->h_addrtype;
    		server.port = (unsigned short) htons( port );
	} else {
    		return -1;
	}

  	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    		return -1;

  	if (connect(s, &server, sizeof(server)) < 0) 
   		return -1;

	return s;
}*/

/*NetSocketRef NetUTCP2Open (Char* hostName, Char* serviceName, Int16 port)
{
    unsigned long	host;
    int			sock;
    Err			error;
    GlobalsType 	*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	showProgress(g);

	if (StringToIPAddr(hostName, &host) != 1) {
		if (g->isImage != true) MyErrorFunc("Host lookup failed. Try again later.",NULL);
		return -1;
	}

	memset((char *) &(g->RmtAddress), 0, sizeof(g->RmtAddress));
	g->RmtAddress.sin_family = AF_INET;

	g->RmtAddress.sin_port = htons((UInt16)port);
	g->RmtAddress.sin_addr.s_addr = host;

	//sock = NetLibSocketOpen(AppNetRefnum, AF_INET, SOCK_STREAM, 0, REPLY_TIMEOUT * SysTicksPerSecond(), &error);
	sock = NetLibSocketOpen(AppNetRefnum, netSocketAddrINET, netSocketTypeStream, netSocketProtoIPTCP, 10 * SysTicksPerSecond(), &error);
	if (error != 0) {
		//MyErrorFunc("\nCould not create communications socket.\nReason:\n",GetErrStr(error));
		if (g->isImage != true) MyErrorFunc("Failed to connect to the remote host or proxy. Try again later.", NULL);
		return -2;
	}

	showProgress(g);

	NetLibSocketConnect(AppNetRefnum, sock, (NetSocketAddrType*)&g->RmtAddress, sizeof(g->RmtAddress), REPLY_TIMEOUT * SysTicksPerSecond(), &error);
	if (error != 0) {
		//MyErrorFunc("\nCould not connect communications socket.\nReason:\n",GetErrStr(error));
		if (g->isImage != true) MyErrorFunc("Failed to connect to the remote host or proxy. Try again later.", NULL);
		return -2;
	}

	g->LclAddress.sin_family = AF_INET;
	g->LclAddress.sin_port   = htons(880);
	g->LclAddress.sin_addr.s_addr = INADDR_ANY;

	showProgress(g);

	http_response = 200;

	return sock;
}*/

/*NetSocketRef NetUTCP2Open(const Char * host, const Char * service, Int16 port)
{
	//struct servent		tcp_serv_info;	// from getservbyname()
	struct sockaddr_in	tcp_srv_addr;	// server's Internet socket addr
	NetSocketRef		fd;
	UInt32			inaddr;
	//struct servent		*sp;
	struct hostent		*hp;
	struct hostent		tcp_host_info;	// from gethostbyname()
	GlobalsType 		*g;

	Err			h_errno=0;
	// Global used for the gethostbyname and gethostbyaddr functions
	NetHostInfoBufType	AppHostInfo;
	// Global used for the getservbyname
	//NetServInfoBufType	AppServInfo;
	UInt32 			AppNetTimeout = 4 * SysTicksPerSecond();

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);



	 // Initialize the server's Internet address structure.
	 // We'll store the actual 4-byte Internet address and the
	 // 2-byte port# below.
	 
	bzero((char *) &tcp_srv_addr, sizeof(tcp_srv_addr));
	tcp_srv_addr.sin_family = AF_INET;

	//if (service != NULL) {
	//	if ( (sp = getservbyname(service, "tcp")) == NULL) {
	//		if (g->isImage != true) MyErrorFunc("Generic network error.", "getservbyname failed");
	//		return(-1);
	//	}
	//	tcp_serv_info = *sp;								// structure copy 
	//	if (port > 0)
	//		tcp_srv_addr.sin_port = htons(port); 	// caller's value 
	//	else
	//		tcp_srv_addr.sin_port = sp->s_port;		// service's value 
	//} else {
		if (port <= 0) {
			if (g->isImage != true) MyErrorFunc("Invalid port defined.", NULL);
			return(-1);
		}
		tcp_srv_addr.sin_port = htons(port);
	//}



	 // First try to convert the host name as a dotted-decimal number.
	 // Only if that fails do we call gethostbyname().
	 
	if ( (inaddr = inet_addr(host)) != INADDR_NONE) {    // it's dotted-decimal
		bcopy((char *) &inaddr, (char *) &tcp_srv_addr.sin_addr,
					sizeof(inaddr));
		tcp_host_info.h_name = NULL;
	} else {
		if ( (hp = gethostbyname(host)) == NULL) {
			if (g->isImage != true) MyErrorFunc("Host lookup failed. Try again later.", WspHostErrStr());
			return(-1);
		}
		tcp_host_info = *hp;	// found it by name, structure copy
		bcopy(hp->h_addr, (char *) &tcp_srv_addr.sin_addr,
			hp->h_length);
	}


	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		if (g->isImage != true) MyErrorFunc("Failed to prepare network communications. Try again later.", WspHostErrStr());
		return(-2);
	}


	 // Connect to the server.
	if (connect(fd, (struct sockaddr *) &tcp_srv_addr,
						sizeof(tcp_srv_addr)) < 0) {
		close(fd);
		if (g->isImage != true) MyErrorFunc("Failed to connect to the remote host or proxy. Try again later.", WspHostErrStr());
		return(-2);
	}

	showProgress(g);

	http_response = 200;

	return(fd);
}*/


/*
 * Name:   CreateURL()
 * Args:   transport - the transport to use when communicating
 *         host - IP address or DNS name
 *         port - port number to connect to on host
 *         path - path to pass in request
 * Return: Newly allocated struct on success
 *         NULL on error
 * Desc:   Creates a new URL encoding struct and populates the members with
 *         the values filled in.  It allocates heap based versions of the data
 *         linked into the fields of the struct, so it's very necessary to 
 *         call DestroyURL() when you're finished with the struct.
 */

URLTarget *CreateURL( URLTransports transport, char *host, UInt16 port,
                      char *path )
{
    URLTarget *url;
    char *hostTmp;
    char *pathTmp;

    url = (URLTarget *)Malloc( sizeof( URLTarget ) );
    if ( url == NULL ) {
        return NULL;
    }

    url->host = NULL;
    url->path = NULL;

    hostTmp = StrDup( host );
    if ( hostTmp == NULL ) {
        DestroyURL( url );
        return NULL;
    }
    url->host = hostTmp;

    pathTmp = StrDup( path );
    if ( pathTmp == NULL ) {
        DestroyURL( url );
        return NULL;
    }
    url->path = pathTmp;

    url->transport = transport;
    url->port = port;

    return url;
}


/*
 * Name:   ParseURL()
 * Args:   text - the text to attempt to parse as a URL
 * Return: Newly allocated struct on success
 *         NULL on error
 * Desc:   Attempts to parse out the information from a human readable URL and
 *         store it in a struct for later use.
 */

URLTarget *ParseURL( char *text )
{
    URLTransports transport;
    char *hostStart;
    char *hostEnd;
    char *hostTmp;
    char *path;
    int hostLen;
    UInt16 port;
    URLTarget *url;

    if ( StrNCompare( text, "http://", 7 ) == 0 ) {
        transport = URLT_http;
        hostStart = text + 7;
    } else if ( StrNCompare( text, "https://", 8 ) == 0 ) {
        transport = URLT_https;
        hostStart = text + 8;
    } else {
        return NULL;
    }

    hostEnd = hostStart;
    while ( (*hostEnd != '/') && (*hostEnd != ':') && (*hostEnd) ) {
        hostEnd++;
    }

    if ( *hostEnd == ':' ) {
        port = StrAToI( hostEnd + 1 );
        path = hostEnd + 1;
        while ( *path && (*path != '/' ) ) {
            path++;
        }
        if ( *path == '\0' ) {
            path = "/";
        }
    } else if ( *hostEnd == '\0' ) {
        path = "/";
	if (transport == URLT_https)
		port = 443;
	else
        	port = 80;
    } else {
        path = hostEnd;
	if (transport == URLT_https)
		port = 443;
	else
        	port = 80;
    }

    hostLen = (hostEnd - hostStart) + 1;
    hostTmp = Malloc( hostLen );
    if ( hostTmp == NULL ) {
        return NULL;
    }
    StrNCopy( hostTmp, hostStart, hostLen - 1 );
    hostTmp[hostLen - 1] = '\0';

    url = CreateURL( transport, hostTmp, port, path );
    if ( url == NULL ) {
        Free( hostTmp );
        return NULL;
    }

    Free( hostTmp );

    return url;
}


/*
 * Name:   DestroyURL()
 * Args:   url - url structure to free
 * Return: none
 * Desc:   Frees the URL structure and any child storage (path, host, or any
 *         other strings that get added in)
 */

void DestroyURL( URLTarget *url )
{
    if ( url == NULL )
	return;

    if ( url->host != NULL ) {
        Free( url->host );
        url->host = NULL;
    }
    if ( url->path != NULL ) {
        Free( url->path );
        url->path = NULL;
    }

    Free( url );
}


/*
 * Name:   CreatePostReq()
 * Args:   none
 * Return: Newly allocated post request on success
 *         NULL on error
 * Desc:   Allocates a new request structure on the heap and initializes the
 *         fields to sane values.
 */

PostReq *CreatePostReq( void )
{
    PostReq *req;

    req = (PostReq *)Malloc( sizeof(PostReq) );
    if ( req == NULL ) {
        return NULL;
    }

    req->contentType = StrDup( DEFAULT_CONTENT_TYPE );
    req->content = NULL;
    req->contentOwner = 0;
    req->extraHeaders = NULL;

    return req;
}


/*
 * Name:   SetReqContentType()
 * Args:   request - request to modify the content type of
 *         type - the string to set the content type to
 * Return: 0 on success
 *         negative on failure
 * Desc:   Removes the old content type string and duplicates the new one.
 */

int SetReqContentType( PostReq *request, char *type )
{
    if ( request->contentType != NULL ) {
        Free( request->contentType );
    }

    request->contentType = StrDup( type );
    if ( request->contentType != NULL ) {
        return 0;
    }

    return -1;
}


/*
 * Name:   SetReqContent()
 * Args:   request - the request to set the content of
 *         content - the content to record
 *         contentLen - number of valid bytes in content
 *         dupContent - set to non-zero to create a duplicate of the data
 * Return: 0 on success
 *         negative on error
 * Desc:   Sets the content to be used in the request to the data passed in 
 *         here.  If the dupContent flag isn't set, the pointers within the
 *         struct are set to point to the users data.  If the flag is set then
 *         a new copy of the content is allocated on the heap.
 */

int SetReqContent( PostReq *request, char *content, UInt32 contentLen,
                   int dupContent )
{
    if ( dupContent == 0 ) {
        request->contentOwner = 0;
        request->contentLen = contentLen;
        request->content = content;
    } else {
        request->content = (char *)Malloc( contentLen );
        if ( request->content == NULL ) {
            return -1;
        }
        MemMove( request->content, content, contentLen );
        request->contentOwner = 1;
        request->contentLen = contentLen;
    }

    return 0;
}


/*
 * Name:   AddReqHeader()
 * Desc:   not currently implemented
 */

int AddReqHeader( PostReq *request, char *header )
{
    return -1;
}


/*
 * Name:   DestroyPostReq()
 * Args:   request - the request to destroy
 * Return: none
 * Desc:   frees the structure itself as well as the heap allocated referenced
 *         strings.
 */

void DestroyPostReq( PostReq *request )
{
    char **temp;

    if ( request == NULL )
	return;

    if ( request->contentType != NULL ) {
        Free( request->contentType );
        request->contentType = NULL;
    }
    if ( request->contentOwner == 1 ) {
        if ( request->content != NULL ) {
            Free( request->content );
            request->content = NULL;
        }
    }

    if ( request->extraHeaders != NULL ) {
        temp = request->extraHeaders;
        while ( *temp != NULL ) {
            Free( *temp );
            *temp = NULL;
            temp++;
        }
    }

    Free( request );
}


void HTTPSet( HTTPParse *parse )
{

	/*parse->state = 0;
	parse->responseCode = 0;
	parse->saveFileName = 0; // ?? we should probably do something else?
	parse->fd = 0;
	parse->contentLength = 0;
	parse->contentRead = 0;
	parse->endOfStream = 0;
	parse->needData = 0;
	parse->bufferPos = 0;
	//char readBuffer[READ_BUF_SIZE];
	while (parse->readBuffer != 0) {
		parse->readBuffer[i] = 0;
		i++;
	}
    
	parse->image = 0;*/

	/*for( i = 0; i < sizeof( HTTPParse ); i++ ) {
		parse[i] = 0;
	}*/
	MemSet( parse, sizeof( HTTPParse ), 0);
}

/*
 * Name:   HTTPPost()
 * Args:   url - location to post data to
 *         request - information about the request to send
 *         resultsDB - name of the stream DB to save the response into
 * Return: HTTPErr_OK on success, != HTTPErr_OK on all errors
 * Desc:   Sends the data passed in as the body of a POST request against the
 *         host/port/path specified in 'url'.  If the request is successfull,
 *         the response text is saved in a stream database with the name given
 *         in 'resultsDB'.
 */

HTTPErr HTTPPost( URLTarget *url, Char *urlStr, PostReq *request, char *resultsDB, Boolean image, HTTPParse *parse )
{
    NetSocketRef 	sock;
    char 		contentLenStr[CLS_LENGTH];
    char		authStr[128];
    char		temp[11];
    Err 		err;
    Err 		err2;
    //HTTPParse 		parse;
    ScratchRec 		scratch;
    char 		*headers;
    int			agent = 0;
    GlobalsType 	*g;
    struct buffer_st 	auth;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    AppNetRefnum = 0;

    http_response = 200;
    http_clength = 0;
    http_received = 0;


    g->sock = -1;

    err = SysLibFind( "Net.lib", &AppNetRefnum );
    err = NetLibOpen( AppNetRefnum, &err2 );
    if ( (err && (err != netErrAlreadyOpen)) || err2 ) {
        NetLibClose( AppNetRefnum, /*true*/ false );
	g->cache.cacheDoNotRead = false;
	g->NetLibOpen = false;
	//if ( err == netErrUserCancel )
	if ( !((err == netErrUserCancel) && image) )
		g->userCancel = true;
        return HTTPErr_ConnectError;
    } else if ( (err == errNone) || (err == netErrAlreadyOpen) ) {
	if ((err = NetLibRefresh())) {
		g->NetLibOpen = false;
		NetLibClose( AppNetRefnum, false );
		g->cache.cacheDoNotRead = false;
        	return HTTPErr_ConnectError;
	}
    }

    sock = NetUTCP2Open( (g->conn.connectionType == 'h') ? g->conn.ipaddress : url->host, NULL, 
			(g->conn.connectionType == 'h') ? StrAToI(g->conn.port) : url->port );
    if ( sock < 0 ) {
        //err = NetLibClose( AppNetRefnum, true );
	DebugOutComms("HTTPPost", "socket error", 0, __FILE__, __LINE__, g);
        return HTTPErr_ConnectError;
    }

    http_response = 200;

    if ( StartScratchArea( &scratch ) != 0 ) {
        close( sock );
        //err = NetLibClose( AppNetRefnum, true );
        return HTTPErr_TempDBErr;
    }

	if (g_prefs.altAgent)
		agent = g_prefs.agentNum + 1;

    if (image != true) StrCopy(g->WapLastUrl, urlStr);

    if ( g->conn.connectionType == 'h' ) {
    	AddScratchLine( &scratch, HTTP_POST_METH );
        AddScratchLine( &scratch, urlStr );
    	AddScratchLine( &scratch, HTTP_VERSION );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );

    	AddScratchLine( &scratch, HTTP_HOST_HDR );
    	AddScratchLine( &scratch, url->host );
    	AddScratchLine( &scratch, ":" );
	StrPrintF(temp, "%i", url->port);
    	AddScratchLine( &scratch, temp );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );
    } else {
    	AddScratchLine( &scratch, HTTP_POST_METH );
    	AddScratchLine( &scratch, url->path );
    	AddScratchLine( &scratch, HTTP_VERSION );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );

    	AddScratchLine( &scratch, HTTP_HOST_HDR );
    	AddScratchLine( &scratch, url->host );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );
    }

    // TODO: for now passwordless usernames are ok, but I am not sure the specs for this...
    if ( StrLen( g->conn.proxyuser ) != 0 && g->conn.proxyuser != NULL && g->conn.connectionType == 'h' ) {
	StrPrintF( authStr, "%s:%s", g->conn.proxyuser, g->conn.proxypass );
	AddScratchLine( &scratch, HTTP_PROXY_AUTH );
	base64_encode( &auth, authStr, StrLen( authStr ) );
	AddScratchLine( &scratch, auth.data );
	AddScratchLine( &scratch, HTTP_LINE_ENDING );
	buffer_delete( &auth );
    }

    if ( g->AuthUser != NULL ) { // StrLen( g->AuthUser ) != 0 && g->AuthUser != NULL ) {
	StrPrintF( authStr, "%s:%s", g->AuthUser, g->AuthPass );
	base64_encode( &auth, authStr, StrLen( authStr ) );
	AddScratchLine( &scratch, HTTP_WEB_AUTH );
	AddScratchLine( &scratch, auth.data );
	AddScratchLine( &scratch, HTTP_LINE_ENDING );
	buffer_delete( &auth );
    }

    AddScratchLine( &scratch, HTTP_CONNECTION );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    AddScratchLine( &scratch, HTTP_USERAGENT_LINE );
    AddScratchLine( &scratch, AgentString[agent] );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    AddScratchLine( &scratch, HTTP_ACCEPT_CHARSET );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    AddScratchLine( &scratch, HTTP_ACCEPT_LANG );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    if (!g_prefs.disableUAProf) {
    	AddScratchLine( &scratch, HTTP_X_WAP_PROFILE );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );
    }

    AddScratchLine( &scratch, HTTP_ACCEPT ); // TODO: possibly remove "text/vnd.wap.wml" when connecting to a gateway?
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    if (g->CookieJar) {
	CookiePtr	cookies = NULL;
	
	cookies = CookieCreateLinkedList(g->CookieJar, url->host, url->path, false, g->privateBrowsing, g);
	if (cookies) {
		Char	*cookieHeader = NULL;

		cookieHeader = CookieCreateCookieHeader(cookies, g);
		if (cookieHeader) {
			AddScratchLine( &scratch, HTTP_COOKIE_HDR );
			AddScratchLine( &scratch, cookieHeader );
			AddScratchLine( &scratch, HTTP_LINE_ENDING );

			Free(cookieHeader);
		}
		CookieCleanCookiePtr(cookies, g);
	}
    }

    AddScratchLine( &scratch, HTTP_CONTENTTYPE_HDR );
    AddScratchLine( &scratch, request->contentType );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    AddScratchLine( &scratch, HTTP_CONTENTLENGTH_HDR );
    StrPrintF( contentLenStr, "%ld", request->contentLen );
    contentLenStr[CLS_LENGTH - 1] = '\0';
    AddScratchLine( &scratch, contentLenStr );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    AddScratchLine( &scratch, request->content );
   
    if ( scratch.errFlag ) {
        close( sock );
        //err = NetLibClose( AppNetRefnum, true );
        return HTTPErr_TempDBErr;
    }

    scratch.handle = DmQueryRecord( gHttpLib, scratch.index );
    headers = MemHandleLock( scratch.handle );
    SendAll( sock, headers, scratch.size, parse );
    MemHandleUnlock( scratch.handle );

    //SendAll( sock, request->content, request->contentLen, parse );

    needLength = true;
    if (g->isImage != true) g->charSet = UTF_8;

    HTTPSet( parse );
    parse->state = PS_ResponseLine;
    //parse->saveFileName = resultsDB;
    parse->image = image;
    parse->url = Malloc(StrLen(urlStr)+1);
    if ( parse->url )
	StrCopy(parse->url, urlStr);

    g->sock = sock;

    return HTTPErr_OK;
}


/*
 * Name:   HTTPGet()
 * Args:   url - location to fetch from
 *         resultsDB - name of the stream DB to save the response into
 * Return: HTTPErr_OK on success, != HTTPErr_OK on all errors
 * Desc:   Attempts to read the data from the URL specified and writes the
 *         body into a stream database names 'resultsDB'.
 */

HTTPErr HTTPGet( URLTarget *url, Char *urlStr, char *resultsDB, Boolean image, HTTPParse *parse )
{
    NetSocketRef 	sock;
    Err 		err;
    Err 		err2;
    //HTTPParse 		parse;
    ScratchRec 		scratch;
    char 		*headers;
    int			agent = 0;
    //char 		contentLenStr[CLS_LENGTH];
    char		temp[11];
    char		authStr[128];
    GlobalsType 	*g;
    struct buffer_st 	auth;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    AppNetRefnum = 0;

    http_response = 200;
    http_clength = 0;
    http_received = 0;

    g->sock = -1;

    DebugOutComms("HTTPGet", "prep, connect net", 0, __FILE__, __LINE__, g);

    // TODO: Bad, use another function
    err = SysLibFind( "Net.lib", &AppNetRefnum );
    err = NetLibOpen( AppNetRefnum, &err2 );
    if ( (err && (err != netErrAlreadyOpen)) || err2 ) {
        NetLibClose( AppNetRefnum, /*true*/ false );
	g->cache.cacheDoNotRead = false;
	g->NetLibOpen = false;
	//if ( err == netErrUserCancel )
	if ( !((err == netErrUserCancel) && image) )
		g->userCancel = true;
	DebugOutComms("HTTPGet", "connect error", 0, __FILE__, __LINE__, g);
        return HTTPErr_ConnectError;
    } else if ( (err == errNone) || (err == netErrAlreadyOpen) ) {
	if ((err = NetLibRefresh())) {
		g->NetLibOpen = false;
		NetLibClose( AppNetRefnum, false );
		g->cache.cacheDoNotRead = false;
        	return HTTPErr_ConnectError;
	}
    }

    DebugOutComms("HTTPGet", "open tcp", 0, __FILE__, __LINE__, g);

    sock = NetUTCP2Open( (g->conn.connectionType == 'h') ? g->conn.ipaddress : url->host, NULL, 
			(g->conn.connectionType == 'h') ? StrAToI(g->conn.port) : url->port );

    if ( sock < 0 ) {
        //err = NetLibClose( AppNetRefnum, true );
	DebugOutComms("HTTPGet", "socket error", 0, __FILE__, __LINE__, g);
        return HTTPErr_ConnectError;
    }


    http_response = 200;

    DebugOutComms("HTTPGet", "setup scratch", 0, __FILE__, __LINE__, g);

    if ( StartScratchArea( &scratch ) != 0 ) {
        close( sock );
        //err = NetLibClose( AppNetRefnum, true );
	DebugOutComms("HTTPGet", "scratch error", 0, __FILE__, __LINE__, g);
        return HTTPErr_TempDBErr;
    }

	if (g_prefs.altAgent)
		agent = g_prefs.agentNum + 1;

    DebugOutComms("HTTPGet", "update last url (if not img)", 0, __FILE__, __LINE__, g);

    if (image != true) StrCopy(g->WapLastUrl, urlStr);

    DebugOutComms("HTTPGet", "add to scratch", 0, __FILE__, __LINE__, g);

    if ( g->conn.connectionType == 'h' ) {
    	AddScratchLine( &scratch, HTTP_GET_METH );
        AddScratchLine( &scratch, urlStr );
    	AddScratchLine( &scratch, HTTP_VERSION );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );

    	AddScratchLine( &scratch, HTTP_HOST_HDR );
    	AddScratchLine( &scratch, url->host );
    	AddScratchLine( &scratch, ":" );
	StrPrintF(temp, "%i", url->port);
    	AddScratchLine( &scratch, temp );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );
    } else {
    	AddScratchLine( &scratch, HTTP_GET_METH );
    	AddScratchLine( &scratch, url->path );
    	AddScratchLine( &scratch, HTTP_VERSION );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );

    	AddScratchLine( &scratch, HTTP_HOST_HDR );
    	AddScratchLine( &scratch, url->host );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );
    }

    DebugOutComms("HTTPGet", "ad password", 0, __FILE__, __LINE__, g);

    // TODO: for now passwordless usernames are ok, but I am not sure the specs for this...
    if ( StrLen( g->conn.proxyuser ) != 0 && g->conn.proxyuser != NULL && g->conn.connectionType == 'h' ) {
	StrPrintF( authStr, "%s:%s", g->conn.proxyuser, g->conn.proxypass );
	AddScratchLine( &scratch, HTTP_PROXY_AUTH );
	base64_encode( &auth, authStr, StrLen( authStr ) );
	AddScratchLine( &scratch, auth.data );
	AddScratchLine( &scratch, HTTP_LINE_ENDING );
	buffer_delete( &auth );
    }

    if ( g->AuthUser != NULL ) { // StrLen( g->AuthUser ) != 0 && g->AuthUser != NULL ) {
	StrPrintF( authStr, "%s:%s", g->AuthUser, g->AuthPass );
	base64_encode( &auth, authStr, StrLen( authStr ) );
	AddScratchLine( &scratch, HTTP_WEB_AUTH );
	AddScratchLine( &scratch, auth.data );
	AddScratchLine( &scratch, HTTP_LINE_ENDING );
	buffer_delete( &auth );
    }

    AddScratchLine( &scratch, HTTP_CONNECTION );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    AddScratchLine( &scratch, HTTP_USERAGENT_LINE );
    AddScratchLine( &scratch, AgentString[agent] );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    AddScratchLine( &scratch, HTTP_ACCEPT_CHARSET );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    AddScratchLine( &scratch, HTTP_ACCEPT_LANG );
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    if (!g_prefs.disableUAProf) {
    	AddScratchLine( &scratch, HTTP_X_WAP_PROFILE );
    	AddScratchLine( &scratch, HTTP_LINE_ENDING );
    }

    AddScratchLine( &scratch, HTTP_ACCEPT ); // TODO: possibly remove "text/vnd.wap.wml" when connecting to a gateway?
    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    DebugOutComms("HTTPGet", "place cookies", 0, __FILE__, __LINE__, g);

    if (g->CookieJar) {
	CookiePtr	cookies = NULL;

	DebugOutComms("HTTPGet", "create cookie list", 0, __FILE__, __LINE__, g);

	cookies = CookieCreateLinkedList(g->CookieJar, url->host, url->path, false, g->privateBrowsing, g);
	if (cookies) {
		Char	*cookieHeader = NULL;

 		DebugOutComms("HTTPGet", "create cookie header", 0, __FILE__, __LINE__, g);

		cookieHeader = CookieCreateCookieHeader(cookies, g);
		if (cookieHeader) {
			AddScratchLine( &scratch, HTTP_COOKIE_HDR );
			AddScratchLine( &scratch, cookieHeader );
			AddScratchLine( &scratch, HTTP_LINE_ENDING );

			Free(cookieHeader);
		}
		CookieCleanCookiePtr(cookies, g);

		DebugOutComms("HTTPGet", "cookie done", 0, __FILE__, __LINE__, g);
	}
    }

    AddScratchLine( &scratch, HTTP_LINE_ENDING );

    DebugOutComms("HTTPGet", "scratch done", 0, __FILE__, __LINE__, g);

    if ( scratch.errFlag ) {
        close( sock );
        //err = NetLibClose( AppNetRefnum, true );
	DebugOutComms("HTTPGet", "scratch err", 0, __FILE__, __LINE__, g);
        return HTTPErr_TempDBErr;
    }

    DebugOutComms("HTTPGet", "send request", 0, __FILE__, __LINE__, g);

    scratch.handle = DmQueryRecord( gHttpLib, scratch.index );
    headers = MemHandleLock( scratch.handle );
    SendAll( sock, headers, scratch.size, parse );
    MemHandleUnlock( scratch.handle );

    DebugOutComms("HTTPGet", "request sent", 0, __FILE__, __LINE__, g);

    needLength = true;
    if (g->isImage != true) g->charSet = UTF_8;

    HTTPSet( parse );
    parse->state = PS_ResponseLine;
    //parse->saveFileName = resultsDB;
    parse->image = image;
    parse->url = Malloc(StrLen(urlStr)+1);
    if ( parse->url )
	StrCopy(parse->url, urlStr);

    g->sock = sock;

    DebugOutComms("HTTPGet", "http ok!!", 0, __FILE__, __LINE__, g);

    return HTTPErr_OK;
}

HTTPErr HTTPCleanUp( NetSocketRef *sock, HTTPParse *parse )
{

	if (*sock != -1)
		close( *sock );
	*sock = -1;
	//NetLibClose( AppNetRefnum, true );

	if ( parse->url) {
		Free( parse->url );
	}

	if ( parse->state != PS_Done ) {
		parse->state = PS_Done;
		return HTTPErr_SizeMismatch;
	}

	//if (parse) Free(parse);
	//parse = NULL;

	return HTTPErr_OK;
}

/*
 * Name:   StartScratchArea()
 * Args:   scratch - struct to use for tracking the status of the area
 * Return: 0 on success, -1 on error
 * Desc:   Creates a new record, or resizes to minimal size, the record with 
 *         index 0 from the temp database.  Initializes the other fields in 
 *         the 'scratch' struct so that the area starts out blank.
 */

static int StartScratchArea( ScratchRec *scratch )
{
    scratch->index = 0;

    if ( DmNumRecords( gHttpLib ) > 0 ) {
        scratch->handle = DmResizeRecord( gHttpLib, scratch->index, 1 );
        if ( scratch->handle == NULL ) {
            scratch->errFlag = 1;
            return -1;
        }
    } else {
        scratch->handle = DmNewRecord( gHttpLib, &(scratch->index), 1 );
        if ( scratch->handle == NULL ) {
            scratch->errFlag = 1;
            return -1;
        }
    }

    DmReleaseRecord( gHttpLib, scratch->index, false );
    scratch->size = 0;
    scratch->errFlag = 0;

    return 0;
}


/*
 * Name:   AddToScratch()
 * Args:   scratch - scratch area to add to
 *         text - pointer to the data to add
 *         length - number of bytes to add from the start of 'text'
 * Return: none
 * Desc:   Adds 'length' bytes to the end of the scratch record.  No status 
 *         is returned directly, on error a flag is set in the 'scratch' struct
 *         instead.  There's a cover, AddScratchLine(), which can be used to 
 *         add a null terminated character string without having to clutter up
 *         the calls with lots of StrLen() calls for the final arg.
 */

static void AddToScratch( ScratchRec *scratch, char *text, int length )
{
    char *scratchTxt;
    int newSize;

    if ( scratch->errFlag ) {
        return;
    }

    newSize = scratch->size + length;

    scratch->handle = DmResizeRecord( gHttpLib, scratch->index, newSize );
    if ( scratch->handle == NULL ) {
        scratch->errFlag = 1;
        return;
    }

    scratch->handle = DmGetRecord( gHttpLib, scratch->index );
    if ( scratch->handle == NULL ) {
        scratch->errFlag = 1;
        return;
    }

    scratchTxt = MemHandleLock( scratch->handle );
    if ( scratchTxt == NULL ) {
        scratch->errFlag = 1;
        DmReleaseRecord( gHttpLib, scratch->index, false );
        return;
    }

    if ( DmWrite( scratchTxt, scratch->size, text, length ) != errNone ) {
        scratch->errFlag = 1;
        MemHandleUnlock( scratch->handle );
        DmReleaseRecord( gHttpLib, scratch->index, false );
    }

    MemHandleUnlock( scratch->handle );
    DmReleaseRecord( gHttpLib, scratch->index, true );

    scratch->size += length;
}


/*
 * Name:   SendAll()
 * Args:   sock - socket to send over
 *         data - the bytes to send
 *         length - the number of bytes to send
 * Return: 0 on success, -1 on error
 * Desc:   Sends exactly 'length' bytes to the socket 'sock'.  If unable to 
 *         send the full set of data, it is considered an error.
 */

static int SendAll( NetSocketRef sock, char *data, UInt32 length, HTTPParse *parse )
{
    UInt32 written;
    Int16 thisWrite;

    written = 0;

    while ( written < length ) {
        thisWrite = send( sock, &(data[written]), length - written, 0 );

        if ( thisWrite <= 0 ) {
            return -1;
        }
        written += thisWrite;
    }

    return 0;
}



/*
 * Name:   BufSizeRemaining()
 * Args:   parse - struct to return info about
 * Return: Number of bytes free in the readBuffer of 'parse'
 * Desc:
 */

static UInt32 BufSizeRemaining( HTTPParse *parse )
{
    return( READ_BUF_SIZE - parse->bufferPos );
}


/*
 * Name:   BufConsumeToPointer()
 * Args:   parse - parse object to operate on
 *         newFirst - pointer to the new first byte within readBuffer
 * Return: none
 * Desc:   The 'newFirst' argument to this function should be a pointer to a
 *         location within the existing readBuffer for 'parse' (there should
 *         be an assert() to check to make sure that 'newFirst' lies within the
 *         buffer).  All the content in the readBuffer before 'newFirst' is
 *         deleted out of the buffer, and the internal state updated to be
 *         consistent.
 */

static void BufConsumeToPointer( HTTPParse *parse, char *newFirst )
{
    UInt32 newLength;

    newLength = NextBufByte( parse ) - newFirst;
    MemMove( parse->readBuffer, newFirst, newLength );
    parse->bufferPos = newLength;
}


/*
 * Name:   NextBufByte()
 * Args:   parse - structure to calculate from
 * Return: pointer to the next empty byte in the read buffer
 * Desc:   Just a convenient wrapper.
 */

static char *NextBufByte( HTTPParse *parse )
{
    return( parse->readBuffer + parse->bufferPos );
}


/*
 * Name:   FillReadBuff()
 * Args:   sock - socket to read from
 *         parse - the structure to write the data into
 * Return: number of bytes read on success, -1 on error
 * Desc:   Attempts to read data from the socket 'sock' and write into the
 *         read buffer associated with 'parse'.  This function should only be
 *         called if data is needed (the parse can't succeed without having 
 *         more data).  If any data at all is read, the needData flag from 
 *         'parse' is cleared.  It's up to the parse functions to determine if
 *         the new data is enough to proceed, and if not to reset the flag and
 *         call this function again.
 */

static Int16 FillReadBuff( NetSocketRef sock, HTTPParse *parse, Int32 maxTimer )
{
    Int16 readRes;
    Err err = errNone;

    AppNetTimeout = NET_BLOCK_TIMEOUT * SysTicksPerSecond();

    if ( BufSizeRemaining( parse ) == 0 ) {
        return -1;
    }


    readRes = recv( sock, NextBufByte( parse ), BufSizeRemaining( parse ), 0 );

    if ( ( ( errno == netErrTimeout ) || ( err == netErrTimeout ) ) && ( TimGetTicks() < maxTimer ) ) {
	return 0;
    }

    if ( readRes < 0 ) {
        return -1;
    }


    if ( readRes == 0 ) {
        parse->endOfStream = 1;
        parse->needData = 0;
	//http_clength = 0;
	return 1;
    } else {
        parse->bufferPos += readRes;
	http_received += readRes;
        parse->needData = 0;
	if (needLength) /*parse->contentLength*/ http_clength += readRes;
    }

    return readRes;
}


/*
 * Name:   MarkEOL()
 * Args:   parse - buffer to look in
 * Return: 0 if a full line was found, -1 otherwise
 * Desc:   Attempts to find a full line (terminated by a caridge 
 *         return/linefeed pair) at the start of the readBuffer associated with
 *         'parse'.  If found, a null character is overwritten at the first
 *         line ending character so that the line can be treated as a string.
 *         This means that the function can only be called once for each line.
 */

static int MarkEOL( HTTPParse *parse )
{
    Int32 i;
    char *endOfLine;

    i = 0;
    endOfLine = NULL;
    while ( (i < parse->bufferPos) && (endOfLine == NULL) ) {
        if ( parse->readBuffer[i] == '\r' ) { // some servers do Header: value\n
            endOfLine = parse->readBuffer + i;
        }
        i++;
    }

    if ( (endOfLine == NULL) || (endOfLine == (NextBufByte( parse ) - 1)) ) {
        if ( parse->endOfStream ) {
            parse->state = PS_Error;
            return -1;
        }

        parse->needData = 1;
        return -1;
    }

    if ( endOfLine[1] != '\n' ) {
        parse->state = PS_Error;
        return -1;
    }

   *endOfLine = '\0';

    return 0;
}


/*
 * Name:   ParseResponseCode()
 * Args:   start - pointer to the start of the string to search in
 * Return: Pointer to first char in the response code on success, NULL on error
 * Desc:   Attempts to find a whitespace delimited word starting at or after
 *         the location pointed to by 'start'.  The value returned is the 
 *         location of the first non-whitespace character in the response code.
 */

static char *ParseResponseCode( char *start )
{
    char *beginField;
    char *endField;

    beginField = start;
    while ( TxtCharIsSpace( *beginField ) ) {
        beginField++;
    }

    if ( *beginField == '\0' ) {
        return( NULL );
    }

    endField = beginField + 1;
    while ( !TxtCharIsSpace( *endField ) && (*endField != '\0') ) {
        endField++;
    }

    if ( *endField == '\0' ) {
        return( NULL );
    }

    return( beginField );
}


/*
 * Name:   ParseEngine()
 * Args:   sock - socket to read from
 *         parse - struct to use to store the parse state
 * Return: none
 * Desc:   I've tried to structure this as a relatively generic parse loop,
 *         hoping that it'll somewhat match what's needed for an event driven
 *         version.  The only place that a read is done is right here, the
 *         parse functions just take care of handling the data that's already
 *         been deposited in the read buffer.
 */

void ParseEngine( NetSocketRef sock, HTTPParse *parse )
{
    GlobalsType *g;
    Int16		readData = 0;
    Int32	maxTimer;


    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    DebugOutComms("ParseEngine", "top", 0, __FILE__, __LINE__, g);

    if (parse->image != true) browserSetTitle("Receiving URL"); // SetFieldFromStr("Receiving URL",fieldTitle);

    //while ( (parse->state != PS_Error) && (parse->state != PS_Done) && (parse->state != PS_UserAbort) ) {

    DebugOutComms("ParseEngine", NULL, 0, __FILE__, __LINE__, g);

    if ( (parse->state == PS_Error) || (parse->state == PS_Done) ) {
	FileClose( parse->fd );
        return;
    } 
	showProgress(g);

	if ( parse->image == true ) {
		if ( ProcessEventLoopTimeout(0) ) {
			parse->state = PS_Error;
			FileClose( parse->fd );
			g->userCancel = true;
			return;
		}
	}

        if ( parse->needData == 1 ) {
	    maxTimer = TimGetTicks() + ( REPLY_TIMEOUT * SysTicksPerSecond() );
top:
	    readData = FillReadBuff( sock, parse, maxTimer );

		if ( parse->image == true ) {
			if ( ProcessEventLoopTimeout(0) ) {
				parse->state = PS_Error;
				FileClose( parse->fd );
				g->userCancel = true;
				return;
			}
		}

            if ( readData < 0 ) {
                parse->state = PS_Error;
                //break;
            } else if ( readData == 0 ) {
		showProgress(g);
		goto top;
	    }
        }

	DebugOutComms("ParseEngine", "mid", 0, __FILE__, __LINE__, g);

        switch ( parse->state ) {
            case PS_ResponseLine:
                ParseResponseLine( parse );
                break;

            case PS_Headers:
                ParseHeaders( parse );
                break;

            case PS_Body:
                ParseBody( parse );
                break;

            default:
                break;
        }

	DebugOutComms("ParseEngine", "bottom", 0, __FILE__, __LINE__, g);
    //}
}


/*
 * Name:   ParseResponseLine()
 * Args:   parse - structure to use to track parse status
 * Return: none
 * Desc:   Attempts to pull the information from the initial response line of
 *         at HTTP response.  If the line seems to be malformed an error is
 *         flagged in the 'parse' struct and the parse loop takes care of
 *         exiting.  On success the response code is stored and the parse state
 *         is updated to process the headers.  The parse engine takes care of
 *         calling the next stage, we just set the state and return to the
 *         caller.
 */

static void ParseResponseLine( HTTPParse *parse )
{
    char *newFirstByte;
    char *pastVersion;
    char *responseVal;


    if ( MarkEOL( parse ) == -1 ) {
        return;
    }
    newFirstByte = parse->readBuffer + StrLen( parse->readBuffer ) + 2;
    if (needLength) /*parse->contentLength*/ http_clength -= StrLen( parse->readBuffer ) + 2;

    pastVersion = parse->readBuffer;
    while ( !TxtCharIsSpace( *pastVersion ) && (*pastVersion != '\0') ) {
        pastVersion++;
    }

    if ( *pastVersion == '\0' ) {
        parse->state = PS_Error;
        return;
    }

    responseVal = ParseResponseCode( pastVersion );
    if ( responseVal == NULL ) {
        parse->state = PS_Error;
        return;
    }

    parse->responseCode = StrAToI( responseVal );
    BufConsumeToPointer( parse, newFirstByte );

    parse->state = PS_Headers;

    http_response = parse->responseCode;
}


/*
 * Name:   ParseHeaders()
 * Args:   parse - struct to use to track the parse state
 * Return: none
 * Desc:   Reads in HTTP headers from the socket associated with 'parse' until
 *         it finds an empty line.  If there's a content length line it gets
 *         parsed and the target byte count stored for later.  Once we see the
 *         terminating empty line the state is updated to process the body.  
 *         Before updating the state and allowing the next stage to progress we
 *         open up a stream database to write the response body into.
 */

static void ParseHeaders( HTTPParse *parse )
{
    char *newFirstByte;
    char *value;
    UInt32 tmpLen;
    GlobalsType *g;
    Char *header = NULL;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    if ( MarkEOL( parse ) == -1 ) {
        return;
    }

    newFirstByte = parse->readBuffer + StrLen( parse->readBuffer ) + 2;
    if (needLength) http_clength -= StrLen( parse->readBuffer ) + 2;

    if ( StrLen( parse->readBuffer ) == 0 ) {
        BufConsumeToPointer( parse, newFirstByte );

        parse->fd = FileOpen( 0, /*parse->saveFileName*/ HTTPDATA_FILE, 'DATA', HTTPDATA_STORE,
                              fileModeReadWrite, NULL );
        if ( parse->fd == NULL ) {
            parse->state = PS_Error;
            return;
        }

        parse->state = PS_Body;
        return;
    }

    if ( StrNCaselessCompare( parse->readBuffer, HTTP_CONTENTLENGTH_HDR, 
                      StrLen( HTTP_CONTENTLENGTH_HDR ) ) == 0 ) {
        value = parse->readBuffer + StrLen( HTTP_CONTENTLENGTH_HDR );
        tmpLen = (UInt32)StrAToI( value );
        if ( tmpLen < 0 ) {
            parse->state = PS_Error;
            return;
        }
        parse->contentLength = (UInt32)tmpLen;
	needLength = false;
	http_clength = (UInt32)tmpLen;
    }

    if ( StrNCaselessCompare( parse->readBuffer, HTTP_CONTENTTYPE_HDR, 
                      StrLen( HTTP_CONTENTTYPE_HDR ) ) == 0 ) {
	int v = 0, b = 0, c = 0, t = 0;
	Boolean hasCharSet = false;
	char charSet[75];

        value = parse->readBuffer + StrLen( "Content-Type: " );
        if (g->contentType) Free(g->contentType);
	g->contentType = Malloc(StrLen(value) + 1);
	ErrFatalDisplayIf(!g->contentType, "Malloc failed");
	//StrCopy(g->contentType, value);

	while ( value[v] != ';' && value[v] != '\n' && value[v] != '\r' &&
		value[v] != '\0' ) {
		g->contentType[t] = value[v];
		v++; t++;
	} if (value[v] == ';') { hasCharSet = true; v++; }

	g->contentType[t] = '\0';

	if (hasCharSet == true) {
		while (value[v] == ' ') v++;

		if ( StrNCaselessCompare( value+v, "charset=", 8 ) == 0 ) {
			v += 8;

			while ( value[v] != ';' && value[v] != '\n' && value[v] != '\r' &&
				value[v] != '\0' && value[v] != '\"' ) {
				charSet[c] = value[v];
				v++; c++;
			}
			charSet[c] = '\0';

			if (StrCompareLower(charSet, "utf-8")) {
				g->charSet = UTF_8;
			} else if (StrCompareLower(charSet, "us-ascii")) {
				g->charSet = US_ASCII;
			} else if (StrCompareLower(charSet, "iso-8859-1")) {
				g->charSet = ISO_8859_1;
			} else if (StrCompareLower(charSet, "usc-2")) {
				g->charSet = USC_2;
			} else {
				g->charSet = UTF_8;
			}
		} else if ( StrNCaselessCompare( value+v, "boundary=", 9 ) == 0) {
			v += 9;
			b = v;

			while ( value[b] != ';' && value[b] != '\n' && value[b] != '\r' &&
				value[b] != '\0' && value[b] != '\"' ) {
				b++;
			}

			if (g->partBound != NULL) {
				Free(g->partBound);
				g->partBound = NULL;
			}

			g->partBound = Malloc(b - v + 50);
			if (!g->partBound) goto stop;

			b = 0;
			while ( value[v] != ';' && value[v] != '\n' && value[v] != '\r' &&
				value[v] != '\0' && value[v] != '\"' ) {
				g->partBound[b] = value[v];
				v++; b++;
			}
			g->partBound[b] = '\0';
		}
stop:
	}
	

	if (g->contentType && (parse->image != true)) {
		if (g->pageContentType) Free(g->pageContentType);
		g->pageContentType = Malloc(StrLen(g->contentType)+1);
			ErrFatalDisplayIf (!g->pageContentType, "WspDecode: Malloc Failed");
	    	StrCopy(g->pageContentType, g->contentType);
	}
    }

    //if ( ( StrNCaselessCompare( parse->readBuffer, HTTP_LOCATION_HDR, 
    //                  StrLen( HTTP_LOCATION_HDR ) ) ) == 0 ) {
    if ( ( ( header = StrStr( parse->readBuffer, HTTP_LOCATION_HDR ) ) ||
	   ( header = StrStr( parse->readBuffer, "location: " ) ) ) &&
        !( StrStr( parse->readBuffer, "Content-Location: " ) || StrStr( parse->readBuffer, "content-location: " ) ) ) {
	int v = 0;

        value = header + StrLen( HTTP_LOCATION_HDR );
        if ( g->urlBuf != NULL ) {
		Free( g->urlBuf );
		g->urlBuf = NULL;
	}
	g->urlBuf = Malloc( StrLen( value ) + 1 );
		ErrFatalDisplayIf( !g->urlBuf, "Malloc failed" );

	while ( value[v] != '\n' && value[v] != '\r' && value[v] != '\0' ) {
		g->urlBuf[v] = value[v];
		v++;
	}
	g->urlBuf[v++] = '\0';

	parse->state = PS_Done;
        return;
    }

    if ( ( header = StrStr( parse->readBuffer, HTTP_CACHE_CONTROL_HDR ) ) ||
	 ( header = StrStr( parse->readBuffer, "cache-control: " ) ) ) {

        value = header + StrLen( HTTP_CACHE_CONTROL_HDR );
        
	if ( StrStr( value, "max-age=0" ) || StrStr( value, "Max-Age=0" ) || 
	     StrStr( value, "must-revalidate" ) || StrStr( value, "Must-Revalidate" ) ||
	     StrStr( value, "no-cache" ) || StrStr( value, "No-Cache" ) ||
	     StrStr( value, "no-store" ) || StrStr( value, "No-Store" ))
		g->cache.cacheDoNotRead = true;
    }

    if ( ( header = StrStr( parse->readBuffer, "Pragma: " ) ) ||
	 ( header = StrStr( parse->readBuffer, "pragma: " ) ) ) {

        value = header + StrLen( "Pragma: " );
        
	if ( StrStr( value, "max-age=0" ) || StrStr( value, "Max-Age=0" ) || 
	     StrStr( value, "must-revalidate" ) || StrStr( value, "Must-Revalidate" ) ||
	     StrStr( value, "no-cache" ) || StrStr( value, "No-Cache" ) ||
	     StrStr( value, "no-store" ) || StrStr( value, "No-Store" ))
		g->cache.cacheDoNotRead = true;
    }

    if ( parse->responseCode == 401 ) {
        if ( ( header = StrStr( parse->readBuffer, HTTP_REALM_HDR ) ) ||
	     ( header = StrStr( parse->readBuffer, "www-authenticate: " ) ) ) {
	    int v = 0;

            value = header + StrLen( HTTP_REALM_HDR );
	    value = StrStr( value, "realm=" );

	    if ( value ) {
		value += StrLen( "realm=" );
		if ( value[0] == '\"' ) value++;

	    	while ( value[v] != '\n' && value[v] != '\r' && 
		    	value[v] != '\0' && value[v] != '\"' &&
		    	value[v] != ',' && v < 199 ) {
				http_realm[v] = value[v];
				v++;
	    	}
	    	http_realm[v++] = '\0';
	    }
        }
    }

    if ( ( header = StrStr( parse->readBuffer, "Set-Cookie: " ) ) ||
	 ( header = StrStr( parse->readBuffer, "set-Cookie: " ) ) ) {
		URLTarget	*url;
        	value = header;

		url = ParseURL( (parse->url)?parse->url:g->Url );
        	if ( url ) {
			CookieCreateCookie( g->CookieJar, value, url->host, url->path, g->privateBrowsing, g );
			DestroyURL( url );
		}
    }

    //if (needLength == false) http_clength = parse->contentLength;

    BufConsumeToPointer( parse, newFirstByte );
}


/*
 * Name:   ParseBody()
 * Args:   parse - struct to use to track the parse
 * Return: none
 * Desc:   There are two different techniques to use while reading the body.
 *         If we know the content length from one of the headers in the
 *         response we have a very exact target to hit.  If there was no length
 *         header we read till we hit end of file on the stream.  The data is
 *         written into a stream database as we grab it.
 */

static void ParseBody( HTTPParse *parse )
{
    UInt32 bytesNeeded;
    UInt32 byteCount;


    if ( parse->contentLength != 0 ) {
        bytesNeeded = parse->contentLength - parse->contentRead;
        if ( bytesNeeded == 0 ) {
            parse->state = PS_Done;
            FileClose( parse->fd );
            return;
        }

        if ( parse->bufferPos == 0 ) {
            parse->needData = 1;
            return;
        }

        if ( parse->bufferPos > bytesNeeded ) {
            byteCount = bytesNeeded;
        } else {
            byteCount = parse->bufferPos;
        }
    } else {
        if ( parse->bufferPos == 0 ) {
            if ( parse->endOfStream != 0 ) {
                parse->state = PS_Done;
                FileClose( parse->fd );
                return;
            }

            parse->needData = 1;
            return;
        }
        byteCount = parse->bufferPos;

    }

    FileWrite( parse->fd, parse->readBuffer, 1, byteCount, NULL );
    parse->contentRead += byteCount;
    BufConsumeToPointer( parse, &(parse->readBuffer[byteCount]) );
}

void HTTPFlushCache(void)
{
    Err		error = errNone;
    //FileHand	cache;

    //cache = FileOpen( 0, "Universe-HTTP-Cache", 'DATA', HTTPDATA_STORE, fileModeReadWrite, NULL );

    //FileFlush( cache );
    //FileClose( cache );

    error = FileDelete(0, HTTPDATA_FILE);
}

Int16 HTTPMultipartExtract( HTTPParse *parse )
{
    GlobalsType *g;
    Char	*data, *off;
    //HTTPParse   parse;

    FtrGet( wuAppType, ftrGlobals, (UInt32 *)&g );

    showProgress( g );

    if ( g->partBound == NULL )
	return 1;

    needLength = true;
    if (g->isImage != true) g->charSet = UTF_8;

    MemSet( parse, sizeof( HTTPParse ), 0 );
    parse->state = PS_Headers;
    //parse->saveFileName = "Universe-HTTP-Cache";
    parse->image = false;
    parse->bufferPos = 0;
    parse->fd = FileOpen( 0, "Universe-HTTP-Cache", 'DATA', HTTPDATA_STORE, fileModeReadOnly, NULL );

    data = Malloc(http_clength + 1);
    if ( !data ) return 1; 
    FileRead( parse->fd, data, http_clength, 1, NULL );
    FileClose( parse->fd );

    if( ( off = StrStr( data, g->partBound ) ) ) {
	Char *new;
	Int32 size = 0, i = 0, o = 0, d = 0;

	d += StrLen( g->partBound );

	while ( StrNCaselessCompare( data + d, g->partBound, StrLen( g->partBound ) ) != 0  && d < StrLen( data ) ) {
		size++; d++;
	}

	o += StrLen( g->partBound );
	while (off[o] == '\r' || off[o] =='\n') o++;

	new = Malloc(size + 1);
	if (!new) return 1;

	parse->contentLength = size;

	while ( StrNCaselessCompare( off + o + 2, g->partBound, StrLen( g->partBound ) ) != 0 && i < size) {
		showProgress( g );
		new[i] = off[o];
		i++; o++;
	}
	new[i] = '\0';

	dataUsed = 0;

	HTTPFlushCache();

	MultipartParseEngine( &g->httpParser, new );

	Free( new );

    	Free( data );
    	Free( g->partBound );
	g->partBound = NULL;

	return 0;
    }

    Free( data );
    Free( g->partBound );
    g->partBound = NULL;

    return 1;
}

static Int32 MultipartFillReadBuff( Char *data, HTTPParse *parse )
{
    Int16 	readRes = 0;
    Int16	len = 0, count = 0;

    if ( BufSizeRemaining( parse ) == 0 ) {
        return -1;
    }

    //readRes = recv( sock, NextBufByte( parse ), BufSizeRemaining( parse ), 0 );
    len = BufSizeRemaining( parse );
    if (len == 0)
	goto bottom;

    while ( count < len && count < READ_BUF_SIZE) {
	parse->readBuffer[parse->bufferPos + count] = data[dataUsed + count];
	count++;
    }


    dataUsed += len;
    readRes = len;
bottom:

    if ( readRes == 0 ) {
        parse->endOfStream = 1;
        parse->needData = 0;
	//http_clength = 0;
	return 1;
    } else {
        parse->bufferPos += readRes;
        parse->needData = 0;
	if (needLength) /*parse->contentLength*/ http_clength += readRes;
    }

    return readRes;
}

static void MultipartParseEngine( HTTPParse *parse, Char *data )
{
    GlobalsType *g;
    Int32	readData = 0;

    FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

    while ( (parse->state != PS_Error) && (parse->state != PS_Done) ) {

	if ( parse->needData == 1 ) {

	    readData = MultipartFillReadBuff( data, parse );

            if ( readData < 0 ) {
                parse->state = PS_Error;
                break;
            }
        }

	showProgress(g); 
        switch ( parse->state ) {
            case PS_ResponseLine:
                ParseResponseLine( parse );
                break;

            case PS_Headers:
                ParseHeaders( parse );
                break;

            case PS_Body:
                ParseBody( parse );
                break;

            default:
                break;
        }
    }
}


int HTTPHandle( WebDataPtr webData, GlobalsType *g )
{
	FileHand		fh;
	int 			code_count = 0;

DebugOutComms("HTTPHandle", "top", 0, __FILE__, __LINE__, g);

	if ( http_response == 401 ) {
	  	//realm = Malloc(MAX_URL_LEN + 1);
		//	ErrFatalDisplayIf (!realm, "WspDecode: Malloc Failed");
	  	//authreq = WSPGetRealm(http_realm, i, instr, hsz);
	  	if ( StrLen(http_realm) > 0 ) {
			Boolean wantAuth = false;

			if ( g->AuthUser != NULL ){ Free( g->AuthUser ); g->AuthUser = NULL; }
			if ( g->AuthPass != NULL ){ Free( g->AuthPass ); g->AuthPass = NULL; }

			g->AuthUser = Malloc( 255 );
				ErrFatalDisplayIf ( !g->AuthUser, "Malloc Failed" );
			g->AuthPass = Malloc( 255 );
				ErrFatalDisplayIf ( !g->AuthPass, "Malloc Failed" );

			wantAuth = Authenticate( http_realm, g->AuthUser, g->AuthPass );

			if (wantAuth == true) {
				followLink( g->Url, g );
				return 1;
			} else {
				if ( g->AuthUser && ( g->AuthUser != NULL ) ) Free( g->AuthUser );
				if ( g->AuthPass && ( g->AuthPass != NULL ) ) Free( g->AuthPass );
		  		g->AuthUser = NULL;
	  			g->AuthPass = NULL;
				HTTPError( "401: Unauthorized" );
				return 3;
			}
	  	}
		if ( g->AuthUser != NULL ){ Free( g->AuthUser ); g->AuthUser = NULL; }
		if ( g->AuthPass != NULL ){ Free( g->AuthPass ); g->AuthPass = NULL; }
	  	g->AuthUser = NULL;
	  	g->AuthPass = NULL;
	  	HTTPError( "401: Unauthorized" );
          	return 3;
    	}

	WspFreePdu(webData);

	DebugOutComms("HTTPHandle", "PDU Freed", 0, __FILE__, __LINE__, g);

	webData->transaction_id = 0x01;
  	webData->pdu_type = WSP_PDU_REPLY;
	webData->length = http_clength;
	if ( g->isImage != true ) g->contentLength = http_clength;

	while ( ( code_list[code_count].code != http_response ) && ( code_count > HTTP_CODE_LIST_MAX ) )
		code_count++;

	if ( code_count == HTTP_CODE_LIST_MAX )
		webData->status_code = code_list[HTTP_CODE_LIST_MAX + 1].wspVal;
	else
		webData->status_code = code_list[code_count].wspVal;

	DebugOutComms("HTTPHandle", NULL, 0, __FILE__, __LINE__, g);

	if ( ( ( http_response == 302 ) || ( http_response == 301 ) || ( http_response == 303 ) ||
	       ( http_response == 307 ) ) && ( g->urlBuf != NULL ) && ( g->isImage != true ) ) {
		browserSetTitle("Redirecting"); // SetFieldFromStr( "Redirecting", fieldTitle );

		if ( g->redirects >= MAX_REDIRECT_COUNT ) {
			MyErrorFunc("The browser has been redirected more than the maximum alotted redirect limit.", NULL);
			Free( g->urlBuf );
			g->urlBuf = NULL;
			g->redirects = 0;
			return 3;
		}
		g->redirects++;

		followLink( g->urlBuf, g );
		Free( g->urlBuf );
		g->urlBuf = NULL;
		return 1;
	}

	g->redirects = 0;

	if ( ( ( http_response == 302 ) || ( http_response == 301 ) || ( http_response == 303 ) ||
	       ( http_response == 307 ) ) && ( g->urlBuf != NULL ) && ( g->isImage == true ) ) {
		DebugOutComms("HTTPHandle", "Redirecting", 0, __FILE__, __LINE__, g);
		return 6;
	}

	DebugOutComms("HTTPHandle", NULL, 0, __FILE__, __LINE__, g);

	if ( http_clength > 0 ) {
		fh = FileOpen( 0, "Universe-HTTP-Cache", 'DATA', HTTPDATA_STORE, fileModeReadOnly, NULL );

		/*webData->data = Malloc(http_clength + 1);
			if ( !webData->data && 
			     ( ( StrNCaselessCompare( g->contentType, "application/vnd.wap.wmlc", 24 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "application/vnd.wap.wmlscriptc", 30 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "application/vnd.wap.xhtml+xml", 29 ) !=0 ) && 
			       ( StrNCaselessCompare( g->contentType, "application/xhtml+xml", 21 ) !=0 ) && 
			       ( StrNCaselessCompare( g->contentType, "application/wml+xml" ,19 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "text/html", 9 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "text/vnd.wap.wml", 16 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "multipart/", 10 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "application/rss+xml", 19 ) !=0 ) ) &&
			     ( g->isImage != true ) ) {
				g->httpUseTempFile = true;
				return 7;
			} else if ( !webData->data ) {
				HTTPFlushCache();
				return 2; // ErrFatalDisplayIf(!webData->data, "Malloc failed");
			}*/

		{
			UInt32	freeMemory = GetFreeMemory(NULL, NULL, true);

			if ( ( freeMemory < http_clength ) && 
			     ( ( StrNCaselessCompare( g->contentType, "application/vnd.wap.wmlc", 24 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "application/vnd.wap.wmlscriptc", 30 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "application/vnd.wap.xhtml+xml", 29 ) !=0 ) && 
			       ( StrNCaselessCompare( g->contentType, "application/xhtml+xml", 21 ) !=0 ) && 
			       ( StrNCaselessCompare( g->contentType, "application/wml+xml" ,19 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "text/html", 9 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "text/vnd.wap.wml", 16 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "multipart/", 10 ) !=0 ) &&
			       ( StrNCaselessCompare( g->contentType, "application/rss+xml", 19 ) !=0 ) ) &&
			     ( g->isImage != true ) ) {
				g->httpUseTempFile = true;
				FileClose( fh );
				return 7;
			} else if ( freeMemory < http_clength ) {
				FileClose( fh );
				HTTPFlushCache();
				return 2; // ErrFatalDisplayIf(!webData->data, "Malloc failed");
			}
			
		}

		webData->data = Malloc(http_clength + 1);
			if ( !webData->data ) {
				FileClose( fh );
				HTTPFlushCache();
				return 2; // ErrFatalDisplayIf(!webData->data, "Malloc failed");
			}

		FileRead( fh, webData->data, http_clength, 1, NULL );
		FileClose( fh );

		if (StrNCaselessCompare(g->contentType, "multipart/", 10) != 0) HTTPFlushCache();
		//FileDelete( 0, "Universe-HTTP-Cache" );
	} else if ( ( http_response > 299 ) && ( http_clength == 0 ) && ( g->isImage != true ) ) {
		while ( ( code_list[code_count].code != http_response ) && ( code_count < HTTP_CODE_LIST_MAX ) )
			code_count++;

		if ( code_count == HTTP_CODE_LIST_MAX )
			HTTPError( code_list[HTTP_CODE_LIST_MAX + 1].error );
		else
			HTTPError( code_list[code_count].error );
		
		return 3;
	} else if ( ( http_response > 299 ) && ( g->isImage == true ) ) {
		return 4;
	}

	DebugOutComms("HTTPHandle", NULL, 0, __FILE__, __LINE__, g);


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

	DebugOutComms("HTTPHandle", NULL, 0, __FILE__, __LINE__, g);

	return 0;
}

