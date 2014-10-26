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
#ifndef _FORM_H_
#define _FORM_H_

#include 	<PalmOS.h>
#include 	"WAPUniverse.h"

// This is the character that the form object name parser uses to separate the unique
// identifier and the object name. Both must match.
#define UNIQUE_ID_SEP_STR	"#"
#define	UNIQUE_ID_SEP_CHAR	'#'

extern int formDrawButton(Char *label, UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g);
extern void formDrawEmptyRadio(UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g);
extern void formDrawFullRadio(UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g);
extern void formDrawUnCheckedCheckbox(UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g);
extern void formDrawCheckedCheckbox(UInt16 *x, UInt16 *y, Boolean draw, GlobalsType *g);

extern void formCreateInputArea(Char *fieldName, Boolean disabled, Int32 columns, Int32 rows, Char *border, Char *background, GlobalsType *g);

extern void formCreateRadioButton(Char *name, Char *value, Boolean checked, Boolean disabled, GlobalsType *g);
//extern Boolean checkRadioButton(Int16 x, Int16 y, GlobalsType *g);
extern Boolean checkRadioButton(RadioInputPtr radio, GlobalsType *g);
extern void formSelectRadioButton(Char *name, Char *value, GlobalsType *g);
extern int disposeRadioButton(GlobalsType *g);

extern void formCreateCheckBox(Char *name, Char *value, Boolean checked, Boolean disabled, GlobalsType *g);
//extern Boolean checkCheckBox(Int16 x, Int16 y, GlobalsType *g);
extern Boolean checkCheckBox(CheckInputPtr check, GlobalsType *g);
extern void formUnCheckBox(Char *name, Char *value, GlobalsType *g);
extern void formCheckBox(Char *name, Char *value, GlobalsType *g);
extern int disposeCheckBox(GlobalsType *g);

extern void formCreateButton(Char *name, Char *value, ButtonType type, int width, int height, Boolean disabled, Boolean image, GlobalsType *g);
//extern Boolean checkFormButton(Int16 x, Int16 y, GlobalsType *g);
extern Boolean checkFormButton(ButtonInputPtr button, GlobalsType *g);
extern int formReDrawButton(Char *label, GlobalsType *g);
extern int disposeButton(GlobalsType *g);

extern void FormInputSetType(InputType *inputP, InputFieldType type);
extern void FormInputSetReadonly(InputType *inputP);
extern void palmWmlInput(Char *fieldName, Boolean disabled, Int16 size, Int32 maxLength, GlobalsType *g);
extern void palmWmlInputType(Boolean pwdType, GlobalsType *g);
extern void palmWmlInputValue(Char *value, GlobalsType *g);
extern void FormInputDeactivate(GlobalsType *g);
//extern Boolean checkInput(Int16 x, Int16 y, GlobalsType *g);
extern Boolean checkInput(InputPtr input, GlobalsType *g);
extern int disposeInput(GlobalsType *g);
extern Boolean formIsInputActive(InputPtr tmp, Int16 x, Int16 y, GlobalsType *g);

extern Boolean selectInitList (SelectPtr slct, GlobalsType *g);
extern void palmWmlSelectOptionTxt(Char *txt, GlobalsType *g);
extern void palmWmlSelectOptionOnPick(Char *proto, Char *url, GlobalsType *g);
extern void palmWmlSelectOptionPost(Boolean post, GlobalsType *g);
extern void palmWmlSelect(Char *name, Boolean disabled, GlobalsType *g);
extern void palmWmlSelectValue(Char *ivalue, GlobalsType *g);
extern void palmWmlSelectOption(Char *value, GlobalsType *g);
extern void selectDrawText(SelectPtr slct, GlobalsType *g);
extern void palmWmlSelectEnd(GlobalsType *g);
//extern Boolean checkSelect(Int16 x, Int16 y, GlobalsType *g);
extern Boolean checkSelect(SelectPtr select, GlobalsType *g);
extern int disposeOptions(OptionPtr options, GlobalsType *g);
extern int disposeSelect(GlobalsType *g);
extern void selectDrawTrigger(Int16 x, Int16 y);

extern void formCreateForm(Method method, Char *action, Char *name, GlobalsType *g);
extern void formDeactivate(GlobalsType *g);
extern HTMLFormType *FormGetAssociatedForm(void *objectP, FormInputType type, HTMLFormType *formP);
extern Boolean FormSubmitForm(HTMLFormType *formP, GlobalsType *g);
extern Boolean FormSetFirstSubmitButton(HTMLFormType *formP, ButtonInputPtr buttons, GlobalsType *g);
extern Char *formCreateVarName(Char *name, GlobalsType *g);
extern Char *formEatVarName(Char *name, GlobalsType *g);
extern void formAddValue(Char *name, Char *initVal, FormInputType type, GlobalsType *g);
extern Char *FormGetInitValue(Char *name, GlobalsType *g);
extern Boolean FormSetInitValue(Char *name, Char *value, GlobalsType *g);
extern int disposeFormValues(NamedValuePtr values, GlobalsType *g);
extern int disposeForms(GlobalsType *g);
extern void destroyForms(GlobalsType *g);

extern void InsertFromWallet(void);

// 

#endif
