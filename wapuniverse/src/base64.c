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
/*

	   Encode or decode file as MIME base64 (RFC 1341)

			    by John Walker
		       http://www.fourmilab.ch/

		This program is in the public domain.

*/


// #include <stdio.h>

/*  ENCODE  --	Encode binary file into base64.  */
// #include <stdlib.h>

#include 	<PalmOS.h>
#include 	"base64.h"
#include 	"WAPUniverse.h"
#include	"PalmUtils.h"

static unsigned char dtable[512];


/*
 */

void buffer_new( struct buffer_st *b )
{
    b->length = 512;
    b->data = (char *)Malloc( sizeof(char) * (b->length) );
    b->ptr = b->data;
    b->offset = 0;
}


/*
 */

void buffer_add( struct buffer_st *b, char c )
{
    *(b->ptr++) = c;
    b->offset++;
    if ( b->offset == b->length ) {
        b->length += 512;
        MemPtrResize( b->data, b->length );
        b->ptr = b->data + b->offset;
    }
}


/*
 */

void buffer_delete( struct buffer_st *b )
{
    Free( b->data );
    b->length = 0;
    b->offset = 0;
    b->ptr = NULL;
    b->data = NULL;
}


/*
 */

void base64_encode( struct buffer_st *b, const char *source, int length )
{
    int i;
    int hiteof = 0;
    int offset = 0;
    int olen;

    olen = 0;

    buffer_new( b );

    /*	Fill dtable with character encodings.  */

    for (i = 0; i < 26; i++) {
        dtable[i] = 'A' + i;
        dtable[26 + i] = 'a' + i;
    }
    for (i = 0; i < 10; i++) {
        dtable[52 + i] = '0' + i;
    }
    dtable[62] = '+';
    dtable[63] = '/';

    while (!hiteof) {
        unsigned char igroup[3];
        unsigned char ogroup[4];
        int c;
        int n;

        igroup[0] = igroup[1] = igroup[2] = 0;

        for (n = 0; n < 3; n++) {
            c = *(source++);
            offset++;
            if (offset > length) {
                hiteof = 1;
                break;
            }
            igroup[n] = (unsigned char) c;
        }
        if (n > 0) {
            ogroup[0] = dtable[igroup[0] >> 2];
            ogroup[1] = dtable[((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
            ogroup[2] = dtable[((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
            ogroup[3] = dtable[igroup[2] & 0x3F];

            /* Replace characters in output stream with "=" pad
               characters if fewer than three characters were
               read from the end of the input stream. */

            if (n < 3) {
                ogroup[3] = '=';
                if (n < 2) {
                    ogroup[2] = '=';
                }
            }
            for (i = 0; i < 4; i++) {
                buffer_add(b, ogroup[i]);
#if 0
                /* Break into lines.. we don't want that - MJR */
                if (!(b->offset % 72)) {
                    buffer_add(b, '\r');
                    buffer_add(b, '\n');
                }
#endif /* 0 */
            }
        }
    }
    buffer_add(b, '\0');
}

/*
 */

#if 0
void base64_decode(struct buffer_st *bfr, const char *source, int length)
{
    int i;
    int offset = 0;
    int endoffile;
    int count;

    buffer_new(bfr);

    for (i = 0; i < 255; i++) {
	dtable[i] = 0x80;
    }
    for (i = 'A'; i <= 'Z'; i++) {
        dtable[i] = 0 + (i - 'A');
    }
    for (i = 'a'; i <= 'z'; i++) {
        dtable[i] = 26 + (i - 'a');
    }
    for (i = '0'; i <= '9'; i++) {
        dtable[i] = 52 + (i - '0');
    }
    dtable['+'] = 62;
    dtable['/'] = 63;
    dtable['='] = 0;

    endoffile = 0;

    /*CONSTANTCONDITION*/
    while (1) {
	unsigned char a[4], b[4], o[3];

	for (i = 0; i < 4; i++) {
	    int c;
	    while (1) {
	      c = *(source++);
	      offset++;
	      if (offset > length) endoffile = 1;
	      if (isspace(c) || c == '\n' || c == '\r') continue;
	      break;
	    }

	    if (endoffile) {
	      /*
		if (i > 0) {
                    fprintf(stderr, "Input file incomplete.\n");
		    exit(1);
		}
	      */
		return;
	    }

	    if (dtable[c] & 0x80) {
	      /*
	      fprintf(stderr, "Offset %i length %i\n", offset, length);
	      fprintf(stderr, "character '%c:%x:%c' in input file.\n", c, c, dtable[c]);
	      exit(1);
	      */
	      i--;
	      continue;
	    }
	    a[i] = (unsigned char) c;
	    b[i] = (unsigned char) dtable[c];
	}
	o[0] = (b[0] << 2) | (b[1] >> 4);
	o[1] = (b[1] << 4) | (b[2] >> 2);
	o[2] = (b[2] << 6) | b[3];
        i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);
	count = 0;
	while (count < i) {
	  buffer_add(bfr, o[count++]);
	}
	if (i < 3) {
	    return;
	}
    }
}
#endif /* 0 */
