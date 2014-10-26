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
#include	<PalmOS.h>
#include 	"WAPUniverse.h"
#include	"build.h"
#include 	"../res/WAPUniverse_res.h"
#include	"PalmUtils.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"variable.h"

void setVar(Char *name, Char *value, GlobalsType *g)
{
  VarPtr tmp;
  
  	tmp= g->Var;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			if(tmp->value)
				Free(tmp->value);
			tmp->value = Malloc(StrLen(value)+1);
			ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
			StrCopy(tmp->value,value);		
			return;			
		}
		tmp = tmp->next;
	}
}

void setMultiVar(Char *name, Char *value, GlobalsType *g)
{
  VarPtr tmp;
  Char *tmpVal;
  
  	tmp= g->Var;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			if (!tmp->value) setVar(name, value, g);

			if (StrStr(tmp->value, value)) return;

			if ((StrLen(tmp->value) > 0) && (value != NULL)) {
				tmpVal = Malloc(StrLen(tmp->value)+1);
				ErrFatalDisplayIf(!tmpVal, "Malloc failed");
				StrCopy(tmpVal, tmp->value);
				if(tmp->value)
					Free(tmp->value);
				tmp->value = Malloc(StrLen(value)+StrLen(tmpVal)+2);
				ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
				StrCopy(tmp->value, tmpVal);
				StrCopy(tmp->value + StrLen(tmp->value), "\x10"); // StrCopy(tmp->value + StrLen(tmp->value), ";");
				StrCopy(tmp->value + StrLen(tmp->value), value);

				Free(tmpVal);
			} else {
				tmp->value = Malloc(StrLen(value)+1);
				ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
				StrCopy(tmp->value,value);
			}
			return;			
		}
		tmp = tmp->next;
	}
}

void removeMultiVar(Char *name, Char *value, GlobalsType *g)
{
  VarPtr tmp;
  Char *tmpVal;
  Int32 pos = 0, len = 0, valLoc = 0;
  
  	tmp= g->Var;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0) //this could probably be done differently
		{				   
	search:
			while ((tmp->value[valLoc] != value[0]) && (tmp->value[valLoc] != '\0')) valLoc++;
			if (tmp->value[valLoc] == '\0') return;

			if (StrNCompare(tmp->value + valLoc, value, StrLen(value))) goto search;

			tmpVal = Malloc(StrLen(tmp->value)+1);
			ErrFatalDisplayIf(!tmpVal, "Malloc failed");
			StrCopy(tmpVal, tmp->value);

			len = StrLen(tmpVal);

			if(tmp->value)
				Free(tmp->value);

			if (StrLen(value) == len) { tmp->value = NULL; goto cleanup; }

			tmp->value = Malloc(StrLen(tmpVal)-StrLen(value)+1);
			ErrFatalDisplayIf (!tmp->value, "Malloc Failed");

			if (valLoc == 0) {
				StrCopy(tmp->value, tmpVal + StrLen(value) + 1);
				tmp->value[len - StrLen(value) - 1] = '\0';
			} else {
				StrCopy(tmp->value, tmpVal);
				pos = valLoc;
				while ((tmpVal[pos] != MULTI_VAR_IND) && (tmpVal[pos] != '\0')) pos++;

				if (tmpVal[pos] == MULTI_VAR_IND) pos++;

				StrCopy(tmp->value + valLoc, tmpVal + pos);

				if (tmp->value[StrLen(tmp->value) - 1] == MULTI_VAR_IND)
					tmp->value[StrLen(tmp->value) - 1] = '\0';
				else
					tmp->value[pos++] = '\0';
			}

	cleanup:
			Free(tmpVal);

			return;			
		}
		tmp = tmp->next;
	}
}

Boolean compareMultiVar(Char *name, Char *value, GlobalsType *g)
{
  VarPtr tmp;
  Char *valLoc;

  	tmp= g->Var;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			if (!tmp->value) return false;

			valLoc = StrStr(tmp->value, value);

			return (valLoc) ? true : false;			
		}
		tmp = tmp->next;
	}

	return false;
}

Char *disectMultiVar(Char *value, Int16 *i)
{
	Char *val;
	Int16 j = 0;

	val = Malloc(StrLen(value)+1);
		if (!val) return NULL;

	for (j = 0; *i < StrLen(value) && value[*i] != '\0' && value[*i] != MULTI_VAR_IND; j++, (*i)++) {
		val[j] = value[*i];
	}

	if (StrLen(val) == 0) {
		Free(val);
		return NULL;
	}

	val[j++] = '\0';
	if (value[*i] == MULTI_VAR_IND) (*i)++;

	return val;
}

Boolean checkVar(Char *name, GlobalsType *g)
{
  VarPtr tmp;
  
  	tmp= g->Var;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			return true;	
		}
		tmp = tmp->next;
	}
	
  return false;
}

int addVar(Char *name, GlobalsType *g)
{
  VarPtr tmp, var;
	
	var= g->Var;
	while(var != NULL)
	{
		
		if (StrCompare(name,var->name)==0)
		{
			return(0); // We found a matching name, do not create a new....
		}
		var = var->next;
	}

	tmp = Malloc (sizeof(VarType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	tmp->value=NULL;
	tmp->name = Malloc(StrLen(name)+1);
	ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
	StrCopy(tmp->name,name);
	tmp->next = g->Var;
    	g->Var = tmp;
			
return(0);
}

Char *getVar(Char *name, GlobalsType *g)
{
  VarPtr tmp;
  
  	tmp= g->Var;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			if(tmp->value)
				return(tmp->value);
			else
				return("");
		}
		tmp = tmp->next;
	}
    return("");
}

unsigned char *expandVar(unsigned char *instr, GlobalsType *g)
{
	int 		i,start=0,exttype=0,newlen,len;
	unsigned char 	*name,*value,*str,*newstr,c;
	Boolean		val = false;

  	len = StrLen(instr);
  	str = Malloc(len+1);	   
  	if (!str)
		return instr; //ErrFatalDisplayIf (!str, "Malloc expandVar");
  	StrCopy(str,instr);
  	i = 0;
  	while(i<len) {
    	c = str[i];
  		if ((c==EXT_T_0) || (c==EXT_T_1) || (c==EXT_T_2) || (c==EXT_I_0) || (c==EXT_I_1) || (c==EXT_I_2)) {
	  		exttype= c; 
	  		start = i+1;
		}
		if (c==END) {
	   		name=&(str[start]);
	   		str[i]=0;
	   		value = getVar(name,g);

	   		if((exttype==EXT_T_0) || (exttype==EXT_I_0)) {
	     			value = escaped(value); // buggy
				val = true;
			}
			if((exttype==EXT_T_1) || (exttype==EXT_I_1)) {
	     			value = unescaped(value);
				val = true;
			}

			newlen = len - (StrLen(name)+2) + StrLen(value) + 1;
	   		newstr = Malloc(newlen);	   
	   		ErrFatalDisplayIf (!newstr, "Malloc expandVar");
	   		StrNCopy(newstr,str,start-1);
	   		StrNCopy(newstr+start-1,value,StrLen(value));
	   		StrNCopy(newstr+start-1+StrLen(value),&(str[i+1]),len-i);
	   		newstr[newlen-1]=0;	
       			Free(str);
	   		str = newstr;
  	   		len = StrLen(str);
	   		i=0;

			if (((exttype==EXT_T_0) || (exttype==EXT_I_0) || (exttype==EXT_T_1) || (exttype==EXT_I_1)) && (val == true))
				Free(value);
		} else {
	   		i++;
		}
  	}
  	return(str);
}

//This is a hack because, for some reason, the code above crashes at "Free(str);"
/*unsigned char *expandVar(unsigned char *instr, GlobalsType *g)
{
    Char *buf, *ptr = NULL;
    Char *vinstr = (Char*)instr;
    UInt32 j;
    Int16 i, c, exttype = 0;
    Int16 len = StrLen(instr);
    //Boolean escape = true;

    buf = Malloc(4096);
    ErrFatalDisplayIf(!buf, "Malloc failed [expandVar]");


    for (j = i = 0; i < len && j < 4096; i++)
    {
        c = (UInt8)vinstr[i];
        if (ptr)
        {
            if (c == END)
            {
                vinstr[i] = 0;
                ptr = getVar(ptr,g);
                vinstr[i] = c;

                if(ptr != NULL)
                {
                    if ((exttype == EXT_T_0) || (exttype == EXT_I_0))
                        ptr = escaped(ptr);
                    else if ((exttype == EXT_T_1) || (exttype == EXT_I_1))
                        ptr = unescaped(ptr);

                    while(*ptr && j < 4096 - 1) // for possible back-slash
                    {
                        //if(escape && (*ptr == '\'' || *ptr == '\"'))
                        //    buf[j++] = '\\';
                        buf[j++] = *ptr++;
                    }
                }
                
                ptr = NULL;
            }
        }
        else
        {
            if((c == EXT_T_0) || (c == EXT_T_1) || (c == EXT_T_2) || (c == EXT_I_0) || (c == EXT_I_1) || (c == EXT_I_2)) // || ((c == '?') && ((g->wmldck.publicID == 0x08) || (g->wmldck.publicID == 0x0D))))
            {
                exttype = c;
		//if (c == '?') exttype = EXT_T_2; // for some reason this code does not like EXT_T_*
		//else exttype = c;

                ptr = vinstr + i + 1;

		{ // read ahead to make sure we really have a var
			int a = i+1;
			Boolean isVar = false;
			Boolean cont = true;
			
			while (cont == true) {
				//if ((instr[a] == EXT_T_0) || (instr[a] == EXT_T_1) || (instr[a] == EXT_T_2) || 
				//    (instr[a] == EXT_I_0) || (instr[a] == EXT_I_1) || (instr[a] == EXT_I_2) || 
				 //   (instr[a] == '?') || (instr[a] == '\0')) {
				if (instr[a] == '\0') {
					cont = false;
					isVar = false;
				} else if (instr[a] == END) {
					cont = false;
					isVar = true;
				}
				a++;
			}
			if (isVar == false) { ptr = NULL; buf[j++] = c; }
		}
            }
            else
                buf[j++] = c;
        }
    }

    buf[j++] = 0;
    MemPtrResize(buf, j);

    return buf;
}*/

int disposeVar(GlobalsType *g)
{
  VarPtr tmp;

	while(g->Var != NULL){
		tmp = g->Var;
		g->Var = g->Var->next;
		if(tmp->name)
			Free(tmp->name);
		if(tmp->value)
			Free(tmp->value);
		Free(tmp);
	}		
  	return(0);
}

Char *InputgetVar(Char *name, GlobalsType *g)
{
  VarPtr tmp;
  
  	tmp= g->Var;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			if(tmp->value)
				return(tmp->value);
			else
				return(NULL);
		}
		tmp = tmp->next;
	}
    return(NULL);
}

int optionAddVar(Char *name, GlobalsType *g)
{
  	VarPtr 		tmp, var;	
	OptionPtr	opt;
	
	opt = g->Select->options;
	if(opt != NULL){
		while(opt->next != NULL){
			opt = opt->next;
		}

		var = opt->optVar;
		while(var != NULL) {
		
			if (StrCompare(name,var->name)==0) {
				return(0); // We found a matching name, do not create a new....
			}
			var = var->next;
		}

		tmp = Malloc (sizeof(VarType));
		ErrFatalDisplayIf (!tmp, "Malloc Failed");
		tmp->value = NULL;
		tmp->name = Malloc(StrLen(name)+1);
		ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
		StrCopy(tmp->name,name);
		tmp->next = opt->optVar;//var;//g->Select->options->optVar;
    		opt->optVar = tmp;//var = tmp;//g->Select->options->optVar = tmp;
	}
			
	return(0);
}

Char *optionGetVar(Char *name, GlobalsType *g)
{
  	VarPtr 		tmp;
	OptionPtr	opt;

	opt = g->Select->options;
	if(opt != NULL){
		while(opt->next != NULL){
			opt = opt->next;
		}
  
  		tmp = opt->optVar;
		while(tmp != NULL) {
		
			if (StrCompare(name,tmp->name)==0) {
				if(tmp->value)
					return(tmp->value);
				else
					return("");
			}
			tmp = tmp->next;
		}
	}
    	return("");
}

void optionSetVar(Char *name, Char *value, GlobalsType *g)
{
  	VarPtr 		tmp;
	OptionPtr	opt;

	opt = g->Select->options;
	if(opt != NULL){
		while(opt->next != NULL){
			opt = opt->next;
		}

  		tmp = opt->optVar;
		while(tmp != NULL) {
		
			if (StrCompare(name,tmp->name)==0) {
				if(tmp->value)
					Free(tmp->value);
				tmp->value = Malloc(StrLen(value)+1);
				ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
				StrCopy(tmp->value,value);			
				return;			
			}
			tmp = tmp->next;
		}
	}
}

int optionAddPostVar(Char *name, GlobalsType *g)
{
  	VarPtr 		tmp, var;
	OptionPtr	opt;

	opt = g->Select->options;
	if(opt != NULL){
		while(opt->next != NULL){
			opt = opt->next;
		}
	
		var = opt->postVar;
		while(var != NULL) {
		
			if (StrCompare(name,var->name)==0) {
				return(0); // We found a matching name, do not create a new....
			}
			var = var->next;
		}

		tmp = Malloc (sizeof(VarType));
		ErrFatalDisplayIf (!tmp, "Malloc Failed");
		tmp->value = NULL;
		tmp->name = Malloc(StrLen(name)+1);
		ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
		StrCopy(tmp->name,name);
		tmp->next = opt->postVar;//var;//g->Select->options->postVar;
    		opt->postVar = tmp;//var = tmp;//g->Select->options->postVar = tmp;
	}
			
	return(0);
}

Char *optionGetPostVar(Char *name, GlobalsType *g)
{
  	VarPtr 		tmp;
	OptionPtr 	opt;

	opt = g->Select->options;
	if(opt != NULL){
		while(opt->next != NULL){
			opt = opt->next;
		}
  
  		tmp = opt->postVar;
		while(tmp != NULL) {
		
			if (StrCompare(name,tmp->name)==0) {
				if(tmp->value)
					return(tmp->value);
				else
					return("");
			}
			tmp = tmp->next;
		}
	}
   	return("");
}

void optionSetPostVar(Char *name, Char *value, GlobalsType *g)
{
  	VarPtr 		tmp;
	OptionPtr 	opt;

	opt = g->Select->options;
	if(opt != NULL){
		while(opt->next != NULL){
			opt = opt->next;
		}

		tmp = opt->postVar;
		while(tmp != NULL) {
		
			if (StrCompare(name,tmp->name)==0) {
				if(tmp->value)
					Free(tmp->value);
				tmp->value = Malloc(StrLen(value)+1);
				ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
				StrCopy(tmp->value,value);	
				return;			
			}
			tmp = tmp->next;
		}
	}
}

int hyperlinkAddVar(Char *name, GlobalsType *g)
{
  	VarPtr tmp, var;
	
	var= g->HyperLinks->linkVar;
	while(var != NULL)
	{
		
		if (StrCompare(name,var->name)==0)
		{
			return(0); // We found a matching name, do not create a new....
		}
		var = var->next;
	}

	tmp = Malloc (sizeof(VarType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	tmp->value=NULL;
	tmp->name = Malloc(StrLen(name)+1);
	ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
	StrCopy(tmp->name,name);
	tmp->next = g->HyperLinks->linkVar;
    	g->HyperLinks->linkVar = tmp;
			
return(0);
}

Char *hyperlinkGetVar(Char *name, GlobalsType *g)
{
  VarPtr tmp;
  
  	tmp= g->HyperLinks->linkVar;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			if(tmp->value)
				return(tmp->value);
			else
				return("");
		}
		tmp = tmp->next;
	}
    return("");
}

void hyperlinkSetVar(Char *name, Char *value, GlobalsType *g)
{
  VarPtr tmp;
  
  	tmp= g->HyperLinks->linkVar;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			if(tmp->value)
				Free(tmp->value);
			tmp->value = Malloc(StrLen(value)+1);
			ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
			StrCopy(tmp->value,value);				
			return;			
		}
		tmp = tmp->next;
	}
}


int hyperlinkAddPostVar(Char *name, GlobalsType *g)
{
  VarPtr tmp, var;
	
	var= g->HyperLinks->postVar;
	while(var != NULL)
	{
		
		if (StrCompare(name,var->name)==0)
		{
			return(0); // We found a matching name, do not create a new....
		}
		var = var->next;
	}

	tmp = Malloc (sizeof(VarType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	tmp->value=NULL;
	tmp->name = Malloc(StrLen(name)+1);
	ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
	StrCopy(tmp->name,name);
	tmp->next = g->HyperLinks->postVar;
    	g->HyperLinks->postVar = tmp;
			
return(0);
}

Char *hyperlinkGetPostVar(Char *name, GlobalsType *g)
{
  VarPtr tmp;
  
  	tmp= g->HyperLinks->postVar;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			if(tmp->value)
				return(tmp->value);
			else
				return("");
		}
		tmp = tmp->next;
	}
    return("");
}

void hyperlinkSetPostVar(Char *name, Char *value, GlobalsType *g)
{
  VarPtr tmp;
  
  	tmp= g->HyperLinks->postVar;
	while(tmp != NULL)
	{
		
		if (StrCompare(name,tmp->name)==0)
		{
			if(tmp->value)
				Free(tmp->value);
			tmp->value = Malloc(StrLen(value)+1);
			ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
			StrCopy(tmp->value,value);				
			return;			
		}
		tmp = tmp->next;
	}
}

Char *convertToVarString(Char *input)
{
	Char *output;
	Int32 i = 0, j = 0, saved = 0;
	Boolean haveVar = false, haveType = false, stop = false;
	//int type = 0; // 0: No type; 1: esc; 2: noesc; 3: unesc

	output = Malloc((StrLen(input) * 2) + 1);
	//if (!output) return input;
	ErrFatalDisplayIf(!output, "Malloc failed [convertToVarString]");

	while (/*input[i] != '\0'*/ stop != true) {
		if (input[i] == '$' && input[i+1] != '$' && input[i+1] != ' ' && haveVar == false) {
			if (input[i+1] == '(') i+=2;
			else i++;
			output[j] = EXT_I_2;
			saved = j;
			haveVar = true;
			j++;
		} else if (input[i] == '$' && input[i+1] == '$') {
			output[j] = input[i++];
			j++; i++;
		} else {
			if (haveType == true && input[i] != ' ' && input[i] != ')') {
				i++;
			} else if (haveVar == true && input[i] == ':') {
				haveType = true;
				if (input[i+1] == 'e' || input[i+1] == 'E')
					output[saved] = EXT_I_0;
				else if (input[i+1] == 'u' || input[i+1] == 'U')
					output[saved] = EXT_I_1;
				else if (input[i+1] == 'n' || input[i+1] == 'N')
					output[saved] = EXT_I_2;
			} else if (haveVar == true && 
					(input[i] == ' ' || input[i] == ')' || input[i] == '$' ||
					(input[i] <= 0x30 || (input[i] >= 0x39 && input[i] <= 0x41) || input[i]  >= 0x7A))) {
				output[j] = END;
				haveVar = false;
				haveType = false;
				if (input[i] == ')')
					i++;
				j++;
			} else if (haveVar == true) {
				output[j] = input[i];
				i++; j++;
			} else if (input[i] == '\0') {
				if (haveVar == true) {
					output[j] = END;
					haveVar = false;
					haveType = false;
				}
				stop = true;
			} else {
				output[j] = input[i];
				i++; j++;
			}
		}
	}
	output[j++] = '\0';

	MemPtrResize(output, j); // usually shrinking

	return output;
}


