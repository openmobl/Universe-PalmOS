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
/* tag: http protocol header file for PalmHTTP
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

#if !defined(PALMHTTP_H_)
#define PALMHTTP_H_ 1

#include <PalmOS.h>
#include "WAPUniverse.h"

#define HTTP_ERR_REDIRECT	6


/*
 * Library initialization/shutdown functions
 */

int HTTPLibStart( UInt32 creator );
void HTTPLibStop( void );


/*
 * Library representation of a URL.  Normal human readable URLs need to be 
 * converted into this form for use with the rest of the functions.
 */

typedef enum URLTransports_enum {
    URLT_http = 0,
    URLT_https = 1, /* not currently implemented */
} URLTransports;

typedef struct URLTarget_struct {
    URLTransports transport;
    char *host;
    UInt16 port;
    char *path;
} URLTarget;

URLTarget *CreateURL( URLTransports transport, char *host, UInt16 port,
                      char *path );
URLTarget *ParseURL( char *text );
void DestroyURL( URLTarget *url );


/*
 * Internal representation of a POST request.  The contentType field is the 
 * text used in the header, the content and contentLen are used for the body 
 * of the request.  The user can add in custom headers if they need,
 * extraHeaders is an array of strings to add to the headers for the request.
 * The strings should be full headers, ie. "My-Custom-Header: headerval".  The
 * library will append the line ending.  The array of strings must be NULL
 * terminated. The contentOwner field tells if the post content has been 
 * duped for ownership by the struct, and if it should be freed with the rest
 * of the info.
 */

typedef struct PostReq_struct {
    char *contentType;
    char *content;
    UInt32 contentLen;
    int contentOwner;
    char **extraHeaders;
} PostReq;

PostReq *CreatePostReq( void );
int SetReqContentType( PostReq *request, char *type );
int SetReqContent( PostReq *request, char *content, UInt32 contentLen,
                   int dupContent );
int AddReqHeader( PostReq *request, char *header );
void DestroyPostReq( PostReq *request );


/*
 * The request functions, these are the ones that really move the data around.
 * Both store the response from the server in a database, the name of which 
 * is what is passed as 'resultsDB'.  When the function returns successfully,
 * that means the result is already in the database.  The client can go open
 * the database to get access to the data.
 */

typedef enum HTTPErr_enum {
    HTTPErr_OK = 0,
    HTTPErr_ConnectError = 1,
    HTTPErr_TempDBErr = 2,
    HTTPErr_SizeMismatch = 3,
    HTTPErr_NoMem = 4,
} HTTPErr;

/*
 * Used to represent the different stages of processing an HTTP response (the
 * first line of the response, the headers, and the body of the message).  The
 * PS_Done state is set after the entire body has been read and there is no
 * more data expected.  PS_Error is used for any error condition.
 */

// move ParseState to WAPUniverse.h

/*
 * Used to hold the values associated with processing an HTTP request/response
 * cycle.  Some of the values in here aren't exposed to end users yet, cause 
 * there's a lot we still don't support.  The readBuffer and bufferPos fields
 * are used in combination.  bufferPos holds the index of the next position to
 * fill in the buffer.  There are a few convenience and cover functions to 
 * keep the bufferPos field in sync with the readBuffer.  endOfStream is used
 * to hold the end of file result from the socket.  If we don't have a 
 * content length header we have to rely on hitting the end of the stream to 
 * tell us how much data there is.
 */

// move HTTPParse to WAPUniverse.h

HTTPErr HTTPPost( URLTarget *url, Char *urlStr, PostReq *request, char *resultsDB, Boolean image, HTTPParse *parse );
HTTPErr HTTPGet( URLTarget *url, Char *urlStr, char *resultsDB, Boolean image, HTTPParse *parse );

int HTTPHandle(WebDataPtr webData, GlobalsType *g);

Int16 HTTPMultipartExtract( HTTPParse *parse );

void HTTPFlushCache(void);
HTTPErr HTTPCleanUp( NetSocketRef *sock, HTTPParse *parse );

#define HTTPParseEngine ParseEngine

void ParseEngine( NetSocketRef sock, HTTPParse *parse );

extern Int16 http_response;
extern UInt32 http_clength;
extern UInt32 http_received;


#endif /* PALMHTTP_H_ */


