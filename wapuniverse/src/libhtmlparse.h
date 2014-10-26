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
/* libhtmlparse: Quick and simple HTML parser
   Mooneer Salem (mooneer@translator.cx) */

#ifndef LIBHTML_H
#define LIBHTML_H

/****************
* from the guts *
*****************/

/* struct holding the arguments of tags */

struct ArgvTable {
	char *arg, *val;
};

/* struct for streamparsing and errorhandling */

struct ErrorTree {
	unsigned int line;
	unsigned char type;
	char *problem;
};

/* obligatory prototypes */

extern const char *parseText     	(const char *);
extern const char *parseComment  	(const char *);
extern const char *parseEndTag   	(const char *);
extern const char *parsePHP      	(const char *);
extern const char *parseXMLtag   	(const char *);
extern const char *eatUp         	(const char *);
extern const char *eatUpText     	(const char *);
extern const char *parseXML      	(const char *);
extern const char *parseStartTag 	(const char *);
extern const char *parseDecl     	(const char *);
extern const char *parseForEntities	(const char *);

/**********************************************************************/

/***********************************************************************
*                        The CallBacks                                 *
* You may call one ore several or even all callbacks. Except of the    *
* XHTMLCallBack, all CallBacks will work as expected and described     *
*                                                                      *
* XHTMLCallBack:                                                       *
* The XHTMLCallBack is a special case, because you can decide, if the  *
* XHTML specific tags should be handeled as a start- AND endtag, or    *
* as an XHTML tag. If you call nothing, except start and endtag, the   *
* behaviour is, that you'll get a start AND an endtag called back.     *
* If you call XHTMLCallBack, it will only give you the XHTML call back.*
*                                                                      *
* If you are in doubt or simply confused now, call XHTMLCallBack()     *
************************************************************************/

/* handle comments and javascript */
extern int (*commentCallBack) (char *comment);
extern int (*commentStartCallBack) ();
extern int (*commentEndCallBack) ();

/* Declaration e.g. <!DOCTYPE HTML ... */
extern int (*declCallBack) (char *tag, /*@null@*/ struct ArgvTable *args, int numargs);

/* Start tag e.g. <html>, with arguments, args may be NULL, numargs may be 0 */
extern int (*startCallBack) (char *tag, /*@null@*/ struct ArgvTable *args, int numargs);

/* End tag e.g. </html>*/
extern int (*endCallBack) (char *tag);

/* handle plain text */
extern int (*textCallBack) (char *text);
extern int (*textStartCallBack) ();
extern int (*textEndCallBack) ();

/* PHP inserts. BUG(?): if someone prints another PHP function from this PHP function
   our lib will get confused. */
extern int (*phpCallBack) (char *text);

/* empty tags like <hr/>, <br/>, with arguments, args may be NULL, numargs may be 0 */
extern int (*XHTMLCallBack) (char *tag, /*@null@*/ struct ArgvTable *args, int numargs);

/* XML tags <?xml>, with arguments, args may be NULL, numargs may be 0 */
extern int (*xmlCallBack) (char *tag, /*@null@*/ struct ArgvTable *args, int numargs);

/* entities like &auml;,&#228; text will inherit all chars between '&' and ';' */

extern int (*entityCallBack) (char *text);

/* the script inside <script> ... </script> */
/* int (*scriptCallBack) (char *text);  */

/* parse the inputfile */
/*@null@*/ struct ErrorTree *parse (const char *html);

/**********************************************************************/

/* more of the obligatory prototypes ( one should clean up here! ;-) */

/* puts found arguments into the argument struct */
extern struct ArgvTable *addArgToTable (/*@null@*/ struct ArgvTable *args, char *arg, /*@null@*/ char *val);

/* cleans the argument struct */
extern void freeArgs (/*@null@*/ struct ArgvTable *args);

#endif /* LIBHTML_H */

