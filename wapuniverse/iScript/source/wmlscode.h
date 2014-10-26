/* 
 * Copyright (c) 2004, TapTarget. All rights reserved.
 * Copyright (c) 2002-2004, Yuri Plaksyuk (york@noir.crocodile.org).
 *
 * http://www.taptarget.com/
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *      - Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 * 
 *      - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 * 
 *      - All advertising materials mentioning features or use of this
 *      software must display the following acknowledgement: This
 *      product includes software developed by TapTarget.
 * 
 *      - The name of TapTarget may not be used to endorse or
 *      promote products derived from this software without specific
 *      prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY TAPTARGET "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL TAPTARGET BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: wmlscode.h,v 1.1 2004/04/19 21:39:36 york Exp $ 
 */

#define RESERVED_00    0x00
#define JUMP_FW        0x01
#define JUMP_FW_W      0x02
#define JUMP_BW        0x03
#define JUMP_BW_W      0x04
#define TJUMP_FW       0x05
#define TJUMP_FW_W     0x06
#define TJUMP_BW       0x07
#define TJUMP_BW_W     0x08
#define CALL           0x09
#define CALL_LIB       0x0A
#define CALL_LIB_W     0x0B
#define CALL_URL       0x0C
#define CALL_URL_W     0x0D
#define LOAD_VAR       0x0E
#define STORE_VAR      0x0F
#define INCR_VAR       0x10
#define DECR_VAR       0x11
#define LOAD_CONST     0x12
#define LOAD_CONST_W   0x13
#define CONST_0        0x14
#define CONST_1        0x15
#define CONST_M1       0x16
#define CONST_ES       0x17
#define CONST_INVALID  0x18
#define CONST_TRUE     0x19
#define CONST_FALSE    0x1A
#define INCR           0x1B
#define DECR           0x1C
#define ADD_ASG        0x1D
#define SUB_ASG        0x1E
#define UMINUS         0x1F

#define ADD            0x20
#define SUB            0x21
#define MUL            0x22
#define DIV            0x23
#define IDIV           0x24
#define REM            0x25
#define B_AND          0x26
#define B_OR           0x27
#define B_XOR          0x28
#define B_NOT          0x29
#define B_LSHIFT       0x2A
#define B_RSSHIFT      0x2B
#define B_RSZSHIFT     0x2C
#define EQ             0x2D
#define LE             0x2E
#define LT             0x2F
#define GE             0x30
#define GT             0x31
#define NE             0x32
#define NOT            0x33
#define SCAND          0x34
#define SCOR           0x35
#define TOBOOL         0x36
#define POP            0x37
#define TYPEOF         0x38
#define ISVALID        0x39
#define RETURN         0x3A
#define RETURN_ES      0x3B
#define DBG            0x3C
#define RESERVED_3D    0x3D
#define RESERVED_3E    0x3E
#define RESERVED_3F    0x3F

#define STORE_VAR_0    0x40
#define STORE_VAR_1    0x41
#define STORE_VAR_2    0x42
#define STORE_VAR_3    0x43
#define STORE_VAR_4    0x44
#define STORE_VAR_5    0x45
#define STORE_VAR_6    0x46
#define STORE_VAR_7    0x47
#define STORE_VAR_8    0x48
#define STORE_VAR_9    0x49
#define STORE_VAR_10   0x4A
#define STORE_VAR_11   0x4B
#define STORE_VAR_12   0x4C
#define STORE_VAR_13   0x4D
#define STORE_VAR_14   0x4E
#define STORE_VAR_15   0x4F

#define LOAD_CONST_0   0x50
#define LOAD_CONST_1   0x51
#define LOAD_CONST_2   0x52
#define LOAD_CONST_3   0x53
#define LOAD_CONST_4   0x54
#define LOAD_CONST_5   0x55
#define LOAD_CONST_6   0x56
#define LOAD_CONST_7   0x57
#define LOAD_CONST_8   0x58
#define LOAD_CONST_9   0x59
#define LOAD_CONST_10  0x5A
#define LOAD_CONST_11  0x5B
#define LOAD_CONST_12  0x5C
#define LOAD_CONST_13  0x5D
#define LOAD_CONST_14  0x5E
#define LOAD_CONST_15  0x5F

#define CALL_0         0x60
#define CALL_1         0x61
#define CALL_2         0x62
#define CALL_3         0x63
#define CALL_4         0x64
#define CALL_5         0x65
#define CALL_6         0x66
#define CALL_7         0x67

#define CALL_LIB_0     0x68
#define CALL_LIB_1     0x69
#define CALL_LIB_2     0x6A
#define CALL_LIB_3     0x6B
#define CALL_LIB_4     0x6C
#define CALL_LIB_5     0x6D
#define CALL_LIB_6     0x6E
#define CALL_LIB_7     0x6F

#define INCR_VAR_0     0x70
#define INCR_VAR_1     0x71
#define INCR_VAR_2     0x72
#define INCR_VAR_3     0x73
#define INCR_VAR_4     0x74
#define INCR_VAR_5     0x75
#define INCR_VAR_6     0x76
#define INCR_VAR_7     0x77

#define RESERVED_78    0x78
#define RESERVED_79    0x79
#define RESERVED_7A    0x7A
#define RESERVED_7B    0x7B
#define RESERVED_7C    0x7C
#define RESERVED_7D    0x7D
#define RESERVED_7E    0x7E
#define RESERVED_7F    0x7F

#define JUMP_FW_0      0x80
#define JUMP_FW_1      0x81
#define JUMP_FW_2      0x82
#define JUMP_FW_3      0x83
#define JUMP_FW_4      0x84
#define JUMP_FW_5      0x85
#define JUMP_FW_6      0x86
#define JUMP_FW_7      0x87
#define JUMP_FW_8      0x88
#define JUMP_FW_9      0x89
#define JUMP_FW_10     0x8A
#define JUMP_FW_11     0x8B
#define JUMP_FW_12     0x8C
#define JUMP_FW_13     0x8D
#define JUMP_FW_14     0x8E
#define JUMP_FW_15     0x8F
#define JUMP_FW_16     0x90
#define JUMP_FW_17     0x91
#define JUMP_FW_18     0x92
#define JUMP_FW_19     0x93
#define JUMP_FW_20     0x94
#define JUMP_FW_21     0x95
#define JUMP_FW_22     0x96
#define JUMP_FW_23     0x97
#define JUMP_FW_24     0x98
#define JUMP_FW_25     0x99
#define JUMP_FW_26     0x9A
#define JUMP_FW_27     0x9B
#define JUMP_FW_28     0x9C
#define JUMP_FW_29     0x9D
#define JUMP_FW_30     0x9E
#define JUMP_FW_31     0x9F

#define JUMP_BW_0      0xA0
#define JUMP_BW_1      0xA1
#define JUMP_BW_2      0xA2
#define JUMP_BW_3      0xA3
#define JUMP_BW_4      0xA4
#define JUMP_BW_5      0xA5
#define JUMP_BW_6      0xA6
#define JUMP_BW_7      0xA7
#define JUMP_BW_8      0xA8
#define JUMP_BW_9      0xA9
#define JUMP_BW_10     0xAA
#define JUMP_BW_11     0xAB
#define JUMP_BW_12     0xAC
#define JUMP_BW_13     0xAD
#define JUMP_BW_14     0xAE
#define JUMP_BW_15     0xAF
#define JUMP_BW_16     0xB0
#define JUMP_BW_17     0xB1
#define JUMP_BW_18     0xB2
#define JUMP_BW_19     0xB3
#define JUMP_BW_20     0xB4
#define JUMP_BW_21     0xB5
#define JUMP_BW_22     0xB6
#define JUMP_BW_23     0xB7
#define JUMP_BW_24     0xB8
#define JUMP_BW_25     0xB9
#define JUMP_BW_26     0xBA
#define JUMP_BW_27     0xBB
#define JUMP_BW_28     0xBC
#define JUMP_BW_29     0xBD
#define JUMP_BW_30     0xBE
#define JUMP_BW_31     0xBF

#define TJUMP_FW_0     0xC0
#define TJUMP_FW_1     0xC1
#define TJUMP_FW_2     0xC2
#define TJUMP_FW_3     0xC3
#define TJUMP_FW_4     0xC4
#define TJUMP_FW_5     0xC5
#define TJUMP_FW_6     0xC6
#define TJUMP_FW_7     0xC7
#define TJUMP_FW_8     0xC8
#define TJUMP_FW_9     0xC9
#define TJUMP_FW_10    0xCA
#define TJUMP_FW_11    0xCB
#define TJUMP_FW_12    0xCC
#define TJUMP_FW_13    0xCD
#define TJUMP_FW_14    0xCE
#define TJUMP_FW_15    0xCF
#define TJUMP_FW_16    0xD0
#define TJUMP_FW_17    0xD1
#define TJUMP_FW_18    0xD2
#define TJUMP_FW_19    0xD3
#define TJUMP_FW_20    0xD4
#define TJUMP_FW_21    0xD5
#define TJUMP_FW_22    0xD6
#define TJUMP_FW_23    0xD7
#define TJUMP_FW_24    0xD8
#define TJUMP_FW_25    0xD9
#define TJUMP_FW_26    0xDA
#define TJUMP_FW_27    0xDB
#define TJUMP_FW_28    0xDC
#define TJUMP_FW_29    0xDD
#define TJUMP_FW_30    0xDE
#define TJUMP_FW_31    0xDF

#define LOAD_VAR_0     0xE0
#define LOAD_VAR_1     0xE1
#define LOAD_VAR_2     0xE2
#define LOAD_VAR_3     0xE3
#define LOAD_VAR_4     0xE4
#define LOAD_VAR_5     0xE5
#define LOAD_VAR_6     0xE6
#define LOAD_VAR_7     0xE7
#define LOAD_VAR_8     0xE8
#define LOAD_VAR_9     0xE9
#define LOAD_VAR_10    0xEA
#define LOAD_VAR_11    0xEB
#define LOAD_VAR_12    0xEC
#define LOAD_VAR_13    0xED
#define LOAD_VAR_14    0xEE
#define LOAD_VAR_15    0xEF
#define LOAD_VAR_16    0xF0
#define LOAD_VAR_17    0xF1
#define LOAD_VAR_18    0xF2
#define LOAD_VAR_19    0xF3
#define LOAD_VAR_20    0xF4
#define LOAD_VAR_21    0xF5
#define LOAD_VAR_22    0xF6
#define LOAD_VAR_23    0xF7
#define LOAD_VAR_24    0xF8
#define LOAD_VAR_25    0xF9
#define LOAD_VAR_26    0xFA
#define LOAD_VAR_27    0xFB
#define LOAD_VAR_28    0xFC
#define LOAD_VAR_29    0xFD
#define LOAD_VAR_30    0xFE
#define LOAD_VAR_31    0xFF
