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
#include	"xhtml_string.h"
#include	"../res/WAPUniverse_res.h"
#include	"WAPUniverse.h"
#include	"formBrowser.h"
#include	"wml.h"
#include	"variable.h"
#include	"history.h"
#include	"Form.h"
#include	"FiveWay.h"
#include	"PalmUtils.h"
#include	"Layout.h"
#include	"URL.h"
#include	"color.h"
#include	"DOM.h"
#include	"Browser.h"
#include	"Font.h"
#include	"Hyperlinks.h"


/* Drawing functions */


int formDrawButton(Char *label, UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g)
{
	BitmapPtr 	resP;
	Int16		width = 0, panels = 0;
	Int16		x1, y1, x2, y2;
	Int16		i = 0;
	FontID		currFont = FntGetFont();

	currFont = FntSetFont(stdFont);

	width = FontCharsWidth(label, StrLen(label));

	while ((panels * (4 * HiresFactor())) < width) panels++;

	if (!draw)
		if (((8 * HiresFactor()) + (panels * (4 * HiresFactor())) + *x) > (browserGetWidgetBounds(DRAW_WIDTH) - 1))
			xhtml_renderLn(g); 

	//g->x++;

	x1 = *x; y1 = *y;

	if (draw) {
    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpButtonL));
    		WinDrawBitmap (resP, *x, *y);
    		MemPtrUnlock(resP);
	}

	*x += (4 * HiresFactor());


	for (i = 0; i < panels; i++) {
		if (draw) {
    			resP = MemHandleLock(DmGetResource(bitmapRsc, bmpButtonC));
    			WinDrawBitmap (resP, *x, *y);
    			MemPtrUnlock(resP);
		}
		
		*x += (4 * HiresFactor());
	}

	if (draw) {
    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpButtonR));
    		WinDrawBitmap (resP, *x, *y);
    		MemPtrUnlock(resP);
	}

	*x += (6 * HiresFactor());

	x2 = *x; y2 = *y;

	if (draw) {
		WinPushDrawState();
		WinSetDrawMode(winOverlay);
		WinPaintChars(label, StrLen(label), x1 + (4 * HiresFactor()), y1);
		WinPopDrawState();
	}

    	//updateScreen(g);

	if (!draw) {
		browserResizeDimensions(0, (12 * HiresFactor()), g);
		if (g->x > g->DrawWidth)
			g->DrawWidth = g->x;
	}

	FntSetFont(currFont);

	return *x - x1;
}

void formDrawEmptyRadio(UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g)
{
	BitmapPtr 	resP;

	if (!draw)
		*x += HiresFactor();

	if (draw) {
    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpRadioEmpty));
    		WinDrawBitmap(resP, *x, *y + HiresFactor());
    		MemPtrUnlock(resP);
	}

	if (!draw)
		*x += (10 * HiresFactor());

    	//updateScreen(g);

	if (!draw) {
		browserResizeDimensions(0, (12 * HiresFactor()), g);
		if (g->x > g->DrawWidth)
			g->DrawWidth = g->x;
	}
}

void formDrawFullRadio(UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g)
{
	BitmapPtr 	resP;

	if (!draw)
		*x += HiresFactor();

	if (draw) {
    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpRadioFull));
    		WinDrawBitmap(resP, *x, *y + HiresFactor());
    		MemPtrUnlock(resP);
	}

	if (!draw)
		*x += (10 * HiresFactor());

    	//updateScreen(g);

	if (!draw) {
		browserResizeDimensions(0, (12 * HiresFactor()), g);
		if (g->x > g->DrawWidth)
			g->DrawWidth = g->x;
	}
}

void formDrawUnCheckedCheckbox(UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g)
{
	BitmapPtr 	resP;


	if (!draw)
		*x += HiresFactor();

	if (draw) {
    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpUnChecked));
    		WinDrawBitmap (resP, *x, *y + HiresFactor());
    		MemPtrUnlock(resP);
	}

	if (!draw)
		*x += (10 * HiresFactor());

    	//updateScreen(g);

	if (!draw) {
		browserResizeDimensions(0, (12 * HiresFactor()), g);
		if (g->x > g->DrawWidth)
			g->DrawWidth = g->x;
	}
}

void formDrawCheckedCheckbox(UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g)
{
	BitmapPtr 	resP;

	if (!draw)
		*x += HiresFactor();

	if (draw) {
    		resP = MemHandleLock(DmGetResource(bitmapRsc, bmpChecked));
    		WinDrawBitmap (resP, *x, *y + HiresFactor());
    		MemPtrUnlock(resP);
	}

	if (!draw)
		*x += (10 * HiresFactor());


    	//updateScreen(g);

	if (!draw) {
		browserResizeDimensions(0, (12 * HiresFactor()), g);
		if (g->x > g->DrawWidth)
			g->DrawWidth = g->x;
	}
}

/* input areas */
/* TODO: Combine with text input below */
void formCreateInputArea(Char *fieldName, Boolean disabled, Int32 columns, Int32 rows, Char *border, Char *background, GlobalsType *g)
{
	InputPtr 		tmp;
	RGBColorType 		rgb;

	if ((columns * BrowserColumnValue()) > (browserGetWidgetBounds(DRAW_WIDTH) - 2))
		columns = ((browserGetWidgetBounds(DRAW_WIDTH) - 2) / BrowserColumnValue()); // 32;

	if ((g->x + (columns * BrowserColumnValue())) > (browserGetWidgetBounds(DRAW_WIDTH) - 2))
				palmprintln(g);


	tmp = Malloc (sizeof(InputType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	MemSet(tmp, sizeof(InputType), 0);
	tmp->name 		= Malloc(StrLen(fieldName)+1);
	ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
	StrCopy(tmp->name,fieldName);
	addVar(fieldName,g);
	tmp->passwordType 	= false;
	tmp->type 		= inputTypeTextArea;
	tmp->disabled 		= disabled;
	tmp->readonly 		= false;


	tmp->border = (border)?StrAToI(border):1;
	if (background) {
		if (StrStr(background, "transparent")) {
			tmp->bkgrndTransparent = true;
		} else {
			rgb = ConvertColorNumberString(background, TYPE_ELEMENT);
			tmp->bkgrndColor = WinRGBToIndex( &rgb );
		}
	} else {
		rgb = ConvertColorNumberString("#FFFFFF", TYPE_ELEMENT);
		tmp->bkgrndColor = WinRGBToIndex( &rgb );
	}

	/*if (g->y == 0) g->y = HiresFactor();
	g->x++;
	g->y++;*/
	g->x++;

   	tmp->x1 = g->x;
    	tmp->y1 = g->y;
	tmp->x2 = tmp->x1 + columns * BrowserColumnValue();
	tmp->y2 = tmp->y1 + (rows * (BrowserLineheightValue() - HiresFactor()));

	tmp->next = g->Input;
    	g->Input = tmp;

	DOMAddElement(tmp, DOM_UI_INPUT, g->x, g->y, columns * BrowserColumnValue(), (rows * (BrowserLineheightValue() - 1)), g);

	g->x += (columns * BrowserColumnValue()) + 1;// + 2;

	browserResizeDimensions(0, (rows * (BrowserLineheightValue() - 1)) + HiresFactor(), g);
	if (g->x > g->DrawWidth)
		g->DrawWidth = g->x;

	updateScreen(g);
}

static void formInitialiseInput(Char *name, Char *value, GlobalsType *g)
{
	Char 			*expanded;
	InputPtr		input = g->Input;
	Boolean			matched = false;

	while (input != NULL) {
		if (StrNCompare(input->name, name, StrLen(name)) == 0) {
			matched = true;
			break;
		} else {
			matched = false;
		}
		input = input->next;
	}

	if (matched == false)
		return;

	expanded = expandVar(value, g);	

	setVar(input->name,expanded,g);

	Free(expanded);
}

/* select */

static void formInitialiseSelect(Char *name, Char *value, GlobalsType *g)
{
	Char 		*expanded;
	SelectPtr	select = g->Select;
	Boolean		matched = false;
	FontID		currFont = FntGetFont();

	while (select != NULL) {
		if (StrNCompare(select->name, name, StrLen(name)) == 0) {
			matched = true;
			break;
		} else {
			matched = false;
		}
		select = select->next;
	}

	if (matched == false)
		return;

	expanded = expandVar(value, g);	

	setVar(select->name,expanded,g);

	currFont = FntSetFont(stdFont);
 	selectDrawText(select, g);
	FntSetFont(currFont);
}


/* check boxes */

void formCreateCheckBox(Char *name, Char *value, Boolean checked, Boolean disabled, GlobalsType *g)
{
 CheckInputPtr tmp;
 Char *formName;

	tmp = Malloc (sizeof(CheckInputType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	MemSet(tmp, sizeof(CheckInputType), 0);
	formName = formCreateVarName(name,g);
	tmp->name = Malloc(StrLen(formName)+1); // Malloc(StrLen(name)+StrLen(g->Forms->id)+2);
	ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
	StrCopy(tmp->name, formName); // StrCopy(tmp->name,name);
	Free(formName);
	addVar(tmp->name,g);

	tmp->value = Malloc(StrLen((value) ? value:"")+1);
	ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
	StrCopy(tmp->value,(value) ? value:"");

   	tmp->x1 = g->x + HiresFactor();
    	tmp->y1 = (g->y) /*+ (g->ScrollY)*/;

	tmp->x2 = tmp->x1 + (10 * HiresFactor());
	tmp->y2 = tmp->y1 + (10 * HiresFactor());

	tmp->checked = checked;
	tmp->disabled = disabled;

	tmp->next = g->checkBox;
    	g->checkBox = tmp;

	DOMAddElement(tmp, DOM_UI_CHECK, g->x + HiresFactor(), g->y, (10 * HiresFactor()), (10 * HiresFactor()), g);

	if (compareMultiVar(tmp->name, (value) ? value:"", g)) {
		formDrawCheckedCheckbox(&g->x, &g->y, false, g);
	} else if (checked) {
		formDrawCheckedCheckbox(&g->x, &g->y, false, g);
		setMultiVar(tmp->name, (value) ? value:"", g);
	} else {
		formDrawUnCheckedCheckbox(&g->x, &g->y, false, g);
	}
}

/*Boolean checkCheckBox(Int16 x, Int16 y, GlobalsType *g)
{
  CheckInputPtr tmp;
	  
  	tmp = g->checkBox;
	while(tmp != NULL)
	{
		if ((y > tmp->y1) && (y < tmp->y2) &&
				(x > tmp->x1) && (x < tmp->x2))
		{
			if (tmp->disabled == true) return(true);

			FormInputDeactivate(g);

			if (compareMultiVar(tmp->name, tmp->value, g)) {
				removeMultiVar(tmp->name, tmp->value, g);
				formUnCheckBox(tmp->name, tmp->value, g);
				//if ((tmp->y1 > browserGetWidgetBounds(WIDGET_HEIGHT)) || (g->ScrollY > 0))
				//	scrollUpdate(g);
			} else {
				setMultiVar(tmp->name, tmp->value, g);
				formCheckBox(tmp->name, tmp->value, g);
				//if ((tmp->y1 > browserGetWidgetBounds(WIDGET_HEIGHT)) || (g->ScrollY > 0))
				//	scrollUpdate(g);
			}

			return(true);			
		}
		tmp = tmp->next;
	}
	return(false);
	
}*/

Boolean checkCheckBox(CheckInputPtr check, GlobalsType *g)
{
	if (!check)
		return false;

	if (check->disabled)
		return true;

	FormInputDeactivate(g);

	if (compareMultiVar(check->name, check->value, g)) {
		removeMultiVar(check->name, check->value, g);
		formUnCheckBox(check->name, check->value, g);
	} else {
		setMultiVar(check->name, check->value, g);
		formCheckBox(check->name, check->value, g);
	}

	return true;
	
}

void formUnCheckBox(Char *name, Char *value, GlobalsType *g)
{
	CheckInputPtr	tmp = g->checkBox;

	while(tmp != NULL)
	{
		if ((StrNCompare(name,tmp->name,StrLen(tmp->name))==0) && 
		    (StrNCompare(value,tmp->value,StrLen(tmp->value))==0))
		{
			/*prevDrawWin = WinGetDrawWindow();
    			WinSetDrawWindow(g->DrawWin);

    			resP = MemHandleLock(DmGetResource(bitmapRsc, bmpUnChecked));
    			WinDrawBitmap (resP, tmp->x1, tmp->y1 + 1);
    			MemPtrUnlock(resP);

			WinSetDrawWindow(prevDrawWin);*/
    			updateScreen(g);
		}
		tmp = tmp->next;
	}
}

void formCheckBox(Char *name, Char *value, GlobalsType *g)
{
	CheckInputPtr	tmp = g->checkBox;

	while(tmp != NULL)
	{
		if ((StrNCompare(name,tmp->name,StrLen(tmp->name))==0) && 
		    (StrNCompare(value,tmp->value,StrLen(tmp->value))==0))
		{
			/*prevDrawWin = WinGetDrawWindow();
    			WinSetDrawWindow(g->DrawWin);

    			resP = MemHandleLock(DmGetResource(bitmapRsc, bmpChecked));
    			WinDrawBitmap (resP, tmp->x1, tmp->y1 + 1);
    			MemPtrUnlock(resP);

			WinSetDrawWindow(prevDrawWin);*/
    			updateScreen(g);
		}
		tmp = tmp->next;
	}
}

void formResetCheckBox(Char *name, Char *initval, GlobalsType *g)
{
	CheckInputPtr	tmp = g->checkBox;

	while(tmp != NULL)
	{
		if ((StrNCompare(name,tmp->name,StrLen(tmp->name))==0) && 
		    (StrNCompare(initval,tmp->value,StrLen(tmp->value))!=0))
		{
			/*prevDrawWin = WinGetDrawWindow();
    			WinSetDrawWindow(g->DrawWin);

    			resP = MemHandleLock(DmGetResource(bitmapRsc, bmpUnChecked));
    			WinDrawBitmap (resP, tmp->x1, tmp->y1 + 1);
    			MemPtrUnlock(resP);

			WinSetDrawWindow(prevDrawWin);*/
    			updateScreen(g);
		}
		tmp = tmp->next;
	}
}

int disposeCheckBox(GlobalsType *g)
{
  CheckInputPtr tmp;

	while(g->checkBox != NULL){
		tmp = g->checkBox;
		g->checkBox = g->checkBox->next;
		if(tmp->name)
			Free(tmp->name);
		if(tmp->value)
			Free(tmp->value);
		Free(tmp);
	}		
  	return(0);
}

/* radio buttons */

void formCreateRadioButton(Char *name, Char *value, Boolean checked, Boolean disabled, GlobalsType *g)
{
 RadioInputPtr tmp;
 Char *formName;

	tmp = Malloc (sizeof(RadioInputType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	MemSet(tmp, sizeof(RadioInputType), 0);
	formName = formCreateVarName(name,g);
	tmp->name = Malloc(StrLen(formName)+1); // Malloc(StrLen(name)+StrLen(g->Forms->id)+2);
	ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
	StrCopy(tmp->name, formName); // StrCopy(tmp->name,name);
	Free(formName);
	addVar(tmp->name,g);

	tmp->value = Malloc(StrLen((value) ? value:"")+1);
	ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
	StrCopy(tmp->value,(value) ? value:"");

   	tmp->x1 = g->x + HiresFactor();
    	tmp->y1 = (g->y) /*+ (g->ScrollY)*/;

	tmp->x2 = tmp->x1 + (10 * HiresFactor());
	tmp->y2 = tmp->y1 + (10 * HiresFactor());

	tmp->checked = checked;
	tmp->disabled = disabled;

	tmp->next = g->radioBtn;
    	g->radioBtn = tmp;

	DOMAddElement(tmp, DOM_UI_RADIO, g->x + HiresFactor(), g->y, (10 * HiresFactor()), (10 * HiresFactor()), g);

	if (StrNCompare(getVar(tmp->name, g), (value) ? value:"", StrLen((value) ? value:"")) == 0) {
		formDrawFullRadio(&g->x, &g->y, false, g);
	} else if (checked) {
		formDrawFullRadio(&g->x, &g->y, false, g);
		setVar(tmp->name, (value) ? value:"", g);
	} else {
		formDrawEmptyRadio(&g->x, &g->y, false, g);
	}
}

/*Boolean checkRadioButton(Int16 x, Int16 y, GlobalsType *g)
{
  RadioInputPtr tmp;

  	tmp = g->radioBtn;
	while(tmp != NULL)
	{
		if ((y > tmp->y1) && (y < tmp->y2) &&
				(x > tmp->x1) && (x < tmp->x2))
		{
			if (tmp->disabled == true) return(true);

			FormInputDeactivate(g);

			if (StrNCompare(getVar(tmp->name, g), tmp->value, StrLen(tmp->value))==0) {
				//if ((tmp->y1 > browserGetWidgetBounds(WIDGET_HEIGHT)) || (g->ScrollY > 0))
				//	scrollUpdate(g);
				updateScreen(g);
				return(true);
			} else {
				setVar(tmp->name, tmp->value, g);
				formSelectRadioButton(tmp->name, tmp->value, g);
				//if ((tmp->y1 > browserGetWidgetBounds(WIDGET_HEIGHT)) || (g->ScrollY > 0))
				//	scrollUpdate(g);
				//updateScreen(g);
				return(true);
			}	
		}
		tmp = tmp->next;
	}
	return(false);
	
}*/

Boolean checkRadioButton(RadioInputPtr radio, GlobalsType *g)
{
	if (!radio)
		return false;

	if (radio->disabled)
		return true;

	FormInputDeactivate(g);

	if (StrNCompare(getVar(radio->name, g), radio->value, StrLen(radio->value))==0) {
		updateScreen(g);
	} else {
		setVar(radio->name, radio->value, g);
		formSelectRadioButton(radio->name, radio->value, g);
	}

	return true;
	
}

void formSelectRadioButton(Char *name, Char *value, GlobalsType *g)
{
	//BitmapPtr 	resP;
	//WinHandle	prevDrawWin;
	RadioInputPtr	tmp = g->radioBtn;

	while(tmp != NULL)
	{
		if ((StrNCompare(name,tmp->name,StrLen(tmp->name))==0) && 
		    (StrNCompare(value,tmp->value,StrLen(tmp->value))==0))
		{
			/*prevDrawWin = WinGetDrawWindow();
    			WinSetDrawWindow(g->DrawWin);

    			resP = MemHandleLock(DmGetResource(bitmapRsc, bmpRadioFull));
    			WinDrawBitmap (resP, tmp->x1, tmp->y1 + 1);
    			MemPtrUnlock(resP);

			WinSetDrawWindow(prevDrawWin);*/
    			updateScreen(g);
		} else if (StrNCompare(name,tmp->name,StrLen(tmp->name))==0) {
			/*prevDrawWin = WinGetDrawWindow();
    			WinSetDrawWindow(g->DrawWin);

    			resP = MemHandleLock(DmGetResource(bitmapRsc, bmpRadioEmpty));
    			WinDrawBitmap (resP, tmp->x1, tmp->y1 + 1);
    			MemPtrUnlock(resP);

			WinSetDrawWindow(prevDrawWin);*/
    			updateScreen(g);
		}
		tmp = tmp->next;
	}
}

int disposeRadioButton(GlobalsType *g)
{
  RadioInputPtr tmp;

	while(g->radioBtn != NULL){
		tmp = g->radioBtn;
		g->radioBtn = g->radioBtn->next;
		if(tmp->name)
			Free(tmp->name);
		if(tmp->value)
			Free(tmp->value);
		Free(tmp);
	}		
  	return(0);
}

/* Buttons */

void formCreateButton(Char *name, Char *value, ButtonType type, int width, int height, Boolean disabled, Boolean image, GlobalsType *g)
{
 ButtonInputPtr tmp;
 Char *formName;

	tmp = Malloc (sizeof(ButtonInputType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	MemSet(tmp, sizeof(ButtonInputType), 0);

	if (name) {
		formName = formCreateVarName(name,g);
		tmp->name = Malloc(StrLen(formName) + 1); //Malloc(StrLen(name)+StrLen(g->Forms->id)+2);
		ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
		StrCopy(tmp->name, formName); // StrCopy(tmp->name,name);
		Free(formName);
		addVar(tmp->name, g);
	} else {
		formName = formCreateVarName("untitled",g);
		tmp->name = Malloc(StrLen(formName) + 1); // Malloc(StrLen("untitled")+StrLen(g->Forms->id)+2);
		ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
		StrCopy(tmp->name, formName); // StrCopy(tmp->name,name);
		Free(formName);
		addVar(tmp->name, g);
	}

	tmp->value = Malloc(StrLen((value) ? value:"")+1);
	ErrFatalDisplayIf (!tmp->value, "Malloc Failed");
	StrCopy(tmp->value, (value) ? value:"");

   	tmp->x1 = g->x - width;
    	tmp->y1 = (g->y) /*+ (g->ScrollY)*/;

	tmp->x2 = tmp->x1 + width;
	tmp->y2 = tmp->y1 + height;

	tmp->disabled = disabled;

	tmp->type = type;
	tmp->form = g->Forms;

	tmp->image = image;

	tmp->next = g->formButton;
    	g->formButton = tmp;

	DOMAddElement(tmp, DOM_UI_BUTTON, g->x - width, g->y, width, height, g);
}

/*Boolean checkFormButton(Int16 x, Int16 y, GlobalsType *g)
{
  	ButtonInputPtr	tmp;
  	NamedValuePtr	resetVal;
  	Char		*newUrl;

  	tmp = g->formButton;
	while(tmp != NULL)
	{
		if ((y > tmp->y1) && (y < tmp->y2) &&
				(x > tmp->x1) && (x < tmp->x2))
		{
			if (tmp->disabled == true) return(true);

			FormInputDeactivate(g);

			switch (tmp->type) {
				case SUBMIT:
					if (tmp->form->method == POST) {
						setVar(tmp->name, tmp->value,g);
						g->post = true;
						if (g->postBuf) Free(g->postBuf);
						g->postBuf = URLCreateHTMLEncodedURL(NULL, tmp->form->value, tmp->name, g); // postBuf should be freed in post code
						g->cache.cacheDoNotRead = true;
						followLink(tmp->form->action,g);
					} else if (tmp->form->method == GET) {
						setVar(tmp->name, tmp->value,g);
						g->post = false;
						newUrl = URLCreateHTMLEncodedURL(tmp->form->action, tmp->form->value, tmp->name, g);
						g->cache.cacheDoNotRead = true;
						followLink(newUrl,g);
						Free(newUrl);
					}
					break;
				case RESET:
					resetVal = tmp->form->value;
					while (resetVal != NULL) {
						switch (resetVal->type) {
							case TEXT:
								formInitialiseInput(resetVal->name, resetVal->initval, g);
								break;
							case RADIO:
								setVar(resetVal->name, resetVal->initval, g);
								formSelectRadioButton(resetVal->name, resetVal->initval, g);
								break;
							case CHECK:
								if (resetVal->initval == NULL) {
									setVar(resetVal->name, "", g);
									formResetCheckBox(resetVal->name, "", g);
								} else {
									setMultiVar(resetVal->name, resetVal->initval, g);
									formCheckBox(resetVal->name, resetVal->initval, g);
								}
								break;
							case SELECT:
								formInitialiseSelect(resetVal->name, resetVal->initval, g);
								break;
							case BUTTON:
							case HIDDEN:
								break;
							default:
								setVar(resetVal->name, resetVal->initval, g);
								break;	
						}
						resetVal = resetVal->next;
					}
					scrollUpdate(g);
					break;
			}

			return(true);			
		}
		tmp = tmp->next;
	}
	return(false);
	
}*/

Boolean checkFormButton(ButtonInputPtr button, GlobalsType *g)
{
  	NamedValuePtr	resetVal;
  	Char		*newUrl;

	if (!button)
		return false;

	if (button->disabled == true)
		return true;

	FormInputDeactivate(g);

	switch (button->type) {
		case SUBMIT:
			if (button->form->method == POST) {
				setVar(button->name, button->value,g);
				g->post = true;
				if (g->postBuf) Free(g->postBuf);
				g->postBuf = URLCreateHTMLEncodedURL(NULL, button->form->value, button->name, g); // postBuf should be freed in post code
				g->cache.cacheDoNotRead = true;
				followLink(button->form->action,g);
			} else if (button->form->method == GET) {
				setVar(button->name, button->value,g);
				g->post = false;
				newUrl = URLCreateHTMLEncodedURL(button->form->action, button->form->value, button->name, g);
				g->cache.cacheDoNotRead = true;
				followLink(newUrl,g);
				Free(newUrl);
			}
			break;
		case RESET:
			resetVal = button->form->value;
			while (resetVal != NULL) {
				switch (resetVal->type) {
					case TEXT:
						formInitialiseInput(resetVal->name, resetVal->initval, g);
						break;
					case RADIO:
						setVar(resetVal->name, resetVal->initval, g);
						formSelectRadioButton(resetVal->name, resetVal->initval, g);
						break;
					case CHECK:
						if (resetVal->initval == NULL) {
							setVar(resetVal->name, "", g);
							formResetCheckBox(resetVal->name, "", g);
						} else {
							setMultiVar(resetVal->name, resetVal->initval, g);
							formCheckBox(resetVal->name, resetVal->initval, g);
						}
						break;
					case SELECT:
						formInitialiseSelect(resetVal->name, resetVal->initval, g);
						break;
					case BUTTON:
					case HIDDEN:
						break;
					default:
						setVar(resetVal->name, resetVal->initval, g);
						break;	
				}
				resetVal = resetVal->next;
			}
			scrollUpdate(g);
			break;
	}
	return true;
	
}

int formReDrawButton(Char *label, GlobalsType *g)
{
	return 0;
}

int disposeButton(GlobalsType *g)
{
  ButtonInputPtr tmp;

	while(g->formButton != NULL){
		tmp = g->formButton;
		g->formButton = g->formButton->next;
		if(tmp->name)
			Free(tmp->name);
		if(tmp->value)
			Free(tmp->value);
		Free(tmp);
	}		
  	return(0);
}

/* Selection Lists */

Boolean selectInitList (SelectPtr slct, GlobalsType *g)
{
  	ListPtr       	optionslstPtr;
  	UInt16        	nrOptions;
  	UInt16		offset=0;
  	FormPtr       	frm;
  	Char          	*txt;
  	Char 		*options;
  	Err           	error;
  	MemHandle	optionsHandle,optionsListArrayH;		  
  	OptionPtr     	tmp;
  	//WinHandle   	prevDrawWin;
  	//RectangleType	rect;
  	VarPtr		var;
	Char 		*newUrl = NULL;
  
  	nrOptions = 0;
  	optionsHandle = MemHandleNew (2);
  	options = MemHandleLock (optionsHandle);
  	*options = 0;
  	tmp = slct->options;
  	while(tmp != NULL) { 
	  	if(tmp->txt != NULL) {
	      		MemHandleUnlock (optionsHandle);
	      		error = MemHandleResize (optionsHandle,StrLen(tmp->txt)+
                        	offset + 2);
	      		options = MemHandleLock (optionsHandle);
	      		StrCopy(options+offset,tmp->txt);
	      		offset += (StrLen(tmp->txt)+1);
      	  		nrOptions++;
	  	}
	  	tmp=tmp->next;
  	}
  	*(options+offset)=0;
  	if (nrOptions == 0) {
      		MemHandleUnlock (optionsHandle);
	  	MemHandleFree (optionsHandle);
      		return(false);
    	}
  	optionsListArrayH = SysFormPointerArrayToStrings (options, nrOptions);
  	frm = FrmGetActiveForm();
  	optionslstPtr = (ListPtr) (FrmGetObjectPtr (frm, (FrmGetObjectIndex
                                                        (frm, listOption))));
  	LstSetListChoices (optionslstPtr, MemHandleLock
                     (optionsListArrayH), nrOptions);
  	LstSetHeight (optionslstPtr, nrOptions);
//  	LstSetSelection(optionslstPtr,0); // current connection 
  	LstPopupList(optionslstPtr);
  	txt = LstGetSelectionText(optionslstPtr,
                          LstGetSelection(optionslstPtr));

	// TODO: Is this right for positioning?
  	if (txt){
 		if(slct->name){
			//value = getVar(slct->name);
			tmp = slct->options;
			while(tmp != NULL)
			{
				if (StrCompare(txt,tmp->txt)==0) {
					setVar(slct->name,tmp->value,g);
					var = tmp->optVar;
					while(var != NULL) {
						addVar(var->name, g);
						setVar(var->name, var->value, g);
						var = var->next;
					}

					if (tmp->onpickUrl != NULL) {
						g->imgH = 0;

						if (!tmp->post) {
							g->post = false;
							newUrl = URLCreateEncodedURL(tmp->onpickUrl, tmp->postVar, g);
							//followLink(newUrl,g);
							//free(newUrl);
							goto return_link;
						} else if (tmp->post) {
							g->post = true;
							if (g->postBuf) Free(g->postBuf);
							g->postBuf = URLCreateEncodedURL(NULL, tmp->postVar, g); // postBuf should be freed in post code
							//followLink(tmp->onpickUrl,g);
							goto return_link;
						}
					}
				}
				tmp=tmp->next;
			}
		}

		updateScreen(g);
  	}
  	MemHandleUnlock (optionsHandle);
  	MemHandleFree (optionsHandle);

  	return(true);

return_link:
  	MemHandleUnlock (optionsHandle);
  	MemHandleFree (optionsHandle);

	if (newUrl) {
		followLink(newUrl,g);
		free(newUrl);
	} else
		followLink(tmp->onpickUrl,g);

	return(true);
}

void palmWmlSelectOptionTxt(Char *txt, GlobalsType *g)
{
 OptionPtr tmp;

	tmp = g->Select->options;
	if(tmp != NULL){
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		//if ((StrLen(txt) > 0) && (txt != NULL)) {
			tmp->txt = Malloc(StrLen(txt)+1);
			ErrFatalDisplayIf (!tmp->txt, "Malloc Failed");
			StrCopy(tmp->txt,txt);
		//} else {
		//	tmp->txt = Malloc(StrLen("- null -")+1);
		//	ErrFatalDisplayIf (!tmp->txt, "Malloc Failed");
		//	StrCopy(tmp->txt,"- null -");
		//}
	}
}

void palmWmlSelectOptionOnPick(Char *proto, Char *url, GlobalsType *g)
{
 OptionPtr tmp;

	tmp = g->Select->options;
	if(tmp != NULL){
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->onpickUrl = Malloc(StrLen(proto)+StrLen(url)+1);
			ErrFatalDisplayIf (!tmp->onpickUrl, "Malloc Failed");
		StrCopy(tmp->onpickUrl,proto);
		StrCopy(tmp->onpickUrl+StrLen(proto),url);
	}
		
}

void palmWmlSelectOptionPost(Boolean post, GlobalsType *g)
{
 OptionPtr tmp;

	tmp = g->Select->options;
	if(tmp != NULL){
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->post = post;
	}
		
}

void selectDrawTrigger(Int16 x, Int16 y)
{
	BitmapPtr resP;

	resP=MemHandleLock(DmGetResource(bitmapRsc, bmpArrow));
	WinDrawBitmap (resP, x, y);
 	MemPtrUnlock(resP);
}

void palmWmlSelect(Char *name, Boolean disabled, GlobalsType *g)
{
 SelectPtr 		tmp;
 
    	if ( g->x > 100)
		palmprintln(g);	
	/*switch (g->Align) {
		case Center:
			if (g->x != (152 / 2))
				palmprintln(g);
			g->x = 0;
			break;
		case Right:
			if (g->x != 152)
				palmprintln(g);
			g->x = 0;
			break;
		case Left:
		default:
			break;
    	}*/

	tmp = Malloc (sizeof(SelectType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	tmp->name = Malloc(StrLen(name)+1);
	ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
	StrCopy(tmp->name,name);
	addVar(name,g);
    	tmp->x1=g->x;
    	tmp->y1=g->y /*+ g->ScrollY*/;
	tmp->x2= tmp->x1 + (10 * HiresFactor());
	tmp->y2= g->y + (BrowserLineheightValue() - HiresFactor());
	tmp->disabled = disabled;
	tmp->next = g->Select;
	tmp->options = NULL;
    	g->Select = tmp;

	DOMAddElement(tmp, DOM_UI_SELECT, g->x, g->y, 10 * HiresFactor(), (BrowserLineheightValue() - HiresFactor()), g);

	/*{
	WinHandle  prevDrawWin;
 
		prevDrawWin = WinGetDrawWindow();
 		palmResizeWindow(13,g);
 		WinSetDrawWindow(g->DrawWin);
 		selectDrawTrigger(g->x,g->y);
 		//palmprintln(g);	
 		WinSetDrawWindow(prevDrawWin);
 		updateScreen(g);
		g->x += 90;
	}*/

	g->x += 90 * HiresFactor();
	browserResizeDimensions(0, 13 * HiresFactor(), g);
	if (g->x > g->DrawWidth)
		g->DrawWidth = g->x;
}

void palmWmlSelectValue(Char *ivalue, GlobalsType *g)
{
	if(g->Select){
		setVar(g->Select->name,ivalue,g);
	}
}

void palmWmlSelectOption(Char *value, GlobalsType *g)
{
 OptionPtr tmp,nw;

	if(g->Select){
		nw = Malloc (sizeof(OptionType));
		ErrFatalDisplayIf (!nw, "Malloc Failed");
		nw->value = Malloc(StrLen(value)+1);
		ErrFatalDisplayIf (!nw->value, "Malloc Failed");
		StrCopy(nw->value,value);
		nw->txt = NULL;
		nw->next = NULL;
		nw->onpickUrl = NULL;
		nw->optVar = NULL;
		nw->post = false;
		nw->postVar = NULL;
		tmp = g->Select->options;
		if(tmp == NULL){
			g->Select->options = nw;
		}else{
			while(tmp->next != NULL){
				tmp = tmp->next;
			}
			tmp->next = nw;
		}
	}
}
 
void selectDrawText(SelectPtr slct, GlobalsType *g)
{
	updateScreen(g);

 	/*Char	*txt, *value;
 	OptionPtr tmp;
	FontID		currFont = FntGetFont();
 
	if(slct){
		if(slct->name){
			value = getVar(slct->name,g);
			tmp = slct->options;
			txt = NULL;
			while(tmp != NULL) {
				if (StrCompare(value,tmp->value)==0)
					txt = tmp->txt;
				tmp=tmp->next;
			}

			currFont = FntSetFont(stdFont);

			if (txt) {
				Int16 			stringWidthP = 79;
				Int16 			stringLengthP = StrLen(txt);
				Boolean 		fitWithinWidth;
				WinDrawOperation	oldMode;
				WinHandle		prevDrawWin;

				oldMode = WinSetDrawMode(winOverlay);

				prevDrawWin = WinGetDrawWindow();
    				WinSetDrawWindow(g->DrawWin);

				FontCharsInWidth(txt, &stringWidthP, &stringLengthP, &fitWithinWidth);
				WinDrawChars(txt, stringLengthP, slct->x1 + 11, slct->y1);

				WinSetDrawWindow(prevDrawWin);

				WinSetDrawMode(oldMode);
			} else {
				RectangleType	rect;
				WinHandle	prevDrawWin;

				rect.topLeft.x = slct->x1 + 10;// + 1;
				rect.extent.x = 79;
				rect.topLeft.y = slct->y1;
				rect.extent.y = (slct->y2) - (slct->y1);// - 1; //BrowserLineheightValue()-2;//12;

    				prevDrawWin = WinGetDrawWindow();
    				WinSetDrawWindow(g->DrawWin);
				WinEraseRectangle (&rect, 0);
    				WinSetDrawWindow(prevDrawWin);			
			}

			FntSetFont(currFont);
		}
 	}*/
}


void palmWmlSelectEnd(GlobalsType *g)
{
	/*WinHandle	prevDrawWin;
	FontID		currFont = FntGetFont();
 
	prevDrawWin = WinGetDrawWindow();
 	palmResizeWindow(12,g);
 	WinSetDrawWindow(g->DrawWin);
 	//selectDrawTrigger(g->x,g->y + 1);
	currFont = FntSetFont(stdFont);
 	selectDrawText(g->Select,g);
	FntSetFont(currFont);
 	//palmprintln(g);	
 	WinSetDrawWindow(prevDrawWin);*/
 	updateScreen(g);
}

int disposeOptions(OptionPtr options, GlobalsType *g)
{
	OptionPtr	tmp;
	VarPtr		var, post;

	while(options != NULL){
		tmp = options;
		options = options->next;

		while(tmp->optVar != NULL){
			var = tmp->optVar;
			tmp->optVar = tmp->optVar->next;
			if(var->name)
				Free(var->name);
			if(var->value)
				Free(var->value);
			Free(var);
		}

		while(tmp->postVar != NULL){
			post = tmp->postVar;
			tmp->postVar = tmp->postVar->next;
			if(post->name)
				Free(post->name);
			if(post->value)
				Free(post->value);
			Free(post);
		}

		if(tmp->value)
			Free(tmp->value);
		if(tmp->txt)
			Free(tmp->txt);
		Free(tmp);
	}		
  	return(0);
}
 
int disposeSelect(GlobalsType *g)
{
  SelectPtr tmp;

	while(g->Select != NULL){
		tmp = g->Select;
		g->Select = g->Select->next;
		if(tmp->name)
			Free(tmp->name);
		disposeOptions(tmp->options,g);
		Free(tmp);
	}		
  	return(0);
}


/*Boolean checkSelect(Int16 x, Int16 y, GlobalsType *g)
{
  SelectPtr tmp;
  ControlPtr ctl;
	  
  	tmp= g->Select;
	while(tmp != NULL)
	{
		if ((y>tmp->y1)&&(y<tmp->y2)&&
				(x>tmp->x1)&&(x<tmp->x2))
		{
			if (tmp->disabled == true) return(true);

			FormInputDeactivate(g);
  			ctl = GetObjectPtr (listOption);

			if ((tmp->y1 > browserGetWidgetBounds(WIDGET_HEIGHT)) || (g->ScrollY > 0)) {
				scrollUpdate(g);
			}

			LstSetPosition((ListPtr)ctl,tmp->x1 + browserGetWidgetBounds(WIDGET_TOPLEFT_X),(tmp->y1 + browserGetWidgetBounds(WIDGET_TOPLEFT_Y)) - g->ScrollY);

			return(selectInitList(tmp,g));
		}
		tmp = tmp->next;
	}
	return(false);
}*/

Boolean checkSelect(SelectPtr select, GlobalsType *g)
{
	ControlPtr ctl;
	  
	if (!select)
		return false;

	if (select->disabled)
		return true;

	FormInputDeactivate(g);
  	ctl = GetObjectPtr(listOption);


	//if ((select->y1 > browserGetWidgetBounds(WIDGET_HEIGHT)) || (g->ScrollY > 0)) {
	//	scrollUpdate(g);
	//}

	LstSetPosition((ListPtr)ctl,((select->x1 + browserGetWidgetBounds(WIDGET_TOPLEFT_X)) - g->ScrollX) / HiresFactor(),((select->y1 + browserGetWidgetBounds(WIDGET_TOPLEFT_Y)) - g->ScrollY) / HiresFactor());

	return selectInitList(select,g);
}

/* Input elements */

void palmWmlInputType(Boolean pwdType, GlobalsType *g)
{
	if(g->Input)
		g->Input->passwordType = pwdType;
}	

void palmWmlInputFormat(Char *format, GlobalsType *g)
{
	if(g->Input)
		StrCopy(g->Input->format, format);
}

void FormInputSetType(InputType *inputP, InputFieldType type)
{
	inputP->type = type;
}

void FormInputSetReadonly(InputType *inputP)
{
	inputP->readonly = true;
}

void palmWmlInputValue(Char *value, GlobalsType *g)
{
    	Char *expanded;

//	expanded = Malloc(64000);
	expanded = expandVar(value, g);	

	if(g->Input){
		//setVar(g->Input->name,value,g);
		//palmDrawChars(value, StrLen(value), g->Input->x1+1, g->Input->y1,g);
		setVar(g->Input->name,expanded,g);

		updateScreen(g);
	}
	Free(expanded);
}		

/* TODO: Merge with input area function */
void palmWmlInput(Char *fieldName, Boolean disabled, Int16 size, Int32 maxLength, GlobalsType *g)
{
	InputPtr 	tmp;
	RGBColorType	rgb;

	/* TODO: Get a little more fancy with handling borders */

	if (maxLength < 0)
		maxLength = maxFieldTextLen;

	tmp = Malloc (sizeof(InputType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	tmp->name = Malloc(StrLen(fieldName)+1);
	ErrFatalDisplayIf (!tmp->name, "Malloc Failed");
	StrCopy(tmp->name,fieldName);
	addVar(fieldName,g);
	tmp->passwordType = false;
	tmp->disabled = disabled;
	tmp->readonly = false;
	tmp->maxlength = (maxFieldTextLen >= maxLength)?maxLength:maxFieldTextLen;

	tmp->border = 1;

	/*if (g->y == 0) g->y = 2 * HiresFactor();
	g->y += HiresFactor();
	g->x += (3 * HiresFactor());*/
	g->x++;

	// TODO: Use width from html
   	tmp->x1 = g->x;
    	tmp->y1 = g->y;
	if (size > 0) {
		size *= BrowserColumnValue();

		if (size > (browserGetWidgetBounds(DRAW_WIDTH) - 2))
			size = browserGetWidgetBounds(DRAW_WIDTH) - 2;

		if (size > ((browserGetWidgetBounds(DRAW_WIDTH) - 2) - tmp->x1)) {
			palmprintln(g);
			g->x++;
			tmp->x1 = g->x;
			tmp->y1 = g->y;
		}
	} else {
		size = ((browserGetWidgetBounds(DRAW_WIDTH) - 2) - tmp->x1);
	}
	tmp->x2 = tmp->x1 + size;
	tmp->y2 = g->y + BrowserLineheightValue();
	rgb = ConvertColorNumberString("#FFFFFF", TYPE_ELEMENT);
	tmp->bkgrndColor = WinRGBToIndex( &rgb );
	tmp->bkgrndTransparent = false;
	tmp->border = 1;
	tmp->type = inputTypeText;
	tmp->next = g->Input;
    	g->Input = tmp;

	DOMAddElement(tmp, DOM_UI_INPUT, g->x, g->y, size, BrowserLineheightValue(), g);


	g->x += size + 1;
	//g->y += (2 * HiresFactor());

	browserResizeDimensions(0, BrowserLineheightValue() + HiresFactor(), g);
	if (g->x > g->DrawWidth)
		g->DrawWidth = g->x;

	//updateScreen(g);	
}

void FormInputDeactivate(GlobalsType *g)
{
  FieldPtr      	fldP;
  FormPtr       	frm;
  Char 			*txt;
  InputPtr		tmp;
 
      
	if(g->InputFldActive) // if there is an active input field, dactivate it 
	{				
		frm = FrmGetActiveForm ();
	  	fldP = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm,fldInput));
  		ErrNonFatalDisplayIf (!fldP, "missing field");
  		txt = FldGetTextPtr (fldP);

		tmp = g->InputFldActive;

		//add checks for formatting and emptyok

	    	setVar(tmp->name,txt,g);
		g->InputFldActive=NULL;
		FrmHideObject(FrmGetActiveForm(),FrmGetObjectIndex(frm,fldInput));

		updateScreen(g);
	}				

}

/*Boolean checkInput(Int16 x, Int16 y, GlobalsType *g)
{
	InputPtr 	tmp;
 	ControlPtr 	ctl;
	RectangleType 	rect;
	Int16		check1;
	Int16		check2;

  	tmp= g->Input;
	while(tmp != NULL)
	{
		if ((y>tmp->y1)&&(y<tmp->y2)&&
				(x>tmp->x1)&&(x<tmp->x2))
		{
			if (tmp->disabled == true)
				return(true);
			if (g->InputFldActive == tmp)
				return(true);
			if (g->InputFldActive)
				FormInputDeactivate(g);

			check1 = (tmp->y2 - g->ScrollY);
			check2 = (tmp->y1 - g->ScrollY);

			if (check1 > (Int16)(browserGetWidgetBounds(WIDGET_HEIGHT) - 1)) {
				g->ScrollY = tmp->y2 - browserGetWidgetBounds(WIDGET_HEIGHT); // redo this so it does not scroll as much??

    				if (g->ScrollY > (g->DrawHeight-browserGetWidgetBounds(WIDGET_HEIGHT)))
        				g->ScrollY = (g->DrawHeight-browserGetWidgetBounds(WIDGET_HEIGHT));

				scrollUpdate(g);
			} else if (check2 < (Int16)0 ) { //(Int16)browserGetWidgetBounds(WIDGET_TOPLEFT_Y)) {
				while (check2 < (Int16)0 ) { //(Int16)browserGetWidgetBounds(WIDGET_TOPLEFT_Y)) {
					g->ScrollY -= 1; // tmp->y2 - g->ScrollY;
					check2 = (tmp->y1 - g->ScrollY);
				}

    				if (g->ScrollY < 0)
        				g->ScrollY = 0;

				scrollUpdate(g);
			}

 			rect.topLeft.x = (tmp->x1 + browserGetWidgetBounds(WIDGET_TOPLEFT_X)) + 1;
			rect.extent.x = (tmp->x2) - (tmp->x1) - 2;
			rect.topLeft.y = (tmp->y1 + browserGetWidgetBounds(WIDGET_TOPLEFT_Y)) - g->ScrollY;
			rect.extent.y = (tmp->y2) - (tmp->y1);

  			ctl = GetObjectPtr (fldInput);
			FldSetBounds((FieldPtr)ctl,&rect);
			//if (tmp->maxlength > 0) FldSetMaxChars((FieldPtr)ctl, tmp->maxlength);

			g->InputFldActive=tmp;
			SetFieldFromStr(getVar(tmp->name,g) , fldInput);

			if ((tmp->y1 > browserGetWidgetBounds(WIDGET_HEIGHT)) || (g->ScrollY > 0)) {
				scrollUpdate(g);
			}


  		    	FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),fldInput));
			FrmSetFocus (FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),fldInput));

			return(true);			
		}
		tmp = tmp->next;
	}
	return(false);
	
}*/

Boolean checkInput(InputPtr input, GlobalsType *g)
{
 	ControlPtr 	ctl;
	RectangleType 	rect;
	Int16		check1;
	Int16		check2;
	FieldAttrType	attr;

	if (!input)
		return false;

	if (input->disabled)
		return true;
	if (g->InputFldActive == input)
		return false;
	if (g->InputFldActive)
		FormInputDeactivate(g);
	
	// TODO: Move these
	g->browserGlobals.tapandhold = 0;
	g->browserGlobals.taptick = 0;
	g->browserGlobals.taptype = 0;
	g->browserGlobals.tapx = 0;
	g->browserGlobals.tapy = 0;

	check1 = (input->y2 - g->ScrollY);
	check2 = (input->y1 - g->ScrollY);

	// TODO: fixme

	if (check1 > (Int16)(browserGetWidgetBounds(WIDGET_HEIGHT) - 1)) {
		g->ScrollY = input->y2 - browserGetWidgetBounds(WIDGET_HEIGHT); // redo this so it does not scroll as much??

    		if (g->ScrollY > (g->DrawHeight-browserGetWidgetBounds(WIDGET_HEIGHT)))
        		g->ScrollY = (g->DrawHeight-browserGetWidgetBounds(WIDGET_HEIGHT));

		scrollUpdate(g);
	} else if (check2 < (Int16)0 /*(Int16)browserGetWidgetBounds(WIDGET_TOPLEFT_Y)*/) {
		while (check2 < (Int16)0 /*(Int16)browserGetWidgetBounds(WIDGET_TOPLEFT_Y)*/) {
			g->ScrollY -= 1; // tmp->y2 - g->ScrollY;
			check2 = (input->y1 - g->ScrollY);
		}

    		if (g->ScrollY < 0)
        		g->ScrollY = 0;

		scrollUpdate(g);
	}

 	rect.topLeft.x = ((input->x1 + browserGetWidgetBounds(WIDGET_TOPLEFT_X)) - g->ScrollX + 1) / HiresFactor();
	rect.extent.x = ((input->x2) - (input->x1) - 2) / HiresFactor();
	rect.topLeft.y = ((input->y1 + browserGetWidgetBounds(WIDGET_TOPLEFT_Y)) - g->ScrollY + 1) / HiresFactor();
	rect.extent.y = ((input->y2) - (input->y1)) / HiresFactor();

  	ctl = GetObjectPtr (fldInput);
	FldSetBounds((FieldPtr)ctl,&rect);
	if (input->maxlength > 0)
		FldSetMaxChars((FieldPtr)ctl, input->maxlength);
	if (input->passwordType)
		FldSetFont(GetObjectPtr(fldInput), passwordFont);
	else
		FldSetFont(GetObjectPtr(fldInput), stdFont);

	g->InputFldActive = input;
	SetFieldFromStr(getVar(input->name,g) , fldInput);

	if ((input->y1 > browserGetWidgetBounds(WIDGET_HEIGHT)) || (g->ScrollY > 0)) {
		scrollUpdate(g);
	}


  	FrmShowObject(FrmGetActiveForm(),FrmGetObjectIndex(FrmGetActiveForm(),fldInput));
	if (input->readonly) {
		FldGetAttributes(GetObjectPtr(fldInput), &attr);
		attr.editable = 0;
		FldSetAttributes(GetObjectPtr(fldInput), &attr);
	} else {
		FldGetAttributes(GetObjectPtr(fldInput), &attr);
		attr.editable = 1;
		FldSetAttributes(GetObjectPtr(fldInput), &attr);
		FrmSetFocus (FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(),fldInput));
	}

	return true;
	
}

int disposeInput(GlobalsType *g)
{
  InputPtr tmp;

	while(g->Input != NULL){
		tmp = g->Input;
		g->Input = g->Input->next;
		if(tmp->name)
			Free(tmp->name);
		Free(tmp);
	}		
  	return(0);
}

Boolean formIsInputActive(InputPtr tmp, Int16 x, Int16 y, GlobalsType *g)
{
	if (tmp == NULL)
		return false;
	  
	if ((y >= tmp->y1) && (y <= tmp->y2) &&
			(x >= tmp->x1) && (x <= tmp->x2))
	{
		return true;
	}
	return false;
	
}

/* generic functions */

void formCreateForm(Method method, Char *action, Char *name, GlobalsType *g)
{
	HTMLFormPtr tmp;

	tmp = Malloc (sizeof(HTMLFormType));
	ErrFatalDisplayIf (!tmp, "Malloc Failed");
	MemSet(tmp, sizeof(HTMLFormType), 0);
	tmp->action = Malloc(StrLen(action)+1);
	ErrFatalDisplayIf(!tmp->action, "Malloc failed");
	StrCopy(tmp->action, action);

	tmp->method = method;

	tmp->value = NULL;

	tmp->id = Malloc(StrLen(name)+1);
	ErrFatalDisplayIf(!tmp->id, "Malloc failed");
	StrCopy(tmp->id, name);

	tmp->next = g->Forms;
    	g->Forms = tmp;

	g->ActiveForm = tmp;
}

void formDeactivate(GlobalsType *g)
{
	g->ActiveForm = NULL;
}

Char *formCreateVarName(Char *name, GlobalsType *g)
{
	Char	*ret = NULL;
	Char	*url = NULL;

	url = URLStripFragmentAndQuery(g->Url);
	ErrFatalDisplayIf(!url, "URL edit failed."); // bad method, fix me!!

	if (g->ActiveForm && g->ActiveForm->id) {
		// We have a form id. Create an internal name like:
		// example: form name: search; object name; query; url: http://search.company.com/
		// search|http://search.company.com/#query
		// <form name or id>|<form url>#<form object name> 
		ret = Malloc(StrLen(name) + StrLen(g->ActiveForm->id) + StrLen(url) + 3);
		ErrFatalDisplayIf(!ret, "Malloc failed");

		StrCopy(ret, g->ActiveForm->id);
		StrCopy(ret + StrLen(ret), "|"); // form id '|' url
		StrCopy(ret + StrLen(ret), url);
		StrCopy(ret + StrLen(ret), UNIQUE_ID_SEP_STR);
		StrCopy(ret + StrLen(ret), name);

		/*ret = Malloc(StrLen(name)+StrLen(g->ActiveForm->id)+2);
		ErrFatalDisplayIf(!ret, "Malloc failed");

		StrCopy(ret, g->ActiveForm->id);
		StrCopy(ret + StrLen(ret), UNIQUE_ID_SEP_STR);
		StrCopy(ret + StrLen(ret), name);*/
	} else { // no Form for this to work with, use URL: http://www.google.com/#q
		ret = Malloc(StrLen(name) + StrLen(url) + 2);
		ErrFatalDisplayIf(!ret, "Malloc failed");

		StrCopy(ret, url);
		StrCopy(ret + StrLen(ret), UNIQUE_ID_SEP_STR);
		StrCopy(ret + StrLen(ret), name);

		/*ret = Malloc(StrLen(name)+StrLen(g->Url)+2);
		ErrFatalDisplayIf(!ret, "Malloc failed");

		StrCopy(ret, g->Url);
		StrCopy(ret + StrLen(ret), UNIQUE_ID_SEP_STR);
		StrCopy(ret + StrLen(ret), name);*/
	}

	Free(url);

	return ret;
}

Char *formEatVarName(Char *name, GlobalsType *g)
{
	Int16 i = 0;

	while (name[i] != UNIQUE_ID_SEP_CHAR)
		i++;

	if (i > 0) i++;

	return name + i;
}

void formAddValue(Char *name, Char *initVal, FormInputType type, GlobalsType *g)
{
	NamedValuePtr	tmp, nw, chk;
	Char		*formName;

	// TODO: Using "untitled" as an alternate name might not be good. Generate a name...

	if (g->Forms && g->ActiveForm) {
		chk = g->Forms->value;
		while(chk != NULL)
		{
			formName = formCreateVarName((name) ? name : "untitled",g);
			if (StrCompare(formName,chk->name)==0)
			{
				Free(formName);
				return; // We found a matching name, do not create a new....
			}
			Free(formName);
			chk = chk->next;
		}

		nw = Malloc (sizeof(NamedValueType));
		ErrFatalDisplayIf (!nw, "Malloc Failed");
		MemSet(nw, sizeof(NamedValueType), 0);

		formName = formCreateVarName((name) ? name : "untitled",g);
		nw->name = Malloc(StrLen(formName)+1);
		ErrFatalDisplayIf(!nw->name, "Malloc failed");
		StrCopy(nw->name, formName);
		Free(formName);

		if (initVal) {
			nw->initval = StrDup(initVal);
		} else {
			nw->initval = NULL;
		}

		nw->type = type;
		nw->next = NULL;

		if (type == HIDDEN) {
			nw->value = Malloc(StrLen((initVal) ? initVal:"")+1);
			ErrFatalDisplayIf(!nw->value, "Malloc failed");
			StrCopy(nw->value, (initVal) ? initVal:"");
		} else {
			formName = formCreateVarName((name) ? name : "untitled",g);
			nw->value = Malloc(StrLen(formName)+3);
			ErrFatalDisplayIf(!nw->value, "Malloc failed");
			nw->value[0] = EXT_I_2; // EXT_I_0;
			StrCopy(nw->value + 1, formName);
			nw->value[StrLen(formName) + 1] = END;
			nw->value[StrLen(formName) + 2] = '\0';
			Free(formName);
		}

		tmp = g->Forms->value;
		if (tmp == NULL) {
			g->Forms->value = nw;
		} else {
			while(tmp->next != NULL) {
				tmp = tmp->next;
			}
			tmp->next = nw;
		}
	}
}

Char *FormGetInitValue(Char *name, GlobalsType *g)
{
	NamedValuePtr	chk;


	if (g->Forms) {
		chk = g->Forms->value;
		while(chk != NULL) {
			if (StrCompare(name,chk->name)==0) {
				return chk->initval;
			}
			chk = chk->next;
		}
	}

	return NULL;
}

Boolean FormSetInitValue(Char *name, Char *value, GlobalsType *g)
{
	NamedValuePtr	chk;

	if (!name || !value)
		return false;

	if (g->Forms) {
		chk = g->Forms->value;
		while(chk != NULL) {
			if (StrCompare(name,chk->name)==0) {
				if (chk->initval)
					Free(chk->initval);

				chk->initval = StrDup(value);

				return true;
			}
			chk = chk->next;
		}
	}

	return false;
}

HTMLFormType *FormGetAssociatedForm(void *objectP, FormInputType type, HTMLFormType *formP)
{
	HTMLFormType	*temp  = NULL;
	NamedValueType	*name  = NULL;
	InputType	*input = NULL;

	if (!objectP || !formP)
		return NULL;

	temp = formP;

	while (temp) {
		name = temp->value;

		while (name) {
			switch (type) {
				case TEXT:
					input = (InputType *)objectP;

					if (!StrNCompare(input->name, name->name, StrLen(input->name))) {
						return temp;
					}
					break;
				case SELECT:
				case CHECK:
				case BUTTON:
				default:
					break;
			}

			name = name->next;
		}
		
		temp = temp->next;
	}

	return NULL;
}


int disposeFormValues(NamedValuePtr values, GlobalsType *g)
{
	NamedValuePtr	tmp;

	while(values != NULL){
		tmp = values;
		values = values->next;

		if(tmp->name)
			Free(tmp->name);
		if(tmp->value)
			Free(tmp->value);
		if(tmp->initval)
			Free(tmp->initval);
		Free(tmp);
	}		
  	return(0);
}

Boolean FormSubmitForm(HTMLFormType *formP, GlobalsType *g)
{
	Char	*newUrl = NULL;

	if (!formP)
		return false;

	if (formP->method == POST) {
		g->post = true;
		if (g->postBuf) Free(g->postBuf);
		g->postBuf = URLCreateHTMLEncodedURL(NULL, formP->value, NULL, g); // postBuf should be freed in post code
		g->cache.cacheDoNotRead = true;
		followLink(formP->action, g);

		return true;
	} else if (formP->method == GET) {
		g->post = false;
		newUrl = URLCreateHTMLEncodedURL(formP->action, formP->value, NULL, g);
		g->cache.cacheDoNotRead = true;
		followLink(newUrl,g);
		Free(newUrl);

		return true;
	}

	return false;
}

Boolean FormSetFirstSubmitButton(HTMLFormType *formP, ButtonInputPtr buttons, GlobalsType *g)
{
	ButtonInputPtr	tmp = buttons, submit = NULL;

	while (tmp) {
		if ((tmp->form == formP) && (tmp->type == SUBMIT)) {
			submit = tmp;
		}
		tmp = tmp->next;
	}

	if (submit) {
		setVar(submit->name, submit->value, g);
		return true;
	}

	return false;
}

int disposeForms(GlobalsType *g)
{
  HTMLFormPtr tmp;

	while(g->Forms != NULL){
		tmp = g->Forms;
		g->Forms = g->Forms->next;
		if(tmp->action)
			Free(tmp->action);
		if(tmp->id)
			Free(tmp->id);
		disposeFormValues(tmp->value,g);
		Free(tmp);
	}		
  	return(0);
}

void destroyForms(GlobalsType *g)
{
	disposeRadioButton(g);
	disposeCheckBox(g);
	disposeButton(g);
	disposeForms(g);

	// TODO: add below??
	//disposeInput(g);
	//disposeSelect(g);
}

