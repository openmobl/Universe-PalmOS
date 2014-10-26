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
#include	"xhtml.h"
#include	"../res/WAPUniverse_res.h"
#include	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"variable.h"
#include	"history.h"
#include	"Form.h"
#include	"PalmUtils.h"
#include	"Entity.h"
#include	"http.h"
#include	"color.h"
#include	"process.h"
#include	"image.h"
#include	"Layout.h"
#include	"Cache.h"
#include	"URL.h"
#include	"DOM.h"
#include	"Text.h"
#include	"Area.h"
#include	"Browser.h"
#include	"debug.h"
#include	"Hyperlinks.h"

#include	"xhtml_string.h"
#include	"xhtml_access.h"
#include	"xhtml_tag_misc.h"
#include	"xhtml_end.h"

#include 	"libhtmlparse.h"

#ifdef __palmos__
#define underline 		palmUnderline
#define bold 			palmBold
#define strong 			palmStrong
#define big 			palmBig
#define small 			palmSmall
#define italic 			palmItalic
#define em 			palmEmphasis
#define pre 			palmPre
#define regular			palmRegular
#define	superscript		palmSuperscript
#define subscript		palmSubscript
#define strike			palmStrike
#define wmlInput 		palmWmlInput
#define wmlInputPassword 	palmWmlInputPassword
#define wmlInputValue 		palmWmlInputValue
#define getPos 			palmGetPos
#define wmlSelectOptionTxt 	palmWmlSelectOptionTxt
#define wmlSelect 		palmWmlSelect 
#define wmlSelectValue 		palmWmlSelectValue
#define wmlSelectOption 	palmWmlSelectOption
#define wmlSelectEnd 		palmWmlSelectEnd
#endif __palmos__

#define Compare(x,y,z) ((StrNCaselessCompare(x,y,z) == 0) && !(StrLen(x) > z)) ? true : false
#define CompareJustTxt(x,y,z) (StrNCaselessCompare(x,y,z) == 0) ? true : false



int entity(char *text);
int textstart();
int texts (char *text);
int textend();
Char *getXHTMLAttrValue(Char *arg, struct ArgvTable *args, int numargs);
int starttag (char *tag, struct ArgvTable *args, int numargs);
int endtag (char *tag);

Char *StrCompareLower(Char *str, const Char *token);

extern Boolean isPre;
extern Boolean definitions;
extern int tableCount;
extern Boolean	isDiv;
extern Boolean gotTitle;
extern Boolean optIsSel;
extern IndexedColorType tableColor;
extern int topTag;
extern Boolean isScript;
extern Int32 cellspacing;
extern Boolean isSelect;
extern int cardCount;
extern int formCount;
extern Boolean streamLink;
extern Boolean isTextarea;

int endtag (char *tag) {
        GlobalsType 		*g;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (g->xhtml.end == true) return 0;

	showProgress(g);

	if (ProcessEventLoopTimeout(0))
		g->userCancel = true;

	EvtResetAutoOffTimer();

	xhtml_debugOut(tag, NULL, 0, true, "before", g);

	if (!PositionIsIgnoreEndTag(tag)) {
		PositionRemoveLastPosition(g);
	}


	if (Compare(tag, "html", 4)) {						// HTML
		//if (topTag == TT_HTML || topTag == TT_NONE) { g->xhtml.end = true; topTag = TT_NONE; g->Render = false; }
		g->xhtml.end = true; topTag = TT_NONE; g->Render = false;

		if (gotTitle == false) {
			if (g->deckTitle != NULL) Free(g->deckTitle);
	        	g->deckTitle = Malloc(StrLen("Untitled")+1);
	        		ErrFatalDisplayIf (!g->deckTitle, "Malloc Failed");
			StrCopy(g->deckTitle,"Untitled");
			browserSetTitle(g->deckTitle);
		}

		//if (hasBG == true) { // just in case
		//	g->xhtml.pageBG = UIColorGetTableEntryIndex(UIFieldBackground);
		//
		//	hasBG = false;
		//}

		removeLastPreDefBackground(false, g);
	} else if (Compare(tag, "body", 4)) {					// BODY
		//if (hasBG == true) {
    		//	g->xhtml.pageBG = UIColorGetTableEntryIndex(UIFieldBackground);
		//
		//	hasBG = false;
		//}
		AreaPtr		area = (g->xhtml.background)?g->xhtml.background->areaPtr:NULL;

		AreaCloseArea(area, g->x, g->y, g);
		AreaApplyMinimumResolution(area, g);
		removeLastBodyBackground(false, g);
		//removeLastBackground(false, g);
	} else if (Compare(tag, "head", 4)) {					// HEAD
		g->Render = true;
	} else if (Compare(tag, "wml:card", 8) ||
		   Compare(tag, "card", 4)) {					// WML:CARD
		g->Render = true;
	} else if (Compare(tag, "wml:onevent", 11) ||
		   Compare(tag, "onevent", 7)) {				// WML:ONEVENT
		g->onEvent = NOEVENT;
		needTimer = false;
		onenterforward = false;
		onenterbackward = false;
		onevent = false;
		isOnpickLink = false;
		if (isOnenterRefresh)
			g->hasEvent = false;
		isOnenterRefresh = false;
		isOnforwardPost = false;
	} else if (Compare(tag, "wml:prev", 8) ||
		   Compare(tag, "wml:refresh", 11) ||
		   Compare(tag, "wml:noop", 8) ||
		   Compare(tag, "prev", 4) ||
		   Compare(tag, "refresh", 7) ||
		   Compare(tag, "noop", 4)) {					// WML:PREV, WML:REFRESH, WML:NOOP

		if (Compare(tag, "wml:refresh", 11) || Compare(tag, "refresh", 7)) isRefresh = false;

		if (isAnchor == true){
			//WinPushDrawState();
			//WinSetTextColor( indexedColor );
	    		//WinSetForeColor( indexedColor );
			//WinSetUnderlineMode(grayUnderline);
		} else {
			if (isLink == true) {
				HyperlinkSetLink(false, NULL, NULL, NULL, g);
			}
		}
		isLink = false;
	} else if (Compare(tag, "title", 5)) {					// TITLE
		g->Render = false;
		if (g->xhtml.buffer) {
			browserSetTitle(XHTMLConvertEncoding(g->xhtml.buffer,g)); // SetFieldFromStr2(g->xhtml.buffer,fieldTitle);
			if (g->deckTitle != NULL) Free(g->deckTitle);
	        	g->deckTitle = Malloc(StrLen(g->xhtml.buffer)+1);
	        		ErrFatalDisplayIf (!g->deckTitle, "Malloc Failed");
			StrCopy(g->deckTitle,g->xhtml.buffer);
			gotTitle = true;
		} else {
			browserSetTitle(" "); // SetFieldFromStr2(g->xhtml.buffer,fieldTitle);
			if (g->deckTitle != NULL) Free(g->deckTitle);
	        	g->deckTitle = Malloc(StrLen(" ")+1);
	        		ErrFatalDisplayIf (!g->deckTitle, "Malloc Failed");
			StrCopy(g->deckTitle," ");
			gotTitle = true;
		}
	} else if (Compare(tag, "center", 6)) {					// CENTER
		xhtml_renderLn(g);
		removeLastAlignment(g);
	} else if (Compare(tag, "p", 1) ||
		   Compare(tag, "div", 3)) {					// P, DIV


		if (Compare(tag, "p", 1))
			g->xhtml.paragraph = false;


		if (Compare(tag, "div", 3))
			isDiv = false;

		if (g->x > g->positionIndent) xhtml_renderLn(g);
		removeLastAlignment(g);

		removeLastPreDefBackground(/*true*/(g->xhtml.background->noRefresh)?false:true, g);

		palmRegular(false);

                //g->x = 0;
		//g->y += 2;
		//g->Render = true;
	} else if (Compare(tag, "caption", 7)) {				// CAPTION
		xhtml_renderLn(g);
		removeLastAlignment(g);
		bold(false);
	} else if (Compare(tag, "wml:go", 6) ||
		   Compare(tag, "go", 2)) {					// WML:GO
		if (isOnforwardPost != true) g->post = false;
		if (isAnchor == true){
			//WinPushDrawState();
			//WinSetTextColor( indexedColor );
	    		//WinSetForeColor( indexedColor );
			//WinSetUnderlineMode(grayUnderline);
		} else {
			if (isLink == true) {
				HyperlinkSetLink(false, NULL, NULL, NULL, g);
			}
		}
		isLink = false;
		isOnpickLink = false;
	} else if (Compare(tag, "wml:postfield", 13) ||
		   Compare(tag, "postfield", 9)) {				// WML:POSTFIELD

	} else if (Compare(tag, "a", 1) ||
		   Compare(tag, "do", 2)) {					// A, DO
		isLink = false;
		HyperlinkSetLink(false, NULL, NULL, NULL, g);
	} else if (Compare(tag, "wml:anchor", 10) ||
		   Compare(tag, "anchor", 6)) {					// WML:ANCHOR
		isLink = false;
		isAnchor = false;
				
		HyperlinkSetLink(false, NULL, NULL, NULL, g);
	} else if (Compare(tag, "b", 1)) {					// B
		bold(false);
	} else if (Compare(tag, "span", 4)) {					// SPAN
		regular(false);
		removeLastBackground(true, g);
	} else if (Compare(tag, "big", 3)) {					// BIG
		big(false);
	} else if (Compare(tag, "u", 1) ||
		   Compare(tag, "ins", 3)) {					// U, INS
		underline(false);
	} else if (Compare(tag, "em", 2)) {					// EM
		em(false);
	} else if (Compare(tag, "strong", 6)) {					// STRONG
		strong(false);
	} else if (Compare(tag, "small", 5)) {					// SMALL
		small(false);
	} else if (Compare(tag, "i", 1) || 
		   Compare(tag, "address", 7) ||
		   Compare(tag, "cite", 4) || 
		   Compare(tag, "dfn", 3) ||
		   Compare(tag, "var", 3)) {					// I, ADDRESS, CITE, DFN, VAR
		if (Compare(tag, "address", 7) && (g->x != 0)) xhtml_renderLn(g); 
		italic(false);
	} else if (Compare(tag, "pre", 3)) {					// PRE
		//g->Render = true;
		isPre = false;
		pre(false);
		xhtml_renderLn(g);
	} else if (Compare(tag, "code", 4) ||
		   Compare(tag, "samp", 4) ||
		   Compare(tag, "kbd", 3)) {					// CODE, SAMP, KBD
		//g->Render = true;
		pre(false);
	} else if (Compare(tag, "h1", 2) || Compare(tag, "h2", 2) || 
		   Compare(tag, "h3", 2) || Compare(tag, "h4", 2) ||
		   Compare(tag, "h5", 2) || Compare(tag, "h6", 2)) {		// H1-H6
			xhtml_renderLn(g);
			strong(false);
	} else if (Compare(tag, "style", 5) || Compare(tag, "script", 6)) {	// STYLE, SCRIPT
		g->Render = true;
		isScript = false;
	} else if (Compare(tag, "abbr", 4) || 
		   Compare(tag, "acronym", 7)) {				// ABBR, ACRONYM
		//g->Render = true;
	} else if (Compare(tag, "blockquote", 10)) {				// BLOCKQUOTE
		g->indent -= 1;
		//g->rightIndent -= 1;
		if (g->indent == 0) g->rightIndent = 0;
		//g->Render = true;
		xhtml_renderLn(g);
		if (isLink == true) {
			isLink = false;
			HyperlinkSetLink(false, NULL, NULL, NULL, g);
		}
	} else if (Compare(tag, "q", 1)) {					// Q
		//g->Render = true;
		if (isLink == true) {
			isLink = false;
			HyperlinkSetLink(false, NULL, NULL, NULL, g);
		}
	} else if (Compare(tag, "hr", 2)) {					// HR
		//g->Render = true;
	} else if (Compare(tag, "dl", 2)) {					// DL
		definitions = false;
		if (g->x != g->positionIndent) xhtml_renderLn(g);
		removeLastAlignment(g);
	} else if (Compare(tag, "dt", 2)) {					// DT
		
	} else if (Compare(tag, "dd", 2)) {					// DD
		if (definitions == true) {
			g->indent = 0;
			g->rightIndent = 0;
		}
	} else if (Compare(tag, "table", 5)) {					// TABLE
		removeLastPreDefBackground(true, g);
		if (g->x != g->positionIndent) xhtml_renderLn(g);
		if (tableCount == 1) {
			removeLastAlignment(g);
			WinSetTextColor(tableColor);
		}
		tableCount--;
	} /*else if (Compare(tag, "tbody", 5)) {					// TBODY
		
	}*/ else if (Compare(tag, "tr", 2)) {					// TR
		if (g->x != g->positionIndent) xhtml_renderLn(g);
		removeLastPreDefBackground(true, g);
		palmCurrent(false);
	} else if (Compare(tag, "th", 2)) {					// TH
		g->x += cellspacing; // g->x += 5;
	} else if (Compare(tag, "td", 2)) {					// TD
		g->x += cellspacing; // g->x += 5;
		removeLastBackground(true, g);
		palmCurrent(false);
	} else if (Compare(tag, "ul", 2) ||
		   Compare(tag, "nl", 2)) {					// UL, NL
		removeLastList(g);
		g->rightIndent = 0;
		if (g->indent == 0) {
			if (g->x != g->positionIndent)
				xhtml_renderLn(g);
			//removeLastAlignment(g);
		}
		//removeLastAlignment(g);
		//xhtml_renderLn(g);
	} else if (Compare(tag, "ol", 2)) {					// OL
		removeLastList(g);
		if (g->indent == 0) {
			g->rightIndent = 0;
			if (g->x != g->positionIndent)
				xhtml_renderLn(g);
			//removeLastAlignment(g);
		}
		//xhtml_renderLn(g);
	} else if (Compare(tag, "li", 2)) {					// LI
		//g->Render = true;
		if (isLink == true) {
			isLink = false;
			HyperlinkSetLink(false, NULL, NULL, NULL, g);
		}
	} else if (Compare(tag, "form", 4)) {					// FORM
		if (g->x != g->positionIndent) //if ((g->x != 0) || (g->x != (152 / 2)) || (g->x != 152)) 
			xhtml_renderLn(g);
		formDeactivate(g);
	} else if (Compare(tag, "button", 6)) {					// BUTTON
		if (g->xhtml.buffer) {
			g->xhtml.buffer = XHTMLConvertEncoding(g->xhtml.buffer,g);
			formDrawButton(g->xhtml.buffer, &g->x, &g->y, false, g);
		}
		g->Render = true;
	} else if (Compare(tag, "label", 5)) {					// LABEL
		
	} else if (Compare(tag, "textarea", 8)) {				// TEXTAREA
		isTextarea = false;

		if (getVar(g->Input->name,g) != "" && (StrLen(getVar(g->Input->name,g)) > 0)) {
			wmlInputValue(getVar(g->Input->name,g),g);

			if (g->xhtml.buffer) { 
				g->xhtml.buffer = XHTMLConvertEncoding(g->xhtml.buffer,g); 
				formAddValue(formEatVarName(g->Input->name,g), g->xhtml.buffer, TEXT, g); //formAddValue(g->Input->name+StrLen(g->Forms->id)+1, g->xhtml.buffer, TEXT, g);
			} else {
				formAddValue(formEatVarName(g->Input->name,g), "", TEXT, g); //formAddValue(g->Input->name+StrLen(g->Forms->id)+1, "", TEXT, g);
			}
		} else {
			if (g->xhtml.buffer) { 
				g->xhtml.buffer = XHTMLConvertEncoding(g->xhtml.buffer,g); 
				wmlInputValue(g->xhtml.buffer,g);

				addVar(g->Input->name, g);
				setVar(g->Input->name, g->xhtml.buffer, g);

				formAddValue(formEatVarName(g->Input->name,g), g->xhtml.buffer, TEXT, g); //formAddValue(g->Input->name+StrLen(g->Forms->id)+1, g->xhtml.buffer, TEXT, g);
			} else {
				formAddValue(formEatVarName(g->Input->name,g), "", TEXT, g); //formAddValue(g->Input->name+StrLen(g->Forms->id)+1, "", TEXT, g);
			}
		}
		g->Render = true;
	} else if (Compare(tag, "fieldset", 8)) {				// FIELDSET
		starttag("hr", NULL, 0);
	} else if (Compare(tag, "input", 5)) {					// INPUT
		
	} else if (Compare(tag, "select", 6)) {					// SELECT
		wmlSelectEnd(g);
		g->Render = true;
		isOption = false;
		isSelect = false;

		if (FormGetInitValue(g->Select->name, g) && !StrLen(getVar(g->Select->name,g))) {
			wmlSelectValue(FormGetInitValue(g->Select->name, g), g);
		} else if (!StrLen(getVar(g->Select->name,g))) {
			wmlSelectValue(g->Select->options->value,g);
		}
	} else if (Compare(tag, "optgroup", 8)) {				// OPTGROUP
		
	} else if (Compare(tag, "option", 6)) {					// OPTION
		if (g->xhtml.buffer) { 
			g->xhtml.buffer = XHTMLConvertEncoding(g->xhtml.buffer,g); 
			wmlSelectOptionTxt(g->xhtml.buffer,g); 
		}

		if (optIsSel) { // && (!StrLen(getVar(g->Select->name,g)))) {
			//if (!StrLen(getVar(g->Select->name,g)))
			//	wmlSelectValue(g->Select->options->value,g);
			if (FormGetInitValue(g->Select->name, g) &&
			    !StrLen(FormGetInitValue(g->Select->name, g))) {
				OptionPtr	opt = g->Select->options;

				while (opt->next)
					opt = opt->next;

				if (opt) {
					/*if (g->Forms->value->initval)
						Free(g->Forms->value->initval);
					g->Forms->value->initval = StrDup(opt->value);*/

					FormSetInitValue(g->Select->name, opt->value, g);
				}
			}
			optIsSel = false;
		}
		optIsSel = false;
		g->Render = true;
		isOption = false;
		isOnpickLink = false;
	} else if (Compare(tag, "font", 4)) {					// FONT
		//if (changedFontColor == true) {
		//	WinSetTextColor( savedColor );
		//
		//	changedFontColor = false;
		//}
		regular(false);
	} else if (Compare(tag, "wml", 3)) {					// WML
		if (topTag == TT_WML || topTag == TT_NONE) { g->xhtml.end = true; topTag = TT_NONE; g->Render = false; }
		if (gotTitle == false) {
			browserSetTitle("WAPUniverse"); // SetFieldFromStr2("WAPUniverse",fieldTitle);
			if (g->deckTitle != NULL) Free(g->deckTitle);
	        	g->deckTitle = Malloc(StrLen("Untitled")+1);
	        		ErrFatalDisplayIf (!g->deckTitle, "Malloc Failed");
			StrCopy(g->deckTitle,"Untitled");
		}
	} else if (Compare(tag, "map", 3)) {					// MAP
		
	} else if (Compare(tag, "area", 4)) {					// AREA
		
	} else if (Compare(tag, "frame", 5)) {					// FRAME
		
	} else if (Compare(tag, "link", 4)) {					// LINK

	} else if (Compare(tag, "sub", 3)) {					// SUB
		//isSubscript = false;
		subscript(false);
	} else if (Compare(tag, "sup", 3)) {					// SUP
		//isSuperscript = false;
		superscript(false);
	} else if (Compare(tag, "strike", 6) ||
		   Compare(tag, "s", 1)) {					// STRIKE, S
		//isStrikethrough = false;
		strike(false);
	} else if (Compare(tag, "img", 3)) {					// IMG
		g->imgAlign = None;
	} else if (Compare(tag, "bdo", 3)) {					// BDO
		if (g->browserGlobals.textDirRTL)
			TextSetDirectionRTL(g);
		g->browserGlobals.textDirRTL = false;
	} else {
		//g->Render = true;
	}

	xhtml_debugOut(tag, NULL, 0, true, "after", g);

        return 0;
}

