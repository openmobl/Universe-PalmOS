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
#ifndef _wsp_h_
#define _wsp_h_

#include "WAP.h"
#include "WAPUniverse.h"

// WSP SDU size, 1400 according to WAP WSP 28-May-1999 V1.1 (SPEC-WSP-19990528.PDF Ch8.3.3)
#define DEF_RECEIVE_BUF 59997 //4096 // looks like some gateways sent larger decks TODO: Maybe make user customizable

//SAR flags
#define SAR_IS_FIRST	1
#define SAR_NOT_FIRST	2


// Internal WSP error codes
#define WSP_ERR_REDIRECT	-320
#define WSP_ERR_UNAUTH		-401
#define WSP_ERR_UNHANDELED	-999
#define WSP_ERR_AUTHORIZE	-1401
#define WSP_ERR_BASE  		-2000
#define WSP_ERR_MEM   		-2001

// WSP PDU Types, according to WAP WSP 28-May-1999 V1.1 (SPEC-WSP-19990528.PDF Appendix A p87)
#define WSP_PDU_CONNECT 	    0x01
#define WSP_PDU_CONNECTREPLY 	    0x02
#define WSP_PDU_REDIRECT 	    0x03
#define WSP_PDU_REPLY 		    0x04
#define WSP_PDU_DISCONNECT 	    0x05
#define WSP_PDU_PUSH 		    0x06
#define WSP_PDU_CONFIRMEDPUSH 	    0x07
#define WSP_PDU_SUSPEND 	    0x08
#define WSP_PDU_RESUME 		    0x09
#define WSP_PDU_GET 		    0x40
#define WSP_PDU_OPTIONS		    0x41
#define WSP_PDU_HEAD		    0x42
#define WSP_PDU_DELETE		    0x43
#define WSP_PDU_TRACE		    0x44
#define WSP_PDU_POST		    0x60
#define WSP_PDU_PUT		    0x61

// WSP STATUS CODES, according to WAP WSP 28-May-1999 V1.1 (SPEC-WSP-19990528.PDF Appendix A p88)
#define WSP_STATUS_OK           0x20
#define WSP_STATUS_ACCEPTED     0x22
#define WSP_STATUS_MOVED_PERM   0x31
#define WSP_STATUS_MOVED_TEMP   0x32
#define WSP_STATUS_SEE_OTHER	0x33
#define WSP_STATUS_TEMP_REDIR	0x37
#define WSP_STATUS_AUTHREQ      0x41


/*
 * Bearer types (from the WDP specification).
 */
#define BT_IPv4			0x00
#define BT_IPv6			0x01
#define BT_GSM_USSD		0x02
#define BT_GSM_SMS		0x03
#define BT_ANSI_136_GUTS	0x04
#define BT_IS_95_SMS		0x05
#define BT_IS_95_CSD		0x06
#define BT_IS_95_PACKET_DATA	0x07
#define BT_ANSI_136_CSD		0x08
#define BT_ANSI_136_PACKET_DATA	0x09
#define BT_GSM_CSD		0x0A
#define BT_GSM_GPRS		0x0B
#define BT_GSM_USSD_IPv4	0x0C
#define BT_AMPS_CDPD		0x0D
#define BT_PDC_CSD		0x0E
#define BT_PDC_PACKET_DATA	0x0F
#define BT_IDEN_SMS		0x10
#define BT_IDEN_CSD		0x11
#define BT_IDEN_PACKET_DATA	0x12
#define BT_PAGING_FLEX		0x13
#define BT_PHS_SMS		0x14
#define BT_PHS_CSD		0x15
#define BT_GSM_USSD_GSM_SC	0x16
#define BT_TETRA_SDS_ITSI	0x17
#define BT_TETRA_SDS_MSISDN	0x18
#define BT_TETRA_PACKET_DATA	0x19
#define BT_PAGING_REFLEX	0x1A
#define BT_GSM_USSD_MSISDN	0x1B
#define BT_MOBITEX_MPAK		0x1C
#define BT_ANSI_136_GHOST	0x1D


extern int WspDecode(char *instr, UInt32 inlen, WebDataPtr webData, GlobalsType *g);
extern int WspFreePdu(WebDataPtr webData);
extern int WspInitPdu(WebDataPtr webData);
//extern int WspReadInt(char *instr, int *i);
extern UInt32 WspReadInt(char *instr, Int32 *i);
extern UInt32 WspGetReply(Int16 sock, char *buf, UInt32 buflen, UInt32 maxTimer, GlobalsType *g);
extern char *WspHostErrStr();
extern void WSPCloseSocket(Int16 *sock);
extern void GetWSPRedirectAddress (char *buf, unsigned short int * port, struct in_addr * address, GlobalsType *g);

extern Boolean SARaddSegment(UInt32 pos, Boolean last, char *data, UInt32 size, int type, int off, GlobalsType *g);
extern UInt32 SARreassemble(char *buf, Int16 buflen, GlobalsType *g);
extern Boolean SARdeleteData(GlobalsType *g);

//extern void SetFieldFromStr2(const Char *srcP , UInt16 fieldIndex);

extern Boolean Authenticate (Char *realm, Char *user, Char *pass);

extern void HTTPError (char* error);

#endif _wsp_h_
