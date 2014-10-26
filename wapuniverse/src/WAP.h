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
#ifndef _wap_h_
#define _wap_h_

#ifdef __palmos__
#include <PalmOS.h>
#include "dbUrl.h"
#include "dbConn.h"
#include <sys_socket.h>
#else
#include "malloc.h"
#include "string.h"
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#define Malloc     malloc
#define Free(addr) free(addr)
#endif

#define WAP_WSP_CL 		9200
#define WAP_WSP_CO 		9201
#define WAP_WSP_WTLS_CL 	9202
#define WAP_WSP_WTLS_CO 	9203

#define WAP_WSP_CL_LOCAL 	2050
#define WAP_WSP_CO_LOCAL 	2051
#define WAP_WSP_WTLS_CL_LOCAL 	2052
#define WAP_WSP_WTLS_CO_LOCAL 	2053

#define WAP_MAX_PACKET		30000 // Maximum WAP Packet size supported by browser


//#define MAX_URL_LEN 128

// WSP

#define WSP_PDU_GET 0x40
#define WSP_PDU_POST 0x60
#define WSP_PDU_VERSION 0x1A
#define WSP_PDU_CONNECT 0x01

#define wapErrorClass		(appErrorClass|20)			
#define errWapNoMemory		(wapErrorClass|1)
#define errWapNoConnection	(wapErrorClass|2)

// function prototypes
int wapGetUrl(dbConnConnection *conn, char *urlstr, Boolean remember, Err *error, GlobalsType *g);
// Boolean remember: true if we are loading a WML page (and not an image), sowe set gLastURL to current URL, needed for relative links later on
int Connect2Server(char *gwip, char *urlStr, GlobalsType *g );
void enc_uintvar(char ostr[], UInt16 *j, unsigned long value);
int WTPDisconnect (Int16 sock, GlobalsType *g);
extern void stripDoubleDots(char *s) SEC_1;
extern void stripSingleDots(char *s);
extern int WSPDisconnect (Int16 sock, GlobalsType *g);
extern int wapConnect(dbConnConnection *conn, char *urlstr, Boolean remember, int wtpFail, GlobalsType *g );
extern void reWtpGet (int sock, dbConnConnection *conn, char *urlstr, Boolean remember, Err *error, GlobalsType *g );
extern char * gstrcopy(char *dst, const char *src);
extern UInt16 gstrlen(char *str);
extern int StringToIPAddr (char *str, unsigned long *addr);
extern char *GetErrStr(Err errno);

#endif _wap_h_
