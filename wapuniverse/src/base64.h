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


struct buffer_st {
  char *data;
  int length;
  char *ptr;
  int offset;
};

void buffer_new(struct buffer_st *b);
void buffer_add(struct buffer_st *b, char c);
void buffer_delete(struct buffer_st *b);

void base64_encode(struct buffer_st *b, const char *source, int length);
/*
void base64_decode(struct buffer_st *b, const char *source, int length);
*/

/*
#define DEBUG_MALLOC
 */

/*
#ifdef DEBUG_MALLOC
void *_malloc_real(size_t s, char *file, int line);
void _free_real(void *p, char *file, int line);

#define malloc(s)	_malloc_real(s,__FILE__,__LINE__)
#define free(p)		_free_real(p, __FILE__,__LINE__)
#endif
*/

