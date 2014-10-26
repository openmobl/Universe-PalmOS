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
/* libhtmlparse: Quick and simple SAX like HTML parser
   Mooneer Salem (mooneer@translator.cx) */
/* Adapted to PalmOS by Donald C. Kirker */

#include 	<PalmOS.h>
#include	<MemGlue.h>

//#include 	<stdio.h>
//#include 	<stdlib.h>
//#include 	<string.h>

#include 	"libhtmlparse.h"


typedef unsigned long size_t;

int (*commentCallBack) (char *comment);
int (*commentStartCallBack) ();
int (*commentEndCallBack) ();
int (*declCallBack) (char *tag, /*@null@*/ struct ArgvTable *args, int numargs);
int (*startCallBack) (char *tag, /*@null@*/ struct ArgvTable *args, int numargs);
int (*endCallBack) (char *tag);
int (*textCallBack) (char *text);
int (*textStartCallBack) ();
int (*textEndCallBack) ();
int (*phpCallBack) (char *text);
int (*XHTMLCallBack) (char *tag, /*@null@*/ struct ArgvTable *args, int numargs);
int (*xmlCallBack) (char *tag, /*@null@*/ struct ArgvTable *args, int numargs);
int (*entityCallBack) (char *text);



/* needed to pass text in <script> tags verbatim */
static unsigned int lhtml_script_passthru;

#ifdef _INLINE_CODE
#ifndef inline
#define inline __inline__ /* works also with -ansi */
#endif
#endif

#define strncasecmp 	StrNCaselessCompare
#define	memcpy(x,y,z)	MemMove(x,(void *)y,z)//(MemMove(x,(void *)y,z) ? x : x)
#define	strncpy(x,y,z)	StrNCopy((Char*)x,(Char*)y,(Int16)z)
#define malloc(x)	MemGluePtrNew(x)
#define free(x)		MemPtrFree(x)
#define strlen(x)	StrLen(x)
//#define calloc(x,y)
//#define realloc(x,y)

/*static inline int strncasecmp(const char *dst, const char *src, size_t num)
{
  while (num && *dst && *src && (*dst++ == *src++))
    num--;
  if( !num )
    return 0;
  if (*dst && *src)
    return *--src - *--dst;
  else
    return *src - *dst;
}

static inline char *strncpy(char *dst, const char *src, size_t num)
{
  register char *ds = dst;
  while (num-- && (*dst++ = *src++));
  if (num >= 0)
    while (num--)
      *dst++ = 0;
  return ds;
}

static inline void *memcpy(void *dst, const void *src, size_t num)
{
  	void *ds = MemPtrNew(num);

  	MemMove(ds, dst, num);

  	//MemPtrFree(dst);

  	return ds;
}*/

static inline char tolower(register char c)
{
  if( c >= 'Z' && c <= 'Z' )
    return c + 32;
  return c;
}

static Boolean isspace(Char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'; 
// added \r for better support with a multiline attributes
}

static inline void *realloc( void *ptr, size_t size) {
	void *ret = malloc(size);

	MemMove(ret, ptr, size);

	MemPtrFree(ptr);

	return ret;
}

static inline void *calloc(size_t num, size_t size)
{
	void *memory = MemPtrNew(num * size);

	if (memory) {
		MemSet(memory, num * size, 0);
	}

	return memory;
}

static int numArgs=0, numArgsStatus=0;

/* Define INLINE here, if you mean, that the result is faster
   May depend on your compiler  */

#ifdef _INLINE_CODE
inline
#endif

/* argument caching (e.g width="80%") */

struct ArgvTable *addArgToTable (struct ArgvTable *args, char *arg, char *val) {
        numArgs++;
        if (args == NULL) {
                args = (struct ArgvTable*) calloc(1, sizeof(struct ArgvTable)*(numArgs+1));
        } else {
		args = (struct ArgvTable*) realloc(args, sizeof(struct ArgvTable)*(numArgs+1));
	}

        ErrFatalDisplayIf(!args, "No arguments");

        args[numArgs-1].arg = arg;
        args[numArgs-1].val = val;
        return args;
}


#ifdef _INLINE_CODE
inline
#endif

/* clean up memory */

void freeArgs (struct ArgvTable *args) {
        register int i;

        if(args != NULL) {
                for(i=0; i<numArgs; i++) {
                        if (args[i].arg) free(args[i].arg);
                        if (args[i].val) free(args[i].val);
                }

                free(args);
                args=NULL;
                numArgs=0;
        }
}


#ifdef _INLINE_CODE
inline
#endif
const char *parseText(const char *html) {
        char *tmp;
        const char *tmp2;
        register int ret=0;

        //while(*html != '\0' && isspace((int)*html)) html++; // now handled by the html presentation engine
	if (*html == '\0') return html;

        if (*html == '<') return html;

        tmp2 = html;
        while(*html != '\0' && *html != '<') html++;

        tmp = (char *)calloc(1, (size_t)(html-tmp2+1));
        if (!tmp) return "";

        memcpy(tmp, tmp2, (size_t)(html-tmp2));

	if (strlen(tmp) > 0) {
	        if (textStartCallBack) {
        	        ret = (*textStartCallBack)();
                	if (ret != 0) {
                        	free(tmp);
                	        return "";
        	        }
	        }

	        if (textCallBack) {

			if (entityCallBack){ /* that is textCallBack(text)
						with entityCallBack(entity) as an extrabonus */
				//printf("entity is here\n");
				(void)parseForEntities(tmp);
			}

			else{
        	        	ret = (*textCallBack)(tmp);
                		if (ret != 0) {
                        		free(tmp);
                 	       		return "";
                			}
			}
        	}

	        if (textEndCallBack) {
        	        ret = (*textEndCallBack)();
                	if (ret != 0) {
                        	free(tmp);
                 	       return "";
                	}
        	}
	}

        free(tmp);

        if (*(html+1) == '>') html += 2;
        return html;
}


#ifdef _INLINE_CODE
inline
#endif
const char *parseComment (const char *html) {
        char *tmp;
        const char *tmp2;
        register int ret=0;


	//while (*html == '-' || isspace((int)*html)) html++;
	while (*html == '-') html++;


        tmp2 = html;
        while(*html != '\0' && !(*html == '-' && *(html+1) == '-' && *(html+2) == '>')) html++;

        tmp = (char *)calloc(1, (size_t)(html-tmp2+1));
        if (!tmp) return "";

        memcpy(tmp, tmp2, (size_t)(html-tmp2));

	if (*(html+3) != '\0') html += 3;

        if (commentStartCallBack) {
                ret = (*commentStartCallBack)();
                if (ret != 0) {
                        free(tmp);
                        return "";
                }
        }

        if (commentCallBack) {
                ret = (*commentCallBack)(tmp);
                if (ret != 0) {
                        free(tmp);
                        return "";
                }
        }

        if (commentEndCallBack) {
                ret = (*commentEndCallBack)();
                if (ret != 0) {
                        free(tmp);
                        return "";
                }
        }

        free(tmp);
        return html;
}



#ifdef _INLINE_CODE
inline
#endif
const char *parseEndTag(const char *html) {
        char *tmp;
        const char *tmp2;
        register int ret=0;

        html++;

        tmp2 = html;
        while(*html != '\0' && *html != '>') html++;

        tmp =(char *) calloc(1, (size_t)(html-tmp2+1));
        if (!tmp) return "";

        memcpy(tmp, tmp2, (size_t)(html-tmp2));

        if (endCallBack) {
                ret = (*endCallBack)(tmp);
                if (ret != 0) {
                        free(tmp);
                        return "";
                }
        }

        if (*html == '>') html++;

        free(tmp);
        return html;
}


#ifdef _INLINE_CODE
inline
#endif
const char *parsePHP(const char *html) {
	const char *tmp;
	char *tmp2;
	register int ret=0;

	html += 4;
	while(*html != '\0' && isspace((int)*html)) html++;

	tmp = html;

	while(*html != '\0' && !(*html == '?' && *(html+1) == '>')) html++;
	tmp2 = (char *)calloc(1, (size_t)(html-tmp+1));
	if (!tmp2) return "";

	memcpy(tmp2, tmp, (size_t)(html-tmp));

	if (phpCallBack) {
		ret = (*phpCallBack)(tmp2);
		if (ret != 0) {
			free(tmp2);
			return "";
		}
	}

	free(tmp2);
	html += 2;
	return html;

}



#ifdef _INLINE_CODE
inline
#endif

/* parse the XML tag itself */

const char *parseXMLtag(const char *html) {

        char *tag, *name, *value;
        const char *tmp;
        int ret;
        struct ArgvTable *tmp2 = NULL;

        numArgs = 0;
        tmp = html;
        while(*html != '\0' && !isspace((int)*html) && *html != '>') html++;

	/* you may want to upper/lower tags, so I leave the tag itself untouched */
        tag = (char *)calloc(1, (size_t)(html-tmp+1));
        if (!tag) {
                return "";
        }

        memcpy(tag, tmp, (size_t)(html-tmp));

        if (*html == '>') {
		if (xmlCallBack != NULL) {
	                ret = (*xmlCallBack)(tag, NULL, 0);
	        	free(tag);
        	        if (*html == '>') html++;
                	return((ret != 0) ? (char *) "" : html);
		}
        }


        while(*html != '\0' && isspace((int)*html)) html++;

        while(*html != '\0' && *html != '>' ) {
		while(isspace((int)*html)) html++;
		if (*html == '>') break;

                tmp = html;
                while(*html != '\0' && !isspace((int)*html) && *html != '=' && *html != '>') html++;
                name = (char *)calloc(1, (size_t)(html-tmp+1));
                if (!name) {
                        free(tag);
			tag = NULL;
                        return "";
                }

                memcpy(name, tmp, (size_t)(html-tmp));

                if (isspace((int)*html)) {
                        tmp2 = addArgToTable(tmp2, name, NULL);
                        while(*html != '\0' && isspace((int)*html) && *html != '>') html++;
                }

                if (*html == '>') {
                        tmp2 = addArgToTable(tmp2, name, NULL);
                        html++;
                        break;
                }

                if (*html == '=') html++;

                if (*html != '"' && *html != '\'') {
                        tmp = html;
                        while(*html != '\0' && *html != '>' && !isspace((int)*html)) html++;
                        value = (char *)calloc(1, (size_t)(html-tmp+1));
                        if (!value) {
                                free(name);
				name = NULL;
                                free(tag);
				tag = NULL;

				if (tmp2 != NULL) {
	                                freeArgs(tmp2);
					tmp2 = NULL;
				}
                                return "";
                        }

                        memcpy(value, tmp, (size_t)(html-tmp));
                        tmp2 = addArgToTable(tmp2, name, value);
                } else if (*html == '"') {
                        html++;
                        tmp = html;
                        while(*html != '\0' && !(*html == '"' && *(html-1) != '\\')) html++;
                        value = (char *) calloc(1, (size_t)(html-tmp+1));
                        if (!value) {
                                free(name);
				name = NULL;
                                free(tag);
				tag = NULL;

				if (tmp2 != NULL) {
	                                freeArgs(tmp2);
					tmp2 = NULL;
				}
                                return "";
                        }

                        memcpy(value, tmp, (size_t)(html-tmp));
                        html++;
                        tmp2 = addArgToTable(tmp2, name, value);
                } else if (*html == '\'') {
                        html++;
                        tmp = html;
                        while(*html != '\0' && !(*html == '\'' && *(html-1) != '\\')) html++;

                        value =  (char *)calloc(1, (size_t)(html-tmp+1));
                        if (!value) {
                                free(name);
				name = NULL;
                                free(tag);
				tag = NULL;

				if (tmp2 != NULL) {
	                                freeArgs(tmp2);
					tmp2 = NULL;
				}
                                return "";
                        }

                        memcpy(value, tmp, (size_t)(html-tmp));
                        html++;
                        tmp2 = addArgToTable(tmp2, name, value);
		}

                tmp = NULL;
		value = NULL;
		name = NULL;
        }

        //if (*html != '\0') html++; // for some reason, if a page does <?xml ...?><!DOCTYPE ...> then with this "DOCTYPE ...>" is printed...

        ret = (*xmlCallBack)(tag, tmp2, numArgs);

	if (tmp2 != NULL) {
		freeArgs(tmp2);
		tmp2 = NULL;
        }

	free(tag);
	tag = NULL;
        numArgsStatus=0;

        return (ret != 0 ? "" : html);

}

#ifdef _INLINE_CODE
inline
#endif

/* cannibalistic function, munches the actuall tag */

const char *eatUp(const char *html){
  while ( (*html != '>') &&
	  (*html != '\0') ) {
    html++;
  }
  if (*html != '\0') 
    html++;
  return html;
}

#ifdef _INLINE_CODE
inline
#endif

/* cannibalistic function, munches the actuall text */

const char *eatUpText(const char *html){

	while(*html != '\0' && *html != '<'){
		html++;
	}

	return html;
}


#ifdef _INLINE_CODE
inline
#endif

/* decides, if a found '?' leads to PHP or XML if requisited
   otherwise it gormandizes them up. *burps* */

const char *parseXML(const char *html) {

	/* conditional expressions inside a conditional expression
	   don't try _this_ at home kids! ;-) */

	 html=(((tolower((int)(*(html+1))))==(int)('p')) ?
	( (phpCallBack) ? parsePHP   (html) :  eatUp(html) ) :
	( (xmlCallBack) ? parseXMLtag(html) :  eatUp(html) )   );

	 return html;
}


#ifdef _INLINE_CODE
inline
#endif
const char *parseStartTag (const char *html) {
	char *tag, *name, *value;
	const char *tmp;
	int ret = 0;
	struct ArgvTable *tmp2 = NULL;

	numArgs = 0;
	tmp = html;
	while(*html != '\0' && !isspace((int)*html) && 
			*html != '>' && *html != '/') html++;

	tag = (char *)calloc(1, (size_t)(html-tmp+1));
	if (!tag) {
		return "";
	}

	memcpy(tag, tmp, (size_t)(html-tmp));

	/*if (strncasecmp("script", tag, 6) == 0) {
		lhtml_script_passthru = 1;
	}
	else if (strncasecmp("pre", tag, 3) == 0) {
		lhtml_script_passthru = 2;
	}*/

	if (*html == '>') {
		if (startCallBack) {
			ret = (*startCallBack)(tag, NULL, 0);
			free(tag);
			tag = NULL;

			/* this check is redundant */
			/* if (*html == '>') */ html++;
			return((ret != 0) ? "" : html);
		}
	}
	else if (*html == '/' ) {   /* XHTML empty tag like <hr/>, <br/>*/

		/**********************************************
		* You may choose now between two behaviors    *
		* of libhtmlparse to handle XHTML empty tags: *
		* a) call XHTMLCallBack                       *
		* b) call start- AND endCallBack              *
		***********************************************/


		if (startCallBack != NULL && !(XHTMLCallBack)) {
			ret = (*startCallBack)(tag, NULL, 0);
		}
		if (endCallBack != NULL && ret==0 && !(XHTMLCallBack)) {
			ret = (*endCallBack)(tag);
		}
		if(XHTMLCallBack){
			ret = (*XHTMLCallBack)(tag, NULL, 0);
		}

		free(tag);
		tag = NULL;

		html += 2;
		return((ret != 0) ? "" : html);
	}

	while(*html != '\0' && isspace((int)*html)) html++;
	
	while(*html != '\0' && *html != '>' ) {
		while(isspace((int)*html)) html++;
		if (*html == '>') break;

		if (*html == '/' && *(html+1) == '>') {
			html++; break;
		}

		tmp = html;
		while(*html != '\0' && !isspace((int)*html) && 
				*html != '=' && *html != '>') html++;
		name = (char *)calloc(1, (size_t)(html-tmp+1));
		if (!name) {
			free(tag);
			tag = NULL;
			return "";
		}

		memcpy(name, tmp, (size_t)(html-tmp));

		if (isspace((int)*html)) {
			char *x = (char*)html; // char *x = (char*)html; // If I don't comment this 'change/fix' out then I get problems
			while(*x != '\0' && *x != '>' && *x != '=') x++;
			if (*x == '=') {
				html = x;
				goto namevalue;
			}

			tmp2 = addArgToTable(tmp2, name, NULL);
			while(*html != '\0' && isspace((int)*html) && 
					*html != '>' && 
					!(*html == '/' && *(html+1) == '>')) 
				html++;
		} else {

			if (*html == '/') {
				html++;
				break;
			}

			/* html++ is repeated after the while loop
			 * and may cause deletion of important info */
			if (*html == '>') {
				tmp2 = addArgToTable(tmp2, name, NULL);
				/*html++;*/
				break; 
			}

namevalue:
			if (*html == '=') html++;

			while(isspace(*html)) html++;

			if (*html != '"' && *html != '\'') {
				tmp = html;
				while(*html != '\0' && *html != '>' && 
						!isspace((int)*html) && 
						!(*html == '/' && *(html+1) == '>')) 
					html++;
				value = (char *)calloc(1, (size_t)(html-tmp+1));
				if (value == NULL) {
					free(name);
					name = NULL;
					free(tag);
					tag = NULL;
	
					freeArgs(tmp2);
					return "";
				}
	
				memcpy(value, tmp, (size_t)(html-tmp));
				tmp2 = addArgToTable(tmp2, name, value);
			} else if (*html == '"') {
				html++;
				tmp = html;
				while(*html != '\0' && 
						!(*html == '"' && *(html-1) != '\\')) 
					html++;
				value = (char *) calloc(1, (size_t)(html-tmp+1));
				if (value == NULL) {
					free(name);
					name = NULL;
					free(tag);
					tag = NULL;
	
					freeArgs(tmp2);
					return "";
				}
	
				memcpy(value, tmp, (size_t)(html-tmp));
				html++;
				tmp2 = addArgToTable(tmp2, name, value);
			} else if (*html == '\'') {
				html++;
				tmp = html;
				while(*html != '\0' && !(*html == '\'' && 
							*(html-1) != '\\')) html++;
	
				value = (char *)calloc(1, (size_t)(html-tmp+1));
				if (value == NULL) {
					free(name);
					name = NULL;
					free(tag);
					tag = NULL;

                                	freeArgs(tmp2);
                                	return "";
                        	}
	
				memcpy(value, tmp, (size_t)(html-tmp));
				html++;
				tmp2 = addArgToTable(tmp2, name, value);
			}
			tmp = NULL;
        	}
	}
	if (*html != '\0') html++;

	if (startCallBack != NULL &&  *(html-2)!='/' ) {
		ret = (*startCallBack)(tag, tmp2, numArgs);
	}
	if (endCallBack != NULL && ret==0 && *(html-2)=='/' 
			&& !(XHTMLCallBack)) {
		ret = (*endCallBack)(tag);
	}
	/* these tags may have arguments too, e.g. <hr noshade/> */
	if (XHTMLCallBack != NULL && *(html-2)=='/') {
		ret = (*XHTMLCallBack)(tag, tmp2, numArgs);
	}
	if(tmp2 != NULL){
		freeArgs(tmp2);
	}
	free(tag);
	tag = NULL;

	numArgsStatus=0;

	/* this is a bad hack, feel free to write a better one (maybe a more readable one? ;-)*/
	return
	(XHTMLCallBack != NULL) ?
	(html) :
	((ret != 0) ? "" : html);
}


#ifdef _INLINE_CODE
inline
#endif
const char *parseDecl(const char *html) {
        char *tag, *name, *value;
        const char *tmp;
        register int ret=0;
        struct ArgvTable *tmp2 = NULL;

        numArgs = 0;
        tmp = html;
        while(*html != '\0' && !isspace((int)*html) && *html != '>') html++;

        tag = (char *)calloc(1, (size_t)(html-tmp+1));
        if (!tag) {
                return "";
        }

        memcpy(tag, tmp, (size_t)(html-tmp));

        if (*html == '>') {
                if (declCallBack) {
                        ret = (*declCallBack)(tag, NULL, 0);
                        free(tag);
			tag = NULL;

                        if (*html == '>') html++;
                	return((ret != 0) ? "" : html);
                }
        }

        while(*html != '\0' && isspace((int)*html)) html++;

        while(*html != '\0' && *html != '>') {
                while(isspace((int)*html)) html++;
                if (*html == '>') break;

                tmp = html;

                switch(*tmp){

                case '\'' :   	html++;
	                	tmp = html;
				while(*html != '\0' && !(*html == '\'' && *html != '\\')) html++;
			        break;
		case '"'  :     html++;
	                	tmp = html;
				while(*html != '\0' && !(*html == '"' && *html != '\\')) html++;
                                break;
		default  :
       	        		while(*html != '\0' && !isspace((int)*html) && *html != '=' && *html != '>') html++;
				break;
		}

                name = (char *) calloc(1, (size_t)(html-tmp+1));
                if (!name) {
                        free(tag);
			tag = NULL;
                        return "";
                }

                memcpy(name, tmp, (size_t)(html-tmp));

                if (isspace((int)*html)) {
                        tmp2 = addArgToTable(tmp2, name, NULL);
                        while(*html != '\0' && isspace((int)*html) && *html != '>') html++;
			continue;
                }

                if (*html == '>') {
                        tmp2 = addArgToTable(tmp2, name, NULL);
                        html++;
                        break;
                }

		if (*(html+1) == '>') {
			tmp2 = addArgToTable(tmp2, name, NULL);
			html += 2;
			break;
		}

                if (*html == '=') html++;


		switch(*html){

		case '\''  :    html++;
                        	tmp = html;
                        	while(*html != '\0' && !(*html == '\'' && *(html-1) != '\\')) html++;

                        	value = (char *) calloc(1, (size_t)(html-tmp+1));
                        	if (!value) {
                                	free(name);
					name = NULL;
                                	free(tag);
					tag = NULL;

                                	freeArgs(tmp2);
                                	return "";
                        	}

                        	memcpy(value, tmp, (size_t)(html-tmp));
                        	html++;
                        	tmp2 = addArgToTable(tmp2, name, value);
                                break;
		case '"'  :     html++;
                        	tmp = html;
                        	while(*html != '\0' && !(*html == '"' && *(html-1) != '\\')) html++;
                        	value =  (char *)calloc(1, (size_t)(html-tmp+1));
                        	if (!value) {
                        	        free(name);
					name = NULL;
                        	        free(tag);
					tag = NULL;

                        	        freeArgs(tmp2);
                        	        return "";
                        	}

                        	memcpy(value, tmp, (size_t)(html-tmp));
                        	html++;
                        	tmp2 = addArgToTable(tmp2, name, value);
                                break;
		default  :      html++;
                        	tmp = html;
                        	while(*html != '\0' && *html != '>' && !isspace((int)*html)) html++;
                        	value = (char *) calloc(1, (size_t)(html-tmp+1));
                        	if (!value) {
                        	        free(name);
					name = NULL;
                        	        free(tag);
					tag = NULL;

                        	        freeArgs(tmp2);
                        	        return "";
                        	}

                        	memcpy(value, tmp, (size_t)(html-tmp));
                        	tmp2 = addArgToTable(tmp2, name, value);
			        break;
		}
                tmp = NULL;
        }

        if (*html != '\0') html++;

        if (declCallBack) {
                ret = (*declCallBack)(tag, tmp2, numArgs);
                freeArgs(tmp2);
		free(tag);
		tag = NULL;

                return((ret != 0) ? "" : html);
        }

        freeArgs(tmp2);
        numArgsStatus=0;

        return html;
}


#ifdef _INLINE_CODE
inline
#endif
const char *parseForEntities (const char *tmp){
	char *entity, *text ;
	const char *tmp1, *tmp2;
	register int ret=0, count=0;

	while(*tmp != '\0'){
		tmp1 = tmp;
		while(*tmp != '\0' && *tmp != '&')tmp++;

		text = (char *)calloc(1, (size_t)(tmp-tmp1+1));
		if (text == NULL) {
			return "";
		}

		memcpy(text, tmp1, (size_t)(tmp-tmp1));
		/* the chunk of text before the first entity will
		   not be called, if it starts with an entity

		   DCK - not anymore */
		if(strlen(text)>0 /*&& (!(isspace((int)*text)))*/){
			if (textCallBack) {
				ret = (*textCallBack)(text);
			}
			free(text);
			text = NULL;

			tmp1 = "";
		}

		if(*tmp == '&'){
			tmp++;
			tmp2=tmp;
			/* sometimes the ';' is absent, it's a bad hack, just to avoid more trouble */
			while( *tmp != '\0' && (*tmp != ';' && count != 9) ){
				tmp++;
				count++;
			}
			entity = (char *)calloc(1, (size_t)(tmp-tmp2+1));
        		if (!entity) {
				return "";
        		} else {
				memcpy(entity, tmp2, (size_t)(tmp-tmp2));
				if(*tmp == ';' || count == 9){  /* should I add an errortrap here? */
					ret = (*entityCallBack)(entity);
					free(entity);
					entity = NULL;
					tmp2 = "";
					count = 0;
				}
			}
		}
		if (*tmp != '\0') tmp++;
	}
	return tmp;
}

struct ErrorTree *parse (const char *html) {

        while(*html != '\0') {
              /* while(isspace(*html)){html++;} there may be leading blanks in some autogenerated files
						 add this or not, that is the question ;-)) */

		if (lhtml_script_passthru != 0) {
			const char *text;
			char *tmp;

			text = html;
			if(lhtml_script_passthru == 1 ){ 

			while(*text != '\0') {
				if (*text == '<') {
					if (*(text+2) == 's' || *(text+2) == 'S') {
						if (*(text+7) == 't' || *(text+7) == 'T') {
							break;
						}
					}
				}
				if(*text != '\0') text++;
			} }

			if(lhtml_script_passthru == 2 ){
				while(*text != '\0') {
					if (*text == '<') {
						if (*(text+2) == 'p' || *(text+2) == 'P') {
							if (*(text+4) == 'e' || *(text+4) == 'E') {
								break;
							}
						}
					}
					if(*text != '\0') text++;
				}
			}

			tmp = (char *) malloc((size_t)(text-html+1));
			ErrFatalDisplayIf(!tmp, "Memory Error");

			strncpy(tmp, html, (size_t)(text-html));

			if (textCallBack != NULL) {
				int ret = (*textCallBack)(tmp);
				if (ret != 0) {
					free(tmp);
					tmp = NULL;

					return NULL;
				}
			}

			free(tmp);
			tmp = NULL;

			lhtml_script_passthru = 0;
			html = text;
		}

                if(*html == '<'){
                        html++;

			switch (*html){
                        case '!'   :
                                html++;
				/* I must admit, I like conditional expressions,
				   they are so obviously obfuscated ;-)          */

				html = (*html == '-') ?
					((commentCallBack) ? parseComment(html) : eatUp(html)) :
					((declCallBack)    ? parseDecl(html)    : eatUp(html))  ;
					break;
                        case '?'  : 			/* XML/PHP tag */
                                	html = (xmlCallBack != NULL || phpCallBack != NULL) ?
						parseXML(html) :
						eatUp(html);
						break;
                        case '/'  : 			/* HTML end tag */
                                	html = (endCallBack) ?
						parseEndTag(html) :
						eatUp(html);
						break;
                        default  : 			/* HTML start tag */
                                	html = (XHTMLCallBack != NULL || startCallBack != NULL) ?
						parseStartTag(html) :
						eatUp(html);
						break;
                        }
		      }
                else{				 /* All other text */
                        /* while(isspace(*html))html++;   it seems to be faster inside the function */
                        	html = (textCallBack)  ?
					parseText(html):
					eatUpText(html);
                }
        }
        return NULL;
}

