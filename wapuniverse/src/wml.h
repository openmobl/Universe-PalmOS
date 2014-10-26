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
#ifndef _wml_h_
#define _wml_h_

#include "WAP.h"
#include "wsp.h"
#include "WAPUniverse.h"

#include "image.h"

#define TMP_BLOCK_SIZE	4096 // temporary block size

#define NOEVENT		0
#define ONTIMER		1
#define ONENTERFORWARD	2
#define ONPICK		3
#define ONENTERBACKWARD	4
#define IGNORE		5


#define MAX_HISTORY	  20 // min. 10 cfr SPEC-WML 12.2.1 

#define ACTION_RENDER 	1
#define ACTION_PARSE  	2
#define ACTION_SRC    	3
#define ACTION_SLEEP  	4

#define WML_ERR_BASE  	-1000
#define WML_ERR_MEM   	-1001

//
// WBXML bitmasks
//
#define WBXML_has_attributes	    0x80
#define WBXML_has_content	    0x40
#define WBXML_tag_id		    0x3F

#define SWITCH_PAGE 0x00
#define END         0x01
#define ENTITY      0x02
#define STR_I       0x03
#define LITERAL     0x04
#define EXT_I_0     0x40
#define EXT_I_1     0x41
#define EXT_I_2     0x42
#define PI          0x43
#define LITERAL_C   0x44
#define EXT_T_0     0x80
#define EXT_T_1     0x81
#define EXT_T_2     0x82
#define STR_T       0x83
#define LITERAL_A   0x084
#define EXT_0       0x0c0
#define EXT_1       0x0c1
#define EXT_2       0x0c2
#define OPAQUE      0x0c3
#define LITERAL_AC  0x0c4

// Charsets
#define UTF_8	    0x6A
#define US_ASCII    UTF_8 // 0x01
#define ISO_8859_1  0x04
#define USC_2	    0x03E8



////
// WML TAG Tokens, according to WAP WML 16-Jun-1999 V1.1 (SPEC-WML-19990616.PDF Ch.14.3.2)
//  
#define WML_a		    0x1C //implemented
#define WML_anchor	    0x22 //implemented
#define WML_access	    0x23 //implemented
#define WML_b		    0x24 //implemented
#define WML_big		    0x25 //implemented
#define WML_br		    0x26 //implemented
#define WML_card	    0x27 //implemented
#define WML_do		    0x28 //implemented
#define WML_em		    0x29 //implemented
#define WML_fieldset	    0x2A
#define WML_go		    0x2B //implemented
#define WML_head	    0x2C //implemented
#define WML_i		    0x2D //implemented
#define WML_img		    0x2E //implemented
#define WML_input	    0x2F //implemented
#define WML_meta	    0x30
#define WML_noop	    0x31 //implemented
#define WML_p		    0x20 //implemented
#define WML_postfield	    0x21 //implemented
#define WML_prev	    0x32 //implemented
#define WML_onevent	    0x33 //implemented
#define WML_optgroup	    0x34
#define WML_option	    0x35 //implemented
#define WML_refresh	    0x36 //implemented
#define WML_select	    0x37 //implemented
#define WML_setvar	    0x3E //implemented
#define WML_small	    0x38 //implemented
#define WML_strong	    0x39 //implemented
#define WML_pre		    0x1B //implemented
#define WML_table	    0x1F //in progress
#define WML_td		    0x1D //in progress
#define WML_template	    0x3B //implemented
#define WML_timer	    0x3C //implemented
#define WML_tr		    0x1E //in progress
#define WML_u		    0x3d //implemented
#define WML_wml 	    0x3F //implemented

//
// WML Attribute Start Tokens, according to WAP WML 16-Jun-1999 V1.1 (SPEC-WML-19990616.PDF Ch.14.3.3)
//  
#define WML_accept_charset	    0x05
#define WML_align		    0x52
#define WML_align_bottom	    0x06
#define WML_align_center	    0x07
#define WML_align_left		    0x08
#define WML_align_middle	    0x09
#define WML_align_right		    0x0A
#define WML_align_top		    0x0B
#define WML_alt			    0x0C
#define WML_accesskey		    0x5E
#define WML_class		    0x54
#define WML_columns		    0x53
#define WML_content		    0x0D
#define WML_content_wmlc	    0x5C
#define WML_domain		    0x0F
#define WML_emptyok_false	    0x10
#define WML_emptyok_true	    0x11
#define WML_format		    0x12
#define WML_forua_false		    0x56
#define WML_forua_true		    0x57
#define WML_height		    0x13
#define WML_href		    0x4A
#define WML_href_http		    0x4B	
#define WML_href_https		    0x4C
#define WML_hspace		    0x14
#define WML_http_equiv		    0x5A
#define WML_http_equiv_content	    0x5B
#define WML_http_equiv_expires	    0x5D
#define WML_id			    0x55
#define WML_ivalue		    0x15
#define WML_iname		    0x16
#define WML_label		    0x18
#define WML_localsrc		    0x19
#define WML_maxlength		    0x1C
#define WML_method_get		    0x1B
#define WML_method_post		    0x1C
#define WML_mode_nowrap		    0x1D
#define WML_mode_wrap		    0x1E
#define WML_multiple_false	    0x1F
#define WML_multiple_true	    0x20
#define WML_name		    0x21
#define WML_newcontect_false	    0x22
#define WML_newcontext_true	    0x23
#define WML_onenterbackward	    0x25
#define WML_onenterforward	    0x26
#define WML_onpick		    0x24
#define WML_ontimer		    0x27
#define WML_optional_false	    0x28
#define WML_optional_true	    0x29
#define WML_path		    0x2A
#define WML_scheme		    0x2E
#define WML_sendreferer_false	    0x2F
#define WML_sendreferer_true	    0x30
#define WML_size		    0x31
#define WML_src			    0x32
#define WML_src_http		    0x58
#define WML_src_https		    0x59
#define WML_ordered_true	    0x33
#define WML_ordered_false	    0x34
#define WML_tabindex		    0x35
#define WML_title		    0x36
#define WML_type		    0x37
#define WML_type_accept		    0x38
#define WML_type_delete		    0x39
#define WML_type_help		    0x3A
#define WML_type_password	    0x3B
#define WML_type_onpick		    0x3C
#define WML_type_onenterback	    0x3D
#define WML_type_onenterforward	    0x3E
#define WML_type_ontimer	    0x3F
#define WML_type_options	    0x45
#define WML_type_prev		    0x46
#define WML_type_reset		    0x47
#define WML_type_text		    0x48
#define WML_type_vnd		    0x49
#define WML_value		    0x4D
#define WML_vspace		    0x4E
#define WML_width		    0x4F
#define WML_xml_lang		    0x50


//
// WML Attribute Value Tokens, according to WAP WML 16-Jun-1999 V1.1 (SPEC-WML-19990616.PDF Ch.14.3.4)
//  
#define WML_com			        0x85
#define WML_edu			        0x86
#define WML_net			        0x87
#define WML_org			        0x88
#define WML_accept		        0x89
#define WML_bottom		        0x8A
#define WML_clear		        0x8B
#define WML_delete		        0x8C
#define WML_help		        0x8D
#define WML_http		        0x8E
#define WML_http_www		        0x8F
#define WML_https		        0x90
#define WML_https_www		        0x91
#define WML_middle		        0x93
#define WML_nowrap		        0x94
#define WML_val_onenterbackward	        0x96
#define WML_val_onenterforward	        0x97
#define WML_val_onpick		        0x95
#define WML_val_ontimer		        0x98
#define WML_options		        0x99
#define WML_password		        0x9A
#define WML_reset		        0x9B
#define WML_text		        0x9D
#define WML_top			        0x9E
#define WML_unknown		        0x9F
#define WML_wrap		        0xA0
#define WML_www			        0xA1

extern Boolean needTimer;
extern Boolean isLink;
extern Boolean isAnchor;
extern Boolean postStart;
extern Boolean onenterforward;
extern Boolean onenterbackward;
extern Boolean onevent;
extern Boolean isRefresh;
extern Boolean isOption;
extern Boolean isOnpickLink;
extern Boolean isOnenterRefresh;
extern Boolean isOnforwardPost;
extern int     off;

extern int wbxml_parse(WebDataPtr webData, int action, WMLDeckPtr wmldck, int offset, GlobalsType *g);
extern int wbxml_free_wmldck( WMLDeckPtr wmldck);

extern void drawDo (char *label,  GlobalsType *g);
extern Boolean checkAccess(Char *attr, unsigned char *url, int type);

extern Char *WmlConvertEncoding(Char *instr, WMLDeckPtr wmldck);

#endif _wml_h_
