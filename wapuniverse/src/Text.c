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
#include	"WAPUniverse.h"
#include	"DOM.h"
#include	"Browser.h"
#include	"PalmUtils.h"
#include	"Font.h"

void TextAddText(Char *text, UInt16 length, Int16 x, Int16 y,
		 UInt16 width, UInt16 height, GlobalsType *g)
{
	TextPtr		tmp, idx;
	UInt16		yChange = 0;

	tmp = Malloc(sizeof(TextType));
	if (!tmp) return;

	tmp->font 		= g->font->font;
	tmp->res 		= g->font->res;
	tmp->italic 		= g->font->italic;
	tmp->underlined 	= g->font->underlined;
	tmp->superScript 	= g->font->superScript;
	tmp->subScript 		= g->font->subScript;
	tmp->strike 		= g->font->strike;
	tmp->color 		= g->font->color;
	tmp->link 		= g->currentLink;
	tmp->direction 		= directionLTR;
	tmp->text 		= Malloc(length + 1);
	if (tmp->text) {
		MemSet(tmp->text, length + 1, 0);
		StrNCopy(tmp->text, text, length);
	}
	tmp->next = NULL;

	if (!g->text) {
		g->text = tmp;
	} else {
		idx = g->text;
		while (idx->next != NULL) {
		   	idx = idx->next;
		}
		idx->next = tmp;
	}

	if (g->font && g->font->superScript) {
		yChange = -1;
	}

	if (g->font && g->font->subScript) {
		yChange = FontCharHeight() - (4 * HiresFactor());
	}

	DOMAddElement(tmp, DOM_UI_TEXT, x, y + yChange, width, height, g);

	//tmp->next = g->text;
	//g->text = tmp;

	return;
}

Boolean TextSetDirectionRTL(GlobalsType *g)
{
	TextPtr	text = g->text;

	while (text->next)
		text = text->next;

	return (text->direction = directionRTL);
}

Boolean TextSetDirectionLTR(GlobalsType *g)
{
	TextPtr	text = g->text;

	while (text->next)
		text = text->next;

	return (text->direction = directionLTR);
}

void TextFreeText(GlobalsType *g)
{
	TextPtr	tmp;

	while (g->text != NULL){
		tmp = g->text;
		g->text = tmp->next;

		tmp->font = 0;
		tmp->res = 0;
		tmp->italic = false;
		tmp->underlined = false;
		tmp->superScript = false;
		tmp->subScript = false;
		tmp->strike = false;
		tmp->color = false;
		tmp->link = NULL;
		if (tmp->text) {
			Free(tmp->text);
			tmp->text = NULL;
		}

		Free(tmp);
	}
	g->text = NULL;
}