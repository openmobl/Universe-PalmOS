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
#include	"formBrowser.h"
#include 	"../res/WAPUniverse_res.h"
#include	"Area.h"
#include	"Layout.h"
#include	"DOM.h"
#include	"variable.h"
#include	"PalmUtils.h"
#include	"Browser.h"
#include	"URL.h"
#include	"Form.h"
#include	"Tab.h"
#include	"Pointer.h"

void HyperlinkSetUrl(Char *prefix, Char *url, GlobalsType *g)
{
	UInt32		urlLen = StrLen(url) + StrLen(prefix);
	
	if (!url) {
		g->HyperLinks->url = NULL;
		return;
	}

	g->HyperLinks->url = Malloc(((urlLen > MAX_URL_LEN) ? MAX_URL_LEN:urlLen) + 1); // A bit more resourcefull?
	if (!g->HyperLinks->url) {
		g->HyperLinks->url = NULL; //ErrFatalDisplayIf (!g->HyperLinks->url, "Malloc Failed");
		return;
	}
	StrCopy(g->HyperLinks->url,prefix);
	StrCopy(g->HyperLinks->url+StrLen(prefix),url);
}

int HyperlinkAddLink(Int16 x1, Int16 y1, Char *id, GlobalsType *g)
{
	HyperLinkPtr tmp;

	tmp = Malloc(sizeof(HyperLinkType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	MemSet(tmp, sizeof(HyperLinkType), 0);
	//if (x1 >= (browserGetWidgetBounds(WIDGET_WIDTH) - 1)) { x1 = 0; y1 += g->imgH; } // chances are the link followed an input or HR tag
    	tmp->x1 = x1;
   	tmp->y1 = y1;
	tmp->url = NULL;
	tmp->id = NULL;
	//tmp->postBuf = NULL;
	tmp->post = false;
	tmp->linkVar = NULL;
	tmp->postVar = NULL;
	if (id)
		tmp->id = StrDup(id);
	tmp->next = g->HyperLinks;
    	g->HyperLinks = tmp;

	g->currentLink = tmp;

	return 0;
}

void HyperlinkSetEnd(Int16 x2, Int16 y2, GlobalsType *g)
{
	HyperLinkPtr	link = g->HyperLinks;

	if(!g->HyperLinks)
		return;

	if (link->id && !link->url) {
		link->x1 = 0;
		link->x2 = 0;
		link->y1 = 0;
		link->y2 = 0;
		return; // a bad hack to check for an existing hyperlink??
	}

	link->x2 = x2;
 	link->y2 = y2;

	g->currentLink = NULL;
}

Boolean HyperlinkCheckLink(HyperLinkPtr link, GlobalsType *g)
{
	VarPtr 		var; 
	Char		*newUrl;
	
	if(link && !(link->id && !link->url))
	{
		var = link->linkVar;
		while(var != NULL)
		{
			addVar(var->name, g);
			setVar(var->name, var->value, g);
			var = var->next;
		}

		g->imgH = 0;

		FormInputDeactivate(g);
		if (link->post == true) {
			g->post = true;
			if (g->postBuf) Free(g->postBuf);
			g->postBuf = URLCreateEncodedURL(NULL, link->postVar, g); // postBuf should be freed in post code
			followLink(link->url,g);
		} else {
			g->post = false;
			newUrl = URLCreateEncodedURL(link->url, link->postVar, g);
			followLink(newUrl,g);
			Free(newUrl);
		}
		return true;			
	}

	return false;
}

int HyperlinkFreeLinks(GlobalsType *g)
{
	HyperLinkPtr tmp;
	VarPtr var;

	while(g->HyperLinks != NULL){
		tmp = g->HyperLinks;
		g->HyperLinks = g->HyperLinks->next;
		
		while(tmp->linkVar != NULL){
			var = tmp->linkVar;
			tmp->linkVar = tmp->linkVar->next;
			if(var->name)
				Free(var->name);
			if(var->value)
				Free(var->value);
			Free(var);
		}

		while(tmp->postVar != NULL){
			var = tmp->postVar;
			tmp->postVar = tmp->postVar->next;
			if(var->name)
				Free(var->name);
			if(var->value)
				Free(var->value);
			Free(var);
		}


		if(tmp->url)
			Free(tmp->url);
		if(tmp->id)
			Free(tmp->id);
		Free(tmp);
	}
		
  	return 0;
}

Boolean HyperlinkIsId(GlobalsType *g)
{
	return (g->HyperLinks->id && !g->HyperLinks->url) ? true:false;
}

void HyperlinkSetPost(Boolean post, GlobalsType *g)
{
	g->HyperLinks->post = post;
}

void HyperlinkSetLink(Boolean ison, Char *prefix, Char *url, Char *id, GlobalsType *g)
{
	UInt32          	supportedDepths;
	RGBColorType       	color;  
	IndexedColorType   	indexedColor;


	WinScreenMode( winScreenModeGetSupportedDepths, NULL, NULL, &supportedDepths, NULL );
  	if (ison) {
		//addNewFont((g->font)?g->font->font:stdFont, (g->font)?g->font->res:NULL, g);
		if (url) {
			if (g->font) {
				addNewFont(g->font->font, g->font->res, g);
			} else {
				palmRegular(true);
			}

  			if (supportedDepths >= 138){
    				color.r = (UInt8)0;
    				color.g = (UInt8)0;
    				color.b = (UInt8)210;

    				//TODO: Make Link color customizable
    				indexedColor  = WinRGBToIndex( &color );
    			
				if (g->font) {
					g->font->color = indexedColor;
					g->font->underlined = true;
				}
  			} else {
				if (g->font) {
					g->font->underlined = true;
				}
			}
		}

		HyperlinkAddLink(g->x, g->y, id, g);

  		if (url) {
			HyperlinkSetUrl(prefix, url,g);
		}
	} else {
		if (!url && !prefix && !HyperlinkIsId(g)) {
			removeLastFont(g);
    			HyperlinkSetEnd(g->x, g->y + ((g->imgH > BrowserLineheightValue()) ? BrowserLineheightValue() : g->imgH),g);
		} 
		if (url) {
			HyperlinkSetUrl(prefix, url,g);
		}
  	}
}

void HyperlinkSelectLink(HyperLinkPtr link, Int16 x1, Int16 x2, Int16 y1, Int16 y2, GlobalsType *g)
{
	/*PointType 	pt;
	RectangleType 	rect;
	RectangleType 	bounds;
	DOMPtr		dom = g->DOM;*/

	g->selectedLink = link;

	/*BrowserGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), &rect);

	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	pt = rect.topLeft;

	//rect.topLeft.x += x1;
    	//rect.topLeft.y += y1;
    	//rect.extent.x = MIN(x2, rect.extent.x) - x1;
    	//rect.extent.y = MIN(y2, rect.extent.y) - y1;

    	WinSetClip(&rect);

	for (; dom; dom = dom->next) {
		bounds = dom->bounds;

//		if((bounds.topLeft.y + bounds.extent.y) < (y1 + g->ScrollY))
//			continue;

		if(bounds.topLeft.y > (y2 + g->ScrollY))
			break;
							
		switch (dom->type) {
			case DOM_UI_TEXT:
				if (dom->ptr.t->link == link)
					BrowserInverseLinkText(dom->ptr.t, &pt, &bounds, g);
				break;
			case DOM_UI_IMAGE:
				if (dom->ptr.p->link == link)
					BrowserInverseLinkImage(dom->ptr.p, &pt, &bounds, g);
				break;
			default:
				break;
		}
	}

	WinResetClip();

	WinPopDrawState();*/
}

void HyperlinkDeSelectLink(HyperLinkPtr link, GlobalsType *g)
{
	/*RectangleType	area;
	Int16		x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	RectangleType 	rect;
	RectangleType 	bounds;
	DOMPtr		dom = g->DOM;

	BrowserGetObjectBounds(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget), &rect);

	WinPushDrawState();

	if (isHires())
		WinSetCoordinateSystem(kCoordinatesNative);

	//rect.topLeft.x += x1;
    	//rect.topLeft.y += y1;
    	//rect.extent.x = MIN(x2, rect.extent.x) - x1;
    	//rect.extent.y = MIN(y2, rect.extent.y) - y1;

	x1 = rect.topLeft.x;
	x2 = rect.extent.x;

    	WinSetClip(&rect);

	for (; dom; dom = dom->next) {
		bounds = dom->bounds;

//		if((bounds.topLeft.y + bounds.extent.y) < (y1 + g->ScrollY))
//			continue;

//		if(bounds.topLeft.y > (y2 + g->ScrollY))
//			break;
							
		switch (dom->type) {
			case DOM_UI_TEXT:
				if (dom->ptr.t->link == link) {
					if (!y1)
						y1 = bounds.topLeft.y;
					y2 += bounds.extent.y;
				}
				break;
			case DOM_UI_IMAGE:
				if (dom->ptr.p->link == link) {
					if (!y1)
						y1 = bounds.topLeft.y;
					y2 += bounds.extent.y;
				}
				break;
			default:
				break;
		}
	}

	if (!y2)
		return;

	//y2 += 15;
	y2 += 2;

	area.topLeft.x = x1;
	area.topLeft.y = y1 - g->ScrollY + rect.topLeft.y;
	area.extent.x = x2;
	area.extent.y = y2;

	WinEraseRectangle(&area, 0);

	WinResetClip();

	WinPopDrawState();*/

	//DOMRenderDOM(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), g->BrowserAPI.BrowserWidget),
	//	     0 /*x1 - g->ScrollX*/, 0 /*y1 - g->ScrollY*/,
	//	     g->ScrollX + rect.extent.x /*x1 + x2 - g->ScrollX*/, g->ScrollY + rect.extent.y /*y1 + y2 - g->ScrollY*/,
	//	     ((g->state != BS_IDLE) && (g->state != BS_ONTIMER) && (g->state != BS_XHTML_ONTIMER))?true:false, true, g);
}


Boolean HyperlinkHandlePenHold(Int16 x, Int16 y, GlobalsType *g)
{
	DOMPtr		ptr;
	Boolean		ret = false;

	ptr = g->DOM;

	if (!ptr)
		return false;

	for (; ptr && !ret; ptr = ptr->next) {
        	if (RctPtInRectangle(x, y, &ptr->bounds)) {
            		switch(ptr->type) {
				case DOM_UI_TEXT:
					if (ptr->ptr.t->link &&
					    !(ptr->ptr.t->link->id && !ptr->ptr.t->link->url)) {
						BrowserPopupList(x, y, ptr->type, (void *)ptr->ptr.t, g);

						/*LstSetPosition(GetObjectPtr(listLinkTap), listX, listY);

						selection = LstPopupList(GetObjectPtr(listLinkTap));
						switch (selection) {
							case 0: // open in new tab
								if (g->OpenUrl)
									Free(g->OpenUrl);
								g->OpenUrl = Malloc(MAX_URL_LEN + 1);
								if (!g->OpenUrl) {
									MyErrorFunc("Failed to load the url. No memory", NULL);
									break;
								}
								URLCreateURLStr(ptr->ptr.t->link->url, g->OpenUrl, g);

								if (g->selectedLink) {
									PointerChangeMode(&g->pointer, pointerModeNone);

									HyperlinkDeSelectLink(g->selectedLink, g);
									g->selectedLink = NULL;

									PointerUpdatePointer(&g->pointer, g);
								}

								FrmUpdateForm(frmBrowser, frmUpdateNewTab);
								break;
							case 1: // copy URL
								url = Malloc(MAX_URL_LEN + 1);
								if (!url) {
									MyErrorFunc("Failed to copy the URL. No memory", NULL);
									break;
								}
								URLCreateURLStr(ptr->ptr.t->link->url, url, g);

								ClipboardAddItem(clipboardText, url, StrLen(url));
								break;
							case -1:
							default:
								break;
						}*/

						ret = true;
					}
					break;
				case DOM_UI_IMAGE:
					if (ptr->ptr.p->link &&
					    !(ptr->ptr.p->link->id && !ptr->ptr.p->link->url)) {
						BrowserPopupList(x, y, ptr->type, (void *)ptr->ptr.p, g);

						/*LstSetPosition(GetObjectPtr(listLinkTapImage),listX, listY);

						selection = LstPopupList(GetObjectPtr(listLinkTapImage));
						switch (selection) {
							case 0: // open in new tab
								if (g->OpenUrl)
									Free(g->OpenUrl);
								g->OpenUrl = Malloc(MAX_URL_LEN + 1);
								if (!g->OpenUrl) {
									MyErrorFunc("Failed to load the url. No memory", NULL);
									break;
								}
								URLCreateURLStr(ptr->ptr.p->link->url, g->OpenUrl, g);

								if (g->selectedLink) {
									PointerChangeMode(&g->pointer, pointerModeNone);

									HyperlinkDeSelectLink(g->selectedLink, g);
									g->selectedLink = NULL;

									PointerUpdatePointer(&g->pointer, g);
								}

								FrmUpdateForm(frmBrowser, frmUpdateNewTab);
								break;
							case 1: // copy URL
								url = Malloc(MAX_URL_LEN + 1);
								if (!url) {
									MyErrorFunc("Failed to copy the URL. No memory", NULL);
									break;
								}
								URLCreateURLStr(ptr->ptr.p->link->url, url, g);

								ClipboardAddItem(clipboardText, url, StrLen(url));
								break;
							case 2:
								checkPageImage(x, y, true, g);
								break;
							case -1:
							default:
								break;
						}*/

						ret = true;
					}
					break;
				case DOM_UI_INPUT:
				case DOM_UI_SELECT:
				case DOM_UI_BUTTON:
				case DOM_UI_RADIO:
				case DOM_UI_CHECK:
				case DOM_UI_AREA:
				default:
					break;
			}
		}
	}

	return ret;
}
