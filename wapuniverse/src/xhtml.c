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
#include	"Media.h"

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
#define hyperLink 		palmHyperLink
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


// TODO: Move to a xhtml global structure in GlobalsType
Boolean isPre = false;
Boolean definitions = false;
int tableCount = 0;
Boolean	isDiv = false;
Boolean gotTitle = false;
Boolean optIsSel = false;
IndexedColorType tableColor = 0;
int topTag = TT_NONE;
Boolean isScript = false;
Int32 cellspacing = 3;
Boolean isSelect = false;
int cardCount = 0;
int formCount = 0;
Boolean streamLink = false;
Boolean isTextarea = false;


int starttag (char *tag, struct ArgvTable *args, int numargs) {
        GlobalsType 		*g;
	Char 			*attr, *alt, *name, *val, *data, 
				*checked, *value, *formName, *rows, 
				*cols, *action, *type, *src, *title,
				*attrWidth, *attrHeight;
	Boolean 		disabled = false;
	UInt16			imgWidth = 0, imgHeight = 0;
	Boolean			imgNoBorder;
	UInt32			width = 0;

	FtrGet(wuAppType, ftrGlobals, (UInt32 *)&g);

	if (g->xhtml.end == true) return 0;

	showProgress(g);

	if (ProcessEventLoopTimeout(0))
		g->userCancel = true;

	EvtResetAutoOffTimer();

	xhtml_debugOut(tag, args, numargs, false, "before", g);


	if (!PositionIsIgnoreStartTag(tag)) {
		Char	*style = NULL, *position;
		Char	*x = NULL, *y = NULL;
		UInt16	newX = g->x, newY = g->y;
		
		style = getXHTMLAttrValue("style", args, numargs);
		if (style) {
			position = extractCSSElement("position: ",style);
			if (position && StrNCaselessCompare(position, "absolute", 8)==0) {

				xhtml_freeAttr(style);
				style = getXHTMLAttrValue("style", args, numargs);
				x = extractCSSElement("left: ",style);
				newX = (UInt16)StrAToI(x);

				xhtml_freeAttr(style);
				style = getXHTMLAttrValue("style", args, numargs);
				y = extractCSSElement("top: ",style);
				newY = (UInt16)StrAToI(y);
				xhtml_freeAttr(style);

				PositionAddNewPosition(g->x, g->y, (newX)?newX:g->x, (newY)?newY:g->y, (UInt16)g->imgH, (newX)?newX:g->x, false, g);
			} else {
				PositionAddNewPosition(g->x, g->y, g->x, g->y, (UInt16)g->imgH, (g->position)?g->position->indent:0, true, g);
			}
		} else {
			PositionAddNewPosition(g->x, g->y, g->x, g->y, (UInt16)g->imgH, (g->position)?g->position->indent:0, true, g);
		}
	}

	if (Compare(tag, "html", 4)) { 						// HTML
		if (topTag == TT_NONE) topTag = TT_HTML; // should we check for previous topTag??
		if ((attr = getXHTMLAttrValue("wml:onenterforward", args, numargs))) {
			if (g->onenterback != true) {
				followLink(attr, g);
			}
			xhtml_freeAttr(attr);
		} else if ((attr = getXHTMLAttrValue("wml:onenterbackward", args, numargs))) {
			if ((g->onenterback == true) && (g->refreshEvent == false)) {
				followLink(attr, g);
			}
			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("wml:ontimer", args, numargs))) {
			if (g->xhtml.ontimerUrl)
				Free(g->xhtml.ontimerUrl);
			g->xhtml.ontimerUrl = Malloc(StrLen(attr)+1);
			ErrFatalDisplayIf (!g->xhtml.ontimerUrl, "Malloc Failed");
			StrCopy(g->xhtml.ontimerUrl,attr);
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "wml", 3)) { 					// wML
		if (topTag == TT_NONE) topTag = TT_WML; // should we check for previous topTag??
	} else if (Compare(tag, "head", 4)) {					// HEAD
		g->Render = false;
	} else if (Compare(tag, "base", 4)) {					// BASE
		attr = getXHTMLAttrValue("href", args, numargs);
		if (attr) {
			StrCopy(g->WapLastUrl, attr);
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "meta", 4)) {					// META
		attr = getXHTMLAttrValue("http-equiv", args, numargs);
		if (attr) {
			if (Compare(attr, "refresh", 7)) {
				xhtml_freeAttr(attr);
				attr = getXHTMLAttrValue("content", args, numargs);
				if (attr) {
					Char *urlStr;
					char refVal[10];
					int  ref = 0, rv = 0;

					if ((urlStr = StrCompareLower(attr, "url="))) {
						while (attr[ref] != ';' && attr[ref] != '\0' && attr[ref] != ' ') {
							refVal[rv] = attr[ref];
							rv++; ref++;
						}
						refVal[rv++] = '\0';
						g->xhtml.timerVal = StrAToI(refVal);

						if (g->xhtml.timerVal == 0)
							g->xhtml.timerVal = 1;

						if (g->xhtml.ontimerUrl)
							Free(g->xhtml.ontimerUrl);
						g->xhtml.ontimerUrl = Malloc(StrLen(urlStr)+1);
						ErrFatalDisplayIf (!g->xhtml.ontimerUrl, "Malloc Failed");

						if (urlStr[StrLen(urlStr) - 1] == '\'')
							urlStr[StrLen(urlStr) - 1] = '\0';

						if (urlStr[4] == '\'')
							StrCopy(g->xhtml.ontimerUrl,urlStr+5);
						else
							StrCopy(g->xhtml.ontimerUrl,urlStr+4);
					} else {
						g->xhtml.timerVal = StrAToI(attr);

						if (g->xhtml.timerVal == 0)
							g->xhtml.timerVal = 1;

						if (g->xhtml.ontimerUrl)
							Free(g->xhtml.ontimerUrl);
						g->xhtml.ontimerUrl = Malloc(StrLen(g->Url)+1);
						ErrFatalDisplayIf (!g->xhtml.ontimerUrl, "Malloc Failed");
						StrCopy(g->xhtml.ontimerUrl,g->Url);
					}

					xhtml_freeAttr(attr);
				}
			} else if (Compare(attr, "content-type", 12)) {
				xhtml_freeAttr(attr);

				attr = getXHTMLAttrValue("content", args, numargs);
				if (attr) {
					char charSet[75];
					Boolean hasCharSet = false;
					int v = 0, c = 0;

					while ( attr[v] != ';' && attr[v] != '\n' && attr[v] != '\r' &&
						attr[v] != '\0' ) v++;
							// no real point in handeling the content type now since we
							// are already handeling it and are going off of the top tag

					if (attr[v] == ';') { hasCharSet = true; v++; }

					if (hasCharSet == true) {
						while (attr[v] == ' ') v++;
						if (StrCompareLower(attr+v, "charset=")) {
							v += 8;

							while ( attr[v] != ';' && attr[v] != '\n' && attr[v] != '\r' &&
								attr[v] != '\0' ) {
								charSet[c] = attr[v];
								v++; c++;
							}
							charSet[c] = '\0';
	
							if (StrCompareLower(charSet, "utf-8")) {
								g->charSet = UTF_8;
							} else if (StrCompareLower(charSet, "us-ascii")) {
								g->charSet = US_ASCII;
							} else if (StrCompareLower(charSet, "iso-8859-1")) {
								g->charSet = ISO_8859_1;
							} else if (StrCompareLower(charSet, "usc-2")) {
								g->charSet = USC_2;
							} else {
								g->charSet = UTF_8;
							}
						}
					}
				}

				xhtml_freeAttr(attr);
			} else if (Compare(attr, "pragma", 6) || Compare(attr, "cache-control", 13) ||
				   Compare(attr, "Pragma", 6) || Compare(attr, "Cache-Control", 13)) {
					xhtml_freeAttr(attr);

					attr = getXHTMLAttrValue("content", args, numargs);
					if (attr) {
						if (Compare(attr, "no-cache", 8) || Compare(attr, "No-Cache", 8) ||
						    Compare(attr, "no-store", 8) || Compare(attr, "No-Store", 8)) {
							g->cache.cacheDoNotRead = true;
							CacheDeleteEntry(g->Url, false, g);
						}

						xhtml_freeAttr(attr);
					}
			} else
				xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "wml:access", 10) ||
		   Compare(tag, "access", 6)) {					// WML:ACCESS
    		int		u = 0;
    		Boolean		gotDomain = false;
   		Boolean		noaccess = false;

		if (g->History->index > 0) {
			u = g->History->index;
			u--;
		}
		attr = getXHTMLAttrValue("domain", args, numargs);
		if (attr){
			if (g->History->index > 0) {
				noaccess = checkAccess(attr, g->History->hist[u].url, 1);
				if (noaccess == true) {
					xhtml_noAccess(&(g->xhtml), g);
					xhtml_freeAttr(attr);
					return(-15);
				}
			} else {
				xhtml_noAccess(&(g->xhtml), g);
				xhtml_freeAttr(attr);
				return(-15);
			}
			gotDomain = true;
			xhtml_freeAttr(attr);
		}
		attr = getXHTMLAttrValue("path", args, numargs);
		if (attr){
			if (gotDomain == true) {
				if (g->History->index > 0) {
					noaccess = checkAccess(attr, g->History->hist[u].url, 2);
					if (noaccess == true) {
						xhtml_noAccess(&(g->xhtml), g);
						xhtml_freeAttr(attr);
						return(-15);
					}
				} else {
					xhtml_noAccess(&(g->xhtml), g);
					xhtml_freeAttr(attr);
					return(-15);
				}
			} else {
				xhtml_noAccess(&(g->xhtml), g);
				xhtml_freeAttr(attr);
				return(-15);
			}
			xhtml_freeAttr(attr);
		}
	} else if(Compare(tag, "wml:do", 6) || Compare(tag, "do", 2)) {		//WML:DO

		attr = getXHTMLAttrValue("label", args, numargs);
		if (attr){ // TODO: should look for "prev" type??
			//hyperLink(true,NULL,NULL,g);
			//g->needSpace=1;
			isLink = true;
			drawDo (attr, g);
			xhtml_freeAttr(attr);
		} else {
			attr = getXHTMLAttrValue("title", args, numargs);
			if (attr){
				//hyperLink(true,NULL,NULL,g);
				//g->needSpace=1;
				isLink = true;
				drawDo (attr, g);
				xhtml_freeAttr(attr);
			} else {
				attr = getXHTMLAttrValue("name", args, numargs);
				if (attr){
					//hyperLink(true,NULL,NULL,g);
					//g->needSpace=1;
					isLink = true;
					drawDo (attr, g);
					xhtml_freeAttr(attr);
				} else {
					//hyperLink(true,NULL,NULL,g);
					//g->needSpace=1;
					isLink = true;

					attr = getXHTMLAttrValue("type", args, numargs);
					if (attr){
						drawDo(attr, g);
						xhtml_freeAttr(attr);
						goto bottom;
					}
						drawDo("Unknown", g);
				bottom:
				}
			}
		}
	} else if (Compare(tag, "title", 5)) {					// TITLE
		g->Render = false;
	} else if (Compare(tag, "wml:onevent", 11) ||
		   Compare(tag, "onevent", 7)) {				// WML:ONEVENT
		onevent = true;
		isOnforwardPost = false;
		attr = getXHTMLAttrValue("type", args, numargs);
		if (attr) {
			if (Compare(attr, "ontimer", 7)){
				g->onEvent = ONTIMER;
				needTimer = true;
			} else if (Compare(attr, "onenterforward", 14)){
				if ((g->onenterback == false) && (g->refreshEvent == false)) {
					g->onEvent = ONENTERFORWARD;
					onenterforward = true;
					g->hasEvent = true;
				} else {
					g->onEvent = IGNORE;
					onenterforward = false;
				}
			} else if (Compare(attr, "onenterbackward", 15)){
				if ((g->onenterback == true) && (g->refreshEvent == false)) {
					g->onEvent = ONENTERBACKWARD;
					onenterbackward = true;
					g->hasEvent = true;
				} else {
					g->onEvent = IGNORE;
					onenterbackward = false;
				}
			} else if (Compare(attr, "onpick", 6)){
				g->onEvent = ONPICK;
			}
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "wml:prev", 8) ||
		   Compare(tag, "prev", 4)) {					// WML:PREV
		if (isLink == true){
			HyperlinkSetLink(false, "", "<PREV/>", NULL, g);
		}
		switch (g->onEvent) {
			case ONTIMER: // Not sure if this will ever happen??
				if (g->xhtml.ontimerUrl)
					Free(g->xhtml.ontimerUrl);
				g->xhtml.ontimerUrl = Malloc(StrLen("<PREV/>")+1);
				ErrFatalDisplayIf (!g->xhtml.ontimerUrl, "Malloc Failed");
				StrCopy(g->xhtml.ontimerUrl,"<PREV/>");
				g->onEvent = NOEVENT;
				break;
			case ONENTERFORWARD:
			case ONENTERBACKWARD:
				if (g->onforward) Free(g->onforward);
				g->onforward = Malloc(StrLen("<PREV/>")+1);
					ErrFatalDisplayIf (!g->onforward, "Malloc Failed");
                		StrCopy(g->onforward, "<PREV/>");
				break;
			case ONPICK:
				palmWmlSelectOptionOnPick("", "<PREV/>", g);
				isOnpickLink = true;
				break;
			case IGNORE:
			case NOEVENT:
			default:
				break;
		}
	} else if (Compare(tag, "wml:refresh", 11) ||
		   Compare(tag, "refresh", 7)) {				// WML:REFRESH
		if (isLink == true){
			HyperlinkSetLink(false, "", "<REFRESH/>", NULL, g);
		}
		switch (g->onEvent) {
			case ONTIMER: // Not sure if this will ever happen??
				g->onEvent = NOEVENT;
				break;
			case ONENTERFORWARD:
			case ONENTERBACKWARD:
				//isRefresh = true;
				isOnenterRefresh = true;
				break;
			case ONPICK:
				palmWmlSelectOptionOnPick("", "<REFRESH/>", g);
				isOnpickLink = true;
				break;
			case IGNORE:
			case NOEVENT:
			default:
				if (onevent) break;
				isRefresh = true;
				break;
		}
	} else if (Compare(tag, "wml:noop", 8) ||
		   Compare(tag, "noop", 4)) {					// WML:NOOP
		//Do not do anything. This is a "NO" "OP"eration.//Well, this is really not nothing
		if (isLink == true){
			HyperlinkSetLink(false, "", "<NOOP/>", NULL, g);
		}
		switch (g->onEvent) {
			case ONTIMER: // Not sure if this will ever happen??
				if (g->xhtml.ontimerUrl)
					Free(g->xhtml.ontimerUrl);
				g->xhtml.timerVal = -1;
				g->onEvent = NOEVENT;
				break;
			case ONENTERFORWARD:
			case ONENTERBACKWARD:
				if (g->onforward) Free(g->onforward);
				g->hasEvent = false;
				break;
			case ONPICK:
				//palmWmlSelectOptionOnPick("", "<NOOP/>", g); // I do not think this exists
				break;
			case IGNORE:
			case NOEVENT:
			default:
				break;
		}
	} else if (Compare(tag, "body", 4)) {					// BODY
		if (topTag == TT_NONE)	topTag = TT_HTML;
		g->Render = true; // needed?

		if ((attr = getXHTMLAttrValue("width", args, numargs))) {
			width = xhtmlReadWidth(attr);


			if ((width > browserGetWidgetBounds(DRAW_WIDTH)) && (g_prefs.renderMode == RENDER_WIDE_SCREEN)) {
				g->DrawWidth = width;
				BrowserRefreshScrollBarHorizontal(g);
			}

			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("wml:onenterforward", args, numargs))) {
			if (g->onenterback != true) {
				followLink(attr, g);
			}
			xhtml_freeAttr(attr);
		} else if ((attr = getXHTMLAttrValue("wml:onenterbackward", args, numargs))) {
			if ((g->onenterback == true) && (g->refreshEvent == false)) {
				followLink(attr, g);
			}
			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("wml:ontimer", args, numargs))) {
			if (g->xhtml.ontimerUrl)
				Free(g->xhtml.ontimerUrl);
			g->xhtml.ontimerUrl = Malloc(StrLen(attr)+1);
			ErrFatalDisplayIf (!g->xhtml.ontimerUrl, "Malloc Failed");
			StrCopy(g->xhtml.ontimerUrl,attr);
			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("wml:newcontext", args, numargs))) {
			disposeVar(g);
			while (historyPop(g) != NULL);
			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("bgcolor", args, numargs))) {
			//RGBColorType 		rgb;

			//rgb = ConvertColorNumberString(attr, TYPE_PAGE);
	
    			//g->xhtml.pageBG = WinRGBToIndex( &rgb );

			//xhtml_ReSetWindow(g);

			//hasBG = true;

			addNewBackground(attr, g);
			AreaSetWidth((g->xhtml.background)?g->xhtml.background->areaPtr:NULL, width, g);
			xhtml_freeAttr(attr);
		} else {
			addNewBackground(NULL, g);
		}
	} else if (Compare(tag, "wml:card", 8) || 
		   Compare(tag, "card", 4)) {					// WML:CARD
		topTag = TT_WML; // incase of WML2, not recommended
		g->Render = true; // needed?
		if ((attr = getXHTMLAttrValue("wml:onenterforward", args, numargs)) ||
		    (attr = getXHTMLAttrValue("onenterforward", args, numargs))) {
			if (g->onenterback != true) {
				followLink(attr, g);
			}
			xhtml_freeAttr(attr);
		} else if ((attr = getXHTMLAttrValue("wml:onenterbackward", args, numargs)) ||
			   (attr = getXHTMLAttrValue("onenterbackward", args, numargs))) {
			if ((g->onenterback == true) && (g->refreshEvent == false)) {
				followLink(attr, g);
			}
			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("wml:ontimer", args, numargs)) ||
		    (attr = getXHTMLAttrValue("ontimer", args, numargs))) {
			if (g->xhtml.ontimerUrl)
				Free(g->xhtml.ontimerUrl);
			g->xhtml.ontimerUrl = Malloc(StrLen(attr)+1);
			ErrFatalDisplayIf (!g->xhtml.ontimerUrl, "Malloc Failed");
			StrCopy(g->xhtml.ontimerUrl,attr);
			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("wml:newcontext", args, numargs)) ||
		    (attr = getXHTMLAttrValue("newcontext", args, numargs))) {
			disposeVar(g);
			while (historyPop(g) != NULL);
			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("title", args, numargs))) {
			if (gotTitle == false) {
				browserSetTitle(XHTMLConvertEncoding(attr,g)); // SetFieldFromStr2(attr,fieldTitle);
				if (g->deckTitle != NULL) Free(g->deckTitle);
	        		g->deckTitle = Malloc(StrLen(attr)+1);
	        			ErrFatalDisplayIf (!g->deckTitle, "Malloc Failed");
				StrCopy(g->deckTitle,attr);
				gotTitle = true;
			}
			if (cardCount > 0) {
				starttag ("hr", NULL, 0);
				//xhtml_renderLn(g);

				addNewAlignmnet(Center, g); //g->x = (152 / 2);
				bold(true);
				xhtml_renderString(&(g->xhtml),attr,g);
				bold(false);
				xhtml_renderLn(g);
				removeLastAlignment(g); //g->x = 0;
			}
			xhtml_freeAttr(attr);
		}
		if ((attr = getXHTMLAttrValue("id", args, numargs))) {
			addPageLocation(attr, g);
			xhtml_freeAttr(attr);
		}
		cardCount++;
	} else if (Compare(tag, "wml:timer", 9) || Compare(tag, "timer", 5)) {	// WML:TIMER
		if ((attr = getXHTMLAttrValue("value", args, numargs))) {
			g->xhtml.timerVal = StrAToI(attr);
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "br", 2)) {					// BR
		if (g->Render == true) xhtml_renderBr(g);
	} else if (Compare(tag, "img", 3)) {					// IMG
		imgNoBorder = false;

		attrWidth = getXHTMLAttrValue("width", args, numargs);
		attrHeight = getXHTMLAttrValue("height", args, numargs);

		if (attrWidth && attrHeight) {
			if (StrChr(attrWidth, '%')) {
				imgWidth = 0;
				// TODO: add percent support
			} else {
				imgWidth = StrAToI(attrWidth);
			}

			if (StrChr(attrHeight, '%')) {
				imgHeight = 0;
				// TODO: add percent support
			} else {
				imgHeight = StrAToI(attrHeight);
			}

			xhtml_freeAttr(attrWidth);
			xhtml_freeAttr(attrHeight);
		} else {
			imgWidth = 0;
			imgHeight = 0;
		}

		/*if (imgWidth) {
			if ((imgWidth > browserGetWidgetBounds(DRAW_WIDTH)) && (g_prefs.renderMode == RENDER_WIDE_SCREEN)) {
				g->DrawWidth = imgWidth;
				BrowserRefreshScrollBarHorizontal(g);
			}
		}*/

		alt = getXHTMLAttrValue("alt", args, numargs);

		if ((attr = getXHTMLAttrValue("border", args, numargs))) {
			if (Compare(attr, "0", 1))
				imgNoBorder = true;

			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("align", args, numargs))){
			if (Compare(attr, "left", 4))
				g->imgAlign = Left;
			else
				g->imgAlign = None;
			xhtml_freeAttr(attr);
		} else {
			g->imgAlign = None; // maybe not a good idea???
		}

		if ((attr = getXHTMLAttrValue("wml:localsrc", args, numargs)) || 
		    (attr = getXHTMLAttrValue("localsrc", args, numargs))){
			//we have a localsrc, img should be blank
			xhtml_freeAttr(attr);
		} else {
			attr = getXHTMLAttrValue("src", args, numargs);
			if (attr) {
				if ( ImageCreateImage(attr, &imgWidth, &imgHeight, (imgNoBorder)?false:true, alt, g) != true ) {
					/*if (alt) {
						ImageCreateEmptyImage(alt, &g->x, &g->y, false, g);
					} else {
						ImageCreateEmptyImage(NULL, &g->x, &g->y, false, g);
					}*/
					ImageCreateEmptyImage((alt)?alt:NULL, &g->x, &g->y, &imgWidth, &imgHeight, false, g);
				}
				xhtml_freeAttr(attr);
			}
		}

		imgNoBorder = false;
		if (alt) xhtml_freeAttr(alt);

		attr = getXHTMLAttrValue("usemap", args, numargs);
		if (attr) {
			imageAddMap(attr, g);
			xhtml_freeAttr(attr);
		}

		//if (isLink == true){ // if image was embeded into a hyperlink then reset the hyperlink.
		//	WinSetTextColor( indexedColor );
	    	//	WinSetForeColor( indexedColor );
		//	WinSetUnderlineMode(grayUnderline);
		//}
	} else if (Compare(tag, "blockquote", 10)) {				// BLOCKQUOTE
		g->indent += 1;
		g->rightIndent = 1;
		//g->Render = true;
		if (g->x != g->positionIndent) xhtml_renderLn(g);
		if (g->x == g->positionIndent) g->x = g->indent * BrowserIndentValue() + g->positionIndent;
		if (isLink) HyperlinkSetLink(false, NULL, NULL, NULL, g);
		if ((attr = getXHTMLAttrValue("cite", args, numargs))) {
			isLink = true;
			HyperlinkSetLink(true, "", attr, NULL, g);
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "b", 1)) {					// B
		bold(true);
	} else if (Compare(tag, "span", 4)) {					// SPAN
		Boolean		fchanged = false;

		if ((attr = getXHTMLAttrValue("style", args, numargs))) {
			if (StrCompareLower(attr,"font:small") || StrCompareLower(attr,"font: small"))
				small(true);
			else if (StrCompareLower(attr,"font:big") || StrCompareLower(attr,"font:large") || 
				 StrCompareLower(attr,"font: big") || StrCompareLower(attr,"font: large"))
				bold(true);

			xhtml_freeAttr(attr);
		}

		attr = getXHTMLAttrValue("style", args, numargs);
		if (attr) {
			Char	*background = NULL;

			background = extractBackground(attr);
			if (background != NULL) {
				addNewBackground(background, g);
			} else {
				addNewBackground(NULL, g);
			}

			xhtml_freeAttr(attr);
		} else {
			addNewBackground(NULL, g);
		}

		if (fchanged == false) {
			if (g->font != NULL)
				addNewFont(g->font->font, g->font->res, g);
			else
				palmRegular(true); // addNewFont(stdFont, NULL, g);
		}
	} else if (Compare(tag, "caption", 7)) {				// CAPTION
		bold(true);
		if (g->x != g->positionIndent) xhtml_renderLn(g);
		addNewAlignmnet(Center, g);
	} else if (Compare(tag, "big", 3)) {					// BIG
		big(true);
	} else if (Compare(tag, "u", 1) ||
		   Compare(tag, "ins", 3)) {					// INS
		underline(true);
	} else if (Compare(tag, "em", 2)) {					// EM
		em(true);
	} else if (Compare(tag, "strong", 6)) {					// STRONG
		strong(true);
	} else if (Compare(tag, "small", 5)) {					// SMALL
		small(true);
	} else if (Compare(tag, "i", 1) || 
		   Compare(tag, "address", 7) ||
		   Compare(tag, "cite", 4) || 
		   Compare(tag, "dfn", 3) ||
		   Compare(tag, "var", 3)) {					// I, ADDRESS, CITE, DFN, VAR
		if (Compare(tag, "address", 7) && (g->x != 0)) xhtml_renderLn(g); 
		italic(true);
	} else if (Compare(tag, "pre", 3)) {					// PRE
		//g->Render = true;
		isPre = true;
		pre(true);
	} else if (Compare(tag, "code", 4) ||
		   Compare(tag, "samp", 4) ||
		   Compare(tag, "kbd", 3)) {					// CODE, SAMP, KBD
		//g->Render = true;
		pre(true);
	} else if (Compare(tag, "h1", 2) || Compare(tag, "h2", 2) || 
		   Compare(tag, "h3", 2) || Compare(tag, "h4", 2) ||
		   Compare(tag, "h5", 2) || Compare(tag, "h6", 2)) {		// H1-H6
			/*if ((g->Align == Left && g->x != 0) ||
			    (g->Align == Center && g->x != (152 / 2)) ||
			    (g->Align == Right && g->x != 152))*/
				if (g->x != g->positionIndent) xhtml_renderLn(g);
			strong(true);
	} else if (Compare(tag, "style", 5) || Compare(tag, "script", 6)) {	// STYLE, SCRIPT
		g->Render = false;

		if (Compare(tag, "script", 6)) isScript = true;
	} else if (Compare(tag, "abbr", 4) || 
		   Compare(tag, "acronym", 7)) {				// ABBR, ACRONYM
		//g->Render = true;
	} else if (Compare(tag, "q", 1)) {					// Q
		//g->Render = true;
		if ((attr = getXHTMLAttrValue("cite", args, numargs))) {
			if (Compare(attr, "http", 4)) {
				isLink = true;
				HyperlinkSetLink(true, "", attr, NULL, g);
			}
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "hr", 2)) {					// HR
		//Boolean 		hrColor = false;
		Char			*widthP = NULL, *sizeP = NULL, *alignP = NULL;
		UInt16			width = browserGetWidgetBounds(DRAW_WIDTH), size = 2;
		Alignment		align = Left, tempAlign = g->Align;
		Boolean			shadow = true;
		RGBColorType 		rgb;
		IndexedColorType	index = 216;


		widthP = getXHTMLAttrValue("width", args, numargs);
		sizeP  = getXHTMLAttrValue("size", args, numargs);
		alignP = getXHTMLAttrValue("align", args, numargs);

		if (widthP)
			width = xhtmlReadWidth(widthP);
		if (sizeP)
			size = xhtmlReadWidth(sizeP);
		if (alignP) {
			if (Compare(alignP, "center", 6))
				align = Center;
			else if (Compare(alignP, "right", 5))
				align = Right;
			else
				align = Left;
		}

		if (widthP) xhtml_freeAttr(widthP);
		if (alignP) xhtml_freeAttr(alignP);
		if (sizeP) xhtml_freeAttr(sizeP);
		
		if (g->x != g->positionIndent) xhtml_renderLn(g);

		g->y++;

		if ((attr = getXHTMLAttrValue("color", args, numargs))) {
			rgb = ConvertColorNumberString(attr, TYPE_ELEMENT);
	
    			index = WinRGBToIndex( &rgb );

		//	origFore = WinSetForeColor(index);

			shadow = false;
		//	hrColor = true;
			xhtml_freeAttr(attr);
		}// else {
		//	origFore = WinSetForeColor(218);
		//
		//	hrColor = true;
		//}

		xhtmlDrawHR(width, size, shadow, index, g);

		//if (hrColor == true) {
		//	WinSetForeColor(origFore);
		//	hrColor = false;
		//}


		browserResizeDimensions(0, size, g);
		g->x = width; // browserGetWidgetBounds(DRAW_WIDTH);

		if (align != Left) {
			tempAlign = g->Align;
			g->Align = align;
			
			xhtml_renderLn(g);

			g->Align = tempAlign;			
		} else {
			g->x = browserGetWidgetBounds(DRAW_WIDTH);
			xhtml_renderLn(g);
		}

		g->y++;
	} else if (Compare(tag, "center", 6)) {					// CENTER
		if (g->x != g->positionIndent) xhtml_renderLn(g);
		addNewAlignmnet(Center, g);
		//if ((g->x > g->positionIndent) || (g->imgH)) xhtml_renderLn(g);
		//g->x = (152 / 2);
	} else if (Compare(tag, "p", 1) ||
		   Compare(tag, "div", 3)) {					// P, DIV
		UInt32	width = browserGetWidgetBounds(DRAW_WIDTH);

		if (Compare(tag, "div", 3)) isDiv = true;

		if (Compare(tag, "p", 1)) {
			g->xhtml.paragraph = true;
			//if (isDiv == true)
			//	pAlignSave = g->Align;
				
		}

		if ((g->x > g->positionIndent) || (g->imgH)) xhtml_renderLn(g);

		// g->Align = Left;
		if ((attr = getXHTMLAttrValue("align", args, numargs))) {
			if (Compare(attr, "left", 4))
				addNewAlignmnet(Left, g);
			else if (Compare(attr, "center", 6))
				addNewAlignmnet(Center, g);
			else if (Compare(attr, "right", 5))
				addNewAlignmnet(Right, g);
			else
				addNewAlignmnet(Left, g);
			
			xhtml_freeAttr(attr);
		}


		if (Compare(tag, "div", 3)) { // TODO: do for <p> too??
			if ((attr = getXHTMLAttrValue("style", args, numargs))) {
				if (StrCompareLower(attr, "text-align: left") || StrCompareLower(attr, "text-align:left"))
					addNewAlignmnet(Left, g);
				else if (StrCompareLower(attr, "text-align: center") || StrCompareLower(attr, "text-align:center"))
					addNewAlignmnet(Center, g);
				else if (StrCompareLower(attr, "text-align: right") || StrCompareLower(attr, "text-align:right"))
					addNewAlignmnet(Right, g);
				else
					addNewAlignmnet(Left, g);

				xhtml_freeAttr(attr);
			}
		}

		attr = getXHTMLAttrValue("style", args, numargs);
		if (attr) {
			Char	*background = NULL;

			background = extractBackground(attr);
			if (background != NULL) {
				addNewBackground(background, g);
				AreaSetWidth((g->xhtml.background)?g->xhtml.background->areaPtr:NULL, width, g);
			} else {
				addNewBackground(NULL, g);
			}

			xhtml_freeAttr(attr);
		} else {
			addNewBackground(NULL, g);
		}

		attr = getXHTMLAttrValue("src", args, numargs);
		if (attr) {
			imgWidth = 0;
			imgHeight = 0;
			ImageCreateImage(attr, &imgWidth,&imgHeight,false,NULL,g);
			xhtml_freeAttr(attr);
		}

		palmRegular(true);

		//g->Render = true;
	} else if (Compare(tag, "wml:go", 6) || Compare(tag, "go", 2)) {	// WML:GO
		//isLink = true;

		attr = getXHTMLAttrValue("method", args, numargs);
		if (attr){
			if (Compare(attr, "post", 4)) {
		       		g->post = true;
				if (g->onEvent == ONPICK)
					palmWmlSelectOptionPost(true, g);
				else
		        		HyperlinkSetPost(true, g);
		        	postStart = true;
			} else {
				g->post = false;
			}
			xhtml_freeAttr(attr);
		} else {
			g->post = false;
		}

		attr = getXHTMLAttrValue("href", args, numargs);
		if (attr){
			switch (g->onEvent) {
				case ONTIMER:
					if (g->xhtml.ontimerUrl)
						Free(g->xhtml.ontimerUrl);
					g->xhtml.ontimerUrl = Malloc(StrLen(attr)+1);
					ErrFatalDisplayIf (!g->xhtml.ontimerUrl, "Malloc Failed");
					StrCopy(g->xhtml.ontimerUrl,attr);
					g->onEvent = NOEVENT;
					break;
				case ONENTERFORWARD:
				case ONENTERBACKWARD:
					if (g->onforward) Free(g->onforward);
					g->onforward = Malloc(StrLen(attr)+1);
						ErrFatalDisplayIf (!g->onforward, "Malloc Failed");
                			StrCopy(g->onforward, attr);
					break;
				case ONPICK:
					palmWmlSelectOptionOnPick("", attr, g);
					isOnpickLink = true;
					break;
				case IGNORE:
					break;
				case NOEVENT:
				default:
					if (onevent) break;
					HyperlinkSetLink(false, "", attr, NULL, g);
					isLink = true;
					break;
			}
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "wml:postfield", 13) ||
		   Compare(tag, "postfield", 9)) {				// WML:POSTFIELD
		if (g->hasEvent) {
			if (g->post) {
				Char *temp;

				UInt32 buflen = (g->postBuf != NULL) ? StrLen(g->postBuf) : 0;

				isOnforwardPost = true;

				if (g->postBuf) AddToBuf(&g->postBuf, &buflen, "&", StrLen("&"));
				else { g->postBuf = Malloc(1024); }

				//name = expandVar(getXHTMLAttrValue("name", args, numargs),g);
				temp = getXHTMLAttrValue("name", args, numargs);
				name = expandVar(temp,g);
				AppendEncoded(&g->postBuf, &buflen, name);
				Free(name);
				xhtml_freeAttr(temp);

				AddToBuf(&g->postBuf, &buflen, "=", StrLen("="));

				//val = expandVar(getXHTMLAttrValue("value", args, numargs),g);
				temp = getXHTMLAttrValue("value", args, numargs);
				val = expandVar(temp,g);
				AppendEncoded(&g->postBuf, &buflen, val);
				Free(val);
				xhtml_freeAttr(temp);
			} else {
				Char *temp;

				UInt32 buflen = StrLen(g->onforward);

				AddToBuf(&g->onforward, &buflen, (!StrChr(g->onforward, '?')) ? "?" : "&", 
					StrLen((!StrChr(g->onforward, '?')) ? "?" : "&"));

				//name = expandVar(getXHTMLAttrValue("name", args, numargs),g);
				temp = getXHTMLAttrValue("name", args, numargs);
				name = expandVar(temp,g);
				AppendEncoded(&g->onforward, &buflen, name);
				Free(name);
				xhtml_freeAttr(temp);

				AddToBuf(&g->onforward, &buflen, "=", StrLen("="));

				//val = expandVar(getXHTMLAttrValue("value", args, numargs),g);
				temp = getXHTMLAttrValue("value", args, numargs);
				val = expandVar(temp,g);
				AppendEncoded(&g->onforward, &buflen, val);
				Free(val);
				xhtml_freeAttr(temp);
			}
                } else if ((isLink == true) && !g->hasEvent) { 
			name = getXHTMLAttrValue("name", args, numargs);
                    	if (name) {
                        	val = getXHTMLAttrValue("value", args, numargs);
                        	hyperlinkAddPostVar(name,g);
                        	hyperlinkSetPostVar(name,val,g);
				xhtml_freeAttr(name);
				xhtml_freeAttr(val);
                    	}
		} else if ((isOnpickLink == true) && !g->hasEvent) {
			name = getXHTMLAttrValue("name", args, numargs);
                    	if (name) {
                        	val = getXHTMLAttrValue("value", args, numargs);
                        	optionAddPostVar(name,g);
                        	optionSetPostVar(name,val,g);
				xhtml_freeAttr(name);
				xhtml_freeAttr(val);
                    	}
		}
	} else if (Compare(tag, "a", 1)) {					// A
		Char	*id = NULL;
		Char	*href = NULL;
		Char	*loc = NULL;

		href = getXHTMLAttrValue("href", args, numargs);
		name = getXHTMLAttrValue("name", args, numargs);
		id   = getXHTMLAttrValue("id", args, numargs);

		if (name) {
			loc = name;
		} else if (id) {
			loc = id;
		}

		if (loc)
			addPageLocation(loc, g);
		HyperlinkSetLink(true, "", href, loc, g);

		if (href)
			isLink = true;

		if (href) xhtml_freeAttr(href);
		if (name) xhtml_freeAttr(name);
		if (id)   xhtml_freeAttr(id);

		/*if ((attr = getXHTMLAttrValue("id", args, numargs))) {
			addPageLocation(attr, g);
			//addHyperLinkName(attr, g);
			nameTemp = getXHTMLAttrValue("href", args, numargs);
			if (nameTemp) {
				xhtml_freeAttr(nameTemp);
			} else {
				addHyperLinkName(attr, g);
			}
			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("name", args, numargs))) {
			//hyperLink(true,"#",attr,g);
			addPageLocation(attr, g);
			nameTemp = getXHTMLAttrValue("href", args, numargs);
			if (nameTemp) {
				xhtml_freeAttr(nameTemp);
			} else {
				addHyperLinkName(attr, g);
			}
			xhtml_freeAttr(attr);
			//isLink = true;
		}

		if ((attr = getXHTMLAttrValue("href", args, numargs))) {
			hyperLink(true,"",attr,g);
			xhtml_freeAttr(attr);
			isLink = true;
		}*/
	} else if (Compare(tag, "wml:anchor", 10) ||
		   Compare(tag, "anchor", 6)) {					// WML:ANCHOR
		isLink = true;
		isAnchor = true;
				
		HyperlinkSetLink(true, NULL, NULL, NULL, g);
	} else if (Compare(tag, "dl", 2)) {					// DL
		definitions = true;
		if (g->x != g->positionIndent) xhtml_renderLn(g);
		//addNewAlignmnet(Left, g);
	} else if (Compare(tag, "dt", 2)) {					// DT
		if (g->x != g->positionIndent) xhtml_renderLn(g);
	} else if (Compare(tag, "dd", 2)) {					// DD
		if (definitions == true) {
			g->indent = 1;
			g->rightIndent = 0;
			if (g->x != g->positionIndent) xhtml_renderLn(g);
			//if (g->Align != Right && g->Align != Center)
				g->x = g->indent * BrowserIndentValue();
		}
	} else if (Compare(tag, "table", 5)) {					// TABLE
		//if (g->x != g->positionIndent) xhtml_renderLn(g);
		tableCount++;
		if (tableCount == 1) {
			if (g->x != g->positionIndent) xhtml_renderLn(g);
			addNewAlignmnet(Left, g);
			tableColor = WinSetTextColor(UIColorGetTableEntryIndex(UIFieldText));
		} 

		if ((attr = getXHTMLAttrValue("width", args, numargs))) {
			width = xhtmlReadWidth(attr);

			if ((width > browserGetWidgetBounds(DRAW_WIDTH)) && (g_prefs.renderMode == RENDER_WIDE_SCREEN)) {
				g->DrawWidth = width;
				BrowserRefreshScrollBarHorizontal(g);
			}

			xhtml_freeAttr(attr);
		}

		// temporary
		cellspacing = 5;
		if ((attr = getXHTMLAttrValue("cellspacing", args, numargs))) {
			cellspacing = StrAToI(attr);
			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("bgcolor", args, numargs))) {
			addNewBackground(attr, g);
			AreaSetWidth((g->xhtml.background)?g->xhtml.background->areaPtr:NULL, width, g);
			xhtml_freeAttr(attr);
		} else {
			addNewBackground(NULL, g);
		}
		if ((attr = getXHTMLAttrValue("style", args, numargs))) {
			setBackgroundColorFromStyle(attr);
			setFontColorFromStyle(attr);
			AreaSetWidth((g->xhtml.background)?g->xhtml.background->areaPtr:NULL, width, g);

			xhtml_freeAttr(attr);			
		}
	} /*else if (Compare(tag, "tbody", 5)) {				// TBODY
		
	}*/ else if (Compare(tag, "tr", 2) ||
		   Compare(tag, "td", 2)) {					// TR, TD
		if (Compare(tag, "tr", 2)) if (g->x > g->positionIndent) xhtml_renderLn(g);

		if ((attr = getXHTMLAttrValue("width", args, numargs))) {
			width = xhtmlReadWidth(attr);

			if ((width > browserGetWidgetBounds(DRAW_WIDTH)) && (g_prefs.renderMode == RENDER_WIDE_SCREEN)) {
				g->DrawWidth = width;
				BrowserRefreshScrollBarHorizontal(g);
			}

			xhtml_freeAttr(attr);
		}

		if ((attr = getXHTMLAttrValue("bgcolor", args, numargs))) {
			addNewBackground(attr, g);
			AreaSetWidth((g->xhtml.background)?g->xhtml.background->areaPtr:NULL, width, g);
			xhtml_freeAttr(attr);
		} else {
			addNewBackground(NULL, g);
		}

		palmCurrent(true);

		if ((attr = getXHTMLAttrValue("style", args, numargs))) {
			setBackgroundColorFromStyle(attr);
			setFontColorFromStyle(attr);
			AreaSetWidth((g->xhtml.background)?g->xhtml.background->areaPtr:NULL, width, g);

			xhtml_freeAttr(attr);			
		}
	} else if (Compare(tag, "th", 2)) {					// TH
		
	} /*else if (Compare(tag, "td", 2)) {					// TD
		if ((attr = getXHTMLAttrValue("bgcolor", args, numargs))) {
			//RGBColorType 		rgb;

			//rgb = ConvertColorNumberString(attr, TYPE_PAGE);
	
    			//g->xhtml.pageBG = WinRGBToIndex( &rgb );

			//xhtml_ReSetWindow(g);

			//hasBG = true;

			addNewBackground(attr, g);
			xhtml_freeAttr(attr);
		} else {
			addNewBackground(NULL, g);
		}

		palmCurrent(true);
	}*/ else if (Compare(tag, "ul", 2) ||
		   Compare(tag, "nl", 2)) {					// UL

		g->indent += 1;
		g->rightIndent = 0;

		//if (g->x != g->positionIndent) xhtml_renderLn(g);
		//addNewAlignmnet(Left, g);

		addNewULList(g);
	} else if (Compare(tag, "ol", 2)) {					// OL
		Int16 olCount = 0;
		Int16 olStart = 0;
		Int16 olType = 0;

		olType = NUMERIC;

		g->indent += 1;
		g->rightIndent = 0;

		//if (g->x != g->positionIndent) xhtml_renderLn(g);
		//addNewAlignmnet(Left, g);

		if ((attr = getXHTMLAttrValue("style", args, numargs))) {
			if (StrCompareLower(attr, "list-style-type: lower-alpha") || StrCompareLower(attr, "list-style-type:lower-alpha")) {
				olType = LWR_ALPHA;
				olCount = 0x60;
			} else if (StrCompareLower(attr, "list-style-type: upper-alpha") || StrCompareLower(attr, "list-style-type:upper-alpha")) {
				olType = UPPR_ALPHA;
				olCount = 0x40;
			} else if (StrCompareLower(attr, "list-style-type: roman") || StrCompareLower(attr, "list-style-type:roman")) {
				olType = ROMAN;
			}
			xhtml_freeAttr(attr);
		}


		if ((attr = getXHTMLAttrValue("start", args, numargs))) {
			olCount += StrAToI (attr) - 1;
			olStart = olCount;
			xhtml_freeAttr(attr);
		}

		addNewOLList(olCount, olStart, olType, g);
	} else if (Compare(tag, "li", 2)) {					// LI
		Int16 			olCount = 0;
		Int16 			olStart = 0;
		Int16 			olType = 0;
		Boolean			isOL = false;
		Boolean			isUL = false;
		UInt16			fixedX = g->xindent + (g->indent * BrowserIndentValue()) + g->positionIndent;

		if (g->organizedList == NULL)
			return 0;

		olCount = g->organizedList->olCount;
		olStart = g->organizedList->olStart;
		olType  = g->organizedList->olType;
		isOL    = g->organizedList->isOL;
		isUL    = g->organizedList->isUL;

		if (isUL == true) {
			BitmapPtr 	resP;
			ImagePtr	img = NULL;

			//g->Render = true;
			if (g->x != fixedX) xhtml_renderLn(g);
			else if (g->x == 0) g->x = fixedX;

    			resP = DOMResourceToPtr(bitmapRsc, bmpLiDot);
			img = addPageImage(NULL, NULL, fixedX - (BrowserIndentValue()-1), fixedX - (BrowserIndentValue()-1) + 7, g->y, g->y + 12, 7, 12, resP, g);
			if (img)
				img->alreadyHires = true;

    			updateScreen(g);
		} else if (isOL == true) {
			
			//g->Render = true;
			if (g->x != fixedX) xhtml_renderLn(g);
			else if (g->x == 0 && olCount == 0) g->x = fixedX;
			else if ((olCount == olStart) && 
			         (g->x != fixedX)) g->x = fixedX;

			g->organizedList->olCount++;
			olCount++;

			switch (olType) {
				case LWR_ALPHA:
				case UPPR_ALPHA:
					{
						Char	mark[4];
						UInt16	charWidth;

						mark[0] = olCount;
						mark[1] = '.';
						mark[2] = '\0';
						charWidth = FntCharsWidth(mark, StrLen(mark));
						TextAddText(mark, StrLen(mark), fixedX - (BrowserIndentValue()-1), g->y, charWidth, FntCharHeight(), g);
					}
					break;
				case ROMAN:
					// break;
				case NUMERIC:
				default:
					{
						Char	mark[4];
						UInt16	charWidth;

						StrPrintF(mark, "%i.", olCount);
						charWidth = FntCharsWidth(mark, StrLen(mark));
						TextAddText(mark, StrLen(mark), fixedX - (BrowserIndentValue()-1), g->y, charWidth, FntCharHeight(), g);
					}
					break;
			}

    			updateScreen(g);
		}

		if ((attr = getXHTMLAttrValue("href", args, numargs))) {
			isLink = true;
			HyperlinkSetLink(true, "", attr, NULL, g);
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "wml:getvar", 10)) {				// WML:GETVAR
		Char *var = NULL;
		if((attr = getXHTMLAttrValue("name", args, numargs))) {
			var = getVar(attr,g);
			xhtml_freeAttr(attr);

			if((attr = getXHTMLAttrValue("conversion", args, numargs))) {
				Char *newvar = NULL;

				if (Compare(attr, "escape", 6)) {
					newvar = escaped(var);

					xhtml_renderString(&(g->xhtml),newvar,g);
					if (newvar) Free(newvar);
				} else if (Compare(attr, "unesc", 5)) {
					newvar =unescaped(var);

					xhtml_renderString(&(g->xhtml),newvar,g);
					if (newvar) Free(newvar);
				}

				xhtml_freeAttr(attr);
			} else
				xhtml_renderString(&(g->xhtml),var,g);
		}
	} else if (Compare(tag, "wml:setvar", 10) ||
		   Compare(tag, "setvar", 6)) {					// WML:SETVAR
                if (g->hasEvent) {
			name = getXHTMLAttrValue("name", args, numargs);
                    	if (name) {
                        	val = getXHTMLAttrValue("value", args, numargs);
                        	addVar(name,g);
                        	setVar(name,val,g);
				xhtml_freeAttr(name);
                    	}
                } else if (((isLink == true) || (isRefresh == true)) && !g->hasEvent) { 	//(g->onEvent == 0) && (onevent == false)) {
                        name = getXHTMLAttrValue("name", args, numargs);
                    	if (name) {
                        	val = getXHTMLAttrValue("value", args, numargs);
                        	hyperlinkAddVar(name,g);
                        	hyperlinkSetVar(name,val,g);
				xhtml_freeAttr(name);
                    	}
		} else if ((isOnpickLink == true) && !g->hasEvent) { 	//(g->onEvent == 0) && (onevent == false)) {
			name = getXHTMLAttrValue("name", args, numargs);
                    	if (name) {
                        	val = getXHTMLAttrValue("value", args, numargs);
                        	optionAddVar(name,g);
                        	optionSetVar(name,val,g);
				xhtml_freeAttr(name);
                    	}
		}
	} else if (Compare(tag, "form", 4)) {					// FORM
		char count[10];
		StrPrintF(count, "%d", formCount++);
		

		if (g->x != g->positionIndent) xhtml_renderLn(g);
		
		name = getXHTMLAttrValue("name", args, numargs);
		attr = getXHTMLAttrValue("method", args, numargs);
		action = getXHTMLAttrValue("action", args, numargs);
		if (attr) {
			if (Compare(attr, "get", 3)) {
				if (action) {
					/*Char	*ques = NULL;

					ques = StrChr(attr, '?');
					if (ques != NULL)
						*ques = '\0';*/

					formCreateForm(GET, action, (name) ? name : count, g);
				} else {
					formCreateForm(GET, "about:blank", (name) ? name : count, g);
				}
			} else if (Compare(attr, "post", 4)) {
				if (action) {
					formCreateForm(POST, action, (name) ? name : count, g);
				} else {
					formCreateForm(POST, "about:blank", (name) ? name : count, g);
				}
			} else {
				if (action) {
					formCreateForm(GET, action, (name) ? name : count, g);
				} else {
					formCreateForm(GET, "about:blank", (name) ? name : count, g);
				}
			}
		} else { // no method
			if (action) {
				formCreateForm(GET, action, (name) ? name : count, g);
			} else {
				formCreateForm(GET, "about:blank", (name) ? name : count, g);
			}
		}
		if (name) xhtml_freeAttr(name);
		if (attr) xhtml_freeAttr(attr);
		if (action) xhtml_freeAttr(action);
	} else if (Compare(tag, "label", 5)) {					// LABEL
		
	} else if (Compare(tag, "textarea", 8)) {				// TEXTAREA
		UInt16	areaWidth = (browserGetWidgetBounds(WIDGET_WIDTH) - 1);
		Char	*background = NULL, *bkgrdValue = NULL;
		Char	*border = NULL, *borderValue = NULL;
		
		//TODO: Account for border width in measuring the width of the area

		if ((attr = getXHTMLAttrValue("disabled", args, numargs))) {
			if (Compare(attr, "disabled", 8))
				disabled = true;
			xhtml_freeAttr(attr);
		}

		rows       = getXHTMLAttrValue("rows", args, numargs);
		cols       = getXHTMLAttrValue("cols", args, numargs);
		name       = getXHTMLAttrValue("name", args, numargs);
		background = getXHTMLAttrValue("style", args, numargs);
		border	   = getXHTMLAttrValue("style", args, numargs);

		if (cols != NULL)
			areaWidth = StrAToI(cols) * BrowserColumnValue();

		if (areaWidth > (browserGetWidgetBounds(WIDGET_WIDTH) - 1))
			areaWidth = 32 * BrowserColumnValue();

		if ((g->x + areaWidth) > (browserGetWidgetBounds(WIDGET_WIDTH) - 1)) xhtml_renderLn(g);

		if (background) {
			bkgrdValue = extractBackground(background);
		}
		if (border) {
			borderValue = extractCSSElement("border:",border);
		}

		if (name) {
			if (topTag == TT_HTML) {
				formName = formCreateVarName(name,g);
				//wmlInput(formName,disabled,g);
				formCreateInputArea(formName, disabled, (cols != NULL) ? StrAToI(cols) : 32, (rows != NULL) ? StrAToI(rows) : 3, borderValue, bkgrdValue, g);
				Free(formName);
			} else { // really will not happen...
				//wmlInput(name,disabled,g);
				formCreateInputArea(name, disabled, (cols != NULL) ? StrAToI(cols) : 32, (rows != NULL) ? StrAToI(rows) : 3, borderValue, bkgrdValue, g);
			}
		}

		if (rows)       xhtml_freeAttr(rows);
		if (cols)       xhtml_freeAttr(cols);
		if (name)       xhtml_freeAttr(name);
		if (background) xhtml_freeAttr(background);
		if (border)     xhtml_freeAttr(border);

		isTextarea = true;

		g->Render = false;
	} else if (Compare(tag, "fieldset", 8)) {				// FIELDSET
		starttag("hr", args, numargs);
	} else if (Compare(tag, "button", 6)) {					// BUTTON
// TODO: cleanup

		Int16		width = 0, height = 0;
		Char		*title = NULL, *src = NULL;
		Boolean		img = false;

		value	= getXHTMLAttrValue("value", args, numargs);
		title	= getXHTMLAttrValue("title", args, numargs);
		name	= getXHTMLAttrValue("name", args, numargs);
		attr	= getXHTMLAttrValue("type", args, numargs);
		src	= getXHTMLAttrValue("src", args, numargs);
		

		if ((attr = getXHTMLAttrValue("disabled", args, numargs))) {
			if (Compare(attr, "disabled", 8))
				disabled = true;
			xhtml_freeAttr(attr);
		}

		if (value) {
			width = formDrawButton(value, &g->x, &g->y, false, g);

			if (Compare(attr, "submit", 6)) {
				if (src) {
					if (!ImageCreateImage(src,&width,&height,false,NULL,g)) {
						width = formDrawButton("Submit Query", &g->x, &g->y, false, g);
						height = 11 * HiresFactor();
					} else {
						img = true;
					}

					formCreateButton((name) ? name : "Submit", value, SUBMIT, width, height, disabled, img, g);
				} else
					formCreateButton((name) ? name : "Submit", value, SUBMIT, width, 11, disabled, img, g);
			} else if (Compare(attr, "reset", 5)) {
				if (src) {
					if (!ImageCreateImage(src,&width,&height,false,NULL,g)) {
						width = formDrawButton("Reset", &g->x, &g->y, false, g);
						height = 11 * HiresFactor();
					} else {
						img = true;
					}

					formCreateButton((name) ? name : "Reset", value, RESET, width, height, disabled, img, g);
				} else
					formCreateButton((name) ? name : "Reset", value, RESET, width, 11, disabled, img, g);
			}

			if (name) formAddValue(name, value, BUTTON, g);
		} else if (title) {
			width = formDrawButton(value, &g->x, &g->y, false, g);

			if (Compare(attr, "submit", 6)) {
				if (src) {
					if (!ImageCreateImage(src,&width,&height,false,NULL,g)) {
						width = formDrawButton("Submit Query", &g->x, &g->y, false, g);
						height = 11 * HiresFactor();
					} else {
						img = true;
					}

					formCreateButton((name) ? name : "Submit", value, SUBMIT, width, height, disabled, img, g);
				} else
					formCreateButton((name) ? name : "Submit", value, SUBMIT, width, 11, disabled, img, g);
			} else if (Compare(attr, "reset", 5)) {
				if (src) {
					if (!ImageCreateImage(src,&width,&height,false,NULL,g)) {
						width = formDrawButton("Reset", &g->x, &g->y, false, g);
						height = 11 * HiresFactor();
					} else {
						img = true;
					}

					formCreateButton((name) ? name : "Reset", value, RESET, width, height, disabled, img, g);
				} else
					formCreateButton((name) ? name : "Reset", value, RESET, width, 11, disabled, img, g);
			}

			formAddValue(name, value, BUTTON, g);
		} else {
			attr = getXHTMLAttrValue("type", args, numargs);

			if (Compare(attr, "submit", 6)) {
				if (src) {
					if (!ImageCreateImage(src,&width,&height,false,NULL,g)) {
						width = formDrawButton("Submit Query", &g->x, &g->y, false, g);
						height = 11 * HiresFactor();
					} else {
						img = true;
					}

					formCreateButton("Submit", "true", SUBMIT, width, height, disabled, img,  g);
				} else {
					width = formDrawButton("Submit Query", &g->x, &g->y, false, g);
					formCreateButton("Submit", "true", SUBMIT, width, 11, disabled, img, g);
				}
			} else if (Compare(attr, "reset", 5)) {
				if (src) {
					if (!ImageCreateImage(src,&width,&height,false,NULL,g)) {
						width = formDrawButton("Reset", &g->x, &g->y, false, g);
						height = 11 * HiresFactor();
					} else {
						img = true;
					}

					formCreateButton("Reset", "true", RESET, width, height, disabled, img, g);
				} else {
					width = formDrawButton("Reset", &g->x, &g->y, false, g);
					formCreateButton("Reset", "true", RESET, width, 11, disabled, img, g);
				}
			}
		}

		if (name)	xhtml_freeAttr(name);
		if (value)	xhtml_freeAttr(value);
		if (attr)	xhtml_freeAttr(attr);
		if (title)	xhtml_freeAttr(title);
		if (src)	xhtml_freeAttr(src);

		g->Render = false;
	} else if (Compare(tag, "input", 5)) {					// INPUT
		Boolean		img = false;
		Char		*maxLengthP, *sizeP, *readonly;
		Int16		size = -1;
		Int32		maxLength = -1;

		if ((attr = getXHTMLAttrValue("disabled", args, numargs))) {
			if (Compare(attr, "disabled", 8))
				disabled = true;
			xhtml_freeAttr(attr);
		}

		type    	= getXHTMLAttrValue("type", args, numargs);
		name    	= getXHTMLAttrValue("name", args, numargs);
		value   	= getXHTMLAttrValue("value", args, numargs);
		checked 	= getXHTMLAttrValue("checked", args, numargs);
		src     	= getXHTMLAttrValue("src", args, numargs);
		title   	= getXHTMLAttrValue("title", args, numargs);
		maxLengthP 	= getXHTMLAttrValue("maxlength", args, numargs);
		sizeP		= getXHTMLAttrValue("size", args, numargs);
		readonly	= getXHTMLAttrValue("readonly", args, numargs);
		
		if (sizeP) {
			size = (Int16)StrAToI(sizeP);
		}
		if (maxLengthP) {
			maxLength = StrAToI(maxLengthP);
		}
		
_input_top:
		if (type) {
			if (Compare(type, "text", 4) || Compare(type, "password", 8)) {
				if (name){
					if (topTag == TT_HTML) {
						formName = formCreateVarName(name,g);
						wmlInput(formName, disabled, size, maxLength, g);

						if (Compare(type, "password", 8)) {
							palmWmlInputType(true, g);
							FormInputSetType(g->Input, inputTypePassword);
						} else {
							palmWmlInputType(false, g);
						}

						Free(formName);
					} else {
						wmlInput(name, disabled, size, maxLength, g);
					}

					if (value){
						if (g->Input->name) {
							if (InputgetVar(g->Input->name, g) != NULL) {
								wmlInputValue(getVar(g->Input->name,g),g);
							} else {
	                					wmlInputValue(value,g);
							}
						} else {
	                				wmlInputValue(value,g);
						}
					} else {
						if (name) {
							if (topTag == TT_HTML) {
								formName = formCreateVarName(name,g);
								wmlInputValue(getVar(formName,g),g);
								Free(formName);
							} else {
								wmlInputValue(getVar(name,g),g);
							}
						}
					}

					if (topTag == TT_HTML) {
						formAddValue(name, (value) ? value : "", TEXT, g);
					}

					if (readonly) {
						FormInputSetReadonly(g->Input);
					}
				}
			} else if (Compare(type, "checkbox", 8)) {
				/*if ((attr = getXHTMLAttrValue("checked", args, numargs))) {
					if (Compare(attr, "checked", 7))
						formDrawCheckedCheckbox(g);
					else
						formDrawUnCheckedCheckbox(g);

					xhtml_freeAttr(attr);
				} else {
					formDrawUnCheckedCheckbox(g);
				}*/
				if (name) {
					if (checked)
						formCreateCheckBox(name, (value)?value:"on", ((Compare(checked, "checked", 7)) ? true : false), disabled, g);
					else
						formCreateCheckBox(name, (value)?value:"on", false, disabled, g);
	
					formAddValue(name, ((checked && Compare(checked, "checked", 7)) ? ((value)?value:"on") : NULL), CHECK, g);
				}
			} else if (Compare(type, "radio", 5)) {
				/*if ((attr = getXHTMLAttrValue("checked", args, numargs))) {
					if (Compare(attr, "checked", 7))
						formDrawFullRadio(g);
					else
						formDrawEmptyRadio(g);

					xhtml_freeAttr(attr);
				} else {
					formDrawEmptyRadio(g);
				}*/
				if (name) {
					if (checked)
						formCreateRadioButton(name, (value)?value:"on", ((Compare(checked, "checked", 7)) ? true : false), disabled, g);
					else
						formCreateRadioButton(name, (value)?value:"on", false, disabled, g);

					formAddValue(name, (value)?value:"on", RADIO, g);
				}
			} else if (Compare(type, "submit", 6)) {
				Int16 width = 0, height = 0;

				if (src) {
					if (!ImageCreateImage(src,&width,&height,false,NULL,g)) {
						width = formDrawButton("Submit Query", &g->x, &g->y, false, g);
						height = 11 * HiresFactor();
					} else {
						img = true;
					}

					formCreateButton((name) ? name : "Submit", (value) ? value : "true", SUBMIT, width, height, disabled, img, g);
					if (name) formAddValue(name, (value) ? value : "true", BUTTON, g);
				} else {
					if (value) {
						width = formDrawButton(value, &g->x, &g->y, false, g);
						formCreateButton(name, value, SUBMIT, width, 11 * HiresFactor(), disabled, false, g);
						formAddValue(name, value, BUTTON, g);
					} else if (title) {
						width = formDrawButton(title, &g->x, &g->y, false, g);
						formCreateButton(name, title, SUBMIT, width, 11 * HiresFactor(), disabled, false, g);
						formAddValue(name, title, BUTTON, g);
					} else {
						width = formDrawButton("Submit Query", &g->x, &g->y, false, g);
						formCreateButton("Submit", "Submit Query", SUBMIT, width, 11 * HiresFactor(), disabled, false, g);
					}
				}
			} else if (Compare(type, "reset", 5)) {
				Int16 width = 0, height = 0;

				if (src) {
					if (!ImageCreateImage(src,&width,&height,false,NULL,g)) {
						width = formDrawButton("Reset", &g->x, &g->y, false, g);
						height = 11  * HiresFactor();
					} else {
						img = true;
					}

					formCreateButton((name) ? name : "Reset", (value) ? value : "true", RESET, width, height, disabled, img, g);
					if (name) formAddValue(name, (value) ? value : "Reset", BUTTON, g);
				} else {
					if (value) {
						width = formDrawButton(value, &g->x, &g->y, false, g);
						formCreateButton(name, value, RESET, width, 11 * HiresFactor(), disabled, false, g);
						formAddValue(name, value, BUTTON, g);
					} else if (title) {
						width = formDrawButton(title, &g->x, &g->y, false, g);
						formCreateButton(name, title, RESET, width, 11 * HiresFactor(), disabled, false, g);
						formAddValue(name, title, BUTTON, g);
					} else {
						width = formDrawButton("Reset", &g->x, &g->y, false, g);
						formCreateButton("Reset", "Reset", RESET, width, 11 * HiresFactor(), disabled, false, g);
					}
				}
			} else if (Compare(type, "image", 5)) {
				Int16 width = 0, height = 0;

				if (src) {
					if (!ImageCreateImage(src,&width,&height,false,NULL,g)) {
						width = formDrawButton("Submit Query", &g->x, &g->y, false, g);
						height = 11 * HiresFactor();
					} else {
						img = true;
					}

					formCreateButton((name) ? name : "Submit", (value) ? value : "true", SUBMIT, width, height, disabled, img, g);
					if (name) formAddValue(name, (value) ? value : "Submit", BUTTON, g);
				}
			} else if (Compare(type, "hidden", 6)) {
				formAddValue(name, value, HIDDEN, g);
			}
		} else {
			type = Malloc(5);
			if (type) {
				StrCopy(type, "text");
				goto _input_top;
			}
		}

		if (name)    	xhtml_freeAttr(name);
		if (value)   	xhtml_freeAttr(value);
		if (type)    	xhtml_freeAttr(type);
		if (checked) 	xhtml_freeAttr(checked);
		if (src)     	xhtml_freeAttr(src);
		if (title)   	xhtml_freeAttr(title);
		if (maxLengthP) xhtml_freeAttr(maxLengthP);
		if (sizeP)   	xhtml_freeAttr(sizeP);
		if (readonly)	xhtml_freeAttr(readonly);
	} else if (Compare(tag, "select", 6)) {					// SELECT

		if ((attr = getXHTMLAttrValue("disabled", args, numargs))) {
			if (Compare(attr, "disabled", 8))
				disabled = true;
			xhtml_freeAttr(attr);
		}

		name	= getXHTMLAttrValue("name", args, numargs);
		value	= getXHTMLAttrValue("value", args, numargs);

		g->Render = false;
		isSelect = true;
		if (name) {
			if (topTag == TT_HTML) {
				formName = formCreateVarName(name,g);
				wmlSelect(formName,disabled,g);
				Free(formName);
			} else {
				wmlSelect(name,disabled,g);
			}
		} else {
			if (topTag == TT_HTML) {
				formName = formCreateVarName("na",g);
				wmlSelect(formName,disabled,g);
				Free(formName);
			} else {
				wmlSelect("na",disabled,g);
			}
			// This is not a good way to do this. Some sites
			// such as www.nytimes.com/wap use only <select>. We need to handle it
			// but if we get more than one titleless select we might be in trouble.
		}
		//if ( (attr = getXHTMLAttrValue("ivalue", args, numargs))) { // I believe that this is incorrect
	        //       	wmlSelectValue(attr,g);
		//		xhtml_freeAttr(attr);
		//}
		if (value){
	                if (g->Select->name) {
				if (InputgetVar(g->Select->name, g) != NULL) {
					wmlSelectValue(getVar(g->Select->name,g),g);
				} else {
	                		wmlSelectValue(value,g);
				}
			} else {
	                	wmlSelectValue(value,g);
			}
		} /*else { // why do I have this here????
			if ((attr = getXHTMLAttrValue("name", args, numargs))) {
				wmlSelectValue(getVar(attr,g),g);
				xhtml_freeAttr(attr);
			}
		}*/

		if (topTag == TT_HTML) {
			formAddValue((name) ? name : "na", (value) ? value : "", SELECT, g);
		}

		if (name)	xhtml_freeAttr(name);
		if (value)	xhtml_freeAttr(value);
	} else if (Compare(tag, "optgroup", 8)) {				// OPTGROUP
		
	} else if (Compare(tag, "option", 6)) {					// OPTION
		g->Render = false;
		isOption = true;

		if ((attr = getXHTMLAttrValue("value", args, numargs)) /*&& StrLen(attr)*/) {
	                wmlSelectOption(attr,g);
			xhtml_freeAttr(attr);
		} else {
			wmlSelectOption("na",g);
			if (attr)
				xhtml_freeAttr(attr);
		}
		if ((attr = getXHTMLAttrValue("onpick", args, numargs))){
	                //wmlSelectOption(attr,g);
	                palmWmlSelectOptionOnPick("", attr, g);
			xhtml_freeAttr(attr);
		}
		/*if ((attr = getXHTMLAttrValue("title", args, numargs))) { // A hack for WML iConnect Channels
			wmlSelectOptionTxt(attr,g);
			xhtml_freeAttr(attr);
		}*/
		if ((attr = getXHTMLAttrValue("selected", args, numargs))) {
			if (Compare(attr, "selected", 8))
				optIsSel = true;
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "object", 6)) {					// OBJECT
		imgWidth = 0;
		imgHeight = 0;
		
		attr = getXHTMLAttrValue("type", args, numargs);
		data = getXHTMLAttrValue("data", args, numargs);

		if (attr) {
			if (CompareJustTxt(attr, "image", 5)) {
				if (data) {
					if (!ImageCreateImage(data, &imgWidth, &imgHeight, false, NULL, g)) {
						ImageCreateEmptyImage(NULL, &g->x, &g->y, &imgWidth, &imgHeight, false, g);  // for html standards, I probably should not do this, oh well
					}
				}
			} else {
				MediaCreateEmbededLink(data, attr, g);
			}
		}

		if (attr) xhtml_freeAttr(attr);
		if (data) xhtml_freeAttr(data);
	} else if (Compare(tag, "embed", 5)) {					// EMBED
		imgWidth = 0;
		imgHeight = 0;

		attr = getXHTMLAttrValue("type", args, numargs);
		data = getXHTMLAttrValue("src", args, numargs);

		if (attr) {
			if (CompareJustTxt(attr, "image", 5)) {
				if (data) {
					if (!ImageCreateImage(data, &imgWidth, &imgHeight, false, NULL, g)) {
						ImageCreateEmptyImage(NULL, &g->x, &g->y, &imgWidth, &imgHeight, false, g);  // for html standards, I probably should not do this, oh well
					}
				}
			} else {
				MediaCreateEmbededLink(data, attr, g);
			}
		}

		if (attr) xhtml_freeAttr(attr);
		if (data) xhtml_freeAttr(data);
	} else if (Compare(tag, "font", 4)) {					// FONT
		Boolean		fchanged = false;

		if ((attr = getXHTMLAttrValue("size", args, numargs))) {
			if (StrAToI(attr) <= 2)
				small(true);
			else if (StrAToI(attr) == 3)
				regular(true);
			else if (StrAToI(attr) >= 4 && StrAToI(attr) <= 5)
				bold(true);
			else if (StrAToI(attr) == 6)
				big(true);
			else if (StrAToI(attr) >= 7)
				strong(true);
			else
				regular(true);

			fchanged = true;
			xhtml_freeAttr(attr);
		}

		if (fchanged == false) {
			if (g->font != NULL)
				addNewFont(g->font->font, g->font->res, g);
			else
				palmRegular(true); // addNewFont(stdFont, NULL, g);
		}
		if ((attr = getXHTMLAttrValue("color", args, numargs))) {
			/*RGBColorType 		rgb;
			IndexedColorType	index;

			rgb = ConvertColorNumberString(attr, TYPE_ELEMENT);
	
    			index = WinRGBToIndex( &rgb );

			//savedColor = WinSetTextColor( index );

			//changedFontColor = true;

			if (g->font)
				g->font->color = index;*/

			setFontColor(attr);

			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "map", 3)) {					// MAP
		if ((attr = getXHTMLAttrValue("name", args, numargs))) {
			MapAddImageMap(attr, g);
			xhtml_freeAttr(attr);
		}
	} else if (Compare(tag, "area", 4)) {					// AREA
		Char	*areaHref = NULL;
		Char	*areaAlt = NULL;
		Char	*areaCoord = NULL;
		Char	*areaShape = NULL;
		Int16	x = 0, y = 0, w = 0, h = 0;


		areaHref = getXHTMLAttrValue("href", args, numargs);
		areaAlt = getXHTMLAttrValue("alt", args, numargs);
		areaCoord = getXHTMLAttrValue("coords", args, numargs);
		areaShape = getXHTMLAttrValue("shape", args, numargs);

		if (areaShape && !StrNCaselessCompare(areaShape, "rect", 4)) {
			if (areaCoord && areaHref) {
				MapParseCoord(areaCoord, &x, &y, &w, &h);
				MapAddArea(areaHref, (areaAlt)?areaAlt:NULL, x, y, w, h, g);
			}
		}


		if (areaHref) xhtml_freeAttr(areaHref);
		if (areaAlt) xhtml_freeAttr(areaAlt);
		if (areaCoord) xhtml_freeAttr(areaCoord);
		if (areaShape) xhtml_freeAttr(areaShape);

		/*if ((attr = getXHTMLAttrValue("href", args, numargs))) {
			Char *temp = NULL;

			temp = Malloc(3);

			if (!temp) {
				xhtml_freeAttr(attr);
				goto area_end;
			}
			StrCopy(temp, " [");

			small(true);
			xhtml_renderString(&(g->xhtml),temp,g);
			HyperlinkSetLink(true, "", attr, NULL, g);
			xhtml_freeAttr(attr);

			if ((attr = getXHTMLAttrValue("title", args, numargs))) {
				xhtml_renderString(&(g->xhtml),attr,g);
				xhtml_freeAttr(attr);
			} else if ((attr = getXHTMLAttrValue("alt", args, numargs))) {
				xhtml_renderString(&(g->xhtml),attr,g);
				xhtml_freeAttr(attr);
			} else {
				Char *untitled = NULL;
				
				untitled = Malloc(StrLen("Untitled")+1);
				if (untitled != NULL) {
					StrCopy(untitled, "Untitled");
					xhtml_renderString(&(g->xhtml),untitled,g);
					Free(untitled);
				}
			}

			HyperlinkSetLink(false, NULL, NULL, NULL, g);
			StrCopy(temp, "] ");
			xhtml_renderString(&(g->xhtml),temp,g);
			small(false);

			Free(temp);
		}
area_end:*/
	} else if (Compare(tag, "frame", 5)) {					// FRAME
		if ((attr = getXHTMLAttrValue("src", args, numargs))) {
			Char *temp = NULL;

			temp = Malloc(3);

			if (!temp) {
				xhtml_freeAttr(attr);
				goto frame_end;
			}

			StrCopy(temp, " [");

			small(true);
			xhtml_renderString(&(g->xhtml),temp,g);
			HyperlinkSetLink(true, "", attr, NULL, g);
			xhtml_freeAttr(attr);

			if ((attr = getXHTMLAttrValue("name", args, numargs))) {
				xhtml_renderString(&(g->xhtml),attr,g);
				xhtml_freeAttr(attr);
			} else {
				Char *untitled = NULL;
				
				untitled = Malloc(StrLen("Untitled Frame")+1);
				if (untitled != NULL) {
					StrCopy(untitled, "Untitled Frame");
					xhtml_renderString(&(g->xhtml),untitled,g);
					Free(untitled);
				}
			}

			HyperlinkSetLink(false, NULL, NULL, NULL, g);
			StrCopy(temp, "] ");
			xhtml_renderString(&(g->xhtml),temp,g);
			small(false);

			xhtml_renderLn(g);

			Free(temp);
		}
frame_end:
	} else if (Compare(tag, "link", 4)) {					// LINK
		if ((attr = getXHTMLAttrValue("rel", args, numargs))) {
			if (Compare(attr, "alternate", 9)) {
				xhtml_freeAttr(attr);

				if ((attr = getXHTMLAttrValue("type", args, numargs))) {
					if (Compare(attr, "application/rss+xml", 19)) {
						xhtml_freeAttr(attr);
						
						if ((attr = getXHTMLAttrValue("href", args, numargs))) {
							if (g->rssFeed != NULL) Free(g->rssFeed);
							g->rssFeed = Malloc(StrLen(attr)+1);
							if (g->rssFeed != NULL)
								StrCopy(g->rssFeed, attr);
							xhtml_freeAttr(attr);
						}
					} else {
						xhtml_freeAttr(attr);
					}
				}
			} else {
				xhtml_freeAttr(attr);
			}
		}
	} else if (Compare(tag, "sub", 3)) {					// SUB
		//isSubscript = true;
		subscript(true);
	} else if (Compare(tag, "sup", 3)) {					// SUP
		//isSuperscript = true;
		superscript(true);
	} else if (Compare(tag, "strike", 6) ||
		   Compare(tag, "s", 1)) {					// STRIKE, S
		//isStrikethrough = true;
		strike(true);
	} else if (Compare(tag, "bdo", 3)) {					// BDO
		Char	*dir = NULL;

		g->browserGlobals.textDirRTL = false;

		dir = getXHTMLAttrValue("dir", args, numargs);
		if (dir) {
			if (Compare(dir, "rtl", 3))
				g->browserGlobals.textDirRTL = true;
			xhtml_freeAttr(dir);
		}
	} else {								// default
		//g->Render = true;
	}

	if (PositionIsCatchStartTag(tag)) {
		PositionRemoveLastPosition(g);
	}

	xhtml_debugOut(tag, NULL, 0, false, "after", g);

        return 0;
}



int xhtml_new(XHTMLPtr xhtml, WebDataPtr webData, GlobalsType *g) {
	//xhtml->data = Malloc(webData->length + 1);
	//	if (!xhtml->data) return 1; // ErrFatalDisplayIf(!xhtml->data, "Malloc failed");
	//
	//StrCopy(xhtml->data, webData->data);

	xhtml->length = webData->length;

	xhtml->action = ACTION_RENDER;

	//g->wmldck.charSet = g->charSet; // 0;
	//g->charSet = g->charSet; // 0; // eventually add support
	
    	//xhtml->pageBG = UIColorGetTableEntryIndex(UIFieldBackground);
	freeBackground(g);
	addNewBackground(NULL, g);

	xhtml->ontimerUrl = NULL;
	xhtml->timerVal = -1;

	xhtml->buffer = NULL;

	isPre = false;
	definitions = false;
	isDiv = false;
	gotTitle = false;
	isScript = false;
	tableCount = 0;
	topTag = TT_NONE;
	if (!StrNCaselessCompare(g->pageContentType, "text/html", 9))
		topTag = TT_HTML;
	optIsSel = false;
	isSelect = false;
	cardCount = 0;
	formCount = 0;
	xhtml->paragraph = false;
	streamLink = false;
	isTextarea = false;
	g->Align = Left;
	g->positionIndent = 0;

	freeAlignment(g);
	freeFont(g);
	freeList(g);
	freeIndent(g);
	PositionFreePosition(g);
	AreaFreeAreas(g);

	regular(true);

	xhtml->end = false;
	xhtml->textAvail = false;

	return 0;
}

