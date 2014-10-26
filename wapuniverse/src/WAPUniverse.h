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
#ifndef _WAPUNIVERSE_H_
#define _WAPUNIVERSE_H_

#include 	<PalmOS.h>
#include 	<SysEvtMgr.h>
#include 	<MemGlue.h>
#include 	<sys_socket.h>
#include	<VFSMgr.h>
#include 	"iscript.h"
#include	"resize.h"
#include    "MemMgr.h"

#define SEC_1 // __attribute__ ((section ("sec_1"))) // as of the new 2.0 this is no longer needed

#define appFileCreator            	'Wapu'
#define appVersionNum              	0x01
#define appPrefID                  	0x00
#define appPrefVersionNum          	0x01

#define dbPageDBType   			'Page'
#define dbPageDBName			"Universe-PageFile"

// sysAppLaunch...
#define sysAppLaunchCmdGotoHomepage	(sysAppLaunchCmdCustomBase|0x01)
#define sysAppLaunchCmdGotoBlank	(sysAppLaunchCmdCustomBase|0x02)
#define sysAppLaunchCmdViewBookmarks	(sysAppLaunchCmdCustomBase|0x03)
#define sysAppLaunchCmdViewConnSet	(sysAppLaunchCmdCustomBase|0x04)

#define	ON	true
#define	OFF	false

#define ABS(a)     (((a) < 0) ?  -(a) : (a))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#define version40 			0x04000000  		// PalmOS 4.0 version number
#define version50			0x05000000		// PalmOS 5.0 version number
#define wuAppType     			'Wapu'      		// type for application.  must be 4 chars, mixed case.
#define noRecordSelected    		-1          		// index for no current record.
#define localizedAppInfoStr 		1000    		// default category strings ** tAIS **

#define ONE_KILOBYTE			1024

// Browser Communications
#define REPLY_TIMEOUT    		g_prefs.timeout 	// 60
#define NET_BLOCK_TIMEOUT		1 			// orig: 4 This timeout is used as the AppNetTimeout, in seconds
#define NET_CONNECT_TIMEOUT		10			// used by functions such as NetLibSocketConnect, currently unused
#define MAX_URL_LEN 			(2 * ONE_KILOBYTE) 	//998 // old: 255
#define MAX_REDIRECT_COUNT		8			// This defines how many times the browser can be redirected in succession

// Tab options
#define UNLIMITED_TABS      255         // The magic number to set unlimited tabs
#define MAX_TABS			3			// If this is set to 255 then support unlimited tabs

// Formatting options
#define	indentMarginValue		10
//#define indentMargin			BrowserIndentValue()

//sent in FrmUpdateEvent
#define frmUpdateGotoUrl 	0x09
#define	frmUpdateMode		0x10
#define frmUpdateNewTab		0x11

// Custom events
#define mouseOverEvent		(sysEventFirstUserEvent + 1)

#define Malloc(x) 		MemMgrChunkNew(x)
#define Free(x) 		MemMgrChunkFree(x) 		// if (x != NULL) MemPtrFree(x)
// x = ptr, y = dest size
#define Resize(x,y)		if((MemPtrSize(x) > y) && (x != NULL)) MemPtrResize(x,y)

//For Jpeg library
extern UInt16 jpegLibRef;

//For WMLScript Library
extern UInt16 		gScriptRef;
extern ScriptHost	scriptHost;    // scripting engine host API
extern Boolean		scriptWarned;

//for GIF Lib
extern UInt16		gGifRef;

//For internal HTTP library
extern UInt16 HttpLibRef; // if 0, library could not be loaded

static const Char gPhoneChars[] = "0123456789,+#*";

#define BROWSER_ABOUT_BLANK	"about:blank"
#define UNIVERSE_UA_PROF	"http://wap2.wapuniverse.com/profiles/universe3.xml"
#define DEFAULT_DOWNLOAD	"/Downloads/"

#define MODE_BROWSER	0
#define MODE_ONEHAND	1

#define START_BLANK     0
#define START_HOME      1
#define START_BOOK      2
#define START_LASTURL   3

// states for FormBrowser Statemachine
#define BS_NULL			0
#define BS_IDLE         	1
#define BS_GET_URL      	2
#define BS_GET_REPLY    	3
#define BS_WSP_DECODE   	4
#define BS_WBXML_PARSE  	5
#define BS_RENDER		6
#define BS_NEW_CARD     	7
#define BS_ONTIMER		8
#define BS_GET_OFFLINE  	9
#define BS_GET_LOCALFILE	10
#define BS_WTP_REGET_URL  	11
#define BS_WTP_CONNECT  	12
#define BS_WTP_GETCONNECT	13
#define BS_XHTML_PARSE_RENDER	14
#define BS_XHTML_ONTIMER	15
#define BS_HTTP_GET		16
#define BS_HTTP_RECEIVE		17
#define BS_HTTP_HANDLE		18
#define BS_RSS_PARSE_RENDER	19
#define BS_CHECK_CACHE		20
#define BS_DOWNLOAD_IMG		21
#define BS_ALT_DOWNLOAD_IMG	22
#define BS_DOWNLOADING_IMG	23
#define BS_DECODING_IMG		24
#define BS_UPDATE_IMG		25
#define BS_DOWNLOAD_STOP	26
#define BS_COMPLETE_WML		27
#define BS_COMPLETE_HTML	28

// states for WTP
#define WTP_Connect		1
#define WTP_Connecting		2
#define WTP_Connected		3
#define WTP_Disconnected	4
#define WTP_Aborted		5


extern void MyErrorFunc (char* error, char* additional);
extern void DisplayDial(const Char *iDialNumberP, int callType);
extern void DisplayPushHelp();
extern Boolean PrvDialListCanBePhoneNumber( Char* text, Int16 textLen );
#define DisplayError(str)	MyErrorFunc(str,__FILE__);  




#define ftrGlobals		1000
#define ftrPageFile		2000	// currently not used
#define ftrCallbackTable	3000	// currently not used
#define ftrMainArmlet		4000	// currently not used
#define ftrSysA4		5000
#define ftrSysA5		6000
#define ftrThreadA4		7000
#define ftrThreadA5		8000

// history.c
// history: the stack containing the URL history
#define MAX_HISTORY	  20 // min. 10 cfr SPEC-WML 12.2.1



enum _Method
{
	GET, POST
};
typedef enum _Method Method;

typedef struct historyEntryType {
  	Char	*url;
	Char	*postData;
	Method	method;
	Char	*title;
  	// TODO : add cache location and exporation here as well
} historyEntryType;

typedef struct historyType {
  	historyEntryType	hist[MAX_HISTORY]; 
  	int			index;
  	int 			count;
} historyType;

typedef struct historyType *historyPtr;


typedef struct OfflineType {
    	DmOpenRef       mtchDb;
    	UInt16          mtchVersion;
    	UInt16		record;
    	Char            channel[32];
} OfflineType;

typedef struct OfflineType *OfflinePtr;


// WML.C
// CardType: used to  build an internal list of all cards in a deck
typedef struct CardType {
  	struct CardType	*next; 
  	Char		*card_id;
  	UInt32 		offset;
} CardType;

typedef struct CardType *CardPtr;


typedef struct WMLDeckStruct *WMLDeckPtr;

typedef struct WMLDeckStruct {
  	unsigned char 	*ontimerUrl;  
  	unsigned char	*data;
  	// temp vars, put here to avoid stack overflows.
  	unsigned char 	*attrstr;
  	unsigned char	*tmpstr;
  	char   		*strTable;
  	unsigned char 	*stack;
  	CardPtr		card;
  	UInt32		decksize;
  	UInt32		offset;
  	UInt32		stackDepth;
  	UInt32		maxstack;
  	unsigned char 	tag;
  	int 		version;
  	int 		publicID;
  	int 		charSet;
  	int		action;
  	int		template;
  	int		timerVal;   // for ontimer event, -1 if no timer
} WMLDeck ;

// dbPush.c
// Push record	
typedef struct {
    	char    url[MAX_URL_LEN+1];
    	char    message[200];
    	char	from[31];
    	char	date[31];
} dbPushMessage;


typedef dbPushMessage * dbPushMessagePtr;

typedef struct {
    	char    name[1];    // actually may be longer than 1
} dbPushPackedMessage;

// dbURL.c
// Connection Settings record	
typedef struct {
    	char    urlstr[MAX_URL_LEN+1];
    	char    name[31];
    	char    connection[16];
} dbUrlURL;


typedef dbUrlURL * dbUrlURLPtr;

typedef struct {
    	char    name[1];    // actually may be longer than 1
} dbUrlPackedURL;



// Connection Settings record, mostly used in dbConn.c
typedef struct {
    	char    ipaddress[255];      // e.g. 172.116.212.123 
    	char	home[300];
    	char    proxyuser[63];
    	char    proxypass[63];
    	char	name[31];
    	char	key[9];
    	char	port[6];
    	char    security;           // 'Y'es or 'N'o
    	char    bearer;             // 'C'SD or 'S'MS
    	char    connectionType;     // C'O' or C'L' 1 or 2
    	Boolean lock;
    	Boolean	securitylst;
} dbConnConnection;

typedef dbConnConnection * dbConnConnectionPtr;

typedef struct {
    	char    name[1];    // actually may be longer than 1
} dbConnPackedConnection;

  
typedef struct {
    	UInt16		renamedCategories;        // bitfield of categories with a changed name
    	Char		categoryLabels[dmRecNumCategories][dmCategoryLength];
    	UInt8    	categoryUniqIDs[dmRecNumCategories];
    	UInt8    	lastUniqID;         // Uniq IDs for categories generated by the device 
    	// are between 0 - 127.  Those from the PC are 128 - 255.
    	UInt8    	reserved1;          // from the compiler UInt16 aligning things
    	UInt16   	reserved2;
} gWAPAppInfoType;

typedef gWAPAppInfoType * gWAPAppInfoPtr; 

typedef struct VarType {
  	struct VarType 	*next;
  	Char 		*name;
  	Char 		*value;
} VarType;

typedef VarType *VarPtr	;

typedef struct HyperLinkType {
  	struct HyperLinkType 	*next;
  	Char 			*url;
	Char			*id;
  	VarPtr 			linkVar;
  	VarPtr 			postVar;
  	Int16 			x1;
  	Int16 			y1;
  	Int16 			x2;
  	Int16 			y2;
  	Boolean 		post;
} HyperLinkType;

typedef HyperLinkType *HyperLinkPtr;

typedef struct AreaTagType {
	struct AreaTagType	*next;
	Char			*href;
	Char			*alt;
	RectangleType		area;
} AreaTagType;

typedef AreaTagType *AreaTagPtr;

typedef struct MapType {
	struct MapType		*next;
	Char			*name;
	AreaTagPtr		area;
} MapType;

typedef MapType *MapPtr;

typedef struct ImageType {
  	struct ImageType 	*next;
	Char			*location;
	Char			*alt;
	Char			*mapName;
	unsigned char		*channel;
	BitmapPtr		bitmap;
	HyperLinkPtr		link;
  	Int16 			x1;
  	Int16 			y1;
  	Int16 			x2;
  	Int16 			y2;
	UInt16			width;
	UInt16			height;
// for coordinate scaling
	UInt16			maxWidth;
	UInt16			origWidth;
	Boolean			alreadyHires;
	Boolean			download;
	Boolean			virtual; // it true, the image shares a ptr with another image
} ImageType;

typedef ImageType *ImagePtr;

typedef enum InputFieldType {
	inputTypeNone,
	inputTypeText,
	inputTypePassword,
	inputTypeTextArea
} InputFieldType;

typedef struct InputType {
  	struct InputType	*next;
  	Char 			*name;
  	Char 			*format;
  	Int16 			x1;
  	Int16 			y1;
  	Int16 			x2;
  	Int16 			y2;
  	Int16 			size;
  	Int32 			maxlength;
	UInt16			border;
	IndexedColorType 	bkgrndColor;	// for color
	Boolean			bkgrndTransparent;
  	Boolean 		passwordType;  // true = Password/ false = text
	Boolean			disabled;
	Boolean			readonly;
  	Boolean 		emptyOK;
	InputFieldType		type;
} InputType;

typedef InputType *InputPtr;

typedef struct OptionType {

  	struct OptionType 	*next;
  	Char			*value;
  	Char 			*txt;
  	Char 			*onpickUrl;
  	VarPtr 			optVar;
  	VarPtr			postVar;
  	Boolean 		post;
} OptionType;

typedef OptionType *OptionPtr;

typedef struct SelectType {
  	struct SelectType 	*next;
  	Char 			*name;
  	OptionPtr 		options;
  	Int16 			x1;
  	Int16 			y1;
  	Int16 			x2;
  	Int16 			y2;
	Boolean			disabled;
} SelectType;

typedef SelectType *SelectPtr;

// 

typedef struct WebDataType {
	// Shared
  	unsigned char 	*data;
	unsigned char	*contentType;
    unsigned char   *charSet;
    unsigned char   *boundry;
  	UInt32          length;

	// WSP
  	unsigned char 	transaction_id;
  	unsigned char 	pdu_type;
  	unsigned char 	status_code;
} WebDataType;

typedef WebDataType * WebDataPtr;

typedef struct PushType {
  	unsigned char 	from;
  	unsigned char 	date;
  	unsigned char 	url;
  	unsigned char 	message;
} PushType;

typedef PushType *PushPtr;

typedef struct SARType {
  	struct SARType 	*next;	// More packets
  	char 		*data;	// The data
  	UInt32 		size;	// length of data
  	UInt32 		pos;	// Packet position, so we can receive out of order packets?
  	Boolean 	last;	// Tells if it is the last packet
} SARType;

typedef SARType *SARPtr;

// Forms

enum _FormInputType
{
	TEXT, RADIO, CHECK, BUTTON, SELECT, HIDDEN
};
typedef enum _FormInputType FormInputType;

enum _ButtonType
{
	SUBMIT, RESET
};
typedef enum _ButtonType ButtonType;


typedef struct NamedValueType {
  	struct NamedValueType	*next;
  	Char 			*name;
  	Char 			*initval;	// for form resetting, will not be changed
	Char			*value;
	FormInputType		type;		// for future use, use to determine if value should be changed
} NamedValueType;

typedef NamedValueType *NamedValuePtr;

typedef struct HTMLFormType {
  	struct HTMLFormType	*next;
	Char 			*action;
	Char			*id;
  	NamedValuePtr		value;
	Method			method;
} HTMLFormType;

typedef HTMLFormType *HTMLFormPtr;

typedef struct RadioInputType {
  	struct RadioInputType	*next;
  	Char 			*name;
  	Char 			*value;
  	//Char 			*initval;
	//struct NamedValueType	*value;
  	Int16 			x1;
  	Int16 			y1;
  	Int16 			x2;
  	Int16 			y2;
	Boolean			checked;
	Boolean			disabled;
} RadioInputType;

typedef RadioInputType *RadioInputPtr;

typedef struct CheckInputType {
  	struct CheckInputType	*next;
  	Char 			*name;
  	Char 			*value;
  	//Char 			*initval;
	//struct NamedValueType	*value;
  	Int16 			x1;
  	Int16 			y1;
  	Int16 			x2;
  	Int16 			y2;
	Boolean			checked;
	Boolean			disabled;
} CheckInputType;

typedef CheckInputType *CheckInputPtr;

typedef struct ButtonInputType {
  	struct ButtonInputType	*next;
  	Char 			*name;
  	Char 			*value;
	HTMLFormPtr		form;
	//struct NamedValueType	*value;
  	Int16 			x1;
  	Int16 			y1;
  	Int16 			x2;
  	Int16 			y2;
	ButtonType		type;
	Boolean			disabled;
	Boolean			image;
} ButtonInputType;

typedef ButtonInputType *ButtonInputPtr;


enum _Alignment
{
	Left, Center, Right, None
};
typedef enum _Alignment Alignment;


typedef struct AreaType {
	struct AreaType		*next;
	IndexedColorType 	color;
	UInt16			x1;
	UInt16			y1;
	UInt16			x2;
	UInt16			y2;
	BitmapPtr		image;
	Boolean			noRefresh; // no color
} AreaType;

typedef struct AreaType *AreaPtr;

typedef struct BackgroundType {
	struct BackgroundType	*next;
	AreaPtr			areaPtr;
	IndexedColorType 	pageBG;
	Boolean			noRefresh;
} BackgroundType;

typedef struct BackgroundType *BackgroundPtr;

/* XHTML structures */

typedef struct XHTMLStruct {
  	//unsigned char		*data;		// Full xHTML page in memory
	UInt32			length;		// length of data
	Char 			*buffer;
	unsigned char 		*ontimerUrl;
	BackgroundPtr		background;
	IndexedColorType 	pageBG;
	int			action;
  	int			timerVal;
	// put parser globals below...
	Boolean 		textAvail;
	Boolean 		end;
	Boolean 		paragraph;
} XHTML;

typedef struct XHTMLStruct *XHTMLPtr;

/*   */

typedef struct AlignType {
	struct AlignType	*next;
	Alignment		currentAlignment;
} AlignType;

typedef struct AlignType *AlignPtr;


#define NUMERIC		1
#define LWR_ALPHA	2
#define UPPR_ALPHA	3
#define ROMAN		4

typedef struct OLType {
  	struct OLType 	*next;
	UInt32		indent;
  	Int16		olCount;
	Int16		olStart;
	Int16		olType;
	Boolean		isUL; // should be a union
	Boolean		isOL;
} OLType;

typedef OLType *OLPtr;


enum _RssItemType
{
	TOP, ITEM
};
typedef enum _RssItemType RssItemType;

typedef struct RSSType {
	struct RSSType	*next;
	Char		*title;
	Char		*link;
	Char		*description;
	Char		*imageTitle;
	Char		*imageUrl;
	Char		*imageLink;
	Char		*date;
	RssItemType	type;
} RSSType;

typedef struct RSSType *RSSPtr;


#define READ_BUF_SIZE (2048)

typedef enum ParseState_enum {
	PS_ResponseLine,
	PS_Headers,
	PS_Body,
	PS_Done,
	PS_Error//,
	//PS_UserAbort
} ParseState;

typedef struct HTTPParse_struct {
    unsigned long 	contentLength;
    unsigned long 	contentRead;
    char            *saveFileName;
    Char            *url;
    void            *fd;
    UInt16          bufferPos;
	ParseState      state;
    char            readBuffer[READ_BUF_SIZE];
	unsigned int 	responseCode;
    Int8            endOfStream;
    Int8            needData;
    Boolean         image;
    WebDataPtr      webData;
} HTTPParse;

typedef struct HTTPParse *HTTPParsePtr;

typedef struct CookieType {
	struct CookieType	*next;
	Char			*host;
	Char			*path;
	Char			*name;
	Char			*value;
	Char			*expires;
	Char			*maxage;
	Char			*version;
	Boolean			secure;
	Boolean			tailmatch;
	Boolean			private;
} CookieType;

typedef CookieType *CookiePtr;


typedef struct CacheType {
	Int16		version;
	DmOpenRef	DbCache;
  	Boolean		cacheDoNotRead;
	Boolean		haveCacheFile;
	Boolean		tryCache;
} CacheType;

typedef CacheType *CachePtr;

typedef struct WebFontType {
  	struct WebFontType	*next;
	UInt16			res;
	FontID			font;
	IndexedColorType 	color;
	Boolean			italic;
	Boolean			underlined;
	Boolean			superScript;
	Boolean			subScript;
	Boolean			strike;
} WebFontType;

typedef WebFontType *WebFontPtr;

typedef enum TextDirection {
	directionLTR,
	directionRTL
} TextDirection;

typedef struct TextType {
	struct TextType		*next;
	HyperLinkPtr		link;
	Char			*text;
	UInt16			res;
	IndexedColorType 	color;
	FontID			font;
	TextDirection		direction;
	Boolean			italic;
	Boolean			underlined;
	Boolean			superScript;
	Boolean			subScript;
	Boolean			strike;
} TextType;

typedef TextType  *TextPtr;

#define DOM_UI_NULL		0
#define DOM_UI_TEXT		1
#define DOM_UI_INPUT 		2
#define DOM_UI_SELECT		3
#define DOM_UI_IMAGE		4
#define DOM_UI_BUTTON		5
#define DOM_UI_RADIO		6
#define DOM_UI_CHECK		7
#define DOM_UI_AREA		8

typedef struct DOMType {
	struct DOMType		*next;

	union {
		InputPtr	i;
		SelectPtr	s;
		RadioInputPtr	r;
		CheckInputPtr	c;
		ButtonInputPtr	b;
		ImagePtr	p;
		TextPtr		t;
		AreaPtr		a;
	} ptr;

	UInt16		type;
	RectangleType	bounds;
} DOMType;

typedef DOMType *DOMPtr;


typedef struct HorizIndentType {
  	struct HorizIndentType	*next;
	UInt16			x;
	UInt16			y;
	UInt16			h;
	UInt16			imgH;
	Alignment		align;
} HorizIndentType;

typedef HorizIndentType *HorizIndentPtr;

typedef struct PositionType {
  	struct PositionType	*next;
	UInt16			x;
	UInt16			y;
	UInt16			h;
	UInt16			indent;
	Boolean			null;
} PositionType;

typedef PositionType *PositionPtr;


typedef struct BrowserGlobalsType {
	Char		*browserTitle;
	UInt32 		savedclength;
	UInt32 		savedrlength;
	Int32 		taptick;
	Int32 		tapx;
	Int32 		tapy;
	Int32 		nextTick;
	WinHandle 	saved;
	int 		retransCount;
	int		taptype;
	Boolean 	imgYAdd;
	Boolean 	optMenuOpen;
	Boolean 	tapandhold;
	Boolean 	updateMenu;
	Boolean 	redrawMenuBar;
	Boolean		textDirRTL; // TODO: change to textDirTranspose
	Boolean		haveAlt;
} BrowserGlobalsType;

typedef BrowserGlobalsType *BrowserGlobalsPtr;


typedef struct BrowserAPIType {
	UInt16			BrowserWidget;
	UInt16			lastWidgetWidth;
	UInt16			BrowserInputField;
	UInt16			BrowserOptionsList;
	UInt16			BrowserScrollBar;
	UInt16			BrowserScrollBarHorizontal;
} BrowserAPIType;

typedef BrowserAPIType *BrowserAPIPtr;


typedef struct PageStatsType {
	UInt32			timePageLoadStart;
	UInt32			timePageLoadFinish;
	UInt32			pageSize;
	UInt32			savedtimePageLoadStart;
	UInt32			savedtimePageLoadFinish;
	UInt32			savedPageSize;
    UInt32          totalMemoryUsed;
} PageStatsType;

typedef enum PointerMode {
	pointerModeNone,
	pointerModeWait,
	pointerModeHover,
	pointerModeLast		/* Leave at end!!*/
} PointerMode;

typedef enum PointerState {
	pointerScrolling,
	pointerNavigating
} PointerState;

typedef struct PointerImageType {
	BitmapPtr	bitmap;
	unsigned char	*channel;
} PointerImageType;

typedef struct WindowPointerType {
	Coord			x;
	Coord			y;
	UInt16			space; // width of pointer
	UInt16			speed; // x or y + speed to determine new pos
	Int8			lastLr; // -1 = left, 1 = right, 0 = no change
	Int8			lastTb; // -1 = top, 1 = bottom, 0 = no change
	Int8			flipCount; // scrolling up,dwn,up would cause page scrolling to stop
	Boolean			holding;
	PointerMode		mode;
	PointerState		lastState;
	PointerImageType	image[pointerModeLast];
} WindowPointerType;

typedef WindowPointerType *WindowPointerPtr;

typedef struct {
	MemHandle    		resBmpPrgrs[14];
    MemHandle           srcHandle;
	MemHandle       	ConnectionsHandle;
	MemHandle       	ConnectionsListArrayH;
	FileRef             xhtmlOutputFileRef;
	FileRef             debugOutputFileRef;
	UInt32              CardOffset;
	UInt32              indent;
	UInt32              xindent;
	UInt32              fontH;
	UInt32              rightIndent;
	UInt32              contentLength;
	UInt32              timeElapse;
	UInt32              sid;
	UInt32              imgH;
	UInt32              imgY;
	UInt32              pushedPageLength;
	UInt32              gifContext;
	//UInt32			registerA4;
	//UInt32			registerA5;
	unsigned char 		*urlBuf;
	unsigned char 		*contentType;
	unsigned char 		*pageContentType;
	unsigned char 		*launchURL;
	char         		*inBuf;
	Char                *Url;
	Char                *EditURLConnection;
	Char                *OpenUrl;
	Char                *deckTitle;
	Char                *postBuf;
	Char                *AuthUser;
    Char                *AuthPass;
	Char                *onforward;
	Char                *partBound;
	Char                *rssFeed;
	Char                *pushedPageContent;
	Char                *pushedPageContentType;
	Char                *pushedPageTitle;
	Char                *wmlsData;
	void                *pngLibrary;
	InputPtr        	InputFldActive;
	InputPtr            Input; 				// linked list of input elements
	VarPtr              Var; 				// linked list of variables
	SelectPtr           Select;
	RadioInputPtr		radioBtn;
	CheckInputPtr		checkBox;
	HTMLFormPtr         Forms;
	HTMLFormPtr         ActiveForm;
	ButtonInputPtr		formButton;
	ImagePtr            inlineImage;
	MapPtr              ImageMaps;
	RSSPtr              rss;
	RSSPtr              tempRss;
	CardPtr             htmlPageLocID;
	ImagePtr            pageImages;
	ImagePtr            downloadImage;
	OLPtr               organizedList;
	WebFontPtr          font;
	SARPtr              segment;
	HorizIndentPtr		horizIndent;
	PositionType		*position;
	CookiePtr           CookieJar;
	HyperLinkPtr     	HyperLinks; 			// linked list of hyperlinks
	HyperLinkPtr		currentLink;
	TextPtr             text;
	DOMPtr              DOM;
	AreaPtr             areaPtr;
	HyperLinkPtr		selectedLink;
	PushPtr             Push; 
	AlignPtr            AlignList;
	historyPtr          History;
	DmOpenRef      		DbHistory;
	DmOpenRef           DbCookieJar;
	DmOpenRef    		DbUrlDb;           		// handle for URL database (bookmarks)
	DmOpenRef    		DbConnDb;          		// handle for Connection database 
	UInt16       		CurrentView;       		// id of current form
	UInt16              nextView;
	UInt16            	CurrentCategory;  		// category filter
	UInt16           	RecordCategory;   		// edited record's category (never All)
	UInt16           	NewCategory;    		// category in details form (never All)
	UInt16              charSet;
	UInt16              prevFormID;
	UInt16              positionIndent;
	UInt16              redirects;
	UInt16              DbUrlCurrentCategory;
	Int16        		progress;
	Int16        		DrawHeight;
	Int16               DrawWidth;
	Int16   	     	ScrollY;
	Int16               ScrollX;
	Int16        		x,y;
	Int16        		state;
	Int16        		sock;		// true if category name has been edited
	Int16               DbConnCurrentRecord;       	// index of current Connection database record
	Int16        		DbUrlCurrentRecord;       	// index of current URL database record
	Int16        		DbPushCurrentRecord;
	Int16               needSpace; 			// render an extra space
	Int16               onEvent;
	Int16        		wtpState;
	Int16               ActiveConnStore;
    unsigned short      tid;
    unsigned short      old_tid;
	char                WapLastUrl[MAX_URL_LEN + 1];
	char                CookieLastUrl[MAX_URL_LEN + 1];
	char         		DbUrlCategoryName[dmCategoryLength];     // Storage for the name
	char           		ActiveConnection[16];
	struct sockaddr_in  RmtAddress;
	struct sockaddr_in 	LclAddress;
	WebDataType       	webData;
	BrowserGlobalsType	browserGlobals;
	BrowserAPIType		BrowserAPI;
	OfflineType         offline;
	XHTML               xhtml;
	HTTPParse           httpParser;
	CacheType           cache;
	Alignment           Align;
	Alignment           imgAlign;
	dbPushMessage     	DbPushCurrentMessage;
	dbConnConnection 	CurrentConnection;
	dbConnConnection	conn;
	dbUrlURL     		DbUrlCurrentURL;
	WMLDeck         	wmldck;
	PageStatsType		pageStats;
	WindowPointerType	pointer;
	UInt8               tabList[8];
	gWAPAppInfoType 	WAPAppInfo;    	// for the connectionslist in the URLEditForm
	SndStreamRef		imageThreadRef;
	Int8                imageThreadLoadCount;
	int                 tid_new;
	Boolean             Render;
	Boolean      		NetLibOpen;
	Boolean             privateBrowsing;
	Boolean             engineStop;
	Boolean             userCancel;
	Boolean             httpUseTempFile;
	Boolean             hasEvent;
	Boolean        		retransmission;
	Boolean             wasPost;
	Boolean             isImage;
	Boolean             onenterback;
	Boolean             refreshEvent;
	Boolean             NewCategoryEdited;
	Boolean             post;
	Boolean             wmlsBrowserContextHandled;
} GlobalsType_dummy;

typedef struct TabStackType {
	GlobalsType_dummy	*g;

	struct TabStackType	*next;
} TabStackType;

typedef TabStackType *TabStackPtr;

typedef struct TabArrayType {
	TabStackPtr	tabStack;
	UInt16		tabCount;
	UInt16		currentTab;
} TabArrayType;

typedef TabArrayType *TabArrayPtr;


/* !!! NOTE: If you update anything in GlobalsType be sure to update it 
       in the GlobalsType_dummy type above !! */
typedef struct {
	MemHandle    		resBmpPrgrs[14];
    MemHandle           srcHandle;
	MemHandle       	ConnectionsHandle;
	MemHandle       	ConnectionsListArrayH;
	FileRef             xhtmlOutputFileRef;
	FileRef             debugOutputFileRef;
	UInt32              CardOffset;
	UInt32              indent;
	UInt32              xindent;
	UInt32              fontH;
	UInt32              rightIndent;
	UInt32              contentLength;
	UInt32              timeElapse;
	UInt32              sid;
	UInt32              imgH;
	UInt32              imgY;
	UInt32              pushedPageLength;
	UInt32              gifContext;
	//UInt32			registerA4;
	//UInt32			registerA5;
	unsigned char 		*urlBuf;
	unsigned char 		*contentType;
	unsigned char 		*pageContentType;
	unsigned char 		*launchURL;
	char         		*inBuf;
	Char                *Url;
	Char                *EditURLConnection;
	Char                *OpenUrl;
	Char                *deckTitle;
	Char                *postBuf;
	Char                *AuthUser;
    Char                *AuthPass;
	Char                *onforward;
	Char                *partBound;
	Char                *rssFeed;
	Char                *pushedPageContent;
	Char                *pushedPageContentType;
	Char                *pushedPageTitle;
	Char                *wmlsData;
	void                *pngLibrary;
	InputPtr        	InputFldActive;
	InputPtr            Input; 				// linked list of input elements
	VarPtr              Var; 				// linked list of variables
	SelectPtr           Select;
	RadioInputPtr		radioBtn;
	CheckInputPtr		checkBox;
	HTMLFormPtr         Forms;
	HTMLFormPtr         ActiveForm;
	ButtonInputPtr		formButton;
	ImagePtr            inlineImage;
	MapPtr              ImageMaps;
	RSSPtr              rss;
	RSSPtr              tempRss;
	CardPtr             htmlPageLocID;
	ImagePtr            pageImages;
	ImagePtr            downloadImage;
	OLPtr               organizedList;
	WebFontPtr          font;
	SARPtr              segment;
	HorizIndentPtr		horizIndent;
	PositionType		*position;
	CookiePtr           CookieJar;
	HyperLinkPtr     	HyperLinks; 			// linked list of hyperlinks
	HyperLinkPtr		currentLink;
	TextPtr             text;
	DOMPtr              DOM;
	AreaPtr             areaPtr;
	HyperLinkPtr		selectedLink;
	PushPtr             Push; 
	AlignPtr            AlignList;
	historyPtr          History;
	DmOpenRef      		DbHistory;
	DmOpenRef           DbCookieJar;
	DmOpenRef    		DbUrlDb;           		// handle for URL database (bookmarks)
	DmOpenRef    		DbConnDb;          		// handle for Connection database 
	UInt16       		CurrentView;       		// id of current form
	UInt16              nextView;
	UInt16            	CurrentCategory;  		// category filter
	UInt16           	RecordCategory;   		// edited record's category (never All)
	UInt16           	NewCategory;    		// category in details form (never All)
	UInt16              charSet;
	UInt16              prevFormID;
	UInt16              positionIndent;
	UInt16              redirects;
	UInt16              DbUrlCurrentCategory;
	Int16        		progress;
	Int16        		DrawHeight;
	Int16               DrawWidth;
	Int16   	     	ScrollY;
	Int16               ScrollX;
	Int16        		x,y;
	Int16        		state;
	Int16        		sock;		// true if category name has been edited
	Int16               DbConnCurrentRecord;       	// index of current Connection database record
	Int16        		DbUrlCurrentRecord;       	// index of current URL database record
	Int16        		DbPushCurrentRecord;
	Int16               needSpace; 			// render an extra space
	Int16               onEvent;
	Int16        		wtpState;
	Int16               ActiveConnStore;
    unsigned short      tid;
    unsigned short      old_tid;
	char                WapLastUrl[MAX_URL_LEN + 1];
	char                CookieLastUrl[MAX_URL_LEN + 1];
	char         		DbUrlCategoryName[dmCategoryLength];     // Storage for the name
	char           		ActiveConnection[16];
	struct sockaddr_in  RmtAddress;
	struct sockaddr_in 	LclAddress;
	WebDataType       	webData;
	BrowserGlobalsType	browserGlobals;
	BrowserAPIType		BrowserAPI;
	OfflineType         offline;
	XHTML               xhtml;
	HTTPParse           httpParser;
	CacheType           cache;
	Alignment           Align;
	Alignment           imgAlign;
	dbPushMessage     	DbPushCurrentMessage;
	dbConnConnection 	CurrentConnection;
	dbConnConnection	conn;
	dbUrlURL     		DbUrlCurrentURL;
	WMLDeck         	wmldck;
	PageStatsType		pageStats;
	WindowPointerType	pointer;
	TabArrayType		tabList;
	gWAPAppInfoType 	WAPAppInfo;    	// for the connectionslist in the URLEditForm
	SndStreamRef		imageThreadRef;
	Int8                imageThreadLoadCount;
	int                 tid_new;
	Boolean             Render;
	Boolean      		NetLibOpen;
	Boolean             privateBrowsing;
	Boolean             engineStop;
	Boolean             userCancel;
	Boolean             httpUseTempFile;
	Boolean             hasEvent;
	Boolean        		retransmission;
	Boolean             wasPost;
	Boolean             isImage;
	Boolean             onenterback;
	Boolean             refreshEvent;
	Boolean             NewCategoryEdited;
	Boolean             post;
	Boolean             wmlsBrowserContextHandled;
} GlobalsType;
/* !!! NOTE: If you update anything in GlobalsType be sure to update it 
       in the GlobalsType_dummy type above !! */

#define RENDER_SMALL_SCREEN	0
#define RENDER_WIDE_SCREEN	1

#define FIVEWAY_MODE_REGULAR	0
#define FIVEWAY_MODE_POINTER	1


typedef struct WAPPreferenceType
{
	UInt32			maxCache;
	//char			ActiveConnStore[16];
	Int16			conn;
	Int16			timeout;
    Int16           debugMask;
	unsigned int    key;
	Char			downloadTo[256];
    Char            lastURL[MAX_URL_LEN];
	//Boolean			imgBrdr;
	Boolean			altAgent;
	Boolean			images;
	Boolean			mode;
	//Boolean			imgResize;
	Boolean			private;
	Boolean			debug;
	Boolean			disableUAProf;
	UInt8			agentNum;
	UInt8			fontSize; // 0 normal, 1 small, 2 large
	UInt8			cookie; // 0 decline, 1 accept, 2 ask
	UInt8			renderMode; // 0 small 1 wide
	UInt8			startDest; // 0 blank, 1 home, 2 book, 3 last url ...
	UInt8			fiveWayMode;
	UInt8			fiveWaySpeed;
	UInt8			fiveWayAccel;
	Int8			timeoutlist;

	/*Boolean		disconnect;
	Boolean			history;
	Boolean			open;
	Boolean			close;
	Boolean			agent;
	Boolean			start;
	char			CustomAgent[80];*/
} WAPPreferenceType;

extern WAPPreferenceType g_prefs;

void SwitchForm(UInt16 formID, GlobalsType *g);
void DisplayAbout();
void DisplayLicense();

#endif

