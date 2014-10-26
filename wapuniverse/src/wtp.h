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
#ifndef _wtp_h_
#define _wtp_h_

#include "WAP.h"
#include "WAPUniverse.h"

// Use Segmentation and Reassembly
#define USE_SAR_SUPPORT


#define GET_WTP_PDU_TYPE(hdr)  (hdr[0] >> 3)
/*int get_wtp_pdu_type(unsigned char hdr[]) {
	return (hdr[0] >> 3);
}*/

#define WTP_PDU_INVOKE  1
#define WTP_PDU_RESULT  2
#define WTP_PDU_ACK     3
#define WTP_PDU_ABORT   4
#define WTP_PDU_SEGINV	5
#define WTP_PDU_SEGRES	6
#define WTP_PDU_NEGACK	7



#define WSP_Connect 		0x01
#define WSP_ConnectReply 	0x02
#define WSP_Redirect 		0x03
#define WSP_Get 		0x40
#define WSP_Reply 		0x04
#define WSP_Disconnect 		0x05
#define WTP_Ack 		0x03 //0x18
#define WTP_Ack1 		0x18
#define WTP_TidVe 		0x1c
#define WTP_Abort 		0x04 //0x20
#define WTP_Abort1 		0x20
#define WTP_User_Abort 		0x21
#define WTP_Result 		0x02
#define WTP_SAR_Result 		0x06
#define WTP_NegAck		0x38

/*
**  In this case it does not matter what is the byte order
*/
#define SET_GTR( hdr ) hdr[0] |= 0x04
#define SET_TID( hdr, tid) hdr[1] |= (0x7f & ((tid) >> 8)); hdr[2] = (char)(tid)
#define GET_TID( hdr ) (((hdr[1] & 0x7f) << 8) + hdr[2])
#define CONSTRUCT_EXPECTED_REPLY_HDR( dest, template, tid ) if (sizeof(dest) < sizeof(template)) ErrDisplay("Buffer Overflow in WTP.C"); memcpy( dest, template, sizeof(template)); SET_TID( dest, tid )

#ifndef min
#define min(a,b) (a < b ? a : b)
#endif


extern int WTPConnect(int sock, char *gwip, char *urlStr, Boolean remember, GlobalsType *g );
extern int wtp_get(int sock, char *gwip, char *urlStr, Boolean remember, Err *error, GlobalsType *g );
extern int wtp_post(int sock, char *gwip, char *urlStr, Boolean remember, Err *error, GlobalsType *g );
void addOctets( unsigned char in[], unsigned char out[], int inoff, int outoff);
int WTPHandleConnectResp(int sock, char *gwip, GlobalsType *g );
//extern int WTPDisconnect (Int16 sock, GlobalsType *g);
extern int WTPAbort (Int16 sock, GlobalsType *g);
extern int SARWTPAck (Int16 sock, int t, Boolean endGroup, Boolean haveTPI, GlobalsType *g);
extern int WTPAck (Int16 sock, GlobalsType *g);
extern unsigned short next_tid(unsigned short old_tid);
extern int WTPTidVe (Int16 sock, GlobalsType *g);
extern int SARWTPNegAck (Int16 sock, Int16 curPkt, Int16 pktCount, GlobalsType *g);

#endif _wtp_h_
